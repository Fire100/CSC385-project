#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include "recordAudio.hpp"
#include <string>
#include "voice_service.hpp"

#ifndef VOICE_SERVICE_SERVER
#define VOICE_SERVICE_SERVER



class VoiceServiceServer : ble::GattServer::EventHandler, public VoiceService {

    public:
        VoiceServiceServer();
        ~VoiceServiceServer() {};

        void start();

        void sendAudio() override;

        virtual void onDataWritten(const GattWriteCallbackParams &params) override;

        virtual void onDataSent(const GattDataSentCallbackParams &params) override;

        virtual void onAttMtuChange	(ble::connection_handle_t connectionHandle, uint16_t attMtuSize) override;	
        
    
    private:
        inline static const std::string start_name = "Audio Start";
        inline static const std::string received_name = "Recieved Audio";
        inline static const std::string sent_name = "Sent Audio";


        Timer t;

        uint8_t audio_buffer[8000];
        int audio_buffer_idx = 0;

        

        ReadOnlyGattCharacteristic<uint8_t> *VOICESERVICE_START;
        uint8_t _voiceservice_start_value = 0;

        ReadWriteArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> *VOICESERVICE_RECEIVE_AUDIO;
        uint8_t _voiceservice_recieve_audio_value = 0;
        
        ReadWriteArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> *VOICESERVICE_SEND_AUDIO;
        uint8_t _voiceservice_send_audio_value = 0;

        int currentDataSent = 0;
};



#endif // VOICE_SERVICE_SERVER