
#pragma once

#ifdef __cplusplus
extern "C"{
#endif

#define FRAME_HEADER 		  0XAB   //帧头
#define GENERAL_OUTPUT_ADDR	  0XFF   //广播帧地址
#define ANO_ADDR			  0XFE  //匿名助手地址
#define USER_ADDR_END 		  0XC7  //用户自定义地址为0x01~0xC7



void Ano_Init(void);

void Ano_Send(uint8_t len, ...);
void Ano_SendFloat(float f1,float f2,float f3,float f4,float f5);
void Ano_Send_U16(uint16_t f1,uint16_t f2,uint16_t f3,uint16_t f4,uint16_t f5);

#ifdef __cplusplus
}
#endif



