/**
  ****************************(C) COPYRIGHT ZhouCc****************************
  * @file       foc_lib.c/h
  * @brief      foc算法程序
  * @note      
  * @history
  *  Version      Date           Author          Modification
  *    v1       10.4.2023        ZhouCc           按照公式写完了
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT ZhouCc****************************
  */
#include "lib/foc_lib.h"
#include "arm_math.h"

/**
 * @brief  弧度转角度
 * @param  rad  弧度
 * @retval 角度
 */
float32_t rad2deg(float32_t rad)
{
    return rad * (180.0f / PI);
}

/**
 * @brief  角度转弧度
 * @param  deg  角度
 * @retval 弧度
 */
float32_t deg2rad(float32_t deg)
{
    return deg * (PI / 180.0f);
}

/**
 * @brief  Park线性变换，将一个二相静止坐标系变换到二相旋转坐标系，实质上只是一个旋转矩阵。
 *         基本公式为：
 *                      | I_d |   =   |  cos(theta)  sin(theta)  |   x   | I_alpha | 
 *                      | I_q |       | -sin(theta)  cos(theta)  |       | I_beta  |
 * 
 * @param  Ialpha_Ibeta 被变换的二相静止电流向量
 * @param  theta 角度
 * @param  outpId_Iq 变换得到的二相旋转电流向量
 * @retval none
 */
void Park(param2_t* Ialpha_Ibeta,param2_t* Id_Iq,float theta)
{
    Id_Iq->x1 = Ialpha_Ibeta->x1 * arm_cos_f32(theta) + Ialpha_Ibeta->x2 * arm_sin_f32(theta);
    Id_Iq->x2 = - (Ialpha_Ibeta->x1 * arm_sin_f32(theta)) + Ialpha_Ibeta->x2 * arm_cos_f32(theta);
}
 
/**
 * @brief  Park反变换，将一个二相旋转坐标系变换到二相静止坐标系，实质上只是一个逆向的旋转矩阵。
 *         基本公式为：
 *                      | U_alpha |   =   |  cos(theta) -sin(theta)  |   x   | U_d | 
 *                      | U_beta  |       |  sin(theta)  cos(theta)  |       | U_q |
 * @param  Ud_Uq 被变换的三相电流向量，是一个三维的向量
 * @param  Uapha_Ubeta 变换得到的二相正交电流向量，是一个二维的向量
 * @param  theta  角度值
 * @retval none
 */
void Inv_Park(param2_t* Ud_Uq,param2_t* Uapha_Ubeta,float theta)
{
    Uapha_Ubeta->x1 = Ud_Uq->x1 * arm_cos_f32(theta) - Ud_Uq->x2 * arm_sin_f32(theta);
    Uapha_Ubeta->x2 = Ud_Uq->x1 * arm_sin_f32(theta) + Ud_Uq->x2 * arm_cos_f32(theta);
}

/**
 * @brief  Clarke线性变换，将一个三相电流向量分解为二相正交向量
 *         基本公式为：  Ia + Ib + Ic = 0
 *                      | I_alpha |       |  1    -1/2     -1/2     |       | Ia |
 *                      | I_beta  |   =   |  0   3^0.5/2  -3^0.5/2  |   x   | Ib |
 *                                                                          | Ic |
 * @param  Ia_Ib_Ic 被变换的三相电流向量，是一个三维的向量
 * @param  Ialpha_Ibeta 变换得到的二相正交电流向量，是一个二维的向量
 * @note    此foc基于等幅值变换，所以最终结果乘了3/2
 * @retval none
 */
void Clarke(param3_t* Ia_Ib_Ic , param2_t* Ialpha_Ibeta)
{
//根据Ia + Ib + Ic = 0 得出下面的公式
    Ialpha_Ibeta->x1 = Ia_Ib_Ic->x1;
    Ialpha_Ibeta->x2 = (Ia_Ib_Ic->x1 + Ia_Ib_Ic->x2 * 2) * SQRT3_3;
}

/**
 * @brief  反Clarke变换，将一个二相正交向量转换为三相电流向量
 *         基本公式为：
 *                      | Ia |   =   |  1    0       |   x   | I_alpha |
 *                      | Ib |       | -1/2  3^0.5/2 |       | I_beta  |
 *                      | Ic |       | -1/2 -3^0.5/2 |
 * @param  Ialpha_Ibeta 被变换的二相正交电流向量
 * @param  Ia_Ib_Ic 变换得到的三相电流向量
 * @retval none
 */
void Inv_Clarke(param2_t* Ualpha_Ubeta, param3_t* Ua_Ub_Uc)
{
    Ua_Ub_Uc->x1 = Ualpha_Ubeta->x1;
    Ua_Ub_Uc->x2 = -0.5f * Ualpha_Ubeta->x1 + SQRT3_2 * Ualpha_Ubeta->x2;
    Ua_Ub_Uc->x3 = -0.5f * Ualpha_Ubeta->x1 - SQRT3_2 * Ualpha_Ubeta->x2;
}

/**
 * @brief  反Clarke变换2，将一个二相正交向量转换为三相电流向量
 *         基本公式为：
 *                      | Ia |   =   |  1    0       |   x   | I_beta |
 *                      | Ib |       | -1/2  3^0.5/2 |       | I_alpha |
 *                      | Ic |       | -1/2 -3^0.5/2 |
 * @param  Ialpha_Ibeta 被变换的二相正交电流向量
 * @param  Ia_Ib_Ic 变换得到的三相电流向量
 * @retval none
 */
void Inv_Clarke_2(param2_t* Ualpha_Ubeta, param3_t* Ua_Ub_Uc)
{
    Ua_Ub_Uc->x1 = Ualpha_Ubeta->x2;
    Ua_Ub_Uc->x2 = -0.5f * Ualpha_Ubeta->x2 + SQRT3_2 * Ualpha_Ubeta->x1;
    Ua_Ub_Uc->x3 = -0.5f * Ualpha_Ubeta->x2 - SQRT3_2 * Ualpha_Ubeta->x1;
}


/**
 * @brief  进行svpwm扇区判断和占空比计算
 * @param  Ualpha_Ubeta 经过Inv_Park解算后的Ualpha和Ubeta
 * @param  Udc moss管两端电压
 * @param  Tpwm pwm记数值，计算方法：2*arr的值（因为选用中心对齐计数模式），根据psc和arr可算出pwm频率
 * @param  Out_ccr 计算得出的三相pwm占空比
 * @return  返回当前扇区
 * @note none
 */
uint8_t Svpwm(param2_t* Ualpha_Ubeta,float Udc,float Tpwm, param3_t* Out_ccr)
{
/*******************作用时长计算使用的变量**********************/
    float com = (SQRT3 * Tpwm)/Udc;//一个计算作用时长的通用参数 (3^0.5 * Ts)/Udc
    float X , Y , Z , Tx , Ty;/*计算得出的作用时长，对应扇区的Tx Ty为 扇区| 1 | 2 | 3 | 4 | 5 | 6 |
                                                                    |Tx| Z |-Z | X |-X |  Y| -Y |
                                                                    |Ty| X |-Y | Y |-Z |  Z| -X|*/
    float ta, tb, tc;//暂存占空比
/********************扇区判断使用的变量*****************/
    uint8_t sector = 0;//扇区
    uint8_t N = 0;/*通过N进行扇区判断         对应值 | 扇区 | 1 | 2 | 3 | 4 | 5 | 6 |
                                                   |   N  | 3 | 1 | 5 | 4 | 6 | 2 |          */
/*************************anti_clarke计算***********************/
/*基本公式
 *              |Ua| = |    1         0  |   | Ubeta  |
 *              |Ub|   | -1/2    3^0.5/2 | x | Ualpha  |
 *              |Uc|   | -1/2   -3^0.5/2 |
 **/                                                 
    float A = Ualpha_Ubeta->x2;
    float B = Ualpha_Ubeta->x1 * SQRT3_2 - Ualpha_Ubeta->x2 * 0.5f;
    float C = -Ualpha_Ubeta->x1 * SQRT3_2 - Ualpha_Ubeta->x2 * 0.5f;

    if(A > 0)
    {
        N += 1;
    }
    if(B > 0)
    {
        N += 2;
    }
    if(C > 0)
    {
        N += 4;
    }
/****************作用时长计算*****************/
    X = com * A; 
    Y = com * B;
    Z = com * C;
//根据扇区计算Tx，Ty
    switch(N)
    {
    case 1:
        sector = 2;
        Tx = -Z;
        Ty = -Y;
        break;
    case 2:
        sector = 6;
        Tx = -Y;
        Ty = -X;
        break;
    case 3:
        sector = 1;
        Tx = Z;
        Ty = X;
        break;
    case 4:
        sector = 4;
        Tx = -X;
        Ty = -Z;
        break;
    case 5:
        sector = 3;
        Tx = X;
        Ty = Y;
        break;
    default:
        sector = 5;
        Tx = Y;
        Ty = Z;
        break; 
    }
    if(Tx + Ty > Tpwm)
    {
        float total = Tx + Ty;
        Tx = (Tx / total)*Tpwm;
        Ty = (Ty / total)*Tpwm;
    }
    ta = (Tpwm - Tx - Ty)/4;
    tb = ta + Tx/2;
    tc = tb + Ty/2; 
    switch(N)
    {
    case 1:
        Out_ccr->x1 = tb;
        Out_ccr->x2 = ta;
        Out_ccr->x3 = tc; 
        break;
    case 2:
        Out_ccr->x1 = ta;
        Out_ccr->x2 = tc;
        Out_ccr->x3 = tb; 
        break;
    case 3:
        Out_ccr->x1 = ta;
        Out_ccr->x2 = tb;
        Out_ccr->x3 = tc; 
        break;
    case 4:
        Out_ccr->x1 = tc;
        Out_ccr->x2 = tb;
        Out_ccr->x3 = ta; 
        break;
    case 5:
        Out_ccr->x1 = tc;
        Out_ccr->x2 = ta;
        Out_ccr->x3 = tb; 
        break;
    case 6:
        Out_ccr->x1 = tb;
        Out_ccr->x2 = tc;
        Out_ccr->x3 = ta; 
        break;
    default:
        Out_ccr->x1 = 0;
        Out_ccr->x2 = 0;
        Out_ccr->x3 = 0; 
        break;
    }
    return sector;
}

/**
  * @brief  foc实际计算过程函数
  * @param  theta 当前角度
  * @param  Ud_Uq q轴与d轴电压
  * @param  Ia_Ib_Ic 三相电流
  * @return  device_status_e
  * @note  无
 */
// void foc_step(param2_t* Ud_Uq,float theta,param3_t* Ia_Ib_Ic)
// {
//     param2_t Ualpha_Ubeta;
//     uint8_t sector;

//     Inv_Park(Ud_Uq,&Ualpha_Ubeta,theta);
//     sector = Svpwm(&Ualpha_Ubeta,24,TPWM,&Ia_Ib_Ic);

// }
 
 