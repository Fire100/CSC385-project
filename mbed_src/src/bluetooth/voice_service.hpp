#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include "recordAudio.hpp"


#ifndef VOICE_SERVICE
#define VOICE_SERVICE

class VoiceService : ble::GattServer::EventHandler{

    public:

        static const uint32_t VOICESERVICE_UUID = 0xB000;
        static const uint32_t VOICESERVICE_START_UUID = 0xB001;
        static const uint32_t VOICESERVICE_RECIEVE_AUDIO_UUID = 0xB002;
        static const uint32_t VOICESERVICE_SEND_AUDIO_UUID = 0xB003;

        VoiceService();
        ~VoiceService() {};

        void start(BLE &ble, events::EventQueue &event_queue);

        void sendAudio();

        virtual void onDataRead(const GattReadCallbackParams &params) override;

        virtual void onDataWritten(const GattWriteCallbackParams &params) override;

        virtual void onDataSent(const GattDataSentCallbackParams &params) override;
        
    
    private:
        ReadOnlyGattCharacteristic<uint8_t> *VOICESERVICE_START;
        uint8_t _voiceservice_start_value = 0;

        ReadOnlyGattCharacteristic<uint8_t> *VOICESERVICE_RECIEVE_AUDIO;
        uint8_t _voiceservice_recieve_audio_value = 0;

        ReadOnlyGattCharacteristic<uint8_t> *VOICESERVICE_SEND_AUDIO;
        uint8_t _voiceservice_send_audio_value = 0;

        int currentDataSent = 0;
};



#endif // VOICE_SERVICE