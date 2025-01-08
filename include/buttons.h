/**
 * @file   buttons.h
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include "mbed.h"

extern InterruptIn* init_button_set_coordinates();
extern InterruptIn* init_button_weather();
extern InterruptIn* init_button_hum_temp();
extern InterruptIn* init_button_news();
extern InterruptIn* init_button_alarm();
extern InterruptIn* init_button_mute_alarm();
extern InterruptIn* init_button_snooze_alarm();

#endif // __BUTTONS_H__