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

/* Physical interface */
#if (ESP8266_USE_SOFT_SERIAL == 1)
static SoftwareSerial *_serialPort = NULL;
#else
static HardwareSerial *_serialPort = NULL;
#endif

static int esp8266_EnablePin = -1;
static int esp8266_ResetPin = -1;

static char _ssidBuffer[ESP8266_MAX_SSID_LEN];

/* Command type, used by sendCommand function */
typedef enum at_cmd_type
{
    ESP8266_CMD_QUERY, ESP8266_CMD_SETUP, ESP8266_CMD_EXECUTE,
};

/**
 * Look for a response from the ESP8266, if found this function can return the instance that matches
 *
 * @param dest - Pointer to save instance if found
 * @param expected - Expected string to match
 * @param delimA - Delimiter character
 * @param delimB - Delimiter character
 * @param timeout - Timeout to match expected string
 *
 * @retval true - success.
 * @retval false - failure.
 */
static uint8_t getResponseTimeout(char* dest, const char* expected, char delimA, char delimB, uint32_t timeout);

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
    esp8266_ResetPin = rst;
    esp8266_EnablePin = en;
    _serialPort = &serialPort;
    _serialPort->begin(baud);

    /* Setup pins */
    pinMode(esp8266_ResetPin, OUTPUT);
    pinMode(esp8266_EnablePin, OUTPUT);
    digitalWrite(esp8266_ResetPin, HIGH);
    digitalWrite(esp8266_EnablePin, HIGH);
    delay(100);
}

void hardReset()
{
    digitalWrite(esp8266_ResetPin, LOW);
    delay(1000);
    digitalWrite(esp8266_ResetPin, HIGH);
    delay(1000);
}

bool ping()
{
    sendCommand(AT_TEST, ESP8266_CMD_EXECUTE, NULL);
    return getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 1000);
}

bool reset()
{
    flush();
    sendCommand(AT_RESET, ESP8266_CMD_EXECUTE, NULL);
    return getResponseTimeout(NULL, AT_RESPONSE_RST, NULL, NULL, 3000);
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
    return getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 3000);
}

bool setWifiMode(int mode)
{
    char modeStr[2];
    itoa(mode, modeStr, 10); /* Convert current int mode into ASCII (string) */
    sendCommand(AT_SET_WIFI_MODE, ESP8266_CMD_SETUP, modeStr);
    return getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 1000);
}

bool setConnMode(int mode)
{
    bool ret = false;
    char modeStr[2];
    itoa(mode, modeStr, 10); /* Convert current int mode into ASCII (string) */
    sendCommand(AT_CIPMUX, ESP8266_CMD_SETUP, modeStr);
    return getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 3000);
}

// Connect to Access Point
bool joinAP(char *ssid, char *ssid_pass)
{
    // Quit AP
    bool conn = false;
    uint8_t ucValidIP = false;
    uint32_t ulStartTime = 0;

    if (NULL != ssid)
    {
        conn = quitAP();

        if (conn) /* Join AP */
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

            conn = getResponseTimeout(NULL, "WIFI CONNECTED", NULL, NULL, 4000);
            if (conn)
            {
                conn = getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 3000);
            }

            if (!conn)
            {
                quitAP();
            }
        }
    }
    return conn;
}

// Disconnect from Access Point
bool quitAP(void)
{
    bool ret = false;
    sendCommand(AT_CWQAP, ESP8266_CMD_EXECUTE, NULL);
    ret = getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 3000);
    if (ret)
    {
        (void) getResponseTimeout(NULL, "WIFI DISCONNECT", NULL, NULL, 1000);
    }
    return ret;
}

bool getVersion(char *dest)
{
    bool ret = false;
    if (NULL != dest)
    {
        sendCommand(AT_GMR, ESP8266_CMD_EXECUTE, NULL);
        ret = getResponseTimeout(dest, "AT version", ':', '(', 1000);
    }
    return ret;
}

char* requestAPList(void)
{
    char* ssidName = NULL;
    sendCommand(AT_CWLAP, ESP8266_CMD_EXECUTE, NULL);
    if(getResponseTimeout(_ssidBuffer, AT_CWLAP_RX, '"', '"', 5000))
    {
        ssidName = (char*)&_ssidBuffer;
    }
    return ssidName;
}

char* getNextAP(void)
{
    char* ssidNext = NULL;
    if(getResponseTimeout(_ssidBuffer, AT_CWLAP_RX, '"', '"', 1000))
    {
        ssidNext = (char*)&_ssidBuffer;
    }
    return ssidNext;
}

bool startTCP(char *server, char *port)
{
    uint8_t ret = false;

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

        ret = getResponseTimeout(NULL, AT_CIPSTART_RX, NULL, NULL, 3000);
        ret &= getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 1000);

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
    uint8_t ret = false;
    sendCommand(AT_CIPCLOSE, ESP8266_CMD_EXECUTE, NULL);
    ret = getResponseTimeout(NULL, AT_CIPCLOSE_OK, NULL, NULL, 1000);
    if (ret)
    {
        ret = getResponseTimeout(NULL, AT_RESPONSE_OK, NULL, NULL, 1000);
    }
    return ret;
}

void flush(void)
{
    for (; _serialPort->available() > 0;)
    {
        (void) _serialPort->read();
    }
}

bool localIP(char *dest)
{
    uint8_t ret = false;
    /* Validate arguments */
    if (NULL != dest)
    {
        sendCommand(AT_CIFSR, ESP8266_CMD_EXECUTE, NULL);
        ret = getResponseTimeout(dest, AT_CIFSR_STATIP, '"', '"', 1000);
        if (ret)
        {
            (void) getResponseTimeout(NULL, AT_CIFSR_STAMAC, '"', '"', 1000);
        }
    }
    return ret;
}

/* TODO: Remove string usage */
bool send(String data)
{
    uint8_t ret = false;
    // uint16_t status = 404;

    data += "\r\n\r\n";

    sendCommand(AT_CIPSEND, ESP8266_CMD_SETUP, (char*)String(data.length()).c_str());
    ret = getResponseTimeout(NULL, ">", NULL, NULL, 1000);

    if (ret)
    {
        /* Send data */
        _serialPort->print(data);
        ret = getResponseTimeout(NULL, AT_CIPSEND_OK, NULL, NULL, 5000);
        /* TODO: return status */
        (void) getResponseTimeout(NULL, AT_IPD, ' ', ' ', 1000);
    }
    else
    {
        /* End connection if request failed */
        stopTCP();
    }

    return ret;
}

/* Private functions */

static void sendCommand(const char *cmd, at_cmd_type type, char *params)
{
    _serialPort->print("AT");
    _serialPort->print(cmd);
    if (ESP8266_CMD_QUERY == type)
    {
        _serialPort->print('?');
    }
    else if ((ESP8266_CMD_SETUP == type) && (NULL != params))
    {
        _serialPort->print('=');
        _serialPort->print(params);
    }
    _serialPort->print("\r\n");
}

static uint8_t getResponseTimeout(char* dest, const char* expected, char delimA, char delimB, uint32_t timeout)
{
    static char data[64]; /* Temp Memory */
    
    uint8_t ret = false;
    uint8_t idx = 0;
    uint32_t ulStartTime = 0;

    char *ucpStart = NULL;
    char *ucpEnd = NULL;

    /* Validate arguments */
    if (NULL != expected)
    {
        ESP8266_DBG_PARSE(F("EXP: "), expected);

        for (ulStartTime = millis(); (timeout >= (millis() - ulStartTime)) && (ret != true);)
        {
            idx = _serialPort->readBytesUntil('\n', data, sizeof(data));
            if (1 < idx)
            {
                data[idx] = '\0'; /* Always add NULL terminator */

                ESP8266_DBG_PARSE(F("ACT: "), data);

                if (0 == strncmp(expected, data, strlen(expected)))
                {
                    ESP8266_DBG_PARSE(F("FND: "), data);
                    if ((delimA != NULL) && (delimB != NULL))
                    {
                        ucpStart = (char *) memchr(data, delimA, idx);
                        if (NULL != ucpStart)
                        {
                            ucpStart++;
                            ucpEnd = (char *) memchr(ucpStart, delimB, idx - (ucpStart - data));
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
                                    ret = true;
                                }
                            }
                        }
                    }
                    else
                    {
                        ret = true;
                    }
                }
            }
        }
    }
    return ret;
}
