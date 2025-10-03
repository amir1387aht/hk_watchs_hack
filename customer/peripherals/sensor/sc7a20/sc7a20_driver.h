#ifndef __SC7A20_DRIVER_H__
#define __SC7A20_DRIVER_H__




#ifdef __cplusplus
extern "C" {
#endif

int Triaxial_Sensor_Init(void);
void Triaxial_Sensor_Disable(void);
void Triaxial_Sensor_Enable(void);

unsigned int Gsensor_Read_Data(void);
unsigned int Gsensor_Set_Step(void);
unsigned int Gsensor_Get_Step(void);
unsigned char Gsensor_Get_Motion_Status(void);
unsigned int Gsensor_Get_Step_Distance(void);
unsigned int Gsensor_Get_Step_KCal(void);
unsigned int Gsensor_Set_Gesture_Init(void);
signed char Gsensor_Get_Gesture_status(void);

#ifdef __cplusplus
}
#endif
#endif

