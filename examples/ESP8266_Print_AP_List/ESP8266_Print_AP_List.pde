/*
 ESP8266_Print_AP_List.pde
 Print the list of available Access Points.

 NOTE: 
 This example use Serial1, which is of type HardwareSerial, 
 only some Arduino boards have this, 
 if needed change this example to use SoftwareSerial.

 modified on 29 Aug 2016
 by @argandas
 http://www.github.com/argandas/ESP8266
*/

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

  if(!hardReset())
  {
    if(reset())
    {
      Serial.println("ESP8266 reset OK");
    }
  }

  if(test())
  {
    Serial.println("ESP8266 is alive!");
  }

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
  /* Do nothing */
}
