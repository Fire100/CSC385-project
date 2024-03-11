#include "gap_handler.hpp"
#include "voice_service.hpp"
#include "ble/BLE.h"
#include "ble/Gap.h"

VoiceService::VoiceService() :
        VOICESERVICE_START(VoiceService::VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        VOICESERVICE_AUDIO(VoiceService::VOICESERVICE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
{
    // Add this new service. This should only happen once.
    BLE &ble = BLE::Instance();
    GattCharacteristic *characteristics[] = {
        &VOICESERVICE_START,
        &VOICESERVICE_AUDIO,
    };

    GattService voice_service(
        VoiceService::VOICESERVICE_UUID,
        characteristics,
        sizeof(characteristics) / sizeof(characteristics[0])
    );
    ble.gattServer().addService(voice_service);



    // This is test code. remove this when audio setup is done
    uint16_t start = 12;
    uint16_t audio = 30;
    ble::GattServer& server = ble.gattServer();
    server.write(VOICESERVICE_START.getValueHandle(), (uint8_t*)&start, sizeof(start));
    server.write(VOICESERVICE_AUDIO.getValueHandle(), (uint8_t*)&audio, sizeof(audio));
}
