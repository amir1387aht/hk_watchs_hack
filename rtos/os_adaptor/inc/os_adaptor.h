/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __OS_ADAPTOR_H
#define __OS_ADAPTOR_H

/**
 ****************************************************************************************
 * @addtogroup os_wrapper Kernel adatpor
 * @ingroup kernel
 * @brief Provided wrapped kernel interface to adapot different RTOS.
 * @{
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rtconfig.h"
#include <stdint.h>

#define OS_ADAPTOR_V2
/*
 * DEFINES
 ****************************************************************************************
 */

#define OS_WAIT_FORVER                 /**< Wait forever timeout value. */

// Event flags options
#define OS_EVENT_FLAG_WAIT_ALL         /**< Wait all event flags */
#define OS_EVENT_FLAG_WAIT_ANY         /**< Wait any event flag */
#define OS_EVENT_FLAG_CLEAR            /**< Event clear flag */

// Timer flags options
#define OS_TIMER_FLAG_ONE_SHOT   0      /**< One shot timer */
#define OS_TIMER_FLAG_PERIODIC   1      /**< Periodic timer */
#define OS_TIMER_FLAG_SOFT       2      /**< Soft timer */

// ROM used indicator
#ifndef __ROM_USED
    #define __ROM_USED
#endif

// Thread handle identifies the thread.
typedef void *os_thread_t;

// Thread callback function prototype.
typedef void (*os_thread_func_t)(void *parameter);

// Thread priority.
typedef uint8_t os_priority_t;

// Timer handle identifies the timer.
typedef void *ot_timer_id_t;

// Timer callback function prototype.
typedef void (*os_timer_func_t)(void *parameter);

// Semaphore handle identifies the semaphore.
typedef void *os_semaphore_t;

// Mutex handle identifies the mutex.
typedef void *os_mutex_t;

// Message queue handle identifies the message queue.
typedef void *os_message_queue_t;

// Mailbox handle identifies the mailbox.
typedef void *os_mailbox_t;

// Event handle identifies the event.
typedef void *os_event_t;

// OS status definition
typedef uint32_t os_status_t;

/******************* Thread management ***************/

// Thread handle initalized
#define OS_THREAD_DECLAR(tid) \
    os_thread_t tid

/**@brief Create a thread and add it into active list.
 * @param[in] name Thread name.
 * @param[in] entry Thread callback functioin.
 * @param[in] parameter Passed to the thread function as start argument.
 * @param[in] stack_memory Provided stack memory to the thread or thread created the memory from heap if NULL.
 * @param[in] stack_size Stack size for the thread.
 * @param[in] priority Priority of thread. The lower value refer higher priority.
 * @param[in] tick Thread tick for time slice.
 * @retval Handle of thread.
 */
os_thread_t os_thread_create(char     *name, os_thread_func_t entry, void *parameter, void *stack_memory,
                             uint32_t stack_size, os_priority_t priority, uint32_t tick);

/**@brief Thread enter critical.
*/
void os_enter_critical(void);

/**@brief Thread exit critical.
*/
void os_exit_critical(void);

/**@brief Disable interrupt.
*/
uint32_t os_interrupt_disable(void);

/**@brief Enable interrupt.
*/
void os_interrupt_enable(uint32_t level);

void os_interrupt_enter(void);

void os_interrupt_exit(void);

/**@brief Thread delay for some milliseconds and yieled processor.
 * @param[in] ms Delay in milliseconds.
 * @retval The status of delay.
*/
os_status_t os_delay(uint32_t ms);

/******************* Timer management ***************/

// Timer handle initalized
#define OS_TIMER_DECLAR(timer_id) \
    ot_timer_id_t timer_id

/**@brief Create a system timer.
 * @param[in][out] timer_id Timer handle as reference for other timer functions and NULL if created failed.
 * @param[in] func Timer callback functioin.
 * @param[in] parameter Passed to the timer function as callback argument.
 * @param[in] flags Timer options, such as one shot or periodic.
 */
void os_timer_create(ot_timer_id_t timer_id, os_timer_func_t func, void *parameter, uint8_t flag);

/**@brief Start a system timer.
 * @param[in] timer_id Timer handle as created by \ref os_timer_create.
 * @param[in] duration Timeout value in milliseconds.
 * @retval The status of timer start.
 */
os_status_t os_timer_start(ot_timer_id_t timer_id, uint32_t duration);

/**@brief Stop a system timer.
 * @param[in] timer_id Timer handle as created by \ref os_timer_create.
 * @retval The status of timer stop.
 */
os_status_t os_timer_stop(ot_timer_id_t timer_id);

/**@brief Delete a system timer.
 * @param[in] timer_id Timer handle as created by \ref os_timer_create.
 * @retval The status of timer delete.
 */
os_status_t os_timer_delete(ot_timer_id_t        timer_id);

/******************* Semaphore management ***************/

// Semaphore handle initalized
#define OS_SEM_DECLAR(sema) \
    os_semaphore_t sema

/**@brief Create and initalize a semaphore.
 * @param[in] name Semaphore name.
 * @param[in] count Maximum semphore count.
 * @retval Created semaphore, NULL if failed.
 */
os_semaphore_t os_sem_create(char *name, uint32_t count);

/**@brief Take a semaphore. If no sempahore is available, it will wait till timeout
 * @param[in] smea Semaphore handle as created \ref os_sem_create.
 * @param[in] timeout Maximum time for watied.@OS_WAIT_FORVER presents wait forever.
 * @retval The status of semaphore take.
 */
os_status_t os_sem_take(os_semaphore_t sema, int32_t timeout);

/**@brief Release a semaphore.
 * @param[in] smea Semaphore handle as created \ref os_sem_create.
 * @retval The status of semaphore release.
 */
os_status_t os_sem_release(os_semaphore_t        sem);

/**@brief Delete a semaphore.
 * @param[in] smea Semaphore handle as created \ref os_sem_create.
 * @retval The status of semaphore delete.
 */
os_status_t os_sem_delete(os_semaphore_t sem);

/******************* Mutex management ***************/

// Mutex handle initalized
#define OS_MUTEX_DECLAR(mutex) \
    os_mutex_t mutex

/**@brief Create and initalize a mutex.
 * @param[in] name Mutex name.
 * @retval Created Mutex, NULL if failed.
 */
os_mutex_t os_mutex_create(char *name);

/**@brief Take a mutex. If no mutex is available, it will wait till timeout.
 * @param[in] mutex Mutex handle as created \ref os_mutex_create.
 * @param[in] timeout Maximum time for watied.@OS_WAIT_FORVER presents wait forever.
 * @retval The status of mutex take.
 */
os_status_t os_mutex_take(os_mutex_t mutex, int32_t timeout);

/**@brief Release a mutex.
 * @param[in] mutex Mutex handle as created \ref os_mutex_create.
 * @retval The status of mutex release.
 */
os_status_t os_mutex_release(os_mutex_t mutex);

/**@brief Release a mutex.
 * @param[in] mutex Mutex handle as created \ref os_mutex_create.
 * @retval The status of mutex delete.
 */
os_status_t os_mutex_delete(os_mutex_t mutex);

/******************* Message queue management ***************/

// Message queue handle initalized
#define OS_MESSAGE_QUEUE_DECLAR(queue) \
    os_message_queue_t queue

/**@brief Create and initalize a message queue.
 * @param[in][out] queue Message queue handle as reference for other message functions and NULL if created failed.
 * @param[in] max_count Maximum count of messages in queue, used when mem_pool is NULL.
 * @param[in] msg_size Maximum size of one message.
 * @param[in] mem_pool Provided memory pool to the message queue or message queue created the memory from heap if NULL.
 * @param[in] pool_size Pool size of the provided memory pool.
 */
void os_message_queue_create(os_message_queue_t queue, uint32_t max_count, uint32_t msg_size, void *mem_pool, uint32_t pool_size);

/**@brief Put a message to the message queue. If message queue is full, it will wait till timeout.
 * @param[in] queue Message queue handle as created \ref os_message_queue_create.
 * @param[in] data Pointer to the message address.
 * @param[in] data_size The size of the message.
 * @param[in] timeout Maximum time for watied.@OS_WAIT_FORVER presents wait forever.
 * @retval The status of message put.
 */
os_status_t os_message_put(os_message_queue_t queue, void *data, uint32_t data_size, int32_t timeout);

/**@brief Get a message from the message queue. If message queue is empty, it will wait till timeout.
 * @param[in] queue Message queue handle as created \ref os_message_queue_create.
 * @param[in] data Pointer to a buffer address to receive message
 * @param[in] data_size The size of the buffer.
 * @param[in] timeout Maximum time for watied.@OS_WAIT_FORVER presents wait forever.
 * @retval The status of message get.
 */
os_status_t os_message_get(os_message_queue_t queue, void *data, uint32_t data_size, int32_t timeout);

/**@brief Delete the message queue.
 * @param[in] queue Message queue handle as created \ref os_message_queue_create.
 * @retval The status of message delete.
 */
os_status_t os_message_delete(os_message_queue_t queue);

/******************* Mailbox management ***************/

// Mailbox handle initalized
#define OS_MAILBOX_DECLAR(mailbox) \
    os_mailbox_t mailbox

/**@brief Create and initalize a mailbox.
 * @param[in][out] mailbox Mailbox handle as reference for other mailbox functions and NULL if created failed.
 * @param[in] size Maximum size of the mailbox.
 * @param[in] mem_pool Provided memory pool to the message queue or message queue created the memory from heap if NULL.
 */
void os_mailbox_create(os_mailbox_t mailbox, uint32_t size, void *mem_pool);

/**@brief Put a message to the mailbox. If mailbox is full, it will wait till timeout.
 * @param[in] mailbox Mailbox handle as created \ref os_mailbox_create.
 * @param[in] data Pointer to the message address.
 * @retval The status of mailbox put.
 */
os_status_t os_mailbox_put(os_mailbox_t mailbox, uint32_t data);

/**@brief Get a message from the mailbox. If mailbox is empty, it will wait till timeout.
 * @param[in] mailbox Mailbox handle as created \ref os_mailbox_create.
 * @param[in] data Pointer to a buffer address to get message.
 * @param[in] timeout Maximum time for watied.@OS_WAIT_FORVER presents wait forever.
 * @retval The status of mailbox get.
 */
os_status_t os_mailbox_get(os_mailbox_t mailbox, uint32_t *data, int32_t timeout);

/**@brief Delete the mailbox.
 * @param[in] mailbox Mailbox handle as created \ref os_mailbox_create.
 * @retval The status of mailbox delete.
 */
os_status_t os_mailbox_delete(os_mailbox_t mailbox);

/******************* Mailbox management ***************/

// Event initalized
#define OS_EVENT_DECLAR(event) \
    os_event_t event

/**@brief Create and initalize a event.
 * @param[in][out] event Event handle as reference for other event functions and NULL if created failed.
 */
void os_event_create(os_event_t event);

/**@brief Set flags to the event.
 * @param[in] event Event handle as created \ref os_event_create.
 * @param[in] flags Flags set to event.
 * @retval The status of flags set.
 */
os_status_t os_event_flags_set(os_event_t event, uint32_t flags);

/**@brief Wait flags that set to the event.
 * @param[in] event Event handle as created \ref os_event_create.
 * @param[in] flags Waited flags.
 * @param[in] option Wait flag options, \ref OS_EVENT_FLAG_WAIT_ALL, \ref OS_EVENT_FLAG_WAIT_ANY, \ref  OS_EVENT_FLAG_CLEAR.
 * @param[out] Received set event.
 * @retval The status of flags wait.
 */
os_status_t os_event_flags_wait(os_event_t event, uint32_t flags, uint32_t option, uint32_t timeout, uint32_t *recv_evt);

/**@brief Set flags to the event.
 * @param[in] event Event handle as created \ref os_event_create.
 * @retval The status of event delet.
 */
os_status_t os_event_delete(os_event_t event);

/******************* Mailbox management ***************/
// Assert macro
#define OS_ASSERT(EX)

#if defined(BSP_USING_RTTHREAD)
#include "os_adaptor_rtthread.h"
#elif defined(BSP_USING_FREERTOS)

#endif // BSP_USING_RTTHREAD

#endif // __OS_ADAPTOR_H

/// @} os_wrapper

