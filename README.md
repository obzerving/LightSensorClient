LightSensorClient

A wifi-enabled Light sensor for the zwhome Home Automation System. Uses the same hardware as the OTALightSensor, but differs from it in that this sensor reports in at defined time intervals rather than providing a restful web service.

Hardware Used:
- ESP8266-12 wifi module
- Adafruit TSL2561 sensor

Software Used (Note: Versions may have changed)
- Arduino development environment for ESP8266 (see https://github.com/esp8266/Arduino)
- Adafruit TSL2561 Driver v2.0 (Unified Sensor Driver) (see https://learn.adafruit.com/tsl2561/downloads)


Work Flow

1. Install the development environment

2. Install the TSL2561 libraries and the aREST library

3. Change the LightSensorClient.ino file accordingly. I strongly suggest these:

   a. "your_ssid" to your wifi ssid

   b. "your_password" to your wifi password
   
   c. "http://IP.Address.Of.zwhome:8000/sensors/" to your zwhome server's address

   d. "front.sensor" to a name (following the zwhome naming convention) that makes sense for you sensor location

4. Wire up the ESP8266 module and the sensor. The components I used require 3.3 volts and do not tolerate 5 volts. It's possible to find modules that can run safely at voltages higher than 3.3 volts, I didn't use them. Be careful.

5. Program the ESP8266 module. I highly recommend not having the light sensor module connected during programming. My components are mounted in sockets, so I can do that easily. Further, socket mounting these components is a good practice.

6. Reset the ESP8266 and it should run the program. Initially, it will send an HTTP GET request every ten seconds to the zwhome server. For example:

http://IP.Address.Of.zwhome:8000/sensors/front.sensor?param=luminance&value=2000

where 2000 is the lux reading from the sensor. The zwhome server will reply back in JSON format with a time interval (in seconds) that it wants to receive future reports from the sensor. For example:

{"sleep":60}

resulting in the sensor sending future reports every minute.