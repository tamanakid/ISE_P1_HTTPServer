/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
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


// extern GLCD_FONT GLCD_Font_6x8;
// extern GLCD_FONT GLCD_Font_16x24;

bool LEDrun;
bool LCDupdate;
char lcd_text[2][20+1];

static void BlinkLed (void const *arg);
static void Display (void const *arg);

osThreadDef(BlinkLed, osPriorityNormal, 1, 0);
osThreadDef(Display, osPriorityNormal, 1, 0);

/// Read analog inputs
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
  }
  return (val);
}

/// Read digital inputs
uint8_t get_button (void) {
  return (Buttons_GetState ());
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static void Display (void const *arg) {
  char lcd_buf[20+1];

	lcd_initialize();
  sprintf (lcd_text[0], "");
  sprintf (lcd_text[1], "");
  LCDupdate = false;

  while(1) {
		/** SEPARAR EN DOS CONDICIONES? */
    if (LCDupdate == true) {
      // sprintf (lcd_buf, "%-20s", lcd_text[0]);
			strcpy(lcd_buf, "                     ");
			strcpy(lcd_buf, lcd_text[0]);
			escribe_frase_L1(lcd_buf, 21);
      // sprintf (lcd_buf, "%-20s", lcd_text[1]);
			strcpy(lcd_buf, "                     ");
			strcpy(lcd_buf, lcd_text[1]);
			escribe_frase_L2(lcd_buf, 21);
			copy_to_lcd();
			LCDupdate = false;
    }
    osDelay (250);
  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static void BlinkLed (void const *arg) {
  const uint8_t led_val[16] = { 0x48,0x88,0x84,0x44,0x42,0x22,0x21,0x11,
                                0x12,0x0A,0x0C,0x14,0x18,0x28,0x30,0x50 };
  int cnt = 0;

  LEDrun = true;
  while(1) {
    // Every 100 ms
    if (LEDrun == true) {
      LED_SetOut (led_val[cnt]);
      if (++cnt >= sizeof(led_val)) {
        cnt = 0;
      }
    }
    osDelay (100);
  }
}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
int main (void) {
  // LED_Initialize     ();
  // Buttons_Initialize ();
  // ADC_Initialize     ();
	hardware_initialize();
  net_initialize     ();

  // osThreadCreate (osThread(BlinkLed), NULL);
  osThreadCreate (osThread(Display), NULL);

  while(1) {
    net_main ();
    osThreadYield ();
  }
}
