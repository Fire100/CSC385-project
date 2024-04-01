#include "gap_handler.hpp"
#include "globals.hpp"
#include "recordAudio.hpp"
#include "voice_service_server.hpp"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include <stdint.h>
#include "USBAudio.h"
#include <algorithm>


VoiceServiceServer::VoiceServiceServer()
{

    BLE &ble = BLE::Instance();

    VOICESERVICE_START = new ReadOnlyGattCharacteristic<uint8_t> (VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
    VOICESERVICE_RECEIVE_AUDIO = new ReadWriteArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_RECEIVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
    VOICESERVICE_SEND_AUDIO = new ReadWriteArrayGattCharacteristic<uint8_t, AUDIO_TRANSFER_SIZE> (VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
    audio_buffer = new uint8_t[8000];
}

void read(){

}

void VoiceServiceServer::start()
{

    BLE &ble = BLE::Instance();

    const UUID uuid = VOICESERVICE_UUID;
    GattCharacteristic* charTable[] = { VOICESERVICE_START, VOICESERVICE_RECEIVE_AUDIO, VOICESERVICE_SEND_AUDIO};
    GattService voice_service_service(uuid, charTable, sizeof(charTable) / sizeof(charTable[0]));

    ble.gattServer().addService(voice_service_service);

    ble.gattServer().setEventHandler(this);
};


void VoiceServiceServer::onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params){
    record_audio();
}


void VoiceServiceServer::sendAudio() {
    if (buffer_location < voiceService->audio_iteration && this->sending_audio) {
        BLE &ble = BLE::Instance();
        int audioTransferIterations = (int) send_audio_size / AUDIO_TRANSFER_SIZE;

        printf("Sending Audio Data\n");
        ble_error_t err = ble.gattServer().write(VOICESERVICE_SEND_AUDIO->getValueHandle(), (uint8_t *)&send_audio_data[this->buffer_location * AUDIO_TRANSFER_SIZE], sizeof(send_audio_data[0]) * AUDIO_TRANSFER_SIZE);
        this->buffer_location++;

        if (this->buffer_location >=  voiceService->audio_iteration) {
            this->sending_audio = 0;
            this->buffer_location = 0;
        }
        
    }
}
void VoiceServiceServer::onDataWritten(const GattWriteCallbackParams &params) {
    printf("SERVICE: Data written from client.\n");

    if (params.handle == VOICESERVICE_RECEIVE_AUDIO->getValueHandle() && params.len == AUDIO_TRANSFER_SIZE){
        voiceService->receiving_audio = 1;
        voiceService->hvx_count++;
        
        int to_copy = min((int)AUDIO_TRANSFER_SIZE, voiceService->audio_buffer_size - (int)audio_buffer_idx);
        memcpy(audio_buffer + audio_buffer_idx, params.data, to_copy);

        audio_buffer_idx += to_copy;
        printf("Audio Data Index: %d\n", audio_buffer_idx);

        if (audio_buffer_idx >= voiceService->audio_buffer_size){
            this->playAudio(audio_buffer, voiceService->audio_buffer_size);
            audio_buffer_idx = 0;
        }

        
        if (voiceService->hvx_count >=  voiceService->audio_iteration) {
            voiceService->receiving_audio = 0;
            voiceService->hvx_count = 0;
        }

    }
}
