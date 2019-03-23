/*
 * Header for RemoteDebug
 *
 * MIT License
 *
 * Copyright (c) 2019 Joao Lopes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 */

#ifndef REMOTEDEBUG_H
#define REMOTEDEBUG_H
#pragma once

///// RemoteDebug configuration

#include "RemoteDebugCfg.h"

// Debug enabled ?

#ifndef DEBUG_DISABLED

//////// Defines

// New color system (comment this to return to old system) - 2019-02-27

#define COLOR_NEW_SYSTEM true

// ANSI Colors

#define COLOR_RESET "\x1B[0m"

#define COLOR_BLACK "\x1B[0;30m"
#define COLOR_RED "\x1B[0;31m"
#define COLOR_GREEN "\x1B[0;32m"
#define COLOR_YELLOW "\x1B[0;33m"
#define COLOR_BLUE "\x1B[0;34m"
#define COLOR_MAGENTA "\x1B[0;35m"
#define COLOR_CYAN "\x1B[0;36m"
#define COLOR_WHITE "\x1B[0;37m"

#define COLOR_DARK_BLACK "\x1B[1;30m"

#define COLOR_LIGHT_RED "\x1B[1;31m"
#define COLOR_LIGHT_GREEN "\x1B[1;32m"
#define COLOR_LIGHT_YELLOW "\x1B[1;33m"
#define COLOR_LIGHT_BLUE "\x1B[1;34m"
#define COLOR_LIGHT_MAGENTA "\x1B[1;35m"
#define COLOR_LIGHT_CYAN "\x1B[1;36m"
#define COLOR_LIGHT_WHITE "\x1B[1;37m"

#define COLOR_BACKGROUND_BLACK "\x1B[40m"
#define COLOR_BACKGROUND_RED "\x1B[41m"
#define COLOR_BACKGROUND_GREEN "\x1B[42m"
#define COLOR_BACKGROUND_YELLOW "\x1B[43m"
#define COLOR_BACKGROUND_BLUE "\x1B[44m"
#define COLOR_BACKGROUND_MAGENTA "\x1B[45m"
#define COLOR_BACKGROUND_CYAN "\x1B[46m"
#define COLOR_BACKGROUND_WHITE "\x1B[47m"

#ifdef COLOR_NEW_SYSTEM
// New system of Colors
// Note: this colors is not equals to SerialDebug colors, due using standard 16 colors of Ansi, for compatibility
#define COLOR_VERBOSE 	COLOR_GREEN
#define COLOR_DEBUG		COLOR_LIGHT_GREEN
//#define COLOR_INFO		COLOR_YELLOW
//#define COLOR_WARNING	COLOR_CYAN
//#define COLOR_ERROR		COLOR_RED
#define COLOR_INFO		COLOR_LIGHT_YELLOW
#define COLOR_WARNING	COLOR_LIGHT_CYAN
#define COLOR_ERROR		COLOR_LIGHT_RED
#define COLOR_RAW		COLOR_WHITE // COLOR_MAGENTA
#endif

//////// Includes

#include "Arduino.h"
#include "Print.h"

// ESP8266 or ESP32 ?

#if defined(ESP8266)

#include <ESP8266WiFi.h>

#elif defined(ESP32)

#include <WiFi.h>

#else

#error "Only for ESP8266 or ESP32"

#endif

////// Shortcuts macros

// Auto function for debug macros?

#ifndef DEBUG_DISABLE_AUTO_FUNC // With auto func

	#ifdef ESP32

		// ESP32 -> Multicore  - show core id ?

		#define DEBUG_AUTO_CORE true   // debug show core id ? (comment to disable it)

		#ifdef DEBUG_AUTO_CORE

			#define rdebugA(fmt, ...) if (Debug.isActive(Debug.ANY)) 		Debug.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
			#define rdebugP(fmt, ...) if (Debug.isActive(Debug.PROFILER))	Debug.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
			#define rdebugV(fmt, ...) if (Debug.isActive(Debug.VERBOSE)) 	Debug.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
			#define rdebugD(fmt, ...) if (Debug.isActive(Debug.DEBUG)) 		Debug.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
			#define rdebugI(fmt, ...) if (Debug.isActive(Debug.INFO)) 		Debug.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
			#define rdebugW(fmt, ...) if (Debug.isActive(Debug.WARNING)) 	Debug.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)
			#define rdebugE(fmt, ...) if (Debug.isActive(Debug.ERROR)) 		Debug.printf("(%s)(C%d) " fmt, __func__, xPortGetCoreID(), ##__VA_ARGS__)

		#endif

	#endif

	#ifndef DEBUG_AUTO_CORE // No auto core or for ESP8266

		#define rdebugA(fmt, ...) if (Debug.isActive(Debug.ANY)) 		Debug.printf("(%s) " fmt, __func__, ##__VA_ARGS__)
		#define rdebugP(fmt, ...) if (Debug.isActive(Debug.PROFILER)) 	Debug.printf("(%s) " fmt, __func__, ##__VA_ARGS__)
		#define rdebugV(fmt, ...) if (Debug.isActive(Debug.VERBOSE)) 	Debug.printf("(%s) " fmt, __func__, ##__VA_ARGS__)
		#define rdebugD(fmt, ...) if (Debug.isActive(Debug.DEBUG)) 		Debug.printf("(%s) " fmt, __func__, ##__VA_ARGS__)
		#define rdebugI(fmt, ...) if (Debug.isActive(Debug.INFO)) 		Debug.printf("(%s) " fmt, __func__, ##__VA_ARGS__)
		#define rdebugW(fmt, ...) if (Debug.isActive(Debug.WARNING)) 	Debug.printf("(%s) " fmt, __func__, ##__VA_ARGS__)
		#define rdebugE(fmt, ...) if (Debug.isActive(Debug.ERROR)) 		Debug.printf("(%s) " fmt, __func__, ##__VA_ARGS__)

	#endif

#else // Without auto func

	#define rdebugA(fmt, ...) if (Debug.isActive(Debug.ANY)) 		Debug.printf(fmt, ##__VA_ARGS__)
	#define rdebugP(fmt, ...) if (Debug.isActive(Debug.PROFILER)) 	Debug.printf(fmt, ##__VA_ARGS__)
	#define rdebugV(fmt, ...) if (Debug.isActive(Debug.VERBOSE)) 	Debug.printf(fmt, ##__VA_ARGS__)
	#define rdebugD(fmt, ...) if (Debug.isActive(Debug.DEBUG)) 		Debug.printf(fmt, ##__VA_ARGS__)
	#define rdebugI(fmt, ...) if (Debug.isActive(Debug.INFO)) 		Debug.printf(fmt, ##__VA_ARGS__)
	#define rdebugW(fmt, ...) if (Debug.isActive(Debug.WARNING)) 	Debug.printf(fmt, ##__VA_ARGS__)
	#define rdebugE(fmt, ...) if (Debug.isActive(Debug.ERROR)) 		Debug.printf(fmt, ##__VA_ARGS__)

#endif

// With newline

#define rdebugAln(fmt, ...) rdebugA(fmt "\n", ##__VA_ARGS__)
#define rdebugPln(fmt, ...) rdebugP(fmt "\n", ##__VA_ARGS__)
#define rdebugVln(fmt, ...) rdebugV(fmt "\n", ##__VA_ARGS__)
#define rdebugDln(fmt, ...) rdebugD(fmt "\n", ##__VA_ARGS__)
#define rdebugIln(fmt, ...) rdebugI(fmt "\n", ##__VA_ARGS__)
#define rdebugWln(fmt, ...) rdebugW(fmt "\n", ##__VA_ARGS__)
#define rdebugEln(fmt, ...) rdebugE(fmt "\n", ##__VA_ARGS__)

// For old versions compatibility

#define rdebug(fmt, ...) 	rdebugA(fmt, ##__VA_ARGS__)

// Another way - for compatibility

#define DEBUG(fmt, ...)   	rdebugA(fmt, ##__VA_ARGS__)

#define DEBUG_A(fmt, ...) 	rdebugA(fmt, ##__VA_ARGS__)
#define DEBUG_P(fmt, ...) 	rdebugP(fmt, ##__VA_ARGS__)
#define DEBUG_V(fmt, ...) 	rdebugV(fmt, ##__VA_ARGS__)
#define DEBUG_D(fmt, ...) 	rdebugD(fmt, ##__VA_ARGS__)
#define DEBUG_I(fmt, ...) 	rdebugI(fmt, ##__VA_ARGS__)
#define DEBUG_W(fmt, ...) 	rdebugW(fmt, ##__VA_ARGS__)
#define DEBUG_E(fmt, ...) 	rdebugE(fmt, ##__VA_ARGS__)

// New way: To compatibility with SerialDebug (can use RemoteDebug or SerialDebug)
// This is my favorite :)

#define debugV(fmt, ...) rdebugVln(fmt, ##__VA_ARGS__)
#define debugD(fmt, ...) rdebugDln(fmt, ##__VA_ARGS__)
#define debugI(fmt, ...) rdebugIln(fmt, ##__VA_ARGS__)
#define debugW(fmt, ...) rdebugWln(fmt, ##__VA_ARGS__)
#define debugE(fmt, ...) rdebugEln(fmt, ##__VA_ARGS__)
#define debugA(fmt, ...) rdebugAln(fmt, ##__VA_ARGS__)

#define debugHandle() Debug.handle()

// Macros used by code converter for codes with several prints to only message
// due the converter cannot
// convert severals Serial.print in one debug* macro.

#define rprintV(x, ...)		if (Debug.isActive(Debug.VERBOSE)) 	Debug.print(x, ##__VA_ARGS__)
#define rprintD(x, ...)		if (Debug.isActive(Debug.DEBUG))	Debug.print(x, ##__VA_ARGS__)
#define rprintI(x, ...)		if (Debug.isActive(Debug.INFO)) 	Debug.print(x, ##__VA_ARGS__)
#define rprintW(x, ...)		if (Debug.isActive(Debug.WARNING)) 	Debug.print(x, ##__VA_ARGS__)
#define rprintE(x, ...)		if (Debug.isActive(Debug.ERROR)) 	Debug.print(x, ##__VA_ARGS__)
#define rprintA(x, ...)		if (Debug.isActive(Debug.ANY)) 		Debug.print(x, ##__VA_ARGS__)


#define rprintVln(x, ...)	if (Debug.isActive(Debug.VERBOSE)) 	Debug.println(x,  ##__VA_ARGS__)
#define rprintDln(x, ...)	if (Debug.isActive(Debug.DEBUG))	Debug.println(x, ##__VA_ARGS__)
#define rprintIln(x, ...)	if (Debug.isActive(Debug.INFO)) 	Debug.println(x, ##__VA_ARGS__)
#define rprintWln(x, ...)	if (Debug.isActive(Debug.WARNING)) 	Debug.println(x, ##__VA_ARGS__)
#define rprintEln(x, ...)	if (Debug.isActive(Debug.ERROR)) 	Debug.println(x, ##__VA_ARGS__)
#define rprintAln(x, ...)	if (Debug.isActive(Debug.ANY)) 		Debug.println(x, ##__VA_ARGS__)

///// Class

class RemoteDebug: public Print
{
	public:

	// Constructor

	RemoteDebug();

	// Methods

	bool begin(String hostName, uint16_t port, uint8_t startingDebugLevel = DEBUG);
	bool begin(String hostName, uint8_t startingDebugLevel = DEBUG);

	void setPassword(String password);

	void stop();

	void handle();

	void disconnect(boolean onlyTelnetClient = false);

	void setSerialEnabled(boolean enable);

	void setResetCmdEnabled(boolean enable);

	void setHelpProjectsCmds(String help);
	void setCallBackProjectCmds(void (*callback)());
	String getLastCommand();
	void clearLastCommand();

	void showTime(boolean show);
	void showProfiler(boolean show, uint32_t minTime = 0);
	void showDebugLevel(boolean show);
	void showColors(boolean show);

	void showRaw(boolean show);

	void setCallBackNewClient(void (*callback)());

#ifdef ALPHA_VERSION // In test, not good yet
	void autoProfilerLevel(uint32_t millisElapsed);
#endif

	void setFilter(String filter);
	void setNoFilter();

	boolean isActive(uint8_t debugLevel = DEBUG);

	void silence(boolean activate, boolean showMessage = true, boolean fromBreak = false, uint32_t timeout = 0);
	boolean isSilence();

	void onConnection(boolean connected);

	boolean isConnected();

#ifdef DEBUGGER_ENABLED
	// For Simple software debugger - based on SerialDebug Library
	void initDebugger(boolean (*callbackEnabled)(), void (*callbackHandle)(const boolean), String (*callbackGetHelp)(), void (*callbackProcessCmd)());
	WiFiClient* getTelnetClient();
#endif

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
	void wsOnReceive(const char* command);
	void wsSendInfo();
	void wsSendLevelInfo();
#endif
	boolean wsIsConnected();

	// Print

	virtual size_t write(uint8_t);

    virtual size_t write(const uint8_t *buffer, size_t size); // Insert due a write bug w/ latest Esp8266 SDK - 17/08/18

    // Debug levels

	static const uint8_t PROFILER = 0; 	// Used for show time of execution of pieces of code(profiler)
	static const uint8_t VERBOSE = 1; 	// Used for show verboses messages
	static const uint8_t DEBUG = 2;   	// Used for show debug messages
	static const uint8_t INFO = 3;		// Used for show info messages
	static const uint8_t WARNING = 4;	// Used for show warning messages
	static const uint8_t ERROR = 5;		// Used for show error messages
	static const uint8_t ANY = 6;		// Used for show always messages, for any current debug level

	// Expand characters as CR/LF to \\r, \\n

	String expand(String string);

	// Destructor

	~RemoteDebug();

private:

	// Variables

	String _hostName = "";				// Host name

	boolean _connected = false;			// Client is connected ?

	String _password = "";				// Password

	boolean _passwordOk = false; 		// Password request ? - 18/07/18
	uint8_t _passwordAttempt = 0;

	boolean _silence = false;			// Silence mode ?
	uint32_t _silenceTimeout = 0;		// Silence timeout

	uint8_t _clientDebugLevel = DEBUG;	// Level setted by user in web app or telnet client
	uint8_t _lastDebugLevel = DEBUG;	// Last Level setted by active()

	uint32_t _lastTimePrint = millis(); // Last time print a line

	uint8_t _levelBeforeProfiler=DEBUG;	// Last Level before Profiler level
	uint32_t _levelProfilerDisable = 0;	// time in millis to disable the profiler level
	uint32_t _autoLevelProfiler = 0;	// Automatic change to profiler level if time between handles is greater than n millis

	boolean _showTime = false;			// Show time in millis

	boolean _showProfiler = false;		// Show time between messages
	uint32_t _minTimeShowProfiler = 0;	// Minimal time to show profiler

	boolean _showDebugLevel = true;		// Show debug Level

	boolean _showColors = false;		// Show colors

	boolean _showRaw = false;			// Show in raw mode ?

	boolean _serialEnabled = false;		// Send to serial too (not recommended)

	boolean _resetCommandEnabled=false;	// Enable command to reset the board

	boolean _newLine = true;			// New line write ?

	String _command = "";				// Command received
	String _lastCommand = "";			// Last Command received
	uint32_t _lastTimeCommand = millis();// Last time command received
	String _helpProjectCmds = "";		// Help of comands setted by project (sketch)
	void (*_callbackProjectCmds)() = NULL; // Callable for projects commands
	void (*_callbackNewClient)() = NULL; // Callable for when have a new client connected

	String _filter = "";				// Filter
	boolean _filterActive = false;

	String _bufferPrint = "";			// Buffer of print write to WiFi

#ifdef CLIENT_BUFFERING
	String _bufferSend = "";			// Buffer to send data to web app or telnet client
	uint16_t _sizeBufferSend = 0;		// Size of it
	uint32_t _lastTimeSend = 0;			// Last time command send data
#endif

#ifdef DEBUGGER_ENABLED
//	// For Simple software debugger - based on SerialDebug Library
	boolean (*_callbackDbgEnabled)() = NULL;// Callable for debugger enabled
	void (*_callbackDbgHandle)(const boolean) = NULL;	// Callable for handle of debugger
	String (*_callbackDbgHelp)() = NULL;	// Callable for get debugger help
	void (*_callbackDbgProcessCmd)() = NULL;// Callable for process commands of debugger
#endif

	//////// Privates

	void showHelp();
	void processCommand();
	String formatNumber(uint32_t value, uint8_t size, char insert='0');
	boolean isCRLF(char character);
	uint32_t getFreeMemory();

#ifdef ALPHA_VERSION // In test, not good yet
	void sendTelnetCommand(uint8_t command, uint8_t option);
#endif

};

#else // DEBUG_DISABLED

// Disable debug macros

#define rdebugAln(...)
#define rdebugPln(...)
#define rdebugVln(...)
#define rdebugDln(...)
#define rdebugIln(...)
#define rdebugWln(...)
#define rdebugEln(...)
#define rdebug(...)

#define DEBUG(...)

#define DEBUG_A(...)
#define DEBUG_P(...)
#define DEBUG_V(...)
#define DEBUG_D(...)
#define DEBUG_I(...)
#define DEBUG_W(...)
#define DEBUG_E(...)

#define debugA(...)
#define debugP(...)
#define debugV(...)
#define debugD(...)
#define debugI(...)
#define debugW(...)
#define debugE(...)

#define rprintA(...)
#define rprintV(...)
#define rprintD(...)
#define rprintI(...)
#define rprintW(...)
#define rprintE(...)

#define rprintAln(...)
#define rprintVln(...)
#define rprintDln(...)
#define rprintIln(...)
#define rprintWln(...)
#define rprintEln(...)

#define debugHandle()

// Note all of Debug. codes need uses "#ifndef DEBUG_DISABLED"
// For example, the initialization codes and:

//#ifndef DEBUG_DISABLED
//if (Debug.isActive(Debug.VERBOSE)) {
//    Debug.printf("bla bla bla: %d %s", number, str); // OR
//    Debug.printf("bla bla bla: %d %s", number, str.c_str()); // Note: if type is String need c_str() // OR
//    Debug.println("bla bla bla 2 ln");
//}
//#endif

#endif // DEBUG_DISABLED

#endif // H

//////// End
