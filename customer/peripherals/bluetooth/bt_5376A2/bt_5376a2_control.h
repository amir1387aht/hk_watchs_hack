/*********************
 *      INCLUDES
 *********************/
#ifndef _BT_5376A2_CONTROL_H
#define _BT_5376A2_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

bt_err_t bt_5376_control(struct rt_bt_device *gps_handle, int cmd, void *args);
void bt_5376_set_cmd_table(at_client_t client);

#ifdef __cplusplus
}
#endif
#endif /* _BT_5376A2_CONTROL_H */
