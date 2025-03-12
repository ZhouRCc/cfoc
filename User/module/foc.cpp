#include "module/foc.hpp"
#include<adc.h>
#include<tim.h>
#include "bsp/bsp_delay.h"

Foc foc;

void foc_init(void){
    delay_ms(1000);
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
    while(!foc.flag_offset)//如果偏置电压校准未完成，则一直等待
        ;
    HAL_GPIO_WritePin(EN_GATE_GPIO_Port, EN_GATE_Pin, GPIO_PIN_SET);

}

void foc_run(void)
{
    foc.foc_step();
}

Foc::Foc()
{
    Ud_Uq.x1 = 0.0f;
    Ud_Uq.x2 = 0.5f;
    Ub_offset = 0;
    Uc_offset = 0;
    offset_cnt = 0;
    flag_offset = false;
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
    Ub_tmp = Ub_offset - adc_value[1];
    Uc_tmp = Uc_offset - adc_value[2];
    Ia_Ib_Ic.x2 = (Ub_tmp * 3.3f) / 4095.0f / 10 / 0.0005f;
    Ia_Ib_Ic.x3 = (Uc_tmp * 3.3f) / 4095.0f / 10 / 0.0005f;
    Ia_Ib_Ic.x1 = - Ia_Ib_Ic.x2 - Ia_Ib_Ic.x3;
}

void Foc::foc_offset_calibration()
{
    offset_cnt ++;
    Ub_offset += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
    Uc_offset += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3);

    if(offset_cnt >= 600)
    {
        Ub_offset = Ub_offset / offset_cnt;
        Uc_offset = Uc_offset / offset_cnt;
        flag_offset = true;
    }
}

void Foc::foc_step()
{
    theta += 0.01f;
    if (theta > 6.28f) theta = 0.0f;
    Inv_Park(&Ud_Uq, &Ualpha_Ubeta, theta);
    sector = Svpwm(&Ualpha_Ubeta, Udc, 6720, &Out_ccr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Out_ccr.x1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, Out_ccr.x2);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, Out_ccr.x3);
    // Ano_Send_U16(sector, Out_ccr.x1, Out_ccr.x2, Out_ccr.x3, 0);

    
    // Ano_SendFloat(Ualpha_Ubeta.x1,Ualpha_Ubeta.x2,theta,0.0f,0.0f);
    // Ano_SendFloat(Ua_Ub_Uc.x1,Ua_Ub_Uc.x2,Ua_Ub_Uc.x3,0.0f,0.0f);
    // Ano_SendFloat(Ialpha_Ibeta.x1,Ialpha_Ibeta.x2,0.0f,0.0f,0.0f);
    // Ano_SendFloat(Id_Iq.x1,Id_Iq.x2,0.0f,0.0f,0.0f);
    delay_ms(1);
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
    }
}
