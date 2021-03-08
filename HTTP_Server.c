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
char lcd_text[2][30+1];

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





/**
 * NEW
 */

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"

#include <stdio.h>
#include "rtc.h"

osThreadId id_thread_lcd, id_thread_led3, id_thread_sntp;

void thread_lcd (void const *argument);
// void thread_led3 (void const *argument);
void thread_sntp (void const *argument);

osThreadDef (thread_lcd, osPriorityNormal, 1, 0);
// osThreadDef (thread_led3, osPriorityNormal, 1, 0);
osThreadDef (thread_sntp, osPriorityNormal, 1, 0);


int init_threads_rtc (void) {

  id_thread_lcd = osThreadCreate (osThread(thread_lcd), NULL);
	id_thread_sntp = osThreadCreate (osThread(thread_sntp), NULL);
  if (!id_thread_lcd || !id_thread_sntp) return(-1);
  
  return(0);
}



/** SNTP Related */

int ntp_seconds = 0;
// NET_ADDR4 *ntp_server = NULL;
// const uint8_t ntp_server[4] = {217,79,179,106};
const uint8_t ntp_server[4] = {130,206,3,166};
// const uint8_t ntp_server[4] = {193,147,107,33};
static void time_cback (uint32_t time);
netStatus set_mode_status, request_status;

void thread_sntp (void const *argument) {
	static int minute_count = 3;

  while (1) {
		minute_count++;
		if (minute_count > 3) {
			request_status = sntp_get_time (&ntp_server[0], time_cback);
			minute_count = 0;
		}		
		osSignalWait(0x02, osWaitForever);
	}
}

static void time_cback (uint32_t time) {
  if (time == 0) {
    ntp_seconds = 0;
  }
  else {
    ntp_seconds = (int) time;
  }
}




/**
 * Callback from RTC ISR
 */
void rtc_handle_interrupt() {
	osSignalSet(id_thread_sntp, 0x02);
	// osSignalSet(id_thread_led3, 0x01);
}


/** Joystick */

/**
 * GPIO Interrupt Service Routine
 */
void EINT3_IRQHandler() {
	
	if (LPC_GPIOINT->IO0IntStatR & (1 << PIN_JST_C)) {
		rtc_reset_full_time();
  }
	LPC_GPIOINT->IO0IntClr |= (1 << PIN_JST_C);
}

void joystick_initialize(void) {
	PIN_Configure(PORT_PINS, PIN_JST_C, PIN_FUNC_0, PIN_PINMODE_PULLDOWN, PIN_PINMODE_NORMAL);

	LPC_GPIOINT->IO0IntEnR |= (1UL << PIN_JST_C);

	NVIC_EnableIRQ(EINT3_IRQn);
}



/**
 * Thread to test the SNTP timestamp value
 * To write it to the screen, press "Send" in the LCD website
 */
void thread_lcd (void const *argument) {
	lcd_initialize();
  
  LCDupdate = false;

  while(1) {
    if (LCDupdate == true) {
			rtc_get_full_time();
		
			snprintf(lcd_text[0], sizeof(lcd_text[0]),
							 "%.2d:%.2d:%.2d - %.10d", rtc_hours, rtc_minutes, rtc_seconds, ntp_seconds);
			escribe_frase_L1(lcd_text[0], sizeof(lcd_text[0]));
			
			snprintf(lcd_text[1], sizeof(lcd_text[1]),
							 "%.2d/%.2d/%.4d - %.1d", rtc_days, rtc_months, rtc_years, (int) request_status);
			escribe_frase_L2(lcd_text[1], sizeof(lcd_text[1]));
		
			copy_to_lcd();
			LCDupdate = false;
    }
    osDelay (1000);
  }
}

/**
 * NEW - end
 */





/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
int main (void) {
  net_initialize     ();
	adc_initialize();
	joystick_initialize();
	
	rtc_initialize(RTC_CIIR_CONFIG);

  // osThreadCreate (osThread(BlinkLed), NULL);
  // osThreadCreate (osThread(Display), NULL);
	init_threads_rtc();

  while(1) {
    net_main ();
    osThreadYield ();
  }
}
