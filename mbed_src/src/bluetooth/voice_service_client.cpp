#include "gap_handler.hpp"
#include "voice_service_client.hpp"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstdint>
#include <stdint.h>


// might want to move these in the future but leaving them until we get basic ble working
uint16_t _discovered_UUID = 0x0000;
GattAttribute::Handle_t _CCCD_handle(0);


VoiceServiceClient::VoiceServiceClient()
{
    voiceservice_receive_audio_found = false;
    voiceservice_send_audio_found = false;
    
    searchForChar = 0;
    hvx_count = 0;

    audio_buffer_idx = 0;
    audio_buffer = new uint8_t[audio_buffer_size];
}


void on_read(const GattHVXCallbackParams *response) {
    voiceService->receiving_audio = 1;
    printf("Data received\n");
    printf("SERVICE: Data written from server.\n");
    printf("Response Length: %d \n", response->len);
    printf("Response Handle: %d \n", response->handle);
    printf("SERVICE: Acquired new audio data! %u\n",*(response->data));


    voiceService->hvx_count++;
    printf("\nhvx_count: %d \n", voiceService->hvx_count);

    if (response->handle == VOICESERVICE_SEND_AUDIO.getValueHandle() && response->len == VoiceServiceClient::AUDIO_TRANSFER_SIZE){
        printf("SERVICE: Acquired new audio data! %u\n", *(response->data));


        int to_copy = min((int)VoiceServiceClient::AUDIO_TRANSFER_SIZE, voiceService->audio_buffer_size - voiceService->audio_buffer_idx);
        printf("TO_COPY: %d\n", to_copy);
        memcpy(voiceService->audio_buffer + voiceService->audio_buffer_idx, response->data, to_copy);
        printf("DONE MEMCPY\n");

        voiceService->audio_buffer_idx += to_copy;
        printf("NEW AUDIO DATA: %d", voiceService->audio_buffer_idx);
        if (voiceService->audio_buffer_idx >= voiceService->audio_buffer_size){
            printf("On Data Written\n");

            voiceService->playAudio(voiceService->audio_buffer, voiceService->audio_buffer_size);
            voiceService->audio_buffer_idx = 0;
        }
    }


    if (voiceService->hvx_count >= voiceService->audio_iteration) {
        voiceService->receiving_audio = 0;
        voiceService->hvx_count = 0;
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
    printf("characteristic run\n");
    printf("uuid %u \n", characteristic->getUUID().getShortUUID());
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

//////////////////////////// This code section is heavily based on https://gist.github.com/pan-/585af73a8fd0ff236b9897c0432d4ef0 ////////////////////////////////////////////

void look_for_Descriptors(){
    printf("    \n");
    BLE &ble = BLE::Instance();
    DiscoveredCharacteristic* characteristic;
    if (voiceService->searchForChar == 0) characteristic = &VOICESERVICE_SEND_AUDIO;
    else characteristic = &VOICESERVICE_RECEIVE_AUDIO;


    printf(
    "start looking for Descriptors of characteristic %d, range [%d, %d] now\r\n",
    characteristic->getValueHandle(),
    characteristic->getValueHandle() + 1,
    characteristic->getLastHandle()
    );
    ble.gattClient().discoverCharacteristicDescriptors(
        *characteristic,
        characteristicDescriptorDiscoveryCallback,
        descriptorDiscoveryTerminationCallback
    );

    
}

void characteristicDescriptorDiscoveryCallback(
    const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *charParams
) {
    DiscoveredCharacteristic* characteristic;
    if (voiceService->searchForChar == 0) characteristic = &VOICESERVICE_SEND_AUDIO;
    else characteristic = &VOICESERVICE_RECEIVE_AUDIO;

    printf("descriptor found with:\n");
    printf("connection_handle[%u] UUID[%X] attribute_Handle[%u]\r\n",
        charParams->descriptor.getConnectionHandle(),
        charParams->descriptor.getUUID().getShortUUID(),
        charParams->descriptor.getAttributeHandle()
    );

    // no reason to pursue the descriptor discovery at this point
    // request to terminate it then get notified in the termination callback
    if (charParams->descriptor.getUUID().getLen() != UUID::LENGTH_OF_LONG_UUID &&
        charParams->descriptor.getUUID().getShortUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG
    ) {
        _discovered_UUID = charParams->descriptor.getUUID().getShortUUID();
        _CCCD_handle = charParams->descriptor.getAttributeHandle();

        GattClient& client = BLE::Instance().gattClient();
        client.terminateCharacteristicDescriptorDiscovery(*characteristic);
        printf("CCCD found; explicit termination of descriptors discovery\r\n");
    }
}

void descriptorDiscoveryTerminationCallback(
    const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *termParams
) {
    if (termParams->error_code || _discovered_UUID != BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG) {
        printf("no cccd found\r\n");
        return;
    }

    printf("descriptorDiscovery terminated without errors\r\n");
    printf("cccd with handle [%u] found!\r\n", _CCCD_handle);
    mainQueue.call(write_cccd);

    record_audio();
}

void write_cccd() {
    // cccd are 16 bit bit long; indication flag is on bit 2
    uint16_t cccd_value = BLE_HVX_NOTIFICATION;
    GattClient& client = BLE::Instance().gattClient();

    DiscoveredCharacteristic* characteristic;
    if (voiceService->searchForChar == 0) characteristic = &VOICESERVICE_SEND_AUDIO;
    else characteristic = &VOICESERVICE_RECEIVE_AUDIO;


    client.negotiateAttMtu(VOICESERVICE_RECEIVE_AUDIO.getConnectionHandle());
    ble_error_t err = client.write(
        GattClient::GATT_OP_WRITE_REQ,
        characteristic->getConnectionHandle(),
        _CCCD_handle,
        sizeof(cccd_value),
        (uint8_t*) &cccd_value
    );
    
}

/////////////////////////////////// code section end /////////////////////////////////////////////////////////////////////////////////////


void discovery_termination(ble::connection_handle_t connectionHandle) {
    
    if (voiceservice_receive_audio_found) {
        printf("discovery term\n");
        printf("%d value \n", voiceservice_receive_audio_found);
        mainQueue.call(look_for_Descriptors);
    }
}


 void VoiceServiceClient::start(BLE &ble, events::EventQueue &event_queue) {
    printf("start ran \n");
    ble.gattClient().onHVX(on_read);
    
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


void VoiceServiceClient::sendAudio() {

    if (voiceservice_send_audio_found && buffer_location < 40 && voiceService->sending_audio) {
        
        BLE &ble = BLE::Instance();
        //printf("writing audio \n");
        //printf("%d \n", send_audio_size);
        int audioTransferIterations = (int) send_audio_size / AUDIO_TRANSFER_SIZE;
        printf("position %d \n", this->buffer_location);
        
        for (int i = 0; i < 1; i++) {
            
            printf("Audio transfer iteterations so far: %d\n", audioTransferIterations);
            printf("About to write\n");
            ble_error_t err = VOICESERVICE_RECEIVE_AUDIO.writeWoResponse(sizeof(send_audio_data[0]) * AUDIO_TRANSFER_SIZE, (uint8_t *)&send_audio_data[this->buffer_location * AUDIO_TRANSFER_SIZE]);
            this->buffer_location++;

            printf("Write Error (0 none): %u \n", err);

            for (int j = 0; j < AUDIO_TRANSFER_SIZE; j++) {
                // printf("data sent\n");
                printf("%u ", send_audio_data[this->buffer_location * AUDIO_TRANSFER_SIZE + j]);
                
            }
            printf("\n");

        }
        printf("\n");

        if (this->buffer_location >= voiceService->audio_iteration) {
            this->sending_audio = 0;
            this->buffer_location = 0;
        }
        
        
    }
}
