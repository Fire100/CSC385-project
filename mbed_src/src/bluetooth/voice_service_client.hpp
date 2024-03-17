#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include "recordAudio.hpp"
#include "voice_service.hpp"

#ifndef VOICE_SERVICE_CLIENT
#define VOICE_SERVICE_CLIENT

class VoiceServiceClient : public VoiceService {

    public:

        // might want to add these constants to a seperate file later to ensure consistency between the server and client.
        static const uint32_t VOICESERVICE_UUID = 0xB000;
        static const uint32_t VOICESERVICE_START_UUID = 0xB001;
        static const uint32_t VOICESERVICE_SEND_AUDIO_UUID = 0xB002;
        static const uint32_t VOICESERVICE_RECEIVE_AUDIO_UUID = 0xB003;

        static const uint32_t AUDIO_TRANSFER_SIZE = 1; // 1024

        VoiceServiceClient();
        ~VoiceServiceClient() {};

        void start(BLE &ble, events::EventQueue &event_queue);

        void start_discovery(BLE &ble, events::EventQueue &event_queue, const ble::ConnectionCompleteEvent &event);

        void sendAudio(uint8_t* audio_data, uint32_t size) override;
        

        // virtual void onDataRead(const GattReadCallbackParams &params) override;

        // virtual void onDataWritten(const GattWriteCallbackParams &params) override;

        // virtual void onDataSent(const GattDataSentCallbackParams &params) override;
        
        
    
    private:
        // ReadOnlyGattCharacteristic<uint8_t> *VOICESERVICE_START;
        // uint8_t _voiceservice_start_value = 0;

        
        uint8_t _voiceservice_receive_audio_value = 0;

        
        uint8_t _voiceservice_send_audio_value = 0;

        int currentDataSent = 0;
};

// have to define these outside the class or else there are linker errors
static DiscoveredCharacteristic VOICESERVICE_RECEIVE_AUDIO;
static bool voiceservice_receive_audio_found = false;

static DiscoveredCharacteristic VOICESERVICE_SEND_AUDIO;
static bool voiceservice_send_audio_found = false;

void on_write(const GattWriteCallbackParams *response);
void service_discovery(const DiscoveredService *service);
void characteristic_discovery(const DiscoveredCharacteristic *characteristic) ;


#endif // VOICE_SERVICE