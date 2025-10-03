/*
 * COPYRIGHT (C) 2018, Real-Thread Information Technology Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-5-30     Bernard      the first version
 */
#include <rtdevice.h>

#ifdef RT_USING_MTD_DHARA
#include "map.h"
#include "mem_section.h"

#define DHARA_DEFAULT_GC_RATIO (4)

static rt_err_t _mtd_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t _mtd_open(rt_device_t dev, rt_uint16_t oflag)
{
    int ret;
    struct rt_mtd_dhara_device *ndev = (struct rt_mtd_dhara_device *)dev;
    dhara_ctx_t *dhara_ctx;

    dhara_ctx = &(ndev->dhara_ctx);

    // init flash translation layer
    dhara_map_init(&dhara_ctx->map, &dhara_ctx->nand, dhara_ctx->page_buffer, ndev->gc_ratio);
    dhara_error_t err = DHARA_E_NONE;
    ret = dhara_map_resume(&dhara_ctx->map, &err);

    return RT_EOK;
}

static rt_err_t _mtd_close(rt_device_t dev)
{
    return RT_EOK;
}


static rt_size_t _mtd_read(rt_device_t dev,
                           rt_off_t    pos,
                           void       *buffer,
                           rt_size_t   size)
{
    rt_size_t res = 0;
    struct rt_mtd_dhara_device *ndev = (struct rt_mtd_dhara_device *)dev;
    dhara_ctx_t *dhara_ctx;
    rt_uint16_t page_size;
    rt_uint32_t i;
    rt_uint8_t *rd_buf;
    dhara_error_t err;

    dhara_ctx = &ndev->dhara_ctx;
    page_size = ndev->mtd_nand->page_size;
    rd_buf = (rt_uint8_t *)buffer;
    for (i = 0; i < size; i++)
    {
        int ret = dhara_map_read(&dhara_ctx->map, pos, rd_buf, &err);
        if (ret)
        {
            rt_kprintf("dhara read failed: %d, error: %d\n", ret, err);
            return 0;
        }
        rd_buf += page_size;
        pos++;
    }

    return size;
}

static rt_size_t _mtd_write(rt_device_t dev,
                            rt_off_t    pos,
                            const void *buffer,
                            rt_size_t   size)
{
    rt_size_t res = 0;
    struct rt_mtd_dhara_device *ndev = (struct rt_mtd_dhara_device *)dev;
    dhara_ctx_t *dhara_ctx;
    rt_uint16_t page_size;
    rt_uint32_t i;
    rt_uint8_t *wr_buf;
    dhara_error_t err;

    dhara_ctx = &ndev->dhara_ctx;
    page_size = ndev->mtd_nand->page_size;
    wr_buf = (rt_uint8_t *)buffer;
    for (i = 0; i < size; i++)
    {
        int ret = dhara_map_write(&dhara_ctx->map, pos, wr_buf, &err);
        if (ret)
        {
            rt_kprintf("dhara write failed: %d, error: %d\n", ret, err);
            return 0;
        }
        wr_buf += page_size;
        pos++;
    }

    return size;
}

static rt_err_t _mtd_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_mtd_dhara_device *ndev = (struct rt_mtd_dhara_device *)dev;
    dhara_ctx_t *dhara_ctx;
    dhara_error_t err;
    int ret;

    dhara_ctx = &ndev->dhara_ctx;
    if (dev == NULL)
    {
        return RT_EINVAL;
    }
    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry = (struct rt_device_blk_geometry *)args;

        geometry->sector_count = dhara_map_capacity(&dhara_ctx->map);
        geometry->bytes_per_sector = ndev->mtd_nand->page_size;
        geometry->block_size = ndev->mtd_nand->pages_per_block * ndev->mtd_nand->page_size;

        rt_kprintf("sec_count:%d\n", geometry->sector_count);
    }
    else if (cmd == RT_DEVICE_CTRL_BLK_SYNC)
    {
        ret = dhara_map_sync(&dhara_ctx->map, &err);
        if (ret)
        {
            rt_kprintf("dhara sync failed: %d, error: %d\n", ret, err);
            return RT_ERROR;
        }
    }
    else if (cmd == RT_DEVICE_CTRL_BLK_ERASE)
    {

        uint32_t start = *(uint32_t *)args;
        uint32_t end = *((uint32_t *)args + 1);
        while (start <= end)
        {
            ret = dhara_map_trim(&dhara_ctx->map, start, &err);
            if (ret)
            {
                rt_kprintf("dhara trim failed: %d, error: %d\n", ret, err);
                return RT_ERROR;
            }
            start++;
        }
    }
    else
    {
        return RT_EINVAL;
    }
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops mtd_dhara_ops =
{
    _mtd_init,
    _mtd_open,
    _mtd_close,
    _mtd_read,
    _mtd_write,
    _mtd_control
};
#endif

rt_err_t rt_mtd_dhara_register_device(const char *name, uint8_t gc_ratio, rt_mtd_nand_t mtd_nand)
{
    rt_device_t dev;
    dhara_ctx_t *dhara_ctx;
    struct rt_mtd_dhara_device *mtd_dhara;

    RT_ASSERT(mtd_nand);

    mtd_dhara = (struct rt_mtd_dhara_device *)rt_malloc(sizeof(*mtd_dhara));
    RT_ASSERT(mtd_dhara);

    if (0 == gc_ratio)
    {
        gc_ratio = DHARA_DEFAULT_GC_RATIO;
    }
    mtd_dhara->gc_ratio = gc_ratio;
    mtd_dhara->mtd_nand = mtd_nand;

    dev = RT_DEVICE(mtd_dhara);
    RT_ASSERT(dev != RT_NULL);

    /* set device class and generic device interface */
    dev->type        = RT_Device_Class_MTD;
#ifdef RT_USING_DEVICE_OPS
    dev->ops         = &mtd_dhara_ops;
#else
    dev->init        = _mtd_init;
    dev->open        = _mtd_open;
    dev->read        = _mtd_read;
    dev->write       = _mtd_write;
    dev->close       = _mtd_close;
    dev->control     = _mtd_control;
#endif

    dhara_ctx = &(mtd_dhara->dhara_ctx);
    dhara_ctx->nand.num_blocks = mtd_nand->block_total;
    if (2048 == mtd_nand->page_size)
    {
        dhara_ctx->nand.log2_page_size = 11; /* page_size = 2048 */
    }
    else if (4096 == mtd_nand->page_size)
    {
        dhara_ctx->nand.log2_page_size = 12; /* page_size = 4096 */
    }
    else
    {
        RT_ASSERT(0);
    }
    if (mtd_nand->pages_per_block == 64)
    {
        dhara_ctx->nand.log2_ppb = 6;        /* pages_per_block = 64, i.e. block_size = 128KB */
    }
    else if (mtd_nand->pages_per_block == 128)
    {
        dhara_ctx->nand.log2_ppb = 7;
    }
    else
    {
        RT_ASSERT(0);
    }

    dhara_ctx->page_buffer = rt_malloc(mtd_nand->page_size);
    dhara_ctx->nand.meta_buf = rt_malloc(mtd_nand->page_size);
    RT_ASSERT(dhara_ctx->page_buffer);
    RT_ASSERT(dhara_ctx->nand.meta_buf);

    dhara_ctx->nand.user_data = (void *)mtd_nand;

    dev->rx_indicate = RT_NULL;
    dev->tx_complete = RT_NULL;

    /* register to RT-Thread device system */
    return rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}

#endif
