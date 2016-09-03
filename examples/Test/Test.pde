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
#include <SoftwareSerial.h>

/* Setup serial port for ESP8266 */
extern SoftwareSerial mySerial(10, 11);

/* Setup ESP8266 control pins */
ESP8266 myESP(13, 12); /* RESET, ENABLE*/
	
void setup()
{
  Serial.begin(9600);
  Serial.println("ESP8266 test example");
  
  myESP.begin(mySerial, 9600);
		
  if(!myESP.hardReset())
  {
    Serial.println("Trying soft reset");
    if(myESP.reset())
    {
      Serial.println("ESP8266 reset OK");
    }
  }  
  
}

void loop()
{
  if(myESP.test())
  {
    Serial.println("ESP8266 is alive!");
    delay(1000);
  }
}
