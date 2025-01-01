#include "../include/common.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// 常數定義
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512

// 最大音量縮放，防止剪切
#define MAX_VOLUME 0.25 // 每個通道的最大音量為總音量的四分之一

// 在文件頂部聲明所有函數
double get_frequency_channel1();
double get_volume_channel1();
double get_frequency_channel2();
double get_volume_channel2();
int is_channel3_enabled();
double get_volume_channel3();
int16_t get_waveform_sample(int index);
double get_volume_channel4();
void audio_callback(void* userdata, Uint8* stream, int len);
void reset_audio_registers();
void disable_channel4();

// 音頻寄存器結構體定義（假設在 common.h 中定義）

// 音頻寄存器實例
static AudioRegisters audio_regs;

// SDL 音頻設備
static SDL_AudioDeviceID audio_device;

// 通道生成參數
static double phase_channel1 = 0.0;
static double phase_channel2 = 0.0;
static double phase_channel3 = 0.0;

// 通道4 噪聲生成的 LFSR
static uint16_t lfsr = 0x7FFF; // 初始值，15位

// 通道4 定時器和當前噪聲樣本
static double channel4_timer = 0.0;
static int16_t current_noise_sample = 0;

// 長度計時器累加器
static double channel4_length_accumulator = 0.0;

// 初始化音頻系統
int audio_init() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        //printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);

    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = AUDIO_S16SYS; // 16-bit signed audio
    desired_spec.channels = 1;          // 單聲道
    desired_spec.samples = BUFFER_SIZE;
    desired_spec.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, NULL, 0);
    if (audio_device == 0) {
        //printf("Failed to open audio device: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    reset_audio_registers();                   // 初始化寄存器和狀態
    SDL_PauseAudioDevice(audio_device, 0);     // 開始播放
    return 0;
}

// 清理音頻系統
void audio_cleanup() {
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}

// 重置音頻寄存器和相關狀態
void reset_audio_registers() {
    memset(&audio_regs, 0, sizeof(AudioRegisters));
    phase_channel1 = 0.0;
    phase_channel2 = 0.0;
    phase_channel3 = 0.0;
    lfsr = 0x7FFF;          // 重置 LFSR 初始值為15位
    channel4_timer = 0.0;   // 重置通道4定時器
    current_noise_sample = 0;
    channel4_length_accumulator = 0.0; // 重置長度計時器累加器
}

// 禁用通道4
void disable_channel4() {
    audio_regs.NR44 &= ~0x80; // 清除bit7，禁用通道4
    audio_regs.NR44 &= ~0x40; // 禁用長度使能
    channel4_timer = 0.0;     // 重置定時器
    current_noise_sample = 0; // 重置噪聲樣本
    channel4_length_accumulator = 0.0; // 重置長度計時器累加器

    // 添加日誌
    //printf("Channel4 已禁用: NR44=0x%02X, channel4_timer=%.2f, channel4_length_accumulator=%.2f\n",
    //        audio_regs.NR44, channel4_timer, channel4_length_accumulator);
}

// 通道4 噪聲生成（基於 LFSR 和定時器）
int16_t generate_noise_sample() {
    // 提取 NR43 中的參數
    uint8_t clock_shift = audio_regs.NR43 & 0x07;           // Bits 0-2: Clock Shift (s)
    uint8_t polynomial = (audio_regs.NR43 >> 3) & 0x01;    // Bit 3: Polynomial Type (w)
    uint8_t divisor_code = (audio_regs.NR43 >> 4) & 0x07;  // Bits 4-6: Divisor Code (d)

    // 添加日誌
    //printf("NR43 設定: clock_shift=%d, polynomial=%d, divisor_code=%d\n", clock_shift, polynomial, divisor_code);

    // 計算有效時鐘頻率
    static const uint_fast8_t divisor_lut[] = {8, 16, 24, 32, 40, 48, 56, 64};
    if (divisor_code >= sizeof(divisor_lut)/sizeof(divisor_lut[0])) {
        divisor_code = 0; // 防止越界
    }
    if (clock_shift > 15) { // 假設 clock_shift 不應超過 15
        clock_shift = 15;
    }
    double frequency = 524288.0 / ((double)divisor_lut[divisor_code] * (double)(1 << clock_shift));

    // 添加日誌
    //printf("計算頻率: frequency=%.2f Hz, samples_per_update=%.2f\n", frequency, SAMPLE_RATE / frequency);

    // 計算每次噪聲更新所需的採樣數
    double samples_per_update = SAMPLE_RATE / frequency;

    // 更新定時器
    channel4_timer -= 1.0;

    // 使用 while 迴圈處理可能的多次更新
    while (channel4_timer <= 0.0) {
        // 更新 LFSR
        uint16_t bit;
        if (polynomial) {
            // 15-bit LFSR: x^15 + x^14 + 1
            bit = (lfsr ^ (lfsr >> 1)) & 0x01;
            lfsr = (lfsr >> 1) | (bit << 14); // 將 bit 放到最高位
            lfsr &= 0x7FFF; // 保持15位
            if (lfsr == 0) lfsr = 0x7FFF; // 防止全0
        } else {
            // 7-bit LFSR: x^7 + x^6 + 1
            bit = (lfsr ^ (lfsr >> 1)) & 0x01;
            lfsr = (lfsr >> 1) | (bit << 6); // 將 bit 放到最高位
            lfsr &= 0x007F; // 保持7位
            if (lfsr == 0) lfsr = 0x007F; // 防止全0
        }

        // 更新當前噪聲樣本
        current_noise_sample = (bit ? 32767 : -32767);

        // 增加定時器
        channel4_timer += samples_per_update;

        // 添加調試日誌
        //printf("LFSR: 0x%04X, Bit: %d, Sample: %d, Timer: %.2f\n", lfsr, bit, current_noise_sample, channel4_timer);
    }

    return current_noise_sample;
}

// 獲取通道1的頻率
double get_frequency_channel1() {
    uint16_t freq = (audio_regs.NR13 | ((audio_regs.NR14 & 0x07) << 8));
    if (freq == 2048) return 0;
    return 131072.0 / (2048 - freq);
}

// 獲取通道1的音量
double get_volume_channel1() {
    uint8_t envelope = (audio_regs.NR12 & 0xF0) >> 4;
    return (envelope / 15.0) * MAX_VOLUME;
}

// 獲取通道2的頻率
double get_frequency_channel2() {
    uint16_t freq = (audio_regs.NR23 | ((audio_regs.NR24 & 0x07) << 8));
    if (freq == 2048) return 0;
    return 131072.0 / (2048 - freq);
}

// 獲取通道2的音量
double get_volume_channel2() {
    uint8_t envelope = (audio_regs.NR22 & 0xF0) >> 4;
    return (envelope / 15.0) * MAX_VOLUME;
}

// 獲取通道3的頻率
double get_frequency_channel3() {
    uint16_t freq = (audio_regs.NR33 | ((audio_regs.NR34 & 0x07) << 8));
    if (freq == 2048) return 0;
    return 131072.0 / (2048 - freq);
}

// 檢查通道3是否啟用
int is_channel3_enabled() {
    return (audio_regs.NR30 & 0x80) != 0;
}

// 獲取通道3的音量
double get_volume_channel3() {
    uint8_t volume_code = (audio_regs.NR32 & 0x60) >> 5; // 2 位音量碼
    switch (volume_code) {
        case 0: return 0.0;
        case 1: return (1.0 / 4.0) * MAX_VOLUME;
        case 2: return (1.0 / 2.0) * MAX_VOLUME;
        case 3: return 1.0 * MAX_VOLUME;
        default: return 0.0;
    }
}

// 獲取通道3的波形樣本
int16_t get_waveform_sample(int index) {
    static const int16_t waveform[32] = {
        0, 1, 0, -1, 0, 1, 0, -1,
        1, 1, 1, -1, -1, -1, -1, 1,
        0, 0, 0, 0, 1, 1, 1, -1,
        -1, -1, -1, -1, 0, 0, 0, 0
    };
    return waveform[index % 32] * 8192; // 放大波形幅度以避免過低音量
}

// 獲取通道4的音量
double get_volume_channel4() {
    uint8_t envelope = (audio_regs.NR42 & 0xF0) >> 4;
    return (envelope / 15.0) * MAX_VOLUME;
}

// 音頻回調函數
void audio_callback(void* userdata, Uint8* stream, int len) {
    int16_t* buffer = (int16_t*)stream;
    int samples = len / sizeof(int16_t);
    memset(buffer, 0, len); // 初始化緩衝區

    // 添加日誌
    //printf("---- Audio Callback Start ----\n");
    //printf("NR41: 0x%02X, NR42: 0x%02X, NR43: 0x%02X, NR44: 0x%02X\n",
    //        audio_regs.NR41, audio_regs.NR42, audio_regs.NR43, audio_regs.NR44);
    //printf("channel4_timer: %.2f, channel4_length_accumulator: %.2f\n",
    //        channel4_timer, channel4_length_accumulator);
    //printf("LFSR: 0x%04X, current_noise_sample: %d\n", lfsr, current_noise_sample);

    // 使用臨時的32位緩衝區進行混音
    int32_t temp_buffer[samples];
    memset(temp_buffer, 0, sizeof(temp_buffer));

    // 通道1：方波生成
    if (audio_regs.NR14 & 0x80) { // 檢查是否啟用通道
        double frequency1 = get_frequency_channel1();
        double volume1 = get_volume_channel1();
        for (int i = 0; i < samples; i++) {
            temp_buffer[i] += (int32_t)(volume1 * 32767 * (((int)(phase_channel1 / M_PI) % 2) ? 1 : -1));
            phase_channel1 += 2.0 * M_PI * frequency1 / SAMPLE_RATE;
            if (phase_channel1 >= 2.0 * M_PI) phase_channel1 -= 2.0 * M_PI;
        }
    }

    // 通道2：方波生成
    if (audio_regs.NR24 & 0x80) { // 檢查是否啟用通道
        double frequency2 = get_frequency_channel2();
        double volume2 = get_volume_channel2();
        for (int i = 0; i < samples; i++) {
            temp_buffer[i] += (int32_t)(volume2 * 32767 * (((int)(phase_channel2 / M_PI) % 2) ? 1 : -1));
            phase_channel2 += 2.0 * M_PI * frequency2 / SAMPLE_RATE;
            if (phase_channel2 >= 2.0 * M_PI) phase_channel2 -= 2.0 * M_PI;
        }
    }

    // 通道3：波形生成器
    if (is_channel3_enabled()) {
        double frequency3 = get_frequency_channel3();
        double volume3 = get_volume_channel3();
        for (int i = 0; i < samples; i++) {
            int wave_index = (int)(phase_channel3 / (2.0 * M_PI) * 32) % 32;
            temp_buffer[i] += (int32_t)(volume3 * get_waveform_sample(wave_index));
            phase_channel3 += 2.0 * M_PI * frequency3 / SAMPLE_RATE;
            if (phase_channel3 >= 2.0 * M_PI) phase_channel3 -= 2.0 * M_PI;
        }
    }

    // 通道4：噪聲生成
    if (audio_regs.NR44 & 0x80) { // 檢查是否啟用通道
        double volume4 = get_volume_channel4();
        if (volume4 > 0) { // 僅在音量 > 0 時生成噪聲
            for (int i = 0; i < samples; i++) {
                temp_buffer[i] += (int32_t)(volume4 * generate_noise_sample());
            }
        }
        // 處理長度計時器
        if (audio_regs.NR44 & 0x40) { // 檢查長度使能位
            channel4_length_accumulator += samples;
            double samples_per_decrement = SAMPLE_RATE / 256.0; // ~172.2656

            while (channel4_length_accumulator >= samples_per_decrement) {
                if (audio_regs.NR41 > 0) {
                    audio_regs.NR41--;
                    //printf("NR41 decremented to %d\n", audio_regs.NR41); // 添加日誌
                    if (audio_regs.NR41 == 0) { // 當長度計時器到期時停止通道
                        disable_channel4();
                        //printf("Channel4 disabled: NR41=0\n"); // 添加日誌
                    }
                }
                channel4_length_accumulator -= samples_per_decrement;
            }
        }
    }

    // 混音歸一化，避免剪切
    for (int i = 0; i < samples; i++) {
        if (temp_buffer[i] > 32767) temp_buffer[i] = 32767;
        else if (temp_buffer[i] < -32768) temp_buffer[i] = -32768;
        buffer[i] = (int16_t)temp_buffer[i];
    }

    // 添加日誌
    //printf("---- Audio Callback End ----\n\n");
}

// 讀取音頻寄存器
uint8_t audio_read(uint16_t address) {
    switch (address) {
        case 0xFF10: return audio_regs.NR10;
        case 0xFF11: return audio_regs.NR11;
        case 0xFF12: return audio_regs.NR12;
        case 0xFF13: return audio_regs.NR13;
        case 0xFF14: return audio_regs.NR14;
        case 0xFF16: return audio_regs.NR21;
        case 0xFF17: return audio_regs.NR22;
        case 0xFF18: return audio_regs.NR23;
        case 0xFF19: return audio_regs.NR24;
        case 0xFF1A: return audio_regs.NR30;
        case 0xFF1B: return audio_regs.NR31;
        case 0xFF1C: return audio_regs.NR32;
        case 0xFF1D: return audio_regs.NR33;
        case 0xFF1E: return audio_regs.NR34;
        case 0xFF20: return audio_regs.NR41;
        case 0xFF21: return audio_regs.NR42;
        case 0xFF22: return audio_regs.NR43;
        case 0xFF23: return (audio_regs.NR44 & 0xC0) | (audio_regs.NR41 & 0x3F); // 包含長度位
        default: return 0xFF;
    }
}

// 寫入音頻寄存器
void audio_write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF10: 
            audio_regs.NR10 = value; 
            //printf("NR10 寫入: 0x%02X\n", value);
            break;
        case 0xFF11: 
            audio_regs.NR11 = value; 
            //printf("NR11 寫入: 0x%02X\n", value);
            break;
        case 0xFF12: 
            audio_regs.NR12 = value; 
            //printf("NR12 寫入: 0x%02X\n", value);
            break;
        case 0xFF13: 
            audio_regs.NR13 = value; 
            //printf("NR13 寫入: 0x%02X\n", value);
            break;
        case 0xFF14: 
            audio_regs.NR14 = value; 
            //printf("NR14 寫入: 0x%02X\n", value);
            if (value & 0x80) phase_channel1 = 0.0; 
            break;
        case 0xFF16: 
            audio_regs.NR21 = value; 
            //printf("NR21 寫入: 0x%02X\n", value);
            break;
        case 0xFF17: 
            audio_regs.NR22 = value; 
            //printf("NR22 寫入: 0x%02X\n", value);
            break;
        case 0xFF18: 
            audio_regs.NR23 = value; 
            //printf("NR23 寫入: 0x%02X\n", value);
            break;
        case 0xFF19: 
            audio_regs.NR24 = value; 
            //printf("NR24 寫入: 0x%02X\n", value);
            if (value & 0x80) phase_channel2 = 0.0; 
            break;
        case 0xFF1A: 
            audio_regs.NR30 = value; 
            //printf("NR30 寫入: 0x%02X\n", value);
            break;
        case 0xFF1B: 
            audio_regs.NR31 = value; 
            //printf("NR31 寫入: 0x%02X\n", value);
            break;
        case 0xFF1C: 
            audio_regs.NR32 = value; 
            //printf("NR32 寫入: 0x%02X\n", value);
            break;
        case 0xFF1D: 
            audio_regs.NR33 = value; 
            //printf("NR33 寫入: 0x%02X\n", value);
            break;
        case 0xFF1E: 
            audio_regs.NR34 = value; 
            //printf("NR34 寫入: 0x%02X\n", value);
            break;
        case 0xFF20: 
            audio_regs.NR41 = value; 
            //printf("NR41 寫入: 0x%02X\n", value);
            break;
        case 0xFF21: 
            audio_regs.NR42 = value; 
            //printf("NR42 寫入: 0x%02X\n", value);
            break;
        case 0xFF22: 
            audio_regs.NR43 = value; 
            //printf("NR43 寫入: 0x%02X\n", value);
            break;
        case 0xFF23:
            audio_regs.NR44 = value;
            //printf("NR44 寫入: 0x%02X\n", value);

            // 處理觸發位（bit 7）
            if (value & 0x80) { 
                audio_regs.NR41 &= 0x3F; // 重置長度計時器
                // 根據多項式類型初始化 LFSR
                if (audio_regs.NR43 & 0x08) { // 多項式類型位（bit 3）
                    lfsr = 0x7FFF; // 15位 LFSR
                } else {
                    lfsr = 0x007F; // 7位 LFSR
                }
                channel4_timer = 0.0;    // 重置通道4定時器
                current_noise_sample = 0; // 重置當前噪聲樣本
                // 移除以下行，因為 audio_regs.NR44 已經被賦值
                // audio_regs.NR44 |= 0x80; // 設置通道為播放狀態
                //printf("Channel4 triggered: NR41=0x%02X, LFSR=0x%04X\n", audio_regs.NR41, lfsr);
            }

            // 處理長度使能位（bit 6）
            if (value & 0x40) {
                audio_regs.NR44 |= 0x40; // 啟用長度檢查
                //printf("Length enable set for Channel4\n");
            } else {
                audio_regs.NR44 &= ~0x40; // 禁用長度檢查
                //printf("Length enable cleared for Channel4\n");
            }
            break;

        default: 
            //printf("Unknown audio register write: 0x%04X = 0x%02X\n", address, value);
            break;
    }

    // 如果需要，可以記錄其他寄存器的寫入
}
