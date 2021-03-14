#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "rtc.h"

#include "HTTP_Server.h"


/* Definitions */


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
