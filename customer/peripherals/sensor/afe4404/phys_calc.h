/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PHYS_PARAMS_CALC_H_
#define PHYS_PARAMS_CALC_H_

#include "pp_config.h"

//If a function is deprecated it means it might be removed in the future.
// ____lifeq_deprecated__ functions have been replaced with __attribute__ ((deprecated)).
#ifndef ____lifeq_deprecated__
#define ____lifeq_deprecated__      __attribute__ ((deprecated))
#endif

//If a function has the not implemented tag it should not be called and is reserved for future implementation.
#ifndef __not_implemented__
#define __not_implemented__
#endif

//If a function has the experimental tag it is not considered ready for commercial use.
#ifndef __experimental__
#define __experimental__
#endif

/**
 * Algorithm type enum.
 */
typedef enum
{
    PP_HEART_RATE  = 0,   // code for heart rate algorithm.
    PP_HRV         = 1,   // code for Heart rate variability algorithm.
    PP_SPO2        = 2,   // code for SPO2 algorithm.
    PP_CALIBRATION = 3,   // code for calibration irrespective of the algorithm in use (while initializing, to not feed unwanted data to algorithms).
    PP_HEART_RATE_AND_HRV = 4, // code for running heart rate variability and heart rate algorithms in parallel.
    PP_SLEEP       = 5,   // code for sleep algorithm, includes heart rate variability and heart rate algorithms in parallel.
    PP_STRESS = 6,        // __not_implemented__
    PP_RR         = 7,    // only runs RR algorithm on inputs.
    PP_SKIN_DETECT = 50,  // all algorithms are switched off and input samples are evaluated for on-skin.

    PP_CALC_ALL    = 99,  // run all available algorithms. Under development.
} alg_input_selection_t;

typedef enum
{
    LQ_LED_NONE     = 100,
    LQ_LED_GREEN    = 0,
    LQ_LED_RED      = 1,
    LQ_LED_INFRARED = 2,
    LQ_LED_AMBIENT  = 3,
} lq_led_t;        /// Wave length of LED.

typedef struct input_sample
{
    uint32_t  sample;                 ///< Sample value.
    uint32_t  rf_kohm;                ///< Channel gain as kohm value.
    int16_t   isub;                   ///< only TI4404 & TI4405 otherwise 0;
    uint16_t  led_curr_mAx10;         ///< Input current in mA x 10.
    uint8_t   count;                  ///< Number of samples accumulated to get sample.
    uint8_t   num_amp;                ///< Number of trans impedance amplifiers being used.
    uint8_t   led;                    ///< Sample wavelength description.
} led_sample_t;

/**
 * pp results struct.
 */
typedef struct pp_results
{

    // -------------------------------------------------------------------------
    // alg_input_selection_t.PP_HEART_RATE
    // -------------------------------------------------------------------------
    uint8_t  lq_hr;         ///< Heart rate (bpm).
    uint8_t  lq_cadence;    ///< Cadence  (rpm).
    int8_t   hr_confidence; ///< @see PP_GetHRConfidence().

    // -------------------------------------------------------------------------
    // lifeq level 1 metrics
    // -------------------------------------------------------------------------
    uint8_t  lq_score ; // __not_implemented__;
    uint16_t lq_vo2;       ///< Oxyen consumption (l/min*100).
    uint16_t lq_epoc;      ///< (ml/kg*10) @PP_GetLifeQ_EPOC().
    uint16_t lq_lactate;   ///< Blood lactate level (mmol/l*100).
    uint16_t lq_calories;  ///< Calories burnt (kCal).
    uint16_t lq_MaxHR;     ///< max hr (bpm).
    // -------------------------------------------------------------------------
} pp_results_t;

/**
 * pp samples struct.
 */
typedef struct pp_samples
{
    int32_t accel1;
    int32_t accel2;
    int32_t accel3;
    led_sample_t ch_green;
    led_sample_t ch_ambient;
    led_sample_t ch_red;
    led_sample_t ch_infrared;
} pp_samples_t;

#endif /* defined(PHYS_PARAMS_CALC_H_) */

#ifdef __cplusplus
}
#endif

