/*
 SoftwareSerial.pde
 This example only test the communication with the ESP8266.

 NOTE: 
 There are some errors when using SoftwareSerial port with 115200 baud rate (max baud). 
 Please manually setup your ESP8266 to work with a lower baud rate.
 In this example we use 9600 baud, to setup this execute command: 
 AT+UART_DEF=115200,8,1,0,0
 This will setup your ESP8266 at 9600 baud for default, just keep it in mind.
 You can always go back to the default baud of 1115200.

 modified on 29 Aug 2016
 by @argandas
 http://www.github.com/argandas/ESP8266
*/

#include <ESP8266.h>

#define ESP8266_RESET_PIN  4 /* Arduino pin D4 */
#define ESP8266_ENABLE_PIN 3 /* Arduino pin D3 */

void setup()
{
  /* Setup ESP8266 serial port and pins */
  setupESP8266(Serial1, 115200, ESP8266_RESET_PIN, ESP8266_ENABLE_PIN);

  if(!hardReset())
  {
    if(reset())
    {
      Serial.println("ESP8266 reset OK");
    }
  }
}

void loop()
{
  if(test())
  {
    Serial.println("ESP8266 is alive!");
    delay(1000);
  }
}
