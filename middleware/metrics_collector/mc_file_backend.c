/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "board.h"
#include <string.h>
#include "metrics_collector.h"
#include "file_logger.h"

#include "dfs_posix.h"
#include "log.h"

void *mc_backend_init(const char *name, uint32_t max_size)
{
    return file_logger_init(name, max_size);
}

mc_err_t mc_backend_write(void *db, void *data, uint32_t data_len)
{
    return (mc_err_t)file_logger_write(db, data, data_len);
}

mc_err_t mc_backend_iter(void *db, mc_backend_iter_cb_t cb, void *arg)
{
    return (mc_err_t)file_logger_iter(db, (fl_iter_cb_t)cb, arg);
}

mc_err_t mc_backend_clear(void *db)
{
    return (mc_err_t)file_logger_clear(db);
}

mc_err_t mc_backend_flush(void *db)
{

    return (mc_err_t)file_logger_flush(db);
}

mc_err_t mc_backend_close(void *db)
{
    return (mc_err_t)file_logger_close(db);
}

