#include "GPIO_LPC17xx.h"
#include "LPC17xx.h"

#include "HTTP_Server.h"



/* RGB LED Initialization */
void rgb_initialize (void) {	
	GPIO_SetDir(PORT_RGB, PIN_GREEN, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_RGB, PIN_RED, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_RGB, PIN_BLUE, GPIO_DIR_OUTPUT);
	
	GPIO_PinWrite (PORT_RGB, PIN_RED, 1);
	GPIO_PinWrite (PORT_RGB, PIN_GREEN, 1);
	GPIO_PinWrite (PORT_RGB, PIN_BLUE, 1);
}



void rgb_set_status (uint16_t adc_value) {
	uint16_t adc_value_8t = adc_value >> 4;
	
	uint8_t current_adc_threshold = rgb_get_flash_threshold();
	
	if (adc_value_8t > current_adc_threshold) {
		GPIO_PinWrite (PORT_RGB, PIN_RED, 1);
		GPIO_PinWrite (PORT_RGB, PIN_GREEN, 1);
		GPIO_PinWrite (PORT_RGB, PIN_BLUE, 0);
	} else {
		GPIO_PinWrite (PORT_RGB, PIN_RED, 0);
		GPIO_PinWrite (PORT_RGB, PIN_GREEN, 1);
		GPIO_PinWrite (PORT_RGB, PIN_BLUE, 1);
	}
}



uint8_t rgb_get_flash_threshold (void) {
	uint8_t dest_array[1] = { 0 };
	
	flash_read_array(FLASH_ADDR_RGB_THRESHOLD, dest_array, 1);

	return dest_array[0];
}
