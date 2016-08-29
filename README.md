#RemoteDebug Library for ESP8266

## A library to remote debug over telnet connection!

#### This works with the ESP8266 Arduino platform with a recent stable release(2.0.0 or newer) https://github.com/esp8266/Arduino

## Contents
 - [About](#about)
 - [Wishlist](#wishlist)
 - [Using](#using)
 - [Know issues](#knowissues)
 - [Releases](#releases)
 - [Thanks](#thanks)

## About

Arduino unfortunately only have as debug tool of the type Serial.print commands

With ESP8266 (NodeMCU) have now interconnected devices on the WiFi network.

Debug only with Serial not good enough and we usually have several nodes to manage,
and debug the communication between them.
And the nodes may be in another location.

With this library, you can **debug remotely** ESP8266 with a client **telnet**.

Telnet clients:
      - telnet: native for MacOS and Linux
      - Putty or another: for MS Windows

Very simple and powerful tool for development

It does all the work, running a telnet server and manage the client.
(only one client at same time, by avoid overheads).

Use Print commands like Serial.print over WiFi with any telnet client.

With debug levels to filter incoming messages

**Debug levels**:
 - Verbose
 - Debug
 - Info
 - Warnings
 - Errors

  These levels is is in order of priority (minor-> verbose / major-> errors)

And by the telnet can be setted the debug level,
after this only messages of this level or greater is showed.
It is useful to filter important messages.

Also have a **Profiler**, that can be turn on in telnet connection or in the code.
This show the time between 2 calls of debug.
Example: One debug message before call of any funcion, and another after
The time showed in last message, is the time that a function spends to run.

overhead mininum if not client telnet connected.

It also allows to run predefined commands in your code, for example, to send status or perform some routine. All this remotely controlled via telnet.

Optimized to reduce overheads, including telnet buffer

Also please see my another library: https://github.com/JoaoLopesF/ArduinoUtil

DISCLAIMER:

This Library is NOT have yet authentications, and is **ONLY for development** NOT to production!
In the future peraps this will supported prodution.

## Wishlist
- Http page to begin/stop the telnet server
- Authentications

## How it looks

![Imgur]
(http://i.imgur.com/QiccbmK.png)

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

Debug.begin("Telnet_HostName"); // Initiaze the telnet server
// OR
Debug.begin(HOST_NAME); // Initiaze the telnet server - HOST_NAME is the used in MDNS.begin

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
    Debug.printf("bla bla bla: %d %s\n", number, str.c_str()); // Note: if type is String need c_str() // OR
    Debug.println("bla bla bla 2 ln");
    // Note: to show floats with printf,
    // you can use my ArduinoUtil library -> https://github.com/JoaoLopesF/ArduinoUtil
    Debug.printf("float: %f\n", value); // Not works :-(
    Debug.printf("float: %s\n", Util.formatFloat(value, 0, 5).c_str());
}
```
- An example of use debug levels: (supposing the data is a lot of characteres)
```cpp
if (Debug.ative(Debug.VERBOSE)) { // Debug message long
    Debug.printf("routine: data received: %s\n", data.c_str()); // Note: if type is String need c_str()
} else if (Debug.ative(Debug.DEBUG)) { // Debug message short
    Debug.printf("routine: data received: %s ...\n", data.substring(0, 20).c_str()); // %.20s not working :-|
}
```
- An example of use debug with serial enabled
  Useful to see messages if setup or
  in cause the ESP8266 is rebooting (telnet connection stop before received all messages)
  Only for this purposes I suggest it
```
// Setup after Debug.begin
Debug.setSerialEnabled(true);
// All messages too send to serial too, and can be see in serial monitor
```

 - For reduce overheads RemoteDebug is disconnect the telnet client if it not active.
 - Please pless enter or any key if you need keep the connection
 - The default is 5 minutes (You can change it in RemoteDebug.h)  
 - You can use mDNS to register each node with different name, it helps to connect without know the IP.

 - Please not forget to use if clause with Debug.Ative
   ---> This is very important to reduce overheads and work of debug levels

 - Please see the samples, basic or advanced, to learn how to use  

 - In advanced sample, I used WifiManager library, ArduinoOTA and mDNS, please see it.

## Releases
#### 0.9
- First Beta

# Know issues

- Sometimes the connection over telnet is slow,
  reset command in telnet connection or turn off/on can be resolve it.
  But I need find why it occurs

## Thanks

First thanks a lot for Igrr for bring to us the Arduino ESP8266.

Resources:

- Example of TelnetServer code in http://www.rudiswiki.de/wiki9/WiFiTelnetServer
