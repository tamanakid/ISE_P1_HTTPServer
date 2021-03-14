#include <string.h>
#include <stdio.h>

#include "lcd.h"

#include "HTTP_Server.h"



/* Definitions */

bool lcd_update;
char lcd_text[2][30+1];



/**
 * Thread: LCD display handler
 */
void thread_lcd (void const *arg) {  
  lcd_update = true;

  while(1) {
    if (lcd_update == true) {
			lcd_write();
			lcd_update = false;
    }
    osDelay (250);
  }
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
