/**
 * @file   coordinates.h
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#ifndef __COORDINATES_H__
#define __COORDINATES_H__

#include "mbed.h"
#include "DFRobot_RGBLCD.h"
#include "HTS221Sensor.h"
#include "metAPI.h"

extern InterruptIn* init_button();

int set_coordinates();
int set_weather(float lat, float lon);
float set_lat();
float set_lon();
void print_weather(float lat, float lon);



#endif // __COORDINATES_H__