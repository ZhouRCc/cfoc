#pragma once

#include "module/foc.h"

#include "lib/foc_lib.h"
#include "lib/filter.h"
#include "lib/pid.h"

#include "main.h"

class Foc
{
public:
    Foc();
    void foc_calc_adc();
    void foc_offset_calibration();
    void foc_step();
    void send_data();
    bool flag_offset;
private:
float theta;
    param2_t Id_Iq_set;
    param2_t Ud_Uq;        
    param2_t Ualpha_Ubeta;  
    param_ccr_t Out_ccr;    //三个pwm的ccr值
    uint8_t sector;         //扇区
    float Udc;              //母线电压
    param3_t Ia_Ib_Ic;      //UVW三相电流值
    param2_t Ialpha_Ibeta;  
    param2_t Id_Iq;

    uint32_t Ub_offset;
    uint32_t Uc_offset;
    uint16_t offset_cnt;

    LowPassFilter current_filter;
    pid_type_def current_pid;
};

