/*********************
 *      INCLUDES
 *********************/
#include <rtthread.h>
#include <rtdevice.h>
#include "lvgl.h"
#include "lvsf.h"
#include "gui_app_fwk.h"
#include "gui_app_int.h"


//#if defined(_MSC_VER)
#if 0
typedef struct
{
    uint32_t app_idx;
    uint32_t page_idx;
    gui_app_msg_type_t  msg;
} page_history_t;


static page_state_enum pages_state[9][9];

static const char *p_expect_history_name = NULL;
#define HISTORY_IDX_INIT  (-1)
#define HISTORY_IDX_DISABLE  (-2)
static int32_t p_received_history_idx;
static page_history_t *p_expect_history_start;
static page_history_t *p_expect_history_end;
static int expect_history_line;
static struct rt_semaphore test_sema;

static page_history_t received_history[30];
/*
 0 - Resumed
 1 - Request suspend
 2 - Suspended
 3 - Request resume
*/
static volatile uint8_t en_suspend_app_schedule = 0;
#define max_test_apps   (sizeof(pages_state)/sizeof(pages_state[0]))
#define max_test_subpages (sizeof(pages_state[0])/sizeof(pages_state[0][0]))
#define max_received_history (sizeof(received_history)/sizeof(received_history[0]))

#define MAX_TIMEOUT_MS 1000
#define max_running_apps_default_v 2
#define destory_suspend_app_defalut_v 1
#define main_app_name_default_v  "Main"

#define DISABLE_EXPECT_LOG()  do{p_received_history_idx = HISTORY_IDX_DISABLE;}while(0)
#define SET_EXPECT_LOG(log) examine_log_init(#log, (page_history_t *) &(log), sizeof(log), __LINE__)
#define CHECK_EXPECT_LOG(wait_ms) examine_log_start(wait_ms, __LINE__)


static bool all_subpages_stopped(void)
{
    for (uint16_t i = 0; i < max_test_apps; i++)
        for (uint16_t j = 0; j < max_test_subpages; j++)
        {
            if (pages_state[i][j] != page_st_stoped)
            {
                rt_kprintf("app[%d], page[%d], state:%d\r\n", i, j, pages_state[i][j]);
                return false;
            }
        }


    return true;
}

static const char *get_msg_str(gui_app_msg_type_t msg_id)
{
    const char *state_str;

    switch (msg_id)
    {
    case GUI_APP_MSG_ONSTART:
        state_str = "GUI_APP_MSG_ONSTART";
        break;
    case GUI_APP_MSG_ONRESUME:
        state_str = "GUI_APP_MSG_ONRESUME";
        break;
    case GUI_APP_MSG_ONPAUSE:
        state_str = "GUI_APP_MSG_ONPAUSE";
        break;
    case GUI_APP_MSG_ONSTOP:
        state_str = "GUI_APP_MSG_ONSTOP";
        break;
    default:
        state_str = "UNKNOWN";
        break;
    }

    return state_str;
}

static void suspend_handler(void *data)
{
    while (en_suspend_app_schedule != 0)
    {
        rt_thread_mdelay(10);
        if (1 == en_suspend_app_schedule) en_suspend_app_schedule = 2;
        if (3 == en_suspend_app_schedule) en_suspend_app_schedule = 0;
    }
}

static void suspend_app_schedule_host_task(void)
{
    en_suspend_app_schedule = 1;
    lv_async_call(suspend_handler, NULL);
    while (en_suspend_app_schedule != 2)
    {
        rt_thread_mdelay(10);
    }
}


static void resume_app_schedule_host_task(void)
{
    en_suspend_app_schedule = 3;
    while (en_suspend_app_schedule != 0)
    {
        rt_thread_mdelay(10);
    }
    //lv_async_call_cancel(suspend_handler, NULL);
}

static void examine_log_init(const char *log_name, page_history_t *p_log, uint32_t log_size, uint32_t line_num)
{
    suspend_app_schedule_host_task();

    p_expect_history_start = p_log;
    p_expect_history_name = log_name;

    expect_history_line = line_num;
    p_expect_history_end = (page_history_t *)(((uint32_t)p_log) + log_size);
    rt_sem_control(&test_sema, RT_IPC_CMD_RESET, 0);
    rt_kprintf("\r\n\r\n----------------------------------------------\r\n");
    rt_kprintf("---------------------------- line %d ----------\n", line_num);
    rt_kprintf("---------------------------------------------\n");

    p_received_history_idx = HISTORY_IDX_INIT;
}


static void examine_log_start(uint32_t wait_ms, uint32_t line_num)
{
    uint32_t rec_cnt, err = 0;

    (void) line_num;
    resume_app_schedule_host_task();

    if (RT_EOK != rt_sem_take(&test_sema, rt_tick_from_millisecond(wait_ms)))
    {
        rt_kprintf("\r\nTimeout to rec history\r\n");
        //err = 1;
    }

    rec_cnt = p_received_history_idx + 1;

    if ((p_expect_history_start + rec_cnt != p_expect_history_end)
            || (memcmp(p_expect_history_start, received_history, sizeof(page_history_t) * rec_cnt) != 0))
    {
        err = 1;
    }

    if (err)
    {

        rt_kprintf("\r\nError at history %s, line %d \r\n", p_expect_history_name, expect_history_line);
        rt_kprintf(" EXPECT \t\t\t  RECEIVEVD  \r\n");
        for (int32_t i = 0; i < max_received_history; i++)
        {
            uint8_t done = 0;
            if (p_expect_history_start + i < p_expect_history_end)
            {
                rt_kprintf("{%d, %d, %s}\t\t", p_expect_history_start[i].app_idx,
                           p_expect_history_start[i].page_idx,
                           get_msg_str(p_expect_history_start[i].msg));
            }
            else
            {
                rt_kprintf("-----------\t\t");
                done ++;
            }



            if (i <= p_received_history_idx)
            {
                if (0 == done)
                {
                    if (0 == memcmp(&received_history[i], &p_expect_history_start[i], sizeof(page_history_t)))
                        rt_kprintf(" == ");
                    else
                        rt_kprintf(" != ");
                }
                else
                {
                    rt_kprintf(" -- ");
                }


                rt_kprintf("{%d, %d, %s},\n", received_history[i].app_idx,
                           received_history[i].page_idx,
                           get_msg_str(received_history[i].msg));

            }
            else
            {
                rt_kprintf("-----------\n");
                done++;
            }

            if (2 == done) break;
        }

        list_schedule_app();
        rt_thread_mdelay(100);
        RT_ASSERT(0);
    }

    list_schedule_app();
}

static void msg_handler(gui_app_msg_type_t msg, void *param)
{
    uint32_t app_idx, page_idx;

    page_state_enum page_new_state;

    subpage_node_t *this_page = app_schedule_get_this_page();
    gui_runing_app_t *this_app = this_page->parent;

    app_idx = strtoul(this_app->id, 0, 10);
    page_idx = strtoul(this_page->name, 0, 10);

    switch (msg)
    {
    case GUI_APP_MSG_ONSTART:
        page_new_state = page_st_created;
        break;

    case GUI_APP_MSG_ONRESUME:
        page_new_state = page_st_resumed;

        break;

    case GUI_APP_MSG_ONPAUSE:
        page_new_state = page_st_paused;

        break;

    case GUI_APP_MSG_ONSTOP:
        page_new_state = page_st_stoped;
        break;

    default:
        RT_ASSERT(0);
        break;
    }


    //Never do same action.
    if (page_new_state != pages_state[app_idx][page_idx])
    {
        //rt_kprintf("app %d, page %d, old_state[%d] -> new_state[%d]\r\n", app_idx, page_idx, pages_state[app_idx][page_idx], page_new_state);
        pages_state[app_idx][page_idx] = page_new_state;
    }
    else
    {
        rt_kprintf("app %d, page %d, %d!=%d\r\n", app_idx, page_idx, pages_state[app_idx][page_idx], page_new_state);
        RT_ASSERT(0);
    }

    if (HISTORY_IDX_DISABLE == p_received_history_idx)
    {
        ;
    }
    else if (p_received_history_idx < (int32_t)max_received_history)
    {
        p_received_history_idx = p_received_history_idx + 1;
        received_history[p_received_history_idx].app_idx = app_idx;
        received_history[p_received_history_idx].page_idx = page_idx;
        received_history[p_received_history_idx].msg = msg;
    }
    else
    {
        RT_ASSERT(0);
    }

}


static int test_app_entry(intent_t i)
{
    uint32_t idx;
    char page_name[SUBPAGE_MAX_LEN];
    const char *page_list;
    //gui_runing_app_t * this_app = app_schedule_get_this();

    page_list = intent_get_string(i, "pages");

    if (page_list)
    {
        rt_kprintf("App[%s] create page list=[%s] \r\n", app_schedule_get_this()->id, page_list);
        for (idx = 0; idx < strlen(page_list); idx ++)
        {
            memset(page_name, 0, sizeof(page_name));
            rt_sprintf(page_name, "%c", page_list[idx]);
            gui_app_create_page(page_name, msg_handler);
        }
    }

    return 0;
}

static int create_page(uint32_t idx)
{
    char page_name[SUBPAGE_MAX_LEN];
    rt_err_t ret;

    memset(page_name, 0, sizeof(page_name));
    rt_sprintf(page_name, "%d", idx);
    ret = gui_app_create_page(page_name, msg_handler);
    RT_ASSERT(RT_EOK == ret);

    return 0;
}

static void idle_hook(app_sche_state state)
{
    if (APP_SCHE_IDLE == state)
    {
        uint32_t rec_cnt = p_received_history_idx + 1;
        if (p_expect_history_start + rec_cnt == p_expect_history_end)
        {
            //Done, release semaphore.
            rt_err_t err = rt_sem_release(&test_sema);
            RT_ASSERT(RT_EOK == err);
        }
    }
}

static int init(void)
{
    {
        //App name must NOT be 0 on MSVC, or MSVC couldn't  found built in app section
        BUILTIN_APP_EXPORT(0, 0, "1", test_app_entry); //App 1
    }

    {
        BUILTIN_APP_EXPORT(0, 0, "2", test_app_entry); //App 2
    }

    {
        BUILTIN_APP_EXPORT(0, 0, "3", test_app_entry); //App 3
    }

    {
        BUILTIN_APP_EXPORT(0, 0, "4", test_app_entry); //App 4
    }

    return 0;
}
#ifndef _MSC_VER
    INIT_APP_EXPORT(init);
#endif


//Sanity test
static rt_err_t app_test_000(void)
{
#ifndef _MSC_VER
    const page_history_t expect_log1[] =
    {
    };

    SET_EXPECT_LOG(expect_log1);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);
#endif

    return RT_EOK;
}


static rt_err_t app_test_001(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log2[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE},
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},

        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 2, 3, GUI_APP_MSG_ONSTART},
        { 2, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log2);
    gui_app_run("2 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log3[] =
    {
        /*Issue 1190*/
        { 1, 3, GUI_APP_MSG_ONSTOP},
        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},


        { 2, 3, GUI_APP_MSG_ONPAUSE},
        { 3, 1, GUI_APP_MSG_ONSTART},
        { 3, 1, GUI_APP_MSG_ONRESUME},

        { 3, 1, GUI_APP_MSG_ONPAUSE},
        { 3, 2, GUI_APP_MSG_ONSTART},
        { 3, 2, GUI_APP_MSG_ONRESUME},

        { 3, 2, GUI_APP_MSG_ONPAUSE},
        { 3, 3, GUI_APP_MSG_ONSTART},
        { 3, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log3);
    gui_app_run("3 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log4[] =
    {
        /*Issue 1588*/
        { 3, 3, GUI_APP_MSG_ONPAUSE},
        { 2, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log4);
    gui_app_run("2 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log5[] =
    {
        { 2, 2, GUI_APP_MSG_ONSTOP}, //Remove page 2

        //{Invalid page, NO event here}, //Remove page 2

        { 2, 3, GUI_APP_MSG_ONPAUSE},  //Goback
        { 2, 1, GUI_APP_MSG_ONRESUME},
        { 2, 3, GUI_APP_MSG_ONSTOP},

        { 2, 1, GUI_APP_MSG_ONPAUSE}, //Goback
        { 3, 3, GUI_APP_MSG_ONRESUME},
        { 2, 1, GUI_APP_MSG_ONSTOP},

    };

    SET_EXPECT_LOG(expect_log5);
    gui_app_remove_page("2");
    gui_app_remove_page("2"); //Remove invalid page
    gui_app_goback();
    gui_app_goback();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log6[] =
    {
        { 3, 3, GUI_APP_MSG_ONPAUSE},
        { 1, 1, GUI_APP_MSG_ONSTART},
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log6);
    gui_app_run("1 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log7[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE},  //gui_app_self_exit();
        { 3, 3, GUI_APP_MSG_ONRESUME},

        { 1, 3, GUI_APP_MSG_ONSTOP},
        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},

    };

    SET_EXPECT_LOG(expect_log7);
    gui_app_self_exit();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log8[] =
    {
        //{3 is resumed, NO event here}, //gui_app_goback_to_page("3");

        { 3, 3, GUI_APP_MSG_ONPAUSE}, //gui_app_goback_to_page("1");
        { 3, 1, GUI_APP_MSG_ONRESUME},
        { 3, 3, GUI_APP_MSG_ONSTOP},
        { 3, 2, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log8);
    gui_app_goback_to_page("3");
    gui_app_goback_to_page("1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log9[] =
    {
        { 3, 1, GUI_APP_MSG_ONPAUSE},
        { 3, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log9);
    gui_app_cleanup();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    return RT_EOK;
}

static rt_err_t app_test_002(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log2[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE},
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},

        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 2, 3, GUI_APP_MSG_ONSTART},
        { 2, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log2);
    gui_app_run("2 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log3[] =
    {
        { 2, 3, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONRESUME},

        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 2, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log3);
    //rt_thread_mdelay(1000);//Wait previous tans anim finished
    gui_app_manual_animation_start(0);
    gui_app_manual_animation_update(MANUAL_TRAN_ANIM_MAX_PROCESS / 3);
    gui_app_manual_animation_stop(MANUAL_TRAN_ANIM_MAX_PROCESS / 4);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log4[] =
    {
        { 2, 3, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONRESUME},

        { 2, 3, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log4);
    //rt_thread_mdelay(1000);//Wait previous tans anim finished
    gui_app_manual_animation_start(0);
    gui_app_manual_animation_update(MANUAL_TRAN_ANIM_MAX_PROCESS / 3);
    gui_app_manual_animation_stop(MANUAL_TRAN_ANIM_MAX_PROCESS);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log5[] =
    {
        { 2, 2, GUI_APP_MSG_ONPAUSE},  //Go back to 1
        { 2, 1, GUI_APP_MSG_ONRESUME},
        { 2, 2, GUI_APP_MSG_ONSTOP},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONRESUME},

        { 1, 3, GUI_APP_MSG_ONPAUSE},
        { 2, 1, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log5);
    //rt_thread_mdelay(1000);//Wait previous tans anim finished
    gui_app_goback_to_page("1");

    gui_app_manual_animation_start(0);
    gui_app_manual_animation_update(MANUAL_TRAN_ANIM_MAX_PROCESS / 3);
    gui_app_manual_animation_stop(MANUAL_TRAN_ANIM_MAX_PROCESS / 4);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log6[] =
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log6);
    gui_app_manual_animation_start(0);
    gui_app_manual_animation_update(MANUAL_TRAN_ANIM_MAX_PROCESS / 3);
    gui_app_manual_animation_stop(MANUAL_TRAN_ANIM_MAX_PROCESS);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log7_1[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE}, //Start manual anim
        { 1, 2, GUI_APP_MSG_ONRESUME},

    };

    SET_EXPECT_LOG(expect_log7_1);
    gui_app_manual_animation_start(0);
    gui_app_manual_animation_update(MANUAL_TRAN_ANIM_MAX_PROCESS / 3);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log7_2[] =
    {
        { 1, 2, GUI_APP_MSG_ONPAUSE},   //Abort anim by "gui_app_run("2 pages=123");"
        { 1, 3, GUI_APP_MSG_ONRESUME},

        { 1, 3, GUI_APP_MSG_ONPAUSE}, //gui_app_run("2 pages=123");
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},
        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},
        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 2, 3, GUI_APP_MSG_ONSTART},
        { 2, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log7_2);
    gui_app_manual_animation_stop(MANUAL_TRAN_ANIM_MAX_PROCESS / 4);
    gui_app_run("2 pages=123"); //Send msg
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log8[] =
    {
        { 2, 3, GUI_APP_MSG_ONPAUSE},

        { 1, 3, GUI_APP_MSG_ONSTOP},
        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},

        { 2, 3, GUI_APP_MSG_ONSTOP},
        { 2, 2, GUI_APP_MSG_ONSTOP},
        { 2, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log8);
    gui_app_cleanup();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    return RT_EOK;
}

static rt_err_t app_test_003(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    const page_history_t expect_log3[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE},  //Remove page 3
        { 1, 2, GUI_APP_MSG_ONRESUME},
        { 1, 3, GUI_APP_MSG_ONSTOP},



        { 1, 2, GUI_APP_MSG_ONPAUSE}, //Create page 3
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},


        { 1, 3, GUI_APP_MSG_ONPAUSE},  //Remove page 3
        { 1, 2, GUI_APP_MSG_ONRESUME},
        { 1, 3, GUI_APP_MSG_ONSTOP},



        { 1, 2, GUI_APP_MSG_ONPAUSE}, //Create page 3
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log3);
    gui_app_remove_page("3"); //Remove active page
    create_page(3);
    gui_app_remove_page("3"); //Remove active page
    create_page(3);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log4[] =
    {
        { 1, 2, GUI_APP_MSG_ONSTOP}, //Remove page 2

        //{Invalid page, NO event here}, //Remove page 2

        { 1, 3, GUI_APP_MSG_ONPAUSE},  //Remove page 3
        { 1, 1, GUI_APP_MSG_ONRESUME},
        { 1, 3, GUI_APP_MSG_ONSTOP},

        { 1, 1, GUI_APP_MSG_ONPAUSE}, //Remove page 1
        { 1, 1, GUI_APP_MSG_ONSTOP},

    };

    SET_EXPECT_LOG(expect_log4);
    gui_app_remove_page("2");
    gui_app_remove_page("2");
    gui_app_remove_page("3"); //Remove active page
    gui_app_remove_page("1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    return RT_EOK;
}


//Clean up test
static rt_err_t app_test_004(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=123");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log2[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONSTOP},
        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log2);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    return RT_EOK;
}


//http://10.21.20.167/issues/1074     2 subpage onresumed at sametime
static rt_err_t app_test_005(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART}, //Run app 1
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},



        { 1, 2, GUI_APP_MSG_ONPAUSE},  //Run app 2
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=12");
    gui_app_run("2 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log2[] =
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE}, //Start manual anim
        { 1, 2, GUI_APP_MSG_ONRESUME},

    };
    SET_EXPECT_LOG(expect_log2);
    gui_app_manual_animation_start(0);
    gui_app_manual_animation_update(MANUAL_TRAN_ANIM_MAX_PROCESS / 3);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log3[] =
    {

        { 1, 2, GUI_APP_MSG_ONPAUSE}, //Abort manual anim by 'create_page(3)'
        { 2, 1, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        //{ 1, 2, GUI_APP_MSG_ONRESUME}, //<<< wrong state: both 1-2, 1-3 are resumed
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log3);
    gui_app_manual_animation_stop(MANUAL_TRAN_ANIM_MAX_PROCESS / 4);
    create_page(3);//Create page on app 1
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log4[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE},
        { 2, 1, GUI_APP_MSG_ONSTOP},
        { 1, 3, GUI_APP_MSG_ONSTOP},
        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},

    };

    SET_EXPECT_LOG(expect_log4);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    return RT_EOK;
}

//Suspend, and resume fwk
static rt_err_t app_test_006(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART}, //Run app 1
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},



        { 1, 2, GUI_APP_MSG_ONPAUSE},  //Run app 2
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=12");
    gui_app_run("2 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    /*Case 1:  suspend & resume only*/
    const page_history_t expect_log2p1_a[] =
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE}, //suspend fwk

    };
    SET_EXPECT_LOG(expect_log2p1_a);
    gui_app_fwk_suspend();//suspend fwk
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    const page_history_t expect_log2p1_b[] =
    {
        { 2, 1, GUI_APP_MSG_ONRESUME}, //resume fwk
    };
    SET_EXPECT_LOG(expect_log2p1_b);
    gui_app_fwk_resume();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    /*Case 2:  suspend and create a page, then resume:


            created page should execution after call 'gui_app_fwk_resume'
    */
    const page_history_t expect_log2p2_a[] =
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE}, //suspend fwk

    };
    SET_EXPECT_LOG(expect_log2p2_a);
    gui_app_fwk_suspend();//suspend fwk
    create_page(2);    //Create page on app 2
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    const page_history_t expect_log2p2_b[] =
    {
        { 2, 2, GUI_APP_MSG_ONSTART}, //Create page 2 on app
        { 2, 2, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log2p2_b);
    gui_app_fwk_resume();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    const page_history_t expect_log2p2_c[] =
    {
        { 2, 2, GUI_APP_MSG_ONPAUSE}, //go back page 2
        { 2, 1, GUI_APP_MSG_ONRESUME},
        { 2, 2, GUI_APP_MSG_ONSTOP},
    };
    SET_EXPECT_LOG(expect_log2p2_c);
    gui_app_goback();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log4[] =
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},
        { 2, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log4);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    return RT_EOK;

}


//Resume suspended app test
static rt_err_t app_test_007(void)
{
    app_schedule_destory_suspend_apps(0);

    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},  //"1 pages=12"
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},  //"2 pages=12"
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=12");
    gui_app_run("2 pages=12");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


#if 1
    const page_history_t expect_log2[] =
    {
        { 1, 2, GUI_APP_MSG_ONSTOP},    //"3 pages=1"
        { 1, 1, GUI_APP_MSG_ONSTOP},
        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 3, 1, GUI_APP_MSG_ONSTART},
        { 3, 1, GUI_APP_MSG_ONRESUME},

    };
    SET_EXPECT_LOG(expect_log2);
    gui_app_run("3 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    const page_history_t expect_log2_1[] =
    {
        { 2, 2, GUI_APP_MSG_ONSTOP},    //"4 pages=1"
        { 2, 1, GUI_APP_MSG_ONSTOP},
        { 3, 1, GUI_APP_MSG_ONPAUSE},
        { 4, 1, GUI_APP_MSG_ONSTART},
        { 4, 1, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log2_1);
    gui_app_run("4 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

#else
    const page_history_t expect_log2[] =
    {
        { 1, 2, GUI_APP_MSG_ONSTOP},    //"3 pages=1"
        { 1, 1, GUI_APP_MSG_ONSTOP},
        { 3, 1, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 3, 1, GUI_APP_MSG_ONRESUME},



        { 2, 2, GUI_APP_MSG_ONSTOP},    //"4 pages=1"
        { 2, 1, GUI_APP_MSG_ONSTOP},
        { 4, 1, GUI_APP_MSG_ONSTART},
        { 3, 1, GUI_APP_MSG_ONPAUSE},
        { 4, 1, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log2);
    gui_app_run("3 pages=1");
    gui_app_run("4 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);
#endif /* 1 */



    const page_history_t expect_log3[] =
    {
        { 4, 1, GUI_APP_MSG_ONPAUSE},    //"goback"
        { 3, 1, GUI_APP_MSG_ONRESUME},
        { 4, 1, GUI_APP_MSG_ONSTOP},


        { 3, 1, GUI_APP_MSG_ONPAUSE},    //"goback"
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},
        { 3, 1, GUI_APP_MSG_ONSTOP},
        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log3);
    gui_app_goback();
    gui_app_exit("3");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log4[] =
    {
        { 2, 2, GUI_APP_MSG_ONPAUSE},    //"exit 2"
        { 1, 1, GUI_APP_MSG_ONSTART},
        { 1, 1, GUI_APP_MSG_ONRESUME},
        { 2, 2, GUI_APP_MSG_ONSTOP},
        { 2, 1, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log4);
    gui_app_exit("2");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log5[] =
    {
        { 1, 2, GUI_APP_MSG_ONPAUSE},
        { 1, 1, GUI_APP_MSG_ONRESUME},
        { 1, 2, GUI_APP_MSG_ONSTOP},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log5);
    gui_app_goback();
    gui_app_goback();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();


    app_schedule_destory_suspend_apps(destory_suspend_app_defalut_v);

    return RT_EOK;
}


//Restart suspended app with different parameters
static rt_err_t app_test_008(void)
{
    app_schedule_destory_suspend_apps(0);

    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},  //"1 pages=12"
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},  //"2 pages=12"
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=12");
    gui_app_run("2 pages=12");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log2[] =
    {
        { 1, 2, GUI_APP_MSG_ONSTOP},    //"3 pages=1"
        { 1, 1, GUI_APP_MSG_ONSTOP},
        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 3, 1, GUI_APP_MSG_ONSTART},
        { 3, 1, GUI_APP_MSG_ONRESUME},

    };
    SET_EXPECT_LOG(expect_log2);
    gui_app_run("3 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    const page_history_t expect_log2_1[] =
    {
        { 2, 2, GUI_APP_MSG_ONSTOP},    //"1 pages=1"
        { 2, 1, GUI_APP_MSG_ONSTOP},
        { 3, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 1, GUI_APP_MSG_ONSTART},
        { 1, 1, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log2_1);
    gui_app_run("1 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);






    const page_history_t expect_cleanup_log[] =
    {
        { 1, 1, GUI_APP_MSG_ONPAUSE},    //
        { 3, 1, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},
    };
    SET_EXPECT_LOG(expect_cleanup_log);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    app_schedule_destory_suspend_apps(destory_suspend_app_defalut_v);

    return RT_EOK;
}


//Refresh subpage & Clean backgroud apps
static rt_err_t app_test_009(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},  //"1 pages=12"
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},

        { 1, 2, GUI_APP_MSG_ONPAUSE},  //"2 pages=12"
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=12");
    gui_app_run("2 pages=12");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log2[] =
    {
        { 2, 2, GUI_APP_MSG_ONPAUSE},
        { 2, 2, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log2);
    gui_app_refr_page("1");
    gui_app_refr_page("2");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log3[] =
    {
        { 1, 2, GUI_APP_MSG_ONSTOP},    //"1 pages=1"
        { 1, 1, GUI_APP_MSG_ONSTOP},
    };
    SET_EXPECT_LOG(expect_log3);
    gui_app_cleanup_bg_apps();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);






    const page_history_t expect_cleanup_log[] =
    {
        { 2, 2, GUI_APP_MSG_ONPAUSE},    //
        { 2, 2, GUI_APP_MSG_ONSTOP},
        { 2, 1, GUI_APP_MSG_ONSTOP},
    };
    SET_EXPECT_LOG(expect_cleanup_log);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    return RT_EOK;
}

//Restart last app(actived)
static rt_err_t app_test_010(void)
{
    app_schedule_change_main_app_id("1");

    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},  //"1 pages=1"
        { 1, 1, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log2[] =
    {
        { 1, 1, GUI_APP_MSG_ONPAUSE},    //"1 pages=2"
        { 1, 1, GUI_APP_MSG_ONSTOP},


        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log2);
    gui_app_run("1 pages=2");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_cleanup_log[] =
    {
        { 1, 2, GUI_APP_MSG_ONPAUSE},    //
        { 1, 2, GUI_APP_MSG_ONSTOP},
    };
    SET_EXPECT_LOG(expect_cleanup_log);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    app_schedule_change_main_app_id(main_app_name_default_v);

    return RT_EOK;
}

//Restart current active app
static rt_err_t app_test_011(void)
{

    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},  //"1 pages=1"
        { 1, 1, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log2[] =   //"2 pages=1"
    {
        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log2);
    gui_app_run("2 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log3[] =   //"2 pages=2"
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE}, //Exit app 2 first.
        { 1, 1, GUI_APP_MSG_ONRESUME},
        { 2, 1, GUI_APP_MSG_ONSTOP},

        { 1, 1, GUI_APP_MSG_ONPAUSE}, //"Open page 2 on app2"
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log3);
    gui_app_run("2 pages=2");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_cleanup_log[] =
    {
        { 2, 2, GUI_APP_MSG_ONPAUSE},    //
        { 1, 1, GUI_APP_MSG_ONSTOP},
        { 2, 2, GUI_APP_MSG_ONSTOP},

    };
    SET_EXPECT_LOG(expect_cleanup_log);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);

    return RT_EOK;
}

//Restart current app with different parameter and run another app immediately. Redmin 3207.
static rt_err_t app_test_012(void)
{

    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART},  //"1 pages=1"
        { 1, 1, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log2[] =   //"2 pages=1"
    {
        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log2);
    gui_app_run("2 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log3[] =   //"2 pages=2"
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE}, //Exit app 2 first.
        { 1, 1, GUI_APP_MSG_ONRESUME},
        { 2, 1, GUI_APP_MSG_ONSTOP},

        { 1, 1, GUI_APP_MSG_ONPAUSE}, //"Open page 2 on app2"
        { 2, 2, GUI_APP_MSG_ONSTART},
        { 2, 2, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONSTOP  },  //Open page 2 on app3
        { 2, 2, GUI_APP_MSG_ONPAUSE  },
        { 3, 2, GUI_APP_MSG_ONSTART  },
        { 3, 2, GUI_APP_MSG_ONRESUME },
    };

    SET_EXPECT_LOG(expect_log3);


    gui_app_run("2 pages=2");
    gui_app_run_now("3 pages=2");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_cleanup_log[] =
    {
        {3, 2, GUI_APP_MSG_ONPAUSE},
        {2, 2, GUI_APP_MSG_ONSTOP},
        {3, 2, GUI_APP_MSG_ONSTOP},
    };
    SET_EXPECT_LOG(expect_cleanup_log);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    return RT_EOK;
}

//Redmine Bug #3415 launch duplicated app1
static rt_err_t app_test_013(void)
{
    const page_history_t expect_log1[] =
    {

        {1, 1, GUI_APP_MSG_ONSTART},
        {1, 1, GUI_APP_MSG_ONRESUME},
        {2, 1, GUI_APP_MSG_ONSTART},
        {1, 1, GUI_APP_MSG_ONPAUSE},
        {2, 1, GUI_APP_MSG_ONRESUME},
        {3, 1, GUI_APP_MSG_ONSTART},
        {2, 1, GUI_APP_MSG_ONPAUSE},
        {1, 1, GUI_APP_MSG_ONSTOP},
        {3, 1, GUI_APP_MSG_ONRESUME},

        {2, 1, GUI_APP_MSG_ONSTOP},
        {3, 1, GUI_APP_MSG_ONPAUSE},
        {1, 1, GUI_APP_MSG_ONSTART},
        {1, 1, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=1");
    gui_app_run("2 pages=1");
    gui_app_run("3 pages=1");
    gui_app_run("1 pages=1");
    gui_app_run("1 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS * 3);



    const page_history_t expect_cleanup_log[] =
    {
        {1, 1, GUI_APP_MSG_ONPAUSE},
        {3, 1, GUI_APP_MSG_ONSTOP},
        {1, 1, GUI_APP_MSG_ONSTOP},
    };
    SET_EXPECT_LOG(expect_cleanup_log);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    return RT_EOK;
}



static rt_err_t app_test_014(void)
{
    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART}, //Run app 1
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},



        { 1, 2, GUI_APP_MSG_ONPAUSE},  //Run app 2
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},

    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=12");
    gui_app_run("2 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log2[] =
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE}, //Start manual anim
        { 1, 2, GUI_APP_MSG_ONRESUME},

    };
    SET_EXPECT_LOG(expect_log2);
    gui_app_manual_animation_start(0);
    gui_app_manual_animation_update(MANUAL_TRAN_ANIM_MAX_PROCESS / 3);
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log3[] =
    {

        { 1, 2, GUI_APP_MSG_ONPAUSE}, //Abort manual anim by 'create_page(3)'
        { 2, 1, GUI_APP_MSG_ONRESUME},

        { 2, 1, GUI_APP_MSG_ONPAUSE},
        //{ 1, 2, GUI_APP_MSG_ONRESUME}, //<<< wrong state: both 1-2, 1-3 are resumed
        { 1, 3, GUI_APP_MSG_ONSTART},
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };
    SET_EXPECT_LOG(expect_log3);
    gui_app_manual_animation_stop(MANUAL_TRAN_ANIM_MAX_PROCESS / 4);
    create_page(3);//Create page on app 1
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log4[] =
    {
        { 2, 1, GUI_APP_MSG_ONSTOP},

        { 1, 3, GUI_APP_MSG_ONPAUSE},  //Run app 3-1
        { 3, 1, GUI_APP_MSG_ONSTART},
        { 3, 1, GUI_APP_MSG_ONRESUME},

    };

    SET_EXPECT_LOG(expect_log4);
    gui_app_run("3 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log5[] =
    {
        { 3, 1, GUI_APP_MSG_ONPAUSE},

        { 1, 3, GUI_APP_MSG_ONSTOP},
        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},
        { 3, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log5);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    return RT_EOK;
}



static rt_err_t app_test_015(void)
{
    app_schedule_max_running_apps(1);

    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART}, //Run app 1
        { 1, 1, GUI_APP_MSG_ONRESUME},

        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTART},
        { 1, 2, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=12");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    const page_history_t expect_log2[] =
    {
        { 1, 2, GUI_APP_MSG_ONPAUSE},  //Run app 2
        { 2, 1, GUI_APP_MSG_ONSTART},
        { 2, 1, GUI_APP_MSG_ONRESUME},


        { 1, 2, GUI_APP_MSG_ONSTOP},
        { 1, 1, GUI_APP_MSG_ONSTOP},

    };
    SET_EXPECT_LOG(expect_log2);
    gui_app_run("2 pages=1");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);




    const page_history_t expect_log5[] =
    {
        { 2, 1, GUI_APP_MSG_ONPAUSE},
        { 2, 1, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log5);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    app_schedule_max_running_apps(max_running_apps_default_v);

    return RT_EOK;
}

/*Can NOT pass for now!!!
    Expect that page 3 is the last resumed, but it is page 2 now.
*/
static rt_err_t app_test_016(void)
{

    const page_history_t expect_log1[] =
    {
        { 1, 1, GUI_APP_MSG_ONSTART}, //Run app 1, page 1
        { 1, 1, GUI_APP_MSG_ONRESUME},
        { 1, 1, GUI_APP_MSG_ONPAUSE},
        { 1, 1, GUI_APP_MSG_ONSTOP},

        { 1, 2, GUI_APP_MSG_ONSTART}, //Run app 1, page 2
        { 1, 2, GUI_APP_MSG_ONRESUME},
        { 1, 2, GUI_APP_MSG_ONPAUSE},
        { 1, 2, GUI_APP_MSG_ONSTOP},

        { 1, 3, GUI_APP_MSG_ONSTART}, //Run app 1, page 3
        { 1, 3, GUI_APP_MSG_ONRESUME},
    };

    SET_EXPECT_LOG(expect_log1);
    gui_app_run("1 pages=1");
    gui_app_run("1 pages=2");
    gui_app_run("1 pages=3");
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);


    const page_history_t expect_log2[] =
    {
        { 1, 3, GUI_APP_MSG_ONPAUSE},
        { 1, 3, GUI_APP_MSG_ONSTOP},
    };

    SET_EXPECT_LOG(expect_log2);
    rt_thread_mdelay(100);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup_now();
    CHECK_EXPECT_LOG(MAX_TIMEOUT_MS);



    return RT_EOK;
}

/*
Send app intents frequently cause app_scheduler crash.
http://10.21.20.167/issues/4476
*/
static rt_err_t app_test_017(void)
{
    int32_t loop_cnt = 10;
    int32_t delay_ms = 200;

    DISABLE_EXPECT_LOG();

    gui_app_run("2 pages=1");
    while (loop_cnt-- > 0)
    {
        rt_thread_mdelay(delay_ms);
        gui_app_run("1 pages=1");
        rt_thread_mdelay(delay_ms);
        gui_app_run("1 pages=2");
        rt_thread_mdelay(delay_ms);
        gui_app_run("1 pages=3");
    }

    rt_thread_mdelay(500);//Make sure app_schedule finish(unlocked)
    gui_app_cleanup();
    rt_thread_mdelay(500);//Make sure app_schedule finish(unlocked)




    return RT_EOK;
}



#ifdef FINSH_USING_MSH
#include <finsh.h>
typedef rt_err_t (*TesecaseFunc)(void);

const TesecaseFunc all_testcases[] =
{
    app_test_000,
    app_test_001,
    app_test_002,
    app_test_003,
    app_test_004,
    app_test_005,
    app_test_006,
    app_test_007,
    app_test_008,
    app_test_009,
    app_test_010,
    app_test_011,
    app_test_012,
    app_test_013,
    app_test_014,
    app_test_015,
    app_test_016,
    app_test_017,
};

#define testcase_length (sizeof(all_testcases)/sizeof(all_testcases[0]))

extern void gui_app_fwk_test_start(void);
extern void gui_app_fwk_test_stop(void);
static rt_err_t gui_app_test(int argc, char **argv)
{
    gui_app_fwk_test_start();

    gui_app_cleanup();
    rt_thread_mdelay(1000);

    app_schedule_max_running_apps(max_running_apps_default_v);
    app_schedule_destory_suspend_apps(destory_suspend_app_defalut_v);
    app_schedule_set_idle_hook(idle_hook);
    rt_sem_init(&test_sema, "gui_test", 0, RT_IPC_FLAG_FIFO);
    for (uint16_t i = 0; i < max_test_apps; i++)
        for (uint16_t j = 0; j < max_test_subpages; j++)
            pages_state[i][j] = page_st_stoped;

    rt_kprintf("-----------START----------\r\n");

    if (argc > 1)
    {
        for (uint32_t i = 1; i < argc; i++)
        {
            uint32_t case_id = strtol(argv[i], 0, 10);
            rt_kprintf("-----------CASE %d----------\r\n", case_id);
            if (case_id < testcase_length)
            {
                RT_ASSERT(RT_EOK == all_testcases[case_id]());
                RT_ASSERT(all_subpages_stopped());
            }
            else
            {
                rt_kprintf("Invalid case_id \r\n");
            }
        }
    }
    else
    {
        for (uint32_t case_id = 0; case_id < testcase_length; case_id++)
        {
            rt_kprintf("-----------CASE %d----------\r\n", case_id);
            RT_ASSERT(RT_EOK == all_testcases[case_id]());
            RT_ASSERT(all_subpages_stopped());
        }
    }


    rt_kprintf("-----------PASSED----------\r\n");
    app_schedule_set_idle_hook(NULL);
    rt_sem_detach(&test_sema);

    gui_app_fwk_test_stop();
    return 0;
}

MSH_CMD_EXPORT(gui_app_test, list all runing app);



#endif





#endif /* _MSC_VER */
