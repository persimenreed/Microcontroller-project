/**
 * @file   alarm.cpp
 * @author Isak Rønningen, Hans Petter Grindheim
 */

#include "alarm.h"
#include "DFRobot_RGBLCD.h"
#include "buttons.h"
#include "mbed.h"
#include "startfile.h"
#include "time.h"
#include "timescreen.h"

#define WAIT_TIME_MS 1000
Timer alarmTimer;
AnalogIn alarmIn(A2);
PwmOut alarmSound(D9);
DigitalIn alarmStatusButton(D5, PullDown);


// Define buttons
InterruptIn *button_set_alarm = init_button_alarm();
InterruptIn *button_mute_alarm = init_button_mute_alarm();
InterruptIn *button_snooze_alarm = init_button_snooze_alarm();

int alarmNum = 0;

// function when alarm rings
void alarmGoOff(bool alarmMute, int alarmHours, int alarmMinutes) {
  alarmTimer.start();
  alarmNum = 2;
  int count = 0;

  // Keeps going in while as long as count is less than 10 minutes, or user press mute.
  while (count < 60 * 10) {
    alarmSound.write(0.5f);
    count++;

    alarmSound.resume();

    printLCD(alarmHours, alarmMinutes, alarmNum);

    // turn of alarm
    if (button_mute_alarm->read()) {
      alarmNum = 1;
      alarmSound.suspend();
      return;
    }

    // snooze alarm
    if (button_snooze_alarm->read()) {
      count = 0;
      snooze(alarmHours, alarmMinutes);
    }

    thread_sleep_for(WAIT_TIME_MS);
  }
  alarmSound.suspend();
}

// function that snooze alarm and adds 5 minutes to next timer
void snooze(int alarmHours, int alarmMinutes) {
  alarmSound.suspend();
  int count = 0;
  alarmNum = 3;
  alarmMinutes = alarmMinutes+5;

  // snooze for 5 minutes
  while (count < 5*60) {

    printLCD(alarmHours, alarmMinutes, alarmNum);
    count++;

    thread_sleep_for(WAIT_TIME_MS);
  }
  alarmSound.resume();
  alarmNum = 2;
}

void alarm() {

  // define
  int dayInSeconds = 86400;
  int clockTime;
  int alarmHours;
  int alarmMinutes;
  int alarmSeconds;
  time_t timecheck;
  char localSeconds[2];
  bool alarmIsSet = false;
  int finalSeconds;
  bool alarmMute = false;
  bool alarmStatus = true;

  while (true) {
    if (alarmStatus) {

      if (alarmStatusButton) {
        alarmStatus = !alarmStatus;
      }
      time_t seconds = time(NULL) + 2 * 60 * 60; // CET

    // find time since dawn to compare to set alarm time
      char buffer2[32];
      char buffer3[32];
      char buffer4[32];

      strftime(buffer2, sizeof(buffer2), "%S", localtime(&seconds));
      strftime(buffer3, sizeof(buffer3), "%M", localtime(&seconds));
      strftime(buffer4, sizeof(buffer4), "%H", localtime(&seconds));

      int32_t seconds_since_dawn = atoi(buffer2);
      int32_t minutes_since_dawn = atoi(buffer3);
      int32_t hours_since_dawn = atoi(buffer4);

      int32_t time_since_dawn = seconds_since_dawn + (minutes_since_dawn * 60) +
                                (hours_since_dawn * 3600);

    // translate from seconds to hh:mm
      if (alarmIsSet == false) {
        alarmSeconds = alarmIn.read() * 86400;
        alarmMinutes = alarmSeconds / 60;
        int newSeconds = alarmSeconds % 60;
        alarmHours = alarmMinutes / 60;
        alarmMinutes = alarmMinutes % 60;

        alarmMinutes = ((alarmMinutes + 5 / 2) / 5) * 5;
      }

      if (alarmMinutes == 60)
        alarmMinutes = 0;

      // Alarm time translated to seconds from dawn
      finalSeconds = (alarmMinutes * 60) + (alarmHours * 3600);

      printLCD(alarmHours, alarmMinutes, alarmNum);

      if (time_since_dawn == finalSeconds ||
          time_since_dawn + 1 == finalSeconds) {
        alarmGoOff(alarmMute, alarmHours, alarmMinutes);
      }

      // Set alarm
      if (button_set_alarm->read()) {
        alarmIsSet = true;
        alarmNum = 1;
        thread_sleep_for(WAIT_TIME_MS);
      }

      thread_sleep_for(WAIT_TIME_MS/4);

    } else {
      bool check = true;
      while (check) {
        if (alarmStatusButton) {
          check = !check;
          alarmStatus = !alarmStatus;
        }
      }
    }
    thread_sleep_for(WAIT_TIME_MS);
  }
}