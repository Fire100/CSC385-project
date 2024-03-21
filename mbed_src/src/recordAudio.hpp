#pragma once
#include "companders.h"
#include "mbed.h"
#include "stm32l475e_iot01_audio.h"
#include "companders.h"
#include <cstdint>
#include "globals.hpp"

// # of seconds of audio to record before sending through Bluetooth
// Can change these values to find optimal sound packet size
static const size_t wavFreq = AUDIO_SAMPLING_FREQUENCY/2;
static const float num_seconds = 1;
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
static const size_t TARGET_AUDIO_BUFFER_NB_SAMPLES = wavFreq * num_seconds;
static int16_t *TARGET_AUDIO_BUFFER = (int16_t*)calloc(TARGET_AUDIO_BUFFER_NB_SAMPLES, sizeof(int16_t));
static size_t TARGET_AUDIO_BUFFER_IX = 0;

static size_t dataSize = (TARGET_AUDIO_BUFFER_NB_SAMPLES * sizeof(int16_t)/sizeof(uint8_t));
static size_t fileSize = 44 + dataSize;

static BSP_AUDIO_Init_t MicParams;




static uint8_t compressedBuf[TARGET_AUDIO_BUFFER_NB_SAMPLES];
static uint8_t sendBuf[TARGET_AUDIO_BUFFER_NB_SAMPLES];
static bool dataUpdated = false;
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
void target_audio_buffer_full();

/**
* @brief  Half Transfer user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance);

/**
* @brief  Transfer Complete user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance);

/**
  * @brief  Manages the BSP audio in error event.
  * @param  Instance Audio in instance.
  * @retval None.
  */
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance);


void print_audio();

void record_audio();