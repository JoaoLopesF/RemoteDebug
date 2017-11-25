////////
// Libraries Arduino
//
// Library: Remote debug - debug over telnet - for Esp8266 (NodeMCU) or ESP32
// Author: Joao Lopes
// Tanks: Example of TelnetServer code in http://www.rudiswiki.de/wiki9/WiFiTelnetServer
//
// Versions:
//    - 0.9.0 Beta 1 - August 2016
//    - 0.9.1 Beta 2 - Octuber 2016
//    - 1.0.0 RC - January 2017
//	  - 1.0.1 New connection logic - August 2017
//	  - 	  New level -> profiler and auto-profiler
//            New commands for CPU frequencies
//    - 1.1.0 Support to ESP32 - August 2017
//    - 1.1.1 Added support for the pass through of commands, and default debug levels - 11/24/2017 - B. Harville
//	  - 1.2.0 Added shortcuts and buffering to avoid delays
//  TODO: - Page HTML for begin/stop Telnet server
//        - Authentications
///////

#define VERSION "1.2.0"

#include <Arduino.h>

#include "RemoteDebug.h"

// Telnet server

WiFiServer telnetServer(TELNET_PORT);
WiFiClient telnetClient;

// Initialize the telnet server

void RemoteDebug::begin(String hostName, uint8_t startingDebugLevel) {

	// Initialize server telnet

	telnetServer.begin();
	telnetServer.setNoDelay(true);

	// Reserve space to buffer of print writes

	_bufferPrint.reserve(BUFFER_PRINT);

#ifdef CLIENT_BUFFERING
	// Reserve space to buffer of send

	_bufferPrint.reserve(MAX_SIZE_SEND);

#endif

	// Host name of this device

	_hostName = hostName;
	_clientDebugLevel = startingDebugLevel;
	_lastDebugLevel = startingDebugLevel;
}

// Stop the server

void RemoteDebug::stop() {

	// Stop Client

	if (telnetClient && telnetClient.connected()) {
		telnetClient.stop();
	}

	// Stop server

	telnetServer.stop();
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
				telnetClient.println("* Debug level profile inactive now");
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
				telnetClient.printf("* Debug level profile active now - time between handels: %u\r\n", diff);
			}
		}

		lastTime = millis();
	}
#endif

	// look for Client connect trial

	if (telnetServer.hasClient()) {

		// Old connection logic

//      if (!telnetClient || !telnetClient.connected()) {
//
//        if (telnetClient) { // Close the last connect - only one supported
//
//          telnetClient.stop();
//
//        }

		// New connection logic - 10/08/17

		if (telnetClient && telnetClient.connected()) {

			// Verify if the IP is same than actual conection

			WiFiClient newClient;
			newClient = telnetServer.available();
			String ip = newClient.remoteIP().toString();

			if (ip == telnetClient.remoteIP().toString()) {

				// Reconnect

				telnetClient.stop();
				telnetClient = newClient;

			} else {

				// Desconnect (not allow more than one connection)

				newClient.stop();

				return;

			}

		} else {

			// New TCP client

			telnetClient = telnetServer.available();

		}

		if (!telnetClient) { // No client yet ???
			return;
		}

		// Set client

		telnetClient.setNoDelay(true); // More faster
		telnetClient.flush(); // clear input buffer, else you get strange characters

		_bufferPrint = "";			// Clean buffer

		_lastTimeCommand = millis(); // To mark time for inactivity

		_command = "";				// Clear command
		_lastCommand = "";			// Clear las command

		_lastTimePrint = millis();	// Clear the time

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

		while (telnetClient.available()) {
			telnetClient.read();
		}

	}

	// Is client connected ? (to reduce overhead in active)

	_connected = (telnetClient && telnetClient.connected());

	// Get command over telnet

	if (_connected) {

		char last = ' '; // To avoid process two times the "\r\n"

		while (telnetClient.available()) {  // get data from Client

			// Get character

			char character = telnetClient.read();

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
			telnetClient.print(_bufferSend);
			_bufferSend = "";
			_sizeBufferSend = 0;
			_lastTimeSend = millis();
		}
#endif

#ifdef MAX_TIME_INACTIVE

		// Inactivit - close connection if not received commands from user in telnet
		// For reduce overheads

		if ((millis() - _lastTimeCommand) > MAX_TIME_INACTIVE) {
			telnetClient.println("* Closing session by inactivity");
			telnetClient.stop();
			_connected = false;
		}
#endif

	}
//DV("*handle time: ", (millis() - timeBegin));
}

// Send to serial too  (not recommended)

void RemoteDebug::setSerialEnabled(boolean enable) {
	_serialEnabled = enable;
	_showColors = false; // Desativa isto para Serial
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
		_showColors = false; // Desativa isto para Serial
	}
}

// Is active ? client telnet connected and level of debug equal or greater then setted by user in telnet

boolean RemoteDebug::isActive(uint8_t debugLevel) {

	// Active -> Debug level ok and
	//           Telnet connected or
	//           Serial enabled (not recommended)

	boolean ret = (debugLevel >= _clientDebugLevel
			&& (_connected || _serialEnabled));

	if (ret) {
		_lastDebugLevel = debugLevel;
	}

	return ret;

}

// Set help for commands over telnet setted by sketch

void RemoteDebug::setHelpProjectsCmds(String help) {

	_helpProjectCmds = help;

}

// Set callback of sketch function to process project messages

void RemoteDebug::setCallBackProjectCmds(void (*callback)()) {
	_callbackProjectCmds = callback;
}

// Print

size_t RemoteDebug::write(uint8_t character) {

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
					show = "v";
					break;
				case DEBUG:
					show = "d";
					break;
				case INFO:
					show = "i";
					break;
				case WARNING:
					show = "w";
					break;
				case ERROR:
					show = "e";
					break;
				}
			} else {
				switch (_lastDebugLevel) {
				case PROFILER:
					show = "P";
					break;
				case VERBOSE:
					show = "v";
					break;
				case DEBUG:
					show = COLOR_BACKGROUND_GREEN;
					show.concat("d");
					break;
				case INFO:
					show = COLOR_BACKGROUND_WHITE;
					show.concat("i");
					break;
				case WARNING:
					show = COLOR_BACKGROUND_YELLOW;
					show.concat("w");
					break;
				case ERROR:
					show = COLOR_BACKGROUND_RED;
					show.concat("e");
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

			// Send to telnet buffered

			if (_connected) {  // send data to Client
#ifndef CLIENT_BUFFERING
				telnetClient.print(_bufferPrint);
#else // Cliente buffering

				uint8_t size = _bufferPrint.length();

				// Buffer too big ?

				if ((_sizeBufferSend + size) >= MAX_SIZE_SEND) {

					// Send it

					telnetClient.print(_bufferSend);
					_bufferSend = "";
					_sizeBufferSend = 0;
					_lastTimeSend = millis();
				}

				// Add to buffer of send

				_bufferSend.concat(_bufferPrint);
				_sizeBufferSend+=size;

				// Client buffering - send data in intervals to avoid delays or if its is too big

				if ((millis() - _lastTimeSend) >= DELAY_TO_SEND) {
					telnetClient.print(_bufferSend);
					_bufferSend = "";
					_sizeBufferSend = 0;
					_lastTimeSend = millis();
				}
#endif
			}

			if (_serialEnabled) { // Echo to serial (not buffering it)
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
#endif
	if (_resetCommandEnabled) {
		help.concat("    reset -> reset the ESP8266\r\n");
	}

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

	telnetClient.print(help);
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

	telnetClient.print("* Debug: Command recevied: ");
	telnetClient.println(_command);

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

		telnetClient.println("* Closing telnet connection ...");

		telnetClient.stop();

	} else if (_command == "m") {

		telnetClient.print("* Free Heap RAM: ");
		telnetClient.println(ESP.getFreeHeap());

#if defined(ESP8266)

	} else if (_command == "cpu80") {

		// Change ESP8266 CPU para 80 MHz

		system_update_cpu_freq(80);
		telnetClient.println("CPU ESP8266 changed to: 80 MHz");

	} else if (_command == "cpu160") {

		// Change ESP8266 CPU para 160 MHz

		system_update_cpu_freq(160);
		telnetClient.println("CPU ESP8266 changed to: 160 MHz");

#endif

	} else if (_command == "v") {

		// Debug level

		_clientDebugLevel = VERBOSE;

		telnetClient.println("* Debug level setted to Verbose");

	} else if (_command == "d") {

		// Debug level

		_clientDebugLevel = DEBUG;

		telnetClient.println("* Debug level setted to Debug");

	} else if (_command == "i") {

		// Debug level

		_clientDebugLevel = INFO;

		telnetClient.println("* Debug level setted to Info");

	} else if (_command == "w") {

		// Debug level

		_clientDebugLevel = WARNING;

		telnetClient.println("* Debug level setted to Warning");

	} else if (_command == "e") {

		// Debug level

		_clientDebugLevel = ERROR;

		telnetClient.println("* Debug level setted to Error");

	} else if (_command == "l") {

		// Show debug level

		_showDebugLevel = !_showDebugLevel;

		telnetClient.printf("* Show debug level: %s\r\n",
				(_showDebugLevel) ? "On" : "Off");

	} else if (_command == "t") {

		// Show time

		_showTime = !_showTime;

		telnetClient.printf("* Show time: %s\r\n", (_showTime) ? "On" : "Off");

	} else if (_command == "p") {

		// Show profiler

		_showProfiler = !_showProfiler;
		_minTimeShowProfiler = 0;

		telnetClient.printf("* Show profiler: %s\r\n",
				(_showProfiler) ? "On" : "Off");

	} else if (_command.startsWith("p ")) {

		// Show profiler with minimal time

		if (options.length() > 0) { // With minimal time
			int32_t aux = options.toInt();
			if (aux > 0) { // Valid number
				_showProfiler = true;
				_minTimeShowProfiler = aux;
				telnetClient.printf(
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

		telnetClient.printf(
				"* Debug level setted to Profiler (disable in %u millis)\r\n",
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

		telnetClient.printf(
				"* Auto profiler debug level active (time >= %u millis)\r\n",
				_autoLevelProfiler);

	} else if (_command == "c") {

		// Show colors

		_showColors = !_showColors;

		telnetClient.printf("* Show colors: %s\r\n",
				(_showColors) ? "On" : "Off");

	} else if (_command.startsWith("filter ") && options.length() > 0) {

		setFilter(options);

	} else if (_command == "nofilter") {

		setNoFilter();
	} else if (_command == "reset" && _resetCommandEnabled) {

		telnetClient.println("* Reset ...");

		telnetClient.println("* Closing telnet connection ...");

#if defined(ESP8266)
		telnetClient.println("* Resetting the ESP8266 ...");
#elif defined(ESP32)
		telnetClient.println("* Resetting the ESP32 ...");
#endif

		telnetClient.stop();
		telnetServer.stop();

		delay(500);

		// Reset

		ESP.restart();

	} else {

		// Project commands - setted by programmer

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

	telnetClient.print("* Debug: Filter active: ");
	telnetClient.println(_filter);

}

void RemoteDebug::setNoFilter() {

	_filter = "";
	_filterActive = false;

	telnetClient.println("* Debug: Filter disabled");

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
/////// End
