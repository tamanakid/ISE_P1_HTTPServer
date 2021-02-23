#include "stdint.h"


#define PORT_LED 1
#define PIN_LED0 18
#define PIN_LED1 20
#define PIN_LED2 21
#define PIN_LED3 23


typedef struct {
  uint8_t led3;
  uint8_t led2;
  uint8_t led1;
  uint8_t led0;
} ledsStatus;

typedef struct {
  uint8_t led3;
  uint8_t led2;
  uint8_t led1;
  uint8_t led0;
  uint8_t override;
} ledsStatusOverride;

extern ledsStatusOverride override_status;
extern ledsStatus leds_status;


void hardware_initialize(void);

void leds_initialize(void);
void adc_initialize(void);

void lcd_initialize(void);

void write_lcd(char lcd_text[2][21]);
