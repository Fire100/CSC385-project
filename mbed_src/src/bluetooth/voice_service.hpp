#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include "globals.hpp"
#include "companders.h"

#ifndef VOICE_SERVICE
#define VOICE_SERVICE


class VoiceService {
    protected:
        uint8_t* play_audio_data;
        uint32_t play_audio_size;
        bool play_audio_decompress;
        bool play_audio_new;
        void playAudioHelper(){
            if (play_audio_decompress){
                decompressAudio(play_audio_data, play_audio_size);
            }

            while (play_audio_new && !audio->write(play_audio_data, play_audio_size)){
                audio->write_wait_ready();
            }

            if (audio->write_underflows(true) != 0){
                // printf("Playing audio caused an underflow\n");
            }

            if (play_audio_decompress){
                free(play_audio_data);
            }

            play_audio_new = false;
        }

        uint8_t* send_audio_data;
        uint32_t send_audio_size;

        virtual void sendAudio() = 0;

        void decompressAudio(uint8_t* audio_data, uint32_t size){
            uint16_t* new_play_audio_data = (uint16_t*)malloc(size * sizeof(uint16_t));
            for (size_t ix = 0; ix < size; ix++) {
                new_play_audio_data[ix] = DIO_ALawToLinear(audio_data[ix]);
            }
            play_audio_data = (uint8_t*)new_play_audio_data;
            play_audio_size *= 2;
        }



    public:
        static const uint32_t VOICESERVICE_UUID = 0xB000;
        static const uint32_t VOICESERVICE_START_UUID = 0xB001;
        static const uint32_t VOICESERVICE_RECEIVE_AUDIO_UUID = 0xB002;
        static const uint32_t VOICESERVICE_SEND_AUDIO_UUID = 0xB003;
        static const uint32_t AUDIO_TRANSFER_SIZE = 200; // 1024
        int searchForChar;


        void sendAudioQueue(uint8_t* audio_data, uint32_t size){
            send_audio_data = audio_data;
            send_audio_size = size;
            mainQueue.call(this, &VoiceService::sendAudio);
        }

        void playAudio(uint8_t* audio_data, uint32_t size, bool decompress = true) {
            play_audio_data = audio_data;
            play_audio_size = size;
            play_audio_decompress = decompress;
            play_audio_new = true;
            mainQueue.call(this, &VoiceService::playAudioHelper);
        }
};



extern VoiceService* voiceService;
#endif // VOICE_SERVICE
