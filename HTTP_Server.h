#include "stdint.h"
#include "stdbool.h"
#include "cmsis_os.h"


/**
 * MACROS AND CONSTANTS
 */

/* Peripherals-related */
#define PORT_PINS 	0
#define PIN_JST_C		16
#define PORT_LED 		1
#define PIN_LED0 		18
#define PIN_LED1 		20
#define PIN_LED2 		21
#define PIN_LED3 		23

/* RTC reset values (center-joystick button) */
#define INIT_DATE_SEC 		55
#define INIT_DATE_MIN 		00
#define INIT_DATE_HOUR 		00
#define INIT_DATE_DOM 		01
#define INIT_DATE_MONTH		01
#define INIT_DATE_YEAR 		2000

/* RTC interrupt register configuration: every minute */
#define RTC_CIIR_CONFIG   	0x00000002

/* Strings and misc */
#define CLEAR_STRING "                        "



/**
 * EXTERN DECLARATIONS
 */

/**
 * Defined in HTTP_Server.c (main) and thread-related
 */
extern bool rtc_active;

extern osThreadId id_thread_lcd, id_thread_leds, id_thread_sntp, id_thread_flash;
int init_threads_rtc (void);

void thread_leds (void const *arg);
void thread_lcd  (void const *argument);
void thread_sntp (void const *argument);
void thread_flash (void const *argument);



/**
 * Defined in hardware_leds.c
 */

extern bool leds_running;
extern bool led2_blink;
extern bool led3_blink;
extern uint8_t leds_on;

void leds_initialize(void);
void leds_get_flash_status(void);
void leds_blink_led3(void);
void leds_blink_led2(void);
void leds_restore_browser_config(void);
void leds_browser_set(char led_id, uint8_t is_checked);
uint8_t leds_running_set(uint8_t current_led);



/**
 * Defined in hardware_lcd.c
 */

extern bool lcd_update;
extern char lcd_text[2][30+1];

void lcd_initialize(void);
void lcd_write(void);



/**
 * Defined in Hardware.c
 */
// ADC-related
void adc_initialize(void);
uint16_t adc_read(void);

// Joystick-related
void joystick_initialize(void);



/**
 * Defined in sntp.c
 */
extern int ntp_timestamp;
extern int ntp_server_selected;
extern const uint8_t ntp_server_1[4];
extern const uint8_t ntp_server_2[4];
extern const uint8_t *ntp_server;
extern char str_time_sntp[50];
extern char str_time_rtc[50];

static void sntp_response_callback (uint32_t timestamp);
uint32_t read_time_strings(const char *env, char *buf, char *str);



/**
 * Defined in network_data.c
 */
extern uint8_t net_mac_address	[6];
extern uint8_t net_ip_address		[4];

void get_network_data (void);




/**
 * Defined in flash_interface.c
 */
 typedef struct {
	uint32_t start;
	uint32_t end;
} FlashAddressRange;

typedef struct {
	uint8_t start;
	uint8_t end;
} FlashSectorRange;

extern uint8_t net_mac_address	[6];
extern uint8_t net_ip_address		[4];

void flash_write_data						(void);
void flash_get_sector_range			(FlashAddressRange *address_range, FlashSectorRange *sector_range);
void flash_erase_sector					 (int start, int end);
void flash_write_array					(uint32_t address_start, uint8_t *array);
void flash_read_array						(uint32_t address, uint8_t *dest, int size);
void flash_write_byte						(uint32_t byte_address, uint8_t value);
void flash_write_bytes_tuple		(uint32_t start_write_address, uint8_t values[], uint8_t size);
