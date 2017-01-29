////////
// Libraries Arduino
//
// Library: Remote debug - debug over telnet - for Esp8266 (NodeMCU)
// Author: Joao Lopes
// Tanks: Example of TelnetServer code in http://www.rudiswiki.de/wiki9/WiFiTelnetServer
//
// Versions:
//    - 0.9.0 Beta 1 - August 2016
//    - 0.9.1 Beta 2 - Octuber 2016
//    - 1.0.0 RC - January 2017
//
//  TODO: - Page HTML for begin/stop Telnet server
//        - Authentications
///////

#define VERSION "1.0.0"

#include <Arduino.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include "RemoteDebug.h"

// Telnet server

WiFiServer telnetServer(TELNET_PORT);
WiFiClient telnetClient;

// Buffer of print write to telnet

String bufferPrint = "";

// Initialize the telnet server

void RemoteDebug::begin (String hostName) {

    // Initialize server telnet

    telnetServer.begin();
    telnetServer.setNoDelay(true);

    // Reserve space to buffer of print writes

    bufferPrint.reserve(BUFFER_PRINT);

    // Host name of this device

    _hostName = hostName;
}

// Stop the server

void RemoteDebug::stop () {

    // Stop Client

    if (telnetClient && telnetClient.connected()) {
        telnetClient.stop();
    }

    // Stop server

    telnetServer.stop();
}

// Handle the connection (in begin of loop in sketch)

void RemoteDebug::handle() {

//uint32_t timeBegin = millis();

    // look for Client connect trial

    if (telnetServer.hasClient()) {

      if (!telnetClient || !telnetClient.connected()) {

        if (telnetClient) { // Close the last connect - only one supported

          telnetClient.stop();

        }

        // Get telnet client

        telnetClient = telnetServer.available();

        telnetClient.flush();  // clear input buffer, else you get strange characters

        _lastTimeCommand = millis(); // To mark time for inactivity

        // Show the initial message

        showHelp ();

        // Empty buffer in

        while(telnetClient.available()) {
            telnetClient.read();
        }

      }
    }

    // Is client connected ? (to reduce overhead in active)

    _connected = (telnetClient && telnetClient.connected());

    // Get command over telnet

    if (_connected) {

        while(telnetClient.available()) {  // get data from Client

            // Get character

            char character = telnetClient.read();

            // Newline or CR

            if (character == '\n' || character == '\r') {

                // Process the command

                if (_command.length() > 0) {

                    processCommand();

                }

                _command = ""; // Init it for next command

            } else if (isPrintable(character)) {

                // Concat

                _command.concat(character);

            }
        }

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

// Show debug level

void RemoteDebug::showDebugLevel(boolean show) {
    _showDebugLevel = show;
}

// Show colors

void RemoteDebug::showColors(boolean show) {
    _showColors = show;
}

// Is active ? client telnet connected and level of debug equal or greater then setted by user in telnet

boolean RemoteDebug::isActive(uint8_t debugLevel) {

    // Active -> Debug level ok and
    //           Telnet connected or
    //           Serial enabled (not recommended)

    boolean ret = (debugLevel >= _clientDebugLevel &&
                    (_connected || _serialEnabled));

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

    static uint32_t lastTime = millis();
    static uint32_t elapsed = 0;

    // New line writted before ?

    if (_newLine) {

        String show = "";

        // Show debug level

        if (_showDebugLevel) {
            if (_showColors == false) {
                switch (_lastDebugLevel) {
                    case VERBOSE:   show = "v"; break;
                    case DEBUG:     show = "d"; break;
                    case INFO:      show = "i"; break;
                    case WARNING:   show = "w"; break;
                    case ERROR:     show = "e"; break;
                }
            } else {
                switch (_lastDebugLevel) {
                    case VERBOSE:   show = "v"; break;
                    case DEBUG:     show = COLOR_BACKGROUND_GREEN; show.concat("d"); break;
                    case INFO:      show = COLOR_BACKGROUND_WHITE; show.concat("i"); break;
                    case WARNING:   show = COLOR_BACKGROUND_YELLOW; show.concat("w"); break;
                    case ERROR:     show = COLOR_BACKGROUND_RED; show.concat("e"); break;
                }
                if (show.length() > 1) {
                    show.concat(COLOR_RESET);
                }
            }
        }

        // Show time in millis

        if (_showTime) {
            if (show != "")
                show.concat (" ");
            show.concat ("t:");
            show.concat (millis());
            show.concat ("ms");
        }

        // Show profiler (time between messages)

        if (_showProfiler) {
            elapsed = (millis() - lastTime);
            boolean resetColors = false;
            if (show != "")
                show.concat (" ");
            if (_showColors) {
                if (elapsed < 250) {
                    ; // not color this
                } else if (elapsed < 1000) {
                    show.concat(COLOR_BACKGROUND_CYAN);
                    resetColors = true;
                } else if (elapsed < 3000) {
                    show.concat (COLOR_BACKGROUND_YELLOW);
                    resetColors = true;
                } else if (elapsed < 3000) {
                    show.concat (COLOR_BACKGROUND_MAGENTA);
                    resetColors = true;
                } else {
                    show.concat (COLOR_BACKGROUND_RED);
                    resetColors = true;
                }
            }
            show.concat("p:^");
            show.concat (formatNumber(elapsed, 4));
            show.concat ("ms");
            if (resetColors) {
                show.concat(COLOR_RESET);
            }
            lastTime = millis();
        }

        if (show != "") {

            String send = "(";
            send.concat(show);
            send.concat(") ");

            // Write to telnet buffered

            if (_connected || _serialEnabled) {  // send data to Client
                bufferPrint = send;
            }
        }

        _newLine = false;

    }

    // Print ?

    boolean doPrint = false;

    // New line ?

    if (character == '\n') {

        bufferPrint.concat("\r"); // Para clientes windows - 29/01/17

        _newLine = true;
        doPrint = true;

    } else if (bufferPrint.length() == BUFFER_PRINT) { // Limit of buffer

        doPrint = true;

    }

    // Write to telnet Buffered

    bufferPrint.concat((char)character);

    // Send the characters buffered by print.h

    if (doPrint) { // Print the buffer

        boolean noPrint = false;

        if (_showProfiler && elapsed < _minTimeShowProfiler) { // Profiler time Minimal
            noPrint = true;
        } else if (_filterActive) { // Check filter before print

            String aux = bufferPrint;
            aux.toLowerCase();

            if (aux.indexOf(_filter) == -1) { // not find -> no print
                noPrint = true;
            }
        }

        if (noPrint == false) {

            // Write to telnet Buffered

            if (_connected) {  // send data to Client
                telnetClient.print(bufferPrint);
            }

            if (_serialEnabled) { // Echo to serial
                Serial.print(bufferPrint);
            }
        }

        // Empty the buffer

        bufferPrint = "";
    }

}

////// Private

// Show help of commands

void RemoteDebug::showHelp() {

    // Show the initial message

    String help = "";

    help.concat("*** Remote debug - over telnet - for ESP8266 (NodeMCU) - version ");
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
    help.concat("      p       -> show time between actual and last message (in millis)\r\n");
    help.concat("      p min   -> show only if time is this minimal\r\n");
    help.concat("    c -> show colors\r\n");
    help.concat("    filter:\r\n");
    help.concat("          filter <string> -> show only debugs with this\r\n");
    help.concat("          nofilter        -> disable the filter\r\n");
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
    help.concat("* Please type the command and press enter to execute.(? or h for this help)\r\n");
    help.concat("***\r\n");

    telnetClient.print(help);
}

// Get last command received

String RemoteDebug::getLastCommand() {

    return _command;
}


// Process user command over telnet

void RemoteDebug::processCommand() {

    telnetClient.print("* Debug: Command recevied: ");
    telnetClient.println(_command);

    String options = "";
    uint8_t pos = _command.indexOf(" ");
    if (pos > 0) {
        options = _command.substring (pos+1);
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

        telnetClient.printf("* Show debug level: %s\r\n", (_showDebugLevel)?"On":"Off");

    } else if (_command == "t") {

        // Show time

        _showTime = !_showTime;

        telnetClient.printf("* Show time: %s\r\n", (_showTime)?"On":"Off");

    } else if (_command == "p") {

        // Show profiler

        _showProfiler = !_showProfiler;
        _minTimeShowProfiler = 0;

        telnetClient.printf("* Show profiler: %s\r\n", (_showProfiler)?"On":"Off");

    } else if (_command.startsWith("p ")) {

        // Show profiler with minimal time

        if (options.length() > 0) { // With minimal time
            int32_t aux = options.toInt();
            if (aux > 0) { // Valid number
                _showProfiler = true;
                _minTimeShowProfiler = aux;
                telnetClient.printf("* Show profiler: On (with minimal time: %u)\r\n", _minTimeShowProfiler);
            }
        }

    } else if (_command == "c") {

        // Show colors

        _showColors = !_showColors;

        telnetClient.printf("* Show colors: %s\r\n", (_showColors)?"On":"Off");

    } else if (_command.startsWith("filter ") && options.length() > 0) {

        setFilter(options);

    } else if (_command == "nofilter") {

        setNoFilter();
    } else if (_command == "reset" && _resetCommandEnabled) {

        telnetClient.println("* Reset ...");

        telnetClient.println("* Closing telnet connection ...");

        telnetClient.println("* Resetting the ESP8266 ...");

        telnetClient.stop();
        telnetServer.stop();

        delay (500);

        // Reset

        ESP.reset();

    } else  {

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

    for (uint8_t i=1; i<=size; i++) {
        uint32_t max = pow(10, i);
        if (value < max) {
            for (uint8_t j=(size - i); j>0; j--) {
                ret.concat(insert);
            }
            break;
        }
    }

    ret.concat(value);

    return ret;
}

/////// End
