/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "stm32l475e_iot01_audio.h"
#include "companders.h"
#include <cstdint>
#include "mainQueue.hpp"
#include "bluetooth_handler.hpp"


EventQueue mainQueue;


// # of seconds of audio to record before sending through Bluetooth
// Can change these values to find optimal sound packet size
const float num_seconds = 1;
static size_t wavFreq = AUDIO_SAMPLING_FREQUENCY;
// NOTE: too high of above values causes there to be not enough memory


// DO NOT CHANGE BELOW

const int BITS_PER_SAMPLE = 16; // This needs to correspond to PCM_buffer's type. I think 16 is the only value supported, so don't change.
// Sound data is normally read in pairs of 2 bytes (16 bits). PCM_BUFFER_LEN is how many bytes of data the BSP stores naturally.
// The BSP normally reads PCM_BUFFER_LEN/2 bytes to fill up the PCM_Buffer. It does this twice, one for each half of the PCM_Buffer array.
// This is why we have 2 callbacks, one for each half. 
const int PCM_Buffer_arr_len = PCM_BUFFER_LEN / sizeof(int16_t); // Want the buffer arr length to be equal to PCM_BUFFER_LEN bytes
static uint16_t PCM_Buffer[PCM_Buffer_arr_len];
// Want this thing below to be half of the length of PCM_Buffer, but not in bytes. 
// This is because we use this to determine where to set the second half of the sound data in PCM_Buffer, so we can do PCM_Buffer + nb_samples
const uint32_t nb_samples = PCM_Buffer_arr_len / 2; 


// To generate num_seconds worth of audio, calculate how big an array we need
static size_t TARGET_AUDIO_BUFFER_NB_SAMPLES = wavFreq * num_seconds;
static int16_t *TARGET_AUDIO_BUFFER = (int16_t*)calloc(TARGET_AUDIO_BUFFER_NB_SAMPLES, sizeof(int16_t));
static size_t TARGET_AUDIO_BUFFER_IX = 0;

static size_t dataSize = (TARGET_AUDIO_BUFFER_NB_SAMPLES * sizeof(int16_t)/sizeof(uint8_t));
static size_t fileSize = 44 + dataSize;

static BSP_AUDIO_Init_t MicParams;


Timer t;
bool compressionOn = true;
static uint8_t compressedBuf[16000];
#define PRIu16 "hu"
/*
Compression halves the size of the data

Compression is currently set to store the compressed audio in a seperate smaller buffer (to send over bluetooth)
In order to hear the audio compressed and decompressed for testing, do the following:
- Uncomment the audio decompression code
- Uncomment TARGET_AUDIO_BUFFER[ix] = DIO_LinearToALaw(TARGET_AUDIO_BUFFER[ix]);
- Comment out compressedBuf[ix] = DIO_LinearToALaw(TARGET_AUDIO_BUFFER[ix]);
- Set if (compressionOn) on line 125 to be false all the time (so if (false))
- remove the spaces from the print out for the wav files.
*/

// callback that gets invoked when TARGET_AUDIO_BUFFER is full
void target_audio_buffer_full() {
    // pause audio stream
    int32_t ret = BSP_AUDIO_IN_Pause(AUDIO_INSTANCE);
    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Pause (%d)\n", ret);
    }
    else {
        printf("OK Audio Pause\n");
    }

    t.stop();
    printf("Recording time: %llu ms\n", t.elapsed_time().count());
    t.reset();

    // compression

    t.start();

    if (compressionOn) {
        for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES; ix++) {
            //printf("Decompressed: %hu", TARGET_AUDIO_BUFFER[ix]);
            compressedBuf[ix] = DIO_LinearToALaw(TARGET_AUDIO_BUFFER[ix]);
            // TARGET_AUDIO_BUFFER[ix] = DIO_LinearToALaw(TARGET_AUDIO_BUFFER[ix]);
            // printf("Compressed: %hu ", compressed_buf[ix]);
        }
    }

    t.stop();
    printf("Compression time: %llu ms\n", t.elapsed_time().count());

    // Decompression code
    // compressed audio is incomprehensible
    // if (compressionOn) {
    //     for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES; ix++) {
    //         // printf("Compressed: %hu", TARGET_AUDIO_BUFFER[ix]);
    //         TARGET_AUDIO_BUFFER[ix] = DIO_ALawToLinear(TARGET_AUDIO_BUFFER[ix]);
    //         // printf("Decompressed: %hu", TARGET_AUDIO_BUFFER[ix]);
    //     }
    // }

    // create WAV file

    uint32_t byte_rate = wavFreq * BITS_PER_SAMPLE / 8; // (Sample Rate * BitsPerSample * Channels) / 8
    // TODO: probably don't send this via bluetooth since it's just constant time operations
    uint8_t wav_header[44] = {
        0x52, 0x49, 0x46, 0x46, // RIFF
        (uint8_t) (fileSize & 0xff), (uint8_t)((fileSize >> 8) & 0xff), (uint8_t)((fileSize >> 16) & 0xff), (uint8_t)((fileSize >> 24) & 0xff),
        0x57, 0x41, 0x56, 0x45, // WAVE
        0x66, 0x6d, 0x74, 0x20, // fmt
        0x10, 0x00, 0x00, 0x00, // length of format data
        0x01, 0x00, // type of format (1=PCM)
        0x01, 0x00, // number of channels
        (uint8_t)(wavFreq & 0xff), (uint8_t)((wavFreq >> 8) & 0xff), (uint8_t)((wavFreq >> 16) & 0xff), (uint8_t)((wavFreq >> 24) & 0xff),
        (uint8_t)(byte_rate & 0xff), (uint8_t)((byte_rate >> 8) & 0xff), (uint8_t)((byte_rate >> 16) & 0xff), (uint8_t)((byte_rate >> 24) & 0xff),
        (uint8_t)(BITS_PER_SAMPLE/8 & 0xff), (uint8_t)(BITS_PER_SAMPLE/8 >> 8 & 0xff), // (BITS_PER_SAMPLE * channels) / 8
        (uint8_t)(BITS_PER_SAMPLE & 0xff), (uint8_t)(BITS_PER_SAMPLE >> 8 & 0xff), // BITS_PER_SAMPLE
        0x64, 0x61, 0x74, 0x61, // data
        (uint8_t)(dataSize & 0xff), (uint8_t)((dataSize >> 8) & 0xff), (uint8_t)((dataSize >> 16) & 0xff), (uint8_t)((dataSize >> 24) & 0xff),
    };


    // print both the WAV header and the audio buffer in HEX format to serial
    // you can use the script in `hex-to-buffer.js` to make a proper WAV file again
    printf("WAV file:\n");
    for (size_t ix = 0; ix < 44; ix++) {
        printf("%02x ", wav_header[ix]);
    }

    if (compressionOn) {
        for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES; ix++) {
            printf("%02x ", compressedBuf[ix]);
        }
    }
    else {
        uint8_t *buf = (uint8_t*)TARGET_AUDIO_BUFFER;
        for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES * 2; ix++) {
            printf("%02x ", buf[ix]);
        }
    }
    

    // TODO: Send data in TARGET_AUDIO_BUFFER to bluetooth
    
    

    
    TARGET_AUDIO_BUFFER_IX = 0; // reset audio buffer idx to begin recording agiai
    printf("\n");
}

/**
* @brief  Half Transfer user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance) {
    if ((TARGET_AUDIO_BUFFER_IX + nb_samples) > TARGET_AUDIO_BUFFER_NB_SAMPLES) {
        return;
    }

    /* Copy first half of PCM_Buffer from Microphones onto Fill_Buffer */
    memcpy(TARGET_AUDIO_BUFFER + TARGET_AUDIO_BUFFER_IX, PCM_Buffer, PCM_Buffer_arr_len);
    TARGET_AUDIO_BUFFER_IX += nb_samples;

    if (TARGET_AUDIO_BUFFER_IX >= TARGET_AUDIO_BUFFER_NB_SAMPLES) {
        mainQueue.call(&target_audio_buffer_full);
        return;
    }
}

/**
* @brief  Transfer Complete user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance) {
    if ((TARGET_AUDIO_BUFFER_IX + nb_samples) > TARGET_AUDIO_BUFFER_NB_SAMPLES) {
        return;
    }

    /* Copy second half of PCM_Buffer from Microphones onto Fill_Buffer */
    memcpy(TARGET_AUDIO_BUFFER + TARGET_AUDIO_BUFFER_IX, PCM_Buffer + nb_samples, PCM_Buffer_arr_len);
    TARGET_AUDIO_BUFFER_IX += nb_samples;

    if (TARGET_AUDIO_BUFFER_IX >= TARGET_AUDIO_BUFFER_NB_SAMPLES) {
        mainQueue.call(&target_audio_buffer_full);
        return;
    }
}

/**
  * @brief  Manages the BSP audio in error event.
  * @param  Instance Audio in instance.
  * @retval None.
  */
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance) {
    printf("BSP_AUDIO_IN_Error_CallBack\n");
}


void print_audio(){
    uint8_t *buf = (uint8_t*)PCM_Buffer;
    for(int i = 0; i < PCM_Buffer_arr_len * 2; i++){
        printf("%02x ", buf[i]);
    }
    printf("\n");
}

void record_audio(){


    int32_t ret;
    uint32_t state;

    ret = BSP_AUDIO_IN_GetState(AUDIO_INSTANCE, &state);
    if (ret != BSP_ERROR_NONE) {
        printf("Cannot start recording: Error getting audio state (%d)\n", ret);
        return;
    }
    if (state == AUDIO_IN_STATE_RECORDING) {
        printf("Cannot start recording: Already recording\n");
        return;
    }

    ret = BSP_AUDIO_IN_Record(AUDIO_INSTANCE, (uint8_t *) PCM_Buffer, PCM_BUFFER_LEN);
    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Record (%ld)\n", ret);
        return;
    }
    else {
        printf("OK Audio Record\n");
    }

    t.start();
}



void init_bluetooth(){

    // BluetoothHandler();
    BluetoothHandler* bluetooth = new BluetoothHandler();

    // TODO: add callback to myBluetooth to call "record_audio" once a connection is found
    // bluetooth->onConnect(record_audio);

    // TODO: add a callback on what to do when receiving data 
    // bluetooth->onReceiveData(receive_func);

    // TODO: connect to addresss
    // bluetooth->connectTo(address);
}

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
    mainQueue.dispatch_forever();
 }
