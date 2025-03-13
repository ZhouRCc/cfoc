import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import find_peaks
from scipy.fft import fft, fftfreq

# 读取CSV文件
df = pd.read_csv('222.csv')

# 提取时间列和信号列
time = pd.to_datetime(df['Time'], format='%H:%M:%S:%f')  # 将时间列转换为时间格式
signal1 = df['F2#1'].values
signal2 = df['F2#2'].values
signal3 = df['F2#0'].values

# 计算时间轴（以秒为单位）
time_seconds = (time - time.iloc[0]).dt.total_seconds()

# 绘制波形
plt.figure(figsize=(12, 8))
plt.plot(time_seconds, signal1, label='F2#1')
plt.plot(time_seconds, signal2, label='F2#2')
plt.plot(time_seconds, signal3, label='F2#0')
plt.xlabel('Time [s]')
plt.ylabel('Amplitude')
plt.title('Waveforms of F2#1, F2#2, and F2#0')
plt.legend()
plt.grid(True)
plt.show()

# 计算幅值
amplitude1 = np.max(signal1) - np.min(signal1)
amplitude2 = np.max(signal2) - np.min(signal2)
amplitude3 = np.max(signal3) - np.min(signal3)
print(f'Amplitude of F2#1: {amplitude1}')
print(f'Amplitude of F2#2: {amplitude2}')
print(f'Amplitude of F2#0: {amplitude3}')

# 计算平均幅值
def calculate_mean_amplitude(signal):
    return np.mean(np.abs(signal))

mean_amplitude1 = calculate_mean_amplitude(signal1)
mean_amplitude2 = calculate_mean_amplitude(signal2)
mean_amplitude3 = calculate_mean_amplitude(signal3)
print(f'Mean amplitude of F2#1: {mean_amplitude1}')
print(f'Mean amplitude of F2#2: {mean_amplitude2}')
print(f'Mean amplitude of F2#0: {mean_amplitude3}')

# 计算相位差
def calculate_phase_difference(signal1, signal2, time_seconds):
    # 使用FFT计算相位差
    n = len(time_seconds)
    fft1 = fft(signal1)
    fft2 = fft(signal2)
    freq = fftfreq(n, d=(time_seconds[1] - time_seconds[0]))  # 计算频率

    # 找到主要频率成分
    dominant_freq = freq[np.argmax(np.abs(fft1)[1:n//2]) + 1]  # 忽略直流分量
    phase1 = np.angle(fft1[np.argmax(np.abs(fft1)[1:n//2]) + 1])
    phase2 = np.angle(fft2[np.argmax(np.abs(fft2)[1:n//2]) + 1])

    # 计算相位差（以度为单位）
    phase_diff = np.rad2deg(phase2 - phase1)
    return phase_diff, dominant_freq

# 计算F2#1和F2#2的相位差
phase_diff_12, freq_12 = calculate_phase_difference(signal1, signal2, time_seconds)
print(f'Phase difference between F2#1 and F2#2: {phase_diff_12:.2f} degrees at {freq_12:.2f} Hz')

# 计算F2#1和F2#0的相位差
phase_diff_10, freq_10 = calculate_phase_difference(signal1, signal3, time_seconds)
print(f'Phase difference between F2#1 and F2#0: {phase_diff_10:.2f} degrees at {freq_10:.2f} Hz')

# 计算F2#2和F2#0的相位差
phase_diff_20, freq_20 = calculate_phase_difference(signal2, signal3, time_seconds)
print(f'Phase difference between F2#2 and F2#0: {phase_diff_20:.2f} degrees at {freq_20:.2f} Hz')
