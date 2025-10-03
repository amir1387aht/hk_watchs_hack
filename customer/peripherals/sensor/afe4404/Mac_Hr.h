/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MAC_HR_HDR_XX01__
#define __MAC_HR_HDR_XX01__


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialize the variables, This function should be called every time the algorithm starts
 * @Param : None
 * @return: None
 */


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void Mac_Init();

/**
 * @brief Calculate Heart rate function
 * @param GreenLed : Green Led value, AccX : Acc X G value, AccY : Acc Y G value, AccZ : Acc Z G value
 * @return: HR
 */

int Partron_ALG(double LED1, double AccX, double AccY, double AccZ);

/**
 * @brief Calculate BBI function
 * @return: BBI
 */

int BBI_Calculation();

/** * @brief Check that the library link is correct
* @param: None
* @return: Some defined hr values
    {0,0,0,0,0,0,0,0,0,0,21,38,57,75,93,91,96,100,105,110,116,116,116,116,119,122,124,127,130,130,130}
*/

int Mac_HR_Validcheck();

double Adaptive_MACancel_2(double *dc_GREEN1, double *ACC_X1, double *ACC_Y1, double *ACC_Z1);
int Get_snr();

#ifdef __cplusplus
}
#endif
#endif  // __MAC_HR_HDR_XX01__

