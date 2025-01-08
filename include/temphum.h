 /**
 * @file   temphum.h
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#ifndef __TEMPHUM_H__
#define __TEMPHUM_H__

#include "DFRobot_RGBLCD.h"
#include "HTS221Sensor.h"
#include "mbed.h"

extern InterruptIn* init_button();

int temphum();



#endif // __TEMPHUM_H__