#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "ss_oled.h"
#include "morse_decoder.h"

// Pin Definitions
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Wi-Fi Configurations
#define WIFI_SSID "Your-WiFi-SSID"
#define WIFI_PASSWORD "Your-WiFi-Password"

// OLED Configuration
#define SDA_PIN 16
#define SCL_PIN 17
#define RESET_PIN -1
SSOLED oled;

// Global Variables
volatile char morse_buffer[64] = {0};   // Holds the current Morse code sequence
volatile char phrase_buffer[256] = {0}; // Holds the entire phrase
volatile uint64_t last_press_time_a = 0;
volatile uint64_t last_press_time_b = 0;

// Initialize OLED
void init_oled() {
    int rc = oledInit(&oled, OLED_128x64, 0x3c, 0, 0, 1, SDA_PIN, SCL_PIN, RESET_PIN, 1000000L);
    if (rc == OLED_NOT_FOUND) {
        printf("OLED initialization failed.\n");
    } else {
        printf("OLED initialized successfully.\n");
    }
}

// Update OLED Display
void update_oled(const char *phrase) {
    oledFill(&oled, 0, 1);                                              // Clear display
    oledWriteString(&oled, 0, 0, 0, (char *)phrase, FONT_NORMAL, 0, 1); // Show phrase
    oledDumpBuffer(&oled, NULL);                                        // Refresh OLED
    printf("OLED updated: %s\n", phrase);
}

// Shared GPIO Callback
void gpio_callback(uint gpio, uint32_t events) {
    uint64_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && current_time - last_press_time_a > 200) { // Debounce Button A
        last_press_time_a = current_time;
        strncat((char *)morse_buffer, ".", sizeof(morse_buffer) - strlen(morse_buffer) - 1);
        printf("Dot detected. Morse Buffer: %s\n", morse_buffer);
    }

    if (gpio == BUTTON_B_PIN && current_time - last_press_time_b > 200) { // Debounce Button B
        last_press_time_b = current_time;
        strncat((char *)morse_buffer, "_", sizeof(morse_buffer) - strlen(morse_buffer) - 1);
        printf("Dash detected. Morse Buffer: %s\n", morse_buffer);
    }
}

// Connect to Wi-Fi
void connect_to_wifi() {
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to Wi-Fi.\n");
    } else {
        printf("Wi-Fi connected successfully!\n");
        uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP Address: %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }
}

int main() {
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
    if (cyw43_arch_init()) {
        printf("Failed to initialize Wi-Fi chip.\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();
    connect_to_wifi();

    while (true) {
        // Decode Morse after inactivity
        if (strlen((const char *)morse_buffer) > 0 &&
            to_ms_since_boot(get_absolute_time()) - last_press_time_a > 1000 &&
            to_ms_since_boot(get_absolute_time()) - last_press_time_b > 1000) {
            char decoded_char = decode_morse((const char *)morse_buffer);
            if (decoded_char == '\n') { // Enter detected
                printf("Enter detected. Clearing phrase buffer.\n");
                memset((char *)phrase_buffer, 0, sizeof(phrase_buffer)); // Clear phrase buffer
            } else {
                size_t len = strlen(phrase_buffer);
                if (len < sizeof(phrase_buffer) - 1) {
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
