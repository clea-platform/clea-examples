# CLEA Accelerometer Demo Example

This demo is based on [Easy Edge](https://products.seco.com/it/easy-edge.html) seco board.

![EASY_EDGE](https://products.seco.com/media/catalog/product/cache/6561462fb70bf4a24230671d096f589e/E/a/Easy_Edge_1000x1000px_front_2_1.png)

## How to use example

### Setup project

* Add Astarte compenent

    ``` bash
    git clone https://github.com/astarte-platform/astarte-device-sdk-esp32.git ./components/astarte
    ```
* Add LIS3DH Driver compenent
    ``` bash
    git clone https://github.com/harlem88/lis3dh-esp-idf.git
    mv ./lis3dh-esp-idf/components/lis3dh ./components
    mv ./lis3dh-esp-idf/components/esp8266_wrapper ./components
    rm -rf lis3dh-esp-idf
    ```

### Configure the project

Open the project configuration menu (`idf.py menuconfig`).

In the `DEMO easyedge` menu:

* Set the Wi-Fi configuration.
    * Set `WiFi SSID`.
    * Set `WiFi Password`.

In the `Component config -> Astarte SDK` submenu:

* Set the Astarte configuration.
    * Set `realm name`.
    * Set `Astarte Pairing base URL`
    * Set `Pairing JWT` you've generated before.

If you have deployed Astarte through docker-compose, the Astarte Pairing base URL is http://<your-machine-url>:4003. On
a common, standard installation, the base URL can be built by adding `/pairing` to your API base URL, e.g.
`https://api.astarte.example.com/pairing`.

### Build and Flash

Build the project and flash it to the board, then run the monitor tool to view the serial output:

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

Example Log:

```
I (2152) wifi:AP's beacon interval = 102400 us, DTIM period = 2
I (2662) esp_netif_handlers: sta ip: 192.168.43.188, mask: 255.255.255.0, gw: 192.168.43.147
I (2662) i2c-simple-example: got ip:192.168.43.188
I (2672) i2c-simple-example: connected to ap SSID:zero
I (2682) ASTARTE_HANDLER: Astarte Device ID -> **********************
I (4942) ASTARTE_HANDLER: ADDING INTERFACES OK
I (4942) ASTARTE_HANDLER: astarte device start
3759.214 LIS3DH (xyz)[g] ax= +0.033 ay= -0.072 az= +1.033
I (7382) ASTARTE_HANDLER: on_connected
I (7382) i2c-simple-example:  ASTARTE CONNECTED
I (78635) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.022 ay= -0.029 az= +1.055
I (78735) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.025 ay= -0.018 az= +1.033
I (78835) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.023 ay= -0.012 az= +1.018
I (78635) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.022 ay= -0.029 az= +1.055
I (78735) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.025 ay= -0.018 az= +1.033
I (78835) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.023 ay= -0.012 az= +1.018
I (78635) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.022 ay= -0.029 az= +1.055
I (78735) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.025 ay= -0.018 az= +1.033
I (78835) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.023 ay= -0.012 az= +1.018
I (78635) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.022 ay= -0.029 az= +1.055
I (78735) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.025 ay= -0.018 az= +1.033
I (78835) i2c-simple-example: LIS3DH (xyz)[g] ax= +0.023 ay= -0.012 az= +1.018

```