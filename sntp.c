#include <time.h>
#include <string.h>
#include <stdio.h>

#include "rl_net.h"

#include "rtc.h"

#include "HTTP_Server.h"


extern bool LEDrun;
extern bool LED2blink;
extern bool LED3blink;



int ntp_timestamp = 0;

char str_time_sntp[50];
char str_time_rtc[50];

time_t timestamp_tt;
struct tm timestamp_tm;

const uint8_t ntp_server_1[4] = {130,206,3,166};
// const uint8_t ntp_server_2[4] = {193,147,107,33};

netStatus set_mode_status, request_status;



void thread_sntp (void const *argument) {
	static int minute_count = 3;
	
	strcpy(str_time_sntp, "No request since reset");

  while (1) {
		minute_count++;
		if (minute_count >= 3) {
			request_status = sntp_get_time (&ntp_server_1[0], sntp_response_callback);
			minute_count = 0;
		}		
		osSignalWait(0x02, osWaitForever);
	}
}


static void sntp_response_callback (uint32_t timestamp) {	
  if (timestamp == 0) {
    ntp_timestamp = 0;

  } else {
		// Consider GMT+1
		timestamp_tt = timestamp + 3600;
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
		
		LED2blink = true;
  }
}


extern uint32_t  read_time_strings(const char *env, char *buf, char *str) {
	rtc_get_full_time();
	snprintf(
		str_time_rtc,
		sizeof(str_time_rtc),
		"%.4d/%.2d/%.2d - %.2d:%.2d:%.2d",
		rtc_years, rtc_months, rtc_days, rtc_hours, rtc_minutes, rtc_seconds
	);
	return sprintf (buf, &env[4], str);
}

