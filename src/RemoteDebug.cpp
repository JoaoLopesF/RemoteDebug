/*
 * Libraries Arduino
 * *****************
 * Library : Remote debug - debug over telnet - for Esp8266 (NodeMCU) or ESP32
 * Author  : Joao Lopes
 * Comments: Telnet server based on example of TelnetServer code in http: *www.rudiswiki.de/wiki9/WiFiTelnetServer
 *           Web socket server uses the arduinWebSockets library (https://github.com/Links2004/arduinoWebSockets)
 *           The author uses Eclise IDE (sloeber) to made this source
 * License : See RemoteDebug.h
 *
 * Versions:
 *  ------	----------	-----------------
 *  3.0.5	2019-03-32  Ajustment on debugA macro, thanks @jetpax and @cmidgley to add this issue
 *  3.0.4	2019-03-19  All public configurations (#defines) have moved to RemoteDebugCfg.h, to facilitate changes for anybody.
 * 						Changed examples with warnings on change any #define in project,
 *					    with workarounds, if it not work. (thanks to @22MarioZ for added this issue)
 *  3.0.3	2019-03-18	Adjustments if web socket is disabled
 *  3.0.2	2019-03-16	Adjustments in examples, added one for debugger
 *  3.0.1	2019-03-13	Adjustments in silente mode
 *                      Commands from RemoteDebugApp now is treated
 *                      Adjusts to RemoteDebugger support connection by web sockets
 *	3.0.0	2019-03-11	If not disabled, add a web socket server to comunicate with RemoteDebugApp (HTML5 web app)
 *	                    The standard telnet still working, to debug with internet offline
 *	                    Ajustment on debugA macro, thanks @jetpax to add this issue
 *  2.1.2	2019-03-08	Add empty rprint* macros, if debug is disabled
 *  2.1.1	2019-03-06	Create option DEBUG_DISABLE_AUTO_FUNC
 *                      Create macros to be used for code converter: rprint and rprintln
 *    					RemoteDebug now have an code converters to help migrate codes
 *
 *  2.1.0	2019-03-04	Create precompiler DEBUG_DISABLED to compile for production/release,
 *                      equal that have in SerialDebug
 *                      Adjustments in examples
 *
 *  2.0.2	2019-03-03	Just to do new release, to update other files
 * 	2.0.1	2019-03-01  Adjustments for the debugger: it still disable until dbg command, equal to SerialDebug
 * 						The callback will to be called before print debug messages now
 * 						And only if debugger is enabled in RemoteDebugger (command dbg)
 * 					    Changed handle debugger logic
 *
 *	2.0.0 	2019-02-28	Added support to RemoteDebug addon library: the RemoteDebugger, an simple software debugger, based on SerialDebug
 *						New color system (uncomment COLOR_NEW_SYSTEM in remotedebug.h to return to old way)
 *	1.5.9 	2019-02-18	Bug> sometimes the processCommand is executed twice. Workaround> check time
 *	1.5.8 	2019-02-08	New macros to compatibility with SerialDebug (can use RemoteDebug or SerialDebug) thanks to @phrxmd
 * 	1.5.7 	2018-11-03	Fixed bug for MAX_TIME_INACTIVE
 * 	1.5.6 	2018-10-19	Adjustments based on pull request from @jeroenst (to allow serial output with telnet password and setPassword method)
 * 	1.5.5 	?			Serial output is now not allowed if telnet password is enabled
 * 	1.5.4 	?			Serial output not depending of telnet password (thanks @jeroenst for suggestion)
 * 	1.5.3 	?			Serial output adjustments (due bug in password logic)
 * 	1.5.2 	?			Correct rdebug macro (thanks @stritti)
 * 	1.5.1 	?			New command: silence
 *  		  			Added new rdebug?ln to put auto new line
 *  		  			Auto function and core if (for ESP32) in rdebug macros
 *  		  			Class destructor implemented
 * 	1.5.0 	?			Port can be pass in begin method (thanks @PjotrekSE for suggestion)
 *     	  				Few adjustments
 *     					this kind of authentication will not be done now.
 *      				Such as RemoteDebug now is not for production releases,
 *     			 		Note: telnet use advanced authentication (kerberos, etc.)
 *	1.4.0 	?			A simple text password request, if enabled (thanks @jeroenst for suggestion)
 *	1.3.1 	?			Retired # from VARGS precompiler macros
 *  		  			Few adjustments as ESP32 includes
 *  	      			Port number can be modified in project Arduino (.ino file)
 *	1.3.0 	Aug 2018	Bug in write with latest ESP8266 SDK
 *	1.2.1 	?			Adjusts to not cause error in Arduino
 *	1.2.0 	?			Added shortcuts and buffering to avoid delays
 *	1.1.1 	2017-11-24	Added support for the pass through of commands, and default debug levels thanks B. Harville
 *	1.1.0 	Aug 2017	Support to ESP32
 *          			New commands for CPU frequencies
 *	   	  				New level> profiler and auto-profiler
 *	1.0.1 	Aug 2017	New connection logic
 *	1.0.0 	Jan 2017	First RC
 *	0.9.1 	Oct 2016	Beta 2
 *	0.9.0 	Aug 2016	Beta 1
 *
 */

/*
 *  TODO: 	- Page HTML for begin/stop Telnet server
 *          - Add support to another Arduino WiFi boards (if have demand on it)
 */

///// RemoteDebug configuration

#include "RemoteDebugCfg.h"

///// Debug disable for compile to production/release ?
///// as nothing of RemotedDebug is compiled, zero overhead :-)

#ifndef DEBUG_DISABLED

///// Defines

#define VERSION "3.0.5"

///// Includes

#include "stdint.h"

#if defined(ESP8266)
// ESP8266 SDK
extern "C" {
bool system_update_cpu_freq(uint8_t freq);
}
#endif

#include "Arduino.h"
#include "Print.h"

#ifdef SERIAL_DEBUG_H
// Cannot used with SerialDebug at same time
#error "RemoteDebug cannot be used with SerialDebug"
#endif

// ESP8266 or ESP32 ?

#if defined(ESP8266)

#include <ESP8266WiFi.h>

#elif defined(ESP32)

#include <WiFi.h>

#else

#error Only for ESP8266 or ESP32

#endif

#include "RemoteDebug.h"		// This library

#ifdef ALPHA_VERSION // In test, not good yet
#include "telnet.h"
#endif

// Support to websocket connection with RemoteDebugApp
// Note: you must install the arduinoWebSocket library before

#ifndef WEBSOCKET_DISABLED // Only if Web socket enabled (RemoteDebugApp)
#include "RemoteDebugWS.h"
#endif

// Internal print macros for send messages to client

#ifndef WEBSOCKET_DISABLED // Only if Web socket enabled (RemoteDebugApp)

#define debugPrintf(fmt, ...) { \
	if (_connected) TelnetClient.printf(fmt, ##__VA_ARGS__);\
	else if (_connectedWS) DebugWS.printf(fmt, ##__VA_ARGS__);\
}
#define debugPrintln(str) { \
	if (_connected) TelnetClient.println(str);\
	else if (_connectedWS) DebugWS.println(str);\
}
#define debugPrint(str) { \
	if (_connected) TelnetClient.print(str);\
	else if (_connectedWS) DebugWS.print(str);\
}

#else // With web socket too

#define debugPrintf(fmt, ...) { \
	if (_connected) TelnetClient.printf(fmt, ##__VA_ARGS__);\
}
#define debugPrintln(str) { \
	if (_connected) TelnetClient.println(str);\
}
#define debugPrint(str) { \
	if (_connected) TelnetClient.print(str);\
}

#endif

// Internal debug macro - recommended stay disable

#define D(fmt, ...) 													// Without this
//#define D(fmt, ...) Serial.printf("rd: " fmt "\n", ##__VA_ARGS__) 	// Serial debug

////// Variables

// Instance

static RemoteDebug* _instance;

// WiFi server (telnet)

static WiFiServer TelnetServer(TELNET_PORT); // @suppress("Abstract class cannot be instantiated")
static WiFiClient TelnetClient; // @suppress("Abstract class cannot be instantiated")

// Support to websocket connection with RemoteDebugApp

#ifndef WEBSOCKET_DISABLED

// Instance of RemoteDebugWS

static RemoteDebugWS DebugWS; // @suppress("Abstract class cannot be instantiated")

static boolean _connectedWS = false; // Connected :

// Callbacks

class MyRemoteDebugCallbacks: public RemoteDebugWSCallbacks {
	void onConnect() {
		// Web socket (app) connected

		D("rd: onconnect");

		_connectedWS = true;

		// Is telnet connected -> disconnect it, due reduce overheads

		if (_instance->isConnected()) {

			_instance->disconnect(true);
		}

		// Call same routine that telnet

		_instance->onConnection(true);
	}
	void onDisconnect() {
		// Web socket (app) disconnected

		D("rd: ondisconnect");

		_connectedWS = false;
	}
	void onReceive(const char *message) {
		// Receive a message

		D("rd: onreceive");

		_instance->wsOnReceive(message);
	}
};

#endif // WEBSOCKET_DISABLED

////// Methods / routines

// Constructor

RemoteDebug::RemoteDebug() {

	// Save the instance

	_instance = this;
}

// Initialize the telnet server

bool RemoteDebug::begin(String hostName, uint8_t startingDebugLevel) {
	return begin(hostName, TELNET_PORT, startingDebugLevel);
}

bool RemoteDebug::begin(String hostName, uint16_t port,  uint8_t startingDebugLevel) {

	// Initialize server telnet

	if (port != TELNET_PORT) { // Bug: not more can use begin(port)..
	    return false;
	}
	
	TelnetServer.begin();
	TelnetServer.setNoDelay(true);

#ifndef WEBSOCKET_DISABLED
	// Initialize web socket (for RemoteDebugApp)

	DebugWS.begin(new MyRemoteDebugCallbacks());

#endif

	// Reserve space to buffer of print writes

	_bufferPrint.reserve(BUFFER_PRINT);

#ifdef CLIENT_BUFFERING
	// Reserve space to buffer of send

	_bufferPrint.reserve(MAX_SIZE_SEND);

#endif

	// Host name of this device

	_hostName = hostName;

	// Debug level

	_clientDebugLevel = startingDebugLevel;
	_lastDebugLevel = startingDebugLevel;

	return true;
}

#ifdef DEBUGGER_ENABLED
// Simple software debugger - based on SerialDebug Library
void RemoteDebug::initDebugger(boolean (*callbackEnabled)(), void (*callbackHandle)(const boolean), String (*callbackGetHelp)(), void (*callbackProcessCmd)()) {

	// Init callbacks for the debugger

	_callbackDbgEnabled = callbackEnabled;
	_callbackDbgHandle = callbackHandle;
	_callbackDbgHelp = callbackGetHelp;
	_callbackDbgProcessCmd = callbackProcessCmd;

}

WiFiClient* RemoteDebug::getTelnetClient() {

	return &TelnetClient;
}

#endif

// Set the password for telnet - thanks @jeroenst for suggest thist method

void RemoteDebug::setPassword(String password) {

	_password = password;

}

// Destructor

RemoteDebug::~RemoteDebug() {

	// Flush

	if (TelnetClient && TelnetClient.connected()) {
		TelnetClient.flush();
	}

	// Stop

	stop();
}

// Stop the server

void RemoteDebug::stop() {

	// Stop Client

	if (TelnetClient && TelnetClient.connected()) {
		TelnetClient.stop();
	}

	// Stop server

	TelnetServer.stop();

#ifndef WEBSOCKET_DISABLED
	// Stop web socket (RemoteDebugApp)

	DebugWS.stop();                 // stop the websocket server
#endif

}

// Handle the connection (in begin of loop in sketch)
// TODO: optimize when loop not have a large delay

void RemoteDebug::handle() {

#ifdef ALPHA_VERSION // In test, not good yet
	static uint32_t lastTime = millis();
#endif

#ifdef DEBUGGER_ENABLED
	static uint32_t dbgTimeHandle = millis(); // To avoid call the handler desnecessary
	static boolean dbgLastConnected = false; // Last is connected ?
#endif

	// Silence timeout ?

	if (_silence && _silenceTimeout > 0 && millis() >= _silenceTimeout) {

		// Get out of silence mode

		silence(false, true);
	}

	// Debug level is profiler -> set the level before

	if (_clientDebugLevel == PROFILER) {
		if (millis() > _levelProfilerDisable) {
			_clientDebugLevel = _levelBeforeProfiler;
			debugPrintln("* Debug level profile inactive now");
		}
	}

#ifdef ALPHA_VERSION // In test, not good yet

	// Automatic change to profiler level if time between handles is greater than n millis

	if (_autoLevelProfiler > 0 && _clientDebugLevel != PROFILER) {

		uint32_t diff = (millis() - lastTime);

		if (diff >= _autoLevelProfiler) {
			_levelBeforeProfiler = _clientDebugLevel;
			_clientDebugLevel = PROFILER;
			_levelProfilerDisable = 1000; // Disable it at 1 sec

			debugPrintf("* Debug level profile active now - time between handels: %u\r\n", diff);
		}

		lastTime = millis();
	}
#endif

	// look for Client connect trial

	if (TelnetServer.hasClient()) {

		// Old connection logic

//      if (!TelnetClient || !TelnetClient.connected()) {
//
//        if (TelnetClient) { // Close the last connect - only one supported
//
//          TelnetClient.stop();
//
//        }

		// New connection logic - 10/08/17

		if (TelnetClient && TelnetClient.connected()) {

			// Verify if the IP is same than actual conection

			WiFiClient newClient; // @suppress("Abstract class cannot be instantiated")
			newClient = TelnetServer.available();
			String ip = newClient.remoteIP().toString();

			if (ip == TelnetClient.remoteIP().toString()) {

				// Reconnect

				TelnetClient.stop();
				TelnetClient = newClient;

			} else {

				// Disconnect (not allow more than one connection)

				newClient.stop();

				return;

			}

		} else {

			// New TCP client

			TelnetClient = TelnetServer.available();

			// Password request ? - 18/07/18

			if (_password != "") {

	#ifdef ALPHA_VERSION // In test, not good yet
				// Send command to telnet client to not do local echos
				// Experimental code !

				sendTelnetCommand(TELNET_WONT, TELNET_ECHO);
	#endif

			}
		}

		if (!TelnetClient) { // No client yet ???
			return;
		}

		// Set client

		TelnetClient.setNoDelay(true); // More faster
		TelnetClient.flush(); // clear input buffer, else you get strange characters

		// Empty buffer

		delay(100);

		while (TelnetClient.available()) {
			TelnetClient.read();
		}

		// Connection event

		onConnection(true);

	}

	// Is client connected ? (to reduce overhead in active)

	_connected = (TelnetClient && TelnetClient.connected());

	// Get command over telnet

	if (_connected) {

		char last = ' '; // To avoid process two times the "\r\n"

		while (TelnetClient.available()) {  // get data from Client

			// Get character

			char character = TelnetClient.read();

			// Newline (CR or LF) - once one time if (\r\n) - 26/07/17

			if (isCRLF(character) == true) {

				if (isCRLF(last) == false) {

					// Process the command

					if (_command.length() > 0) {

						_lastCommand = _command; // Store the last command
						processCommand();

					}
				}

				_command = ""; // Init it for next command

			} else if (isPrintable(character)) {

				// Concat

				_command.concat(character);

			}

			// Last char

			last = character;
		}

	}

	// Client connected ?

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
	boolean connected = (_connected || _connectedWS);
#else // By telnet
	boolean connected = _connected;
#endif

	if (connected) {

#ifdef CLIENT_BUFFERING
		// Client buffering - send data in intervals to avoid delays or if its is too big

		if ((millis() - _lastTimeSend) >= DELAY_TO_SEND || _sizeBufferSend >= MAX_SIZE_SEND) {
			debugPrint(_bufferSend);
			_bufferSend = "";
			_sizeBufferSend = 0;
			_lastTimeSend = millis();
		}
#endif

#ifdef MAX_TIME_INACTIVE
#if MAX_TIME_INACTIVE > 0

		// Inactivity - close connection if not received commands from user in telnet
		// For reduce overheads

		uint32_t maxTime = MAX_TIME_INACTIVE; // Normal

		if (_password != "" && !_passwordOk) { // Request password - 18/08/08
			maxTime = 60000; // One minute to password
		}

		if ((millis() - _lastTimeCommand) > maxTime) {

			debugPrintln("* Closing session by inactivity");

			// Disconnect

			disconnect();
			return;
		}
#endif
#endif
	}

#ifndef WEBSOCKET_DISABLED // For websocket server

	// Web socket server handle

	DebugWS.handle();

#endif

#ifdef DEBUGGER_ENABLED

	// For Simple software debugger - based on SerialDebug Library

	// Changed handle debugger logic - 2018-03-01

	if (_callbackDbgEnabled && _callbackDbgHandle) { // Calbacks ok ?

		boolean callHandle = false;

		if (dbgLastConnected != connected) { // Change connection -> always call

			dbgLastConnected = connected;
			callHandle = true;

		} else if (millis() >= dbgTimeHandle) {

			if (_callbackDbgEnabled()) { // Only if it is enabled
				callHandle = true;
			}
		}

		if (callHandle) {

			// Call the handle

			_callbackDbgHandle(true);

			// Save time

			dbgTimeHandle = millis() + DEBUGGER_HANDLE_TIME;
		}
	}
#endif

	//DV("*handle time: ", (millis() - timeBegin));
}


// Disconnect client

void RemoteDebug::disconnect(boolean onlyTelnetClient) {

	// Disconnect

	if (onlyTelnetClient) {
		if (_connected) {
			TelnetClient.println("* Closing client connection ..."); // this is to web app new conn not receive it
		}
	} else {
		debugPrintln("* Closing client connection ...");
	}

	_silence = false;
	_silenceTimeout = 0;

	if (_connected) { // By telnet
		TelnetClient.stop();
		_connected = false;
	}
#ifndef WEBSOCKET_DISABLED // For web socket server (app)
	if (_connectedWS && !onlyTelnetClient) {
		DebugWS.disconnect(); // Disconnect client
		_connectedWS = false;
	}
#endif
}

// Connection/disconnection event

void RemoteDebug::onConnection(boolean connected) {

	// Clear variables

	D("rd onconn %d", connected);

	_bufferPrint = "";			// Clean buffer

	_lastTimeCommand = millis(); // To mark time for inactivity

	_command = "";				// Clear command
	_lastCommand = "";			// Clear las command

	_lastTimePrint = millis();	// Clear the time

	_silence = false;			// No silence
	_silenceTimeout = 0;

#ifdef CLIENT_BUFFERING
	// Client buffering - send data in intervals to avoid delays or if its is too big
	_bufferSend = "";
	_sizeBufferSend = 0;
	_lastTimeSend = millis();
#endif

	// Password request ? - 18/07/18

	if (_password != "") {

		_passwordOk = false;

#ifdef REMOTEDEBUG_PWD_ATTEMPTS
		_passwordAttempt = 1;
#endif
	}

	// Save it

	_connected = connected;

	// Process

	if (connected) { // Connected ?


		// Callback

		if (_callbackNewClient) {
			_callbackNewClient();
		}

		// Show the initial message

#if SHOW_HELP
		showHelp();
#endif
	}
}

boolean RemoteDebug::isConnected() {

	// Is connected

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
	return (_connected || _connectedWS);
#else
	return _connected;
#endif
}

// Send to serial too (use only if need)

void RemoteDebug::setSerialEnabled(boolean enable) {

	_serialEnabled = enable;
	_showColors = false; // Disable it for Serial

}

// Allow ESP reset over telnet client

void RemoteDebug::setResetCmdEnabled(boolean enable) {
	_resetCommandEnabled = enable;
}

// Show time in millis

void RemoteDebug::showTime(boolean show) {
	_showTime = show;
}

// Show profiler - time in millis between messages of debug

void RemoteDebug::showProfiler(boolean show, uint32_t minTime) {
	_showProfiler = show;
	_minTimeShowProfiler = minTime;
}

#ifdef ALPHA_VERSION // In test, not good yet
// Automatic change to profiler level if time between handles is greater than n mills (0 - disable)

void RemoteDebug::autoProfilerLevel(uint32_t millisElapsed) {
	_autoLevelProfiler = millisElapsed;
}
#endif

// Show debug level

void RemoteDebug::showDebugLevel(boolean show) {
	_showDebugLevel = show;
}

// Show colors

void RemoteDebug::showColors(boolean show) {
	if (_serialEnabled == false) {
		_showColors = show;
	} else {
		_showColors = false; // Disable it for Serial
	}
}

// Show in raw mode - only data ?

void RemoteDebug::showRaw(boolean show) {
	_showRaw = show;
}


// Is active ? client telnet connected and level of debug equal or greater then set by user in telnet

boolean RemoteDebug::isActive(uint8_t debugLevel) {

	// Active ->
	//	Not in silence (new)
	//	Debug level ok and
	//	Telnet connected or
	//	Serial enabled (use only if need)
	//	Password ok (if enabled) - 18/08/18

#ifndef WEBSOCKET_DISABLED // For web socket server (app)

	boolean ret = (debugLevel >= _clientDebugLevel &&
					!_silence &&
					(_connected || _connectedWS || _serialEnabled));

#else // Telnet only

	boolean ret = (debugLevel >= _clientDebugLevel &&
					!_silence &&
					(_connected || _serialEnabled));

#endif


	if (ret) {
		_lastDebugLevel = debugLevel;
	}

	return ret;

}

// Set help for commands over telnet set by sketch

void RemoteDebug::setHelpProjectsCmds(String help) {

	_helpProjectCmds = help;

}

// Set callback of sketch function to process project messages

void RemoteDebug::setCallBackProjectCmds(void (*callback)()) {
	_callbackProjectCmds = callback;
}

void RemoteDebug::setCallBackNewClient(void (*callback)()) {
	_callbackNewClient = callback;
}

// Print

size_t RemoteDebug::write(const uint8_t *buffer, size_t size) {

	// Process buffer
	// Insert due a write bug w/ latest Esp8266 SDK - 17/08/18

	for(size_t i=0; i<size; i++) {
		write((uint8_t) buffer[i]);
	}

	return size;
}

size_t RemoteDebug::write(uint8_t character) {

	// Write logic

    uint32_t elapsed = 0;

	size_t ret = 0;

#ifdef COLOR_NEW_SYSTEM
	String colorLevel = "";
#endif

	// Connected ?

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
	boolean connected = (_connected || _connectedWS);
#else
	boolean connected = _connected;
#endif

	// In silente mode now ?

	if (_silence) {
		return 0;
	}

	// New line writted before ?

	if (_newLine ) {

#ifdef DEBUGGER_ENABLED

	// For Simple software debugger - based on SerialDebug Library

	// Changed handle debugger logic - 2018-02-29

	if (!_showRaw) { // Not for raw mode

		if (_callbackDbgEnabled && _callbackDbgEnabled()) { // Callbacks ok

			if (connected && _callbackDbgEnabled()) { // Only call if is connected and debugger is enabled

				// Call the handle

				_callbackDbgHandle(false);
			}
		}
	}
#endif

		String show = "";

		// Not in raw mode (only data)

		if (!_showRaw) {

#ifdef COLOR_NEW_SYSTEM

			// New color system

			if (_showColors) {
				switch (_lastDebugLevel) {
				case VERBOSE:
					show = COLOR_VERBOSE;
					break;
				case DEBUG:
					show = COLOR_DEBUG;
					break;
				case INFO:
					show = COLOR_INFO;
					break;
				case WARNING:
					show = COLOR_WARNING;
					break;
				case ERROR:
					show = COLOR_ERROR;
					break;
				}
				colorLevel = show;
			}

			// Show debug level

			if (_showDebugLevel) {
				switch (_lastDebugLevel) {
				case PROFILER:
					show.concat("(P");
					break;
				case VERBOSE:
					show.concat("(V");
					break;
				case DEBUG:
					show.concat("(D");
					break;
				case INFO:
					show.concat("(I");
					break;
				case WARNING:
					show.concat("(W");
					break;
				case ERROR:
					show.concat("(E");
					break;
				}
			}

			// Show time in millis

			if (_showTime) {
				if (show != "") {
					show.concat(" ");
				}
				show.concat("t:");
				show.concat(millis());
				show.concat("ms");
			}

			// Show profiler (time between messages)

			if (_showProfiler) {
				elapsed = (millis() - _lastTimePrint);
				boolean resetColors = false;
				if (show != "") {
					show.concat(" ");
				}
				if (_showColors) {
					if (elapsed < 250) {
						; // not color this
					} else if (elapsed < 1000) {
						show.concat(COLOR_BLACK);
						show.concat(COLOR_BACKGROUND_GREEN);
						resetColors = true;
					} else if (elapsed < 3000) {
						show.concat(COLOR_BLACK);
						show.concat(COLOR_BACKGROUND_YELLOW);
						resetColors = true;
					} else if (elapsed < 5000) {
						show.concat(COLOR_WHITE);
						show.concat(COLOR_BACKGROUND_MAGENTA);
						resetColors = true;
					} else {
						show.concat(COLOR_WHITE);
						show.concat(COLOR_BACKGROUND_RED);
						resetColors = true;
					}
				}
				show.concat("p:^");
				show.concat(formatNumber(elapsed, 4));
				show.concat("ms");
				if (resetColors) {
					show.concat(COLOR_RESET);
#ifdef COLOR_NEW_SYSTEM
					show.concat(colorLevel);
#endif
				}
				_lastTimePrint = millis();
			}

#else // Old colors way

			// Show debug level

			if (_showDebugLevel) {
				show = "(";
				if (_showColors == false) {
					switch (_lastDebugLevel) {
					case PROFILER:
						show.concat("P");
						break;
					case VERBOSE:
						show.concat("V");
						break;
					case DEBUG:
						show.concat("D");
						break;
					case INFO:
						show.concat("I");
						break;
					case WARNING:
						show.concat("W");
						break;
					case ERROR:
						show.concat("E");
						break;
					}
				} else {
					switch (_lastDebugLevel) {
					case PROFILER:
						show.concat("P");
						break;
					case VERBOSE:
						show.concat("V");
						break;
					case DEBUG:
						show = COLOR_BACKGROUND_GREEN;
						show.concat("D");
						break;
					case INFO:
						show = COLOR_BACKGROUND_WHITE;
						show.concat("I");
						break;
					case WARNING:
						show = COLOR_BACKGROUND_YELLOW;
						show.concat("W");
						break;
					case ERROR:
						show = COLOR_BACKGROUND_RED;
						show.concat("E");
						break;
					}
					if (show.length() > 1) {
						show.concat(COLOR_RESET);
					}
				}
			}

			// Show time in millis

			if (_showTime) {
				if (show != "")
					show.concat(" ");
				show.concat("t:");
				show.concat(millis());
				show.concat("ms");
			}

			// Show profiler (time between messages)

			if (_showProfiler) {
				elapsed = (millis() - _lastTimePrint);
				boolean resetColors = false;
				if (show != "")
					show.concat(" ");
				if (_showColors) {
					if (elapsed < 250) {
						; // not color this
					} else if (elapsed < 1000) {
						show.concat(COLOR_BACKGROUND_CYAN);
						resetColors = true;
					} else if (elapsed < 3000) {
						show.concat(COLOR_BACKGROUND_YELLOW);
						resetColors = true;
					} else if (elapsed < 5000) {
						show.concat(COLOR_BACKGROUND_MAGENTA);
						resetColors = true;
					} else {
						show.concat(COLOR_BACKGROUND_RED);
						resetColors = true;
					}
				}
				show.concat("p:^");
				show.concat(formatNumber(elapsed, 4));
				show.concat("ms");
				if (resetColors) {
					show.concat(COLOR_RESET);
				}
				_lastTimePrint = millis();
			}

#endif

		} else { // Raw mode - only data - e.g. used for debugger messages

#ifdef COLOR_NEW_SYSTEM
			show.concat(COLOR_RAW);
#endif
		}

		// Show anything ?

		if (show != "") {

			if (!_showRaw) {
				show.concat(") ");
			}

			// Write to telnet buffered

			if (connected || _serialEnabled) {  // send data to Client
				_bufferPrint = show;
			}
		}

		_newLine = false;

	}

	// Print ?

	boolean doPrint = false;

	// New line ?

	if (character == '\n') {

		_bufferPrint.concat("\r"); // Para clientes windows - 29/01/17

		_newLine = true;
		doPrint = true;

	} else if (_bufferPrint.length() == BUFFER_PRINT) { // Limit of buffer

		doPrint = true;

	}

	// Write to telnet Buffered

	_bufferPrint.concat((char) character);

	// Send the characters buffered by print.h

	if (doPrint) { // Print the buffer

		boolean noPrint = false;

		if (_showProfiler && elapsed < _minTimeShowProfiler) { // Profiler time Minimal
			noPrint = true;
		} else if (_filterActive) { // Check filter before print

			String aux = _bufferPrint;
			aux.toLowerCase();

			if (aux.indexOf(_filter) == -1) { // not find -> no print
				noPrint = true;
			}
		}

		if (noPrint == false) {

#ifdef COLOR_NEW_SYSTEM
			_bufferPrint.concat(COLOR_RESET);
#endif
			// Send to telnet or websocket (buffered)

			boolean sendToClient = connected;

			if (_password != "" && !_passwordOk) { // With no password -> no telnet output - 2018-10-19
				sendToClient = false;
			}

			if (sendToClient) {  // send data to Client


#ifndef CLIENT_BUFFERING
				debugPrint(_bufferPrint);
#else // Cliente buffering

				uint8_t size = _bufferPrint.length();

				// Buffer too big ?

				if ((_sizeBufferSend + size) >= MAX_SIZE_SEND) {

					// Send it

					debugPrint(_bufferSend);
					_bufferSend = "";
					_sizeBufferSend = 0;
					_lastTimeSend = millis();
				}

				// Add to buffer of send

				_bufferSend.concat(_bufferPrint);
				_sizeBufferSend+=size;

				// Client buffering - send data in intervals to avoid delays or if its is too big
				// Not for raw mode
				if (_showRaw || (millis() - _lastTimeSend) >= DELAY_TO_SEND) {
					debugPrint(_bufferSend);
					_bufferSend = "";
					_sizeBufferSend = 0;
					_lastTimeSend = millis();
				}
#endif
			}

			// Echo to serial (not buffering it)

			if (_serialEnabled) {
				Serial.print(_bufferPrint);
			}
		}

		// Empty the buffer

		ret = _bufferPrint.length();
		_bufferPrint = "";
	}

	// Retorna

	return ret;
}

////// Private


// Show help of commands

void RemoteDebug::showHelp() {

	// Show the initial message

	String help = "";

	// Password request ? - 04/03/18

	if (_password != "" && !_passwordOk) {

		help.concat("\r\n");
		help.concat("* Please enter with a password to access");
#ifdef REMOTEDEBUG_PWD_ATTEMPTS
		help.concat(" (attempt ");
		help.concat(_passwordAttempt);
		help.concat(" of ");
		help.concat(REMOTEDEBUG_PWD_ATTEMPTS);
		help.concat(")");
#endif
		help.concat(':');
		help.concat("\r\n");

		debugPrint(help);

		return;
}

	// Show help

#if defined(ESP8266)
	help.concat("*** Remote debug - over telnet - for ESP8266 (NodeMCU) - version ");
#elif defined(ESP32)
	help.concat("*** Remote debug - over telnet - for ESP32 - version ");
#endif
	help.concat(VERSION);
	help.concat("\r\n");
	help.concat("* Host name: ");
	help.concat(_hostName);
	help.concat(" IP:");
	help.concat(WiFi.localIP().toString());
	help.concat(" Mac address:");
	help.concat(WiFi.macAddress());
	help.concat("\r\n");
	help.concat("* Free Heap RAM: ");
	help.concat(ESP.getFreeHeap());
	help.concat("\r\n");
	help.concat("* ESP SDK version: ");
	help.concat(ESP.getSdkVersion());
	help.concat("\r\n");
	help.concat("******************************************************\r\n");
	help.concat("* Commands:\r\n");
	help.concat("    ? or help -> display these help of commands\r\n");
	help.concat("    q -> quit (close this connection)\r\n");
	help.concat("    m -> display memory available\r\n");
	help.concat("    v -> set debug level to verbose\r\n");
	help.concat("    d -> set debug level to debug\r\n");
	help.concat("    i -> set debug level to info\r\n");
	help.concat("    w -> set debug level to warning\r\n");
	help.concat("    e -> set debug level to errors\r\n");
	help.concat("    s -> set debug silence on/off\r\n");
	help.concat("    l -> show debug level\r\n");
	help.concat("    t -> show time (millis)\r\n");
	help.concat("    profiler:\r\n");
	help.concat(
			"      p      -> show time between actual and last message (in millis)\r\n");
	help.concat("      p min  -> show only if time is this minimal\r\n");
	help.concat("      P time -> set debug level to profiler\r\n");
#ifdef ALPHA_VERSION // In test, not good yet
	help.concat("      A time -> set auto debug level to profiler\r\n");
#endif
	help.concat("    c -> show colors\r\n");
	help.concat("    filter:\r\n");
	help.concat("          filter <string> -> show only debugs with this\r\n");
	help.concat("          nofilter        -> disable the filter\r\n");
#if defined(ESP8266)
	help.concat("    cpu80  -> ESP8266 CPU a 80MHz\r\n");
	help.concat("    cpu160 -> ESP8266 CPU a 160MHz\r\n");
	if (_resetCommandEnabled) {
		help.concat("    reset -> reset the ESP8266\r\n");
	}
#elif defined(ESP32)
	if (_resetCommandEnabled) {
		help.concat("    reset -> reset the ESP32\r\n");
	}
#endif

	// Callbacks

	if (_helpProjectCmds != "" && (_callbackProjectCmds)) {
		help.concat("\r\n");
		help.concat("    * Project commands:\r\n");
		String show = "\r\n";
		show.concat(_helpProjectCmds);
		show.replace("\n", "\n    "); // ident this
		help.concat(show);
	}

#ifdef DEBUGGER_ENABLED
	// Get help for the debugger

	if (_callbackDbgHelp) {
		help.concat("\r\n");
		help.concat(_callbackDbgHelp());
	}
#endif
	help.concat("\r\n");

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
	if (!_connectedWS) {  // For telnet only
		help.concat("****\r\n");
		help.concat("* New features available:\r\n");
		help.concat("* - Now you can debug in web browser too.\r\n");
		help.concat("*   Please access: http://joaolopesf.net/remotedebugapp\r\n");
		help.concat("* - Now you can add an simple software debuggger.\r\n");
		help.concat("*   Please access: https://github.com/JoaoLopesF/RemoteDebugger\r\n");
		help.concat("****\r\n");
	}
#endif

	help.concat("\r\n");
	help.concat(
			"* Please type the command and press enter to execute.(? or h for this help)\r\n");
	help.concat("***\r\n");

	// Send to client

	debugPrint(help);
}

// Get last command received

String RemoteDebug::getLastCommand() {

	return _lastCommand;
}

// Clear the last command received

void RemoteDebug::clearLastCommand() {
	_lastCommand = "";
}

// Process user command over telnet or web socket

void RemoteDebug::processCommand() {

	static uint32_t lastTime = 0;

	// Bug -> sometimes the command is process twice
	// Workaround -> check time
	// TODO: see correction for this

	if (lastTime > 0 && (millis() - lastTime) < 500) {
		debugPrintln("* Bug workaround: ignoring command repeating");
		return;
	}
	lastTime = millis();

	D("cmd: %s" , _command.c_str());

	// Password request ? - 18/07/18

	if (_password != "" && !_passwordOk) { // Process the password - 18/08/18 - adjust in 04/09/08 and 2018-10-19

		if (_command == _password) {

			debugPrintln("* Password ok, allowing access now...");

			_passwordOk = true;

#ifdef ALPHA_VERSION // In test, not good yet
			sendTelnetCommand(TELNET_WILL, TELNET_ECHO); // Send a command to telnet to restore echoes = 18/08/18
#endif
			showHelp();

		} else {

			debugPrintln("* Wrong password!");

	#ifdef REMOTEDEBUG_PWD_ATTEMPTS

			_passwordAttempt++;

			if (_passwordAttempt > REMOTEDEBUG_PWD_ATTEMPTS) {

				debugPrintln("* Many attempts. Closing session now.");

				// Disconnect

				disconnect();

			} else {

				showHelp();
			}

	#endif
		}

		return;

	}

	// Process commands

	debugPrint("* Debug: Command received: ");
	debugPrintln(_command);

	String options = "";
	uint8_t pos = _command.indexOf(" ");
	if (pos > 0) {
		options = _command.substring(pos + 1);
	}

	// Set time of last command received

	_lastTimeCommand = millis();

	// Get out of silent mode

	if (_command != "s" && _silence) {

		silence(false, true);
	}

	// Process the command

	if (_command == "h" || _command == "?") {

		// Show help

		showHelp();

	} else if (_command == "q") {

		// Quit

		debugPrintln("* Closing client connection ...");

		TelnetClient.stop();

	} else if (_command == "m") {

		uint32_t free = ESP.getFreeHeap();

		debugPrint("* Free Heap RAM: ");
		debugPrintln(free);

#ifndef WEBSOCKET_DISABLED // For web socket server (app)

	// Send status to app

	if (_connectedWS) {
		DebugWS.printf("$app:M:%lu:\n", free);
	}

#endif

#if defined(ESP8266)

	} else if (_command == "cpu80") {

		// Change ESP8266 CPU para 80 MHz

		system_update_cpu_freq(80);
		debugPrintln("CPU ESP8266 changed to: 80 MHz");

	} else if (_command == "cpu160") {

		// Change ESP8266 CPU para 160 MHz

		system_update_cpu_freq(160);
		debugPrintln("CPU ESP8266 changed to: 160 MHz");

#endif

	} else if (_command == "v") {

		// Debug level

		_clientDebugLevel = VERBOSE;

		debugPrintln("* Debug level set to Verbose");

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
		wsSendLevelInfo();
#endif

	} else if (_command == "d") {

		// Debug level

		_clientDebugLevel = DEBUG;

		debugPrintln("* Debug level set to Debug");

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
		wsSendLevelInfo();
#endif

	} else if (_command == "i") {

		// Debug level

		_clientDebugLevel = INFO;

		debugPrintln("* Debug level set to Info");

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
		wsSendLevelInfo();
#endif

	} else if (_command == "w") {

		// Debug level

		_clientDebugLevel = WARNING;

		debugPrintln("* Debug level set to Warning");

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
		wsSendLevelInfo();
#endif

	} else if (_command == "e") {

		// Debug level

		_clientDebugLevel = ERROR;

		debugPrintln("* Debug level set to Error");

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
		wsSendLevelInfo();
#endif

	} else if (_command == "l") {

		// Show debug level

		_showDebugLevel = !_showDebugLevel;

		debugPrintf("* Show debug level: %s\r\n",
				(_showDebugLevel) ? "On" : "Off");

	} else if (_command == "t") {

		// Show time

		_showTime = !_showTime;

		debugPrintf("* Show time: %s\r\n", (_showTime) ? "On" : "Off");

	} else if (_command == "s") {

		// Toogle silence (new) = 28/08/18

		silence(!_silence);

	} else if (_command == "p") {

		// Show profiler

		_showProfiler = !_showProfiler;
		_minTimeShowProfiler = 0;

		debugPrintf("* Show profiler: %s\r\n",
				(_showProfiler) ? "On" : "Off");

	} else if (_command.startsWith("p ")) {

		// Show profiler with minimal time

		if (options.length() > 0) { // With minimal time
			int32_t aux = options.toInt();
			if (aux > 0) { // Valid number
				_showProfiler = true;
				_minTimeShowProfiler = aux;
				debugPrintf(
						"* Show profiler: On (with minimal time: %u)\r\n",
						_minTimeShowProfiler);
			}
		}

	} else if (_command == "P") {

		// Debug level profile

		_levelBeforeProfiler = _clientDebugLevel;
		_clientDebugLevel = PROFILER;

		if (_showProfiler == false) {
			_showProfiler = true;
		}

		_levelProfilerDisable = 1000; // Default

		if (options.length() > 0) { // With time of disable
			int32_t aux = options.toInt();
			if (aux > 0) { // Valid number
				_levelProfilerDisable = millis() + aux;
			}
		}

		debugPrintf(
				"* Debug level set to Profiler (disable in %u millis)\r\n",
				_levelProfilerDisable);

	} else if (_command == "A") {

		// Auto debug level profile

		_autoLevelProfiler = 1000; // Default

		if (options.length() > 0) { // With time of disable
			int32_t aux = options.toInt();
			if (aux > 0) { // Valid number
				_autoLevelProfiler = aux;
			}
		}

		debugPrintf(
				"* Auto profiler debug level active (time >= %u millis)\r\n",
				_autoLevelProfiler);

	} else if (_command == "c") {

		// Show colors

		_showColors = !_showColors;

		debugPrintf("* Show colors: %s\r\n",
				(_showColors) ? "On" : "Off");

	} else if (_command.startsWith("filter ") && options.length() > 0) {

		setFilter(options);

	} else if (_command == "nofilter") {

		setNoFilter();
	} else if (_command == "reset" && _resetCommandEnabled) {

		debugPrintln("* Reset ...");

		debugPrintln("* Closing client connection ...");

#if defined(ESP8266)
		debugPrintln("* Resetting the ESP8266 ...");
#elif defined(ESP32)
		debugPrintln("* Resetting the ESP32 ...");
#endif

		TelnetClient.stop();
		TelnetServer.stop();

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
		DebugWS.stop();
#endif

		delay(500);

		// Reset

		ESP.restart();

#ifdef DEBUGGER_ENABLED

	} else if (!_callbackDbgProcessCmd && _command.startsWith("dbg")) {

		// Show a message of debugger not is active

		debugPrintln("* RemoteDebugger not activate for this project");
		debugPrintln("* Please access it to see how activate this:");
		debugPrintln("* https://github.com/JoaoLopesF/RemoteDebugger");

#endif

	} else {

		// Callbacks

#ifdef DEBUGGER_ENABLED
		// Process commands for the debugger

		if (_callbackDbgProcessCmd) {
			_callbackDbgProcessCmd();
		}
#endif

		// Project commands - set by programmer

		if (_callbackProjectCmds) {

			_callbackProjectCmds();

		}
	}
}

// Filter

void RemoteDebug::setFilter(String filter) {

	_filter = filter;
	_filter.toLowerCase(); // TODO: option to case insensitive ?
	_filterActive = true;

	debugPrint("* Debug: Filter active: ");
	debugPrintln(_filter);

}

void RemoteDebug::setNoFilter() {

	_filter = "";
	_filterActive = false;

	debugPrintln("* Debug: Filter disabled");

}

// Silence

void RemoteDebug::silence(boolean activate, boolean showMessage, boolean fromBreak, uint32_t timeout) {

	// Set silence and timeout

	if (showMessage) {

		if (activate) {

			debugPrintln("* Debug now is in silent mode!");
#ifndef WEBSOCKET_DISABLED // For web socket server (app)
			if (_connectedWS) {
				debugPrintln("* Press button \"Silence\" or another command to return show debugs");
			} else {
				debugPrintln("* Press s again or another command to return show debugs");
			}
#else
			debugPrintln("* Press s again or another command to return show debugs");
#endif
		} else {

			debugPrintln("* Debug now exit from silent mode!");
		}
	}

	// Set it

	_silence = activate;
	_silenceTimeout = (timeout == 0)?0:(millis() + timeout);

#ifndef WEBSOCKET_DISABLED // For web socket server (app)

	// Send status to app

	if (_connectedWS) {
		DebugWS.printf("$app:S:%c\n", ((_silence)? '1':'0'));
	}

#endif

}

boolean RemoteDebug::isSilence() {

	return _silence;
}

// Format numbers

String RemoteDebug::formatNumber(uint32_t value, uint8_t size, char insert) {

	// Putting zeroes in left

	String ret = "";

	for (uint8_t i = 1; i <= size; i++) {
		uint32_t max = pow(10, i);
		if (value < max) {
			for (uint8_t j = (size - i); j > 0; j--) {
				ret.concat(insert);
			}
			break;
		}
	}

	ret.concat(value);

	return ret;
}

#ifndef WEBSOCKET_DISABLED // For web socket server (app)

/////// Web socket routines

// Process user command over telnet or web socket

void RemoteDebug::wsOnReceive(const char* command) { // @suppress("Unused function declaration")

	// Process the command

	_command = command;
	_lastCommand = _command; // Store the last command

	D("cmd: %s", command);

	// Is app commands

	if (_command == "$app") {

		// RemoteDebug connected, send info

		wsSendInfo ();

	} else { // Normal commands

		processCommand();
	}
}

// Send info to RemoteDebugApp

void RemoteDebug::wsSendInfo() {

	// Send version, board, debugger disabled and  if is low or enough memory board

	char features;
	char dbgEnabled;

	// Not connected ?

	if (!_connectedWS) {
		return;
	}

	// Features

#ifndef DEBUGGER_ENABLED
		features = 'M'; // Disabled
		dbgEnabled = 'D';
#else
		if (_callbackDbgProcessCmd) {
			features = 'E'; // Enough
			dbgEnabled = 'E';
		} else {
			features = 'M'; // Medium
			dbgEnabled = 'D';
		}
#endif

		// Send info

		String version = String(VERSION);
		String board;

#ifdef ESP32
		board = "ESP32";
#else
		board = "ESP8266";
#endif

		DebugWS.println(); // Workaround to not get dirty "[0m" ???
		DebugWS.printf("$app:V:%s:%s:%c:%lu:%c:N\n", version.c_str(), board.c_str(), features, getFreeMemory(), dbgEnabled);

		// Status of debug level

		wsSendLevelInfo();

		// Send status of debugger

		// TODO: made it

}

void RemoteDebug::wsSendLevelInfo() {

	// Send debug level info to app

	if (_connectedWS) {
		DebugWS.printf("$app:L:%u\n", _clientDebugLevel);
	}
}

#endif // WEBSOCKET_DISABLED

boolean RemoteDebug::wsIsConnected() {

	// Web socket is connected (RemoteDebugApp)

#ifndef WEBSOCKET_DISABLED // For web socket server (app)
	return _connectedWS;
#else
	return false;
#endif
}

/////// Utilities

// Get free memory

uint32_t RemoteDebug::getFreeMemory() {

	return ESP.getFreeHeap();

}

// Is CR or LF ?

boolean RemoteDebug::isCRLF(char character) {

	return (character == '\r' || character == '\n');

}

// Expand characters as CR/LF to \\r, \\n
// TODO: make this for another chars not printable

String RemoteDebug::expand(String string) {

	string.replace("\r", "\\r");
	string.replace("\n", "\\n");

	return string;
}

#ifdef ALPHA_VERSION // In test, not good yet
// Send telnet commands (as used with password request) - 18/08/18
// Experimental code !

void RemoteDebug::sendTelnetCommand(uint8_t command, uint8_t option) {

	// Send a command to the telnet client

	debugPrintf("%c%c%c", TELNET_IAC, command, option);
	TelnetClient.flush();
}
#endif

#else // DEBUG_DISABLED

/////// All debug is disabled, this include is to define empty debug macros

#include "RemoteDebug.h"		// This library

#endif // DEBUG_DISABLED

/////// End
