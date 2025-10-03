/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LV_EX_DATA_H
#define LV_EX_DATA_H

#define MAX_EX_DATA_NAME_LEN  32

typedef enum
{
    LV_EX_DATA_STRING,
    LV_EX_DATA_INT32,
    LV_EX_DATA_UINT32,
    LV_EX_DATA_FLOAT,
    LV_EX_DATA_TIME,
    LV_EX_DATA_POINTER,
    LV_EX_DATA_LIST,
} lv_ex_data_type_t;

typedef lv_res_t (* lv_ex_data_target_string_setter_t)(lv_obj_t *target, const char *data);
typedef lv_res_t (* lv_ex_data_target_int32_setter_t)(lv_obj_t *target, int32_t data);
typedef lv_res_t (* lv_ex_data_target_uint32_setter_t)(lv_obj_t *target, uint32_t data);
typedef lv_res_t (* lv_ex_data_target_float_setter_t)(lv_obj_t *target, float data);
typedef lv_res_t (* lv_ex_data_target_time_setter_t)(lv_obj_t *target, uint32_t data);
typedef lv_res_t (* lv_ex_data_target_pointer_setter_t)(lv_obj_t *target, void *data);
typedef lv_res_t (* lv_ex_data_target_list_setter_t)(lv_obj_t *target, void *data);

typedef struct
{
    /** binding target object */
    lv_obj_t *target;
    /** binding target object property setter
     *
     *  prototype of setter is:
     *   lv_res_t setter(lv_obj_t *target, arg_type arg);
     */
    void *setter;
    /** property setter argument type */
    lv_ex_data_type_t arg_type;
} lv_ex_binding_t;

typedef union
{
    int32_t i;
    uint32_t ui;
    float f;
    const char *s;
    void *ptr;
} lv_ex_data_value_t;

typedef struct
{
    char name[MAX_EX_DATA_NAME_LEN];
    lv_ex_data_value_t value;
    lv_ex_data_type_t type;
    /* list of lv objects who listen to the data update and consume the data*/
    lv_ll_t listener;
    bool updated;
} lv_ex_data_t;

// create data object
lv_ex_data_t *lv_ex_data_create(const char *name, lv_ex_data_type_t type);
// delete data object
lv_res_t lv_ex_data_delete(lv_ex_data_t *data);


// bind data object
void *lv_ex_bind_data(lv_ex_data_t *data, lv_ex_binding_t *binding);
// variant: ui_elem doesn't need to be lv obj, it just points to one property of the object
//          according to data type, data value is copied to the memory pointed by ui_elem
//          Then any number of properties of one object can be bound
// limitation: how to invalidate the object
//lv_res_t lv_ex_bind_data(void *ui_elem, lv_ex_data_t *data);

lv_res_t lv_ex_unbind_data(lv_ex_data_t *data, void *handle);


// update data object value and notify listeners
lv_res_t lv_ex_data_set_value(lv_ex_data_t *data, void *value);

void lv_ex_process_data(void);

void lv_ex_data_pool_init(void);


#endif /* LV_EX_DATA_H */


