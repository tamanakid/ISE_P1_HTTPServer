/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.00
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rl_net.h"
#include "rl_net_lib.h"
#include "GPIO_LPC17xx.h"
#include "cmsis_os.h"

#include "rtc.h"

#include "HTTP_Server.h"



// net_sys.c
extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]

// Net_Config.c
extern struct tcp_cfg   tcp_config;
extern struct http_cfg  http_config;
#define tcp_NumSocks    tcp_config.NumSocks
#define tcp_socket      tcp_config.Scb
#define http_EnAuth     http_config.EnAuth
#define http_auth_passw http_config.Passw



/* Extern application declarations */

extern bool leds_running;
extern bool led2_blink;
extern bool led3_blink;
extern uint8_t leds_on;

extern bool lcd_update;
extern char lcd_text[2][30+1];

extern uint8_t adc_threshold;

extern bool rtc_active;

extern int ntp_timestamp;
extern const uint8_t *ntp_server;
extern const uint8_t ntp_server_1[4];
extern const uint8_t ntp_server_2[4];
extern int ntp_server_selected;
extern char str_time_sntp[50];
extern char str_time_rtc[50];



// Structure of CGI status variable.
typedef struct {
  uint16_t xcnt;
  uint16_t unused;
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)



// Process query string received by GET request.
void cgi_process_query (const char *qstr) {
  char var[40];

  do {
    // Loop through all the parameters
    qstr = http_get_env_var (qstr, var, sizeof (var));
    // Check return string, 'qstr' now points to the next parameter
    if (var[0] != 0) {
      // First character is non-null, string exists
      if (strncmp (var, "ip=", 3) == 0) {
        // Local IP address
        ip4_aton (&var[3], LocM.IpAddr);
      }
      else if (strncmp (var, "msk=", 4) == 0) {
        // Local network mask
        ip4_aton (&var[4], LocM.NetMask);
      }
      else if (strncmp (var, "gw=", 3) == 0) {
        // Default gateway IP address
        ip4_aton (&var[3], LocM.DefGW);
      }
      else if (strncmp (var, "pdns=", 5) == 0) {
        // Primary DNS server IP address
        ip4_aton (&var[5], LocM.PriDNS);
      }
      else if (strncmp (var, "sdns=", 5) == 0) {
        // Secondary DNS server IP address
        ip4_aton (&var[5], LocM.SecDNS);
      }
    }
  } while (qstr);
}



// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void cgi_process_data (uint8_t code, const char *data, uint32_t len) {
  char var[40],passw[12];
	bool write_to_flash = false;
	char threshold_char[3];
	uint8_t threshold_placeholder;

  if (code != 0) {
    // Ignore all other codes
    return;
  }
	
	if (strncmp (data, "pg=led", 6) == 0) {
		leds_on = 0;
		write_to_flash = true;
	} else if (strncmp (data, "pg=ad&threshold", 15) == 0) {
		strncpy(threshold_char, &data[16], 3);
		threshold_placeholder = (uint8_t) strtol(threshold_char, (char **)NULL, 10);
		if (threshold_placeholder < 256) {
			adc_threshold = threshold_placeholder;
			write_to_flash = true;
		}
	}
  // leds_running = true;
  if (len == 0) {
    // No data or all items (radio, checkbox) are off
    return;
  }
  passw[0] = 1;
  do {
    // Parse all parameters
    data = http_get_env_var (data, var, sizeof (var));
    if (var[0] != 0) {
      // First character is non-null, string exists
      if (strcmp (var, "led0=on") == 0) {
        leds_on |= 0x01;
      }
      else if (strcmp (var, "led1=on") == 0) {
        leds_on |= 0x02;
      }
      else if (strcmp (var, "led2=on") == 0) {
        leds_on |= 0x04;
      }
      else if (strcmp (var, "led3=on") == 0) {
        leds_on |= 0x08;
      }
			else if (strncmp (var, "ctrl=Running", 10) == 0) {
        leds_running = true;
      }
      else if (strcmp (var, "ctrl=Browser") == 0) {
        leds_running = false;
      }
			
			else if (strncmp (var, "sntp=Server1", 12) == 0) {
        ntp_server_selected = 1;
				ntp_server = &ntp_server_1[0];
      }
      else if (strncmp (var, "sntp=Server2", 12) == 0) {
        ntp_server_selected = 2;
				ntp_server = &ntp_server_2[0];
      }
			else if (strncmp (var, "alarm=Active", 12) == 0) {
        rtc_active = true;
      }
			else if (strncmp (var, "alarm=Inactive", 14) == 0) {
        rtc_active = false;
      }
			
      else if ((strncmp (var, "pw0=", 4) == 0) ||
               (strncmp (var, "pw2=", 4) == 0)) {
        // Change password, retyped password
        if (http_EnAuth) {
          if (passw[0] == 1) {
            strcpy (passw, var+4);
          }
          else if (strcmp (passw, var+4) == 0) {
            // Both strings are equal, change the password
            strcpy (http_auth_passw, passw);
          }
        }
      }
      else if (strncmp (var, "lcd1=", 5) == 0) {
        // LCD Module line 1 text
        strcpy (lcd_text[0], var+5);
        lcd_update = true;
      }
      else if (strncmp (var, "lcd2=", 5) == 0) {
        // LCD Module line 2 text
        strcpy (lcd_text[1], var+5);
        lcd_update = true;
      }
    }
  } while (data);
	
	if (write_to_flash == true) {
		osSignalSet(id_thread_flash, 0x01);
	}		
}



// Generate dynamic web data from a script line.
uint32_t cgi_script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  TCP_INFO *tsoc;
  uint32_t len = 0;
  uint8_t id;
	uint8_t is_checked;
	char led_id;
	char checkbox_text[50];
	uint8_t adc_threshold;
  static uint32_t adv;
	static int timestamp;

	
  switch (env[0]) {
    // Analyze a 'c' script line starting position 2
		
    case 'a' :
      // Network parameters from 'network.cgi'
      switch (env[2]) {
        case 'i':
          // Write local IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.IpAddr));
          break;
        case 'm':
          // Write local network mask
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.NetMask));
          break;
        case 'g':
          // Write default gateway IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.DefGW));
          break;
        case 'p':
          // Write primary DNS server IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.PriDNS));
          break;
        case 's':
          // Write secondary DNS server IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.SecDNS));
          break;
      }
      break;

			
    case 'b':
      // LED control from 'led.cgi'
      if (env[2] == 'c') {
        // Select Control
        len = sprintf (buf, &env[4], leds_running ?     ""     : "selected",
                                     leds_running ? "selected" :    ""     );
        break;
      } else if (env[2] == 'o') {
				len = sprintf (buf, &env[4], leds_running ? "disabled" : "");
				break;
			}
      // LED CheckBoxes
      id = env[2] - '0';
      if (id > 7) {
        id = 0;
      }
      id = 1 << id;
      
      led_id = env[2];
			is_checked = (leds_on & id);
      
			// Evaluate both 'checked' and 'disabled' attributes
			if (is_checked && leds_running) {
				strcpy(checkbox_text, "checked disabled");
			} else if (!is_checked && leds_running) {
				strcpy(checkbox_text, "disabled");
			} else if (is_checked && !leds_running) {
				strcpy(checkbox_text, "checked");
			} else {
				strcpy(checkbox_text, "");
			}
			
			if (led3_blink != true && led2_blink != true) {
				leds_browser_set(led_id, is_checked);
			}
			len = sprintf (buf, &env[4], &checkbox_text);
			
			break;

			
    case 'c':
      // TCP status from 'tcp.cgi'
      while ((len + 150) < buflen) {
        tsoc = &tcp_socket[MYBUF(pcgi)->xcnt];
        MYBUF(pcgi)->xcnt++;
        // 'sprintf' format string is defined here
        len += sprintf (buf+len,   "<tr align=\"center\">");
        if (tsoc->State <= tcpStateCLOSED) {
          len += sprintf (buf+len, "<td>%d</td><td>%s</td><td>-</td><td>-</td>"
                                   "<td>-</td><td>-</td></tr>\r\n",
                                   MYBUF(pcgi)->xcnt,tcp_ntoa(tsoc->State));
        }
        else if (tsoc->State == tcpStateLISTEN) {
          len += sprintf (buf+len, "<td>%d</td><td>%s</td><td>%d</td><td>-</td>"
                                   "<td>-</td><td>-</td></tr>\r\n",
                                   MYBUF(pcgi)->xcnt, tcp_ntoa(tsoc->State), tsoc->LocPort);
        }
        else {
          len += sprintf (buf+len, "<td>%d</td><td>%s</td><td>%d</td>"
                                   "<td>%d</td><td>%s</td><td>%d</td></tr>\r\n",
                                   MYBUF(pcgi)->xcnt, tcp_ntoa(tsoc->State), tsoc->LocPort,
                                   tsoc->AliveTimer, ip4_ntoa (tsoc->RemAddr), tsoc->RemPort);
        }
        // Repeat for all TCP Sockets
        if (MYBUF(pcgi)->xcnt == tcp_NumSocks) {
          break;
        }
      }
      if (MYBUF(pcgi)->xcnt < tcp_NumSocks) {
        // Hi bit is a repeat flag
        len |= (1u << 31);
      }
      break;

			
    case 'd':
      // System password from 'system.cgi'
      switch (env[2]) {
        case '1':
          len = sprintf (buf, &env[4], http_EnAuth ? "Enabled" : "Disabled");
          break;
        case '2':
          len = sprintf (buf, &env[4], http_auth_passw);
          break;
      }
      break;


    case 'f':
      // LCD Module control from 'lcd.cgi'
      switch (env[2]) {
        case '1':
          len = sprintf (buf, &env[4], lcd_text[0]);
          break;
        case '2':
          len = sprintf (buf, &env[4], lcd_text[1]);
          break;
      }
      break;

			
    case 'g':
      // AD Input from 'ad.cgi'
      switch (env[2]) {
        case '1':
          adv = adc_read();
          len = sprintf (buf, &env[4], adv);
          break;
        case '2':
          len = sprintf (buf, &env[4], (float)adv*3.3f/4096);
          break;
        case '3':
          adv = (adv * 100) / 4096;
          len = sprintf (buf, &env[4], adv);
          break;
				// 4&5 are the ADC Threshold value in hex and decimal notations
				case '4':
          adc_threshold = adc_get_flash_threshold();
          len = sprintf (buf, &env[4], adc_threshold);
          break;
				case '5':
          adc_threshold = adc_get_flash_threshold();
          len = sprintf (buf, &env[4], (int)adc_threshold);
          break;
      }
      break;

    case 'x':
      // AD Input from 'ad.cgx'
		
      adv = adc_read();
      len = sprintf (buf, &env[1], adv);
      break;
		
		
		case 't':
			// Time from RTC & SNTP
		
			if (env[2] == 's') {
				// Timestamp
				timestamp = ntp_timestamp;
				len = sprintf (buf, &env[4], timestamp);
				break;

			} else if (env[2] == 'n') {
				// Last SNTP request time
				len = sprintf (buf, &env[4], str_time_sntp);
				break;

			} else if (env[2] == 'm') {
				// RTC time
				rtc_get_full_time();
				snprintf(
					str_time_rtc,
					sizeof(str_time_rtc),
					"%.4d/%.2d/%.2d - %.2d:%.2d:%.2d",
					rtc_years, rtc_months, rtc_days, rtc_hours, rtc_minutes, rtc_seconds
				);
				len = sprintf (buf, &env[4], str_time_rtc);
				break;

			} else if (env[2] == '1') {
        // Select SNTP Server from browser
        len = sprintf (buf, &env[4], ntp_server_selected == 1 ? "selected" : "");
        break;
			} else if (env[2] == '2') {
        // Select SNTP Server from browser
        len = sprintf (buf, &env[4], ntp_server_selected == 2 ? "selected" : "");
        break;
				
			} else if (env[2] == '3') {
				// Select RTC alarm as active
				len = sprintf (buf, &env[4], rtc_active ? "selected" : "");
			} else if (env[2] == '4') {
				// Select RTC alarm as active
				len = sprintf (buf, &env[4], rtc_active ? "" : "selected");
			}
		
		
		case 'r':
      // RTC Input from 'time_rtc.cgx'
			if (env[2] == 'n') {
				// Last SNTP request time
				len = read_time_strings(env, buf, str_time_sntp);
				break;

			} else if (env[2] == 'm') {
				// RTC time
				len = read_time_strings(env, buf, str_time_rtc);
				break;

			}
      break;
			
  }
  return (len);
}
