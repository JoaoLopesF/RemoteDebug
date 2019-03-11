/*
 * Header for RemoteDebugWS
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


#ifndef REMOTEDEBUGWS_H_
#define REMOTEDEBUGWS_H_

#ifndef WEBSOCKET_DISABLED // Only if Web socket enabled (RemoteDebugApp)

///////// Defines

// Websocket port, if is not defined

#ifndef WEBSOCKET_PORT
#define WEBSOCKET_PORT 8232
#endif

// Library arduinoWebSockets already installed, uncomment to use it
// Do this if you receive errors of multiple definition ...
//#define USE_LIB_WEBSOCKET true

// Connected

#define WS_NOT_CONNECTED -1 // Not connected

//////// Includes

#ifdef USE_LIB_WEBSOCKET // This library already installed

#include <WebSockets.h>			// https://github.com/Links2004/arduinoWebSockets
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#else // local copy

#include "utility/WebSockets.h"			// https://github.com/Links2004/arduinoWebSockets
#include "utility/WebSocketsClient.h"
#include "utility/WebSocketsServer.h"

#endif

///// Callbacks class - based in Kolban BLE callback example code

class RemoteDebugWSCallbacks {

public:
	virtual ~RemoteDebugWSCallbacks() {}
	virtual void onConnect() = 0;
	virtual void onDisconnect() = 0;
	virtual void onReceive(const char* message) = 0;

};

///// Main Class

class RemoteDebugWS: public Print
{
	public:

	// Constructor

	//RemoteDebugWS();

	// Methods

	void begin(RemoteDebugWSCallbacks* callbacks);

	void stop();

	void disconnectAllClients();

	void disconnect();

	boolean isConnected();

	void handle();

	// Print

	virtual size_t write(uint8_t);

    virtual size_t write(const uint8_t *buffer, size_t size);

	// Destructor

	~RemoteDebugWS();

private:

	//////// Variables

	//////// Privates

};

/////// Prototypes

// Websocket (RemoteDebugApp)
// Note: in is out of class, do not cause errors on onEvent
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t payloadlength);

#endif // WEBSOCKET_DISABLED

#endif /* REMOTEDEBUGWS_H_ */
