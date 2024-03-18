#include "gap_handler.hpp"
#include "voice_service_server.hpp"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include <stdint.h>
#include "USBAudio.h"


VoiceServiceServer::VoiceServiceServer()
{

    BLE &ble = BLE::Instance();

    // GattAttribute* nameDescrStart = new GattAttribute( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC , (uint8_t *)start_name.c_str(), start_name.size());
    // GattAttribute *descriptorsStart[] = {nameDescrStart};

    // GattAttribute* nameDescrRecieveAudio = new GattAttribute( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC , (uint8_t *)received_name.c_str(), received_name.size());
    // GattAttribute *descriptorsRecievedAudio[] = {nameDescrRecieveAudio};

    // GattAttribute* nameDescrSentAudio = new GattAttribute( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC , (uint8_t *)sent_name.c_str(), sent_name.size());
    // GattAttribute *descriptorsSentAudio[] = {nameDescrSentAudio};

    // VOICESERVICE_START = new ReadOnlyGattCharacteristic<uint8_t> (VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY, descriptorsStart,  sizeof(descriptorsStart)/sizeof(GattAttribute*));
    // VOICESERVICE_RECIEVE_AUDIO = new WriteOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_RECIEVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY, descriptorsRecievedAudio, sizeof(descriptorsRecievedAudio)/sizeof(GattAttribute*));
    // VOICESERVICE_SEND_AUDIO = new ReadOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY, descriptorsSentAudio, sizeof(descriptorsSentAudio)/sizeof(GattAttribute*));
    
    VOICESERVICE_START = new ReadOnlyGattCharacteristic<uint8_t> (VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
    VOICESERVICE_RECEIVE_AUDIO = new WriteOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_RECEIVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
    VOICESERVICE_SEND_AUDIO = new ReadOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
}

void VoiceServiceServer::start()
{

    BLE &ble = BLE::Instance();

    const UUID uuid = VOICESERVICE_UUID;
    GattCharacteristic* charTable[] = { VOICESERVICE_START, VOICESERVICE_RECEIVE_AUDIO, VOICESERVICE_SEND_AUDIO};
    GattService voice_service_service(uuid, charTable, sizeof(charTable) / sizeof(charTable[0]));

    ble.gattServer().addService(voice_service_service);

    ble.gattServer().setEventHandler(this);

    // This is test code. remove this when audio setup is done
    // uint16_t start = 12;
    // uint16_t audio = 30;
    // ble::GattServer& server = ble.gattServer();
    // server.write(VOICESERVICE_START->getValueHandle(), (uint8_t*)&start, sizeof(start));
    // server.write(VOICESERVICE_RECEIVE_AUDIO->getValueHandle(), (uint8_t*)&audio, sizeof(audio));
};

void VoiceServiceServer::sendAudio(uint8_t* audio_data, uint32_t size) {
    BLE &ble = BLE::Instance();
    //printf("writing audio \n");
    int audioTransferIterations = (int) size / AUDIO_TRANSFER_SIZE;
    // printf("%d \n", audioTransferIterations);
    for (int i = 0; i < audioTransferIterations; i++) {
        //printf("%d \n", (int) audio_data[i]);
        ble.gattServer().write(VOICESERVICE_SEND_AUDIO->getValueHandle(), (uint8_t *)&audio_data[i * AUDIO_TRANSFER_SIZE], sizeof(audio_data[0]) * AUDIO_TRANSFER_SIZE);
    }
}

void VoiceServiceServer::onDataWritten(const GattWriteCallbackParams &params) {
    printf("SERVICE: Data written from client.\n");

    if (params.handle == VOICESERVICE_RECEIVE_AUDIO->getValueHandle() && params.len == 1){
        printf("SERVICE: Acquired new audio data! %u\n", *params.data);

        this->playAudio((uint8_t *)params.data, AUDIO_TRANSFER_SIZE);

    }
}