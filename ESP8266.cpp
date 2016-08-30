/**
 * @file ESP8266.cpp
 * @author Hugo Arganda
 * @date 4 Aug 2016
 * @brief Arduino ESP8266 library.
 */

#include "ESP8266.h"
#include "ESP8266_AT_CMD.h"

/* Macro for debug ESP8266 Responses */
#if (ESP8266_DBG_PARSE_EN == 1)
#define ESP8266_DBG_PARSE(label, data)\
    Serial.print(F("[ESP8266] "));\
    Serial.print(label);\
    Serial.println(data);
#else
#define ESP8266_DBG_PARSE(label, data)
#endif

#if (ESP8266_DBG_HTTP_RES == 1)
#define ESP8266_DBG_HTTP(label, data) ESP8266_DBG_PARSE(label, data)
#else
#define ESP8266_DBG_HTTP(label, data)
#endif

/* Command type, used by sendCommand function */
typedef enum at_cmd_type
{
    ESP8266_CMD_QUERY, ESP8266_CMD_SETUP, ESP8266_CMD_EXECUTE,
};

/* Response codes for getResponse */
typedef enum cmd_rsp_code
{
    ESP8266_CMD_RSP_FAILED = -4, ESP8266_CMD_RSP_TIMEOUT = -3, ESP8266_CMD_RSP_BUSY = -2, ESP8266_CMD_RSP_ERROR = -1, ESP8266_CMD_RSP_WAIT = 0, ESP8266_CMD_RSP_SUCCESS = 1,
};

/* Private data */
#if (ESP8266_USE_SOFT_SERIAL == 1)
static SoftwareSerial* _serialPort = NULL;
#else
static HardwareSerial* _serialPort = NULL;
#endif
static int _enablePin = -1;
static int _resetPin = -1;
static char _ssidBuffer[ESP8266_MAX_SSID_LEN];

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
static int8_t getResponse(char* dest, const char* pass, const char* fail, char delimA, char delimB, uint32_t timeout);

/**
 * Send command to ESP8266.
 *
 * @param cmd - Command to send
 * @param type - Command type, check at_cmd_type
 * @param params - Parameters to use when passed setup type
 */
static void sendCommand(const char *cmd, at_cmd_type type, char *params);

#if (ESP8266_USE_SOFT_SERIAL == 1)
void setupESP8266(SoftwareSerial &serialPort, uint32_t baud, int rst, int en)
#else
void setupESP8266(HardwareSerial &serialPort, uint32_t baud, int rst, int en)
#endif
{
    /* Save hardware configurations */
    _resetPin = rst;
    _enablePin = en;
    _serialPort = &serialPort;
    _serialPort->begin(baud);

    /* Setup pins */
    pinMode(_resetPin, OUTPUT);
    pinMode(_enablePin, OUTPUT);
    digitalWrite(_resetPin, HIGH);
    digitalWrite(_enablePin, HIGH);
    delay(100);
}

bool hardReset()
{
    digitalWrite(_resetPin, LOW);
    delay(1000);
    digitalWrite(_resetPin, HIGH);
    return (getResponse(NULL, AT_RESPONSE_RST, NULL, NULL, NULL, 1000) > 0);
}

bool test()
{
    sendCommand(AT_TEST, ESP8266_CMD_EXECUTE, NULL);
    return (getResponse(NULL, AT_RESPONSE_OK, NULL, NULL, NULL, 1000) > 0);
}

bool reset()
{
    flush();
    sendCommand(AT_RESET, ESP8266_CMD_EXECUTE, NULL);
    return (getResponse(NULL, AT_RESPONSE_RST, NULL, NULL, NULL, 3000) > 0);
}

bool echo(bool enable)
{
    if (enable)
    {
        sendCommand(AT_ECHO_ENABLE, ESP8266_CMD_EXECUTE, NULL);
    }
    else
    {
        sendCommand(AT_ECHO_DISABLE, ESP8266_CMD_EXECUTE, NULL);
    }
    return (getResponse(NULL, AT_RESPONSE_OK, NULL, NULL, NULL, 3000) > 0);
}

bool setWifiMode(int mode)
{
    char modeStr[2];
    itoa(mode, modeStr, 10); /* Convert current int mode into ASCII (string) */
    sendCommand(AT_SET_WIFI_MODE, ESP8266_CMD_SETUP, modeStr);
    return (getResponse(NULL, AT_RESPONSE_OK, NULL, NULL, NULL, 1000) > 0);
}

bool setConnMode(int mode)
{
    bool ret = false;
    char modeStr[2];
    itoa(mode, modeStr, 10); /* Convert current int mode into ASCII (string) */
    sendCommand(AT_CIPMUX, ESP8266_CMD_SETUP, modeStr);
    return (getResponse(NULL, AT_RESPONSE_OK, NULL, NULL, NULL, 3000) > 0);
}

// Connect to Access Point
bool joinAP(char *ssid, char *ssid_pass)
{
    bool conn = false;

    if (NULL != ssid)
    {
        _serialPort->print(AT_CMD);
        _serialPort->print(AT_CWJAP);
        _serialPort->print("=\"");
        _serialPort->print(ssid);
        _serialPort->print("\"");
        if (NULL != ssid_pass)
        {
            _serialPort->print(",\"");
            _serialPort->print(ssid_pass);
            _serialPort->print("\"");
        }
        _serialPort->print("\r\n");

        conn = (getResponse(NULL, "WIFI CONNECTED", NULL, NULL, NULL, 4000) > 0);
        if (conn)
        {
            conn = (getResponse(NULL, AT_RESPONSE_OK, NULL, NULL, NULL, 5000) > 0);
        }

        if (!conn)
        {
            quitAP();
        }
    }
    return conn;
}

// Disconnect from Access Point
bool quitAP(void)
{
    bool ret = false;
    sendCommand(AT_CWQAP, ESP8266_CMD_EXECUTE, NULL);
    ret = (getResponse(NULL, AT_RESPONSE_OK, NULL, NULL, NULL, 3000) > 0);
    if (ret)
    {
        (void) getResponse(NULL, "WIFI DISCONNECT", NULL, NULL, NULL, 1000);
    }
    return ret;
}

bool getVersion(char *dest)
{
    sendCommand(AT_GMR, ESP8266_CMD_EXECUTE, NULL);
    return (getResponse(dest, "AT version", NULL, ':', '(', 1000) > 0);
}

char* requestAPList(void)
{
    char* ssidName = NULL;
    sendCommand(AT_CWLAP, ESP8266_CMD_EXECUTE, NULL);
    if (getResponse(_ssidBuffer, AT_CWLAP_RX, NULL, '"', '"', 5000) > 0)
    {
        ssidName = (char*) &_ssidBuffer;
    }
    return ssidName;
}

char* getNextAP(void)
{
    char* ssidNext = NULL;
    if (getResponse(_ssidBuffer, AT_CWLAP_RX, NULL, '"', '"', 1000) > 0)
    {
        ssidNext = (char*) &_ssidBuffer;
    }
    return ssidNext;
}

bool ping(char *address)
{
    _serialPort->print("AT");
    _serialPort->print(AT_PING);
    _serialPort->print("=\"");
    _serialPort->print(address);
    _serialPort->print("\"\r\n");
    return (getResponse(NULL, AT_RESPONSE_OK, NULL, NULL, NULL, 5000) > 0);
}

bool startTCP(char *server, char *port)
{
    uint8_t ret = false;
    int8_t conn = 0;

    if ((NULL != server) && (NULL != port))
    {
        flush();

        /* Build command */
        _serialPort->print("AT");
        _serialPort->print(AT_CIPSTART);
        _serialPort->print("=\"TCP\",\"");
        _serialPort->print(server);
        _serialPort->print("\",");
        _serialPort->print(port);
        _serialPort->print("\r\n");

        conn = getResponse(NULL, AT_CIPSTART_RX, AT_CIPSTART_ALRDY, NULL, NULL, 3000);
        ret = ((conn == ESP8266_CMD_RSP_FAILED) || (conn > 0));

        /* If failed to connect, disconnect */
        if (!ret)
        {
            stopTCP();
        }
    }
    return ret;
}

bool stopTCP(void)
{
    int8_t conn = false;
    sendCommand(AT_CIPCLOSE, ESP8266_CMD_EXECUTE, NULL);
    conn = getResponse(NULL, AT_RESPONSE_OK, AT_RESPONSE_ERROR, NULL, NULL, 1000);
    return ((conn == ESP8266_CMD_RSP_FAILED) || (conn > 0));
}

void flush(void)
{
    for (; _serialPort->available() > 0;)
    {
        (void) _serialPort->read();
    }
}

bool localIP(char *ip)
{
    sendCommand(AT_CIFSR, ESP8266_CMD_EXECUTE, NULL);
    return (getResponse(ip, AT_CIFSR_STATIP, NULL, '"', '"', 1000) > 0);
}

bool localMAC(char *mac)
{
    sendCommand(AT_CIFSR, ESP8266_CMD_EXECUTE, NULL);
    return (getResponse(mac, AT_CIFSR_STAMAC, NULL, '"', '"', 1000) > 0);
}

/* TODO: Remove string usage */
bool send(String data)
{
    uint8_t ret = false;
    // uint16_t status = 404;

    data += "\r\n\r\n";

    sendCommand(AT_CIPSEND, ESP8266_CMD_SETUP, (char*) String(data.length()).c_str());
    ret = (getResponse(NULL, ">", NULL, NULL, NULL, 1000) > 0);

    if (ret)
    {
        /* Send data */
        _serialPort->print(data);
        ret = (getResponse(NULL, AT_CIPSEND_OK, NULL, NULL, NULL, 5000) > 0);
    }
    else
    {
        /* End connection if request failed */
        stopTCP();
    }

    return ret;
}

uint16_t httpReceive(httpResponse* response)
{
    uint16_t rxLength = 0;
    int16_t remaining = 0;

    char *ucpStart = NULL;
    char *ucpEnd = NULL;

    uint16_t count = (uint16_t) getResponse(response->content, AT_IPD, NULL, ',', '\r', 1000);
    if (0 < count)
    {
        /* Get response length */
        ucpEnd = (char *) memchr(response->content, ':', count - (ucpStart - response->content));
        if (NULL != ucpEnd)
        {
            if ((ucpEnd - response->content) > 1)
            {
                *ucpEnd = '\0';
                rxLength = (uint16_t) atoi(response->content);
            }
        }

        /* Get response status */
        ucpStart = (char *) memchr(ucpEnd, ' ', count);
        if (NULL != ucpStart)
        {
            ucpStart++;
            ucpEnd = (char *) memchr(ucpStart, ' ', count - (ucpStart - response->content));
            if (NULL != ucpEnd)
            {
                if ((ucpEnd - ucpStart) > 1)
                {
                    *ucpEnd = '\0';
                    response->status = (uint16_t) atoi(ucpStart);
                }
            }
        }

        rxLength = (uint16_t) atoi(response->content);
        count -= (strlen(AT_IPD) + strlen(response->content) + 2);
        remaining = rxLength - count;

        ESP8266_DBG_HTTP("CNT: ", count);
        ESP8266_DBG_HTTP("RMN: ", remaining);

        /* TODO: Add timeout */
        while (0 < remaining)
        {
            if (remaining < ESP8266_RX_BUFF_LEN)
            {
                count = (uint16_t) _serialPort->readBytesUntil('\n', response->content, remaining);
            }
            else
            {
                count = (uint16_t) _serialPort->readBytesUntil('\n', response->content, ESP8266_RX_BUFF_LEN);
            }

            if (0 < count)
            {
                if (remaining != count)
                {
                    /* Always add NULL terminator */
                    if (count < ESP8266_RX_BUFF_LEN)
                    {
                        response->content[count] = '\0';
                        count++;
                    }
                    else
                    {
                        response->content[ESP8266_RX_BUFF_LEN - 1] = '\0';
                    }
                    ESP8266_DBG_HTTP("CNT: ", count);

                    /* Get content length */
                    if (0 == strncmp("Content-Length:", response->content, strlen("Content-Length:")))
                    {
                        ucpStart = (char *) memchr(response->content, ' ', count);
                        if (NULL != ucpStart)
                        {
                            ucpStart++;
                            ucpEnd = (char *) memchr(ucpStart, '\0', count - (ucpStart - response->content));
                            if (NULL != ucpEnd)
                            {
                                if ((ucpEnd - ucpStart) > 1)
                                {
                                    *ucpEnd = '\0';
                                    response->len = (uint16_t) atoi(ucpStart);
                                }
                            }
                        }
                    }

                }
                else
                {
                    /* Last batch of data is the response content */
                    /* There's nothing to do since the data is already on the content body at this point */
                }

                remaining -= count;
                ESP8266_DBG_HTTP("RMN: ", remaining);
            }
        }
    }
    return rxLength;
}

/* Private functions */

static void sendCommand(const char *cmd, at_cmd_type type, char *params)
{
    ESP8266_DBG_PARSE(F("CMD: "), cmd);
    _serialPort->print("AT");
    _serialPort->print(cmd);
    if (ESP8266_CMD_QUERY == type)
    {
        _serialPort->print('?');
    }
    else if ((ESP8266_CMD_SETUP == type) && (NULL != params))
    {
        ESP8266_DBG_PARSE(F("PRM: "), params);
        _serialPort->print('=');
        _serialPort->print(params);
    }
    _serialPort->print("\r\n");
}

static int8_t getResponse(char* dest, const char* pass, const char* fail, char delimA, char delimB, uint32_t timeout)
{
    static char _rxBuffer[ESP8266_RX_BUFF_LEN];
    int8_t ret = ESP8266_CMD_RSP_WAIT;
    uint8_t idx = 0;
    uint32_t ulStartTime = 0;

    char *ucpStart = NULL;
    char *ucpEnd = NULL;

    /* Validate arguments */
    if (NULL == pass)
    {
        ret = ESP8266_CMD_RSP_ERROR;
    }
    else
    {
        ESP8266_DBG_PARSE(F("EXP: "), pass);

        for (ulStartTime = millis(); ESP8266_CMD_RSP_WAIT == ret;)
        {
            /* Check for timeout */
            if (timeout < (millis() - ulStartTime))
            {
                ESP8266_DBG_PARSE(F("TIMEOUT: "), (millis() - ulStartTime));
                ret = ESP8266_CMD_RSP_TIMEOUT;
                break;
            }

            /* Read line */
            idx = _serialPort->readBytesUntil('\n', _rxBuffer, sizeof(_rxBuffer));
            if (0 < idx)
            {
                /* Always add NULL terminator */
                if (idx < ESP8266_RX_BUFF_LEN)
                {
                    _rxBuffer[idx] = '\0';
                }
                else
                {
                    _rxBuffer[ESP8266_RX_BUFF_LEN - 1] = '\0';
                }

                ESP8266_DBG_PARSE(F("ACT: "), _rxBuffer);

                /* Check for expected response */
                if (0 == strncmp(pass, _rxBuffer, strlen(pass)))
                {
                    ESP8266_DBG_PARSE(F("FND: "), _rxBuffer);

                    /* Search for delimeters */
                    if ((delimA != NULL) && (delimB != NULL))
                    {
                        ucpStart = (char *) memchr(_rxBuffer, delimA, idx);
                        if (NULL != ucpStart)
                        {
                            ucpStart++;
                            ucpEnd = (char *) memchr(ucpStart, delimB, idx - (ucpStart - _rxBuffer));
                            if (NULL != ucpEnd)
                            {
                                if ((ucpEnd - ucpStart) > 1)
                                {
                                    *ucpEnd = '\0';
                                    ESP8266_DBG_PARSE("INS: ", ucpStart);
                                    if (NULL != dest)
                                    {
                                        strcpy(dest, ucpStart);
                                    }
                                    ret = idx;
                                    if (idx < ESP8266_RX_BUFF_LEN)
                                    {
                                        ret++;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        /* Expected response found and no need to find instances */
                        ret = ESP8266_CMD_RSP_SUCCESS;
                    }
                }
                /* Check for failed response */
                else if (NULL != fail)
                {
                    if (0 == strncmp(fail, _rxBuffer, strlen(fail)))
                    {
                        ret = ESP8266_CMD_RSP_FAILED;
                    }
                }
                /* Check if device is busy */
                else if (0 == strncmp(AT_RESPONSE_BUSY, _rxBuffer, strlen(AT_RESPONSE_BUSY)))
                {
                    ret = ESP8266_CMD_RSP_BUSY;
                }
                /* Check if there is an error */
                else if (0 == strncmp(AT_RESPONSE_ERROR, _rxBuffer, strlen(AT_RESPONSE_ERROR)))
                {
                    ret = ESP8266_CMD_RSP_ERROR;
                }
            }
        }
    }
    return ret;
}
