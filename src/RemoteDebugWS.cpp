
/*
 * Libraries Arduino
 * *****************
 * Library : Remote debug - debug over telnet - for Esp8266 (NodeMCU) or ESP32
 * Author  : Joao Lopes
 * File    : RemoteDebugWS - web socket server to RemoteDebugAapp
 * Comments: Web socket server uses the arduinWebSockets library (https://github.com/Links2004/arduinoWebSockets)
 *           The author uses Eclise IDE (sloeber) to made this source
 * License : See RemoteDebugWS.h
 *
 * Versions:
 *  ------	----------	-----------------
 *  0.2.0	2019-03-19  All public configurations (#defines) have moved to RemoteDebugCfg.h, to facilitate changes for anybody.
 *  					Several adjustments.
 *  0.1.0	2019-03-11	Fist version
 *
 */

/* TODO:
 */

///// RemoteDebug configuration

#include "RemoteDebugCfg.h"

// Debug enabled ?

#ifndef DEBUG_DISABLED

/////// Includes

#include "RemoteDebugWS.h"

// Only if web socket (RemoteDebugApp) is enabled

#ifndef WEBSOCKET_DISABLED

#include "Arduino.h"

#ifdef USE_LIB_WEBSOCKET // This library already installed

#include <WebSockets.h>			// https://github.com/Links2004/arduinoWebSockets
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#else // local copy

#include "utility/WebSockets.h"			// https://github.com/Links2004/arduinoWebSockets
#include "utility/WebSocketsClient.h"
#include "utility/WebSocketsServer.h"

#endif

#include "RemoteDebug.h"

/////// Defines

// Version

#define REMOTEDEBUGWS_VERSION "0.1.1"

// Internal debug macro - recommended stay disable

#define D(fmt, ...) 													// Without this
//#define D(fmt, ...) Serial.printf("rdws: " fmt "\n", ##__VA_ARGS__) 	// Serial debug

/////// Variables

// Arduino websocket server (to comunicate with RemoteDebugApp)

static WebSocketsServer WebSocketServer(WEBSOCKET_PORT);    	// Websocket server on port 81

// Variables used by webSocketEvent

static int8_t _webSocketConnected = WS_NOT_CONNECTED;	// Client is connected ?

static RemoteDebugWSCallbacks* _callbacks; // callbacks to RemoteDebug

////// Methods

// Initialize the socket server

void RemoteDebugWS::begin(RemoteDebugWSCallbacks* callbacks) {

	// Initialize web socket (RemoteDebugApp)

	WebSocketServer.begin();                 // start the websocket server
	WebSocketServer.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'

	// Set callbacks

	_callbacks = callbacks;

	// Debug

	D("socket server started");

}

// Finalize the socket server

void RemoteDebugWS::stop() {

	// Finalize web socket (RemoteDebugApp)

	WebSocketServer.close();

	_webSocketConnected = WS_NOT_CONNECTED;

}

void RemoteDebugWS::disconnectAllClients() {

	// Disconnect all clients

	WebSocketServer.disconnect();

	// Disconnected

	_webSocketConnected = WS_NOT_CONNECTED;

	// Callback

	if (_callbacks) {
		_callbacks->onDisconnect();
	}

}

void RemoteDebugWS::disconnect() {

	// Disconnect actual clients

	if (_webSocketConnected != WS_NOT_CONNECTED) {

		WebSocketServer.disconnect(_webSocketConnected);

		// Disconnected

		_webSocketConnected = WS_NOT_CONNECTED;

		// Callback

		if (_callbacks) {
			_callbacks->onDisconnect();
		}
	}
}

// Handle

void RemoteDebugWS::handle() {

	WebSocketServer.loop();

//	// Test
//
//	if (_webSocketConnected) {
//		WebSocketServer.sendTXT(_webSocketConnected, "AAAAAAAAA\n");
//	}

}

// Is connected ?

boolean RemoteDebugWS::isConnected() {
	return (_webSocketConnected != WS_NOT_CONNECTED);
}

// Print

size_t RemoteDebugWS::write(const uint8_t *buffer, size_t size) {

	// Process buffer

	for(size_t i=0; i<size; i++) {
		write((uint8_t) buffer[i]);
	}

	return size;
}

size_t RemoteDebugWS::write(uint8_t character) {

	static String buffer = "";

	size_t ret = 0;

	// Process

	if (character == '\n') {

		if (_webSocketConnected != WS_NOT_CONNECTED) {

			D("write send buf[%u]: %s", buffer.length(), buffer.c_str());

			WebSocketServer.sendTXT(_webSocketConnected, buffer.c_str(), buffer.length());
		}

		// Empty the buffer

		ret = buffer.length();
		buffer = "";

	} else if (character != '\r' && isPrintable(character)) {

		// Append

		buffer.concat((char)character);

	}

	// Return

	return ret;
}

// Destructor

RemoteDebugWS::~RemoteDebugWS() {

	// Stop

	stop();
}


/////// WebSocket to comunicate with RemoteDebugApp

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t payloadlength) { // When a WebSocket message is received

	int blk_count = 0;
	char ipaddr[26];
	IPAddress localip;

	switch (type) {
		case WStype_DISCONNECTED:             // if the websocket is disconnected

			D("[%u] Disconnected!", num);

			// Disconnected

			_webSocketConnected = WS_NOT_CONNECTED;

			// Callback

			if (_callbacks) {
				_callbacks->onDisconnect();
			}

			break;
		case WStype_CONNECTED:               // if a new websocket connection is established
		{
#ifdef D // serial debug
			IPAddress ip = WebSocketServer.remoteIP(num);
			D("[%u] Connected from %d.%d.%d.%d url: %s", num, ip[0], ip[1], ip[2], ip[3],
					payload);
#endif

			// Have in another connection
			// One connection to reduce overheads

			if (num != _webSocketConnected) {

				WebSocketServer.sendTXT(_webSocketConnected, "* Closing client connection ...");
				WebSocketServer.disconnect(_webSocketConnected);

			}

			// Set as connected

			_webSocketConnected = num;

			// Send initial message

			WebSocketServer.sendTXT(_webSocketConnected, "$app:I");

			// Callback

			if (_callbacks) {
				_callbacks->onConnect();
			}
		}
			break;

		case WStype_TEXT:                     // if new text data is received
		{

//			if (payloadlength == 0) {
//				return;
//			}

			D("[%u] get Text: %s", num, payload);

			// Callback

			if (_callbacks) {
				_callbacks->onReceive((const char*)payload);
			}

		}
			break;

		case WStype_ERROR:                     // if new text data is received
			D("Error");
			// Disconnected

			_webSocketConnected = WS_NOT_CONNECTED;

			// Callback

			if (_callbacks) {
				_callbacks->onDisconnect();
			}

			break;
		default:
			// Disconnected

			_webSocketConnected = WS_NOT_CONNECTED;

			// Callback

			if (_callbacks) {
				_callbacks->onDisconnect();
			}

			D("WStype %x not handled ", type);
	}
}

#endif // WEBSOCKET_DISABLED

#endif // DEBUG_DISABLED

/// End

