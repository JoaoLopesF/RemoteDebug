////////
// Header for RemoteDebug
///////

#ifndef RemoteDebug_h
#define RemoteDebug_h

//#define ALPHA_VERSION true // In development, not yet good

#if defined(ESP8266)
// ESP8266 SDK
extern "C" {
bool system_update_cpu_freq(uint8 freq);
}
#endif

// Libraries

#include "Arduino.h"
#include "Print.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#elif defined(ESP32)
#include <WiFi.h>
#else
#error Only for ESP8266 or ESP32
#endif

// Port for telnet

#define TELNET_PORT 23

// Maximun time for inactivity (em miliseconds)
// Default: 10 minutes
// Comment it if you not want this

#define MAX_TIME_INACTIVE 600000

// Buffered print write to telnet -> length of buffer

#define BUFFER_PRINT 150

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

// Shortcuts

#define DEBUG(...)   { if (Debug.isActive(Debug.ANY)) Debug.printf(__VA_ARGS__); }

#define DEBUG_P(...) { if (Debug.isActive(Debug.PROFILER)) Debug.printf(__VA_ARGS__); }
#define DEBUG_V(...) { if (Debug.isActive(Debug.VERBOSE)) Debug.printf(__VA_ARGS__); }
#define DEBUG_D(...) { if (Debug.isActive(Debug.DEBUG)) Debug.printf(__VA_ARGS__); }
#define DEBUG_I(...) { if (Debug.isActive(Debug.INFO)) Debug.printf(__VA_ARGS__); }
#define DEBUG_W(...) { if (Debug.isActive(Debug.WARNING)) Debug.printf(__VA_ARGS__); }
#define DEBUG_E(...) { if (Debug.isActive(Debug.ERROR)) Debug.printf(__VA_ARGS__); }

// Buffering (sends in interval of time to avoid ESP misterious delays)

#define CLIENT_BUFFERING true
#ifdef CLIENT_BUFFERING
#define DELAY_TO_SEND 10 // Time to send buffer
#define MAX_SIZE_SEND 1460 // Maximum size of packet (limit of TCP/IP)
#endif

// Class

class RemoteDebug: public Print
{
	public:

	void begin(String hostName, uint8_t startingDebugLevel = DEBUG);

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

private:

	// Variables

	String _hostName = "";				// Host name

	boolean _connected = false;			// Client is connected ?

	uint8_t _clientDebugLevel = DEBUG;	// Level setted by user in telnet
	uint8_t _lastDebugLevel = DEBUG;		// Last Level setted by active()

	uint32_t _lastTimePrint = millis();  // Last time print a line

	uint8_t _levelBeforeProfiler = DEBUG;// Last Level before Profiler level
	uint32_t _levelProfilerDisable = 0;	// time in millis to disable the profiler level
	uint32_t _autoLevelProfiler = 0;		// Automatic change to profiler level if time between handles is greater than n millis

	boolean _showTime = false;			// Show time in millis

	boolean _showProfiler = false;		// Show time between messages
	uint32_t _minTimeShowProfiler = 0;	// Minimal time to show profiler

	boolean _showDebugLevel = true;		// Show debug Level

	boolean _showColors = false;			// Show colors

	boolean _serialEnabled = false;		// Send to serial too (not recommended)

	boolean _resetCommandEnabled = false;// Command in telnet to reset ESP8266

	boolean _newLine = true;				// New line write ?

	String _command = "";				// Command received
	String _lastCommand = "";			// Last Command received
	uint32_t _lastTimeCommand = millis();// Last time command received
	String _helpProjectCmds = "";		// Help of comands setted by project (sketch)
	void (*_callbackProjectCmds)();		// Callable for projects commands

	String _filter = "";					// Filter
	boolean _filterActive = false;

	String _bufferPrint = "";			// Buffer of print write to telnet

#ifdef CLIENT_BUFFERING
	String _bufferSend = "";				// Buffer to send data to telnet
	uint16_t _sizeBufferSend = 0;		// Size of it
	uint32_t _lastTimeSend = 0;			// Last time command send data
#endif

	// Privates

	void showHelp();
	void processCommand();
	String formatNumber(uint32_t value, uint8_t size, char insert='0');
	boolean isCRLF(char character);

};

#endif
