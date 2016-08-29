////////
// Libraries Arduino
//
// Library: Remote debug - debug over telnet - for Esp8266 (NodeMCU)
// Author: Joao Lopes
// Tanks: Example of TelnetServer code in http://www.rudiswiki.de/wiki9/WiFiTelnetServer
//
// Versions:
//    - 0.9.0 Beta 1 - August 2016
//
//  TODO: - Page HTML for begin/stop Telnet server
//        - Authentications
///////

#include <Arduino.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include "RemoteDebug.h"

// Telnet server

WiFiServer telnetServer(TELNET_PORT);
WiFiClient telnetClient;

#ifdef BUFFER_PRINT

// Buffer of print write to telnet

String bufferPrint = "";

#endif
// Initialize the telnet server

void RemoteDebug::begin (String hostName) {

    // Initialize server telnet

    telnetServer.begin();
    telnetServer.setNoDelay(true);

#ifdef BUFFER_PRINT
    // Reserv space to buffer of print writes
    bufferPrint.reserve(BUFFER_PRINT);
#endif

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

    // Is client connected ? (to reduce overhead in isAtive)

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

void RemoteDebug::showProfiler(boolean show) {
    _showProfiler = show;
}

// Show debug level

void RemoteDebug::showDebugLevel(boolean show) {
    _showProfiler = show;
}

// Is ative ? client telnet connected and level of debug equal or greater then setted by user in telnet

boolean RemoteDebug::ative(uint8_t debugLevel) {

    // Ative -> Debug level ok and
    //          Telnet connected or
    //          Serial enabled (not recommended)

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

    // New line writted before ?

    if (_newLine) {

        String show = "";

        // Show debug level

        if (_showDebugLevel) {
            switch (_lastDebugLevel) {
                case VERBOSE:   show = "v"; break;
                case DEBUG:     show = "d"; break;
                case INFO:      show = "i"; break;
                case WARNING:   show = "w"; break;
                case ERROR:     show = "e"; break;
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
            if (show != "")
                show.concat (" ");
            show.concat("p:^");
            show.concat (formatNumber((millis() - lastTime), 4));
            show.concat ("ms");
            lastTime = millis();
        }

        if (show != "") {

            String send = "(";
            send.concat(show);
            send.concat(") ");

#ifndef BUFFER_PRINT

            // Write not Buffered

            if (_connected) {  // send data to Client
                telnetClient.print(send);
            }

            if (_serialEnabled) { // Echo to serial
                Serial.print(send);
            }

#else
            // Write to telnet buffered

            if (_connected || _serialEnabled) {  // send data to Client
                bufferPrint = send;
            }
#endif
        }

        _newLine = false;

    }

    // Send the character by print.h

#ifndef BUFFER_PRINT

    // Write not Buffered

    if (_connected) {  // send data to Client
        telnetClient.write(character);
    }

    if (_serialEnabled) { // Echo to serial
        Serial.write(character);
    }

#else

    // Write to telnet Buffered

    bufferPrint.concat((char)character);

    if (character != '\n' &&
        bufferPrint.length() == BUFFER_PRINT) { // Limit of buffer

        if (_connected) {  // send data to Client
            telnetClient.print(bufferPrint);
        }

        if (_serialEnabled) { // Echo to serial
            Serial.print(bufferPrint);
        }

        bufferPrint = "";
    }
#endif

    // New line ?

    if (character == '\n') {

        _newLine = true;

#ifdef BUFFER_PRINT

        // Write to telnet Buffered

        if (_connected) {  // send data to Client
            telnetClient.print(bufferPrint);
        }

        if (_serialEnabled) { // Echo to serial
            Serial.print(bufferPrint);
        }

        bufferPrint = "";
#endif

    }

}

////// Private

// Show help of commands

void RemoteDebug::showHelp() {

    // Show the initial message

    telnetClient.println("*** Remote debug - over telnet - for ESP8266 (NodeMCU)");
    telnetClient.print("* Host name: ");
    telnetClient.print(_hostName);
    telnetClient.print(" (");
    telnetClient.print(WiFi.localIP());
    telnetClient.println(")");
    telnetClient.print("* Free Heap RAM: ");
    telnetClient.println(ESP.getFreeHeap());
    telnetClient.println("******************************************************");
    telnetClient.println("* Commands:");
    telnetClient.println("    ? or help -> display these help of commands");
    telnetClient.println("    q -> quit (close this connection)");
    telnetClient.println("    m -> display memory available");
    telnetClient.println("    v -> set debug level to verbose");
    telnetClient.println("    d -> set debug level to debug");
    telnetClient.println("    i -> set debug level to info");
    telnetClient.println("    w -> set debug level to warning");
    telnetClient.println("    e -> set debug level to errors");
    telnetClient.println("    l -> show debug level");
    telnetClient.println("    t -> show time (millis)");
    telnetClient.println("    p -> profiler - show time between actual and last message (in millis)");

    if (_resetCommandEnabled) {
        telnetClient.println("    reset -> reset the ESP8266");
    }

    if (_helpProjectCmds != "" && (_callbackProjectCmds)) {
        telnetClient.println("");
        telnetClient.println("    * Project commands:");
        String show = "\n";
        show.concat(_helpProjectCmds);
        show.replace("\n", "\n    "); // ident this
        telnetClient.println(show);
    }

    telnetClient.println();
    telnetClient.println("* Please type the command and press enter to execute.(? or h for this help)");
    telnetClient.println("***");

}

// Get last command received

String RemoteDebug::getLastCommand() {

    return _command;
}


// Process user command over telnet

void RemoteDebug::processCommand() {

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

        telnetClient.printf("* Show debug level: %s\n", (_showDebugLevel)?"On":"Off");

    } else if (_command == "t") {

        // Show time

        _showTime = !_showTime;

        telnetClient.printf("* Show time: %s\n", (_showTime)?"On":"Off");

    } else if (_command == "p") {

        // Show profiler

        _showProfiler = !_showProfiler;

        telnetClient.printf("* Show profiler: %s\n", (_showProfiler)?"On":"Off");

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
