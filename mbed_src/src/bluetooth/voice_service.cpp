#include "gap_handler.hpp"
#include "voice_service.hpp"
#include "ble/BLE.h"
#include "ble/Gap.h"

GattAttribute nameDescrStart( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC, (uint8_t *)"Audio Start", strlen("Audio Start"));
GattAttribute *descriptorsStart[] = {&nameDescrStart};

GattAttribute nameDescrRecieveAudio( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC, (uint8_t *)"Recieved Audio", strlen("Recieved Audio"));
GattAttribute *descriptorsRecievedAudio[] = {&nameDescrRecieveAudio};

GattAttribute nameDescrSentAudio( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC, (uint8_t *)"Sent Audio", strlen("Sent Audio"));
GattAttribute *descriptorsSentAudio[] = {&nameDescrSentAudio};

VoiceService::VoiceService() :
        VOICESERVICE_START(VoiceService::VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY ),
        VOICESERVICE_RECIEVE_AUDIO(VoiceService::VOICESERVICE_RECIEVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        VOICESERVICE_SEND_AUDIO(VoiceService::VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
{
    // Add this new service. This should only happen once.
    BLE &ble = BLE::Instance();
    GattCharacteristic *characteristics[] = {
        &VOICESERVICE_START,
        &VOICESERVICE_RECIEVE_AUDIO,
        &VOICESERVICE_SEND_AUDIO
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
    server.write(VOICESERVICE_RECIEVE_AUDIO.getValueHandle(), (uint8_t*)&audio, sizeof(audio));
}

void VoiceService::sendAudio() {
    /*
    Todo tomorrow:
        - move audio recording methods to a separate location
        - figure out how to record and package audio to send over bluetooth
    */
}

void VoiceService::recieveAudio() {

}