/* 
  Light sensor with http client
  Written for ESP8266 ESP-12, and Adafruit TSL2561 sensor

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC
   Connect GROUND to common ground

   I2C Address
   ===========
   The address will be different depending on whether you leave
   the ADDR pin floating (addr 0x39), or tie it to ground or vcc. 
   The default addess is 0x39, which assumes the ADDR pin is floating
   (not connected to anything).  If you set the ADDR pin high
   or low, use TSL2561_ADDR_HIGH (0x49) or TSL2561_ADDR_LOW
   (0x29) respectively. 

   As of 1/1/2016, the reference to "pgmspace.h" in Adafruit_TSL2561_U.h
   needs to be changed to <pgmspace.h>
*/
// Import required libraries
// #include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <pgmspace.h>
#include <Adafruit_TSL2561_U.h>
#include <ArduinoOTA.h>

#define USE_SERIAL Serial
#define delay_seconds(_val)   delay(1000L *(_val))

ESP8266WiFiMulti WiFiMulti;
const char* ssid = "your_ssid";
const char* password = "your_password";
const char* zwhome = "http://IP.Address.Of.zwhome:8000/sensors/";
const char* thisnode = "front.sensor";
const char* sensortype = "luminance";
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 1);

int luminance;
int voltage;
unsigned long dcnt = 10; // Initially 10 seconds
char smsg[100];

ADC_MODE(ADC_VCC);

void configureSensor(void) {
  tsl.enableAutoRange(true); /* Auto-gain ... switches automatically between 1x and 16x */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS); /* 16-bit data but slowest conversions */
}

void setup() {
    USE_SERIAL.begin(115200);
//  USE_SERIAL.setDebugOutput(true);
    USE_SERIAL.println();
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    /* Initialise the sensor */
    if(!tsl.begin()) {
        USE_SERIAL.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
        while(1);
    }
    /* Setup the sensor gain and integration time */
    configureSensor();
    WiFiMulti.addAP(ssid, password);
  ArduinoOTA.onStart([]() {
    String type;
    USE_SERIAL.println("\nStart");
  });
  ArduinoOTA.onEnd([]() {
    USE_SERIAL.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    USE_SERIAL.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    USE_SERIAL.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) USE_SERIAL.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) USE_SERIAL.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) USE_SERIAL.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) USE_SERIAL.println("Receive Failed");
    else if (error == OTA_END_ERROR) USE_SERIAL.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {
        ArduinoOTA.handle();
        /* Get a new sensor event */ 
        sensors_event_t event;
        tsl.getEvent(&event);
        delay(1);
        /* Display the results (light is measured in lux) */
        if (event.light)
        {
            luminance = event.light;
//          USE_SERIAL.print(event.light); USE_SERIAL.println(" lux");
        }
        voltage = ESP.getVcc();
//      USE_SERIAL.print(voltage); USE_SERIAL.println(" volts");

        HTTPClient http;
        snprintf(smsg, sizeof(smsg)-1, "%s%s?param=%s&value=%d", zwhome, thisnode, sensortype, luminance);
        http.begin(smsg);
//        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been sent and Server response header has been handled
//            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
//              USE_SERIAL.println(payload);
// format:  {"sleep":540}
                int startstr = payload.indexOf(String("sleep"));
                if(startstr > -1) {
                  startstr = payload.indexOf(':', startstr);
                  if(startstr > -1) {
                    startstr = startstr + 1; // position at first digit
                    int endstr = startstr;
                    while(isDigit(payload.charAt(endstr))) endstr++;
                    dcnt = (unsigned long) payload.substring(startstr, endstr).toInt();
//                    USE_SERIAL.print("Sleep for ");
//                    USE_SERIAL.print(payload.substring(startstr, endstr).toInt());
//                    USE_SERIAL.println(" seconds");
                  }
                }
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }

    delay_seconds(dcnt);
}

