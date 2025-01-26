#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ss_oled.h"
#include "morse_decoder.h"

// Pin Definitions
#define LED_PIN 13
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

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
void init_oled()
{
    int rc = oledInit(&oled, OLED_128x64, 0x3c, 0, 0, 1, SDA_PIN, SCL_PIN, RESET_PIN, 1000000L);
    if (rc != OLED_NOT_FOUND)
    {
        printf("OLED initialized successfully.\n");
    }
    else
    {
        printf("OLED initialization failed.\n");
    }
}

// Update OLED
void update_oled(const char *phrase)
{
    oledFill(&oled, 0, 1);                                              // Clear display
    oledWriteString(&oled, 0, 0, 0, (char *)phrase, FONT_NORMAL, 0, 1); // Show full phrase
    oledDumpBuffer(&oled, NULL);                                        // Refresh OLED
    printf("OLED updated: %s\n", phrase);
}

// Shared GPIO Callback
void gpio_callback(uint gpio, uint32_t events)
{
    uint64_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && current_time - last_press_time_a > 200)
    { // Debounce Button A
        last_press_time_a = current_time;
        strncat((char *)morse_buffer, ".", sizeof(morse_buffer) - strlen(morse_buffer) - 1);
        printf("Dot detected. Buffer: %s\n", morse_buffer);
    }

    if (gpio == BUTTON_B_PIN && current_time - last_press_time_b > 200)
    { // Debounce Button B
        last_press_time_b = current_time;
        strncat((char *)morse_buffer, "_", sizeof(morse_buffer) - strlen(morse_buffer) - 1);
        printf("Dash detected. Buffer: %s\n", morse_buffer);
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(2000); // Allow USB initialization
    printf("Program started...\n");

    // GPIO Configuration
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_B_PIN);

    // Set Shared Interrupt
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Initialize OLED
    init_oled();

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
                printf("Enter detected. Clearing phrase buffer.\n");
                memset((char *)phrase_buffer, 0, sizeof(phrase_buffer)); // Clear phrase buffer
            }
            else
            {
                size_t len = strlen(phrase_buffer);
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
