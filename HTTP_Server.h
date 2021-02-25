#include "stdint.h"


#define PORT_LED 1
#define PIN_LED0 18
#define PIN_LED1 20
#define PIN_LED2 21
#define PIN_LED3 23


void hardware_initialize(void);

void leds_initialize(void);
void adc_initialize(void);

void lcd_initialize(void);

void write_lcd(char lcd_text[2][21]);
