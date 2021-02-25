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

/// Read analog inputs
uint16_t AD_in (uint32_t ch) {
	/**
  int32_t val = 0;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
  }
  return (val);
	*/
	uint16_t buffer_adc_sensor;
	static int adc_value;
	
	if (ch == 0) {
		LPC_ADC->ADCR |= 1UL << 4; // Select AD0[4]
		LPC_ADC->ADCR |= 1UL << 24; // Start conversion now.
		while((LPC_ADC->ADGDR >> 31) == 0); // Wait until conversion completes
		buffer_adc_sensor = (LPC_ADC->ADGDR & (0xFFF << 4)) >> 4;
		LPC_ADC->ADCR &= ~(1UL << 24); //Start conversion clear.
		adc_value = buffer_adc_sensor; // *mutex* ?
  }
	
	return adc_value;	
}

/// Read digital inputs
uint8_t get_button (void) {
  return (Buttons_GetState ());
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static void Display (void const *arg) {
  char lcd_buf[24+1];

	lcd_initialize();
  sprintf (lcd_text[0], "");
  sprintf (lcd_text[1], "");
  LCDupdate = false;

  while(1) {
		/** SEPARAR EN DOS CONDICIONES? */
    if (LCDupdate == true) {
      // reset_lcd();
      // sprintf (lcd_buf, "%-20s", lcd_text[0]);
			strcpy(lcd_buf, lcd_text[0]);
			escribe_frase_L1(lcd_buf, sizeof(lcd_buf));
      // sprintf (lcd_buf, "%-20s", lcd_text[1]);
			// strcpy(lcd_buf, "                     ");
			strcpy(lcd_buf, lcd_text[1]);
			escribe_frase_L2(lcd_buf, sizeof(lcd_buf));
			copy_to_lcd();
			strcpy(lcd_buf, "                        ");
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
  while(1) {
    // Every 100 ms
    if (LEDrun == true) {
			switch (current_led) {
				case 0:
					GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED0, 1);
					current_led++;
					break;
				case 1:
					GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED1, 1);
					GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
					current_led++;
					break;
				case 2:
					GPIO_PinWrite(PORT_LED, PIN_LED3, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED2, 1);
					GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
					current_led++;
					break;
				case 3:
					GPIO_PinWrite(PORT_LED, PIN_LED3, 1);
					GPIO_PinWrite(PORT_LED, PIN_LED2, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED1, 0);
					GPIO_PinWrite(PORT_LED, PIN_LED0, 0);
					current_led = 0;
					break;
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

  osThreadCreate (osThread(BlinkLed), NULL);
  osThreadCreate (osThread(Display), NULL);

  while(1) {
    net_main ();
    osThreadYield ();
  }
}
