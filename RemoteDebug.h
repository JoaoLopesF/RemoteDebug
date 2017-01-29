////////
// Header for RemoteDebug
///////

#ifndef RemoteDebug_h
#define RemoteDebug_h

#include "Arduino.h"
#include "Print.h"

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

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

// Class

class RemoteDebug: public Print
{
	public:

	void begin(String hostName);

	void stop();

	void handle();

	void setSerialEnabled(boolean enable);

	void setResetCmdEnabled(boolean enable);

	void setHelpProjectsCmds(String help);
	void setCallBackProjectCmds(void (*callback)());
	String getLastCommand();

	void showTime(boolean show);
	void showProfiler(boolean show, uint32_t minTime = 0);
	void showDebugLevel(boolean show);
	void showColors(boolean show);

	void setFilter(String filter);
	void setNoFilter();

	boolean isActive(uint8_t debugLevel = DEBUG);

	// Print

	virtual size_t write(uint8_t);

    // Debug levels

	static const uint8_t VERBOSE = 0;
	static const uint8_t DEBUG = 1;
	static const uint8_t INFO = 2;
	static const uint8_t WARNING = 3;
	static const uint8_t ERROR = 4;

private:

	// Variables

	String _hostName = "";					// Host name

	boolean _connected = false;				// Client is connected ?

	uint8_t _clientDebugLevel = DEBUG;		// Level setted by user in telnet
	uint8_t _lastDebugLevel = DEBUG;		// Last Level setted by active()

	boolean _showTime = false;				// Show time in millis

	boolean _showProfiler = false;			// Show time between messages
	uint32_t _minTimeShowProfiler = 0;		// Minimal time to show profiler

	boolean _showDebugLevel = true;			// Show debug Level

	boolean _showColors = false;			// Show colors

	boolean _serialEnabled = false;			// Send to serial too (not recommended)

	boolean _resetCommandEnabled = false;	// Command in telnet to reset ESP8266

	boolean _newLine = true;				// New line write ?

	String _command = "";					// Command received
	uint32_t _lastTimeCommand = millis();	// Last time command received
	String _helpProjectCmds = "";			// Help of comands setted by project (sketch)
	void (*_callbackProjectCmds)();			// Callable for projects commands

	String _filter = "";					// Filter
	boolean _filterActive = false;

	// Privates

	void showHelp();
	void processCommand();
	String formatNumber(uint32_t value, uint8_t size, char insert='0');

};

#endif
