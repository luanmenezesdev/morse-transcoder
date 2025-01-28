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
#define WIFI_SSID "MENEZES(giganet)"
#define WIFI_PASSWORD "17134529"

// MQTT Configuration
#define MQTT_BROKER "52.57.5.85"
#define MQTT_TOPIC "morse-transcoder/chat"

#define LED_R_PIN 13
#define LED_G_PIN 11
#define BTN_A_PIN 5
#define BTN_B_PIN 6

#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

uint8_t ssd[ssd1306_buffer_length];
struct render_area frame_area;

// Global Variables
volatile char morse_buffer[64] = {0};   // Holds the current Morse code sequence
volatile char phrase_buffer[256] = {0}; // Holds the entire phrase
char last_received_message[256] = {0};  // Holds the last received message
volatile uint64_t last_press_time_a = 0;
volatile uint64_t last_press_time_b = 0;

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
    }
    else
    {
        printf("Failed to publish message. Error: %d\n", result);
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

// MQTT Incoming Message Callback
void mqtt_incoming_message_cb(void *arg, const char *topic, u32_t tot_len)
{
    const struct pbuf *p = (const struct pbuf *)arg;

    if (p)
    {
        char received_message[256] = {0};
        size_t len = (tot_len < sizeof(received_message) - 1) ? tot_len : sizeof(received_message) - 1;
        memcpy(received_message, p->payload, len);
        received_message[len] = '\0'; // Null-terminate

        // Split the message into sender and content
        char *delimiter = strchr(received_message, ':');
        if (delimiter)
        {
            *delimiter = '\0'; // Replace ':' with '\0' to split sender and content
            char *sender = received_message;
            char *content = delimiter + 1;

            // Trim whitespace from content
            while (*content == ' ')
                content++;

            printf("Sender: %s, Content: %s\n", sender, content);

            // Only display if the sender is not "pico"
            if (strcmp(sender, "pico") != 0)
            {
                strncpy((char *)last_received_message, content, sizeof(last_received_message) - 1);
                last_received_message[sizeof(last_received_message) - 1] = '\0'; // Null-terminate
                printf("Displaying received message: %s\n", last_received_message);
            }
            else
            {
                printf("Ignored own message.\n");
            }
        }
        else
        {
            printf("Malformed message received: %s\n", received_message);
        }
    }
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

    mqtt_set_inpub_callback(mqtt_client, mqtt_incoming_message_cb, NULL, NULL);

    err_t err = mqtt_client_connect(mqtt_client, &broker_ip, MQTT_PORT, mqtt_connection_cb, NULL, &mqtt_client_info);

    if (err != ERR_OK)
    {
        printf("MQTT connection failed with error code: %d\n", err);
        gpio_put(LED_G_PIN, 0); // Turn off green LED
        gpio_put(LED_R_PIN, 1); // Turn on red LED
        return;
    }

    printf("Connecting to MQTT broker at %s:%d...\n", MQTT_BROKER, MQTT_PORT);
}

// inicializar o OLED
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

    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

// Função para exibir mensagens no OLED
void display_message(char *line1)
{
    // Limpa o buffer
    memset(ssd, 0, ssd1306_buffer_length);

    // Desenha as strings
    ssd1306_draw_string(ssd, 5, 0, line1);

    render_on_display(ssd, &frame_area);
}

// Shared GPIO Callback
void gpio_callback(uint gpio, uint32_t events)
{
    uint64_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN_A_PIN && current_time - last_press_time_a > 200)
    { // Debounce Button A
        last_press_time_a = current_time;
        strncat((char *)morse_buffer, ".", sizeof(morse_buffer) - strlen((const char *)morse_buffer) - 1);
        printf("Dot detected. Morse Buffer: %s\n", morse_buffer);
    }

    if (gpio == BTN_B_PIN && current_time - last_press_time_b > 200)
    { // Debounce Button B
        last_press_time_b = current_time;
        strncat((char *)morse_buffer, "_", sizeof(morse_buffer) - strlen((const char *)morse_buffer) - 1);
        printf("Dash detected. Morse Buffer: %s\n", morse_buffer);
    }
}

int main()
{
    stdio_init_all();

    // Inicialização de LEDs e botão
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);

    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    // Set Shared GPIO Interrupt
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    init_oled();

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
        cyw43_arch_poll();

        // Decode Morse after inactivity
        if (strlen((const char *)morse_buffer) > 0 &&
            to_ms_since_boot(get_absolute_time()) - last_press_time_a > 1000 &&
            to_ms_since_boot(get_absolute_time()) - last_press_time_b > 1000)
        {
            char decoded_char = decode_morse((const char *)morse_buffer);
            if (decoded_char == '\n')
            { // Enter detected
                printf("Enter detected. Publishing message: %s\n", phrase_buffer);
                mqtt_send_message(mqtt_client, (const char *)phrase_buffer); // Publish the full phrase
                memset((char *)phrase_buffer, 0, sizeof(phrase_buffer));     // Clear phrase buffer
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
            display_message((char *)phrase_buffer);
            memset((char *)morse_buffer, 0, sizeof(morse_buffer)); // Clear Morse buffer
        }

        if (strlen((const char *)last_received_message) > 0 && !(strlen((const char *)phrase_buffer) > 0))
        {
            display_message((char *)last_received_message); // Display the last received message
        }

        sleep_ms(10);
    }

    cyw43_arch_deinit();
    return 0;
}