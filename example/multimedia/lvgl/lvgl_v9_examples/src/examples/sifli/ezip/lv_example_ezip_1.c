#include "../../lv_examples.h"
#if LV_USE_IMAGE && LV_BUILD_EXAMPLES && !defined(_MSC_VER)

void lv_example_ezip_1(void)
{
    LV_IMAGE_DECLARE(logo_white);
    lv_obj_t *img1 = lv_image_create(lv_screen_active());
    lv_image_set_src(img1, &logo_white);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
}

#endif
