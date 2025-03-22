#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float Kp;
    float Ki;
    float Kd;

    float max_out;
    float max_iout;

    float set;
    float fdb;

    float out;
    float Pout;
    float Iout;
    float Dout;
    float Dbuf[3];
    float error[3];

} pid_type_def;

void PID_init(pid_type_def *pid, const float PID[3], float max_out, float max_iout);
float PID_calc(pid_type_def *pid, float ref, float set);


#ifdef __cplusplus
}
#endif

