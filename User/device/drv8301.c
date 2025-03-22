#include "device/drv8301.h"
#include "bsp/bsp_delay.h"

// 全局变量定义
DRV8301_SPIConfig_t drv8301_config = {
    .hspi = &hspi3,
    .cs_gpio_port = M0_nCS_GPIO_Port,
    .cs_pin = M0_nCS_Pin
};

// DRV8301/DRV8303 SPI 命令
#define DRV8301_READ_CMD  0x8000  // 读命令（MSB = 1）
#define DRV8301_WRITE_CMD 0x0000  // 写命令（MSB = 0）

// DRV8301/DRV8303 寄存器地址
#define DRV8301_REG_STATUS1  0x00
#define DRV8301_REG_STATUS2  0x01
#define DRV8301_REG_CONTROL1 0x02
#define DRV8301_REG_CONTROL2 0x03

// CS 操作宏
#define DRV8301_CS_LOW()  HAL_GPIO_WritePin(drv8301_config.cs_gpio_port, drv8301_config.cs_pin, GPIO_PIN_RESET)
#define DRV8301_CS_HIGH() HAL_GPIO_WritePin(drv8301_config.cs_gpio_port, drv8301_config.cs_pin, GPIO_PIN_SET)

/**
 * @brief 读取DRV8301寄存器的值
 * @param reg: 寄存器地址
 * @return 寄存器的值（11位）
 */
uint16_t DRV8301_ReadRegister(uint8_t reg) {
    // 构造控制字（读命令 + 寄存器地址）
    uint16_t controlword = DRV8301_READ_CMD | (reg << 11);
    uint16_t recbuff = 0x0000;  // 用于存储接收到的数据

    // 拉低 CS 引脚
    DRV8301_CS_LOW();
    delay_us(1);  // 等待 CS 稳定

    // 第一次 SPI 传输：发送读命令
    HAL_SPI_Transmit(drv8301_config.hspi, (uint8_t*)&controlword, 1, HAL_MAX_DELAY);

    // 拉高 CS 引脚
    DRV8301_CS_HIGH();
    delay_us(1);  // 等待 CS 稳定

    // 拉低 CS 引脚
    DRV8301_CS_LOW();
    delay_us(1);  // 等待 CS 稳定

    // 第二次 SPI 传输：发送 0x0000，接收真正的数据
    uint16_t zerobuff = 0x0000;  // 发送 0x0000
    HAL_SPI_TransmitReceive(drv8301_config.hspi, (uint8_t*)&zerobuff, (uint8_t*)&recbuff, 1, HAL_MAX_DELAY);

    // 拉高 CS 引脚
    DRV8301_CS_HIGH();
    delay_us(1);  // 等待 CS 稳定

    // 返回接收到的数据（只保留 11 位有效数据）
    return recbuff & 0x07FF;
}



/**
 * @brief 向DRV8301寄存器写入值
 * @param reg: 寄存器地址
 * @param data: 要写入的数据（11位）
 * @return 状态寄存器1的值（11位）
 */
uint16_t DRV8301_WriteRegister(uint8_t reg, uint16_t data) {
    uint16_t tx_word = DRV8301_WRITE_CMD | (reg << 11) | (data & 0x07FF); // 构造16位写命令
    uint8_t rx_data = 0;  // 接收数据
    uint8_t tx_dummy = 0x0000; // 发送填充数据
    uint8_t rx_dummy = 0; // 接收状态寄存器

    DRV8301_CS_LOW();
    delay_us(1);

    // 第一次传输：发送写命令
    HAL_SPI_TransmitReceive(drv8301_config.hspi, (uint8_t*)&tx_word, (uint8_t*)&rx_dummy, 1, HAL_MAX_DELAY);

    DRV8301_CS_HIGH();

    delay_ms(1);
    
    DRV8301_CS_LOW();
    delay_us(1);

    // 第二次传输：接收状态寄存器1的值
    HAL_SPI_TransmitReceive(drv8301_config.hspi, (uint8_t*)&tx_dummy, (uint8_t*)&rx_data, 1, HAL_MAX_DELAY);

    DRV8301_CS_HIGH(); // 拉高CS引脚

    return rx_data & 0x07FF; // 返回16位状态寄存器值
}


/**
 * @brief 修改寄存器的特定位
 * @param reg: 寄存器地址
 * @param mask: 需要修改的位掩码
 * @param value: 新的值
 * @return 状态寄存器1的值
 */
uint16_t DRV8301_ModifyRegister(uint8_t reg, uint16_t mask, uint16_t value) {
    // 读取当前寄存器值
    uint16_t current_value = DRV8301_ReadRegister(reg);

    // 修改特定位
    current_value = (current_value & ~mask) | (value & mask);

    // 写回寄存器
    return DRV8301_WriteRegister(reg, current_value);
}

/**
 * @brief 配置控制寄存器1
 * @param gate_current: 门极驱动峰值电流
 * @param gate_reset: 门极驱动复位模式
 * @param pwm_mode: PWM模式
 * @param ocp_mode: 过流保护模式
 * @return 状态寄存器1的值
 */
uint16_t DRV8301_ConfigureControl1(uint16_t gate_current, uint16_t gate_reset, uint16_t pwm_mode, uint16_t ocp_mode) {
    uint16_t status = 0;

    // 修改门极驱动峰值电流
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL1, GATE_CURRENT_MASK, gate_current);

    // 修改门极驱动复位模式
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL1, GATE_RESET_MASK, gate_reset);

    // 修改PWM模式
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL1, PWM_MODE_MASK, pwm_mode);

    // 修改过流保护模式
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL1, OCP_MODE_MASK, ocp_mode);

    return status;
}

/**
 * @brief 配置控制寄存器2
 * @param octw_mode: OCTW模式
 * @param gain: 电流放大倍数
 * @param dc_cal_ch1: 通道1直流校准
 * @param dc_cal_ch2: 通道2直流校准
 * @param oc_toff: 过流关闭时间控制
 * @return 状态寄存器1的值
 */
uint16_t DRV8301_ConfigureControl2(uint16_t octw_mode, uint16_t gain, uint16_t dc_cal_ch1, uint16_t dc_cal_ch2, uint16_t oc_toff) {
    uint16_t status = 0;

    // 修改OCTW模式
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL2, OCTW_MODE_MASK, octw_mode);

    // 修改电流放大倍数
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL2, GAIN_MASK, gain);

    // 修改通道1直流校准
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL2, DC_CAL_CH1_MASK, dc_cal_ch1);

    // 修改通道2直流校准
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL2, DC_CAL_CH2_MASK, dc_cal_ch2);

    // 修改过流关闭时间控制
    status = DRV8301_ModifyRegister(DRV8301_REG_CONTROL2, OC_TOFF_MASK, oc_toff);

    return status;
}

/**
 * @brief 获取DRV8303的设备ID
 * @return 设备ID
 */
uint16_t DRV8301_GetDeviceID(void) {
    return DRV8301_ReadRegister(DRV8301_REG_STATUS2) & 0x0F; // 设备ID在STATUS2寄存器的低4位
}

/**
 * @brief 设置 PWM 模式
 * @param pwm_mode: PWM 模式 (PWM_MODE_6_INPUTS 或 PWM_MODE_3_INPUTS)
 * @return 状态寄存器1的值
 */
uint16_t DRV8301_SetPWMMode(uint16_t pwm_mode) {
    // 读取当前寄存器1的值
    uint16_t current_value = DRV8301_ReadRegister(DRV8301_REG_CONTROL1);

    // 修改 PWM 模式位
    current_value = (current_value & ~PWM_MODE_MASK) | (pwm_mode & PWM_MODE_MASK);

    // 写回寄存器1
    return DRV8301_WriteRegister(DRV8301_REG_CONTROL1, current_value);
}

/**
 * @brief 设置电流增益
 * @param gain: 电流增益 (GAIN_10_V_V, GAIN_20_V_V, GAIN_40_V_V, GAIN_80_V_V)
 * @return 状态寄存器2的值
 */
uint16_t DRV8301_SetCurrentGain(uint16_t gain) {
    // 读取当前寄存器2的值
    uint16_t current_value = DRV8301_ReadRegister(DRV8301_REG_CONTROL2);

    // 修改电流增益位
    current_value = (current_value & ~GAIN_MASK) | (gain & GAIN_MASK);

    // 写回寄存器2
    return DRV8301_WriteRegister(DRV8301_REG_CONTROL2, current_value);
}

