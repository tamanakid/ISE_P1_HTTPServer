#include <time.h>
#include <string.h>
#include <stdio.h>

#include "rl_net.h"

#include "rtc.h"

#include "HTTP_Server.h"


/* Extern Declarations */

extern bool led2_blink;


/* Definitions */

int ntp_timestamp = 0;
int ntp_server_selected = 1;

const uint8_t ntp_server_1[4] = {130,206,3,166};
const uint8_t ntp_server_2[4] = {193,147,107,33};
const uint8_t *ntp_server;

char str_time_sntp[50];
char str_time_rtc[50];

// Local-only definitions
time_t timestamp_tt;
struct tm timestamp_tm;


/**
 * @brief		Thread: SNTP Client - Makes SNTP request upon reset and every three minutes
 */
void thread_sntp (void const *argument) {
	static int minute_count = 3;
	
	strcpy(str_time_sntp, "No request since reset");
	ntp_server = &ntp_server_1[0];
	osDelay(1000);

  while (1) {		
		minute_count++;
		if (minute_count >= 3) {
			sntp_get_time (ntp_server, sntp_response_callback);
			minute_count = 0;
		}
		osSignalWait(0x02, osWaitForever);
	}
}



/**
 * @brief 	Callback upon SNTP response: Set time to GMT+2 (7200 seconds after GMT)
 * @param 	uint32_t timestamp - timestamp response from SNTP Server
 */
static void sntp_response_callback (uint32_t timestamp) {
  if (timestamp == 0) {
    ntp_timestamp = 0;

  } else {
		// Consider GMT+1
		timestamp_tt = timestamp + UTC_GMT_PLUS2;
		(void) localtime_r(&timestamp_tt, &timestamp_tm);
		
		strftime(str_time_sntp, sizeof(str_time_sntp), "%Y/%m/%d - %H:%M:%S\n", &timestamp_tm);
		
		rtc_set_full_time(
			timestamp_tm.tm_sec,
			timestamp_tm.tm_min,
			timestamp_tm.tm_hour,
			timestamp_tm.tm_mday,
			timestamp_tm.tm_mon + 1,
			timestamp_tm.tm_year + 1900
		);
		
    ntp_timestamp = (int) timestamp;
		
		led2_blink = true;
  }
}



/**
 * @brief				Function called from HTTP_Server_CGI.c to get the RTC/SNTP dates formatted to string
 * 								Either date may be asked by the "str" parameter, but the RTC clock is always refreshed
 * @param[out] 	env - char pointer to HTML content
 * @param[out] 	buf - char pointer to HTML content
 * @param[in]		str - char pointer to the string to get the data from (RTC or SNTP)
 * @return length of data stream to response (used by HTTP_Server_CGI.c)
 */
extern uint32_t read_time_strings(const char *env, char *buf, char *str) {
	rtc_get_full_time();
	
	snprintf(
		str_time_rtc,
		sizeof(str_time_rtc),
		"%.4d/%.2d/%.2d - %.2d:%.2d:%.2d",
		rtc_years, rtc_months, rtc_days, rtc_hours, rtc_minutes, rtc_seconds
	);
	
	return sprintf (buf, &env[4], str);
}

