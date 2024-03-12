#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>


#ifndef VOICE_SERVICE
#define VOICE_SERVICE

class VoiceService{

    public:

        static const uint32_t VOICESERVICE_UUID = 0xB000;
        static const uint32_t VOICESERVICE_START_UUID = 0xB001;
        static const uint32_t VOICESERVICE_RECIEVE_AUDIO_UUID = 0xB002;
        static const uint32_t VOICESERVICE_SEND_AUDIO_UUID = 0xB003;

        VoiceService();

        void sendAudio();

        void recieveAudio();
    
    private:
        ReadOnlyGattCharacteristic<int16_t> VOICESERVICE_START;

        ReadOnlyGattCharacteristic<uint16_t> VOICESERVICE_RECIEVE_AUDIO;

        ReadOnlyGattCharacteristic<uint16_t> VOICESERVICE_SEND_AUDIO;
};


#endif // VOICE_SERVICE