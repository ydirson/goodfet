/*! \file monitor.h
  \author Travis Goodspeed
  \brief Debug monitor commands.
*/

#ifndef MONITOR_H
#define MONITOR_H

#include "app.h"

// Monitor application number
#define MONITOR 0x00

// Monitor Commands
#define MONITOR_CHANGE_BAUD 0x80
#define MONITOR_ECHO 0x81
#define MONITOR_LIST_APPS 0x82
#define MONITOR_RAM_PATTERN 0x90
#define MONITOR_RAM_DEPTH 0x91

#define MONITOR_DIR 0xA0
#define MONITOR_OUT 0xA1
#define MONITOR_IN  0xA2

#define MONITOR_SILENT 0xB0
#define MONITOR_CONNECTED 0xB1

#define MONITOR_READBUF 0xC0
#define MONITOR_WRITEBUF 0xC1
#define MONITOR_SIZEBUF 0xC2

#define MONITOR_LEDTEST 0xD0
#define MONITOR_SETPIN 0xD1

extern app_t const monitor_app;

#define MONITOR_PIN_TDO 1
// 2: VCC
#define MONITOR_PIN_TDI 3
// 4: VCC
#define MONITOR_PIN_TMS 5
// 6: unused
#define MONITOR_PIN_TCK 7
#define MONITOR_PIN_TST 8
// 9: GND
// 10: unused
#define MONITOR_PIN_RST 11
#define MONITOR_PIN_RX  12
// 13: unused
#define MONITOR_PIN_TX  14


#endif // MONITOR_H

