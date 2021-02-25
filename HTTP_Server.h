#include "stdint.h"


#define PORT_LED 1
#define PIN_LED0 18
#define PIN_LED1 20
#define PIN_LED2 21
#define PIN_LED3 23
#define CLEAR_STRING "                        "


extern char lcd_text[2][20+1];

/**
 * Functions defined in Hardware.c
 */

/* LED-related */
void leds_initialize(void);
void leds_browser_set(char led_id, uint8_t is_checked);
uint8_t leds_running_set(uint8_t current_led);

/* ADC-related */
void adc_initialize(void);
uint16_t adc_read(void);

/* LCD-related */
void lcd_initialize(void);
void lcd_write(void);
