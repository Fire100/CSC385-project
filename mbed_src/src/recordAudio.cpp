#include "recordAudio.hpp"
#include "voice_service.hpp"
Timer t;
bool compressionOn = false;

// callback that gets invoked when TARGET_AUDIO_BUFFER is full
void target_audio_buffer_full() {
    // pause audio stream
    // int32_t ret = BSP_AUDIO_IN_Pause(AUDIO_INSTANCE);
    // if (ret != BSP_ERROR_NONE) {
    //     printf("Error Audio Pause (%d)\n", ret);
    // }
    // else {
    //     printf("OK Audio Pause\n");
    // }

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
            // printf("Compressed: %hu ", compressedBuf[ix]);
        }
        printf("Compressed: %hu\n", compressedBuf[12]);

    }

    t.stop();
    printf("Compression time: %llu ms\n", t.elapsed_time().count());

    // Decompression code
    // compressed audio is incomprehensible
    // if (compressionOn) {
    //     for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES; ix++) {
    //         // printf("Compressed: %hu", TARGET_AUDIO_BUFFER[ix]);
    //         TARGET_AUDIO_BUFFER[ix] = DIO_ALawToLinear(compressedBuf[ix]);
    //         // printf("Decompressed: %hu", TARGET_AUDIO_BUFFER[ix]);
    //     }
    //     printf("Decompressed: %hu\n", TARGET_AUDIO_BUFFER[12]);

    // }

    //might need to to multithreaded locking for updating this node.
    dataUpdated = true;

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
    // printf("WAV file:\n");
    // for (size_t ix = 0; ix < 44; ix++) {
    //     printf("%02x ", wav_header[ix]);
    // }

    // if (compressionOn) {
    //     for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES; ix++) {
    //         printf("%02x ", compressedBuf[ix]);
    //     }
    // }
    // else {
    //     uint8_t *buf = (uint8_t*)TARGET_AUDIO_BUFFER;
    //     for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES * 2; ix++) {
    //         printf("%02x ", buf[ix]);
    //     }
    // }
    

    // TODO: Send data in TARGET_AUDIO_BUFFER to bluetooth
    
    if (compressionOn){
        voiceService->playAudio((uint8_t*)compressedBuf, TARGET_AUDIO_BUFFER_NB_SAMPLES, true);
    } else {
        voiceService->playAudio((uint8_t*)TARGET_AUDIO_BUFFER, TARGET_AUDIO_BUFFER_NB_SAMPLES * 2, false);

    }
    

    TARGET_AUDIO_BUFFER_IX = 0; // reset audio buffer idx to begin recording agiai
    printf("\n");
    t.start();
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
