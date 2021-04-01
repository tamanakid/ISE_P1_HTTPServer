#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "rtc.h"

#include "HTTP_Server.h"


#define FLASH_ADDR_LEDS	0x0001800A


/* Definitions */

bool leds_running = true;
bool led2_blink = false;
bool led3_blink = false;
uint8_t leds_on = 0;



/**
 * Thread: LEDs handler
 */
void thread_leds (void const *arg) {
	
	uint8_t current_led = 0;
	
  while(1) {
		if (led3_blink == true) {
			leds_blink_led3();
			led3_blink = false;
    }
		if (led2_blink == true) {
			leds_blink_led2();
			led2_blink = false;
		}
		if (leds_running == true) {
			current_led = leds_running_set(current_led);
    } else {
			leds_restore_browser_config();
		}
    osDelay (100);
  }
}



/**
 * Initializes all 4 LEDs as GPIO elements.
 */
void leds_initialize(void) {
	GPIO_SetDir(PORT_LED, PIN_LED0, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED1, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED2, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED3, GPIO_DIR_OUTPUT);
	
	leds_get_flash_status();
}



/**
 * Set LEDs status variables according to Flash register (FLASH_ADDR_LEDS)
 */
void leds_get_flash_status (void) {
	uint8_t dest_array[11] = { 0 };
	
	flash_read_array(0x00018000, dest_array, 11);

	leds_running = (dest_array[10] & 0x10) ? false : true;
	leds_on = dest_array[10] & 0x0F; // (leds_running == false) ? dest_array[10] & 0x0F : 0x00;
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


/**
 * Function called upon SNTP response to blink the LED2
 */
void leds_blink_led2() {
	int i;
	bool led2_on = true;

	GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
	GPIO_PinWrite(PORT_LED, PIN_LED2, 1);
	GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
	GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
	
	for (i = 0; i < 6; i++) {
		led2_on = !led2_on;
		GPIO_PinWrite(PORT_LED, PIN_LED2, led2_on);
		osDelay(300);
	}
	
	GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
}



/**
 * Function called upon RTC interrupt to blink the LED3 for 5 seconds
 */
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



/**
 * Restores browser-set leds configuration after led2/led3 blinking is finished.
 */
void leds_restore_browser_config() {
	if (leds_on & 0x08) {
		GPIO_PinWrite(PORT_LED, PIN_LED3, 1);
	}
	if (leds_on & 0x04) {
		GPIO_PinWrite(PORT_LED, PIN_LED2, 1);
	}
	if (leds_on & 0x02) {
		GPIO_PinWrite(PORT_LED, PIN_LED1, 1);
	}
	if (leds_on & 0x01) {
		GPIO_PinWrite(PORT_LED, PIN_LED0, 1);
	}
}
