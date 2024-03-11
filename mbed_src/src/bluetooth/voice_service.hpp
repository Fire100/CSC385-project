#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"


#ifndef VOICE_SERVICE
#define VOICE_SERVICE

class VoiceService{

    public:

        static const uint32_t VOICESERVICE_UUID = 0xB000;
        static const uint32_t VOICESERVICE_START_UUID = 0xB001;
        static const uint32_t VOICESERVICE_AUDIO_UUID = 0xB002;


        VoiceService();

    
    private:
        ReadOnlyGattCharacteristic<int16_t> VOICESERVICE_START;

        ReadOnlyGattCharacteristic<uint16_t> VOICESERVICE_AUDIO;
};


#endif // VOICE_SERVICE