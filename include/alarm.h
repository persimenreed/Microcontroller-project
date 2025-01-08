/**
 * @file   buttons.h
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#ifndef __ALARM_H__
#define __ALARM_H__

#include "mbed.h"

void alarm();
void snooze(int alarmHours, int alarmMinutes);
void alarmGoOff(bool alarmMute, int alarmHours, int alarmMinutes);

#endif // __ALARM_H__