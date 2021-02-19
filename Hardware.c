#include <string.h>

#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#include "HTTP_Server.h"
#include "lcd.h"


void hardware_initialize() {	
	GPIO_SetDir(PORT_LED, PIN_LED0, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED1, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED2, GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_LED, PIN_LED3, GPIO_DIR_OUTPUT);
}


void lcd_initialize() {
	init_lcd();
  reset_lcd();
	/*
	char clearString[7] = "P";
  escribe_frase_L1(clearString, sizeof(clearString));
  escribe_frase_L2(clearString, sizeof(clearString));
  copy_to_lcd();
	*/
}


void write_to_lcd(char lcd_text[2][21]) {
	static int on = 0;
	int len1 = sizeof(lcd_text[0]);
	int len2 = sizeof(lcd_text[1]);
	escribe_frase_L1(lcd_text[0], len1);
	escribe_frase_L2(lcd_text[1], len2);
	copy_to_lcd();
	on = !on;
	GPIO_PinWrite(PORT_LED, PIN_LED0, on);
}
