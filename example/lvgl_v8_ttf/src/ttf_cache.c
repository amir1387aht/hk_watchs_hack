#include "rtthread.h"
#include "schrift.h"

//SFT_HASH_TABLE_NUMBER must be 2^n
#define SFT_HASH_TABLE_NUMBER       256     //big enough to reduce hash confict, one launguage sways has continue unicode
#define MAX_UNICODE_CACHED_NUMBER   1024    //how many unicode char to cache

typedef struct
{
    uint32_t unicode_letter; //max unicode is 0x10FFFF
    uint16_t font_size;
    uint16_t padding;
} sft_hash_key_t;

typedef struct sft_lru_link
{
    struct sft_lru_link *prev;
    struct sft_lru_link *next;
} sft_lru_link_t;

typedef struct sft_hash_node_tag
{
    sft_lru_link_t              lru_link;   //LRU double link
    sft_hash_key_t              key;
    uint8_t                     width, height;
    int8_t                      y_off;
    struct sft_hash_node_tag    *next;      //hash conflict list,has same hash map,  use single link to save memory
    //uint32_t                    value[0]; //MSVC not support 0 length array, value is attached after sft_hash_node_t
} sft_hash_node_t;

typedef struct
{
    int                 ok;  //stress test use
    void                *stresstest_tube;
    SFT_Font            sft_font;
    int                 cached_number;
    int                 cached_number_limit;
    sft_lru_link_t      lru_root;
    sft_hash_node_t    *hash_table[SFT_HASH_TABLE_NUMBER]; //only save first element pointer in hash_table to save memory
} sft_cache_t;

#define  IS_KEY_MATCH(key1, key2)  ((key1.unicode_letter == key2.unicode_letter) && (key1.font_size == key2.font_size))
#define  SFT_HASH_MAP(key) (key.unicode_letter & (SFT_HASH_TABLE_NUMBER - 1))

static sft_cache_t *g_cache_p;

/*
....hashmap is index of hash array.to save memory, hash array only save element pointer.
    if some elements have same hashmap, first element save
    it's pointer in array[hashmap], other save in the list apend at array[hashmap]->next
*/
static void *sft_cache_get(sft_cache_t *cache, sft_hash_key_t *p_key)
{
    sft_hash_key_t k = *p_key;
    sft_hash_node_t *node = cache->hash_table[SFT_HASH_MAP(k)];

    //if has first in hashmap array
    if (node == NULL)
    {
        return NULL;
    }
    //check which match in first or it's conflict list
    if (IS_KEY_MATCH(node->key, k))
    {
        return node; //return attached value after sft_hash_node_t
    }

    while (node->next)
    {
        if (IS_KEY_MATCH(node->key, k))
        {
            return node;
        }
        node = node->next;
    }

    return NULL;
}

static void sft_cache_delete_one(sft_cache_t *cache)
{
    if (cache->lru_root.prev != &cache->lru_root)
    {
        sft_hash_node_t *first_node;
        sft_hash_node_t *del_node;
        sft_hash_node_t *cur_node;
        RT_ASSERT(cache->cached_number > 0);
        //delete from LRU
        del_node = rt_container_of(cache->lru_root.prev, sft_hash_node_t, lru_link);
        cache->lru_root.prev = cache->lru_root.prev->prev;
        cache->lru_root.prev->next = &cache->lru_root;
        //delete from cache array or conflict list
        int hash_index = SFT_HASH_MAP(del_node->key);
        first_node = cache->hash_table[hash_index];

        RT_ASSERT(first_node);
        cur_node = first_node;
        if (cur_node == del_node)
        {
            cache->hash_table[hash_index] = cur_node->next;
        }
        else
        {
            while (1)
            {
                cur_node = first_node->next;
                if (cur_node == del_node)
                {
                    first_node->next = cur_node->next;
                    break;
                }
                first_node = first_node->next;
                RT_ASSERT(first_node);
            }
        }
        RT_ASSERT(IS_KEY_MATCH(del_node->key, cur_node->key));
        cache->cached_number--;
        rt_free(del_node);
    }
    else
    {
        RT_ASSERT(cache->cached_number == 0);
    }
}

static void sft_cache_delete_old(sft_cache_t *cache, int num)
{
    int j = num <= cache->cached_number ? num :  cache->cached_number;
    for (int i = 0; i < j; i++)
    {
        sft_cache_delete_one(cache);
    }
}

static void sft_cache_delete_all(sft_cache_t *cache)
{
    for (int i = 0; i < SFT_HASH_TABLE_NUMBER; i++)
    {
        sft_hash_node_t *first_node = cache->hash_table[i];

        if (first_node)
        {
            sft_hash_node_t *del = first_node;
            first_node = first_node->next;
            rt_free(del);
            cache->cached_number--;
            while (first_node)
            {
                del = first_node;
                first_node = first_node->next;
                rt_free(del);
                cache->cached_number--;
            }
        }
    }
    memset(cache->hash_table, 0, sizeof(cache->hash_table));
    cache->lru_root.next = &cache->lru_root;
    cache->lru_root.prev =  &cache->lru_root;

    RT_ASSERT(cache->cached_number == 0);
}

static sft_hash_node_t *sft_cache_alloc(sft_cache_t *cache, sft_hash_key_t *p_key, uint32_t value_size)
{
    value_size = sizeof(sft_hash_node_t) + value_size;

    sft_hash_node_t *new_node = (sft_hash_node_t *)rt_malloc(value_size);
    if (new_node)
    {
        goto got_it;
    }
    sft_cache_delete_old(cache, cache->cached_number >> 4);
    new_node = (sft_hash_node_t *)rt_malloc(value_size);
    if (new_node)
    {
        goto got_it;
    }
    sft_cache_delete_old(cache, cache->cached_number >> 3);
    new_node = (sft_hash_node_t *)rt_malloc(value_size);
    if (new_node)
    {
        goto got_it;
    }
    sft_cache_delete_old(cache, cache->cached_number >> 2);
    new_node = (sft_hash_node_t *)rt_malloc(value_size);
    if (new_node)
    {
        goto got_it;
    }
    sft_cache_delete_old(cache, cache->cached_number >> 1);
    new_node = (sft_hash_node_t *)rt_malloc(value_size);
    if (new_node)
    {
        goto got_it;
    }
    sft_cache_delete_all(cache);
    new_node = (sft_hash_node_t *)rt_malloc(value_size);
    RT_ASSERT(new_node);

got_it:
    memset(new_node, 0, value_size);
    new_node->key = *p_key;
    return new_node;
}

static void sft_cache_free(sft_hash_node_t *p)
{
    rt_free((void *)p);
}

static void sft_cache_set(sft_cache_t *cache, sft_hash_node_t *new_node)
{
    //add to LRU list head
    new_node->lru_link.prev = &cache->lru_root;
    new_node->lru_link.next = cache->lru_root.next;
    cache->lru_root.next->prev = &new_node->lru_link;
    cache->lru_root.next = &new_node->lru_link;
    int hash_index = SFT_HASH_MAP(new_node->key);
    sft_hash_node_t *first = cache->hash_table[hash_index];
    if (first == NULL)
    {
        //first element for this key
        cache->hash_table[hash_index] = new_node;
    }
    else
    {
        // conflict hash map, insert to list head
        new_node->next = first->next;
        first->next = new_node;
    }
    cache->cached_number++;
    cache->ok = 1;
}


static sft_cache_t *sft_cache_init(int limit)
{

    sft_cache_t *p = (sft_cache_t *)rt_malloc(sizeof(*p));
    RT_ASSERT(p);
    memset(p, 0, sizeof(*p));
    p->cached_number_limit = limit;
    p->lru_root.next = &p->lru_root;
    p->lru_root.prev =  &p->lru_root;
    return p;
}

static void sft_cache_deinit(sft_cache_t *p)
{
    if (p)
    {
        sft_cache_delete_all(p);
        rt_free(p);
    }
}

/*+ 1: to be sure no fractional row*/
#define IMG_BUF_SIZE_ALPHA_1BIT(w, h) ((((w / 8) + 1) * h))
#define IMG_BUF_SIZE_ALPHA_2BIT(w, h) ((((w / 4) + 1) * h))
#define IMG_BUF_SIZE_ALPHA_4BIT(w, h) ((((w / 2) + 1) * h))
#define IMG_BUF_SIZE_ALPHA_8BIT(w, h) ((w * h))

/*4 * X: for palette*/
#define IMG_BUF_SIZE_INDEXED_1BIT(w, h) (IMG_BUF_SIZE_ALPHA_1BIT(w, h) + 4 * 2)
#define IMG_BUF_SIZE_INDEXED_2BIT(w, h) (IMG_BUF_SIZE_ALPHA_2BIT(w, h) + 4 * 4)
#define IMG_BUF_SIZE_INDEXED_4BIT(w, h) (IMG_BUF_SIZE_ALPHA_4BIT(w, h) + 4 * 16)
#define IMG_BUF_SIZE_INDEXED_8BIT(w, h) (IMG_BUF_SIZE_ALPHA_8BIT(w, h) + 4 * 256)

#define IMG_THRESH_HOLD 80
void convert_to_1bit(uint8_t *pixel_8bit, uint8_t *pixel_1bit, int width, int height)
{
    int x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            /*Get the current pixel.
             *dsc->header.w + 7 means rounding up to 8 because the lines are byte aligned
             *so the possible real width are 8 ,16, 24 ...*/
            if (pixel_8bit[y * width + x] > IMG_THRESH_HOLD)
            {
                uint8_t bit = x & 0x7;
                int     x2 = x >> 3;
                uint32_t px = ((width + 7) >> 3) * y + x2;
                pixel_1bit[px] = pixel_1bit[px] | (1 << (7 - bit));
            }
        }
    }
}

uint8_t *sft_get_glyph(SFT *sft, uint32_t unicode_letter, int font_size, int *x_size, int *y_size, int *y_offset)
{
    uint8_t *r = NULL;


    sft_hash_key_t key;
    key.unicode_letter = unicode_letter;
    key.font_size = font_size;
    sft_hash_node_t *node = sft_cache_get(g_cache_p, &key);

    if (node)
    {
        r = (uint8_t *)(&node[1]);
        *x_size = node->width;
        *y_size = node->height;
        *y_offset = node->y_off;
    }
    else
    {
        unsigned long   gid;
        SFT_GMetrics    mtx;
        SFT_Image       image;

        if (sft_lookup(sft, unicode_letter, (SFT_Glyph *)&gid) < 0)
        {
            rt_kprintf("sft: lookup error\n");
            goto end;
        }

        sft->xScale = font_size;
        sft->yScale = font_size;
        sft->flags = SFT_DOWNWARD_Y;
        if (sft_gmetrics(sft, gid, &mtx) < 0)
        {
            rt_kprintf("sft: gmetrics error\n");
            goto end;
        }
        image.width = mtx.minWidth;
        image.height = mtx.minHeight;
        if (image.width && image.height)
        {
#ifdef USE_8BIT_PIXEL
            int size = IMG_BUF_SIZE_ALPHA_8BIT(image.width, image.height);
            node = sft_cache_alloc(g_cache_p, &key, size);
            RT_ASSERT(node);
            image.pixels = (uint8_t *)&node[1];
            if (sft_render(sft, gid, image) == 0)
                r = image.pixels;
            else
            {
                sft_cache_free(node);
                goto end;
            }
#else
            int size = IMG_BUF_SIZE_INDEXED_1BIT(image.width, image.height);
            node = sft_cache_alloc(g_cache_p, &key, size);
            image.pixels = rt_malloc(image.width * image.height);
            RT_ASSERT(image.pixels && node);
            if (sft_render(sft, gid, image) == 0)
            {
                uint8_t *pixels_1bit;
                pixels_1bit = (uint8_t *)&node[1];
                pixels_1bit += 8;               // Reserved for indexed color
                convert_to_1bit(image.pixels, pixels_1bit, image.width, image.height);
                rt_free(image.pixels);
                r = (uint8_t *)&node[1];
            }
            else
            {
                sft_cache_free(node);
                rt_free(image.pixels);
                goto end;
            }
#endif
            sft_cache_set(g_cache_p, node);
            node->width = image.width;
            node->height = image.height;
            node->y_off = mtx.yOffset;
            *x_size = image.width;
            *y_size = image.height;
            *y_offset = mtx.yOffset;
        }
    }
end:
    return r;
}

static SFT sft_font;

SFT *load_ttf(void *font, int size)
{
    if (size == 0)
        sft_font.font = sft_loadfile(font);
    else
    {
        SFT_Font *thefont;
        if (!(thefont = rt_calloc(1, sizeof(*thefont))))
        {
            return NULL;
        }
        sft_font.font = sft_loadmem(thefont, font, size);
    }
    if (sft_font.font && g_cache_p == NULL)
        g_cache_p = sft_cache_init(MAX_UNICODE_CACHED_NUMBER);


    return sft_font.font ? &sft_font : NULL;
}


