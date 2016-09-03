#ifndef ESP8266_H
#define ESP8266_H

#include "Arduino.h"
#include <SoftwareSerial.h>

#define ESP8266_USE_HW_SW_PORT      (1)  /* Enable/Disable ESP8266 Debug  */

#define ESP8266_DBG_PARSE_EN        (0)  /* Enable/Disable ESP8266 Debug  */
#define ESP8266_DBG_HTTP_RES        (0)  /* Enable/Disable ESP8266 Debug for HTTP responses */

#define ESP8266_MODE_STATION        (1)  /* Station mode */
#define ESP8266_MODE_AP             (2)  /* AP mode */
#define ESP8266_MODE_AP_STATION     (3)  /* AP + Station mode */

#define ESP8266_CONN_SINGLE         (0)  /* Single connection mode */
#define ESP8266_CONN_MULTIPLE       (1)  /* Multi-Channel connection mode */

#define ESP8266_RX_BUFF_LEN        (64)  /* ESP8266 Rx Buffer length */
#define ESP8266_MAX_SSID_LEN       (32)  /* Maximum SSID data length */

class ESP8266: public Stream
{
    public:
        /**
         * Class constructor
         *
         * @param rst - ESP8266 Reset Pin
         * @param en - ESP8266 Enable Pin
         */
        ESP8266(int rst, int en);

        /**
         * Start connection to serial port
         *
         * @param serialPort - Serial port where ESP8266 Tx/Rx are connectected
         * @param baud - ESP8266 current baud rate
         */
        void begin(SoftwareSerial &serialPort, uint32_t baud);
        void begin(HardwareSerial &serialPort, uint32_t baud);

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
        bool operationMode(int mode);

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
        bool connectionMode(int mode);

        /**
         * Get current ESP8266´s firmware version of AT Command Set.
         *
         * @param dest - Pointer to char to save version information
         *
         * @retval true - success.
         * @retval false - failure.
         */
        bool version(char *dest);

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
        bool startTCP(char *server, int port);

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
         * Start data send to TCP connection .
         *
         * @param len - Data length to send.
         * @retval true - ready to send.
         * @retval false - failure.
         */
        bool startSendTCP(int len);

        /**
         * Wait for data send OK message when sending data trough TCP connection.
         *
         * @retval true - Data send OK.
         * @retval false - failure.
         */
        bool endSendTCP(void);

        /**
         * Get status from HTTP request.
         *
         * @retval - Request status.
         */
        int httpStatus(void);

#if 0
        /**
         * Get response for the last TCP connection.
         *
         * @param response - struct to store response.
         * @retval - Number of received bytes.
         */
        uint16_t httpReceive(httpResponse* response);
#endif

        /**
         * Virtual method to match Stream class
         */
        virtual size_t write(uint8_t);

        virtual int available();
        virtual int read();
        virtual int peek();
        virtual void flush();

    private:
        /* Command type, used by sendCommand function */
        typedef enum at_cmd_type
        {
            ESP8266_CMD_QUERY, ESP8266_CMD_SETUP, ESP8266_CMD_EXECUTE,
        };

        /* Response codes for getResponse */
        typedef enum cmd_rsp_code
        {
            ESP8266_CMD_RSP_FAILED = -4,
            ESP8266_CMD_RSP_TIMEOUT = -3,
            ESP8266_CMD_RSP_BUSY = -2,
            ESP8266_CMD_RSP_ERROR = -1,
            ESP8266_CMD_RSP_WAIT = 0,
            ESP8266_CMD_RSP_SUCCESS = 1,
        };

        typedef struct serialPorthandler
        {
                SoftwareSerial* _soft;
                HardwareSerial* _hard;
                bool isSoftSerial;
        };

        /* ESP8266 Serial Port handler */
        serialPorthandler _serialPortHandler;

        /* ESP8266 control pins */
        int _enablePin;
        int _resetPin;

        char _ssidBuffer[ESP8266_MAX_SSID_LEN];

        /**
         * Setup ESP8266 Serial port.
         *
         * @param baud - ESP8266 current baud rate
         */
        void setup(uint32_t baud);

        /**
         * Look for a response from the ESP8266, if found this function can return the instance that matches
         *
         * @param dest - Pointer to save instance if found
         * @param pass - Expected response if succeed
         * @param fail - Expected response if fails
         * @param delimA - Delimiter character
         * @param delimB - Delimiter character
         * @param timeout - Timeout to match expected responses
         *
         * @retval cmd_rsp_code (1 =  success).
         */
        int8_t getResponse(char* dest, const char* pass, const char* fail, char delimA, char delimB, uint32_t timeout);

        /**
         * Send command to ESP8266.
         *
         * @param cmd - Command to send
         * @param type - Command type, check at_cmd_type
         * @param params - Parameters to use when passed setup type
         */
        void sendCommand(const char *cmd, at_cmd_type type, char *params);
};

#endif /* ESP8266_H */
