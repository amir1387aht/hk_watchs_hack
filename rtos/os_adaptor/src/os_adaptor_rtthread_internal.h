/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __OS_ADAPTOR_RTTHREAD_INTERNAL_H
#define __OS_ADAPTOR_RTTHREAD_INTERNAL_H

#define RT_OS_ADAPTOR_STATIC 0
#define RT_OS_ADAPTOR_DYNAMIC 1

typedef struct
{
    void *handle;
    uint8_t flag;
} os_handle;

typedef os_handle *os_handle_t;

extern os_thread_t os_thread_create_int(const char *name, os_thread_func_t entry, void *parameter,
                                        void *stack_memory, uint32_t stack_size, os_priority_t priority,
                                        uint32_t tick);
extern rt_err_t os_thread_delete(os_thread_t thread);
extern os_message_queue_t os_message_queue_create_int(const char *name, uint32_t max_count,
        uint32_t msg_size, void *mem_pool, uint32_t pool_size);
extern rt_err_t os_message_delele_int(os_message_queue_t queue);
extern os_mailbox_t os_mailbox_create_int(const char *name, uint32_t size, void *mem_pool);
extern rt_err_t os_mailbox_delete_int(os_mailbox_t mailbox);

rt_err_t rt_timer_start_int(rt_timer_t timer_id, uint32_t duration);

#endif // __OS_ADAPTOR_RTTHREAD_INTERNAL_H

