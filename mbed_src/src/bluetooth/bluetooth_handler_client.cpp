#include "bluetooth_handler_client.hpp"
#include "globals.hpp"
#include "voice_service_client.hpp"
#include "gatt_client_process.h"




void init_bluetooth_client()
{
    printf("Initializing Bluetooth\n");
    printf("sample size %u \n", TARGET_AUDIO_BUFFER_NB_SAMPLES);

    VoiceServiceClient voiceServiceClient{};

    // The BLE class is a singleton
    BLE &ble = BLE::Instance();
    GattClientProcess ble_process(mainQueue, ble);

    //voiceServiceClient.start(ble, mainQueue);
    
    ble_process.on_init(callback(&voiceServiceClient, &VoiceServiceClient::start));
    ble_process.on_connect(callback(&voiceServiceClient, &VoiceServiceClient::start_discovery));
    
    ble_process.start();

    // NOTE: this must be here inside this function.
    // Putting this outside the function does not work
    mainQueue.dispatch_forever();
}
