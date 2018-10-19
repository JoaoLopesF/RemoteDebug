/*
 * Libraries Arduino
 * *****************
 * Library : Remote debug - debug over telnet - for Esp8266 (NodeMCU) or ESP32
 * Author  : Joao Lopes
 * Comments: Based on example of TelnetServer code in http: *www.rudiswiki.de/wiki9/WiFiTelnetServer
 * License : See RemoteDebug.h
 *
 * Versions:
 *    - 0.9.0 Beta 1 - August 2016
 *    - 0.9.1 Beta 2 - Octuber 2016
 *    - 1.0.0 RC - January 2017
 *	  - 1.0.1 New connection logic - August 2017
 *	  - 	  New level -> profiler and auto-profiler
 *            New commands for CPU frequencies
 *    - 1.1.0 Support to ESP32 - August 2017
 *    - 1.1.1 Added support for the pass through of commands, and default debug levels - 11/24/2017 - B. Harville
 *	  - 1.2.0 Added shortcuts and buffering to avoid delays
 *	  - 1.2.1 Adjusts to not cause error in Arduino
 *    - 1.3.0 Bug in write with latest ESP8266 SDK - August 2018
 *    	      Port number can be modified in project Arduino (.ino file)
 *    		  Few adjustments as ESP32 includes
 *    - 1.3.1 Retired # from VARGS precompiler macros
 *    - 1.4.0 A simple text password request, if enabled (thanks @jeroenst for suggestion)
 *        		Note: telnet use advanced authentication (kerberos, etc.)
 *        		Such as RemoteDebug now is not for production releases,
 *       		this kind of authentication will not be done now.
 *       	  Few adjustments
 *    - 1.5.0 Port can be pass in begin method (thanks @PjotrekSE for suggestion)
 *    		  Class destructor implemented
 *    		  Auto function and core if (for ESP32) in rdebug macros
 *    		  Added new rdebug?ln to put auto new line
 *    - 1.5.1 New command: silence 
 *    - 1.5.2 Correct rdebug macro (thanks @stritti)
 *    - 1.5.3 Serial output adjustments (due bug in password logic)
 *    - 1.5.4 Serial output not depending of telnet password (thanks @jeroenst for suggestion)
 *    - 1.5.5 Serial output is now not allowed if telnet password is enabled
 *    - 1.5.6 Adjustments based on pull request from @jeroenst (to allow serial output with telnet password and setPassword method) - 2018-10-19
 *
 */

/*
 *  TODO: 	- Page HTML for begin/stop Telnet server
 */

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

// ESP8266 or ESP32 ?

#if defined(ESP8266)

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#elif defined(ESP32)

#include <WiFi.h>

#else

#error Only for ESP8266 or ESP32

#endif

#define VERSION "1.5.6"

#include <Arduino.h>

#include "RemoteDebug.h"

#ifdef ALPHA_VERSION // In test, not good yet
#include "telnet.h"
#endif

////// Variables

// Telnet server

WiFiServer TelnetServer(TELNET_PORT);
WiFiClient TelnetClient;

// Initialize the telnet server

void RemoteDebug::begin(String hostName, uint8_t startingDebugLevel) {
	begin(hostName, TELNET_PORT, startingDebugLevel);
}

void RemoteDebug::begin(String hostName, uint16_t port,  uint8_t startingDebugLevel) {

	// Initialize server telnet

	TelnetServer.begin(port);
	TelnetServer.setNoDelay(true);

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

}

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
}

// Handle the connection (in begin of loop in sketch)

void RemoteDebug::handle() {

#ifdef ALPHA_VERSION // In test, not good yet
	static uint32_t lastTime = millis();
#endif

	// Debug level is profiler -> set the level before

	if (_clientDebugLevel == PROFILER) {
		if (millis() > _levelProfilerDisable) {
			_clientDebugLevel = _levelBeforeProfiler;
			if (_connected) {
				TelnetClient.println("* Debug level profile inactive now");
			}
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
			if (_connected) {
				TelnetClient.printf("* Debug level profile active now - time between handels: %u\r\n", diff);
			}
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

			WiFiClient newClient;
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

				_passwordOk = false;

	#ifdef REMOTEDEBUG_PWD_ATTEMPTS
				_passwordAttempt = 1;
	#endif

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

		_bufferPrint = "";			// Clean buffer

		_lastTimeCommand = millis(); // To mark time for inactivity

		_command = "";				// Clear command
		_lastCommand = "";			// Clear las command

		_lastTimePrint = millis();	// Clear the time

		_silence = false;			// No silence 

		// Show the initial message

		showHelp();

#ifdef CLIENT_BUFFERING
		// Client buffering - send data in intervals to avoid delays or if its is too big
		_bufferSend = "";
		_sizeBufferSend = 0;
		_lastTimeSend = millis();
#endif

		// Empty buffer in

		delay(100);

		while (TelnetClient.available()) {
			TelnetClient.read();
		}

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

#ifdef CLIENT_BUFFERING
		// Client buffering - send data in intervals to avoid delays or if its is too big

		if ((millis() - _lastTimeSend) >= DELAY_TO_SEND || _sizeBufferSend >= MAX_SIZE_SEND) {
			TelnetClient.print(_bufferSend);
			_bufferSend = "";
			_sizeBufferSend = 0;
			_lastTimeSend = millis();
		}
#endif

#ifdef MAX_TIME_INACTIVE

		// Inactivity - close connection if not received commands from user in telnet
		// For reduce overheads

		uint32_t maxTime = MAX_TIME_INACTIVE; // Normal

		if (_password != "") { // Request password - 18/08/08
			maxTime = 60000; // One minute to password
		}

		if ((millis() - _lastTimeCommand) > maxTime) {
			TelnetClient.println("* Closing session by inactivity");
			TelnetClient.stop();
			_connected = false;
			_silence = false;
		}
#endif

	}

	//DV("*handle time: ", (millis() - timeBegin));
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

// Is active ? client telnet connected and level of debug equal or greater then set by user in telnet

boolean RemoteDebug::isActive(uint8_t debugLevel) {

	// Active -> Not in silence (new)
	// 			 Debug level ok and
	//           Telnet connected or
	//           Serial enabled (use only if need)
	//			 Password ok (if enabled) - 18/08/18

	boolean ret = (debugLevel >= _clientDebugLevel &&
					(_connected || _serialEnabled));

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

	// New line writted before ?

	if (_newLine) {

		String show = "";

		// Show debug level

		if (_showDebugLevel) {
			if (_showColors == false) {
				switch (_lastDebugLevel) {
				case PROFILER:
					show = "P";
					break;
				case VERBOSE:
					show = "V";
					break;
				case DEBUG:
					show = "D";
					break;
				case INFO:
					show = "I";
					break;
				case WARNING:
					show = "W";
					break;
				case ERROR:
					show = "E";
					break;
				}
			} else {
				switch (_lastDebugLevel) {
				case PROFILER:
					show = "P";
					break;
				case VERBOSE:
					show = "V";
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
				} else if (elapsed < 3000) {
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

		if (show != "") {

			String send = "(";
			send.concat(show);
			send.concat(") ");

			// Write to telnet buffered

			if (_connected || _serialEnabled) {  // send data to Client
				_bufferPrint = send;
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

			// Send to telnet (buffered)

			boolean sendToTelnet = _connected;

			if (_password != "" && !_passwordOk) { // With no password -> no telnet output - 2018-10-19
				sendToTelnet = false;
			}

			if (sendToTelnet) {  // send data to Client


#ifndef CLIENT_BUFFERING
				TelnetClient.print(_bufferPrint);
#else // Cliente buffering

				uint8_t size = _bufferPrint.length();

				// Buffer too big ?

				if ((_sizeBufferSend + size) >= MAX_SIZE_SEND) {

					// Send it

					TelnetClient.print(_bufferSend);
					_bufferSend = "";
					_sizeBufferSend = 0;
					_lastTimeSend = millis();
				}

				// Add to buffer of send

				_bufferSend.concat(_bufferPrint);
				_sizeBufferSend+=size;

				// Client buffering - send data in intervals to avoid delays or if its is too big

				if ((millis() - _lastTimeSend) >= DELAY_TO_SEND) {
					TelnetClient.print(_bufferSend);
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

		TelnetClient.print(help);

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

	if (_helpProjectCmds != "" && (_callbackProjectCmds)) {
		help.concat("\r\n");
		help.concat("    * Project commands:\r\n");
		String show = "\r\n";
		show.concat(_helpProjectCmds);
		show.replace("\n", "\n    "); // ident this
		help.concat(show);
	}

	help.concat("\r\n");
	help.concat(
			"* Please type the command and press enter to execute.(? or h for this help)\r\n");
	help.concat("***\r\n");

	TelnetClient.print(help);
}

// Get last command received

String RemoteDebug::getLastCommand() {

	return _lastCommand;
}

// Clear the last command received

void RemoteDebug::clearLastCommand() {
	_lastCommand = "";
}

// Process user command over telnet

void RemoteDebug::processCommand() {

	// Password request ? - 18/07/18

	if (_password != "" && !_passwordOk) { // Process the password - 18/08/18 - adjust in 04/09/08 and 2018-10-19

		if (_command == _password) {

			TelnetClient.println("* Password ok, allowing access now...");

			_passwordOk = true;

#ifdef ALPHA_VERSION // In test, not good yet
			sendTelnetCommand(TELNET_WILL, TELNET_ECHO); // Send a command to telnet to restore echoes = 18/08/18
#endif
			showHelp();

		} else {

			TelnetClient.println("* Wrong password!");

	#ifdef REMOTEDEBUG_PWD_ATTEMPTS

			_passwordAttempt++;

			if (_passwordAttempt > REMOTEDEBUG_PWD_ATTEMPTS) {

				TelnetClient.println("* Many attempts. Closing session now.");
				TelnetClient.stop();
				_connected = false;
				_silence = false;

			} else {

				showHelp();
			}

	#endif
		}

		return;

	}

	// Process commands

	TelnetClient.print("* Debug: Command recevied: ");
	TelnetClient.println(_command);

	String options = "";
	uint8_t pos = _command.indexOf(" ");
	if (pos > 0) {
		options = _command.substring(pos + 1);
	}

	// Set time of last command received

	_lastTimeCommand = millis();

	// Process the command

	if (_command == "h" || _command == "?") {

		// Show help

		showHelp();

	} else if (_command == "q") {

		// Quit

		TelnetClient.println("* Closing telnet connection ...");

		TelnetClient.stop();

	} else if (_command == "m") {

		TelnetClient.print("* Free Heap RAM: ");
		TelnetClient.println(ESP.getFreeHeap());

#if defined(ESP8266)

	} else if (_command == "cpu80") {

		// Change ESP8266 CPU para 80 MHz

		system_update_cpu_freq(80);
		TelnetClient.println("CPU ESP8266 changed to: 80 MHz");

	} else if (_command == "cpu160") {

		// Change ESP8266 CPU para 160 MHz

		system_update_cpu_freq(160);
		TelnetClient.println("CPU ESP8266 changed to: 160 MHz");

#endif

	} else if (_command == "v") {

		// Debug level

		_clientDebugLevel = VERBOSE;

		TelnetClient.println("* Debug level set to Verbose");

	} else if (_command == "d") {

		// Debug level

		_clientDebugLevel = DEBUG;

		TelnetClient.println("* Debug level set to Debug");

	} else if (_command == "i") {

		// Debug level

		_clientDebugLevel = INFO;

		TelnetClient.println("* Debug level set to Info");

	} else if (_command == "w") {

		// Debug level

		_clientDebugLevel = WARNING;

		TelnetClient.println("* Debug level set to Warning");

	} else if (_command == "e") {

		// Debug level

		_clientDebugLevel = ERROR;

		TelnetClient.println("* Debug level set to Error");

	} else if (_command == "l") {

		// Show debug level

		_showDebugLevel = !_showDebugLevel;

		TelnetClient.printf("* Show debug level: %s\r\n",
				(_showDebugLevel) ? "On" : "Off");

	} else if (_command == "t") {

		// Show time

		_showTime = !_showTime;

		TelnetClient.printf("* Show time: %s\r\n", (_showTime) ? "On" : "Off");

	} else if (_command == "s") {

		// Silence (new) = 28/08/18

		_silence = !_silence;

		if (_silence) {

			TelnetClient.println("* Debug now is in silent mode!");
			TelnetClient.println("* Press s again to return show debugs");

		} else {

			TelnetClient.println("* Debug now exit from silent mode!");
		}

	} else if (_command == "p") {

		// Show profiler

		_showProfiler = !_showProfiler;
		_minTimeShowProfiler = 0;

		TelnetClient.printf("* Show profiler: %s\r\n",
				(_showProfiler) ? "On" : "Off");

	} else if (_command.startsWith("p ")) {

		// Show profiler with minimal time

		if (options.length() > 0) { // With minimal time
			int32_t aux = options.toInt();
			if (aux > 0) { // Valid number
				_showProfiler = true;
				_minTimeShowProfiler = aux;
				TelnetClient.printf(
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

		TelnetClient.printf(
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

		TelnetClient.printf(
				"* Auto profiler debug level active (time >= %u millis)\r\n",
				_autoLevelProfiler);

	} else if (_command == "c") {

		// Show colors

		_showColors = !_showColors;

		TelnetClient.printf("* Show colors: %s\r\n",
				(_showColors) ? "On" : "Off");

	} else if (_command.startsWith("filter ") && options.length() > 0) {

		setFilter(options);

	} else if (_command == "nofilter") {

		setNoFilter();
	} else if (_command == "reset" && _resetCommandEnabled) {

		TelnetClient.println("* Reset ...");

		TelnetClient.println("* Closing telnet connection ...");

#if defined(ESP8266)
		TelnetClient.println("* Resetting the ESP8266 ...");
#elif defined(ESP32)
		TelnetClient.println("* Resetting the ESP32 ...");
#endif

		TelnetClient.stop();
		TelnetServer.stop();

		delay(500);

		// Reset

		ESP.restart();

	} else {

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

	TelnetClient.print("* Debug: Filter active: ");
	TelnetClient.println(_filter);

}

void RemoteDebug::setNoFilter() {

	_filter = "";
	_filterActive = false;

	TelnetClient.println("* Debug: Filter disabled");

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

	TelnetClient.printf("%c%c%c", TELNET_IAC, command, option);
	TelnetClient.flush();
}
#endif

/////// End
