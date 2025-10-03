/*********************
 *      INCLUDES
 *********************/
#ifndef _BT_FR508X_CONTROL_H
#define _BT_FR508X_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

bt_err_t fr508x_control(struct rt_bt_device *gps_handle, int cmd, void *args);
void fr508x_set_cmd_table(at_client_t client);

#ifdef __cplusplus
}
#endif
#endif /* _BT_FR508X_CONTROL_H */
