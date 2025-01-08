/**
 * @file   main.cpp
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#include "buttons.h"
#include "mbed.h"
#include "startfile.h"
#include "temphum.h"
#include "timescreen.h"
#include "coordinates.h"
#include "alarm.h"

#define WAIT_TIME_MS 1000
using namespace std::chrono;

// DEFINE BUTTONS
DigitalOut led(LED1);

int main() {

  int num = 1;
  Values result;
  bool state;
  while (true) {

    // Switch case to send user to different screens
    switch (num) {

    // case 1: startfile. User is sent here when starting program.
    case 1:
      printf("Start up screen\n");
      result = startfile();
      num = 2;
      break;

    // case 2: timescreen. Main screen where time is shown.
    case 2:
      num = timescreen(result.rtc);
      break;

    // case 3: screen to show temperature and humidity.
    case 3:
      temphum();
      num = temphum();
      break;

    // case 4: check weather from coords fetched in startscreen.cpp
    case 4:
        num = set_weather(result.lat, result.lon);
      break;
    }

    // LAMP BLINKING WHEN NOT IN A FUNCTION
    led = !led;
    thread_sleep_for(WAIT_TIME_MS);
  }
}
