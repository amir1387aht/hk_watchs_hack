#include "lv_compass.h"
#include "math.h"
#include "bf0_hal.h"
#include "lvgl.h"
#include <stdio.h>

LV_IMG_DECLARE(compass_dial_diagram);//Dial image resources
LV_IMG_DECLARE(compass_pointer_diagram);//Compass pointer image resources
/**
 * Demonstrate how scrolling appears automatically
 */

typedef struct
{
    lv_obj_t *compass_dial;
    lv_obj_t *compass_pointer;

    lv_img_dsc_t *cp_point;

} Compass;

typedef struct
{
    float current_angle;
    float target_angle;
    bool anim_running;
    lv_anim_t anim;
} CompassAnimation;

static CompassAnimation compass_anim;
Compass *r_dial = NULL;


static lv_obj_t *angle_label;
static lv_obj_t *char_label;

//According to the size of the incoming target, set the scaling of the specified image object
static void lv_obj_img_png_set_zoom(lv_obj_t *obj_img, const void *src, uint32_t obj_width, uint32_t obj_height)
{
    uint32_t img_width = 0, img_height = 0, zoom_factor = 0;
    lv_img_header_t header;
    if (lv_img_decoder_get_info(src, &header) != LV_RES_OK)
    {
        rt_kprintf("lv_img_decoder_get_info error\n");
        return;
    }

    img_width = header.w;
    img_height = header.h;

    rt_kprintf("img_widtg:%u, img_height%u, obj_width:%u, obj_height%u\n", img_width, img_height, obj_width, obj_height);
    if (img_width != 0 && img_height != 0)
    {
        uint32_t y_a = obj_height;
        uint32_t x_b = obj_width;

        if (x_b >= y_a)
        {
            uint32_t x = obj_height * 256;
            zoom_factor = x / img_height;
            lv_img_set_zoom(obj_img, zoom_factor);
        }
        else
        {
            uint32_t x = obj_width * 256;
            zoom_factor = x / img_width;
            lv_img_set_zoom(obj_img, zoom_factor);
        }
    }
}

void update_angle_text(int32_t angle)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%0.1f°", angle / 10.0f);
    lv_label_set_text(angle_label, buf);
}
static const char *DIRECTIONS[] =
{
    "North", "Northwest", "West", "Southwest",
    "South", "Southeast", "East",  "Northeast"
};
void set_direction_text(int index)
{
    lv_label_set_text(char_label, DIRECTIONS[index]);
    lv_obj_invalidate(char_label);
}
// 全局变量：记录当前角度和旋转方向
static float last_angle_360 = 0.0f;  // 上一次归一化后的角度（0~360°）
static float total_rotation = 0.0f;  // 累计旋转角度（处理跨圈）
float target_angle = 0.0f;
float current_angle_360 = 0.0f;


void pointer_rotate_cb(float angle)
{
    //Set the rotation Angle
    lv_img_set_angle(r_dial->compass_pointer, angle);

    current_angle_360 = fmodf((angle + 3600.0), 3600.0f);

    update_angle_text(current_angle_360);
    int index;

    // Corrected condition judgment (0.1 degree unit)
    if (current_angle_360 >= 0 && current_angle_360 < 450)
    {
        index = 0;  // North
    }
    else if (current_angle_360 >= 450 && current_angle_360 < 900)
    {
        index = 1;  // Northwest
    }
    else if (current_angle_360 >= 900 && current_angle_360 < 1350)
    {
        index = 2;  // west
    }
    else if (current_angle_360 >= 1350 && current_angle_360 < 1800)
    {
        index = 3;  // Southwest
    }
    else if (current_angle_360 >= 1800 && current_angle_360 < 2250)
    {
        index = 4;  // South
    }
    else if (current_angle_360 >= 2250 && current_angle_360 < 2700)
    {
        index = 5;  // Southeast
    }
    else if (current_angle_360 >= 2700 && current_angle_360 < 3150)
    {
        index = 6;  // East
    }
    else
    {
        index = 7;  // Northesst
    }

    // Update display
    set_direction_text(index);

}

void handle_img(void)
{
    //Allocate descriptor memory
    lv_img_dsc_t *img_dos = (lv_img_dsc_t *) rt_malloc(sizeof(compass_pointer_diagram));
    if (img_dos == NULL)
    {
        rt_kprintf("rt_malloc fail\n");
    }
    //Allocate image data memory
    uint8_t *img_data_size = (uint8_t *)rt_malloc(compass_pointer_diagram.data_size);
    if (img_data_size == NULL)
    {
        rt_kprintf("malloc fail\n");
    }
    memcpy(img_data_size, compass_pointer_diagram.data, compass_pointer_diagram.data_size);

    *img_dos = compass_pointer_diagram;
    img_dos->data = img_data_size;

    r_dial->cp_point = img_dos;
    if (r_dial->compass_pointer != NULL)
    {
        /* Set the image source */
        lv_img_set_src(r_dial->compass_pointer, r_dial->cp_point);
    }

}
extern void my_timer(lv_timer_t *timer);
void example_compass(void)
{
    r_dial = (Compass *)rt_malloc(sizeof(Compass));
    memset(r_dial, 0, sizeof(Compass));

    //Create background
    r_dial->compass_dial = lv_img_create(lv_scr_act());
    //lv_obj_set_size(compass_dial, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_align(r_dial->compass_dial, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_src(r_dial->compass_dial, &compass_dial_diagram);
    unsigned int obj_width = 390, obj_height = 450;
    lv_obj_img_png_set_zoom(r_dial->compass_dial, &compass_dial_diagram, obj_width, obj_height);

    //Create pointer
    r_dial->compass_pointer = lv_img_create(r_dial->compass_dial);
    lv_img_set_src(r_dial->compass_pointer, &compass_pointer_diagram);
    /* Center align the pointer */
    lv_obj_align(r_dial->compass_pointer, LV_ALIGN_CENTER, 0, 0);
    /* Set the pointer rotation center */
    lv_img_set_pivot(r_dial->compass_pointer, compass_pointer_diagram.header.w / 2, compass_pointer_diagram.header.h / 2);

    angle_label = lv_label_create(r_dial->compass_dial);
    lv_obj_set_style_text_font(angle_label, &lv_font_montserrat_24, 0);
    lv_obj_align(angle_label, LV_ALIGN_CENTER, 0, -140);
    update_angle_text(0);

    char_label = lv_label_create(r_dial->compass_dial);
    lv_obj_set_style_text_font(char_label, &lv_font_montserrat_24, 0);
    lv_obj_align(char_label, LV_ALIGN_CENTER, 0, 140);

    handle_img();

    //The creation of the timer
    lv_timer_t *timer = lv_timer_create(my_timer, 16, NULL);
    lv_timer_set_repeat_count(timer, -1);

}
