#ifndef ESP8266_H
#define ESP8266_H

#define ESP8266_USE_SOFT_SERIAL       (0)     /* Set this value to 1 to enable SoftwareSerial usage */
#define ESP8266_DBG_EN                (0)     /* Enable/Disable ESP8266 Debug  */
#define ESP8266_DBG_PARSE_EN          (0)     /* Enable/Disable ESP8266 Debug  */

#include "Arduino.h"
#if (ESP8266_USE_SOFT_SERIAL == 1)
#include <SoftwareSerial.h>
#endif

#define ESP8266_MAX_SSID_LEN         32     /* Maximum SSID data length */
#define ESP8266_MAX_SSID              8     // Maximum SSID to be saved in List Access Points function

#if (ESP8266_USE_SOFT_SERIAL == 1)
void setupESP8266(SoftwareSerial &serialPort, uint32_t baud, int rst, int en);
#else
void setupESP8266(HardwareSerial &serialPort, uint32_t baud, int rst, int en);
#endif

/**
 * Ping (test) connection to ESP8266.
 *
 * @retval true - success.
 * @retval false - failure.
 */
bool ping(void);

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
void hardReset(void);

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
 * @param mode - Connection mode (1 - Single, 2 - Multi-Channel)
 *
 * @retval true - success.
 * @retval false - failure.
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
 * Get list of available Access Points.
 *
 * @param matrix - Array to store the AP list.
 * @param num - Number of available APs.
 * @retval true - success.
 * @retval false - failure.
 * @note This method will take a couple of seconds.
 */
bool getAPList(char matrix[][ESP8266_MAX_SSID_LEN], uint8_t* num);

/**
 * Get ESP8266 IP Address.
 *
 * @param ip - Pointer to store current IP address.
 * @retval true - success.
 * @retval false - failure.
 */
bool localIP(char* ip);

/**
 * Flush ESP8266 serial buffer.
 */
void flush(void);

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

#endif /* ESP8266_H */
