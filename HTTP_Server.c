/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "rl_net.h"                     /* Network definitions                */

// #include "Board_GLCD.h"
// #include "GLCD_Config.h"
#include "Board_LED.h"
#include "Board_Buttons.h"
#include "Board_ADC.h"

#include "HTTP_Server.h"
#include "lcd.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"


// extern GLCD_FONT GLCD_Font_6x8;
// extern GLCD_FONT GLCD_Font_16x24;

bool LEDrun = true;
bool LCDupdate;
char lcd_text[2][20+1];

static void BlinkLed (void const *arg);
static void Display (void const *arg);

osThreadDef(BlinkLed, osPriorityNormal, 1, 0);
osThreadDef(Display, osPriorityNormal, 1, 0);



/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static void Display (void const *arg) {
  

	lcd_initialize();
  
  LCDupdate = false;

  while(1) {
    if (LCDupdate == true) {
			lcd_write();
			LCDupdate = false;
    }
    osDelay (250);
  }
}



/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static void BlinkLed (void const *arg) {								
	uint8_t current_led = 0;

  LEDrun = true;
	leds_initialize();
  while(1) {
    // Every 100 ms
    if (LEDrun == true) {
			current_led = leds_running_set(current_led);
    }
    osDelay (100);
  }
}



/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
int main (void) {
  net_initialize     ();
	adc_initialize();

  osThreadCreate (osThread(BlinkLed), NULL);
  osThreadCreate (osThread(Display), NULL);

  while(1) {
    net_main ();
    osThreadYield ();
  }
}
