#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"

#include "ssd1306.h"
#include "morse_decoder.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/netif.h"

#define GPIO_IRQ_EDGE_RISE (1u << 3) // Rising edge
#define GPIO_IRQ_EDGE_FALL (1u << 4) // Falling edge

// Pin Definitions
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

// Wi-Fi Configurations
#define WIFI_SSID "MENEZES(giganet)"
#define WIFI_PASSWORD "17134529"

// MQTT Configurations
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_TOPIC "morse-transcoder/chat"

// OLED Configuration
ssd1306_t oled;
uint8_t oled_buffer[ssd1306_buffer_length];
struct render_area frame_area;

// Global Variables
volatile char morse_buffer[64] = {0};   // Holds the current Morse code sequence
volatile char phrase_buffer[256] = {0}; // Holds the entire phrase
volatile uint64_t last_press_time_a = 0;
volatile uint64_t last_press_time_b = 0;

mqtt_client_t *mqtt_client;
ip_addr_t broker_addr;

// MQTT Connection Callback
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        printf("MQTT connected successfully.\n");
    }
    else
    {
        printf("MQTT connection failed with status %d.\n", status);
    }
}

// Publish a message via MQTT
void mqtt_publish_message(const char *message)
{
    err_t err = mqtt_publish(mqtt_client, MQTT_TOPIC, message, strlen(message), 0, 0, NULL, NULL);
    if (err == ERR_OK)
    {
        printf("Message published: %s\n", message);
    }
    else
    {
        printf("Failed to publish message. Error: %d\n", err);
    }
}

// Initialize MQTT
void init_mqtt()
{
    mqtt_client = mqtt_client_new();
    if (mqtt_client == NULL)
    {
        printf("Failed to create MQTT client.\n");
        return;
    }

    ipaddr_aton(MQTT_BROKER, &broker_addr);
    mqtt_client_connect(mqtt_client, &broker_addr, 1883, mqtt_connection_cb, NULL, NULL);
}

// Initialize OLED
void init_oled()
{
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    ssd1306_init();

    frame_area.start_column = 0;
    frame_area.end_column = ssd1306_width - 1;
    frame_area.start_page = 0;
    frame_area.end_page = ssd1306_n_pages - 1;

    calculate_render_area_buffer_length(&frame_area);

    memset(oled_buffer, 0, ssd1306_buffer_length);
    render_on_display(oled_buffer, &frame_area);
}

// Update OLED Display
void update_oled(const char *phrase)
{
    memset(oled_buffer, 0, ssd1306_buffer_length);
    ssd1306_draw_string(oled_buffer, 0, 0, (char *)phrase);
    ssd1306_send_data(&oled);
    printf("OLED updated: %s\n", phrase);

    render_on_display(oled_buffer, &frame_area);
}

// Shared GPIO Callback
void gpio_callback(uint gpio, uint32_t events)
{
    uint64_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && current_time - last_press_time_a > 200)
    { // Debounce Button A
        last_press_time_a = current_time;
        strncat((char *)morse_buffer, ".", sizeof(morse_buffer) - strlen((const char *)morse_buffer) - 1);
        printf("Dot detected. Morse Buffer: %s\n", morse_buffer);
    }

    if (gpio == BUTTON_B_PIN && current_time - last_press_time_b > 200)
    { // Debounce Button B
        last_press_time_b = current_time;
        strncat((char *)morse_buffer, "_", sizeof(morse_buffer) - strlen((const char *)morse_buffer) - 1);
        printf("Dash detected. Morse Buffer: %s\n", morse_buffer);
    }
}

// Connect to Wi-Fi
void connect_to_wifi()
{
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect to Wi-Fi.\n");
    }
    else
    {
        printf("Wi-Fi connected successfully!\n");

        if (netif_default)
        {
            const ip4_addr_t *ip = netif_ip4_addr(netif_default);
            if (ip)
            {
                printf("IP Address: %s\n", ip4addr_ntoa(ip));
            }
            else
            {
                printf("Failed to retrieve IP address.\n");
            }
        }
        else
        {
            printf("No default network interface available.\n");
        }
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(2000); // Allow USB initialization
    printf("Program started...\n");

    // Initialize GPIO Pins
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_B_PIN);

    // Set Shared GPIO Interrupt
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Initialize OLED
    init_oled();

    // Initialize Wi-Fi
    if (cyw43_arch_init())
    {
        printf("Failed to initialize Wi-Fi chip.\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();
    connect_to_wifi();

    // Initialize MQTT
    init_mqtt();

    while (true)
    {
        // Decode Morse after inactivity
        if (strlen((const char *)morse_buffer) > 0 &&
            to_ms_since_boot(get_absolute_time()) - last_press_time_a > 1000 &&
            to_ms_since_boot(get_absolute_time()) - last_press_time_b > 1000)
        {
            char decoded_char = decode_morse((const char *)morse_buffer);
            if (decoded_char == '\n')
            { // Enter detected
                printf("Enter detected. Publishing message: %s\n", phrase_buffer);
                mqtt_publish_message((const char *)phrase_buffer);       // Publish the full phrase
                memset((char *)phrase_buffer, 0, sizeof(phrase_buffer)); // Clear phrase buffer
            }
            else
            {
                size_t len = strlen((const char *)phrase_buffer);
                if (len < sizeof(phrase_buffer) - 1)
                {
                    phrase_buffer[len] = decoded_char; // Append character
                    phrase_buffer[len + 1] = '\0';
                }
            }
            update_oled((const char *)phrase_buffer);
            memset((char *)morse_buffer, 0, sizeof(morse_buffer)); // Clear Morse buffer
        }
        sleep_ms(10); // Reduce CPU usage
    }

    return 0;
}
