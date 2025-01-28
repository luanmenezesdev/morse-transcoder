#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

// Wi-Fi Credentials
#define WIFI_SSID "MENEZES(giganet)"
#define WIFI_PASSWORD "17134529"

// MQTT Configuration
#define MQTT_BROKER "52.57.5.85"
#define MQTT_TOPIC "morse-transcoder/chat"

// LED Pins
#define GREEN_LED_PIN 11
#define RED_LED_PIN 13

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

// MQTT Connection Callback
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        printf("MQTT connected successfully.\n");
        const char *message = "pico: Hello from Raspberry Pi Pico W!";
        err_t result = mqtt_publish(client, MQTT_TOPIC, message, strlen(message), 0, 0, NULL, NULL);
        if (result == ERR_OK)
        {
            printf("Message published: %s\n", message);
        }
        else
        {
            printf("Failed to publish message. Error: %d\n", result);
        }
        gpio_put(GREEN_LED_PIN, 1); // Turn on green LED
        gpio_put(RED_LED_PIN, 0);   // Turn off red LED
    }
    else
    {
        printf("MQTT connection failed with status: %d\n", status);
        gpio_put(GREEN_LED_PIN, 0); // Turn off green LED
        gpio_put(RED_LED_PIN, 1);   // Turn on red LED
    }
}

// Connect to Wi-Fi
void connect_to_wifi()
{
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Failed to connect to Wi-Fi.\n");
        gpio_put(GREEN_LED_PIN, 0); // Turn off green LED
        gpio_put(RED_LED_PIN, 1);   // Turn on red LED
        while (1)
            sleep_ms(1000);
    }
    printf("Connected to Wi-Fi.\n");
    gpio_put(GREEN_LED_PIN, 1); // Turn on green LED
    gpio_put(RED_LED_PIN, 0);   // Turn off red LED
}

// Initialize MQTT
void init_mqtt()
{
    ip_addr_t broker_ip;
    mqtt_client = mqtt_client_new();
    if (!mqtt_client)
    {
        printf("Failed to create MQTT client.\n");
        gpio_put(GREEN_LED_PIN, 0); // Turn off green LED
        gpio_put(RED_LED_PIN, 1);   // Turn on red LED
        return;
    }

    if (!ip4addr_aton(MQTT_BROKER, &broker_ip))
    {
        printf("Failed to resolve broker IP address: %s\n", MQTT_BROKER);
        gpio_put(GREEN_LED_PIN, 0); // Turn off green LED
        gpio_put(RED_LED_PIN, 1);   // Turn on red LED
        return;
    }

    err_t err = mqtt_client_connect(mqtt_client, &broker_ip, MQTT_PORT, mqtt_connection_cb, NULL, &mqtt_client_info);

    if (err != ERR_OK)
    {
        printf("MQTT connection failed with error code: %d\n", err);
        gpio_put(GREEN_LED_PIN, 0); // Turn off green LED
        gpio_put(RED_LED_PIN, 1);   // Turn on red LED
        return;
    }

    printf("Connecting to MQTT broker at %s:%d...\n", MQTT_BROKER, MQTT_PORT);
}

int main()
{
    stdio_init_all();

    // Initialize LEDs
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_put(GREEN_LED_PIN, 0);

    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_put(RED_LED_PIN, 0);

    if (cyw43_arch_init())
    {
        printf("Failed to initialize CYW43.\n");
        gpio_put(GREEN_LED_PIN, 0); // Turn off green LED
        gpio_put(RED_LED_PIN, 1);   // Turn on red LED
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    connect_to_wifi();
    init_mqtt();

    // Keep polling to handle MQTT events
    while (1)
    {
        cyw43_arch_poll();
        sleep_ms(10);
    }

    cyw43_arch_deinit();
    return 0;
}