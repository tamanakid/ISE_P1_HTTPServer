#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "HTTP_Server.h"


void hardware_initialize() {
	GPIO_SetDir(PORT_LED, PIN_LED0, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED1, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED2, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED3, GPIO_DIR_OUTPUT);
}
