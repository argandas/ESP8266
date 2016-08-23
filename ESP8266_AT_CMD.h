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
const char AT_TEST[] = "";
const char AT_RESET[] = "+RST";
const char AT_GMR[] = "+GMR";
const char AT_ECHO_ENABLE[] = "E1";
const char AT_ECHO_DISABLE[] = "E0";

/* WiFi AT Commands */
const char AT_SET_WIFI_MODE[] = "+CWMODE_CUR";
const char AT_CWJAP[] = "+CWJAP_CUR";
const char AT_CWLAP[] = "+CWLAP";
const char AT_CWQAP[] = "+CWQAP";
const char AT_CIFSR[] = "+CIFSR";
const char AT_CIPMUX[] = "+CIPMUX";
const char AT_PING[] = "+PING";

/* WiFi responses*/
const char AT_CIFSR_STATIP[] = "+CIFSR:STAIP,";
const char AT_CIFSR_STAMAC[] = "+CIFSR:STAMAC,";
const char AT_CIPSTART_RX[] = "CONNECT";
const char AT_IPD[] = "+IPD,";
const char AT_CWLAP_RX[] = "+CWLAP:";

/* TCP AT Commands */
const char AT_CIPSTART[] = "+CIPSTART";
const char AT_CIPSEND[] = "+CIPSEND";
const char AT_CIPCLOSE[] = "+CIPCLOSE";

/* TCP Responses */
const char AT_CIPSEND_RX[] = "SEND OK";
const char AT_CIPCLOSE_RX[] = "CLOSED";

#endif /* ESP8266_AT_CMD_H_ */
