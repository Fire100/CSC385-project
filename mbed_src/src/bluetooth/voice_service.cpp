#include "gap_handler.hpp"
#include "voice_service.hpp"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include <stdint.h>

GattAttribute nameDescrStart( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC, (uint8_t *)"Audio Start", strlen("Audio Start"));
GattAttribute *descriptorsStart[] = {&nameDescrStart};

GattAttribute nameDescrRecieveAudio( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC, (uint8_t *)"Recieved Audio", strlen("Recieved Audio"));
GattAttribute *descriptorsRecievedAudio[] = {&nameDescrRecieveAudio};

GattAttribute nameDescrSentAudio( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC, (uint8_t *)"Sent Audio", strlen("Sent Audio"));
GattAttribute *descriptorsSentAudio[] = {&nameDescrSentAudio};

VoiceService::VoiceService()
{
    VOICESERVICE_START = new ReadOnlyGattCharacteristic<uint8_t> (VoiceService::VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY );
    VOICESERVICE_RECIEVE_AUDIO = new ReadOnlyGattCharacteristic<uint8_t> (VoiceService::VOICESERVICE_RECIEVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY );
    VOICESERVICE_SEND_AUDIO = new ReadOnlyGattCharacteristic<uint8_t> (VoiceService::VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY );


    // Add this new service. This should only happen once.
    // BLE &ble = BLE::Instance();
    // GattCharacteristic *characteristics[] = {
    //     &VOICESERVICE_START,
    //     &VOICESERVICE_RECIEVE_AUDIO,
    //     &VOICESERVICE_SEND_AUDIO
    // };

    // GattService voice_service(
    //     VoiceService::VOICESERVICE_UUID,
    //     characteristics,
    //     sizeof(characteristics) / sizeof(characteristics[0])
    // );
    // ble.gattServer().addService(voice_service);

    // Set an event handler that is called after a connected peer has written an attribute.
    // server.onDataWritten(VoiceService::onDataWritten);
}

void VoiceService::start(BLE &ble, events::EventQueue &event_queue)
    {
        const UUID uuid = VOICESERVICE_UUID;
        GattCharacteristic* charTable[] = { VOICESERVICE_START, VOICESERVICE_RECIEVE_AUDIO, VOICESERVICE_SEND_AUDIO };
        GattService voice_service_service(uuid, charTable, 1);

        ble.gattServer().addService(voice_service_service);

        ble.gattServer().setEventHandler(this);

        printf("Voice service added with UUID 0xB000\r\n");
        printf("Connect and write to characteristic 0xB002\r\n");

        // This is test code. remove this when audio setup is done
        uint16_t start = 12;
        uint16_t audio = 30;
        ble::GattServer& server = ble.gattServer();
        server.write(VOICESERVICE_START->getValueHandle(), (uint8_t*)&start, sizeof(start));
        server.write(VOICESERVICE_RECIEVE_AUDIO->getValueHandle(), (uint8_t*)&audio, sizeof(audio));
    };

void VoiceService::sendAudio() {
    // send data from the compacted channel
    // this will be from compressedBuf but just sending dummy data for now
    uint8_t bytesSent = 5;
    uint8_t bytesToSend[5] = {0, 1, 2, 3, 4};
    BLE &ble = BLE::Instance();

    ble.gattServer().write(VOICESERVICE_SEND_AUDIO->getValueHandle(), (uint8_t *)&bytesToSend[currentDataSent], sizeof(bytesToSend[currentDataSent]));

    currentDataSent = (currentDataSent + 1) % bytesSent;
}

void VoiceService::onDataRead(const GattReadCallbackParams &params) {
    printf("Data read\n");
    //VoiceService::sendAudio();
}

void VoiceService::onDataSent(const GattDataSentCallbackParams &params) {
    printf("Data sent\n");
    //VoiceService::sendAudio();
}

// need to see if this will also get triggered when audio is written to the other device.
void VoiceService::onDataWritten(const GattWriteCallbackParams &params) {
    printf("Data written\n");
    // set to get n
    // uint8_t dataIn;
    // BLE &ble = BLE::Instance();
    // ble.gattServer().read(VOICESERVICE_RECIEVE_AUDIO.getValueHandle(), (uint8_t *)&dataIn, sizeof(dataIn));
    // dataIn = *(params.data);
    
}