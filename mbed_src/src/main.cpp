/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "globals.hpp"
#include "recordAudio.hpp"
#include "bluetooth_handler.hpp"
#include "bluetooth_handler_client.hpp"
#include "globals.hpp"

EventQueue mainQueue;
USBAudio* audio = new USBAudio(true, wavFreq, 1, wavFreq);

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


    

    // record_audio();
    // mainQueue.call_every(2000ms, print_audio);
    // this is just here for testing, don't uncomment otherwise
    // mainQueue.dispatch_forever();

    
    // init_bluetooth();
    init_bluetooth_client();
 }

