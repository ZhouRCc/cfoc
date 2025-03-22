#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// 低通滤波器结构体
typedef struct {
    float output;       // 当前输出
    float last_output;  // 上次输出
    float a_coef;       // 系数α
    float b_coef;       // 系数(1-α)
    float sampleRate; // 采样频率
    float fc;           // 截止频率(Hz)
} LowPassFilter;


// 二阶巴特沃斯滤波器结构体
typedef struct {
    float a1, a2; // 分母系数
    float b0, b1, b2; // 分子系数
    float x1, x2; // 输入延迟
    float y1, y2; // 输出延迟
} BiquadFilter;


#ifdef __cplusplus
}
#endif

