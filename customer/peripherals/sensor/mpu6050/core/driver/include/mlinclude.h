/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INV_INCLUDE_H__
#define INV_INCLUDE_H__



#ifdef __cplusplus
extern "C" {
#endif

#define INVENSENSE_FUNC_START  typedef int invensensePutFunctionCallsHere

#ifdef COVERAGE
    #include "utestCommon.h"
#endif
#ifdef PROFILE
    #include "profile.h"
#endif

#ifdef WIN32
#ifdef COVERAGE

extern int functionEnterLog(const char *file, const char *func);
extern int functionExitLog(const char *file, const char *func);

#undef INVENSENSE_FUNC_START
#define INVENSENSE_FUNC_START  __pragma(message(__FILE__ "|"__FUNCTION__ )) \
    int dslkQjDsd = functionEnterLog(__FILE__, __FUNCTION__)
#endif // COVERAGE
#endif // WIN32

#ifdef PROFILE
    #undef INVENSENSE_FUNC_START
    #define INVENSENSE_FUNC_START int dslkQjDsd = profileEnter(__FILE__, __FUNCTION__)
    #define return if ( profileExit(__FILE__, __FUNCTION__) ) return
#endif // PROFILE

// #define return if ( functionExitLog(__FILE__, __FUNCTION__) ) return

#ifdef __cplusplus
}
#endif
#endif //INV_INCLUDE_H__
