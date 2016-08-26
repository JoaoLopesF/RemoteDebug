#RemoteDebug Library

## The remote debug solution over telnet connection for ESP8266!

#### This works with the ESP8266 Arduino platform with a recent stable release(2.0.0 or newer) https://github.com/esp8266/Arduino

## Contents
 - [About](#about)
 - [Wishlist](#wishlist)
 - [Using](#using)
 - [Releases](#releases)
 - [Thanks](#thanks)

## About

Arduino unfortunately only have as debug tool of the type Serial.print commands

With ESP8266 (NodeMCU) have now interconnected devices on the WiFi network.

Debug only with Serial not good enough and we usually have several nodes to manage,
and debug the communication between them.
And the nodes may be in another location.

With this library, you can debug remotely ESP8266 with a client telnet.

Telnet clients:
      - Telnet: for MacOS and Linux
      - Putty or another: for MS Windows

Debug your devices over a telnet connection!

Very simple and powerful tool for development

It does all the work, running a telnet server and manage the client.
(only one client at same time, by avoid overheads).

Use Print commands like Serial.print over WiFi with any telnet client.

With debug levels to filter incoming messages

Debug levels:
 - Verbose
 - Debug
 - Info
 - Warnings
 - Errors

  These levels is is in order of priority (minor-> verbose / major-> errors)

overhead mininum if not client telnet connected.

It also allows to run predefined commands in your code, for example, to send status or perform some routine. All this remotely controlled via telnet.

Optimized to reduce overheads, including telnet buffer

## Wishlist
- Http page to begin/stop the telnet server
- Authentications

## Using

- Include in your sketch
  - Required
```cpp
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
```
  - RemoteDebug Library
```cpp
// Remote debug over telnet - not recommended for production, only for development

#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

RemoteDebug Debug;
```
- In the setup function after WiFi initialization
```cpp
// Initialize the telnet server of RemoteDebug

Debug.begin(); // Initiaze the telnet server

Debug.setResetCmdEnabled(true); // Enable the reset command
//Debug.showTime(true); // To show time
// Debug.showProfiler(true); // To show profiler - time between messages of Debug

```
- In the tail of loop function
```cpp
// Remote debug over telnet

Debug.handle();

```
- In any place of you code:
```cpp
if (Debug.ative(Debug.<level>)) {
    Debug.printf("bla bla bla: %d %s\n", number, str); // OR
    Debug.println("bla bla bla");
}
```
 - Please not forget to use if clause with Debug.Ative
   ---> This is very important to reduce overheads and work of debug levels

 - Please see the samples, basic or advanced, to learn how to use  

## Releases
#### 0.9
- First Beta

## Thanks

Example of TelnetServer code in http://www.rudiswiki.de/wiki9/WiFiTelnetServer
