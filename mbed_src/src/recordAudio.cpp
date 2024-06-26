#include "recordAudio.hpp"
#include "voice_service.hpp"
#include <cstring>
Timer t;
bool compressionOn = true;


InterruptIn button(BUTTON1);

// callback that gets invoked when TARGET_AUDIO_BUFFER is full
void target_audio_buffer_full() {
    printf("about to start\n");
    int32_t ret = BSP_AUDIO_IN_Pause(AUDIO_INSTANCE);
    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Pause (%d)\n", ret);
    }
    else {
        printf("OK Audio Pause\n");
    }    


    if (compressionOn) {
        for (size_t ix = 0; ix < TARGET_AUDIO_BUFFER_NB_SAMPLES; ix++) {
            compressedBuf[ix] = DIO_LinearToALaw(TARGET_AUDIO_BUFFER[ix]);
        }
       
    }

    //might need to to multithreaded locking for updating this node.
    dataUpdated = true;

    // create WAV file
    if (compressionOn){
        memcpy(sendBuf, compressedBuf, TARGET_AUDIO_BUFFER_NB_SAMPLES);
        voiceService->sendAudioQueue((uint8_t*)sendBuf, TARGET_AUDIO_BUFFER_NB_SAMPLES);
    } else {
        voiceService->sendAudioQueue((uint8_t*)TARGET_AUDIO_BUFFER, TARGET_AUDIO_BUFFER_NB_SAMPLES * 2);
    }
    

    TARGET_AUDIO_BUFFER_IX = 0; // reset audio buffer idx to begin recording agiai
  
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

void set_sending_audio(){
    if (!voiceService->sending_audio && !voiceService->receiving_audio){
        BSP_AUDIO_IN_Resume(AUDIO_INSTANCE);
    }
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

    ret = BSP_AUDIO_IN_Pause(AUDIO_INSTANCE);
    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Pause (%d)\n", ret);
    }
    else {
        printf("OK Audio Pause\n");
    }    

    button.fall(set_sending_audio);

}