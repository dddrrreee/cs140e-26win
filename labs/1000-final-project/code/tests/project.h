#ifndef __PROJECT_THREADS_H__
#define __PROJECT_THREADS_H__

#include <stdint.h>

#include "../net-stack/inet.h"

// No destination mac. ARP-filled

#define PI_IP ((uint8_t[4]){192, 168, 0, 3})
#define PI_MAC ((uint8_t[6]){76, 67, 67, 67, 67, 67})
#define DEST_IP ((uint8_t[4]){1,2,3,4})

#define ONBOARD_LED_PORT 40000
#define PARTHIV_LED_PORT 40001

// Any message toggles the LED
void parthiv_led_handler(const uint8_t* src_ip, uint16_t src_port, uint16_t dest_port,
    const uint8_t* data, uint16_t len); // *


// Message for morse code (or just blinking)
void onboard_led_handler(const uint8_t* src_ip, uint16_t src_port, uint16_t dest_port,
    const uint8_t* data, uint16_t len); // *
    
// Parthiv LED blinking on and off is port 40001
void parthiv_led_thread(void* arg);
        
// Onboard LED blinking in morse code
void onboard_led_thread(void* arg); // *
void network_thread(void* arg); // *
void typing_thread(void* arg);



#endif