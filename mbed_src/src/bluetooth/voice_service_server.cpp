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

    // GattAttribute* nameDescrRECEIVEAudio = new GattAttribute( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC , (uint8_t *)received_name.c_str(), received_name.size());
    // GattAttribute *descriptorsRECEIVEdAudio[] = {nameDescrRECEIVEAudio};

    // GattAttribute* nameDescrSentAudio = new GattAttribute( BLE_UUID_DESCRIPTOR_CHAR_USER_DESC , (uint8_t *)sent_name.c_str(), sent_name.size());
    // GattAttribute *descriptorsSentAudio[] = {nameDescrSentAudio};

    // VOICESERVICE_START = new ReadOnlyGattCharacteristic<uint8_t> (VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY, descriptorsStart,  sizeof(descriptorsStart)/sizeof(GattAttribute*));
    // VOICESERVICE_RECEIVE_AUDIO = new WriteOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_RECEIVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY, descriptorsRECEIVEdAudio, sizeof(descriptorsRECEIVEdAudio)/sizeof(GattAttribute*));
    // VOICESERVICE_SEND_AUDIO = new ReadOnlyArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY, descriptorsSentAudio, sizeof(descriptorsSentAudio)/sizeof(GattAttribute*));
    
    VOICESERVICE_START = new ReadOnlyGattCharacteristic<uint8_t> (VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
    VOICESERVICE_RECEIVE_AUDIO = new ReadWriteArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_RECEIVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
    VOICESERVICE_SEND_AUDIO = new ReadWriteArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
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

void VoiceServiceServer::sendAudio() {

    BLE &ble = BLE::Instance();
    //printf("writing audio \n");

    int audioTransferIterations = (int) send_audio_size / AUDIO_TRANSFER_SIZE;
    printf("SENDING AUDIO %d %d\n", send_audio_size, audioTransferIterations);
    // printf("%d \n", audioTransferIterations);
    for (int i = 0; i < audioTransferIterations; i++) {
        ble.gattServer().write(VOICESERVICE_SEND_AUDIO->getValueHandle(), (uint8_t *)&send_audio_data[i * AUDIO_TRANSFER_SIZE], sizeof(send_audio_data[0]) * AUDIO_TRANSFER_SIZE);
    }
    printf("DONE SEND\n");
}

void VoiceServiceServer::onDataSent(const GattDataSentCallbackParams &params){
    // printf("SENT DATA\n");
}

void VoiceServiceServer::onAttMtuChange(ble::connection_handle_t connectionHandle, uint16_t attMtuSize) {
    printf("MTTUSIZE: %d %u\n", connectionHandle, attMtuSize);
}


void VoiceServiceServer::onDataWritten(const GattWriteCallbackParams &params) {
    // printf("SERVICE: Data written from client.\n");


    if (params.handle == VOICESERVICE_RECEIVE_AUDIO->getValueHandle() && params.len == AUDIO_TRANSFER_SIZE){
        // printf("SERVICE: Acquired new audio data! %u\n", *params.data);

        if (audio_buffer_idx == 0){
            t.start();
        }
        int to_copy = min((int)AUDIO_TRANSFER_SIZE, 8000 - audio_buffer_idx);
        memcpy(audio_buffer + audio_buffer_idx, params.data, to_copy);
        audio_buffer_idx += to_copy;
        printf("NEW AUDIO DATA: %d", audio_buffer_idx);
        if (audio_buffer_idx >= 8000){
            t.stop();
            printf("On Data Written: %llu ms\n", t.elapsed_time().count());

            this->playAudio(audio_buffer, 8000);
            audio_buffer_idx = 0;
        }

    }
}