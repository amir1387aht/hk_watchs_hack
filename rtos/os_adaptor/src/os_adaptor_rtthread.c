/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtthread.h"
#include "os_adaptor.h"
#include "os_adaptor_rtthread.h"
#include "string.h"

os_semaphore_t os_sem_create(char *name, uint32_t count)
{
    return (os_semaphore_t)rt_sem_create(name, count, RT_IPC_FLAG_FIFO);
}

os_status_t os_sem_delete(os_semaphore_t sem)
{
    return (os_status_t)rt_sem_delete((rt_sem_t)sem);
}

os_status_t os_sem_take(os_semaphore_t sema, int32_t timeout)
{
    return (os_status_t)rt_sem_take((rt_sem_t)sema, timeout);
}
os_status_t os_sem_release(os_semaphore_t      sem)
{
    return (os_status_t)rt_sem_release((rt_sem_t)sem);
}

os_mutex_t os_mutex_create(char *name)
{
    return (os_mutex_t)rt_mutex_create(name, RT_IPC_FLAG_FIFO);
}

os_status_t os_mutex_take(os_mutex_t mutex, int32_t timeout)
{
    return (os_status_t)rt_mutex_take((rt_mutex_t)mutex, timeout);
}

os_status_t os_mutex_release(os_mutex_t mutex)
{
    return (os_status_t)rt_mutex_release((rt_mutex_t)mutex);

}

os_status_t os_mutex_delete(os_mutex_t mutex)
{
    return (os_status_t)rt_mutex_delete((rt_mutex_t)mutex);

}

os_thread_t os_thread_create(char     *name, os_thread_func_t entry, void *parameter, void *stack_memory,
                             uint32_t stack_size, os_priority_t priority, uint32_t tick)

{
    os_handle_t tid = RT_NULL;
    do
    {
        tid = rt_malloc(sizeof(os_handle));
        if (tid == RT_NULL)
            break;
        if (stack_memory != NULL)
        {
            tid->handle = rt_malloc(sizeof(struct rt_thread));
            RT_ASSERT(tid->handle);
            memset(tid->handle, 0, sizeof(struct rt_thread));
            tid->flag = RT_OS_ADAPTOR_STATIC;
            if (tid->handle != RT_NULL)
            {
                rt_thread_init((struct rt_thread *)(tid->handle), name, entry, parameter, stack_memory, stack_size, priority, tick);
            }
        }
        else
        {
            tid->handle = (void *)rt_thread_create(name, entry, parameter, stack_size, priority, tick);
            tid->flag = RT_OS_ADAPTOR_DYNAMIC;
        }
        if (tid->handle != RT_NULL)
            rt_thread_startup((struct rt_thread *)(tid->handle));
        else
        {
            rt_free(tid);
            tid = NULL;
        }
    }
    while (0);

    return (os_thread_t)tid;

}

__ROM_USED rt_err_t os_thread_delete(os_thread_t thread)
{
    os_handle_t tid = (os_handle_t)thread;

    if (tid == RT_NULL) return RT_EINVAL;

    if (RT_OS_ADAPTOR_STATIC == tid->flag)
    {
        if (tid->handle)
        {
            rt_thread_t t = (struct rt_thread *)(tid->handle);
            //Make sure thread was detached
            RT_ASSERT(NULL == rt_thread_find(&t->name[0]));

            rt_free(tid->handle);
            tid->handle = NULL;
        }
    }
    else
    {
        rt_thread_delete(tid->handle);
    }

    rt_free(tid);

    return RT_EOK;
}

__ROM_USED rt_err_t rt_timer_start_int(rt_timer_t timer_id, uint32_t duration)
{
    rt_tick_t ticks;

    ticks = rt_tick_from_millisecond(duration);
    rt_timer_control(timer_id, RT_TIMER_CTRL_SET_TIME, &ticks);
    return rt_timer_start(timer_id);
}

#ifdef RT_USING_MESSAGEQUEUE
__ROM_USED os_message_queue_t os_message_queue_create_int(const char *name, uint32_t max_count, uint32_t msg_size, void *mem_pool, uint32_t pool_size)
{
    os_handle_t queue = RT_NULL;
    do
    {
        queue = rt_malloc(sizeof(os_handle));
        if (queue == RT_NULL)
            break;
        if (mem_pool != RT_NULL)
        {
            queue->handle = rt_malloc(sizeof(struct rt_messagequeue));
            queue->flag = RT_OS_ADAPTOR_STATIC;
            if (queue->handle != RT_NULL)
            {
                rt_mq_init((struct rt_messagequeue *)(queue->handle), name, mem_pool, msg_size, pool_size, RT_IPC_FLAG_FIFO);
            }
        }
        else
        {
            queue->handle = (void *)rt_mq_create(name, msg_size, max_count, RT_IPC_FLAG_FIFO);
            queue->flag = RT_OS_ADAPTOR_DYNAMIC;
        }

        if (queue->handle == RT_NULL)
        {
            rt_free(queue);
            queue = NULL;
        }

    }
    while (0);
    return (os_message_queue_t)queue;
}

__ROM_USED rt_err_t os_message_delele_int(os_message_queue_t queue)
{
    rt_err_t err = RT_ERROR;
    os_handle_t q_cb = (os_handle_t)queue;
    if (q_cb->flag == RT_OS_ADAPTOR_STATIC)
        err = rt_mq_detach((rt_mq_t)(q_cb->handle));
    else if (q_cb->flag == RT_OS_ADAPTOR_DYNAMIC)
        err = rt_mq_delete((rt_mq_t)(q_cb->handle));
    rt_free(queue);
    return err;
}
#endif

#ifdef RT_USING_MAILBOX
__ROM_USED os_mailbox_t os_mailbox_create_int(const char *name, uint32_t size, void *mem_pool)
{
    os_handle_t mailbox;

    do
    {
        mailbox = rt_malloc(sizeof(os_handle));
        if (mailbox == RT_NULL)
            break;
        if (mem_pool != NULL)
        {
            mailbox->handle = rt_malloc(sizeof(struct rt_mailbox));
            mailbox->flag = RT_OS_ADAPTOR_STATIC;
            if (mailbox)
                rt_mb_init((rt_mailbox_t)mailbox->handle, name, mem_pool, size, RT_IPC_FLAG_FIFO);
        }
        else
        {
            mailbox->handle = (void *)rt_mb_create(name, size, RT_IPC_FLAG_FIFO);
            mailbox->flag = RT_OS_ADAPTOR_DYNAMIC;
        }

        if (mailbox->handle == RT_NULL)
        {
            rt_free(mailbox);
            mailbox = NULL;
        }

    }
    while (0);

    return (os_mailbox_t)mailbox;
}

__ROM_USED rt_err_t os_mailbox_delete_int(os_mailbox_t mailbox)
{
    rt_err_t err = RT_ERROR;
    os_handle_t q_cb = (os_handle_t)mailbox;
    if (q_cb->flag == RT_OS_ADAPTOR_STATIC)
        err = rt_mb_detach((rt_mailbox_t)(q_cb->handle));
    else if (q_cb->flag == RT_OS_ADAPTOR_DYNAMIC)
        err = rt_mb_delete((rt_mailbox_t)(q_cb->handle));
    rt_free(mailbox);
    return err;
}
#endif

/**@brief Create a system timer.
 * @param[in][out] timer_id Timer handle as reference for other timer functions and NULL if created failed.
 * @param[in] func Timer callback functioin.
 * @param[in] parameter Passed to the timer function as callback argument.
 * @param[in] flags Timer options, such as one shot or periodic.
 */
void os_timer_create(ot_timer_id_t timer_id, os_timer_func_t func, void *parameter, uint8_t flag)
{
    rt_timer_init((rt_timer_t)timer_id, STRINGIFY(timer_id), func, parameter, 0, flag);
}

/**@brief Start a system timer.
 * @param[in] timer_id Timer handle as created by \ref os_timer_create.
 * @param[in] duration Timeout value in milliseconds.
 * @retval The status of timer start.
 */
os_status_t os_timer_start(ot_timer_id_t timer_id, uint32_t duration)
{
    return rt_timer_start_int((rt_timer_t)timer_id, duration);
}

/**@brief Stop a system timer.
 * @param[in] timer_id Timer handle as created by \ref os_timer_create.
 * @retval The status of timer stop.
 */
os_status_t os_timer_stop(ot_timer_id_t timer_id)
{
    return rt_timer_stop((rt_timer_t)timer_id);
}

/**@brief Delete a system timer.
 * @param[in] timer_id Timer handle as created by \ref os_timer_create.
 * @retval The status of timer delete.
 */
os_status_t os_timer_delete(ot_timer_id_t        timer_id)
{
    return rt_timer_detach((rt_timer_t)timer_id);
}

