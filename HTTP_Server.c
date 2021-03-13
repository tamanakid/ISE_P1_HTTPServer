/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

// #define osObjectsPublic
// #include "osObjects.h"
/* CMSIS RTOS and Network definitions */
#include "cmsis_os.h"
#include "rl_net.h"

/* LPC17xx definitions */
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

/* Application specific libraries */
#include "lcd.h"
#include "rtc.h"

/* Application specific declarations */
#include "HTTP_Server.h"



bool LEDrun = true;
bool LED2blink = false;
bool LED3blink = false;
bool LCDupdate;
char lcd_text[2][30+1];

void thread_leds (void const *arg);
void thread_lcd  (void const *argument);
void thread_sntp (void const *argument);

osThreadId id_thread_lcd, id_thread_leds, id_thread_sntp;

osThreadDef (thread_lcd, osPriorityNormal, 1, 0);
osThreadDef (thread_leds, osPriorityNormal, 1, 0);
osThreadDef (thread_sntp, osPriorityNormal, 1, 0);



/**
 * Initialize all threads
 */
int init_threads (void) {

	id_thread_leds = osThreadCreate (osThread(thread_leds), NULL);
  id_thread_lcd = osThreadCreate (osThread(thread_lcd), NULL);
	id_thread_sntp = osThreadCreate (osThread(thread_sntp), NULL);
  if (!id_thread_leds || !id_thread_lcd || !id_thread_sntp) return(-1);
  
  return(0);
}



/**
 * Thread: LCD display handler
 */
static void thread_lcd (void const *arg) {
  
  LCDupdate = false;

  while(1) {
    if (LCDupdate == true) {
			lcd_write();
			LCDupdate = false;
    }
    osDelay (250);
  }
}



/**
 * Thread: LEDs handler
 */
static void thread_leds (void const *arg) {
	
	uint8_t current_led = 0;
  LEDrun = true;
	
  while(1) {
		if (LED3blink == true) {
			leds_blink_led3();
			LED3blink = false;
    }
		if (LED2blink == true) {
			leds_blink_led2();
			LED2blink = false;
		}
		if (LEDrun == true) {
			current_led = leds_running_set(current_led);
    } else {
			leds_restore_browser_config();
		}
    osDelay (100);
  }
}



/**
 * Callback from RTC ISR
 */
void rtc_handle_interrupt() {
	osSignalSet(id_thread_sntp, 0x02);
	LED3blink = true;
	// osSignalSet(id_thread_leds3, 0x01);
}



/**
 * Main Thread
 */
int main (void) {
	/* Network initialization */
  net_initialize();
	
	/* Peripherals initialization */
	leds_initialize();
	lcd_initialize();
	adc_initialize();
	joystick_initialize();

	/* RTC initialization */
	rtc_initialize(RTC_CIIR_CONFIG);

	/* RTOS threads initialization */
	init_threads();

	/* Run program */
  while(1) {
    net_main ();
    osThreadYield ();
  }
}
