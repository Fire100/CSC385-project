#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include "recordAudio.hpp"
#include <string>


#ifndef VOICE_SERVICE
#define VOICE_SERVICE

class VoiceService : ble::GattServer::EventHandler{

    public:

        static const uint32_t VOICESERVICE_UUID = 0xB000;
        static const uint32_t VOICESERVICE_START_UUID = 0xB001;
        static const uint32_t VOICESERVICE_RECIEVE_AUDIO_UUID = 0xB002;
        static const uint32_t VOICESERVICE_SEND_AUDIO_UUID = 0xB003;
        static const uint32_t VOICESERVICE_RECEIVE_FREQ_UUID = 0xB004;
        static const uint32_t VOICESERVICE_SEND_FREQ_UUID = 0xB005;

        static const uint32_t AUDIO_TRANSFER_SIZE = 1; // 1024

        VoiceService();
        ~VoiceService() {};

        void start();

        void sendAudio();

        virtual void onDataRead(const GattReadCallbackParams &params) override;

        virtual void onDataWritten(const GattWriteCallbackParams &params) override;

        virtual void onDataSent(const GattDataSentCallbackParams &params) override;
        
    
    private:
        inline static const std::string start_name = "Audio Start";
        inline static const std::string received_name = "Recieved Audio";
        inline static const std::string sent_name = "Sent Audio";
        

        ReadOnlyGattCharacteristic<uint8_t> *VOICESERVICE_START;
        uint8_t _voiceservice_start_value = 0;

        WriteOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> *VOICESERVICE_RECEIVE_AUDIO;
        uint8_t _voiceservice_recieve_audio_value = 0;



        

        ReadOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> *VOICESERVICE_SEND_AUDIO;
        uint8_t _voiceservice_send_audio_value = 0;

        int currentDataSent = 0;

        WriteOnlyGattCharacteristic<uint32_t> *VOICESERVICE_RECEIVE_FREQ;
        
        ReadOnlyGattCharacteristic<uint32_t> *VOICESERVICE_SEND_FREQ;
};



#endif // VOICE_SERVICE