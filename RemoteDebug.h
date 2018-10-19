/*
 * Header for RemoteDebug
 *
 * Copyright (C) 2018  Joao Lopes https://github.com/JoaoLopesF/RemoteDebug
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * This header file describes the public API for sending debug messages to a telnet client.
 *
 */

#ifndef RemoteDebug_h
#define RemoteDebug_h

//////// Includes

#include "Arduino.h"
#include "Print.h"

//////// Defines

// Port for telnet server (now can be defined in project too - 17/08/18)
// Can be by project, just define it before include this file

#ifndef TELNET_PORT
#define TELNET_PORT 23
#endif

// Simple password request - left commented if not need this - 18/07/18
// Notes:
// It is very simple feature, only text, no cryptography,
// and the password is echoed in screen (I not discovery yet how disable it)
// telnet use advanced authentication (kerberos, etc.)
// Such now as RemoteDebug now is not for production releases,
// this kind of authentication will not be done now.
// Can be by project, just call setPassword method

#ifndef REMOTEDEBUG_PASSWORD
//#define REMOTEDEBUG_PASSWORD "r3m0t3."
#endif

#define REMOTEDEBUG_PWD_ATTEMPTS 3

// Maximum time for inactivity (em milliseconds)
// Default: 10 minutes
// Comment it if you not want this
// Can be by project, just define it before include this file

#ifndef MAX_TIME_INACTIVE
#define MAX_TIME_INACTIVE 600000
#endif

// Buffered print write to telnet -> length of buffer
// Can be by project, just define it before include this file

#ifndef BUFFER_PRINT
#define BUFFER_PRINT 150
#endif

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
#define COLOR_DARK_RED "\x1B[1;31m"
#define COLOR_DARK_GREEN "\x1B[1;32m"
#define COLOR_DARK_YELLOW "\x1B[1;33m"
#define COLOR_DARK_BLUE "\x1B[1;34m"
#define COLOR_DARK_MAGENTA "\x1B[1;35m"
#define COLOR_DARK_CYAN "\x1B[1;36m"
#define COLOR_DARK_WHITE "\x1B[1;37m"

#define COLOR_BACKGROUND_BLACK "\x1B[40m"
#define COLOR_BACKGROUND_RED "\x1B[41m"
#define COLOR_BACKGROUND_GREEN "\x1B[42m"
#define COLOR_BACKGROUND_YELLOW "\x1B[43m"
#define COLOR_BACKGROUND_BLUE "\x1B[44m"
#define COLOR_BACKGROUND_MAGENTA "\x1B[45m"
#define COLOR_BACKGROUND_CYAN "\x1B[46m"
#define COLOR_BACKGROUND_WHITE "\x1B[47m"

// Buffering (sends in interval of time to avoid ESP misterious delays)

#define CLIENT_BUFFERING true

#ifdef CLIENT_BUFFERING
#define DELAY_TO_SEND 10 // Time to send buffer
#define MAX_SIZE_SEND 1460 // Maximum size of packet (limit of TCP/IP)
#endif

// Enable if you test features yet in development

//#define ALPHA_VERSION true

////// Shortcuts macros

// Macros whith auto function ? (comment this if not want this) - 25/08/18

#define DEBUG_AUTO_FUNC true

#ifdef DEBUG_AUTO_FUNC

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

///// Class

class RemoteDebug: public Print
{
	public:

	void begin(String hostName, uint16_t port, uint8_t startingDebugLevel = DEBUG);
	void begin(String hostName, uint8_t startingDebugLevel = DEBUG);

	void setPassword(String password);

	void stop();

	void handle();

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

#ifdef ALPHA_VERSION // In test, not good yet
	void autoProfilerLevel(uint32_t millisElapsed);
#endif

	void setFilter(String filter);
	void setNoFilter();

	boolean isActive(uint8_t debugLevel = DEBUG);

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

#ifndef REMOTEDEBUG_PASSWORD
	String _password = "";				// Password for telnet
#else
	String _password = REMOTEDEBUG_PASSWORD;
#endif

	boolean _passwordOk = false; 		// Password request ? - 18/07/18
	uint8_t _passwordAttempt = 0;

	boolean _silence = false;			// Silence mode ?

	uint8_t _clientDebugLevel = DEBUG;	// Level setted by user in telnet
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

	boolean _serialEnabled = false;		// Send to serial too (not recommended)

	boolean _resetCommandEnabled=false;	// Command in telnet to reset ESP8266

	boolean _newLine = true;			// New line write ?

	String _command = "";				// Command received
	String _lastCommand = "";			// Last Command received
	uint32_t _lastTimeCommand = millis();// Last time command received
	String _helpProjectCmds = "";		// Help of comands setted by project (sketch)
	void (*_callbackProjectCmds)();		// Callable for projects commands

	String _filter = "";				// Filter
	boolean _filterActive = false;

	String _bufferPrint = "";			// Buffer of print write to telnet

#ifdef CLIENT_BUFFERING
	String _bufferSend = "";			// Buffer to send data to telnet
	uint16_t _sizeBufferSend = 0;		// Size of it
	uint32_t _lastTimeSend = 0;			// Last time command send data
#endif

	// Privates

	void showHelp();
	void processCommand();
	String formatNumber(uint32_t value, uint8_t size, char insert='0');
	boolean isCRLF(char character);

#ifdef ALPHA_VERSION // In test, not good yet
	void sendTelnetCommand(uint8_t command, uint8_t option);
#endif
};

#endif
