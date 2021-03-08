#include "LPC17xx.h"



/** MACROS & CONSTANTS */

#define RTC_ILR_RTCCIF    0

#define RTC_CCR_CLKEN     0
#define RTC_CCR_CTCRST    4
#define RTC_CCR_CCALEN    4

#define RTC_CIIR_CONFIG   	0x00000002

#define RESET_DATE_SEC 			55
#define RESET_DATE_MIN 			00
#define RESET_DATE_HOUR 		00
#define RESET_DATE_DOM 			01
#define RESET_DATE_MONTH		01
#define RESET_DATE_YEAR 		2000



/** GLOBAL VARIABLES DECLARATION */

// Global Time Variables
extern int rtc_seconds, rtc_minutes, rtc_hours,
	rtc_days, rtc_months, rtc_years;



/**
 * EXTERNAL FUNCTION DECLARATION
 * These functions MUST BE DEFINED by the implementation program
 */

/**
 * Called from the RTC interrupt handler
 * Should be as reduced as possible (i.e. setting an OS signal)
 */
extern void rtc_handle_interrupt(void);



/** FUNCTION DECLARATION */

/**
 * Method to be called during peripheral initialization
 * Sets the RTC registers to reset/enable it and configure its interrupts
 * @param (uint32_t) rtc_ciir_config: value to set in the CIIR register for interrupts
 */
void rtc_initialize(uint32_t rtc_ciir_config);


/**
 * Method to be called during program execution (Preferrably by threads)
 * Should be called before using global time variables
 * Gets the CTIME0 & CTIME1 registers values and writes them to the global time variables
 */
void rtc_get_full_time(void);


/**
 * Method to be called during program execution (Preferrably by threads)
 * Sets the RTC registers to the argument values
 */
void rtc_set_full_time(int seconds, int minutes, int hours,
											 int days, int months, int years);


/**
 * Method to be called during program execution (Preferrably by threads)
 * Internally calls "rtc_set_full_time" with default reset values
 */
void rtc_reset_full_time(void);


/**
 * Method used internally by "rtc_get_full_time" to extract time values from CTIME registers
 * @param (int) first_bit: LSb position of desired time value
 * @param (int) last_bit: MSb position of desired time value
 * @param (uint32_t) reg: reference to the CTIME register to extract value from
 */
int rtc_read_bits (int first_bit, int last_bit, uint32_t reg);
