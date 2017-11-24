////////
// Libraries Arduino
//
// Library: Remote debug - debug over telnet - for Esp8266 (NodeMCU) or ESP32
// Author: Joao Lopes
//
// Attention: This library is only for help development. Please not use this in production
//
// First sample to show how to use it - basic one
//
// Example of use:
//
//        if (Debug.isActive(Debug.<level>)) { // <--- This is very important to reduce overheads and work of debug levels
//            Debug.printf("bla bla bla: %d %s\n", number, str);
//            Debug.println("bla bla bla");
//        }
//
// Or shortcuts:
//
//		DEBUG("This is a any (always showed) - var %d\n", var);
//
//		DEBUG_V("This is a verbose - var %d\n", var);
//		DEBUG_D("This is a debug - var %d\n", var);
//		DEBUG_I("This is a information - var %d\n", var);
//		DEBUG_W("This is a warning - var %d\n", var);
//		DEBUG_E("This is a error - var %d\n", var);
//
//		// Note: if you want a simple println you must ended with new line characters
//
//		DEBUG_V("This println\n");
//
///////

// Libraries

#if defined (ESP8266)

#define USE_MDNS true // Use the MDNS ?

// Includes do ESP8266

#include <ESP8266WiFi.h>

#ifdef USE_MDNS
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#endif

#elif defined(ESP32)

//#define USE_MDNS true // Use the MDNS ? //TODO: not tested in Esp32 yet

// Includes do ESP32

#include <WiFi.h>

#ifdef USE_MDNS
#include "ESPmDNS.h"
#endif

#else

#error The board must be ESP8266 or ESP32

#endif // ESP

// Remote debug over telnet - not recommended for production, only for development

#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

RemoteDebug Debug;

// SSID and password

const char* ssid = "........";
const char* password = "........";

// Host mDNS

#define HOST_NAME "remotedebug-sample"

// Time

uint32_t mLastTime = 0;
uint32_t mTimeSeconds = 0;

// Buildin Led ON ?

boolean mLedON = false;

////// Setup

void setup() {

    // Initialize the Serial (educattional use only, not need in production)

    Serial.begin(115200);

    // Buildin led off ESP8266

    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW);

#ifdef ESP32
    // ESP32 configuration // TODO: see if it is necessary
  WiFi.enableSTA(true);
  delay(1000);
#endif

    // WiFi connection

    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Register host name in WiFi and mDNS

    String hostNameWifi = HOST_NAME;
    hostNameWifi.concat(".local");

#ifdef ESP8266 // Only for it
    WiFi.hostname(hostNameWifi);
#endif

#ifdef USE_MDNS  // Use the MDNS ?

    if (MDNS.begin(HOST_NAME)) {
        Serial.print("* MDNS responder started. Hostname -> ");
        Serial.println(HOST_NAME);
    }

    MDNS.addService("telnet", "tcp", 23);

#endif

    // Initialize the telnet server of RemoteDebug

    Debug.begin(HOST_NAME); // Initiaze the telnet server

    Debug.setResetCmdEnabled(true); // Enable the reset command

    //Debug.showTime(true); // To show time

    // Debug.showProfiler(true); // To show profiler - time between messages of Debug
                              // Good to "begin ...." and "end ...." messages

    // This sample (serial -> educattional use only, not need in production)

    Serial.println("* Arduino RemoteDebug Library");
    Serial.println("*");
    Serial.print("* WiFI connected. IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("*");
    Serial.println("* Please use the telnet client (telnet for Mac/Unix or putty and others for Windows)");
    Serial.println("*");
    Serial.println("* This sample will send messages of debug in all levels.");
    Serial.println("*");
    Serial.println("* Please try change debug level in telnet, to see how it works");
    Serial.println("*");

}

void loop()
{
    // Each second

    if ((millis() - mLastTime) >= 1000) {

        // Time

        mLastTime = millis();

        mTimeSeconds++;

        // Blink the led

        mLedON = !mLedON;
        digitalWrite(BUILTIN_LED, (mLedON)?LOW:HIGH);

        // Debug the time (verbose level)

       DEBUG_V("* Time: %u seconds (VERBOSE)\n", mTimeSeconds);

        if (mTimeSeconds % 5 == 0) { // Each 5 seconds

            // Debug levels

			DEBUG_V("* This is a message of debug level VERBOSE\n");
			DEBUG_D("* This is a message of debug level DEBUG\n");
			DEBUG_I("* This is a message of debug level INFO\n");
			DEBUG_W("* This is a message of debug level WARNING\n");
			DEBUG_E("* This is a message of debug level ERROR\n");
        }
     }

    // Remote debug over telnet

    Debug.handle();

    // Give a time for ESP8266

    yield();

}

/////////// End
