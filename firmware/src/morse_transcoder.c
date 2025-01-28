#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "ssd1306.h"
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
#define LED_B_PIN 12
#define BTN_A_PIN 5

#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

uint8_t ssd[ssd1306_buffer_length];
struct render_area frame_area;

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

// inicializar o OLED
void init_oled() {
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
void display_message(char *line1, char *line2, char *line3) {
    // Limpa o buffer
    memset(ssd, 0, ssd1306_buffer_length);

    // Desenha as strings
    ssd1306_draw_string(ssd, 5, 0, line1);
    if (line2 != NULL) {
        ssd1306_draw_string(ssd, 5, 8, line2);
    }
    if (line3 != NULL) {
        ssd1306_draw_string(ssd, 5, 16, line3);
    }

    render_on_display(ssd, &frame_area);
}

void SinalAberto() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    display_message("SINAL ABERTO", "ATRAVESSAR COM", "CUIDADO");
}

void SinalAtencao() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    display_message("SINAL DE", "ATENCAO", "PREPARE-SE");
}

void SinalFechado() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    display_message("SINAL FECHADO", "AGUARDE", NULL);
}

// aguarda entrada
int WaitWithRead(int timeMS) {
    for (int i = 0; i < timeMS; i += 100) {
        int A_state = !gpio_get(BTN_A_PIN);
        if (A_state == 1) {
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}

int main() {
    stdio_init_all();

    // Inicialização de LEDs e botão
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    init_oled();

    while (true) {
        SinalAberto();
        int A_state = WaitWithRead(8000);

        if (A_state) {
            SinalAtencao();
            sleep_ms(5000);

            SinalFechado();
            sleep_ms(10000);
        } else {
            SinalAtencao();
            sleep_ms(2000);

            SinalFechado();
            sleep_ms(8000);
        }
    }
}