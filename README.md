# RemoteDebug Library for ESP8266 or ESP32

### A library to remotely debug over a telnet connection

### Sets-up telnet server that you connect to as an alternative to the standard serial monitor

#### Works with the ESP8266 Arduino platform v2.0.0 or higher / or ESP32. Refer to https://github.com/esp8266/Arduino
https://github.com/espressif/arduino-esp32

## Contents
 - [About](#about)
 - [Standard telnet](#telnet)
 - [Wishlist](#wishlist)
 - [Using](#usage)
 - [Know issues](#knowissues)
 - [Releases](#releases)
 - [Thanks](#thanks)

## About

By default the Arduino only has as debug possibility via the Serial port.
This has a few disadvantages:

- requires a physical cable to the Arduino device (if the device is far away or in a remote location this is not easy)
- debugging multiple Arduinos at the same time requires many serial ports and a lot of cables

With the ESP8266 (NodeMCU) or ESP32 we now have network connectivity which can be used for streaming debugging information in real-time.

News:

Now _RemoteDebug_ is improved with client buffering (is last send is <= 10ms),
to avoid misterious delays of networking on ESP32 and ESP8266 boards

And now have a shortcuts (see it above)

## Telnet

Telnet is a standard way of remotely connecting to a server and is supported on all operating systems (Windows, Mac, Linux...).
A typical telnet client for Windows is Putty for example.

_RemoteDebug_  sets-up a telnet server which is listening to any telnet client that wants to connect. After connection, logging is streamed to the telenet client.

_RemoteDebug_ is very simple to use, after a few lines of initialization code, you can use the well-known "print" commands to stream your logging to the remote client.

### debug levels

_RemoteDebug_ supports the filtering of logging based on **debug levels**:

 - Verbose
 - Debug
 - Info
 - Warnings
 - Errors

These levels are in the order of most-logging -> least-logging.

The telnet client can set the debug level by typing a few simple commands.

### profiler

_RemoteDebug_ includes a simple profiler. It can be enabled by the connected telnet client or the Arduino code itself.

When enabled, it shows the time between 2 debug statements, using different colors depending on the elapsed time.

A typical example would be to insert logging just before and after a function after which you can see how much the is spent in the function.

### overhead

_RemoteDebug_ is designed to give minimal overhead if there is not telnet client connected.

### custom commands

_RemoteDebug_ supports custom commands that can be entered in the telnet client. These trigger the execution of a custom function in the Arduino code. For example this can be used to send back a status on request of the telnet client.


Also please see my another library: https://github.com/JoaoLopesF/ArduinoUtil

##Warning

In beta versions, the **isActive** method was misspelled,
Please when upgrading from a beta to the current one, please review your code.

From:

```
if (Debug.ative(Debug.<level>)) ....
```

To:

```
if (Debug.isActive(Debug.<level>)) ....
```

##DISCLAIMER:

The current version of _RemoteDebug_ does not yet include any authentication and is intended only for development.

Future extension could include a secure way for authentication and further testing to support production environments.

## Wishlist
- Http page to begin/stop the telnet server
- Authentication
- Support for production environment

## How it looks

![Imgur]
(http://i.imgur.com/QiccbmK.png)

Youtube:
[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/lOo-MAD8gPo/0.jpg)](http://www.youtube.com/watch?v=lOo-MAD8gPo)

## Usage

###includes

```cpp
#include <ESP8266WiFi.h> //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
//
// Please see the samples how do your code to run in ESP32 too
//
```
```cpp
#include "RemoteDebug.h" // Remote debug over telnet - not recommended for production, only for development       
```
###instance

RemoteDebug Debug;

###setup

- In the setup function after WiFi initialization
```cpp
// Initialize the telnet server of RemoteDebug

Debug.begin("Telnet_HostName"); // Initiaze the telnet server - this name is used in MDNS.begin
// OR
Debug.begin(HOST_NAME); // Initiaze the telnet server - HOST_NAME is the used in MDNS.begin
// OR
Debug.begin(HOST_NAME, startingDebugLevel); // Initiaze the telnet server - HOST_NAME is the used in MDNS.begin and set the initial debug level

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
if (Debug.isActive(Debug.<level>)) {
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
if (Debug.isActive(Debug.VERBOSE)) { // Debug message long
    Debug.printf("routine: data received: %s\n", data.c_str()); // Note: if type is String need c_str()
} else if (Debug.isActive(Debug.DEBUG)) { // Debug message short
    Debug.printf("routine: data received: %s ...\n", data.substring(0, 20).c_str()); // %.20s not working :-|
}
```
- An example of shortcuts (NEW)
```cpp
DEBUG("This is a any (always showed) - var %d\n", var);

DEBUG_V("This is a verbose - var %d\n", var);
DEBUG_D("This is a debug - var %d\n", var);
DEBUG_I("This is a information - var %d\n", var);
DEBUG_W("This is a warning - var %d\n", var);
DEBUG_E("This is a error - var %d\n", var);

// Note: if you want a simple println you must ended with new line characters

DEBUG_V("This println\n");

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

 - Please not forget to use if clause with Debug.isActive
   ---> This is very important to reduce overheads and work of debug levels

 - Please see the samples, basic or advanced, to learn how to use  

 - In advanced sample, I used WifiManager library, ArduinoOTA and mDNS, please see it.

## Releases

#### 1.2.0

Shortcuts and client buffering to avoid misterious delay of ESP networking

#### 1.1.0
Adjustments and now runs in Esp32 too.

#### 1.0.0
Adjustments and improvements from Beta versions.
News features:
- Filter
- Colors
- Support to Windows telnet client

#### 0.9
- First Beta

# Know issues

- Sometimes (rarely) the connection over telnet becomes very slow.
  Especially right after uploading firmware.
  Reset command in telnet connection or turn off/on can be resolve it.
  But I need find why it occurs

## Thanks

First thanks a lot for Igrr for bring to us the Arduino ESP8266.

Resources:

- Example of TelnetServer code in http://www.rudiswiki.de/wiki9/WiFiTelnetServer
