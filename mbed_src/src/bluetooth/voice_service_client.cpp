#include "gap_handler.hpp"
#include "voice_service_client.hpp"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include <stdint.h>


VoiceServiceClient::VoiceServiceClient()
{
    voiceservice_receive_audio_found = false;
    voiceservice_send_audio_found = false;

    // VOICESERVICE_START = new ReadOnlyGattCharacteristic<uint8_t> (VoiceService::VOICESERVICE_START_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY );
    // VOICESERVICE_RECEIVE_AUDIO = new ReadOnlyGattCharacteristic<uint8_t> (VoiceService::VOICESERVICE_RECEIVE_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY );
    // VOICESERVICE_SEND_AUDIO = new ReadOnlyGattCharacteristic<uint8_t> (VoiceService::VOICESERVICE_SEND_AUDIO_UUID, 0, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY );


    // Add this new service. This should only happen once.
    // BLE &ble = BLE::Instance();
    // GattCharacteristic *characteristics[] = {
    //     &VOICESERVICE_START,
    //     &VOICESERVICE_RECEIVE_AUDIO,
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

void on_read(const GattReadCallbackParams *response) {
    printf("Data received\n");
    // printf("SERVICE: Data written from server.\n");

    if (response->handle == VOICESERVICE_SEND_AUDIO.getValueHandle() && response->len == 1){
        // printf("SERVICE: Acquired new audio data! %u\n", *(response->data));

        voiceService->playAudio((uint8_t *)response->data, VoiceServiceClient::AUDIO_TRANSFER_SIZE);

    }

}

void service_discovery(const DiscoveredService *service) {
    if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
        if (service->getUUID().getShortUUID() == VoiceServiceClient::VOICESERVICE_UUID) {
            printf("We found the service we were looking for\r\n");
        }
    }
    printf("looking for service\n");
}

void characteristic_discovery(const DiscoveredCharacteristic *characteristic) {
    if (characteristic->getUUID().getShortUUID() == VoiceServiceClient::VOICESERVICE_SEND_AUDIO_UUID) {
        printf("We found the send audio characteristic\r\n");
        VOICESERVICE_SEND_AUDIO = *characteristic;
        
        voiceservice_send_audio_found = true;
    }
    if (characteristic->getUUID().getShortUUID() == VoiceServiceClient::VOICESERVICE_RECEIVE_AUDIO_UUID ) {
        printf("We found the receive audio characteristic\r\n");
        VOICESERVICE_RECEIVE_AUDIO = *characteristic;
        printf("%u \n", VOICESERVICE_RECEIVE_AUDIO.getUUID().getShortUUID());
        voiceservice_receive_audio_found = true;
    }
    printf("test\n");
}

void discovery_termination(ble::connection_handle_t connectionHandle) {
    
    if (voiceservice_send_audio_found) {
        printf("discovery term\n");
        printf("%d value \n", voiceservice_send_audio_found);
        //VOICESERVICE_RECEIVE_AUDIO.read();
        // mainQueue.call_every(100ms, []{ VOICESERVICE_SEND_AUDIO.read(); });
    }
    // also do for send?
}


 void VoiceServiceClient::start(BLE &ble, events::EventQueue &event_queue) {
    printf("start ran \n");
    ble.gattClient().onDataRead(on_read);
}

void VoiceServiceClient::start_discovery(BLE &ble, events::EventQueue &event_queue, const ble::ConnectionCompleteEvent &event) {
    printf("We are looking for a service with UUID 0xB000\r\n");
    printf("And a characteristic with UUID 0xB001\r\n");

    ble.gattClient().onServiceDiscoveryTermination(discovery_termination);
    // UUID unknown is a wildcard that lets us find all characteristics
    ble.gattClient().launchServiceDiscovery(
        event.getConnectionHandle(),
        service_discovery,
        characteristic_discovery,
        VoiceServiceClient::VOICESERVICE_UUID,
        BLE_UUID_UNKNOWN
    );
}

void VoiceServiceClient::sendAudio(uint8_t* audio_data, uint32_t size) {
    if (voiceservice_receive_audio_found) {
        // printf("Data sent \n");
        // BLE &ble = BLE::Instance();
        // //printf("writing audio \n");
        // int audioTransferIterations = (int) size / AUDIO_TRANSFER_SIZE;
        // // printf("%d \n", audioTransferIterations);
        // for (int i = 0; i < audioTransferIterations; i++) {
        //     //printf("%d \n", (int) audio_data[i]);
        //     VOICESERVICE_RECEIVE_AUDIO.write(sizeof(audio_data[0]) * AUDIO_TRANSFER_SIZE, (uint8_t *)&audio_data[i * AUDIO_TRANSFER_SIZE]);
        // }

        // write one value at a time
        BLE &ble = BLE::Instance();
        //printf("data sent\n ");
        // printf("%d \n", ble.gattClient().isCharacteristicDescriptorDiscoveryActive(VOICESERVICE_RECEIVE_AUDIO));
        // printf("%d \n", voiceservice_receive_audio_found);
        // printf("data sent\n");
        for (int i = 0; i < size; i++) {
            VOICESERVICE_RECEIVE_AUDIO.write(sizeof(audio_data[i]), (uint8_t *)&audio_data[i]);
        }
        
        
    }
}

// void VoiceService::onDataRead(const GattReadCallbackParams &params) {
//     printf("Data read\n");
//     //VoiceService::sendAudio();
// }

// void VoiceService::onDataSent(const GattDataSentCallbackParams &params) {
//     printf("Data sent\n");
//     //VoiceService::sendAudio();
// }

// // need to see if this will also get triggered when audio is written to the other device.
// void VoiceService::onDataWritten(const GattWriteCallbackParams &params) {
//     printf("Data written\n");
//     // set to get n
//     // uint8_t dataIn;
//     // BLE &ble = BLE::Instance();
//     // ble.gattServer().read(VOICESERVICE_RECEIVE_AUDIO.getValueHandle(), (uint8_t *)&dataIn, sizeof(dataIn));
//     // dataIn = *(params.data);
//  
//}