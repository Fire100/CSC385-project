#include "gap_handler.hpp"
#include "pretty_print.hpp"
#include "bluetooth_handler.hpp"
#include "mainQueue.hpp"

static const ble::AdvertisingParameters advertising_params(
    // CTRL+Click on the advertising type below to see other types.
    ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
    // The interval between advertisements is a trade-off between
    // being discovered lower power usage.
    ble::adv_interval_t(ble::millisecond_t(25)),
    ble::adv_interval_t(ble::millisecond_t(50))
);



void advertise()
{
    BLE &ble = BLE::Instance();
    auto &_gap = ble.gap();

    ble_error_t error = _gap.setAdvertisingParameters(
        ble::LEGACY_ADVERTISING_HANDLE, advertising_params);
    if (error) {
        print_error(error, "Gap::setAdvertisingParameters() failed");
        return;
    }

    ble::AdvertisingDataSimpleBuilder<ble::LEGACY_ADVERTISING_MAX_SIZE> data_builder;
    data_builder.setFlags();
    
    data_builder.setName("Bluetooth Phone");

    //  Use a "legacy" advertising handle
    error = _gap.setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE, data_builder.getAdvertisingData());
        if (error) {
        print_error(error, "Gap::setAdvertisingPayload() failed");
        return;
    }

    error = _gap.startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (error) {
        print_error(error, "Gap::startAdvertising() failed");
        return;
    }
}


void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error) {
        print_error(event->error, "Error during the initialisation");
        return;
    }

    // You will see this in Mbed Studio and, if all goes well, on your
    // Bluetooth Scanner
    print_mac_address();

    BLE &ble = BLE::Instance();
    auto &gap = ble.gap();

    // Setup the default phy used in connection to 2M to reduce power consumption
    if (gap.isFeatureSupported(ble::controller_supported_features_t::LE_2M_PHY)) {
        ble::phy_set_t phys(false, true, false);

        ble_error_t error = gap.setPreferredPhys(&phys, &phys);
        if (error) {
            print_error(error, "GAP::setPreferedPhys failed");
        }
    }

    // Rely on the event queue to advertise the device over BLE
    // TODO: probably don't need this
    mainQueue.call(advertise);
}


/**
 * @brief Do not change this.
 */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    mainQueue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}


void init_bluetooth()
{
    printf("Initializing Bluetooth\n");

    // The BLE class is a singleton
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    // Initialize BLE and then call our own function
    ble_error_t error = ble.init(&on_init_complete);
    if (error) {
        print_error(error, "Error returned by BLE::init");
        return;
    }

    // Setup our own listener for specific events.
    GapHandler handler;
    auto &gap = ble.gap();
    gap.setEventHandler(&handler);

    // NOTE: this must be here inside this function.
    // Putting this outside the function does not work
    mainQueue.dispatch_forever();
}
