#include <ESP8266.h>

#define ESP8266_RESET_PIN  4 /* Arduino pin D4 */
#define ESP8266_ENABLE_PIN 3 /* Arduino pin D3 */

const char AP_SSID[] = "<YourWiFiNetwork>";
const char AP_PASS[] = "<YourWiFiPassword>";

void setup()
{
  /* Setup ESP8266 serial port and pins */
  setupESP8266(Serial1, 115200, ESP8266_RESET_PIN, ESP8266_ENABLE_PIN);

  if(reset())
  {
    Serial.println("ESP8266 reset OK");
    if(ping())
    {
      Serial.println("ESP8266 is alive!");
      setWifiMode(1);
      setConnMode(1);
      joinAP(AP_SSID, AP_PASS);
      startTCP();
    }
  }



}

void loop()
{
}
