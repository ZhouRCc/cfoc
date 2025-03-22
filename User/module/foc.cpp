#include "module/foc.hpp"
#include<adc.h>
#include<tim.h>
#include "bsp/bsp_delay.h"
#include "device/ano.h"

Foc foc;

float CURRENT_PID[3] = {0.1f, 0.001f, 0.0f};

void foc_init(void){

    HAL_ADCEx_InjectedStart(&hadc1);
	__HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_JEOC);
	
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  
    HAL_TIMEx_OCN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_OCN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_OCN_Start(&htim1, TIM_CHANNEL_3);
	
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);
    HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_SET);

    while(!foc.flag_offset)//如果偏置电压校准未完成，则一直等待
    ;

}

void foc_send(void)
{
    foc.send_data();
}


Foc::Foc()
{
    Id_Iq_set.x1 = 0.0f;
    Id_Iq_set.x2 = 0.3f;
    Ub_offset = 0;
    Uc_offset = 0;
    offset_cnt = 0;
    flag_offset = false;

    LowPassFilter_Init(&current_filter, 25000 , 2500);
    PID_init(&current_pid[0], CURRENT_PID, 6.0f, 6.0f);
    PID_init(&current_pid[1], CURRENT_PID, 6.0f, 6.0f);
}


void Foc::foc_calc_adc()
{
    uint32_t adc_value[3] = {0};
    float Ub_tmp = 0.0f;
    float Uc_tmp = 0.0f;

    adc_value[0] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
    Udc = (adc_value[0] * 3.3f * 11.0f) / 4095.0f;
    adc_value[1] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
    adc_value[2] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3);
    Ub_tmp = Ub_offset - (float)adc_value[1];
    Uc_tmp = Uc_offset - (float)adc_value[2];
    // Ia_Ib_Ic.x2 = Ub_tmp * 3.3f / 4095 / 10 / 0.0005;
    // Ia_Ib_Ic.x3 = Uc_tmp * 3.3f / 4095 / 10 / 0.0005;
    Ia_Ib_Ic.x2 = Ub_tmp * 1.6117 / 80;
    Ia_Ib_Ic.x3 = Uc_tmp * 1.6117 / 80;
    Ia_Ib_Ic.x1 = - Ia_Ib_Ic.x2 - Ia_Ib_Ic.x3;
}

void Foc::send_data()
{
    Ano_SendFloat(Id_Iq.x1,Id_Iq.x2,0);
}

void Foc::foc_offset_calibration()
{
    uint32_t adc_tmp[2] = {0};
    adc_tmp[0] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
    adc_tmp[1] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3);
    if(adc_tmp[0] > 2000 && adc_tmp[1] > 2000)
    {
        if(offset_cnt < 200)
        {
            offset_cnt ++;
            Ub_offset += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
            Uc_offset += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3);
        }
        else
        {
            Ub_offset = Ub_offset / offset_cnt;
            Uc_offset = Uc_offset / offset_cnt;
            flag_offset = true;
        }
    }
}

void Foc::foc_step()
{
    param2_t Id_Iq_tmp = {0.0f, 0.0f};

    theta += 0.002f;
    if (theta > 6.28f) theta = 0.0f;

    Clarke(&Ia_Ib_Ic,&Ialpha_Ibeta);
    Park(&Ialpha_Ibeta,&Id_Iq_tmp, theta);
    LowPassFilter_Calc(&current_filter, Id_Iq_tmp.x1);
    LowPassFilter_Calc(&current_filter, Id_Iq_tmp.x2);

    Id_Iq.x1 = Id_Iq_tmp.x1;
    Id_Iq.x2 = Id_Iq_tmp.x2;

    Ud_Uq.x1 = PID_calc(&current_pid[0], Id_Iq.x1, Id_Iq_set.x1);
    Ud_Uq.x2 = PID_calc(&current_pid[1], Id_Iq.x2, Id_Iq_set.x2);

    Inv_Park(&Ud_Uq, &Ualpha_Ubeta, theta);
    sector = Svpwm(&Ualpha_Ubeta, Udc, 6720, &Out_ccr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Out_ccr.x1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, Out_ccr.x2);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, Out_ccr.x3);
    
}


void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(hadc == &hadc1)
    {
        if(foc.flag_offset)
        {
            foc.foc_calc_adc();
        }
        else
        {
            foc.foc_offset_calibration();
        }
        foc.foc_step();
    }
}
