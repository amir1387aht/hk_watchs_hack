#include <rtthread.h>
#include "littlevgl2rtt.h"
#include "lvgl.h"
#include <py/compile.h>
#include <py/runtime.h>
#include <py/repl.h>
#include <py/gc.h>
#include <py/mperrno.h>
#include <py/stackctrl.h>
#include <py/frozenmod.h>
#include <lib/mp-readline/readline.h>
#include <lib/utils/pyexec.h>
#include "mpgetcharport.h"
#include "mpputsnport.h"
#include "lvsf.h"
#include "gui_app_fwk.h"
#include "dfs_file.h"

#define LV_PY_CMD_LEN 128
#define LV_PY_TASK_INTERVAL 20

#define EXEC_FLAG_PRINT_EOF (1)
#define EXEC_FLAG_ALLOW_DEBUGGING (2)
#define EXEC_FLAG_IS_REPL (4)
#define EXEC_FLAG_RERAISE (8)
#define EXEC_FLAG_SOURCE_IS_RAW_CODE (16)
#define EXEC_FLAG_SOURCE_IS_VSTR (32)
#define EXEC_FLAG_SOURCE_IS_FILENAME (64)

extern void mpy_run(const char *filename);

static vstr_t py_line;
static lv_task_t* lv_python_task;

#ifdef _MSC_VER
#pragma section("LV_NAMED_IMG$a", read)
const char __lv_named_img_begin_name[] = "__start";
__declspec(allocate("LV_NAMED_IMG$a")) const named_img_var_t __lv_named_img_begin =
{
    __lv_named_img_begin_name,
    NULL
};

#pragma section("LV_NAMED_IMG$z", read)
const char __lv_named_img_end_name[] = "__end";
__declspec(allocate("LV_NAMED_IMG$z")) const named_img_var_t __lv_named_img_end =
{
	__lv_named_img_end_name,
    NULL
};

void *lv_img_get_buf(const char *name)
{
	static named_img_var_t * lv_named_img_begin, *lv_named_img_end;
	
	if (lv_named_img_begin==NULL) {
		uint32_t * begin_ptr;
		begin_ptr=(uint32_t *)&__lv_named_img_begin;
		begin_ptr++;
		while ((*begin_ptr)==0) begin_ptr++;
		lv_named_img_begin=(named_img_var_t*)begin_ptr;		
	}
	if (lv_named_img_end==NULL) {
		uint32_t * end_ptr;
		end_ptr = (uint32_t *)&__lv_named_img_end;
		end_ptr--;
		while (*(end_ptr-1)==0) end_ptr--;
		lv_named_img_end=(named_img_var_t*)end_ptr;				
	}
	
    named_img_var_t * named_img = lv_named_img_begin;
    int count = (named_img_var_t *)lv_named_img_end - named_img;
	int i;
	void * r=NULL;
	
	for (i=0;i<count;i++) {
        if (named_img[i].name&&strcmp(name, named_img[i].name) == 0)
        {
            rt_kprintf("Got image var for %s\n", name);
            r= named_img[i].var;
			break;
        }
	}
	return r;
}
#else
void *lv_img_get_buf(const char *name)
{
    extern const int LV_NAMED_IMGTAB$$Base;
    extern const int LV_NAMED_IMGTAB$$Limit;
    named_img_var_t * named_img = (named_img_var_t *)&LV_NAMED_IMGTAB$$Base;
    int count = (named_img_var_t *)&LV_NAMED_IMGTAB$$Limit - named_img;
	int i;
	void * r=NULL;
	
	for (i=0;i<count;i++) {
        if (strcmp(name, named_img[i].name) == 0)
        {
            rt_kprintf("Got image var for %s\n", name);
            r= named_img[i].var;
			break;
        }
	}
	return r;
}

#endif

/************************* PYTHON application support *******************************************/


static const char python_app_prefix[]="PA_";
static char python_fname[32];
static void python_app_init(void *param)
{
    strcpy(python_fname, (const char*)param);
    strcat(python_fname, "/main.py");
	rt_kprintf("Python run %s\n", python_fname);
	mpy_run((const char *)python_fname);
}

void pyapp_msg_handler(gui_app_msg_type_t msg, void *param)
{
	char cbk_func[32];
    switch (msg)
    {
    case GUI_APP_MSG_ONSTART:
        python_app_init(param);
        break;

    case GUI_APP_MSG_ONRESUME:
    case GUI_APP_MSG_ONPAUSE:
    case GUI_APP_MSG_ONSTOP:
    default:
    {
		strcpy(cbk_func, (const char *)param);
		strcat(cbk_func, "_callback");
		rt_kprintf("Python callback %s(%d)\n", cbk_func,msg);
		mp_obj_t cbk = mp_load_name(qstr_from_str(cbk_func));
        mp_obj_t msg_obj = mp_obj_new_int(msg);
        mp_call_function_1(cbk, msg_obj);

    }
    break;
    }
}

static int pyapp_main(intent_t i)
{
    /* Regist root page message handler */
    gui_app_regist_msg_handler((const char *)i, pyapp_msg_handler);
    return 0;
}

#include "lv_ext_resource_manager.h"

const builtin_app_desc_t *gui_python_app_list_get_next(const builtin_app_desc_t *ptr_app)
{
	static struct dfs_fd fd;
	static struct dirent dirent;
	static builtin_app_desc_t desc;
	static char name[80];

	int length;
	builtin_app_desc_t * r=NULL;
    /* list directory */
	if (ptr_app==NULL)
		if (dfs_file_open(&fd, "/", O_DIRECTORY) != 0)
			return r;
    memset(&dirent, 0, sizeof(struct dirent));
    length = dfs_file_getdents(&fd, &dirent, sizeof(struct dirent));
	while (length>0&&strncmp(dirent.d_name,python_app_prefix, strlen(python_app_prefix)))
		length = dfs_file_getdents(&fd, &dirent, sizeof(struct dirent));		
    if (length > 0) {		
		r = &desc;
		r->entry=pyapp_main;
		r->icon= name;
		strcpy((char*)r->icon,dirent.d_name);
		strcat((char*)r->icon,"/thumb.bin");
		strcpy(r->id,dirent.d_name);
#ifdef _MSC_VER		
		r->magic_flag=MSC_APP_STRUCT_MAGIC_HEAD;
#endif
		rt_kprintf("name: %s\n", dirent.d_name);
    }
	else {
		dfs_file_close(&fd);
	}
    return (const builtin_app_desc_t *)r;
}

/************************* PYTHON watch face support *******************************************/

const char python_watch_prefix[]="PW_";

#ifdef SOLUTION_WATCH
#include "app_clock_comm.h"
#else
#include "app_clock_main.h"
#endif

int32_t pw_init(lv_obj_t * parent)
{
	pyapp_msg_handler(GUI_APP_MSG_ONSTART, (void*)app_clock_change_context());
	return RT_EOK;
}

int32_t pw_pause(void)
{
	pyapp_msg_handler(GUI_APP_MSG_ONPAUSE, (void*)app_clock_change_context());
	return RT_EOK;
}

int32_t pw_resume(void)
{
	pyapp_msg_handler(GUI_APP_MSG_ONRESUME, (void*)app_clock_change_context());
	return RT_EOK;
}
int32_t pw_deinit(void)
{
	pyapp_msg_handler(GUI_APP_MSG_ONSTOP, (void*)app_clock_change_context());
	return RT_EOK;
}

static app_clock_ops_t python_watch = {
	.init=pw_init,
	.pause=pw_pause,
	.resume=pw_resume,
	.deinit=pw_deinit,
};

void gui_python_watch_face_register()
{
	static struct dfs_fd fd;
	static struct dirent dirent;

	int length;
	if (dfs_file_open(&fd, "/", O_DIRECTORY) != 0)
		return ;
    memset(&dirent, 0, sizeof(struct dirent));
    length = dfs_file_getdents(&fd, &dirent, sizeof(struct dirent));
	while (length>0) {
		if (strncmp(dirent.d_name,python_watch_prefix, strlen(python_watch_prefix))==0)
		{
			char * name=malloc(strlen(dirent.d_name)+strlen("/thumb.bin"));
			
			strcpy(name,dirent.d_name);
			strcat(name,"/thumb.bin");
#ifdef SOLUTION_WATCH		
			app_clock_register(dirent.d_name,
				NULL,
				&python_watch,
				(const void *)name				
				);
#else
			app_clock_register(dirent.d_name, &python_watch);
#endif
		}
		length = dfs_file_getdents(&fd, &dirent, sizeof(struct dirent));		
    }
	dfs_file_close(&fd);
}

/************************* PYTHON main entry********* *******************************************/

extern int parse_compile_execute(const void *source, mp_parse_input_kind_t input_kind, int exec_flags) ;
void py_task_main(struct _lv_task_t * task)
{
    vstr_reset(&py_line);

raw_repl_reset:     
    for (;;) {
        int c = mp_getchar();
        if (c == 0xff) {
            break;
        }
        else if (c == CHAR_CTRL_A) {
            // reset raw REPL
            goto raw_repl_reset;
        } else if (c == CHAR_CTRL_B) {
            // change to friendly REPL
            rt_kprintf("\r\n");
            vstr_clear(&py_line);
            //pyexec_mode_kind = PYEXEC_MODE_FRIENDLY_REPL;
            //return 0;
        } else if (c == CHAR_CTRL_C) {
            // clear line
            vstr_reset(&py_line);
        } else if (c == CHAR_CTRL_D) {
            // input finished
            break;
        } else {
            // let through any other raw 8-bit value
            vstr_add_byte(&py_line, c);
            if (py_line.len >= LV_PY_CMD_LEN-1)
                break;
        }     
    }
    if (py_line.len > 0) {        
        int ret = parse_compile_execute(&py_line, MP_PARSE_FILE_INPUT, EXEC_FLAG_PRINT_EOF | EXEC_FLAG_SOURCE_IS_VSTR);        
        if (ret & PYEXEC_FORCED_EXIT) {
            rt_kprintf("Python force to exit???\n");
            //return ret;
        }
        vstr_reset(&py_line);
    }
}


void mpy_lv_init(void)
{
    vstr_init(&py_line, LV_PY_CMD_LEN);
    lv_python_task = lv_task_create(py_task_main, LV_PY_TASK_INTERVAL, LV_TASK_PRIO_MID, NULL);
}

