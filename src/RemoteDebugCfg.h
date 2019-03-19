
/*
 * Header for RemoteDebugCfg
 *
 * MIT License
 *
 * Copyright (c) 2019 Joao Lopes
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
 *
 */

/*
 * All configurations have moved to RemoteDebugCfg.h,
 * to facilitate changes
 */

///////////// User config, to not lost in library updates

#ifndef REMOTEDEBUGCFG_H_
#define REMOTEDEBUGCFG_H_
#pragma once

///////////// For RemoteDebug ///////////////////

///// Debug disable for compile to production/release
///// as nothing of RemotedDebug is compiled, zero overhead :-)
//#define DEBUG_DISABLED true

// Debug enabled ?

#ifndef DEBUG_DISABLED

///// Port for telnet server
#define TELNET_PORT 23

// Disable auto function for debug macros? (uncomment this if not want this)
//#define DEBUG_DISABLE_AUTO_FUNC true

// Simple password request - left commented if not need this - 18/07/18
// Notes:
// It is very simple feature, only text, no cryptography,
// and the password is echoed in screen (I not discovery yet how disable it)
// telnet use advanced authentication (kerberos, etc.)
// Such now as RemoteDebug now is not for production releases,
// this kind of authentication will not be done now.
// Can be by project, just call setPassword method
#define REMOTEDEBUG_PWD_ATTEMPTS 3

// Maximum time for inactivity (em milliseconds)
// Default: 10 minutes
// Comment it if you not want this
// Can be by project, just define it before include this file
#define MAX_TIME_INACTIVE 600000

// Buffered print write to WiFi -> length of buffer
// Can be by project, just define it before include this file
#define BUFFER_PRINT 150

// Should the help text be displayed on connection.
// Enabled by default, comment to disable
#define SHOW_HELP true

// Buffering (sends in interval of time to avoid ESP misterious delays)
// Uncomment this to disable it
#define CLIENT_BUFFERING true
#ifdef CLIENT_BUFFERING
#define DELAY_TO_SEND 10 // Time to send buffer
#define MAX_SIZE_SEND 1460 // Maximum size of packet (limit of TCP/IP)
#endif

// Enable if you test features yet in development
//#define ALPHA_VERSION true

// Debugger support enabled ?
// Comment this to disable it
#define DEBUGGER_ENABLED true
#ifdef DEBUGGER_ENABLED
#define DEBUGGER_HANDLE_TIME 850 // Interval to call handle of debugger - equal to implemented in debugger
// App have debugger elements on screen ?
// Note: app not have it yet
//#define DEBUGGER_SEND_INFO true
#endif

///// Websocket server to support debug over web browser (RemoteDebugApp)
// Uncomment this to disable it
//#define WEBSOCKET_DISABLED true

///////////// For RemoteDebugWS ///////////////////

#ifndef WEBSOCKET_DISABLED

// Websocket port
#define WEBSOCKET_PORT 8232

// Library arduinoWebSockets already installed, uncomment to use it
// Do this if you receive errors of multiple definition ...
//#define USE_LIB_WEBSOCKET true
#endif

///////////// For RemoteDebugger ///////////////////

// Enable Flash variables support - F()
// Used internally in SerialDebug and in public API
// If is a low memory board, like AVR, all strings in SerialDebug is using flash memory
// If have RAM memory, this is more fast than flash
//#define DEBUG_USE_FLASH_F true

// For Espressif boards, default is not flash support for printf,
// due it have a lot of memory and Serial.printf is not compatible with it
// If you need more memory, can force it:
//#define DEBUG_USE_FLASH_F true

#endif /* DEBUG_DISABLED */

#endif /* REMOTEDEBUGCFG_H_ */

////// End
