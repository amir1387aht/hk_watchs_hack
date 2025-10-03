/*
 * SPDX-FileCopyrightText: 2019-2022 SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BUZZER_TEST_MUSIC_H__
#define __BUZZER_TEST_MUSIC_H__



#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    short mName; // 音名：取值L1～L7、M1～M7、H1～H7分别表示低音、中音、高音的1234567，取值0表示休止符
    short mTime; // 时值：取值T、T/2、T/4、T/8、T/16、T/32分别表示全音符、二分音符、四分音符、八分音符.，取值0表示演奏结束
} tNote;

extern const tNote MyScore1[];
extern const tNote MyScore2[];
extern const tNote MyScore3[];


void musicPlay(tNote *MyScore) ;

#ifdef __cplusplus
}
#endif
#endif /* __BUZZER_TEST_MUSIC_H__ */
