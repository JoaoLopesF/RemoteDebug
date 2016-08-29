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

// Buffered print write to telnet ? (comment if you not wants this)

#define BUFFER_PRINT 100

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
	void showProfiler(boolean show);
	void showDebugLevel(boolean show);

	boolean ative(uint8_t debugLevel = DEBUG);

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
	uint8_t _lastDebugLevel = DEBUG;		// Last Level setted by ative()

	boolean _showTime = false;				// Show time in millis

	boolean _showProfiler = false;			// Show time between messages

	boolean _showDebugLevel = true;			// Show debug Level

	boolean _serialEnabled = false;			// Send to serial too (not recommended)

	boolean _resetCommandEnabled = false;	// Command in telnet to reset ESP8266

	boolean _newLine = true;				// New line write ?

	String _command = "";					// Command received
	uint32_t _lastTimeCommand = millis();	// Last time command received
	String _helpProjectCmds = "";			// Help of comands setted by project (sketch)
	void (*_callbackProjectCmds)();			// Callable for projects commands

	void showHelp();
	void processCommand();
	String formatNumber(uint32_t value, uint8_t size, char insert='0');

};

#endif
