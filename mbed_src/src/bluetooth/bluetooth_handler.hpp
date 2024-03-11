#ifndef BLUETOOTH_HANDLER
#define BLUETOOTH_HANDLER

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "gap_handler.hpp"
#include "pretty_print.hpp"

// class BluetoothHandler {
//     private:
//         static void advertise();
//         static void on_init_complete(BLE::InitializationCompleteCallbackContext *event);
//         static void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context);
//     public:
//         /**
//         * @brief Construct a new BluetoothHandler
//         */
//         BluetoothHandler();

//         // TODO: add functions for cleanliness
        
// };

void init_bluetooth();

#endif // BLUETOOTH_HANDLER