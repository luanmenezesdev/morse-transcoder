# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(morse_transcoder C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

# Compiler options
add_compile_options(-Wall
    -Wno-format          # Ignore int != int32_t warnings
    -Wno-unused-function # Ignore unused functions
    -Wno-maybe-uninitialized
)

if (PICO_CYW43_SUPPORTED) # Ensures Pico W-specific build
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping build as support is not available")
    else()
        # Retrieve Wi-Fi SSID and password from environment variables
        set(WIFI_SSID $ENV{WIFI_SSID})
        set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})

        if ("${WIFI_SSID}" STREQUAL "")
            message(FATAL_ERROR "WIFI_SSID environment variable is not defined")
        elseif ("${WIFI_PASSWORD}" STREQUAL "")
            message(FATAL_ERROR "WIFI_PASSWORD environment variable is not defined")
        else()
            # Add the executable for the project
            add_executable(morse_transcoder
                src/morse_transcoder.c
                src/BitBang_I2C.c
                src/morse_decoder.c
                src/ssd1306_i2c.c
            )

            # Enable USB output, disable UART output
            pico_enable_stdio_usb(morse_transcoder 0)
            pico_enable_stdio_uart(morse_transcoder 1)

            # Define preprocessor macros for Wi-Fi credentials
            target_compile_definitions(morse_transcoder PRIVATE
                WIFI_SSID=\"${WIFI_SSID}\"
                WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
                NO_SYS=1
            )

            # Include directories for the project
            target_include_directories(morse_transcoder PRIVATE
                ${CMAKE_CURRENT_LIST_DIR}/src
                ${CMAKE_CURRENT_LIST_DIR}/include
                ${PICO_SDK_PATH}/lib/mbedtls/include
            )

            # Link required libraries for LWIP MQTT and TLS functionality
            target_link_libraries(morse_transcoder
                pico_cyw43_arch_lwip_poll
                pico_stdlib
                pico_lwip_mbedtls
                pico_lwip_mqtt
                pico_stdlib
                hardware_gpio
                hardware_i2c
                pico_lwip_mqtt
            )

            # Ensure additional outputs are added for the program
            pico_add_extra_outputs(morse_transcoder)
        endif()
    endif()
endif()
