/*
 * ESP8266_AT_CMD.h
 *
 *  Created on: 22/08/2016
 *      Author: Desarrollo 01
 */

#ifndef ESP8266_AT_CMD_H_
#define ESP8266_AT_CMD_H_

/* Common AT responses */
const char AT_RESPONSE_OK[] = "OK";
const char AT_RESPONSE_ERROR[] = "ERROR";
const char AT_RESPONSE_BUSY[] = "busy";
const char AT_RESPONSE_FAIL[] = "FAIL";
const char AT_RESPONSE_READY[] = "READY!";
const char AT_RESPONSE_RST[] = "ready";

/* Basic AT Commands */
const char AT_CMD[] = "AT";
const char AT_TEST[] = ""; /* Test AT startup */
const char AT_RESET[] = "+RST"; /* Restart module */
const char AT_GMR[] = "+GMR"; /* View version info */
const char AT_ECHO_ENABLE[] = "E1"; /* AT commands echo Enable */
const char AT_ECHO_DISABLE[] = "E0"; /* AT commands echo Disable */

/* Wi-Fi AT Commands */
const char AT_SET_WIFI_MODE[] = "+CWMODE_CUR"; /* Current WiFi mode */
const char AT_CWJAP[] = "+CWJAP_CUR"; /* Connect to AP, for current */
const char AT_CWLAP[] = "+CWLAP"; /* List available APs */
const char AT_CWQAP[] = "+CWQAP"; /* Disconnect from AP */

/* WiFi responses*/
const char AT_CWLAP_RX[] = "+CWLAP:";

/* TCP/IP Related AT Commands */
const char AT_CIPSTART[] = "+CIPSTART"; /* Establish TCP, UDP or SSL connection */
const char AT_CIPSEND[] = "+CIPSEND"; /* Send data */
const char AT_CIPCLOSE[] = "+CIPCLOSE"; /* Close TCP, UDP or SSL connection */
const char AT_CIPMUX[] = "+CIPMUX"; /* Enable multiple connections */
const char AT_CIFSR[] = "+CIFSR"; /* Get local IP address */
const char AT_CIPSTAMAC[] = "+CIPSTAMAC_CUR"; /* Set/Get MAC address */
const char AT_PING[] = "+PING"; /* DESC */
const char AT_IPD[] = "+IPD";

/* TCP Responses */
const char AT_CIPSTART_ALRDY[] = "ALREADY CONNECT";
const char AT_CIPSTART_RX[] = "CONNECT";
const char AT_CIPSEND_OK[] = "SEND OK";
const char AT_CIPCLOSE_OK[] = "CLOSED";
const char AT_CIFSR_STATIP[] = "+CIFSR:STAIP,";
const char AT_CIPSTAMAC_CURR[] = "+CIPSTAMAC_CUR:";
const char AT_CIFSR_STAMAC[] = "+CIFSR:STAMAC,";

#endif /* ESP8266_AT_CMD_H_ */
