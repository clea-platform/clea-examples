# poc-engine-micro

## Build and Run instructions

- Pull git submodule `astarte-device-sdk-esp32` repo in `components/`.

```bash
git submodule add https://github.com/astarte-platform/astarte-device-sdk-esp32.git components/astarte-device-sdk-esp32
git submodule update --init --recursive
```

- Checkout branch `release 1.0` of `astarte-device-sdk-esp32`

```bash
cd components/astarte-device-sdk-esp32
git checkout release-1.0
```

- Build and flash using the IDF Docker or IDF at `IDF_CHECKOUT_REF=8bc19ba893e5544d571a753d82b44a84799b94b1`. Even better, use the VSCode extension for ESP32 to build and deploy.

## EDGEHOG Micro

Starts a FreeRTOS task to for EDGEHOG IoT platform.
Please use the given docker image to build projects including EDGEHOG micro,
it has been tested in a specific environment.

## Importing EDGEHOG Micro

To use the EDGEHOG micro static library the project must have a component folder for EDGEHOG micro itself containing the static library, and the include folder where the .h file is sotored.

In the EDGEHOG micro component folder the CMakeLists.txt file must be formatted as follows


    idf_component_register(INCLUDE_DIRS "include")
    #target_link_libraries(${COMPONENT_LIB} INTERFACE "-L ${CMAKE_CURRENT_SOURCE_DIR}")

    add_prebuilt_library(edgehog_micro "libedgehog_micro.a" REQUIRES nvs_flash mbedtls bt protocomm wifi_provisioning app_update mqtt json esp_https_ota driver protobuf-c)

    target_link_libraries(${COMPONENT_LIB} INTERFACE edgehog_micro)



## D47 specific recommendations

Before flashing the board must be booted with JP1 jumper closed.
Boot can be triggered using the on-board button.

When the flashing part ends JP1 jumper must be open and the board booted again manually.

## IDF Docker Image

Esressif docker image must be builded with option <code>--build-arg IDF_CHECKOUT_REF=8bc19ba893e5544d571a753d82b44a84799b94b1</code>.

Dockerfile can be found [here](https://hub.docker.com/r/espressif/idf/dockerfile).

### Building sources launching in project directory:

    docker run --rm -v $PWD:/project -w /project espressif/idf idf.py build

It is possible to give additional commands such as <code>flash</code> and <code>monitor</code>.

### Using an interacrive shell in container:

    docker run --rm -v $PWD:/project -w /project -it espressif/idf

Than inside the container is possible to use all commands supported by Esp IDF.

### Usefoul links

[Docker Image guide by Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-docker-image.html)

[Esp - IDF supported commands](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#step-7-configure)
