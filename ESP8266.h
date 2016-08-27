#ifndef ESP8266_H
#define ESP8266_H

#define ESP8266_USE_SOFT_SERIAL     (0)  /* Set this value to 1 to enable SoftwareSerial usage */

#include "Arduino.h"
#if (ESP8266_USE_SOFT_SERIAL == 1)
#include <SoftwareSerial.h>
#endif

#define ESP8266_DBG_PARSE_EN        (0)  /* Enable/Disable ESP8266 Debug  */
#define ESP8266_DBG_HTTP_RES        (0)  /* Enable/Disable ESP8266 Debug for HTTP responses */

#define ESP8266_MODE_STATION 		(1)  /* Station mode */
#define ESP8266_MODE_AP 			(2)  /* AP mode */
#define ESP8266_MODE_AP_STATION 	(3)  /* AP + Station mode */

#define ESP8266_CONN_SINGLE  		(0)  /* Single connection mode */
#define ESP8266_CONN_MULTIPLE  	    (1)  /* Multi-Channel connection mode */

#define ESP8266_RX_BUFF_LEN        (64)  /* ESP8266 Rx Buffer length */
#define ESP8266_MAX_SSID_LEN       (32)  /* Maximum SSID data length */

typedef struct
{
        uint16_t status;
        uint16_t len;
        char content[ESP8266_RX_BUFF_LEN];
} httpResponse;

/**
 * Setup ESP8266 connection.
 *
 * @param serialPort - Serial port where ESP8266 Tx/Rx are connectected
 * @param baud - ESP8266 current baud rate
 * @param rst - ESP8266 Reset Pin
 * @param en - ESP8266 Enable Pin
 */
#if (ESP8266_USE_SOFT_SERIAL == 1)
void setupESP8266(SoftwareSerial &serialPort, uint32_t baud, int rst, int en);
#else
void setupESP8266(HardwareSerial &serialPort, uint32_t baud, int rst, int en);
#endif

/**
 * Test connection to ESP8266.
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool test(void);

/**
 * Enable/Disable echo from ESP8266 when receiving a command.
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool echo(bool enable);

/**
 * Perform a software reset (Send "AT+RST").
 *
 * This function will take 3 seconds or more.
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool reset(void);

/**
 * Perform a hardware reset (Toggle RST pin).
 *
 * This function will take 2 seconds or more.
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool hardReset(void);

/**
 * Set SoftAP parameters.
 *
 * @param mode - Operation mode (1 - Station, 2 - SoftAP, 3 - SoftAP + Station)
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool setWifiMode(int mode);

/**
 * Enable/Disable multiple connections
 *
 * @param mode - Connection mode (0 - Single Connection, 1 - Multiple connection)
 *
 * @retval true - success.
 * @retval false - failure.
 * 
 * @note You should use CONN_MODE_SINGLE & CONN_MODE_MULTIPLE values
 */
bool setConnMode(int mode);

/**
 * Get current ESP8266´s firmware version of AT Command Set.
 *
 * @param dest - Pointer to char to save version information
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool getVersion(char *dest);

/**
 * Join AP.
 *
 * @param ssid - SSID of AP to join in.
 * @param pwd - Password of AP to join in.
 * @retval true - success.
 * @retval false - failure.
 * @note This method will take a couple of seconds.
 */
bool joinAP(char *ssid, char *ssid_pass);

/**
 * Quit from current AP.
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool quitAP(void);

/**
 * Get available Access Points. 
 *
 * @retval - Pointer to first available SSID name, returns NULL if no APs found.
 *
 * @note This function will return the first available SSID, 
 * for later AP names you should use the getNextAP() function.
 */
char* requestAPList(void);

/**
 * Get next available SSID name. 
 *
 * @param ssid - Pointer to store first available SSID name.
 *
 * @retval true - success.
 * @retval false - failure.
 *
 * @note Before using this function call requestAPList() fucntion.
 */
char* getNextAP(void);

/**
 * Get ESP8266 IP Address.
 *
 * @param ip - Pointer to store current IP address.
 * @retval true - success.
 * @retval false - failure.
 */
bool localIP(char* ip);

/**
 * Get ESP8266 MAC Address.
 *
 * @param ip - Pointer to store current IP address.
 * @retval true - success.
 * @retval false - failure.
 */
bool localMAC(char* mac);

/**
 * Flush ESP8266 serial buffer.
 */
void flush(void);

/**
 * Ping server or IP address
 *
 * @param address - Server or IP address to ping
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool ping(char *address);

/**
 * Start connection to TCP server.
 *
 * @param server - Server address to connect.
 * @param port - Server port to connect.
 * @retval true - success.
 * @retval false - failure.
 */
bool startTCP(char *server, char *port);

/**
 * Stop connection from current TCP server.
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool stopTCP(void);

/**
 * Send data to current TCP connection.
 *
 * @param data - Data to send.
 * @retval true - success.
 * @retval false - failure.
 */
bool send(String data);

/**
 * Get response for the last TCP connection.
 *
 * @param response - struct to store response.
 * @retval - Number of received bytes.
 */
uint16_t httpReceive(httpResponse* response);

#endif /* ESP8266_H */
