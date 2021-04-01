#include "LPC17xx.h"

#include "HTTP_Server.h"


/* Definitions */
uint8_t adc_threshold = ADC_THRESHOLD;


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
	
	rgb_set_status(buffer_adc_sensor);
	
	return buffer_adc_sensor;
}



uint8_t adc_get_flash_threshold (void) {
	uint8_t dest_array[1] = { 0 };
	
	flash_read_array(FLASH_ADDR_RGB_THRESHOLD, dest_array, 1);

	return dest_array[0];
}
