
#pragma once
 
#ifdef __cplusplus
extern "C"{
#endif

#include "main.h"

#define SQRT3 1.732050807568f
#define SQRT3_3 0.577350269190f
#define SQRT3_2 0.866025403784f

#define PI_2 1.570796326795f

#define TPWM 3360
#define ADC2U 1

typedef struct 
{
    float x1;
    float x2;
    float x3;
}param3_t;


typedef struct 
{
    float x1;
    float x2;
}param2_t;
 

typedef struct
{
    param2_t Iab;
    param2_t Iqd;
    param3_t Iabc;
}foc_param_t;

void Park(param2_t* Ialpha_Ibeta,param2_t* Id_Iq,float theta);
void Inv_Park(param2_t* Ud_Uq,param2_t* Uapha_Ubeta,float theta);
void Clarke(param3_t* Ia_Ib_Ic , param2_t* Ialpha_Ibeta);
void Inv_Clarke(param2_t* Ualpha_Ubeta, param3_t* Ua_Ub_Uc);
// void foc_step(param2_t* Ud_Uq,float theta,param3_t* Ia_Ib_Ic);
uint8_t Svpwm(param2_t* Ualpha_Ubeta,float Udc,float Tpwm, param3_t* Out_ccr);
 
#ifdef __cplusplus
}
#endif

