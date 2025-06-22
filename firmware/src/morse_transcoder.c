#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "ssd1306.h"
#include "morse_decoder.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

// Wi-Fi Credentials
#define WIFI_SSID "brisa-4370576"
#define WIFI_PASSWORD "mmy6opmr"

// MQTT Configuration
#define MQTT_BROKER "52.29.49.80"
#define MQTT_TOPIC "transcoder/chat"

#define LED_R_PIN 13
#define LED_G_PIN 11

// MQTT Client
mqtt_client_t *mqtt_client;

static const struct mqtt_connect_client_info_t mqtt_client_info = {
    .client_id = "PicoWLuanMenezes", // Client ID (required)
    .client_user = NULL,             // Username (optional, NULL if not used)
    .client_pass = NULL,             // Password (optional, NULL if not used)
    .keep_alive = 60,                // Keep-alive interval in seconds
    .will_topic = NULL,              // Last will topic (optional)
    .will_msg = NULL,                // Last will message (optional)
    .will_retain = 0,                // Last will retain flag
    .will_qos = 0                    // Last will QoS level
};

// Function to send a message via MQTT
void mqtt_send_message(mqtt_client_t *client, const char *message)
{
    char formatted_message[256];
    snprintf(formatted_message, sizeof(formatted_message), "pico: %s", message);

    err_t result = mqtt_publish(client, MQTT_TOPIC, formatted_message, strlen(formatted_message), 0, 0, NULL, NULL);
    if (result == ERR_OK)
    {
        printf("Message published: %s\n", formatted_message);
        gpio_put(LED_R_PIN, 1); // Turn on red LED
        sleep_ms(500);
        gpio_put(LED_R_PIN, 0); // Turn off red LED
    }
    else
    {
        printf("Failed to publish message. Error: %d\n", result);
    }
}

void mqtt_subscription_cb(void *arg, err_t err)
{
    if (err == ERR_OK)
    {
        printf("Successfully subscribed to topic.\n");
    }
    else
    {
        printf("Subscription failed. Error: %d\n", err);
    }
}

// MQTT Connection Callback
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        printf("MQTT connected successfully.\n");

        // Call the send_message function with the desired message
        mqtt_send_message(client, "Hello from Raspberry Pi Pico W!");
        mqtt_subscribe(client, MQTT_TOPIC, 1, mqtt_subscription_cb, NULL);

        gpio_put(LED_G_PIN, 1); // Turn on green LED
        gpio_put(LED_R_PIN, 0); // Turn off red LED
    }
    else
    {
        printf("MQTT connection failed with status: %d\n", status);
        gpio_put(LED_G_PIN, 0); // Turn off green LED
        gpio_put(LED_R_PIN, 1); // Turn on red LED
    }
}

// Publish Callback
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t total_length)
{
    printf("Message arrived on topic: %s, length: %d\n", topic, total_length);
}

// Data Callback
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    // Debug: print received chunk
    printf("Received MQTT data chunk (%d bytes): ", len);
    for (int i = 0; i < len; i++)
    {
        putchar(data[i]);
    }
    putchar('\n');

    // Turn on both LEDs
    gpio_put(LED_G_PIN, 1); // Turn on green LED
    gpio_put(LED_R_PIN, 1); // Turn on red LED
}

// Initialize MQTT
void init_mqtt()
{
    ip_addr_t broker_ip;
    mqtt_client = mqtt_client_new();
    if (!mqtt_client)
    {
        printf("Failed to create MQTT client.\n");
        gpio_put(LED_G_PIN, 0); // Turn off green LED
        gpio_put(LED_R_PIN, 1); // Turn on red LED
        return;
    }

    if (!ip4addr_aton(MQTT_BROKER, &broker_ip))
    {
        printf("Failed to resolve broker IP address: %s\n", MQTT_BROKER);
        gpio_put(LED_G_PIN, 0); // Turn off green LED
        gpio_put(LED_R_PIN, 1); // Turn on red LED
        return;
    }

    err_t err = mqtt_client_connect(mqtt_client, &broker_ip, MQTT_PORT, mqtt_connection_cb, NULL, &mqtt_client_info);

    mqtt_set_inpub_callback(mqtt_client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

    if (err != ERR_OK)
    {
        printf("MQTT connection failed with error code: %d\n", err);
        gpio_put(LED_G_PIN, 0); // Turn off green LED
        gpio_put(LED_R_PIN, 1); // Turn on red LED
        return;
    }

    printf("Connecting to MQTT broker at %s:%d...\n", MQTT_BROKER, MQTT_PORT);
}

// Connect to Wi-Fi
void connect_to_wifi()
{
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Failed to connect to Wi-Fi.\n");
        gpio_put(LED_G_PIN, 0); // Turn off green LED
        gpio_put(LED_R_PIN, 1); // Turn on red LED
        while (1)
            sleep_ms(1000);
    }
    printf("Connected to Wi-Fi.\n");
    gpio_put(LED_G_PIN, 1); // Turn on green LED
    gpio_put(LED_R_PIN, 0); // Turn off red LED
}

int main()
{
    stdio_init_all();

    // Initialize Leds
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);

    if (cyw43_arch_init())
    {
        printf("Failed to initialize CYW43.\n");
        gpio_put(LED_G_PIN, 0); // Turn off green LED
        gpio_put(LED_R_PIN, 1); // Turn on red LED
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    connect_to_wifi();

    init_mqtt();

    while (true)
    {
        // Poll the Wi-Fi chip
        cyw43_arch_poll();

        // Aguarda um pequeno intervalo para reduzir o uso da CPU
        sleep_ms(10);
    }

    cyw43_arch_deinit();
    return 0;
}
