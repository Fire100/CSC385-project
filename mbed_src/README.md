#Additional libraries needed to build:

### companders (https://github.com/deftio/companders.git)
Need to comment out compandit.c.

### mbed-os-ble-utils

Need to replace the name in gatt_client_process.h with the name of the bluetooth device
we want to connect to. In our case, it should be "Bluetooth Phone":
```
    const char* get_peer_device_name()
    {
        static const char name[] = "Bluetooth Phone";
        return name;
    }
```

In the same file, change line 115 to be:
```
    const ble::ConnectionParameters connection_params(ble::phy_t::LE_1M, ble::scan_window_t::min(), ble::conn_interval_t::min(), ble::conn_interval_t::min());
```
This changes the connection interval to be a few millescond quicker, increasing throughput.