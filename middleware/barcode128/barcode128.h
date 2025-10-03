/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BARCODE128_H
#define __BARCODE128_H

#ifdef __cplusplus
extern "C" {
#endif

int barcode128_parse(char *barcode, int len, char *code_str, int *code_len, char type);

#ifdef __cplusplus
}
#endif

#endif /* __BARCODE128_H */

