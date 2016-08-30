/*
 SoftwareSerial.pde
 In this example we use a SoftwareSerial port for the ESP8266.

 NOTE: 
 There are some errors when using SoftwareSerial port with 115200 baud rate (max baud). 
 Please manually setup your ESP8266 to work with a lower baud rate.
 In this example we use 9600 baud, to setup this execute command: 
 AT+UART_DEF=9600,8,1,0,0
 This will setup your ESP8266 at 9600 baud for default, just keep it in mind.
 You can always go back to the default baud of 1115200.

 modified on 29 Aug 2016
 by @argandas
 http://www.github.com/argandas/ESP8266
*/

#include <ESP8266.h>
#include <SoftwareSerial.h>

#define ESP8266_RESET_PIN  8 /* Arduino pin D8 */
#define ESP8266_ENABLE_PIN 9 /* Arduino pin D9 */

char* AP_SSID = "<YourWiFiNetwork>";
char* AP_PASS = "<YourWiFiPassword>";

SoftwareSerial mySerial(10, 11);

void setup()
{

  Serial.begin(9600);

  Serial.println("ESP8266 SoftwareSerial example");

  /* Setup ESP8266 serial port and pins */
  setupESP8266(mySerial, 9600, ESP8266_RESET_PIN, ESP8266_ENABLE_PIN);

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

  if(setWifiMode(ESP8266_MODE_STATION))
  {
    Serial.println("Setup as station OK!");
  }

  if(setConnMode(ESP8266_CONN_SINGLE))
  {
    Serial.println("Single connection setup OK!");
  }

  if(joinAP(AP_SSID, AP_PASS))
  {
    Serial.print("Connected to SSID: ");
    Serial.println(AP_SSID);
  }

  if(ping("www.google.com"))
  {
    Serial.println("Ping to Google OK!");
  }

}

void loop()
{
  /* This loop let you enter AT commands manually to the ESP8266 */
  while(Serial.available())
  {
    mySerial.write(Serial.read());
  }
  while(mySerial.available())
  {
    Serial.write(mySerial.read());
  }
}
