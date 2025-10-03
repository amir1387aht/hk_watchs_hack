# !/bin/bash 
rm rtconfig.h
rm lvgl
rm lvsf
rm lvsf_conf_internal.h
ln -s ../../../../example/watch_demo/project/ec-lb555/rtconfig.h rtconfig.h
ln -s ../../../lvgl_v8 lvgl
ln -s ../../../../middleware/lvgl/lvsf lvsf
ln -s ../../../../middleware/lvgl/lvsf_conf_internal.h lvsf_conf_internal.h