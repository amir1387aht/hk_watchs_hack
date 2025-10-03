/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#ifndef _vg_lite_ex_h_
#define _vg_lite_ex_h_

#include "vg_lite.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 * Convention: 
 *   1. Use right-hand coordinates, i.e. rotation also use right-hand rule
 *   2. Counter-clockwise polygon
 *
 ***********************************************************************************/


/** 4x4 matrix */
typedef struct vg_lite_matrix_4x4 
{
    vg_lite_float_t m[4][4];    /*! The 4x4 matrix itself, in [row][column] order. */
} vg_lite_matrix_4x4_t;

/** vertex in 3d space*/
typedef struct vg_lite_vertex
{
    float x;
    float y;
    float z;
} vg_lite_vertex_t;

/** vertex in 2d space */
typedef struct vg_lite_vertex_2d
{
    float x;
    float y;
} vg_lite_vertex_2d_t;

/** vector in 2d space */
typedef vg_lite_vertex_2d_t vg_lite_vec_2d_t;

/** vector in 3d space */
typedef vg_lite_vertex_t vg_lite_vec_t;

/** 3d quadrilateral */
typedef vg_lite_vertex_t  vg_lite_quad_t[4];

/** 2d quadrilateral */
typedef vg_lite_vertex_2d_t  vg_lite_quad_2d_t[4];

vg_lite_error_t vg_lite_init_buf(vg_lite_buffer_t *buffer, int32_t width, int32_t height,
                                 vg_lite_buffer_format_t format, void *memory);


vg_lite_error_t vg_lite_init_etc2_buf(vg_lite_buffer_t *buffer, const void *memory);

vg_lite_error_t vg_lite_load_etc2_buf(vg_lite_buffer_t *buffer, const void *memory);


int vg_lite_mat_vertex_multiply(vg_lite_matrix_t *matrix, vg_lite_float_t *x, vg_lite_float_t *y);

vg_lite_error_t vg_lite_mat_vertex_multiply_4x4(vg_lite_matrix_4x4_t *matrix, vg_lite_vertex_t *input, vg_lite_vertex_t *output);

void vg_lite_mat_vertex_multiply2_4x4(vg_lite_matrix_4x4_t *matrix, vg_lite_float_t *x, vg_lite_float_t *y, vg_lite_float_t *z);

void vg_lite_multiply(vg_lite_matrix_t * matrix, vg_lite_matrix_t * mult);

void vg_lite_multiply_4x4(vg_lite_matrix_4x4_t * matrix, vg_lite_matrix_4x4_t * mult);

void vg_lite_vec_cross_multiply(vg_lite_vec_t *product, vg_lite_vec_t *l, vg_lite_vec_t *r);

void vg_lite_mirror_x(vg_lite_matrix_t * matrix);

void vg_lite_mirror_y(vg_lite_matrix_t * matrix);

/** (nx,ny) is normal of mirror axis  */
void vg_lite_mirror(vg_lite_float_t nx, vg_lite_float_t ny, vg_lite_matrix_t * matrix);

void vg_lite_identity_4x4(vg_lite_matrix_4x4_t *matrix);

/** (nx,ny,nz) is normal of mirror plane */
void vg_lite_mirror_4x4(vg_lite_float_t nx, vg_lite_float_t ny, vg_lite_float_t nz, vg_lite_matrix_4x4_t * matrix);


void vg_lite_translate_4x4(vg_lite_float_t x, vg_lite_float_t y, vg_lite_float_t z, vg_lite_matrix_4x4_t *matrix);

/*!
    @abstract Scale a matrix.

    @discussion
    Scale a matrix in both x, y and z directions.

    @param scale_x
    X scale.

    @param scale_y
    Y scale.

    @param scale_y
    Z scale.

    @param matrix
    Pointer to a <code>vg_lite_matrix_t</code> structure that will be scaled.
    */
void vg_lite_scale_4x4(vg_lite_float_t scale_x, vg_lite_float_t scale_y, vg_lite_float_t scale_z, vg_lite_matrix_4x4_t *matrix);

/*!
    @abstract Rotate a matrix.

    @discussion
    Rotate a matrix a certain number of degrees.

    @param degrees
    Number of degrees to rotate the matrix around. Positive numbers rotate counter clock wise.

    @param matrix
    Pointer to a <code>vg_lite_matrix_t</code> structure that will be rotated.
    */
void vg_lite_rotate_x(vg_lite_float_t degrees, vg_lite_matrix_4x4_t *matrix);
void vg_lite_rotate_y(vg_lite_float_t degrees, vg_lite_matrix_4x4_t *matrix);
void vg_lite_rotate_z(vg_lite_float_t degrees, vg_lite_matrix_4x4_t *matrix);

/* aspect: ResX/ResY */
void vg_lite_perspective_4x4(float fovy_degrees, float aspect,
                             float near_val, float far_val, vg_lite_matrix_4x4_t *matrix);


/** construct 3x3 perspective matrix by the quadrilateral, the quadrilateral must be on the plane and not be parallel to axis-z */
vg_lite_error_t vg_lite_perspective_3x3(float fovy_degrees, int width, int height, vg_lite_quad_t quad, vg_lite_matrix_t *matrix);

/** construct 3x3 perspective matrix by the quadrilateral, the quadrilateral must be on the plane and not be parallel to axis-z */
vg_lite_error_t vg_lite_perspective2_3x3(float scaling_factor, int width, int height, vg_lite_vertex_t *v0, vg_lite_vertex_t *v1, 
                                         vg_lite_vertex_t *v2, vg_lite_vertex_t *v3, vg_lite_matrix_t *matrix);

/** get tranform matrix for texture mapping, quad defines a quadrilateral on the plane in the 3D space.
 * 
 * image point (0,0) map to quad[0], point (0,h) map to quad[1], 
 * point (w,h) map to quad[2], point (w,0) map to quad[3]
 * */
vg_lite_error_t vg_lite_get_texture_map_matrix(float fov, float img_w, float img_h, float canvas_w, float canvas_h, 
                                               vg_lite_quad_t quad, vg_lite_matrix_t *matrix);


vg_lite_error_t vg_lite_get_texture_map_matrix2(float scale, float img_w, float img_h, float canvas_w, float canvas_h, 
                                                vg_lite_vertex_t *v0, vg_lite_vertex_t *v1, vg_lite_vertex_t *v2, 
                                                vg_lite_vertex_t *v3, vg_lite_matrix_t *matrix);


int vg_lite_4x4_obj_to_ndc_coords(vg_lite_matrix_4x4_t *matrix,
                                float x_orig,  float y_orig,
                                int width, int height,
                                float near_val, float far_val,
                                float *x,
                                float *y,
                                float *z,
                                float *w);

void vg_lite_4x4_ndc_to_win_coords(float x_orig,  float y_orig,
                                  int width, int height,
                                  float *x,
                                  float *y);

/**  n=(v1-v0)x(v2-v0) */
void vg_lite_get_normal(vg_lite_vertex_t *v0, vg_lite_vertex_t *v1, vg_lite_vertex_t *v2, vg_lite_vertex_t *normal);

void vg_lite_get_affine_matrix(float w, float h, vg_lite_vertex_2d_t *v0, vg_lite_vertex_2d_t *v1, vg_lite_vertex_2d_t *v2, vg_lite_vertex_2d_t *v3, vg_lite_matrix_t *matrix);

void vg_lite_get_affine_matrix2(float w, float h, vg_lite_float_t *v0x, vg_lite_float_t *v0y, 
                                vg_lite_float_t *v1x, vg_lite_float_t *v1y, 
                                vg_lite_float_t *v2x, vg_lite_float_t *v2y, 
                                vg_lite_float_t *v3x, vg_lite_float_t *v3y,
                                vg_lite_matrix_t *matrix);

vg_lite_float_t vg_lite_get_perspective_scaling_factor(float fov_degrees);

#ifdef __cplusplus
}
#endif
#endif /* _vg_lite_ex_h_ */
