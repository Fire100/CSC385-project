/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "globals.hpp"
#include "recordAudio.hpp"
#include "bluetooth_handler.hpp"
#include "bluetooth_handler_client.hpp"
#include "voice_service_server.hpp"
#include "globals.hpp"
#include "voice_service_client.hpp"
#include "voice_service.hpp"


EventQueue mainQueue;
USBAudio* audio = new USBAudio(true, wavFreq, 1, wavFreq);
VoiceService* voiceService;

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

    if (CLIENT){
        voiceService = new VoiceServiceClient();
        record_audio();
        init_bluetooth_client();
    }
    else{
        voiceService = new VoiceServiceServer();
        record_audio();
        init_bluetooth();
    }
 }

