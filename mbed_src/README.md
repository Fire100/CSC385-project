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



