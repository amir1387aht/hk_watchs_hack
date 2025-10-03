#include "../../lv_examples.h"
#if LV_USE_IMG && LV_BUILD_EXAMPLES && !defined(_MSC_VER)


void lv_example_ezip_1(void)
{
    LV_IMG_DECLARE(logo_white);
    lv_obj_t *img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &logo_white);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
}

#endif
