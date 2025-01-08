 /**
 * @file   timescreen.h
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#ifndef __TIMESCREEN_H__
#define __TIMESCREEN_H__

#include "mbed.h"

int timescreen(int rtc);
void printLCD(int alarmHours, int alarmMinutes, int alarmNum);


static Thread threadAlarm;

#endif // __TIMESCREEN_H__
