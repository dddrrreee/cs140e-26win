#include "project.h"
#include "rpi-thread.h"

int network_thread_enable = 1;
// static const parthiv_led_enable = 1;
int parthiv_led_state = 0;
int onboard_led_enable = 1;


void parthiv_led_handler(const uint8_t* src_ip, uint16_t src_port, uint16_t dest_port,
    const uint8_t* data, uint16_t len) {

    printk("Entered Parthiv LED handler with %d bytes of data\n", len);
    
    // Turns on and off the LED, or just toggles
    // if (len > 0) {
    //     parthiv_led_state = *data;
    //     printk("Setting LED to %d", *data);
    //     const char* led_response = "Received Parthiv LED command\n";
    //     inet_udp_send(parthiv_led_port, src_port, src_ip, led_response, strlen(led_response));
    // }
    // else {
    parthiv_led_state = !parthiv_led_state;
    printk("Toggling LED to %d", parthiv_led_state);
    const char* led_response = "Toggling LED\n";
    inet_udp_send(PARTHIV_LED_PORT, src_port, src_ip, led_response, strlen(led_response));
    // }
}

void onboard_led_handler(const uint8_t* src_ip, uint16_t src_port, uint16_t dest_port,
    const uint8_t* data, uint16_t len) {

    printk("Entered Onboard LED handler with %d bytes of data. Toggling blink\n", len);

    onboard_led_enable = !onboard_led_enable;

    const char* msg = "Onboard LED handler response\n";
    inet_udp_send(ONBOARD_LED_PORT, src_port, src_ip, msg, strlen(msg));
}




void onboard_led_thread(void* arg) {
    gpio_set_output(GPIO_PI_LED);

    unsigned i = 0;
    while (1) {

        while (!onboard_led_enable) {
            rpi_yield();
        }
        
        gpio_set_on(GPIO_PI_LED);

        uint32_t start = timer_get_usec();
        while (timer_get_usec() - start < 1000000) {
            rpi_yield();
        }

        gpio_set_off(GPIO_PI_LED);

        start = timer_get_usec();
        while (timer_get_usec() - start < 1000000) {
            rpi_yield();
        }
    }
}


void parthiv_led_thread(void* arg) {
    gpio_set_output(GPIO_HAT_LED);

    while (1) {

        if (parthiv_led_state) { // Toggled by handler
            gpio_set_on(GPIO_HAT_LED);
        } else {
            gpio_set_off(GPIO_HAT_LED);
        }
        rpi_yield();
    }
}

void network_thread(void* arg) {
    while(1) {
        inet_poll_frame(0);
        rpi_yield();
    }
}



void typing_thread(void* arg) {
    // If a certain character is sent, stop the network thread
    printk("Typing thread\n");
    while(1) {
        if (uart_has_data()) {
            printk("Stuff");
            
        }


        rpi_yield();
    };
}