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


# How to Run the Project

Steps:
- Set up a "microphone input to speaker outuput" app on computer
    - For Mac, you can use Loopback
- Get 2 boards. 
- For one board, make sure it is a client. Make sure the other board is not a client and is instead a server.
    - To do this, set the CLIENT flag in src/globals.hpp for one of the boards
- For each board:
    - connect the computer to the STLINK port on the board via USB to flash compiled code. 
    - connect a USB to the USB OTG port on the board.
- The two boards should have connected via Bluetooth by now. 
- When ready, press the USER button on one of the boards to send an audio clip
    - Do not press the buttons on both boards simultaenesouly. This is undefined behavior.
- Wait until the other board has received the audio clip. This may take ~1 minute
- You can repeat the above two steps for any of the boards indefinitely.
