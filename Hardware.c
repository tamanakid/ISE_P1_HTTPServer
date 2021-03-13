#include <string.h>
#include <stdio.h>

#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "lcd.h"
#include "rtc.h"

#include "HTTP_Server.h"


extern char lcd_text[2][30+1];

extern uint8_t leds_status;
extern bool LED2blink;


/**
 * Initializes all 4 LEDs as GPIO elements.
 */
void leds_initialize(void) {
	GPIO_SetDir(PORT_LED, PIN_LED0, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED1, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED2, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED3, GPIO_DIR_OUTPUT);
}


/**
 * Called for each led when "Browser" is selected as LED Control.
 * @param char led_id - reference to LED from .cgi
 * @param uint8_t is_checked - whether the LED is to be on/off.
 */
void leds_browser_set(char led_id, uint8_t is_checked) {
	if (led_id == '3') {
		GPIO_PinWrite(PORT_LED, PIN_LED3, is_checked);
	} else if (led_id == '2') {
		GPIO_PinWrite(PORT_LED, PIN_LED2, is_checked);
	} else if (led_id == '1') {
		GPIO_PinWrite(PORT_LED, PIN_LED1, is_checked);
	} else if (led_id == '0') {
		GPIO_PinWrite(PORT_LED, PIN_LED0, is_checked);
	}
}


/**
 * Called in intervals when "Running Lights" is selected as LED Control.
 * @param uint8_t current_led - BlinkLed Thread variable
 * @return uint8_t - new led position
 */
uint8_t leds_running_set(uint8_t current_led) {
	switch (current_led) {
		case 0:
			GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED0, 1);
			break;
		case 1:
			GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED1, 1);
			GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
			break;
		case 2:
			GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED2, 1);
			GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
			break;
		case 3:
			GPIO_PinWrite(PORT_LED, PIN_LED3, 1);
			GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
			GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
			return 0;
	}
	return ++current_led;
}



void leds_blink_led2() {
	int i;
	bool led2_on = true;

	GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
	GPIO_PinWrite(PORT_LED, PIN_LED2, 1);
	GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
	GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
	
	for (i = 0; i < 5; i++) {
		led2_on = !led2_on;
		GPIO_PinWrite(PORT_LED, PIN_LED2, led2_on);
		osDelay(300);
	}
	
	GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
}



void leds_blink_led3() {
	bool led3_on = true;

	GPIO_PinWrite(PORT_LED, PIN_LED3, 1);
	GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
	GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
	GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
	
	do {
		rtc_get_full_time();
		led3_on = !led3_on;
		GPIO_PinWrite(PORT_LED, PIN_LED3, led3_on);
		osDelay(100);
	} while (rtc_seconds < 5);
	
	GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
}



void leds_restore_browser_config() {
	if (leds_status & 0x08) {
		GPIO_PinWrite(PORT_LED, PIN_LED3, 1);
	}
	if (leds_status & 0x04) {
		GPIO_PinWrite(PORT_LED, PIN_LED2, 1);
	}
	if (leds_status & 0x02) {
		GPIO_PinWrite(PORT_LED, PIN_LED1, 1);
	}
	if (leds_status & 0x01) {
		GPIO_PinWrite(PORT_LED, PIN_LED0, 1);
	}
}



/**
 * Configure AD[0] peripheral to perform Analog-to-Digital Conversion
 */
void adc_initialize(void) {
  LPC_PINCON->PINSEL3 |= (3 << 28);
  LPC_SC->PCONP |= (1 << 12);
  LPC_ADC->ADCR &= ~(1UL << 0);
  LPC_ADC->ADCR |= (1 << 21);
}


/**
 * Execute ADC read from previously enabled AD[0] peripheral
 * @return uint16_t - Voltage read from the variable resistor
 */
uint16_t adc_read(void) {
	uint16_t buffer_adc_sensor;

	LPC_ADC->ADCR |= 1UL << 4;
	LPC_ADC->ADCR |= 1UL << 24;
	while((LPC_ADC->ADGDR >> 31) == 0);
	buffer_adc_sensor = (LPC_ADC->ADGDR & (0xFFF << 4)) >> 4;
	LPC_ADC->ADCR &= ~(1UL << 24);
	
	return buffer_adc_sensor;
}



/**
 * Configure the LCD and initialize to clear values
 */
void lcd_initialize(void) {
	init_lcd();
  reset_lcd();
	sprintf (lcd_text[0], "");
  sprintf (lcd_text[1], "");
}


/**
 * Write values read from the cgi POST request to the LCD
 */
void lcd_write(void) {
	char lcd_buf[24+1];
	strcpy(lcd_buf, CLEAR_STRING);
	
	strcpy(lcd_buf, lcd_text[0]);
	escribe_frase_L1(lcd_buf, sizeof(lcd_buf));
	
	strcpy(lcd_buf, lcd_text[1]);
	escribe_frase_L2(lcd_buf, sizeof(lcd_buf));
	
	copy_to_lcd();
}



/**
 * Configure the Joystick to trigger interrupts upon center-key press
 */
void joystick_initialize(void) {
	PIN_Configure(PORT_PINS, PIN_JST_C, PIN_FUNC_0, PIN_PINMODE_PULLDOWN, PIN_PINMODE_NORMAL);

	LPC_GPIOINT->IO0IntEnR |= (1UL << PIN_JST_C);

	NVIC_EnableIRQ(EINT3_IRQn);
}


/**
 * GPIO ISR (Joystick Center press): Resets time to default date upon
 */
void EINT3_IRQHandler() {
	
	if (LPC_GPIOINT->IO0IntStatR & (1 << PIN_JST_C)) {
		rtc_reset_full_time();
  }
	LPC_GPIOINT->IO0IntClr |= (1 << PIN_JST_C);
}
