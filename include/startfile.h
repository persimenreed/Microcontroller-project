 /**
 * @file   startfile.h
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#ifndef __STARTFILE_H__
#define __STARTFILE_H__

#include "mbed.h"
#include "DFRobot_RGBLCD.h"

// struct to return lat, lon and rtc
struct Values {
    float lat;
    float lon;
    int rtc;
};

Values startfile();
void print_unix(int date_time_unix);
void print_lat_lon(float latitude, float longitude);
void print_city(std::string city_name);

extern DFRobot_RGBLCD lcd;
extern Timer t;


#endif // __STARTFILE_H__