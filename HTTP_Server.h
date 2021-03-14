#include "stdint.h"


#define PORT_LED 1
#define PIN_LED0 18
#define PIN_LED1 20
#define PIN_LED2 21
#define PIN_LED3 23
#define CLEAR_STRING "                        "


extern char lcd_text[2][30+1];


/**
 * Functions defined in Hardware.c
 */

/* LED-related */
void leds_initialize(void);
void leds_browser_set(char led_id, uint8_t is_checked);
uint8_t leds_running_set(uint8_t current_led);

/* ADC-related */
void adc_initialize(void);
uint16_t adc_read(void);

/* LCD-related */
void lcd_initialize(void);
void lcd_write(void);

/* Joystick-related */
void joystick_initialize(void);


/**
 * Functions defined in sntp.c
 */
static void sntp_response_callback (uint32_t timestamp);



/**
 * NEW
 */
 
 #include "cmsis_os.h"

#define INIT_DATE_SEC 		55
#define INIT_DATE_MIN 		00
#define INIT_DATE_HOUR 		00
#define INIT_DATE_DOM 		01
#define INIT_DATE_MONTH		01
#define INIT_DATE_YEAR 		2000


#define PORT_LED 1
#define PIN_LED0 18
#define PIN_LED1 20
#define PIN_LED2 21
#define PIN_LED3 23

#define PORT_PINS 	0
#define PIN_JST_C		16


//#define RTC_CIIR_CONFIG   0x000000CF
#define RTC_CIIR_CONFIG   	0x00000002



extern osThreadId id_thread_lcd, id_thread_leds, id_thread_sntp;

extern int init_threads_rtc (void);

extern uint8_t leds_status;

extern void leds_blink_led3(void);
extern void leds_blink_led2(void);
extern void leds_restore_browser_config(void);

extern uint32_t  read_time_strings(const char *env, char *buf, char *str);

extern int ntp_timestamp;
extern const uint8_t ntp_server_1[4];
extern const uint8_t ntp_server_2[4];
extern const uint8_t *ntp_server;

extern char str_time_sntp[50];
extern char str_time_rtc[50];

/**
 * NEW - end
 */

