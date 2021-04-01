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

/* Application specific libraries */
#include "rtc.h"

/* Application specific declarations */
#include "HTTP_Server.h"



/* Extern Declarations */

void thread_leds (void const *arg);
void thread_lcd  (void const *argument);
void thread_sntp (void const *argument);

osThreadId id_thread_lcd, id_thread_leds, id_thread_sntp, id_thread_flash;

osThreadDef (thread_lcd, osPriorityNormal, 1, 0);
osThreadDef (thread_leds, osPriorityNormal, 1, 0);
osThreadDef (thread_sntp, osPriorityNormal, 1, 0);
osThreadDef (thread_flash, osPriorityNormal, 1, 0);



/* Definitions */

bool rtc_active = true;



/**
 * Initialize all threads
 */
int init_threads (void) {

	id_thread_leds = osThreadCreate (osThread(thread_leds), NULL);
  id_thread_lcd = osThreadCreate (osThread(thread_lcd), NULL);
	id_thread_sntp = osThreadCreate (osThread(thread_sntp), NULL);
	id_thread_flash = osThreadCreate (osThread(thread_flash), NULL);
  if (!id_thread_leds || !id_thread_lcd || !id_thread_sntp ||!id_thread_flash) return(-1);
  
  return(0);
}



/**
 * Callback from RTC ISR
 */
void rtc_handle_interrupt() {
	if (rtc_active == true) {
		osSignalSet(id_thread_sntp, 0x02);
		led3_blink = true;
	}
}



/**
 * Main Thread
 */
int main (void) {
	/* Network initialization */
  net_initialize();
	get_network_data();
	
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
