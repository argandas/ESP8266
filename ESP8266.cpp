/**
 * @file ESP8266.cpp
 * @author Hugo Arganda
 * @date 4 Aug 2016
 * @brief Arduino ESP8266 library.
 */

#include "ESP8266.h"

#define AT_EOL                  "\r\n"
#define AT_PING                 "AT"
#define AT_OK_RESPONSE          "OK"
#define AT_RESET                "AT+RST"
#define AT_RST_RESPONSE         "ready"
#define AT_ECHO_DIS             "ATE0"
#define AT_SET_WIFI_MODE        "AT+CWMODE_CUR="

#define AT_CIPMUX               "AT+CIPMUX="
#define AT_CIPMUX_QUERY         "AT+CIPMUX?"
#define AT_CIPMUX_RX            "+CIPMUX:"

#define AT_GMR                  "AT+GMR"

#define AT_CWJAP                "AT+CWJAP_CUR="
#define AT_CWQAP                "AT+CWQAP"
#define AT_CWLAP                "AT+CWLAP"
#define AT_CWLAP_RX             "+CWLAP:"

#define AT_CIFSR                "AT+CIFSR"
#define AT_CIFSR_STATIP         "+CIFSR:STAIP,"
#define AT_CIFSR_STAMAC         "+CIFSR:STAMAC,"

#define AT_IPD                  "+IPD,"

#define AT_CIPSTART             "AT+CIPSTART="
#define AT_CIPSTART_RX          "CONNECT"
#define AT_CIPSTART_RX_2        "ALREADY CONNECT"
#define AT_CIPSEND              "AT+CIPSEND="
#define AT_CIPSEND_RX           "SEND OK"
#define AT_CIPCLOSE             "AT+CIPCLOSE"
#define AT_CIPCLOSE_RX          "CLOSED"

#define CONN_TYPE_TCP           "TCP"

#define WIFI_MODE_STATION 		1  // Station mode
#define WIFI_MODE_AP 			2  // AP mode
#define WIFI_MODE_AP_STATION 	3  // AP + Station mode
#define CONN_MODE_SINGLE  		0  // Single connection mode
#define CONN_MODE_MULTIPLE  	1  // Multi-Channel connection mode
#define ESP_IP_BUFF_SIZE        16

#if (ESP8266_DBG_EN == 1)
#define ESP8266_DBG(data)\
    Serial.print(F("[ESP8266] "));\
    Serial.println(data);
#endif

/* Macro for debug ESP8266 Responses */
#if (ESP8266_DBG_PARSE_EN == 1)
#define ESP8266_DBG_PARSE(data)    ESP8266_DBG(data)
#else
#define ESP8266_DBG_PARSE(data)
#endif

/* This buffer handles the join command
 * AT+CWJAP_CUR="SSID","PASSWORD"
 */
#define CONN_APP_BUFFER_SIZE 128

/* Physical interface */
#if (ESP8266_USE_SOFT_SERIAL == 1)
static SoftwareSerial *port = NULL;
#else
static HardwareSerial *port = NULL;
#endif

static int esp8266_EnablePin = -1;
static int esp8266_ResetPin = -1;

/* Buffer for ESP IP Address */
static char esp_ip[ESP_IP_BUFF_SIZE];

/**
 * Enable/Disable multiple connections
 *
 * @param mode - Connection mode (1 - Single, 2 - Multi-Channel)
 *
 * @retval true - success.
 * @retval false - failure.
 */
static uint8_t getResponseTimeout(char* dest, char* expected, char delimA, char delimB, uint32_t timeout);

uint8_t requestStatus(void);

#if (ESP8266_USE_SOFT_SERIAL == 1)
void setupESP8266(SoftwareSerial &serialPort, uint32_t baud, int rst, int en)
#else
void setupESP8266(HardwareSerial &serialPort, uint32_t baud, int rst, int en)
#endif
{
    /* Save hardware configurations */
    esp8266_ResetPin = rst;
    esp8266_EnablePin = en;
    port = &serialPort;
    port->begin(baud);

    /* Setup pins */
    pinMode(esp8266_ResetPin, OUTPUT);
    pinMode(esp8266_EnablePin, OUTPUT);
    digitalWrite(esp8266_ResetPin, HIGH);
    digitalWrite(esp8266_EnablePin, HIGH);
    delay(100);
}

void hardReset()
{
    ESP8266_DBG(F("HARD reset"));
    digitalWrite(esp8266_ResetPin, LOW);
    delay(1000);
    digitalWrite(esp8266_ResetPin, HIGH);
    delay(1000);
}

bool ping()
{
    sendCommand(AT_PING);
    return getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 3000);
}

// Send command
void sendCommand(char *cmd)
{
    ESP8266_DBG("CMD: " + String(cmd));
    port->println(cmd);
}

bool reset()
{
    flush();
    ESP8266_DBG(F("SOFT reset"));
    sendCommand(AT_RESET);
    return getResponseTimeout(NULL, AT_RST_RESPONSE, NULL, NULL, 3000);
}

bool echo(bool enable)
{
    char cmd[6];
    /* Save ECHO_DISABLE cmd */
    strcpy(cmd, AT_ECHO_DIS);
    if (enable)
    {
        /* Replace '0' by '1' to enable ECHO */
        cmd[3] = '1';
    }
    sendCommand(cmd);
    return getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 3000);
}

bool setWifiMode(int mode)
{
    bool ret = false;
    char cmd[16];
    char modeStr[2];

    /* Validate available modes */
    if (mode <= 3)
    {
        /* Convert current int mode into ASCII (string) */
        itoa(mode, modeStr, 10);

        /* Build command */
        strcpy(cmd, AT_SET_WIFI_MODE);
        strcat(cmd, modeStr);

        /* Send command */
        sendCommand(cmd);
        ret = getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 1000);
    }
    return ret;
}

bool setConnMode(int mode)
{
    bool ret = false;
    char desiredMode[16];
    char modeStr[2];

    /* Validate available modes */
    if (mode <= 1)
    {
        /* Convert current int mode into ASCII (string) */
        itoa(mode, modeStr, 10);
        /* Copy desired response from ESP8266 */
        strcpy(desiredMode, AT_CIPMUX_RX);
        /* Concatenate the desire mode in ASCII format */
        strcat(desiredMode, modeStr);
        /* Check current mode, if already in desired mode, then return TRUE */
        sendCommand(AT_CIPMUX_QUERY);
        ret = getResponseTimeout(NULL, desiredMode, NULL, NULL, 3000);
        if (!ret)
        {
            /* Set desired mode */
            strcpy(&desiredMode[0], AT_CIPMUX);
            /* Concatenate desired mode (ASCII)*/
            strcat(desiredMode, modeStr);
            /* Send command */
            sendCommand(desiredMode);
            ret = getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 3000);
        }
        else
        {
            /* Mode is already selected, look for OK response */
            ret = getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 3000);
        }
    }
    return ret;
}

// Connect to Access Point
bool joinAP(char *ssid, char *ssid_pass)
{
    // Quit AP
    bool conn = false;
    uint8_t ucValidIP = false;
    uint32_t ulStartTime = 0;
    char cmd[CONN_APP_BUFFER_SIZE];

    if ((NULL == ssid) || (NULL == ssid_pass))
    {
        ESP8266_DBG(F("ERR: Passed null pointers"));
    }
    else
    {
        conn = quitAP();

        if (conn) /* Join AP */
        {
            ESP8266_DBG("Join AP \"" + String(ssid) + "\"");

            strcpy(cmd, AT_CWJAP);
            strcat(cmd, "\"");
            strcat(cmd, ssid);
            strcat(cmd, "\",\"");
            strcat(cmd, ssid_pass);
            strcat(cmd, "\"");

            sendCommand(cmd);
            conn = getResponseTimeout(NULL, "WIFI CONNECTED", NULL, NULL, 4000);
            if (conn)
            {
                conn = getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 3000);
            }

            flush();
            if (conn)
            {
                /* Connected to AP, now check for valid IP */
                ulStartTime = millis();
                for(ucValidIP = false; !ucValidIP && ((millis() - ulStartTime) < 5000); delay(500))
                {
                    ucValidIP = localIP(esp_ip);
                }

                if (ucValidIP)
                {
                    ESP8266_DBG("Local IP: " + String(esp_ip));
                }
                else
                {
                    ESP8266_DBG(F("Failed to get IP"));
                    conn = false;
                }
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

    ESP8266_DBG(F("Disconnect from AP"));

    sendCommand(AT_CWQAP);
    ret = getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 3000);
    if (ret)
    {
        (void) getResponseTimeout(NULL, "WIFI DISCONNECT", NULL, NULL, 1000);
    }
    return ret;
}

bool getVersion(char *dest)
{
    bool ret = false;
    char firmwareVersion[8];

    if (NULL != dest)
    {
        ESP8266_DBG(F("Firmware version..."));
        sendCommand(AT_GMR);
        ret = getResponseTimeout(firmwareVersion, "AT version", ':', '(', 3000);
        if (ret)
        {
            ESP8266_DBG(firmwareVersion);
            strcpy(dest, firmwareVersion);
        }
    }
    else
    {
        ESP8266_DBG(F("ERR: NULL pointer"));
    }
    return ret;
}

bool getAPList(char matrix[][ESP8266_MAX_SSID_LEN], uint8_t* num)
{
    uint8_t ret = true;
    uint8_t ucParseOK = true;
    uint8_t qty = 0; /* Number of APs found */

    ESP8266_DBG(F("List Access Points..."));

    /* Validate arguments */
    if ((NULL == matrix) || (NULL == num))
    {
        ESP8266_DBG(F("ERR: NULL pointer"));
        ret = false;
    }
    else
    {
        /* Save first AP Name */
        sendCommand(AT_CWLAP);
        ucParseOK = getResponseTimeout(&(matrix[qty][0]), AT_CWLAP_RX, '"', '"', 5000);
        if (ucParseOK)
        {
            qty++;
            for (; (ucParseOK) && (ESP8266_MAX_SSID > qty);)
            {
                /* Save AP Names */
                ucParseOK = getResponseTimeout(&(matrix[qty][0]), AT_CWLAP_RX, '"', '"', 500);
                if (ucParseOK)
                {
                    qty++;
                }
            }
        }

        /* Save AP Number */
        *num = qty;
        ESP8266_DBG("APs found: " + String(qty));

        if (0 >= qty)
        {
            /* Clean full array to avoid previous data stored */
            for (qty = 0; ESP8266_MAX_SSID > qty; qty++)
            {
                matrix[qty][0] = '\0'; /* Add NULL terminator for empty string */
            }
        }
    }

    return ret;
}

bool startTCP(char *server, char *port)
{
    uint8_t ret = false;
    char cmd[64];

    if ((NULL == server) || (NULL == port))
    {
        ESP8266_DBG(F("ERR: NULL pointer"));
    }
    else
    {
        flush();
        /* Build command */
        strcpy(cmd, AT_CIPSTART);
        strcat(cmd, "\"");
        strcat(cmd, CONN_TYPE_TCP);
        strcat(cmd, "\",\"");
        strcat(cmd, server);
        strcat(cmd, "\",");
        strcat(cmd, port);
        sendCommand(cmd);
        ret = getResponseTimeout(NULL, AT_CIPSTART_RX, NULL, NULL, 3000);
        ret &= getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 1000);

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
    sendCommand(AT_CIPCLOSE);
    ret = getResponseTimeout(NULL, AT_CIPCLOSE_RX, NULL, NULL, 1000);
    if (ret)
    {
        ret = getResponseTimeout(NULL, AT_OK_RESPONSE, NULL, NULL, 1000);
    }
    return ret;
}

void flush(void)
{
    for(;port->available() > 0;)
    {
        (void) port->read();
    }
}

bool localIP(char *dest)
{
    uint8_t ret = false;
    /* Validate arguments */
    if (NULL == dest)
    {
        ESP8266_DBG(F("ERR: NULL pointer"));
    }
    else
    {
        sendCommand(AT_CIFSR);
        ret = getResponseTimeout(dest, AT_CIFSR_STATIP, '"', '"', 1000);
        if (ret)
        {
            (void) getResponseTimeout(NULL, AT_CIFSR_STAMAC, '"', '"', 1000);
        }
    }
    return ret;
}

static uint8_t getResponseTimeout(char* dest, char* expected, char delimA, char delimB, uint32_t timeout)
{
    uint8_t ret = false;
    uint8_t idx = 0;
    uint32_t ulStartTime = 0;

    char data[64]; /* Temp Memory */

    char *ucpStart = NULL;
    char *ucpEnd = NULL;

    /* Validate arguments */
    if (NULL == expected)
    {
        ESP8266_DBG(F("ERR: Passed null pointer"));
    }
    else
    {
        ESP8266_DBG_PARSE("EXP: \"" + String(expected) + "\"");

        for (ulStartTime = millis(); (timeout >= (millis() - ulStartTime)) && (ret != true);)
        {
            idx = port->readBytesUntil('\n', data, sizeof(data));
            if (1 < idx)
            {
                data[idx] = '\0'; /* Always add NULL terminator */

                if (0 == strncmp(expected, data, strlen(expected)))
                {
                    ESP8266_DBG_PARSE("FND: \"" + String(data) + "\"");
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
                                    ESP8266_DBG_PARSE("INS: \"" + String(ucpStart) + "\"");
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
                else if (0 == strncmp("ERROR", data, strlen("ERROR")))
                {
                    ESP8266_DBG(F("DEVICE ERROR"));
                }
                else if (0 == strncmp("busy", data, strlen("busy")))
                {
                    ESP8266_DBG(F("DEVICE BUSY"));
                }
                else
                {
                    ESP8266_DBG_PARSE("<< : \"" + String(data) + "\"");
                }
            }
        }
    }

    return ret;
}

/* Return connection status */
uint8_t requestStatus(void)
{
    uint8_t status = -1;
    char uCHTTPStatus[4] = "404";
    uint8_t ret = true;
    /* Get response status */
    if (getResponseTimeout(uCHTTPStatus, AT_IPD, ' ', ' ', 1000))
    {
        if (0 == strncmp("200", uCHTTPStatus, strlen("200")))
        {
            status = 200;
        }
        else if (0 == strncmp("404", uCHTTPStatus, strlen("404")))
        {
            status = 404;
        }
        else if (0 == strncmp("403", uCHTTPStatus, strlen("403")))
        {
            status = 403;
        }
        else if (0 == strncmp("500", uCHTTPStatus, strlen("500")))
        {
            status = 500;
        }
        else if (0 == strncmp("504", uCHTTPStatus, strlen("504")))
        {
            status = 504;
        }
        else
        {
            ESP8266_DBG("UNK Status: " + String(uCHTTPStatus));
        }
    }
    return status;
}

bool send(String data)
{
    uint8_t ret = false;
    uint8_t status = false;
    char cmd[16];

    data += "\r\n\r\n";

    /* Build command */
    strcpy(cmd, AT_CIPSEND);
    strcat(cmd, String(data.length()).c_str());

    sendCommand(cmd);

    ret = getResponseTimeout(NULL, ">", NULL, NULL, 1000);
    if (ret) /* Send data */
    {
        ESP8266_DBG(F("Sending data"));
        port->print(data);
        ret = getResponseTimeout(NULL, AT_CIPSEND_RX, NULL, NULL, 5000);
        if (ret) /* Wait for status */
        {
            status = requestStatus();
            ESP8266_DBG("CONN STAT: " + String(status));
            if (200 != status)
            {
                ret = false;
            }
        }
    }
    else
    {
        ESP8266_DBG(F("Unable to find wrap return \">\""));
    }

    if (!ret) /* End connection if request failed */
    {
        sendCommand(AT_EOL);
        sendCommand(AT_EOL);
        ping();
        stopTCP();
    }

    return ret;
}
