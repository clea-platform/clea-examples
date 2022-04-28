
# Easy Edge firmware for EVA-DTS Tabletop Coffe Machine

### Setup

Go to `components` folder and clone there the [`Astarte SDK`](https://github.com/astarte-platform/astarte-device-sdk-esp32.git):

``` bash
cd components
git clone https://github.com/astarte-platform/astarte-device-sdk-esp32.git ./astarte
cd ..
```

Configure the project with WI-FI and Astarte settings.
Launch the project configuration tool with the command

```
idf.py menuconfig
```

Increase the app_main stack size by assigning to item `CONFIG_ESP_MAIN_TASK_STACK_SIZE` at `Component config/Common ESP32-related -> Main task stack size` the value 4096.  
Set the WI-FI configuration by editing in `Tabletop Coffee Machine demo` menu items:
+ `WIFI SSID`
+ `WIFI Password`
Optionally set also `WiFi maximum retry` item.

Set the Astarte configuration by editing in `Component config/Astarte SDK` items:
+ `Astarte realm`
+ `Astarte pairing base URL`
+ `Pairing JWT token`
+ `Astarte connectivity test URL`

---


# Setup board
TODO


# Build

Build the project and flash it to the board, then run the monitor tool to view the serial output.
Run following command, where `PORT` is the path to file which identifies the serial connection with the EasyEdge (usually `/dev/ttyUSB0`):
```bash
idf.py -p PORT build flash monitor