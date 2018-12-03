/* 
 * telnet.h - Telnet Defines 
 * 
 * Note: only used is uncommented
 * */

#ifndef TELNET_H
#define TELNET_TELNET_H

#define TELNET_IAC   255
#define TELNET_DONT  254
#define TELNET_DO    253
#define TELNET_WONT  252
#define TELNET_WILL  251

// #define TELNET_SE  240  // Subnegotiation End
// #define TELNET_NOP 241  // No Operation
// #define TELNET_DM  242  // Data Mark
// #define TELNET_BRK 243  // Break
// #define TELNET_IP  244  // Interrupt process
// #define TELNET_AO  245  // Abort output
// #define TELNET_AYT 246  // Are You There
// #define TELNET_EC  247  // Erase Character
// #define TELNET_EL  248  // Erase Line
//#define TELNET_GA  249  // Go Ahead
// #define TELNET_SB  250  // Subnegotiation Begin

// #define TELNET_BINARY 0 // 8-bit data path
#define TELNET_ECHO 1 // echo
// #define TELNET_RCP 2 // prepare to reconnect
#define TELNET_SGA 3 // suppress go ahead
// #define TELNET_NAMS 4 // approximate message size
// #define TELNET_STATUS 5 // give status
// #define TELNET_TM 6 // timing mark
// #define TELNET_RCTE 7 // remote controlled transmission and echo
// #define TELNET_NAOL 8 // negotiate about output line width
// #define TELNET_NAOP 9 // negotiate about output page size
// #define TELNET_NAOCRD 10 // negotiate about CR disposition
// #define TELNET_NAOHTS 11 // negotiate about horizontal tabstops
// #define TELNET_NAOHTD 12 // negotiate about horizontal tab disposition
// #define TELNET_NAOFFD 13 // negotiate about formfeed disposition
// #define TELNET_NAOVTS 14 // negotiate about vertical tab stops
// #define TELNET_NAOVTD 15 // negotiate about vertical tab disposition
// #define TELNET_NAOLFD 16 // negotiate about output LF disposition
// #define TELNET_XASCII 17 // extended ascii character set
// #define TELNET_LOGOUT 18 // force logout
// #define TELNET_BM 19 // byte macro
// #define TELNET_DET 20 // data entry terminal
// #define TELNET_SUPDUP 21 // supdup protocol
// #define TELNET_SUPDUPOUTPUT 22 // supdup output
// #define TELNET_SNDLOC 23 // send location
// #define TELNET_TTYPE 24 // terminal type
// #define TELNET_EOR 25 // end or record
// #define TELNET_TUID 26 // TACACS user identification
// #define TELNET_OUTMRK 27 // output marking
// #define TELNET_TTYLOC 28 // terminal location number
// #define TELNET_VT3270REGIME 29 // 3270 regime
// #define TELNET_X3PAD 30 // X.3 PAD
// #define TELNET_NAWS 31 // window size
// #define TELNET_TSPEED 32 // terminal speed
// #define TELNET_LFLOW 33 // remote flow control
// #define TELNET_LINEMODE 34 // Linemode option
// #define TELNET_XDISPLOC 35 // X Display Location
// #define TELNET_OLD_ENVIRON 36 // Old - Environment variables
// #define TELNET_AUTHENTICATION 37 // Authenticate
// #define TELNET_ENCRYPT 38 // Encryption option
// #define TELNET_NEW_ENVIRON 39 // New - Environment variables
// #define TELNET_TN3270E 40 // TN3270E
// #define TELNET_XAUTH 41 // XAUTH
// #define TELNET_CHARSET 42 // CHARSET
// #define TELNET_RSP 43 // Telnet Remote Serial Port
// #define TELNET_COM_PORT_OPTION 44 // Com Port Control Option
#define TELNET_SUPPRESS_LOCAL_ECHO 45 // Telnet Suppress Local Echo
// #define TELNET_TLS 46 // Telnet Start TLS
// #define TELNET_KERMIT 47 // KERMIT
// #define TELNET_SEND_URL 48 // SEND-URL
// #define TELNET_FORWARD_X 49 // FORWARD_X
// #define TELNET_PRAGMA_LOGON 138 // TELOPT PRAGMA LOGON
// #define TELNET_SSPI_LOGON 139 // TELOPT SSPI LOGON
// #define TELNET_PRAGMA_HEARTBEAT 140 // TELOPT PRAGMA HEARTBEAT
// #define TELNET_EXOPL 255 // Extended-Options-List
// #define TELNET_NOOPT 0

// #define TELNET_IS 0
// #define TELNET_SEND 1

#endif // TELNET_H

