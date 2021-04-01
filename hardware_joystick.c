#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"

#include "rtc.h"

#include "HTTP_Server.h"



/* Definitions */


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
