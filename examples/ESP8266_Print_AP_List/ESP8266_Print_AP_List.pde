#include <ESP8266.h>

#define ESP8266_RESET_PIN  4 /* Arduino pin D4 */
#define ESP8266_ENABLE_PIN 3 /* Arduino pin D3 */

/* Pointer to store each Access Point SSID name */
char* ssid_name = NULL;

void setup()
{
  /* Variable to count how many APs were found */
  int APCounter = 0;

  Serial.println("Print available Access Points:");

  /* Setup ESP8266 serial port and pins */
  setupESP8266(Serial1, 115200, ESP8266_RESET_PIN, ESP8266_ENABLE_PIN);

  /* Reset device */
  reset();

  /* Set ESP8266 to Station Mode */
  setWifiMode(ESP8266_MODE_STATION);

  /* Set connection mode to single */
  setConnMode(ESP8266_CONN_SINGLE);

  /* Both requestAPList() and getNextAP() will return NULL if no AP found */
  for(ssid_name = requestAPList(); ssid_name != NULL; ssid_name = getNextAP())
  {
    Serial.println(ssid_name);
    APCounter++;
  }
  
  Serial.print("Access Points found: ");
  Serial.println(APCounter);  
}

void loop()
{
}
