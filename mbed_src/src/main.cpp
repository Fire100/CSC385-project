/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "globals.hpp"
#include "Callback.h"
#include "recordAudio.hpp"
#include "bluetooth_handler.hpp"
#include "bluetooth_handler_client.hpp"
#include "voice_service_server.hpp"
#include "globals.hpp"
#include "voice_service_client.hpp"
#include "voice_service.hpp"
#include "mbed_trace.h"

EventQueue mainQueue;
USBAudio* audio = new USBAudio(true, wavFreq, 1, wavFreq);
VoiceService* voiceService;

int main()
{
    // mbed_trace_init();
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
        printf("OK Audio Init\t(Audio Freq=%ld) (num_seconds=%f)\r\n", wavFreq, num_seconds);
    }

    if (CLIENT){
        voiceService = new VoiceServiceClient();

        mainQueue.call_every(4000ms, callback(voiceService, &VoiceService::sendAudio));
        //record_audio();
        mainQueue.call_every(4000ms, callback(voiceService, &VoiceService::sendAudio));
        init_bluetooth_client();
    }
    else{
        voiceService = new VoiceServiceServer();
        mainQueue.call_every(4000ms, callback(voiceService, &VoiceService::sendAudio));
    
        //record_audio();
        mainQueue.call_every(4000ms, callback(voiceService, &VoiceService::sendAudio));
        init_bluetooth();
    }
 }

