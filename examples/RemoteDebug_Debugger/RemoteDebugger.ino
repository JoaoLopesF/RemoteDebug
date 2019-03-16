////////
// Library: Remote debug - debug over WiFi - for Esp8266 (NodeMCU) or ESP32
// Author : Joao Lopes
// File   : RemoteDebugger.ino
// Notes  :
//
// Attention: This library is only for help development. Please not use this in production
//
// This example use the RemoteDebugger library: ao addon to Remotedebug with an simple software debug, based on SerialDebug library
// Attention: this library must be installed too
// Please open github repo to informations: //https://github.com/JoaoLopesF/RemoteDebugger
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

// Web server (uncomment this to need this)

//#define WEB_SERVER_ENABLED true

// Disable all debug ?
//
// Important to compile for prodution/release
// Disable all debug ? Good to release builds (production)
// as nothing of RemoteDebug is compiled, zero overhead :-)
// For it just uncomment the DEBUG_DISABLED
// On change it, if in another IDE than Arduino IDE, like Eclipse or VSCODE,
// please clean the project, before compile

//#define DEBUG_DISABLED true

////// Includes

#if defined ESP8266

// Includes of ESP8266

#include <ESP8266WiFi.h>

#ifdef USE_MDNS
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#endif

#ifdef WEB_SERVER_ENABLED
#include <ESP8266WebServer.h>
#endif

#elif defined ESP32

// Includes of ESP32

#include <WiFi.h>

#ifdef USE_MDNS
#include <DNSServer.h>
#include "ESPmDNS.h"
#endif

#ifdef WEB_SERVER_ENABLED
#include <WebServer.h>
#endif

#else

#error "Now RemoteDebug support only boards Espressif, as ESP8266 and ESP32"

#endif // ESP

// Arduino OTA

#ifdef USE_ARDUINO_OTA
#include <ArduinoOTA.h>
#endif

// HTTP Web server

#ifdef WEB_SERVER_ENABLED

#if defined ESP8266

ESP8266WebServer HTTPServer(80);

#elif defined ESP32

WebServer HTTPServer(80);

#endif

#endif // WEB_SERVER_ENABLED

// Remote debug over WiFi - not recommended for production/release, only for development

#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

#ifndef DEBUG_DISABLED // Only if debug is not disabled (for production/release)

// RemoteDebug addon library: RemoteDebugger, an Simple software debugger - based on SerialDebug Library

#include "RemoteDebugger.h"		//https://github.com/JoaoLopesF/RemoteDebugger

// Instance of RemoteDebug

RemoteDebug Debug;

#endif

// WiFi credentials
// Note: if commented, is used the smartConfig
// That allow to it in mobile app
// See more details in http://www.iotsharing.com/2017/05/how-to-use-smartconfig-on-esp32.html

//#define WIFI_SSID "..."  // your network SSID (name)
//#define WIFI_PASS "..."  // your network key

/////// Variables

// Time

uint32_t mTimeToSec = 0;

uint8_t mRunSeconds = 0;
uint8_t mRunMinutes = 0;
uint8_t mRunHours = 0;

// Globals for example of debugger

boolean mBoolean = false;
char mChar = 'X';
byte mByte = 'Y';
int mInt = 1;
unsigned int mUInt = 2;
long mLong = 3;
unsigned long mULong = 4;
float mFloat = 5.0f;
double mDouble = 6.0;

String mString = "This is a string";
String mStringLarge = "This is a large stringggggggggggggggggggggggggggggggggggggggggggggg";

char mCharArray[] = "This is a char array";
char mCharArrayLarge[] = "This is a large char arrayyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";

int mIntArray[5] = {1 ,2 ,3, 4, 5};

//const char mCharArrayConst[] = "This is const";

////// Setup

void setup() {

	// Initialize the Serial (use only in setup codes)

	Serial.begin(230400);

	// Buildin led

#ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
#endif

	// Connect WiFi

	connectWiFi();

	// Host name of WiFi

#ifdef ESP8266
	WiFi.hostname(HOST_NAME);
#endif

#ifdef USE_ARDUINO_OTA
	// Update over air (OTA)

	initializeOTA();
#endif

	// Register host name in mDNS

#if defined USE_MDNS && defined HOST_NAME

	if (MDNS.begin(HOST_NAME)) {
		Serial.print("* MDNS responder started. Hostname -> ");
		Serial.println(HOST_NAME);
	}

	// Register the services

#ifdef WEB_SERVER_ENABLED
	MDNS.addService("http", "tcp", 80);   // Web server
#endif

#ifndef DEBUG_DISABLED
	MDNS.addService("telnet", "tcp", 23);// Telnet server RemoteDebug
#endif

#endif // MDNS

	// HTTP web server

#ifdef WEB_SERVER_ENABLED
	 HTTPServer.on("/", handleRoot);

	 HTTPServer.onNotFound(handleNotFound);

	 HTTPServer.begin();

	 Serial.println("* HTTP server started");
#endif

#ifndef DEBUG_DISABLED // Only for development

	// Initialize RemoteDebug

	Debug.begin(HOST_NAME); // Initialize the WiFi server

	//Debug.setPassword("r3m0t0."); // Password on telnet connection ?

	Debug.setResetCmdEnabled(true); // Enable the reset command

	Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)

	Debug.showColors(true); // Colors

	// Debug.setSerialEnabled(true); // if you wants serial echo - only recommended if ESP is plugged in USB

	// Project commands

	String helpCmd = "bench1 - Benchmark 1\n";
	helpCmd.concat("bench2 - Benchmark 2");

	Debug.setHelpProjectsCmds(helpCmd);
	Debug.setCallBackProjectCmds(&processCmdRemoteDebug);

	// Init the simple software debugger, based on SerialDebug library

#ifndef DEBUG_DISABLE_DEBUGGER

	Debug.initDebugger(debugGetDebuggerEnabled, debugHandleDebugger, debugGetHelpDebugger, debugProcessCmdDebugger); // Set the callbacks

	debugInitDebugger(&Debug); // Init the debugger

    // Add Functions and global variables to RemoteDebuggger

    // Notes: descriptions is optionals

    // Add functions that can called from SerialDebug

    if (debugAddFunctionVoid("benchInt", &benchInt) >= 0) {
    	debugSetLastFunctionDescription("To run a benchmark of integers");
    }
    if (debugAddFunctionVoid("benchFloat", &benchFloat) >= 0) {
    	debugSetLastFunctionDescription("To run a benchmark of float");
    }
    if (debugAddFunctionVoid("benchGpio", &benchGpio) >= 0) {
    	debugSetLastFunctionDescription("To run a benchmark of Gpio operations");
    }
    if (debugAddFunctionVoid("benchAll", &benchAll) >= 0) {
    	debugSetLastFunctionDescription("To run all benchmarks");
    }

    if (debugAddFunctionStr("funcArgStr", &funcArgStr) >= 0) {
    	debugSetLastFunctionDescription("To run with String arg");
    }
    if (debugAddFunctionChar("funcArgChar", &funcArgChar) >= 0) {
    	debugSetLastFunctionDescription("To run with Character arg");
    }
    if (debugAddFunctionInt("funcArgInt", &funcArgInt) >= 0) {
    	debugSetLastFunctionDescription("To run with Integer arg");
    }

    // Add global variables that can showed/changed from SerialDebug
    // Note: Only global, if pass local for SerialDebug, can be dangerous

    if (debugAddGlobalUInt8_t("mRunSeconds", &mRunSeconds) >= 0) {
    	debugSetLastGlobalDescription("Seconds of run time");
    }
    if (debugAddGlobalUInt8_t("mRunMinutes", &mRunMinutes) >= 0) {
    	debugSetLastGlobalDescription("Minutes of run time");
    }
    if (debugAddGlobalUInt8_t("mRunHours", &mRunHours) >= 0) {
    	debugSetLastGlobalDescription("Hours of run time");
    }

    // Note: easy way, no descriptions ....

    debugAddGlobalBoolean("mBoolean", 	&mBoolean);
    debugAddGlobalChar("mChar", 		&mChar);
    debugAddGlobalByte("mByte", 		&mByte);
    debugAddGlobalInt("mInt", 			&mInt);
    debugAddGlobalUInt("mUInt", 		&mUInt);
    debugAddGlobalLong("mLong", 		&mLong);
    debugAddGlobalULong("mULong", 		&mULong);
    debugAddGlobalFloat("mFloat", 		&mFloat);
    debugAddGlobalDouble("mDouble", 	&mDouble);

    debugAddGlobalString("mString", 	&mString);

    // Note: For char arrays, not use the '&'

    debugAddGlobalCharArray("mCharArray", mCharArray);

    // Note, here inform to show only 20 characteres of this string or char array

    debugAddGlobalString("mStringLarge", &mStringLarge, 20);

    debugAddGlobalCharArray("mCharArrayLarge",
    									mCharArrayLarge, 20);

    // For arrays, need add for each item (not use loop for it, due the name can not by a variable)
    // Notes: Is good added arrays in last order, to help see another variables
    //        In next versions, we can have a helper to do it in one command

	debugAddGlobalInt("mIntArray[0]", 	&mIntArray[0]);
	debugAddGlobalInt("mIntArray[1]", 	&mIntArray[1]);
	debugAddGlobalInt("mIntArray[2]", 	&mIntArray[2]);
	debugAddGlobalInt("mIntArray[3]",	&mIntArray[3]);
	debugAddGlobalInt("mIntArray[4]",	&mIntArray[4]);

    // Add watches for some global variables
    // Note: watches can be added/changed in serial monitor too

	// Watch -> mBoolean when changed (put 0 on value)

	debugAddWatchBoolean("mBoolean", DEBUG_WATCH_CHANGED, 0);

	// Watch -> mRunSeconds == 10

	debugAddWatchUInt8_t("mRunSeconds", DEBUG_WATCH_EQUAL, 10);

	// Watch -> mRunMinutes > 3

	debugAddWatchUInt8_t("mRunMinutes", DEBUG_WATCH_GREAT, 3);

	// Watch -> mRunMinutes == mRunSeconds (just for test)

	debugAddWatchCross("mRunMinutes", DEBUG_WATCH_EQUAL, "mRunSeconds");

#endif

	// End of setup - show IP

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

#endif

}

void loop() {

#ifndef DEBUG_DISABLED
	// Time of begin of this loop
	uint32_t timeBeginLoop = millis();
#endif

	// Each second

	if (millis() >= mTimeToSec) {

		// Time

		mTimeToSec = millis() + 1000;

		// Count run time (just a test - for real suggest the TimeLib and NTP, if board have WiFi)

		mRunSeconds++;

		if (mRunSeconds == 60) {
			mRunMinutes++;
			mRunSeconds = 0;
		}
		if (mRunMinutes == 60) {
			mRunHours++;
			mRunMinutes = 0;
		}
		if (mRunHours == 24) {
			mRunHours = 0;
		}

		// Blink the led

#ifdef LED_BUILTIN
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
#endif

		// Debug the time (verbose level)

		debugV("* Time: %u seconds (VERBOSE)", mRunSeconds);

		if (mRunSeconds % 5 == 0) { // Each 5 seconds

			// Debug levels

			debugV("* This is a message of debug level VERBOSE");
			debugD("* This is a message of debug level DEBUG");
			debugI("* This is a message of debug level INFO");
			debugW("* This is a message of debug level WARNING");
			debugE("* This is a message of debug level ERROR");


			// RemoteDebug isActive? Use this RemoteDebug sintaxe if you need process anything only for debug
			// It is good to avoid overheads (this is only use that is suggest to use isActive)
			// Note this need be surrounded by DEBUG_DISABLED precompiler condition to not compile for production/release

#ifndef DEBUG_DISABLED

			if (Debug.isActive(Debug.VERBOSE)) {

				Debug.println("Calling a foo function");
				Debug.printf("At time of %d sec.\n", mRunSeconds);

				// Call a function

				foo();
			}
#endif

		}
	}

	////// Services on Wifi

#ifdef USE_ARDUINO_OTA
	// Update over air (OTA)

	ArduinoOTA.handle();
#endif

#ifdef WEB_SERVER_ENABLED
	// Web server

	HTTPServer.handleClient();
#endif

#ifndef DEBUG_DISABLED
	// Remote debug over telnet

	Debug.handle();
#endif

	// Give a time for ESP

	yield();

#ifndef DEBUG_DISABLED
	// Show a debug - warning if time of these loop is over 50 (info) or 100 ms (warning)

	uint32_t time = (millis() - timeBeginLoop);

	if (time > 100) {
		debugI("* Time elapsed for the loop: %u ms.", time);
	} else if (time > 200) {
		debugW("* Time elapsed for the loop: %u ms.", time);
	}
#endif

}


// Function example to show a new auto function name of debug* macros

void foo() {

  uint8_t var = 1;

  debugV("this is a debug - var %u", var);
  debugV("This is a println");
}

#ifndef DEBUG_DISABLED

// Process commands from RemoteDebug

void processCmdRemoteDebug() {

	String lastCmd = Debug.getLastCommand();

	if (lastCmd == "bench1") {

		// Benchmark 1 - Printf

		debugA("* Benchmark 1 - one Printf");


		uint32_t timeBegin = millis();
		uint8_t times = 50;

		for (uint8_t i = 1; i <= times; i++) {
			debugA("%u - 1234567890 - AAAA", i);

		}

		debugA("* Time elapsed for %u printf: %ld ms.\n", times,
					(millis() - timeBegin));

	} else if (lastCmd == "bench2") {

		// Benchmark 2 - Print/println

		debugA("* Benchmark 2 - Print/Println");

		uint32_t timeBegin = millis();
		uint8_t times = 50;

		for (uint8_t i = 1; i <= times; i++) {
			if (Debug.isActive(Debug.ANY)) {
				Debug.print(i);
				Debug.print(" - 1234567890");
				Debug.println(" - AAAA");
			}
		}

		debugA("* Time elapsed for %u printf: %ld ms.\n", times,
					(millis() - timeBegin));
	}
}
#endif

////// Benchmarks - simple

// Note: how it as called by SerialDebug, must be return type void and no args
// Note: Flash F variables is not used during the tests, due it is slow to use in loops

#define BENCHMARK_EXECS 10000

// Simple benckmark of integers

void benchInt() {

	int test = 0;

	for (int i = 0; i < BENCHMARK_EXECS; i++) {

		// Some integer operations

		test++;
		test += 2;
		test -= 2;
		test *= 2;
		test /= 2;
	}

	// Note: Debug always is used here

	debugA("*** Benchmark of integers. %u exec.", BENCHMARK_EXECS);

}

// Simple benckmark of floats

void benchFloat() {

	float test = 0;

	for (int i = 0; i < BENCHMARK_EXECS; i++) {

		// Some float operations

		test++;
		test += 2;
		test -= 2;
		test *= 2;
		test /= 2;
	}

	// Note: Debug always is used here

	debugA("*** Benchmark of floats, %u exec.", BENCHMARK_EXECS);

}

// Simple benckmark of GPIO

void benchGpio() {

//	const int execs = (BENCHMARK_EXECS / 10); // Reduce it
	const int execs = BENCHMARK_EXECS;

	for (int i = 0; i < execs; i++) {

		// Some GPIO operations

		digitalWrite(LED_BUILTIN, HIGH);
		digitalRead(LED_BUILTIN);
		digitalWrite(LED_BUILTIN, LOW);

		analogRead(A0);
		analogRead(A0);
		analogRead(A0);

	}

	// Note: Debug always is used here

	debugA("*** Benchmark of GPIO. %u exec.", execs);

}

// Run all benchmarks

void benchAll() {

	benchInt();
	benchFloat();
	benchGpio();

	// Note: Debug always is used here

	debugA("*** All Benchmark done.");

}

// Example functions with argument (only 1) to call from serial monitor
// Note others types is not yet available in this version of SerialDebug

void funcArgStr (String str) {

	debugA("*** called with arg.: %s", str.c_str());
}
void funcArgChar (char character) {

	debugA("*** called with arg.: %c", character);
}
void funcArgInt (int number) {

	debugA("*** called with arg.: %d", number);
}

////// WiFi

void connectWiFi() {

	////// Connect WiFi

#ifdef EM_DEPURACAO
	Serial.println("*** connectWiFi: begin conection ...");
#endif

#ifdef ESP32
	// ESP32 // TODO: is really necessary ?
	WiFi.enableSTA(true);
	delay(100);
#endif

	// Connect with SSID and password stored

#ifndef WIFI_SSID
	WiFi.begin();
#else
	WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif

	// Wait connection

	uint32_t timeout = millis() + 20000; // Time out

	while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
		delay(250);
		Serial.print(".");
	}

	// Not connected yet?

	if (WiFi.status() != WL_CONNECTED) {

#ifndef WIFI_SSID
		// SmartConfig

		WiFi.beginSmartConfig();

		// Wait for SmartConfig packet from mobile

		Serial.println("connectWiFi: Waiting for SmartConfig.");

		while (!WiFi.smartConfigDone()) {
			delay(500);
			Serial.print(".");
		}

		Serial.println("");
		Serial.println("connectWiFi: SmartConfig received.");

		// Wait for WiFi to connect to AP

		Serial.println("connectWiFi: Waiting for WiFi");

		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			Serial.print(".");
		}
#else
		Serial.println("Not possible connect to WiFi, rebooting");
		ESP.restart();
#endif
	}

	// End

	Serial.println("");
	Serial.print("connectWiFi: connect a ");
	Serial.println(WiFi.SSID());
	Serial.print("IP: ");
	Serial.println(WiFi.localIP().toString());

}

#ifdef USE_ARDUINO_OTA

// Initialize o Arduino OTA

void initializeOTA() {

	// TODO: option to authentication (password)

#if defined ESP8266

	ArduinoOTA.onStart([]() {
		Serial.println("* OTA: Start");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\n*OTA: End");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("*OTA: Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("*OTA: Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});

#elif defined ESP32

	// ArduinoOTA

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";
			Serial.println("Start updating " + type);
		}).onEnd([]() {
		Serial.println("\nEnd");
	}).onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	}).onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});

#endif

	// Begin

	ArduinoOTA.begin();

}

#endif

#ifdef WEB_SERVER_ENABLED

/////////// Handles

 void handleRoot() {

     // Root web page

     HTTPServer.send(200, "text/plain", "hello from esp - RemoteDebug Sample!");
 }

 void handleNotFound(){

     // Page not Found

     String message = "File Not Found\n\n";
     message.concat("URI: ");
     message.concat(HTTPServer.uri());
     message.concat("\nMethod: ");
     message.concat((HTTPServer.method() == HTTP_GET)?"GET":"POST");
     message.concat("\nArguments: ");
     message.concat(HTTPServer.args());
     message.concat("\n");
     for (uint8_t i=0; i<HTTPServer.args(); i++){
         message.concat(" " + HTTPServer.argName(i) + ": " + HTTPServer.arg(i) + "\n");
     }
     HTTPServer.send(404, "text/plain", message);
 }

#endif

/////////// End
