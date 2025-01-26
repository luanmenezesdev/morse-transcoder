// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/i2c.h"
// #include "hardware/timer.h"
// #include "pico/cyw43_arch.h"

// // I2C defines
// // This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// // Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// #define I2C_PORT i2c0
// #define I2C_SDA 8
// #define I2C_SCL 9

// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//     // Put your timeout handler code in here
//     return 0;
// }




// int main()
// {
//     stdio_init_all();

//     // Initialise the Wi-Fi chip
//     if (cyw43_arch_init()) {
//         printf("Wi-Fi init failed\n");
//         return -1;
//     }

//     // I2C Initialisation. Using it at 400Khz.
//     i2c_init(I2C_PORT, 400*1000);
    
//     gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
//     gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
//     gpio_pull_up(I2C_SDA);
//     gpio_pull_up(I2C_SCL);
//     // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

//     // Timer example code - This example fires off the callback after 2000ms
//     add_alarm_in_ms(2000, alarm_callback, NULL, false);
//     // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

//     // Enable wifi station
//     cyw43_arch_enable_sta_mode();

//     printf("Connecting to Wi-Fi...\n");
//     if (cyw43_arch_wifi_connect_timeout_ms("Your Wi-Fi SSID", "Your Wi-Fi Password", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
//         printf("failed to connect.\n");
//         return 1;
//     } else {
//         printf("Connected.\n");
//         // Read the ip address in a human readable way
//         uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
//         printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
//     }

//     while (true) {
//         printf("Hello, world!\n");
//         sleep_ms(1000);
//     }
// }
