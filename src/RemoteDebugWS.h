/*
 * Header for RemoteDebugWS
 *
 * Copyright (C) 2018  Joao Lopes
 * MIT License
 *
 * Copyright (c) 2019 Joao Lopes https://github.com/JoaoLopesF/RemoteDebug
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


#ifndef REMOTEDEBUGWS_H_
#define REMOTEDEBUGWS_H_
#pragma once

///// RemoteDebug configuration

#include "RemoteDebugCfg.h"

// Debug enabled ?

#ifndef DEBUG_DISABLED

// Only if Web socket enabled (RemoteDebugApp)

#ifndef WEBSOCKET_DISABLED

///////// Defines

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

#else

#error "aaaah"

#endif // DEBUG_DISABLED

#endif /* REMOTEDEBUGWS_H_ */

///// End
