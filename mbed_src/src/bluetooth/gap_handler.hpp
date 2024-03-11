/**
 * @file lab5.hpp
 * @author Mario Badr, Lucas Wilkinson
 * @version 20221
 * @copyright Copyright (c) 2022 Mario Badr
 *
 * @brief This module contains functions and classes for communicating over BLE.
 */
#ifndef CSC385_20221_LAB_4_HPP
#define CSC385_20221_LAB_4_HPP

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"


/**
 * @brief A simple listener for some BLE events.
 */
class GapHandler : private mbed::NonCopyable<GapHandler>, public ble::Gap::EventHandler
{
public:
    /**
     * @brief Called when the device starts advertising itself to others.
     */
    void onAdvertisingStart(const ble::AdvertisingStartEvent &event) override;

    /**
     * @brief Called when another device connects to ours.
     */
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

    /**
     * @brief Called when another connected evice disconnects from ours.
     */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;
};

#endif //CSC385_20221_LAB_4_HPP
