/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef SECTION_ITER_H__
#define SECTION_ITER_H__

#include <stddef.h>
#include "section.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup section_iter Section variables iterator
 * @ingroup middleware
 * @{
 */

/**@brief Single section description structure. */
typedef struct
{
    void *p_start;      //!< Pointer to the start of section.
    void *p_end;        //!< Pointer to the end of section.
} section_t;


/**@brief Set of the sections description structure. */
typedef struct
{
#if defined(__GNUC__)
    section_t           section;    //!< Description of the set of sections.
    /**<
     * In case of GCC all sections in the set are sorted and
     * placed in contiguous area, because they are treated as
     * one section.
     */
#else
    section_t const    *p_first;    //!< Pointer to the first section in the set.
    section_t const    *p_last;     //!< Pointer to the last section in the set.
#endif
    size_t                  item_size;  //!< Size of the single item in the section.
} section_set_t;


/**@brief Section iterator structure. */
typedef struct
{
    section_set_t const *p_set;         //!< Pointer to the appropriate section set.
#if !defined(__GNUC__)
    section_t const      *p_section;    //!< Pointer to the selected section.
    /**<
     * In case of GCC all sections in the set are sorted and
     * placed in contiguous area, because they are treated
     * as one section.
     */
#endif
    void                     *p_item;       //!< Pointer to the selected item in the section.
} section_iter_t;


/**@brief Create a set of sections.
 *
 * @note  This macro reserves memory for the given set of sections.
 *
 * @details  A set of sections, is an ordered collections of sections.
 *
 * @param[in]   _name   Name of the set.
 * @param[in]   _type   Type of the elements stored in the sections.
 * @param[in]   _count  Number of the sections in the set. This parameter is ignored in case of GCC.
 * @hideinitializer
 */
#if defined(__GNUC__)

#define SECTION_SET_DEF(_name, _type, _count)                                                   \
                                                                                                    \
    SECTION_DEF(_name, _type);                                                                  \
    static section_set_t const _name =                                                          \
    {                                                                                               \
        .section =                                                                                  \
        {                                                                                           \
            .p_start = SECTION_START_ADDR(_name),                                               \
            .p_end   = SECTION_END_ADDR(_name),                                                 \
        },                                                                                          \
        .item_size  = sizeof(_type),                                                                \
    }

#else

#define SECTION_SET_DEF(_name, _type, _count)                                                   \
MACRO_REPEAT_FOR(_count, SECTION_DEF_, _name, _type)                                            \
static section_t const CONCAT_2(_name, _array)[] =                                              \
{                                                                                                   \
    MACRO_REPEAT_FOR(_count, SECTION_SET_DEF_, _name)                                           \
};                                                                                                  \
static section_set_t const _name =                                                              \
{                                                                                                   \
    .p_first   = CONCAT_2(_name, _array),                                                           \
    .p_last    = CONCAT_2(_name, _array) + ARRAY_SIZE(CONCAT_2(_name, _array)),                     \
    .item_size = sizeof(_type),                                                                     \
}

#ifndef DOXYGEN
#define SECTION_DEF_(_priority, _name, _type)                                                   \
SECTION_DEF(CONCAT_2(_name, _priority), _type);

#define SECTION_SET_DEF_(_priority, _name)                                                      \
{                                                                                                   \
    .p_start = SECTION_START_ADDR(CONCAT_2(_name, _priority)),                                  \
    .p_end   = SECTION_END_ADDR(CONCAT_2(_name, _priority)),                                    \
},
#endif // DOXYGEN
#endif // __GNUC__


/**@brief   Macro to declare a variable and register it in the section set.
 *
 * @note    The order of the section in the set is based on the priority. The order with which
 *          variables are placed in a section is dependant on the order with which the linker
 *          encouters the variables during linking.
 *
 * @param[in]   _name       Name of the section set.
 * @param[in]   _priority   Priority of the desired section.
 * @param[in]   _var        The variable to register in the given section.
 * @hideinitializer
 */
#define SECTION_SET_ITEM_REGISTER(_name, _priority, _var)                                       \
    SECTION_ITEM_REGISTER(CONCAT_2(_name, _priority), _var)


/**@brief Function for initializing the section set iterator.
 *
 * @param[in]   p_iter  Pointer to the iterator.
 * @param[in]   p_set   Pointer to the sections set.
 */
void section_iter_init(section_iter_t *p_iter, section_set_t const *p_set);


/**@brief Function for incrementing iterator.
 *
 * @param[in]   p_iter   Pointer to the iterator.
 */
void section_iter_next(section_iter_t *p_iter);


/**@brief Function for getting the element pointed to by the iterator.
 *
 * @param[in]   p_iter  Pointer to the iterator.
 *
 * @retval  Pointer to the element or NULL if iterator points end of the set.
 */
static inline void *section_iter_get(section_iter_t const *p_iter)
{
    ASSERT(p_iter);
    return p_iter->p_item;
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif // SECTION_ITER_H__
