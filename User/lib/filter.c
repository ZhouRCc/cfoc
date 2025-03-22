#include "lib/filter.h"
#include "arm_math.h"

/**
  * @brief          初始化一阶低通滤波器
  * @param[out]     lpf: 低通滤波器结构体指针
  * @param[in]      sampleRate: 采样频率 (Hz)
  * @param[in]      fc: 截止频率 (Hz)
  * @retval         无
  */
void LowPassFilter_Init(LowPassFilter *lpf, float sampleRate, float fc) 
{
    lpf->sampleRate = sampleRate;
    lpf->fc = fc;
    float RC = 1.0 / (2 * M_PI * fc); // 时间常数
    float dt = 1.0 / sampleRate;      // 采样时间
    lpf->a_coef = dt / (RC + dt);     // 计算系数 a
    lpf->b_coef = 1.0 - lpf->a_coef;  // 计算系数 b
    lpf->last_output = 0.0;           // 初始化上一次的输出值
}

/**
  * @brief          计算一阶低通滤波器的输出
  * @param[out]     lpf: 低通滤波器结构体指针
  * @param[in]      input: 输入信号
  * @retval         滤波后的输出信号
  */
float LowPassFilter_Calc(LowPassFilter *lpf, float input) 
{
    // 计算当前输出值
    lpf->output = lpf->a_coef * input + lpf->b_coef * lpf->last_output;
    lpf->last_output = lpf->output; // 更新上一次的输出值
    return lpf->output;
}


/**
  * @brief          初始化二阶巴特沃斯低通滤波器
  * @param[out]     filter: 滤波器结构体指针
  * @param[in]      cutoffFreq: 截止频率
  * @param[in]      sampleRate: 采样率
  * @retval         无
  */
void biquadLowFilter_Init(BiquadFilter* filter, float cutoffFreq, float sampleRate) {
    float omega = 2.0f * M_PI * cutoffFreq / sampleRate;
    float alpha = arm_sin_f32(omega) / 2.0f; // Q = 0.7071 (Butterworth)

    filter->b0 = (1.0f - arm_cos_f32(omega)) / 2.0f;
    filter->b1 = 1.0f - arm_cos_f32(omega);
    filter->b2 = filter->b0;
    filter->a1 = -2.0f * arm_cos_f32(omega);
    filter->a2 = 1.0f - alpha;

    // 归一化系数
    float norm = 1.0f + alpha;
    filter->b0 /= norm;
    filter->b1 /= norm;
    filter->b2 /= norm;
    filter->a1 /= norm;
    filter->a2 /= norm;

    // 初始化延迟
    filter->x1 = filter->x2 = 0.0f;
    filter->y1 = filter->y2 = 0.0f;
}


/**
  * @brief          初始化二阶巴特沃斯高通滤波器
  * @param[out]     filter: 滤波器结构体指针
  * @param[in]      cutoffFreq: 截止频率
  * @param[in]      sampleRate: 采样率
  * @retval         无
  */
void biquadHighFilter_Init(BiquadFilter* filter, float cutoffFreq, float sampleRate) {
    float omega = 2.0f * M_PI * cutoffFreq / sampleRate;
    float alpha = arm_sin_f32(omega) / 2.0f; // Q = 0.7071 (Butterworth)

    filter->b0 = (1.0f + arm_cos_f32(omega)) / 2.0f;
    filter->b1 = -(1.0f + arm_cos_f32(omega));
    filter->b2 = filter->b0;
    filter->a1 = -2.0f * arm_cos_f32(omega);
    filter->a2 = 1.0f - alpha;

    // 归一化系数
    float norm = 1.0f + alpha;
    filter->b0 /= norm;
    filter->b1 /= norm;
    filter->b2 /= norm;
    filter->a1 /= norm;
    filter->a2 /= norm;

    // 初始化延迟
    filter->x1 = filter->x2 = 0.0f;
    filter->y1 = filter->y2 = 0.0f;
}

/**
  * @brief          计算滤波器输出（通用函数，适用于高通和低通）
  * @param[out]     filter: 滤波器结构体指针
  * @param[in]      input: 输入信号
  * @retval         滤波后的输出信号
  */
float computeFilter(BiquadFilter* filter, float input) {
    float output = filter->b0 * input + filter->b1 * filter->x1 + filter->b2 * filter->x2
                   - filter->a1 * filter->y1 - filter->a2 * filter->y2;

    // 更新延迟
    filter->x2 = filter->x1;
    filter->x1 = input;
    filter->y2 = filter->y1;
    filter->y1 = output;

    return output;
}


