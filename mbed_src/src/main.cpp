/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

// #include "mbed.h"
// #include "stm32l475e_iot01_audio.h"
// #include "companders.h"
// #include <cstdint>
// #include "mainQueue.hpp"
#include "recordAudio.hpp"
#include "bluetooth_handler.hpp"
#include "mainQueue.hpp"

EventQueue mainQueue;
int main()
{
    printf("Hello from microphone demo\n");

    // set up the microphone
    MicParams.BitsPerSample = BITS_PER_SAMPLE;
    MicParams.ChannelsNbr = AUDIO_CHANNELS;
    MicParams.Device = AUDIO_IN_DIGITAL_MIC1;
    MicParams.SampleRate = wavFreq;
    MicParams.Volume = 32;

    int32_t ret = BSP_AUDIO_IN_Init(AUDIO_INSTANCE, &MicParams);

    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Init (%ld)\r\n", ret);
        return 1;
    } else {
        printf("OK Audio Init\t(Audio Freq=%ld)\r\n", wavFreq);
    }

    // mainQueue.call_every(2000ms, print_audio);

    
    init_bluetooth();
 }
