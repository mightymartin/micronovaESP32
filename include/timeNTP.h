#ifndef TIMENTP_H_
#define TIMENTP_H_

#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TimeZone.h>
#include "NTPClient.h"


extern void TimeInit();
extern void TimeTick();

extern uint8_t    TimeSeconds();
extern uint8_t    TimeMinutes();
extern uint8_t    TimeHours();
extern uint8_t    TimeDay();
extern uint8_t    TimeMonth();
extern uint16_t   TimeYear();

extern String TimeformatedDateTime();
extern String TimeformatedTime();

#endif