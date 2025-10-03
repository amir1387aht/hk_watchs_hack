#include "config.h"

#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"

#include "avformat.h"
#include "internal.h"
#include "os_support.h"
#include "url.h"
#if CONFIG_NAND_PROTOCOL

typedef struct NandContext {
    const AVClass *class;
    int64_t img_start;
    int64_t img_size;
    int64_t offset;
} NandContext;

static const AVClass nand_class = {
    .class_name = "nand",
    .item_name  = av_default_item_name,
    .option     = NULL,
    .version    = LIBAVUTIL_VERSION_INT,
};

RT_WEAK uint32_t lv_img_decode_flash_read(uint32_t addr, uint8_t *buf, int size)
{
    RT_ASSERT(0);
    return 0;
}

static int nand_read(URLContext *h, unsigned char *buf, int size)
{
    NandContext *c = h->priv_data;
    int ret;
    if (size < 0)
        return -1;
    if (c->offset + size >= c->img_size)
    {
        size = c->img_size - c->offset;
    }
    if (size == 0)
    {
        return 0;
    }

    lv_img_decode_flash_read((uint32_t)(c->img_start + c->offset), buf, size);
    c->offset += size;
    return size;
}

static int nand_open(URLContext *h, const char *filename, int flags)
{
    NandContext *c = h->priv_data;
    uint32_t address;
    uint32_t len;
    int fd;
    if (strncmp("nand://", filename, 7))
    {
        return -1;
    }
    sscanf(filename, FFMPEG_NAND_URL_FMT, &address, &len);
    c->img_start = address;
    c->img_size = len;
    c->offset = 0;
    return 0;
}
static int nand_close(URLContext *h)
{
     return 0;
}
static int64_t nand_seek(URLContext *h, int64_t pos, int whence)
{
    NandContext *c = h->priv_data;
    int64_t ret;

    if (whence == AVSEEK_SIZE) {
        return c->img_size;
    }
    if (whence == SEEK_SET)
    {
        if (pos < 0)
        {
            pos = 0;
        }
        if (pos > c->img_size)
        {
            pos = c->img_size;
        }
        c->offset = pos;
    }
    else if (whence == SEEK_CUR)
    {
        c->offset += pos;
    }
    else if (whence == SEEK_END)
    {
        c->offset = c->img_size + pos;
    }
    if (c->offset > c->img_size)
    {
        c->offset = c->img_size;
    }
    else if (c->offset < 0)
    {
        c->offset = 0;
    }
    return c->offset;
}


URLProtocol ff_nand_protocol = {
    .name                = "nand",
    .url_open            = nand_open,
    .url_read            = nand_read,
    .url_seek            = nand_seek,
    .url_close           = nand_close,
    .priv_data_size      = sizeof(NandContext),
    .priv_data_class     = &nand_class,
    .default_whitelist   = "nand"
};

#endif
