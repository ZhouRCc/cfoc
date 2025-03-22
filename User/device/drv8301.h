#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <spi.h>

// DRV8301 SPI配置结构体
typedef struct {
    SPI_HandleTypeDef *hspi;       // SPI外设句柄
    GPIO_TypeDef *cs_gpio_port;    // CS引脚所在的GPIO端口
    uint16_t cs_pin;               // CS引脚编号
} DRV8301_SPIConfig_t;

// 控制寄存器1的位定义
// 控制寄存器1的位定义
#define GATE_CURRENT_MASK 0x0003 // D1 和 D0 位
#define GATE_CURRENT_1_7A 0x0000
#define GATE_CURRENT_0_7A 0x0001
#define GATE_CURRENT_0_25A 0x0002

#define GATE_RESET_MASK 0x0004 // D2 位
#define GATE_RESET_NORMAL 0x0000
#define GATE_RESET_RESET 0x0004

#define PWM_MODE_MASK 0x0008 // D3 位
#define PWM_MODE_6_INPUTS 0x0000
#define PWM_MODE_3_INPUTS 0x0008

#define OCP_MODE_MASK 0x0030 // D5 和 D4 位
#define OCP_MODE_CURRENT_LIMIT 0x0000
#define OCP_MODE_LATCH_SHUTDOWN 0x0010
#define OCP_MODE_REPORT_ONLY 0x0020
#define OCP_MODE_DISABLED 0x0030

#define OC_ADJ_SET_MASK 0x07C0 // D10-D6 位

// 控制寄存器2的位定义
// 控制寄存器2的位定义
#define OCTW_MODE_MASK 0x0003 // D1 和 D0 位
#define OCTW_MODE_BOTH 0x0000
#define OCTW_MODE_OT_ONLY 0x0001
#define OCTW_MODE_OC_ONLY 0x0002

#define GAIN_MASK 0x000C // D3 和 D2 位
#define GAIN_10_V_V 0x0000
#define GAIN_20_V_V 0x0004
#define GAIN_40_V_V 0x0008
#define GAIN_80_V_V 0x000C

#define DC_CAL_CH1_MASK 0x0020 // D5 位
#define DC_CAL_CH1_CONNECT 0x0000
#define DC_CAL_CH1_SHORT 0x0020

#define DC_CAL_CH2_MASK 0x0040 // D6 位
#define DC_CAL_CH2_CONNECT 0x0000
#define DC_CAL_CH2_SHORT 0x0040

#define OC_TOFF_MASK 0x0080 // D7 位
#define OC_TOFF_CYCLE_BY_CYCLE 0x0000
#define OC_TOFF_OFF_TIME 0x0080

// 函数声明
uint16_t DRV8301_ReadRegister(uint8_t reg);
uint16_t DRV8301_WriteRegister(uint8_t reg, uint16_t data);
uint16_t DRV8301_ModifyRegister(uint8_t reg, uint16_t mask, uint16_t value);
uint16_t DRV8301_ConfigureControl1(uint16_t gate_current, uint16_t gate_reset, uint16_t pwm_mode, uint16_t ocp_mode);
uint16_t DRV8301_ConfigureControl2(uint16_t octw_mode, uint16_t gain, uint16_t dc_cal_ch1, uint16_t dc_cal_ch2, uint16_t oc_toff);
uint16_t DRV8301_GetDeviceID(void);
uint16_t DRV8301_SetPWMMode(uint16_t pwm_mode);
uint16_t DRV8301_SetCurrentGain(uint16_t gain);

#ifdef __cplusplus
}
#endif

