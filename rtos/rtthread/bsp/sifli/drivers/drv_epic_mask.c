/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*********************
 *      INCLUDES
 *********************/
#include "drv_epic_mask.h"

#ifdef BSP_USING_EPIC
#include "string.h"


/*********************
 *      DEFINES
 *********************/
#define MATH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MATH_MIN3(a, b, c) (MATH_MIN(MATH_MIN(a,b), c))
#define MATH_MIN4(a, b, c, d) (MATH_MIN(MATH_MIN(a,b), MATH_MIN(c,d)))

#define MATH_MAX(a, b) ((a) > (b) ? (a) : (b))
#define MATH_MAX3(a, b, c) (MATH_MAX(MATH_MAX(a,b), c))
#define MATH_MAX4(a, b, c, d) (MATH_MAX(MATH_MAX(a,b), MATH_MAX(c,d)))

#define MATH_CLAMP(min, val, max) (MATH_MAX(min, (MATH_MIN(val, max))))

#define MATH_ABS(x) ((x) > 0 ? (x) : (-(x)))
#define MATH_UDIV255(x) (((x) * 0x8081U) >> 0x17)

#define MASK_OPA_MIX2(a1, a2) (((int32_t)(a1) * (a2)) >> 8)



#define DRAW_CIRCLE_CACHE_SIZE  12
#define CIRCLE_CACHE_LIFE_MAX           1000
#define CIRCLE_CACHE_AGING(life, r)     life = MATH_MIN(life + (r < 16 ? 1 : (r >> 4)), 1000)
#define circle_cache_mutex              //circle_cache_mutex


#define DRV_EPIC_ATTRIBUTE_FAST_MEM



#define mutex_init(mutex)
#define mutex_delete(mutex)
#define mutex_lock(mutex)
#define mutex_unlock(mutex)

#define ASSERT_MALLOC(p) RT_ASSERT(p != NULL);


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static drv_epic_mask_res_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ draw_mask_line(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_line_param_t *param);
static drv_epic_mask_res_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ draw_mask_radius(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_radius_param_t *param);
static drv_epic_mask_res_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ draw_mask_angle(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_angle_param_t *param);
static drv_epic_mask_res_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ draw_mask_fade(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_fade_param_t *param);
static drv_epic_mask_res_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ draw_mask_map(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_map_param_t *param);

static drv_epic_mask_res_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ line_mask_flat(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y,
        int32_t len,
        drv_epic_mask_line_param_t *p);
static drv_epic_mask_res_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ line_mask_steep(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y,
        int32_t len,
        drv_epic_mask_line_param_t *p);

static void circ_init(drv_epic_mask_point_t *c, int32_t *tmp, int32_t radius);
static bool circ_cont(drv_epic_mask_point_t *c);
static void circ_next(drv_epic_mask_point_t *c, int32_t *tmp);
static void circ_calc_aa4(drv_epic_mask_radius_circle_dsc_t *c, int32_t radius);
static drv_epic_mask_opa_t *get_next_line(drv_epic_mask_radius_circle_dsc_t *c, int32_t y, int32_t *len,
        int32_t *x_start);
static inline drv_epic_mask_opa_t /* DRV_EPIC_ATTRIBUTE_FAST_MEM */ mask_mix(drv_epic_mask_opa_t mask_act, drv_epic_mask_opa_t mask_new);

static void point_set(drv_epic_mask_point_t *p, int32_t x, int32_t y);
static int32_t area_get_width(const drv_epic_mask_area_t *area_p);
static int32_t area_get_height(const drv_epic_mask_area_t *area_p);
inline static void area_copy(drv_epic_mask_area_t *dest, const drv_epic_mask_area_t *src);
static inline void *mem_alloc(size_t size);
static void *mem_alloc_zeroed(size_t size);
static void mem_free(void *data);
static inline void memzero(void *dst, size_t len);


/**********************
 *  EXTERNAL FUNCTION
 **********************/

extern int16_t EPIC_TrigoSin(int16_t angle);

/**********************
 *  STATIC VARIABLES
 **********************/
static drv_epic_mask_radius_circle_dsc_t _circle_cache[DRAW_CIRCLE_CACHE_SIZE];

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void drv_epic_mask_init(void)
{
    mutex_init(&circle_cache_mutex);
}

void drv_epic_mask_deinit(void)
{
    mutex_delete(&circle_cache_mutex);
}

drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM drv_epic_mask_apply(void *masks[], drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y,
        int32_t len)
{
    bool changed = false;
    drv_epic_mask_common_dsc_t *dsc;

    uint32_t i;
    for (i = 0; masks[i]; i++)
    {
        dsc = masks[i];
        drv_epic_mask_res_t res = DRAW_MASK_RES_FULL_COVER;
        res = dsc->cb(mask_buf, abs_x, abs_y, len, masks[i]);
        if (res == DRAW_MASK_RES_TRANSP) return DRAW_MASK_RES_TRANSP;
        else if (res == DRAW_MASK_RES_CHANGED) changed = true;
    }

    return changed ? DRAW_MASK_RES_CHANGED : DRAW_MASK_RES_FULL_COVER;
}

void drv_epic_mask_free_param(void *p)
{
    mutex_lock(&circle_cache_mutex);
    drv_epic_mask_common_dsc_t *pdsc = p;
    if (pdsc->type == DRAW_MASK_TYPE_RADIUS)
    {
        drv_epic_mask_radius_param_t *radius_p = (drv_epic_mask_radius_param_t *) p;
        if (radius_p->circle)
        {
            if (radius_p->circle->life < 0) //Release temporary circle
            {
                mem_free(radius_p->circle->buf);
                mem_free(radius_p->circle);
            }
            else
            {
                RT_ASSERT(radius_p->circle->used_cnt > 0);
                radius_p->circle->used_cnt--;
            }
        }
    }

    mutex_unlock(&circle_cache_mutex);
}

void drv_epic_mask_cleanup(void)
{
    uint8_t i;
    for (i = 0; i < DRAW_CIRCLE_CACHE_SIZE; i++)
    {
        if (_circle_cache[i].buf)
        {
            mem_free(_circle_cache[i].buf);
        }
        memzero(&(_circle_cache[i]), sizeof(_circle_cache[i]));
    }
}

void drv_epic_mask_line_points_init(drv_epic_mask_line_param_t *param, int32_t p1x, int32_t p1y,
                                    int32_t p2x,
                                    int32_t p2y, drv_epic_mask_line_side_t side)
{
    memzero(param, sizeof(drv_epic_mask_line_param_t));

    if (p1y == p2y && side == DRAW_MASK_LINE_SIDE_BOTTOM)
    {
        p1y--;
        p2y--;
    }

    if (p1y > p2y)
    {
        int32_t t;
        t = p2x;
        p2x = p1x;
        p1x = t;

        t = p2y;
        p2y = p1y;
        p1y = t;
    }

    point_set(&param->cfg.p1, p1x, p1y);
    point_set(&param->cfg.p2, p2x, p2y);
    param->cfg.side = side;

    point_set(&param->origo, p1x, p1y);
    param->flat = (MATH_ABS(p2x - p1x) > MATH_ABS(p2y - p1y)) ? 1 : 0;
    param->yx_steep = 0;
    param->xy_steep = 0;
    param->dsc.cb = (drv_epic_mask_xcb_t)draw_mask_line;
    param->dsc.type = DRAW_MASK_TYPE_LINE;

    int32_t dx = p2x - p1x;
    int32_t dy = p2y - p1y;

    if (param->flat)
    {
        /*Normalize the steep. Delta x should be relative to delta x = 1024*/
        int32_t m;

        if (dx)
        {
            m = (1L << 20) / dx;  /*m is multiplier to normalize y (upscaled by 1024)*/
            param->yx_steep = (m * dy) >> 10;
        }

        if (dy)
        {
            m = (1L << 20) / dy;  /*m is multiplier to normalize x (upscaled by 1024)*/
            param->xy_steep = (m * dx) >> 10;
        }
        param->steep = param->yx_steep;
    }
    else
    {
        /*Normalize the steep. Delta y should be relative to delta x = 1024*/
        int32_t m;

        if (dy)
        {
            m = (1L << 20) / dy;  /*m is multiplier to normalize x (upscaled by 1024)*/
            param->xy_steep = (m * dx) >> 10;
        }

        if (dx)
        {
            m = (1L << 20) / dx;  /*m is multiplier to normalize x (upscaled by 1024)*/
            param->yx_steep = (m * dy) >> 10;
        }
        param->steep = param->xy_steep;
    }

    if (param->cfg.side == DRAW_MASK_LINE_SIDE_LEFT) param->inv = 0;
    else if (param->cfg.side == DRAW_MASK_LINE_SIDE_RIGHT) param->inv = 1;
    else if (param->cfg.side == DRAW_MASK_LINE_SIDE_TOP)
    {
        if (param->steep > 0) param->inv = 1;
        else param->inv = 0;
    }
    else if (param->cfg.side == DRAW_MASK_LINE_SIDE_BOTTOM)
    {
        if (param->steep > 0) param->inv = 0;
        else param->inv = 1;
    }

    param->spx = param->steep >> 2;
    if (param->steep < 0) param->spx = -param->spx;
}

void drv_epic_mask_line_angle_init(drv_epic_mask_line_param_t *param, int32_t p1x, int32_t py, int16_t angle,
                                   drv_epic_mask_line_side_t side)
{
    /*Find an optimal degree.
     *drv_epic_mask_line_angle_init will swap the points to keep the smaller y in p1
     *Theoretically a line with `angle` or `angle+180` is the same only the points are swapped
     *Find the degree which keeps the origo in place*/
    if (angle > 180) angle -= 180; /*> 180 will swap the origo*/

    int32_t p2x;
    int32_t p2y;

    p2x = (EPIC_TrigoSin(angle + 90) >> 5) + p1x;
    p2y = (EPIC_TrigoSin(angle) >> 5) + py;

    drv_epic_mask_line_points_init(param, p1x, py, p2x, p2y, side);
}

void drv_epic_mask_angle_init(drv_epic_mask_angle_param_t *param, int32_t vertex_x, int32_t vertex_y,
                              int32_t start_angle, int32_t end_angle)
{
    drv_epic_mask_line_side_t start_side;
    drv_epic_mask_line_side_t end_side;

    /*Constrain the input angles*/
    if (start_angle < 0)
        start_angle = 0;
    else if (start_angle > 359)
        start_angle = 359;

    if (end_angle < 0)
        end_angle = 0;
    else if (end_angle > 359)
        end_angle = 359;

    if (end_angle < start_angle)
    {
        param->delta_deg = 360 - start_angle + end_angle;
    }
    else
    {
        param->delta_deg = MATH_ABS(end_angle - start_angle);
    }

    param->cfg.start_angle = start_angle;
    param->cfg.end_angle = end_angle;
    point_set(&param->cfg.vertex_p, vertex_x, vertex_y);
    param->dsc.cb = (drv_epic_mask_xcb_t)draw_mask_angle;
    param->dsc.type = DRAW_MASK_TYPE_ANGLE;

    RT_ASSERT(start_angle >= 0 && start_angle <= 360); // "Unexpected start angle"

    if (start_angle >= 0 && start_angle < 180)
    {
        start_side = DRAW_MASK_LINE_SIDE_LEFT;
    }
    else
        start_side = DRAW_MASK_LINE_SIDE_RIGHT; /*silence compiler*/

    RT_ASSERT(end_angle >= 0 && start_angle <= 360); // "Unexpected end angle"

    if (end_angle >= 0 && end_angle < 180)
    {
        end_side = DRAW_MASK_LINE_SIDE_RIGHT;
    }
    else if (end_angle >= 180 && end_angle < 360)
    {
        end_side = DRAW_MASK_LINE_SIDE_LEFT;
    }
    else
        end_side = DRAW_MASK_LINE_SIDE_RIGHT; /*silence compiler*/

    drv_epic_mask_line_angle_init(&param->start_line, vertex_x, vertex_y, start_angle, start_side);
    drv_epic_mask_line_angle_init(&param->end_line, vertex_x, vertex_y, end_angle, end_side);
}

void drv_epic_mask_radius_init(drv_epic_mask_radius_param_t *param, const drv_epic_mask_area_t *rect, int32_t radius,
                               bool inv)
{
    int32_t w = area_get_width(rect);
    int32_t h = area_get_height(rect);
    int32_t short_side = MATH_MIN(w, h);
    if (radius > short_side >> 1) radius = short_side >> 1;
    if (radius < 0) radius = 0;

    area_copy(&param->cfg.rect, rect);
    param->cfg.radius = radius;
    param->cfg.outer = inv ? 1 : 0;
    param->dsc.cb = (drv_epic_mask_xcb_t)draw_mask_radius;
    param->dsc.type = DRAW_MASK_TYPE_RADIUS;

    if (radius == 0)
    {
        param->circle = NULL;
        return;
    }

    mutex_lock(&circle_cache_mutex);

    uint32_t i;

    /*Try to reuse a circle cache entry*/
    for (i = 0; i < DRAW_CIRCLE_CACHE_SIZE; i++)
    {
        if (_circle_cache[i].radius == radius)
        {
            _circle_cache[i].used_cnt++;
            CIRCLE_CACHE_AGING(_circle_cache[i].life, radius);
            param->circle = &(_circle_cache[i]);
            mutex_unlock(&circle_cache_mutex);
            return;
        }
    }

    /*If not cached use the free entry with lowest life*/
    drv_epic_mask_radius_circle_dsc_t *entry = NULL;
    for (i = 0; i < DRAW_CIRCLE_CACHE_SIZE; i++)
    {
        if (_circle_cache[i].used_cnt == 0)
        {
            if (!entry) entry = &(_circle_cache[i]);
            else if (_circle_cache[i].life < entry->life) entry = &(_circle_cache[i]);
        }
    }

    /*There is no unused entry. Allocate one temporarily*/
    if (!entry)
    {
        entry = mem_alloc_zeroed(sizeof(drv_epic_mask_radius_circle_dsc_t));
        ASSERT_MALLOC(entry);
        entry->life = -1;
    }
    else
    {
        entry->used_cnt++;
        entry->life = 0;
        CIRCLE_CACHE_AGING(entry->life, radius);
    }

    param->circle = entry;

    circ_calc_aa4(param->circle, radius);
    mutex_unlock(&circle_cache_mutex);

}

void drv_epic_mask_fade_init(drv_epic_mask_fade_param_t *param, const drv_epic_mask_area_t *coords, drv_epic_mask_opa_t drv_epic_mask_opa_top,
                             int32_t y_top,
                             drv_epic_mask_opa_t opa_bottom, int32_t y_bottom)
{
    area_copy(&param->cfg.coords, coords);
    param->cfg.drv_epic_mask_opa_top = drv_epic_mask_opa_top;
    param->cfg.opa_bottom = opa_bottom;
    param->cfg.y_top = y_top;
    param->cfg.y_bottom = y_bottom;
    param->dsc.cb = (drv_epic_mask_xcb_t)draw_mask_fade;
    param->dsc.type = DRAW_MASK_TYPE_FADE;
}

void drv_epic_mask_map_init(drv_epic_mask_map_param_t *param, const drv_epic_mask_area_t *coords, const drv_epic_mask_opa_t *map)
{
    area_copy(&param->cfg.coords, coords);
    param->cfg.map = map;
    param->dsc.cb = (drv_epic_mask_xcb_t)draw_mask_map;
    param->dsc.type = DRAW_MASK_TYPE_MAP;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM draw_mask_line(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_line_param_t *p)
{
    /*Make to points relative to the vertex*/
    abs_y -= p->origo.y;
    abs_x -= p->origo.x;

    /*Handle special cases*/
    if (p->steep == 0)
    {
        /*Horizontal*/
        if (p->flat)
        {
            /*Non sense: Can't be on the right/left of a horizontal line*/
            if (p->cfg.side == DRAW_MASK_LINE_SIDE_LEFT ||
                    p->cfg.side == DRAW_MASK_LINE_SIDE_RIGHT) return DRAW_MASK_RES_FULL_COVER;
            else if (p->cfg.side == DRAW_MASK_LINE_SIDE_TOP && abs_y < 0) return DRAW_MASK_RES_FULL_COVER;
            else if (p->cfg.side == DRAW_MASK_LINE_SIDE_BOTTOM && abs_y > 0) return DRAW_MASK_RES_FULL_COVER;
            else
            {
                return DRAW_MASK_RES_TRANSP;
            }
        }
        /*Vertical*/
        else
        {
            /*Non sense: Can't be on the top/bottom of a vertical line*/
            if (p->cfg.side == DRAW_MASK_LINE_SIDE_TOP ||
                    p->cfg.side == DRAW_MASK_LINE_SIDE_BOTTOM) return DRAW_MASK_RES_FULL_COVER;
            else if (p->cfg.side == DRAW_MASK_LINE_SIDE_RIGHT && abs_x > 0) return DRAW_MASK_RES_FULL_COVER;
            else if (p->cfg.side == DRAW_MASK_LINE_SIDE_LEFT)
            {
                if (abs_x + len < 0) return DRAW_MASK_RES_FULL_COVER;
                else
                {
                    int32_t k = - abs_x;
                    if (k < 0) return DRAW_MASK_RES_TRANSP;
                    if (k >= 0 && k < len) memzero(&mask_buf[k], len - k);
                    return  DRAW_MASK_RES_CHANGED;
                }
            }
            else
            {
                if (abs_x + len < 0) return DRAW_MASK_RES_TRANSP;
                else
                {
                    int32_t k = - abs_x;
                    if (k < 0) k = 0;
                    if (k >= len) return DRAW_MASK_RES_TRANSP;
                    else if (k >= 0 && k < len) memzero(&mask_buf[0], k);
                    return  DRAW_MASK_RES_CHANGED;
                }
            }
        }
    }

    drv_epic_mask_res_t res;
    if (p->flat)
    {
        res = line_mask_flat(mask_buf, abs_x, abs_y, len, p);
    }
    else
    {
        res = line_mask_steep(mask_buf, abs_x, abs_y, len, p);
    }

    return res;
}

static drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM line_mask_flat(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y,
        int32_t len,
        drv_epic_mask_line_param_t *p)
{

    int32_t y_at_x;
    y_at_x = (int32_t)((int32_t)p->yx_steep * abs_x) >> 10;

    if (p->yx_steep > 0)
    {
        if (y_at_x > abs_y)
        {
            if (p->inv)
            {
                return DRAW_MASK_RES_FULL_COVER;
            }
            else
            {
                return DRAW_MASK_RES_TRANSP;
            }
        }
    }
    else
    {
        if (y_at_x < abs_y)
        {
            if (p->inv)
            {
                return DRAW_MASK_RES_FULL_COVER;
            }
            else
            {
                return DRAW_MASK_RES_TRANSP;
            }
        }
    }

    /*At the end of the mask if the limit line is smaller than the mask's y.
     *Then the mask is in the "good" area*/
    y_at_x = (int32_t)((int32_t)p->yx_steep * (abs_x + len)) >> 10;
    if (p->yx_steep > 0)
    {
        if (y_at_x < abs_y)
        {
            if (p->inv)
            {
                return DRAW_MASK_RES_TRANSP;
            }
            else
            {
                return DRAW_MASK_RES_FULL_COVER;
            }
        }
    }
    else
    {
        if (y_at_x > abs_y)
        {
            if (p->inv)
            {
                return DRAW_MASK_RES_TRANSP;
            }
            else
            {
                return DRAW_MASK_RES_FULL_COVER;
            }
        }
    }

    int32_t xe;
    if (p->yx_steep > 0) xe = ((abs_y * 256) * p->xy_steep) >> 10;
    else xe = (((abs_y + 1) * 256) * p->xy_steep) >> 10;

    int32_t xei = xe >> 8;
    int32_t xef = xe & 0xFF;

    int32_t px_h;
    if (xef == 0) px_h = 255;
    else px_h = 255 - (((255 - xef) * p->spx) >> 8);
    int32_t k = xei - abs_x;
    drv_epic_mask_opa_t m;

    if (xef)
    {
        if (k >= 0 && k < len)
        {
            m = 255 - (((255 - xef) * (255 - px_h)) >> 9);
            if (p->inv) m = 255 - m;
            mask_buf[k] = mask_mix(mask_buf[k], m);
        }
        k++;
    }

    while (px_h > p->spx)
    {
        if (k >= 0 && k < len)
        {
            m = px_h - (p->spx >> 1);
            if (p->inv) m = 255 - m;
            mask_buf[k] = mask_mix(mask_buf[k], m);
        }
        px_h -= p->spx;
        k++;
        if (k >= len) break;
    }

    if (k < len && k >= 0)
    {
        int32_t x_inters = (px_h * p->xy_steep) >> 10;
        m = (x_inters * px_h) >> 9;
        if (p->yx_steep < 0) m = 255 - m;
        if (p->inv) m = 255 - m;
        mask_buf[k] = mask_mix(mask_buf[k], m);
    }

    if (p->inv)
    {
        k = xei - abs_x;
        if (k > len)
        {
            return DRAW_MASK_RES_TRANSP;
        }
        if (k >= 0)
        {
            memzero(&mask_buf[0], k);
        }
    }
    else
    {
        k++;
        if (k < 0)
        {
            return DRAW_MASK_RES_TRANSP;
        }
        if (k <= len)
        {
            memzero(&mask_buf[k], len - k);
        }
    }

    return DRAW_MASK_RES_CHANGED;
}

static drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM line_mask_steep(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y,
        int32_t len,
        drv_epic_mask_line_param_t *p)
{
    int32_t k;
    int32_t x_at_y;
    /*At the beginning of the mask if the limit line is greater than the mask's y.
     *Then the mask is in the "wrong" area*/
    x_at_y = (int32_t)((int32_t)p->xy_steep * abs_y) >> 10;
    if (p->xy_steep > 0) x_at_y++;
    if (x_at_y < abs_x)
    {
        if (p->inv)
        {
            return DRAW_MASK_RES_FULL_COVER;
        }
        else
        {
            return DRAW_MASK_RES_TRANSP;
        }
    }

    /*At the end of the mask if the limit line is smaller than the mask's y.
     *Then the mask is in the "good" area*/
    x_at_y = (int32_t)((int32_t)p->xy_steep * (abs_y)) >> 10;
    if (x_at_y > abs_x + len)
    {
        if (p->inv)
        {
            return DRAW_MASK_RES_TRANSP;
        }
        else
        {
            return DRAW_MASK_RES_FULL_COVER;
        }
    }

    /*X start*/
    int32_t xs = ((abs_y * 256) * p->xy_steep) >> 10;
    int32_t xsi = xs >> 8;
    int32_t xsf = xs & 0xFF;

    /*X end*/
    int32_t xe = (((abs_y + 1) * 256) * p->xy_steep) >> 10;
    int32_t xei = xe >> 8;
    int32_t xef = xe & 0xFF;

    drv_epic_mask_opa_t m;

    k = xsi - abs_x;
    if (xsi != xei && (p->xy_steep < 0 && xsf == 0))
    {
        xsf = 0xFF;
        xsi = xei;
        k--;
    }

    if (xsi == xei)
    {
        if (k >= 0 && k < len)
        {
            m = (xsf + xef) >> 1;
            if (p->inv) m = 255 - m;
            mask_buf[k] = mask_mix(mask_buf[k], m);
        }
        k++;

        if (p->inv)
        {
            k = xsi - abs_x;
            if (k >= len)
            {
                return DRAW_MASK_RES_TRANSP;
            }
            if (k >= 0) memzero(&mask_buf[0], k);

        }
        else
        {
            if (k > len) k = len;
            if (k == 0) return DRAW_MASK_RES_TRANSP;
            else if (k > 0) memzero(&mask_buf[k],  len - k);
        }

    }
    else
    {
        int32_t y_inters;
        if (p->xy_steep < 0)
        {
            y_inters = (xsf * (-p->yx_steep)) >> 10;
            if (k >= 0 && k < len)
            {
                m = (y_inters * xsf) >> 9;
                if (p->inv) m = 255 - m;
                mask_buf[k] = mask_mix(mask_buf[k], m);
            }
            k--;

            int32_t x_inters = ((255 - y_inters) * (-p->xy_steep)) >> 10;

            if (k >= 0 && k < len)
            {
                m = 255 - (((255 - y_inters) * x_inters) >> 9);
                if (p->inv) m = 255 - m;
                mask_buf[k] = mask_mix(mask_buf[k], m);
            }

            k += 2;

            if (p->inv)
            {
                k = xsi - abs_x - 1;

                if (k > len) k = len;
                else if (k > 0) memzero(&mask_buf[0],  k);

            }
            else
            {
                if (k > len) return DRAW_MASK_RES_FULL_COVER;
                if (k >= 0) memzero(&mask_buf[k],  len - k);
            }

        }
        else
        {
            y_inters = ((255 - xsf) * p->yx_steep) >> 10;
            if (k >= 0 && k < len)
            {
                m = 255 - ((y_inters * (255 - xsf)) >> 9);
                if (p->inv) m = 255 - m;
                mask_buf[k] = mask_mix(mask_buf[k], m);
            }

            k++;

            int32_t x_inters = ((255 - y_inters) * p->xy_steep) >> 10;
            if (k >= 0 && k < len)
            {
                m = ((255 - y_inters) * x_inters) >> 9;
                if (p->inv) m = 255 - m;
                mask_buf[k] = mask_mix(mask_buf[k], m);
            }
            k++;

            if (p->inv)
            {
                k = xsi - abs_x;
                if (k > len)  return DRAW_MASK_RES_TRANSP;
                if (k >= 0) memzero(&mask_buf[0],  k);

            }
            else
            {
                if (k > len) k = len;
                if (k == 0) return DRAW_MASK_RES_TRANSP;
                else if (k > 0) memzero(&mask_buf[k],  len - k);
            }
        }
    }

    return DRAW_MASK_RES_CHANGED;
}

static drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM draw_mask_angle(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_angle_param_t *p)
{
    int32_t rel_y = abs_y - p->cfg.vertex_p.y;
    int32_t rel_x = abs_x - p->cfg.vertex_p.x;

    if (p->cfg.start_angle < 180 && p->cfg.end_angle < 180 &&
            p->cfg.start_angle != 0  && p->cfg.end_angle != 0 &&
            p->cfg.start_angle > p->cfg.end_angle)
    {

        if (abs_y < p->cfg.vertex_p.y)
        {
            return DRAW_MASK_RES_FULL_COVER;
        }

        /*Start angle mask can work only from the end of end angle mask*/
        int32_t end_angle_first = (rel_y * p->end_line.xy_steep) >> 10;
        int32_t start_angle_last = ((rel_y + 1) * p->start_line.xy_steep) >> 10;

        /*Do not let the line end cross the vertex else it will affect the opposite part*/
        if (p->cfg.start_angle > 270 && p->cfg.start_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.start_angle > 0 && p->cfg.start_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.start_angle > 90 && p->cfg.start_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        if (p->cfg.end_angle > 270 && p->cfg.end_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.end_angle > 0 &&   p->cfg.end_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.end_angle > 90 &&  p->cfg.end_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        int32_t dist = (end_angle_first - start_angle_last) >> 1;

        drv_epic_mask_res_t res1 = DRAW_MASK_RES_FULL_COVER;
        drv_epic_mask_res_t res2 = DRAW_MASK_RES_FULL_COVER;

        int32_t tmp = start_angle_last + dist - rel_x;
        if (tmp > len) tmp = len;
        if (tmp > 0)
        {
            res1 = draw_mask_line(&mask_buf[0], abs_x, abs_y, tmp, &p->start_line);
            if (res1 == DRAW_MASK_RES_TRANSP)
            {
                memzero(&mask_buf[0], tmp);
            }
        }

        if (tmp > len) tmp = len;
        if (tmp < 0) tmp = 0;
        res2 = draw_mask_line(&mask_buf[tmp], abs_x + tmp, abs_y, len - tmp, &p->end_line);
        if (res2 == DRAW_MASK_RES_TRANSP)
        {
            memzero(&mask_buf[tmp], len - tmp);
        }
        if (res1 == res2) return res1;
        else return DRAW_MASK_RES_CHANGED;
    }
    else if (p->cfg.start_angle > 180 && p->cfg.end_angle > 180 && p->cfg.start_angle > p->cfg.end_angle)
    {

        if (abs_y > p->cfg.vertex_p.y)
        {
            return DRAW_MASK_RES_FULL_COVER;
        }

        /*Start angle mask can work only from the end of end angle mask*/
        int32_t end_angle_first = (rel_y * p->end_line.xy_steep) >> 10;
        int32_t start_angle_last = ((rel_y + 1) * p->start_line.xy_steep) >> 10;

        /*Do not let the line end cross the vertex else it will affect the opposite part*/
        if (p->cfg.start_angle > 270 && p->cfg.start_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.start_angle > 0 && p->cfg.start_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.start_angle > 90 && p->cfg.start_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        if (p->cfg.end_angle > 270 && p->cfg.end_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.end_angle > 0 &&   p->cfg.end_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if (p->cfg.end_angle > 90 &&  p->cfg.end_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        int32_t dist = (end_angle_first - start_angle_last) >> 1;

        drv_epic_mask_res_t res1 = DRAW_MASK_RES_FULL_COVER;
        drv_epic_mask_res_t res2 = DRAW_MASK_RES_FULL_COVER;

        int32_t tmp = start_angle_last + dist - rel_x;
        if (tmp > len) tmp = len;
        if (tmp > 0)
        {
            res1 = draw_mask_line(&mask_buf[0], abs_x, abs_y, tmp, (drv_epic_mask_line_param_t *)&p->end_line);
            if (res1 == DRAW_MASK_RES_TRANSP)
            {
                memzero(&mask_buf[0], tmp);
            }
        }

        if (tmp > len) tmp = len;
        if (tmp < 0) tmp = 0;
        res2 = draw_mask_line(&mask_buf[tmp], abs_x + tmp, abs_y, len - tmp, (drv_epic_mask_line_param_t *)&p->start_line);
        if (res2 == DRAW_MASK_RES_TRANSP)
        {
            memzero(&mask_buf[tmp], len - tmp);
        }
        if (res1 == res2) return res1;
        else return DRAW_MASK_RES_CHANGED;
    }
    else
    {

        drv_epic_mask_res_t res1 = DRAW_MASK_RES_FULL_COVER;
        drv_epic_mask_res_t res2 = DRAW_MASK_RES_FULL_COVER;

        if (p->cfg.start_angle == 180)
        {
            if (abs_y < p->cfg.vertex_p.y) res1 = DRAW_MASK_RES_FULL_COVER;
            else res1 = DRAW_MASK_RES_UNKNOWN;
        }
        else if (p->cfg.start_angle == 0)
        {
            if (abs_y < p->cfg.vertex_p.y) res1 = DRAW_MASK_RES_UNKNOWN;
            else res1 = DRAW_MASK_RES_FULL_COVER;
        }
        else if ((p->cfg.start_angle < 180 && abs_y < p->cfg.vertex_p.y) ||
                 (p->cfg.start_angle > 180 && abs_y >= p->cfg.vertex_p.y))
        {
            res1 = DRAW_MASK_RES_UNKNOWN;
        }
        else
        {
            res1 = draw_mask_line(mask_buf, abs_x, abs_y, len, &p->start_line);
        }

        if (p->cfg.end_angle == 180)
        {
            if (abs_y < p->cfg.vertex_p.y) res2 = DRAW_MASK_RES_UNKNOWN;
            else res2 = DRAW_MASK_RES_FULL_COVER;
        }
        else if (p->cfg.end_angle == 0)
        {
            if (abs_y < p->cfg.vertex_p.y) res2 = DRAW_MASK_RES_FULL_COVER;
            else res2 = DRAW_MASK_RES_UNKNOWN;
        }
        else if ((p->cfg.end_angle < 180 && abs_y < p->cfg.vertex_p.y) ||
                 (p->cfg.end_angle > 180 && abs_y >= p->cfg.vertex_p.y))
        {
            res2 = DRAW_MASK_RES_UNKNOWN;
        }
        else
        {
            res2 = draw_mask_line(mask_buf, abs_x, abs_y, len, &p->end_line);
        }

        if (res1 == DRAW_MASK_RES_TRANSP || res2 == DRAW_MASK_RES_TRANSP) return DRAW_MASK_RES_TRANSP;
        else if (res1 == DRAW_MASK_RES_UNKNOWN && res2 == DRAW_MASK_RES_UNKNOWN) return DRAW_MASK_RES_TRANSP;
        else if (res1 == DRAW_MASK_RES_FULL_COVER &&
                 res2 == DRAW_MASK_RES_FULL_COVER) return DRAW_MASK_RES_FULL_COVER;
        else return DRAW_MASK_RES_CHANGED;
    }
}

static drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM draw_mask_radius(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_radius_param_t *p)
{
    bool outer = p->cfg.outer;
    int32_t radius = p->cfg.radius;
    drv_epic_mask_area_t rect;
    area_copy(&rect, &p->cfg.rect);

    if (outer == false)
    {
        if ((abs_y < rect.y1 || abs_y > rect.y2))
        {
            return DRAW_MASK_RES_TRANSP;
        }
    }
    else
    {
        if (abs_y < rect.y1 || abs_y > rect.y2)
        {
            return DRAW_MASK_RES_FULL_COVER;
        }
    }

    if ((abs_x >= rect.x1 + radius && abs_x + len <= rect.x2 - radius) ||
            (abs_y >= rect.y1 + radius && abs_y <= rect.y2 - radius))
    {
        if (outer == false)
        {
            /*Remove the edges*/
            int32_t last = rect.x1 - abs_x;
            if (last > len) return DRAW_MASK_RES_TRANSP;
            if (last >= 0)
            {
                memzero(&mask_buf[0], last);
            }

            int32_t first = rect.x2 - abs_x + 1;
            if (first <= 0) return DRAW_MASK_RES_TRANSP;
            else if (first < len)
            {
                memzero(&mask_buf[first], len - first);
            }
            if (last == 0 && first == len) return DRAW_MASK_RES_FULL_COVER;
            else return DRAW_MASK_RES_CHANGED;
        }
        else
        {
            int32_t first = rect.x1 - abs_x;
            if (first < 0) first = 0;
            if (first <= len)
            {
                int32_t last = rect.x2 - abs_x - first + 1;
                if (first + last > len) last = len - first;
                if (last >= 0)
                {
                    memzero(&mask_buf[first], last);
                }
            }
        }
        return DRAW_MASK_RES_CHANGED;
    }

    int32_t k = rect.x1 - abs_x; /*First relevant coordinate on the of the mask*/
    int32_t w = area_get_width(&rect);
    int32_t h = area_get_height(&rect);
    abs_x -= rect.x1;
    abs_y -= rect.y1;

    int32_t aa_len;
    int32_t x_start;
    int32_t cir_y;
    if (abs_y < radius)
    {
        cir_y = radius - abs_y - 1;
    }
    else
    {
        cir_y = abs_y - (h - radius);
    }
    drv_epic_mask_opa_t *aa_opa = get_next_line(p->circle, cir_y, &aa_len, &x_start);
    int32_t cir_x_right = k + w - radius + x_start;
    int32_t cir_x_left = k + radius - x_start - 1;
    int32_t i;

    if (outer == false)
    {
        for (i = 0; i < aa_len; i++)
        {
            drv_epic_mask_opa_t opa = aa_opa[aa_len - i - 1];
            if (cir_x_right + i >= 0 && cir_x_right + i < len)
            {
                mask_buf[cir_x_right + i] = mask_mix(opa, mask_buf[cir_x_right + i]);
            }
            if (cir_x_left - i >= 0 && cir_x_left - i < len)
            {
                mask_buf[cir_x_left - i] = mask_mix(opa, mask_buf[cir_x_left - i]);
            }
        }

        /*Clean the right side*/
        cir_x_right = MATH_CLAMP(0, cir_x_right + i, len);
        memzero(&mask_buf[cir_x_right], len - cir_x_right);

        /*Clean the left side*/
        cir_x_left = MATH_CLAMP(0, cir_x_left - aa_len + 1, len);
        memzero(&mask_buf[0], cir_x_left);
    }
    else
    {
        for (i = 0; i < aa_len; i++)
        {
            drv_epic_mask_opa_t opa = 255 - (aa_opa[aa_len - 1 - i]);
            if (cir_x_right + i >= 0 && cir_x_right + i < len)
            {
                mask_buf[cir_x_right + i] = mask_mix(opa, mask_buf[cir_x_right + i]);
            }
            if (cir_x_left - i >= 0 && cir_x_left - i < len)
            {
                mask_buf[cir_x_left - i] = mask_mix(opa, mask_buf[cir_x_left - i]);
            }
        }

        int32_t clr_start = MATH_CLAMP(0, cir_x_left + 1, len);
        int32_t clr_len = MATH_CLAMP(0, cir_x_right - clr_start, len - clr_start);
        memzero(&mask_buf[clr_start], clr_len);
    }

    return DRAW_MASK_RES_CHANGED;
}

static drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM draw_mask_fade(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_fade_param_t *p)
{
    if (abs_y < p->cfg.coords.y1) return DRAW_MASK_RES_FULL_COVER;
    if (abs_y > p->cfg.coords.y2) return DRAW_MASK_RES_FULL_COVER;
    if (abs_x + len < p->cfg.coords.x1) return DRAW_MASK_RES_FULL_COVER;
    if (abs_x > p->cfg.coords.x2) return DRAW_MASK_RES_FULL_COVER;

    if (abs_x + len > p->cfg.coords.x2) len -= abs_x + len - p->cfg.coords.x2 - 1;

    if (abs_x < p->cfg.coords.x1)
    {
        int32_t x_ofs = 0;
        x_ofs = p->cfg.coords.x1 - abs_x;
        len -= x_ofs;
        mask_buf += x_ofs;
    }

    int32_t i;

    if (abs_y <= p->cfg.y_top)
    {
        for (i = 0; i < len; i++)
        {
            mask_buf[i] = mask_mix(mask_buf[i], p->cfg.drv_epic_mask_opa_top);
        }
        return DRAW_MASK_RES_CHANGED;
    }
    else if (abs_y >= p->cfg.y_bottom)
    {
        for (i = 0; i < len; i++)
        {
            mask_buf[i] = mask_mix(mask_buf[i], p->cfg.opa_bottom);
        }
        return DRAW_MASK_RES_CHANGED;
    }
    else
    {
        /*Calculate the opa proportionally*/
        int16_t opa_diff = p->cfg.opa_bottom - p->cfg.drv_epic_mask_opa_top;
        int32_t y_diff = p->cfg.y_bottom - p->cfg.y_top + 1;
        drv_epic_mask_opa_t opa_act = MASK_OPA_MIX2(abs_y - p->cfg.y_top, opa_diff) / y_diff;
        opa_act += p->cfg.drv_epic_mask_opa_top;

        for (i = 0; i < len; i++)
        {
            mask_buf[i] = mask_mix(mask_buf[i], opa_act);
        }
        return DRAW_MASK_RES_CHANGED;
    }
}

static drv_epic_mask_res_t DRV_EPIC_ATTRIBUTE_FAST_MEM draw_mask_map(drv_epic_mask_opa_t *mask_buf, int32_t abs_x,
        int32_t abs_y, int32_t len,
        drv_epic_mask_map_param_t *p)
{
    /*Handle out of the mask cases*/
    if (abs_y < p->cfg.coords.y1) return DRAW_MASK_RES_FULL_COVER;
    if (abs_y > p->cfg.coords.y2) return DRAW_MASK_RES_FULL_COVER;
    if (abs_x + len < p->cfg.coords.x1) return DRAW_MASK_RES_FULL_COVER;
    if (abs_x > p->cfg.coords.x2) return DRAW_MASK_RES_FULL_COVER;

    /*Got to the current row in the map*/
    const drv_epic_mask_opa_t *map_tmp = p->cfg.map;
    map_tmp += (abs_y - p->cfg.coords.y1) * area_get_width(&p->cfg.coords);

    if (abs_x + len > p->cfg.coords.x2) len -= abs_x + len - p->cfg.coords.x2 - 1;

    if (abs_x < p->cfg.coords.x1)
    {
        int32_t x_ofs = 0;
        x_ofs = p->cfg.coords.x1 - abs_x;
        len -= x_ofs;
        mask_buf += x_ofs;
    }
    else
    {
        map_tmp += (abs_x - p->cfg.coords.x1);
    }

    int32_t i;
    for (i = 0; i < len; i++)
    {
        mask_buf[i] = mask_mix(mask_buf[i], map_tmp[i]);
    }

    return DRAW_MASK_RES_CHANGED;
}

/**
 * Initialize the circle drawing
 * @param c pointer to a point. The coordinates will be calculated here
 * @param tmp point to a variable. It will store temporary data
 * @param radius radius of the circle
 */
static void circ_init(drv_epic_mask_point_t *c, int32_t *tmp, int32_t radius)
{
    c->x = radius;
    c->y = 0;
    *tmp = 1 - radius;
}

/**
 * Test the circle drawing is ready or not
 * @param c same as in circ_init
 * @return true if the circle is not ready yet
 */
static bool circ_cont(drv_epic_mask_point_t *c)
{
    return c->y <= c->x;
}

/**
 * Get the next point from the circle
 * @param c same as in circ_init. The next point stored here.
 * @param tmp same as in circ_init.
 */
static void circ_next(drv_epic_mask_point_t *c, int32_t *tmp)
{

    if (*tmp <= 0)
    {
        (*tmp) += 2 * c->y + 3; /*Change in decision criterion for y -> y+1*/
    }
    else
    {
        (*tmp) += 2 * (c->y - c->x) + 5; /*Change for y -> y+1, x -> x-1*/
        c->x--;
    }
    c->y++;
}

static void circ_calc_aa4(drv_epic_mask_radius_circle_dsc_t *c, int32_t radius)
{
    if (radius == 0) return;
    c->radius = radius;

    /*Allocate buffers*/
    if (c->buf) mem_free(c->buf);

    c->buf = mem_alloc(radius * 6 + 6);  /*Use uint16_t for opa_start_on_y and x_start_on_y*/
    ASSERT_MALLOC(c->buf);
    c->cir_opa = c->buf;
    c->opa_start_on_y = (uint16_t *)(c->buf + 2 * radius + 2);
    c->x_start_on_y = (uint16_t *)(c->buf + 4 * radius + 4);

    /*Special case, handle manually*/
    if (radius == 1)
    {
        c->cir_opa[0] = 180;
        c->opa_start_on_y[0] = 0;
        c->opa_start_on_y[1] = 1;
        c->x_start_on_y[0] = 0;
        return;
    }

    const size_t cir_xy_size = (radius + 1) * 2 * 2 * sizeof(int32_t);
    int32_t *cir_x = mem_alloc_zeroed(cir_xy_size);
    ASSERT_MALLOC(cir_x);
    int32_t *cir_y = &cir_x[(radius + 1) * 2];

    uint32_t y_8th_cnt = 0;
    drv_epic_mask_point_t cp;
    int32_t tmp;
    circ_init(&cp, &tmp, radius * 4);    /*Upscale by 4*/
    int32_t i;

    uint32_t x_int[4];
    uint32_t x_fract[4];
    int32_t cir_size = 0;
    x_int[0] = cp.x >> 2;
    x_fract[0] = 0;

    /*Calculate an 1/8 circle*/
    while (circ_cont(&cp))
    {
        /*Calculate 4 point of the circle */
        for (i = 0; i < 4; i++)
        {
            circ_next(&cp, &tmp);
            if (circ_cont(&cp) == false) break;
            x_int[i] = cp.x >> 2;
            x_fract[i] = cp.x & 0x3;
        }
        if (i != 4) break;

        /*All lines on the same x when downscaled*/
        if (x_int[0] == x_int[3])
        {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            c->cir_opa[cir_size] = x_fract[0] + x_fract[1] + x_fract[2] + x_fract[3];
            c->cir_opa[cir_size] *= 16;
            cir_size++;
        }
        /*Second line on new x when downscaled*/
        else if (x_int[0] != x_int[1])
        {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            c->cir_opa[cir_size] = x_fract[0];
            c->cir_opa[cir_size] *= 16;
            cir_size++;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            c->cir_opa[cir_size] = 1 * 4 + x_fract[1] + x_fract[2] + x_fract[3];;
            c->cir_opa[cir_size] *= 16;
            cir_size++;
        }
        /*Third line on new x when downscaled*/
        else if (x_int[0] != x_int[2])
        {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            c->cir_opa[cir_size] = x_fract[0] + x_fract[1];
            c->cir_opa[cir_size] *= 16;
            cir_size++;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            c->cir_opa[cir_size] = 2 * 4 + x_fract[2] + x_fract[3];;
            c->cir_opa[cir_size] *= 16;
            cir_size++;
        }
        /*Forth line on new x when downscaled*/
        else
        {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            c->cir_opa[cir_size] = x_fract[0] + x_fract[1] + x_fract[2];
            c->cir_opa[cir_size] *= 16;
            cir_size++;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            c->cir_opa[cir_size] = 3 * 4 + x_fract[3];;
            c->cir_opa[cir_size] *= 16;
            cir_size++;
        }

        y_8th_cnt++;
    }

    /*The point on the 1/8 circle is special, calculate it manually*/
    int32_t mid = radius * 723;
    int32_t mid_int = mid >> 10;
    if (cir_x[cir_size - 1] != mid_int || cir_y[cir_size - 1] != mid_int)
    {
        int32_t tmp_val = mid - (mid_int << 10);
        if (tmp_val <= 512)
        {
            tmp_val = tmp_val * tmp_val * 2;
            tmp_val = tmp_val >> (10 + 6);
        }
        else
        {
            tmp_val = 1024 - tmp_val;
            tmp_val = tmp_val * tmp_val * 2;
            tmp_val = tmp_val >> (10 + 6);
            tmp_val = 15 - tmp_val;
        }

        cir_x[cir_size] = mid_int;
        cir_y[cir_size] = mid_int;
        c->cir_opa[cir_size] = tmp_val;
        c->cir_opa[cir_size] *= 16;
        cir_size++;
    }

    /*Build the second octet by mirroring the first*/
    for (i = cir_size - 2; i >= 0; i--, cir_size++)
    {
        cir_x[cir_size] = cir_y[i];
        cir_y[cir_size] = cir_x[i];
        c->cir_opa[cir_size] = c->cir_opa[i];
    }

    int32_t y = 0;
    i = 0;
    c->opa_start_on_y[0] = 0;
    while (i < cir_size)
    {
        c->opa_start_on_y[y] = i;
        c->x_start_on_y[y] = cir_x[i];
        for (; cir_y[i] == y && i < (int32_t)cir_size; i++)
        {
            c->x_start_on_y[y] = MATH_MIN(c->x_start_on_y[y], cir_x[i]);
        }
        y++;
    }

    mem_free(cir_x);
}

static drv_epic_mask_opa_t *get_next_line(drv_epic_mask_radius_circle_dsc_t *c, int32_t y, int32_t *len,
        int32_t *x_start)
{
    *len = c->opa_start_on_y[y + 1] - c->opa_start_on_y[y];
    *x_start = c->x_start_on_y[y];
    return &c->cir_opa[c->opa_start_on_y[y]];
}

static inline drv_epic_mask_opa_t DRV_EPIC_ATTRIBUTE_FAST_MEM mask_mix(drv_epic_mask_opa_t mask_act, drv_epic_mask_opa_t mask_new)
{
    if (mask_new >= 253) return mask_act;
    if (mask_new <= 2) return 0;

    return MATH_UDIV255(mask_act * mask_new);
}


static void point_set(drv_epic_mask_point_t *p, int32_t x, int32_t y)
{
    p->x = x;
    p->y = y;
}

static int32_t area_get_width(const drv_epic_mask_area_t *area_p)
{
    return (int32_t)(area_p->x2 - area_p->x1 + 1);
}

static int32_t area_get_height(const drv_epic_mask_area_t *area_p)
{
    return (int32_t)(area_p->y2 - area_p->y1 + 1);
}

inline static void area_copy(drv_epic_mask_area_t *dest, const drv_epic_mask_area_t *src)
{
    dest->x1 = src->x1;
    dest->y1 = src->y1;
    dest->x2 = src->x2;
    dest->y2 = src->y2;
}

static inline void *mem_alloc(size_t size)
{
    void *alloc = rt_malloc(size);
    return alloc;
}

static void *mem_alloc_zeroed(size_t size)
{
    void *alloc = rt_malloc(size);
    if (alloc) memset(alloc, 0x00, size);
    return alloc;
}


static void mem_free(void *data)
{
    rt_free(data);
}

static inline void memzero(void *dst, size_t len)
{
    memset(dst, 0x00, len);
}


#endif /*1*/
