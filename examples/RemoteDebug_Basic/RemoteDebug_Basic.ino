////////
// Library: Remote debug - debug over WiFi - for Esp8266 (NodeMCU) or ESP32
// Author : Joao Lopes
// File   : RemoteDebug_Basic.ino
// Notes  :
//
// Attention: This library is only for help development. Please not use this in production
//
// First sample to show how to use it - basic one
//
// Example of use:
//
//#ifndef DEBUG_DISABLED
//        if (Debug.isActive(Debug.<level>)) { // <--- This is very important to reduce overheads and work of debug levels
//            Debug.printf("bla bla bla: %d %s\n", number, str);
//            Debug.println("bla bla bla");
//        }
//#endif
//
// Or short way (prefered if only one debug at time)
//
//		debugA("This is a any (always showed) - var %d", var);
//
//		debugV("This is a verbose - var %d", var);
//		debugD("This is a debug - var %d", var);
//		debugI("This is a information - var %d", var);
//		debugW("This is a warning - var %d", var);
//		debugE("This is a error - var %d", var);
//
//		debugV("This is println");
//
//
///////

////// Defines

// Host name (please change it)

#define HOST_NAME "remotedebug"

// Board especific libraries

#if defined ESP8266 || defined ESP32

// Use mDNS ? (comment this do disable it)

#define USE_MDNS true

// Arduino OTA (uncomment this to enable)

//#define USE_ARDUINO_OTA true

#else

// RemoteDebug library is now only to Espressif boards,
// as ESP32 and ESP82266,
// If need for another WiFi boards,
// please add an issue about this
// and we will see if it is possible made the port for your board.
// access: https://github.com/JoaoLopesF/RemoteDebug/issues

#error "The board must be ESP8266 or ESP32"

#endif // ESP

//////// Libraries

#if defined ESP8266

// Includes of ESP8266

#include <ESP8266WiFi.h>

#ifdef USE_MDNS
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#endif

#elif defined ESP32

// Includes of ESP32

#include <WiFi.h>

#ifdef USE_MDNS
#include <DNSServer.h>
#include "ESPmDNS.h"
#endif

#endif // ESP

// Remote debug over WiFi - not recommended for production, only for development

#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

RemoteDebug Debug;

// SSID and password

const char* ssid = "........";
const char* password = "........";

// Time

uint32_t mLastTime = 0;
uint32_t mTimeSeconds = 0;

////// Setup

void setup() {

	// Initialize the Serial (use only in setup codes)

    Serial.begin(230400);

    // Buildin led of ESP

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

  	// Debug

    Serial.println("**** Setup: initializing ...");

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

	// Initialize RemoteDebug

	Debug.begin(HOST_NAME); // Initialize the WiFi server

    Debug.setResetCmdEnabled(true); // Enable the reset command

	Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
	Debug.showColors(true); // Colors

    // End off setup

    Serial.println("* Arduino RemoteDebug Library");
    Serial.println("*");
    Serial.print("* WiFI connected. IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("*");
    Serial.println("* Please use the telnet client (telnet for Mac/Unix or putty and others for Windows)");
	Serial.println("* or the RemoteDebugApp (in browser: http://joaolopesf.net/remotedebugapp)");
   Serial.println("*");
    Serial.println("* This sample will send messages of debug in all levels.");
    Serial.println("*");
	Serial.println("* Please try change debug level in client (telnet or web app), to see how it works");
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

        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

        // Debug the time (verbose level)

        debugV("* Time: %u seconds (VERBOSE)", mTimeSeconds);

        if (mTimeSeconds % 5 == 0) { // Each 5 seconds

            // Debug levels

			debugV("* This is a message of debug level VERBOSE");
			debugD("* This is a message of debug level DEBUG");
			debugI("* This is a message of debug level INFO");
			debugW("* This is a message of debug level WARNING");
			debugE("* This is a message of debug level ERROR");

			// Call a function

			foo();
        }
     }

    // RemoteDebug handle

    Debug.handle();

    // Give a time for ESP

    yield();

}

// Function example to show a new auto function name of debug* macros

void foo() {

  uint8_t var = 1;

  debugV("this is a debug - var %u", var);
  debugV("This is a println");
}

/////////// End
