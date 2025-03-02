/**
  ****************************(C) COPYRIGHT ZhouCc****************************
  * @file       ano.c/h
  * @brief      用于匿名助手通信，方便观察数据，调试
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0       2025.3.1        ZhouCc          初步完成
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT ZhouCc****************************
  */

#include "main.h"
#include "device/ano.h"
#include "usart.h"
#include "bsp/bsp_delay.h"
#include <stdarg.h>


// 将数据强制转换为 uint32_t，然后拆分字节（小端模式）
#define BYTE0(dwTemp)       (uint8_t)((*(uint32_t*)&(dwTemp)) & 0xFF)        // 低字节
#define BYTE1(dwTemp)       (uint8_t)(((*(uint32_t*)&(dwTemp)) >> 8) & 0xFF)  // 第二字节
#define BYTE2(dwTemp)       (uint8_t)(((*(uint32_t*)&(dwTemp)) >> 16) & 0xFF) // 第三字节
#define BYTE3(dwTemp)       (uint8_t)(((*(uint32_t*)&(dwTemp)) >> 24) & 0xFF) // 高字节

uint8_t BUFF[100];//缓冲区

/**
  * @brief          初始化
  * @param[in]    	none
  * @retval         none
  */
void Ano_Init(void)
{
	delay_init();

	// __HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);  //receive interrupt
	// __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);  //idle interrupt

}

/**
  * @brief          向匿名助手发送灵活格式帧数据
  * @param[in]      第一个参数为数据个数，全部使用uint32_t数据类型
  * @note           如果要发送float，先乘100然后再转成int
  * @retval         none
  */
void Ano_Send(uint8_t len, ...)
{
	int i;
	int intbuf;
	uint8_t sumcheck = 0;
	uint8_t addcheck = 0;
	uint8_t _cnt=0;
	uint16_t datelen = len * 4;

	va_list args;

	va_start(args,len);

	BUFF[_cnt++] = FRAME_HEADER;
	BUFF[_cnt++] = 0x01;
	BUFF[_cnt++] = ANO_ADDR;
	BUFF[_cnt++] = 0xF1;//灵活格式帧参数id为0xF1-0xFA
	BUFF[_cnt++] = BYTE0(datelen);
	BUFF[_cnt++] = BYTE1(datelen);//数据长度

	while(len --)
	{
		intbuf=va_arg(args,int);
		BUFF[_cnt ++] = BYTE0(intbuf);
		BUFF[_cnt ++] = BYTE1(intbuf);
		BUFF[_cnt ++] = BYTE2(intbuf);
		BUFF[_cnt ++] = BYTE3(intbuf);
	}

	va_end(args);

	for(i = 0;i < datelen + 6;i++) 
	{
		sumcheck+=BUFF[i];
		addcheck+=sumcheck;
	}
	BUFF[_cnt++]=sumcheck;	
	BUFF[_cnt++]=addcheck;	
	
	HAL_UART_Transmit(&huart4,BUFF,_cnt,0xFFF);

}

/**
  * @brief          向匿名助手发送5个float数据
  * @param[in]      5个float参数
  * @note           float数据被乘以100后再转成int
  * @retval         none
  */
void Ano_SendFloat(float f1,float f2,float f3,float f4,float f5)
{
    int i;
    uint8_t sumcheck = 0;
    uint8_t addcheck = 0;
    uint8_t _cnt = 0;
    uint16_t datelen = 5 * 4;



    BUFF[_cnt++] = FRAME_HEADER;
    BUFF[_cnt++] = 0x01;
    BUFF[_cnt++] = ANO_ADDR;
    BUFF[_cnt++] = 0xF2; // 灵活格式帧参数id为0xF1-0xFA
    BUFF[_cnt++] = BYTE0(datelen);
    BUFF[_cnt++] = BYTE1(datelen); // 数据长度

    BUFF[_cnt++] = BYTE0(f1);
    BUFF[_cnt++] = BYTE1(f1);
    BUFF[_cnt++] = BYTE2(f1);
    BUFF[_cnt++] = BYTE3(f1);

	BUFF[_cnt++] = BYTE0(f2);
    BUFF[_cnt++] = BYTE1(f2);
    BUFF[_cnt++] = BYTE2(f2);
    BUFF[_cnt++] = BYTE3(f2);

	BUFF[_cnt++] = BYTE0(f3);
    BUFF[_cnt++] = BYTE1(f3);
    BUFF[_cnt++] = BYTE2(f3);
    BUFF[_cnt++] = BYTE3(f3);

	BUFF[_cnt++] = BYTE0(f4);
    BUFF[_cnt++] = BYTE1(f4);
    BUFF[_cnt++] = BYTE2(f4);
    BUFF[_cnt++] = BYTE3(f4);

	BUFF[_cnt++] = BYTE0(f5);
    BUFF[_cnt++] = BYTE1(f5);
    BUFF[_cnt++] = BYTE2(f5);
    BUFF[_cnt++] = BYTE3(f5);

    for(i = 0; i < datelen + 6; i++) 
    {
        sumcheck += BUFF[i];
        addcheck += sumcheck;
    }
    BUFF[_cnt++] = sumcheck;    
    BUFF[_cnt++] = addcheck;    
    
    HAL_UART_Transmit(&huart4, BUFF, _cnt, 0xFFF);
}


