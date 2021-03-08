#include <math.h>

#include "LPC17xx.h"

#include "rtc.h"



/** GLOBAL VARIABLES DEFINITION */

int rtc_seconds, rtc_minutes, rtc_hours,
	rtc_days, rtc_months, rtc_years;



/** FUNCTION DEFINITION */

void rtc_initialize(uint32_t rtc_ciir_config) {
	// RTC Reset, Calibration and Enabling
  LPC_RTC->CCR = ((1 << RTC_CCR_CCALEN) | (1 << RTC_CCR_CTCRST));
	LPC_RTC->CALIBRATION = 0x00;
	LPC_RTC->CCR = (1 << RTC_CCR_CLKEN);
  
  // RTC Interrupt Configuration
	LPC_RTC->CIIR = rtc_ciir_config;
  
  // RTC Interrupts Enabling in NVIC
  NVIC_EnableIRQ(RTC_IRQn);
}


void rtc_get_full_time (void) {
	// Read CTIME registers
	uint32_t ctime0 = LPC_RTC->CTIME0;
	uint32_t ctime1 = LPC_RTC->CTIME1;
	
	// Set global time variables
	rtc_seconds = LPC_RTC->SEC;
	rtc_minutes = LPC_RTC->MIN;
	rtc_hours = LPC_RTC->HOUR;
	rtc_days = LPC_RTC->DOM;
	rtc_months = LPC_RTC->MONTH;
	rtc_years = LPC_RTC->YEAR;
}


void rtc_set_full_time (
	int seconds, int minutes, int hours,
	int days, int months, int years
) {
	// Set RTC value registers
	LPC_RTC->SEC = seconds;
	LPC_RTC->MIN = minutes;
	LPC_RTC->HOUR = hours;
	LPC_RTC->DOM = days;
	LPC_RTC->MONTH = months;
	LPC_RTC->YEAR = years;
}


void rtc_reset_full_time(void) {
	rtc_set_full_time(
		RESET_DATE_SEC, RESET_DATE_MIN, RESET_DATE_HOUR,
		RESET_DATE_DOM, RESET_DATE_MONTH, RESET_DATE_YEAR
	);
}


int rtc_read_bits (int first_bit, int last_bit, uint32_t reg) {
	int and_operator = pow((double) 2, last_bit - first_bit + 1) - 1;
	
	uint32_t value = (reg >> first_bit) & and_operator;
	return (int) value;
}


void RTC_IRQHandler () {
  // Clear ILR "Counter Increment Interrupt" bit
  LPC_RTC->ILR |= (1 << RTC_ILR_RTCCIF);
	
	// Delegates execution to program implementation
	rtc_handle_interrupt();
}
