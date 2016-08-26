#include <ESP8266.h>

#define ESP8266_RESET_PIN  4 /* Arduino pin D4 */
#define ESP8266_ENABLE_PIN 3 /* Arduino pin D3 */

/* Pointer to store each Access Point SSID name */
char* ssid_name = NULL;

void setup()
{
  /* Variable to count how many APs were found */
  int APCounter = 0;
  Serial.begin(9600);
  Serial.println("Print available Access Points:");

  /* Setup ESP8266 serial port and pins */
  setupESP8266(Serial1, 115200, ESP8266_RESET_PIN, ESP8266_ENABLE_PIN);

  hardReset();

  delay(1000);
  if (!reset())
  {
    Serial.println("SOFT reset failed");
  }

  /* Wait until reset has been completed */
  delay(1000);
  flush();

  if (!ping())
  {
    Serial.println("Ping failed");
    flush();
  }

  if (!echo(false))
  {
    Serial.println("Echo enable failed");
    flush();
  }

  if (!setWifiMode(ESP8266_MODE_STATION))
  {
    Serial.println("Setup as client failed");
    flush();
  }

  if (!setConnMode(ESP8266_CONN_SINGLE))
  {
    Serial.println("Setup as single connection failed");
    flush();
  }

  delay(1000);

  /* Both requestAPList() and getNextAP() will return NULL if no AP found */
  for(ssid_name = requestAPList(); ssid_name != NULL; ssid_name = getNextAP())
  {

    Serial.print(APCounter);
    Serial.print(": ");
    Serial.println(ssid_name);
    APCounter++;
  }
  
  Serial.print("Access Points found: ");
  Serial.println(APCounter);  
}

void loop()
{
}
