/**
 * @file   timescreen.cpp
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#include "timescreen.h"
#include "alarm.h"
#include "buttons.h"
#include "mbed.h"
#include "news.h"
#include "startfile.h"
#include "temphum.h"
#include <cstdint>

#define WAIT_TIME_MS 1000

char buffer[32];
bool threadState = false;

int timescreen(int rtc) {


  // DEFINE BUTTONS
  InterruptIn *button_hum_temp = init_button_hum_temp();
  InterruptIn *button_set_weather = init_button_weather();
  InterruptIn *button_set_news = init_button_news();
  InterruptIn *button_mute_alarm = init_button_mute_alarm();

  // Set time. Timer t + UNIX time.
  int total_time =
      rtc + duration_cast<std::chrono::seconds>(t.elapsed_time()).count();

  set_time(total_time);

  int minutes = 0;
  int seconds = 0;

  while (true) {

    time_t seconds = time(NULL) + 2 * 60 * 60; // CET
    strftime(buffer, 32, "%a %d %b %H:%M:%S", localtime(&seconds));
    printf("Tid: %s\n", buffer);

    // check if user wants to show temp and hum
    if (button_hum_temp->read()) {
      printf("Showing temperature and humidity. Press button again to exit\n");
      return 3;
    }

    // check if user wants to show weather
    if (button_set_weather->read()) {
      printf("Going to weather screen\n");
      return 4;
    }

    // check if user wants to show news
    if (button_set_news->read()) {
      printf("Going to news screen\n");
      print_news();
    }

    // start alarm thread
    if (button_mute_alarm->read()) {
     threadAlarm.start(alarm);
     bool threadState = true;
    }

    // print time if no alarm-thread is started
    if (threadState == false){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.printf(buffer);
    }
    



    thread_sleep_for(WAIT_TIME_MS);
  }

  return 2;
}


void printLCD(int alarmHours, int alarmMinutes, int alarmNum) {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf(buffer);
  lcd.setCursor(0, 1);

  if (alarmNum == 0) {
    if (alarmMinutes >= 10) {
      lcd.printf("Set alarm: %d:%d", alarmHours, alarmMinutes);
    } else {
      lcd.printf("Set alarm: %d:0%d", alarmHours, alarmMinutes);
    }
  } else if (alarmNum == 1) {
    if (alarmMinutes >= 10) {
      lcd.printf("Alarm: %d:%d", alarmHours, alarmMinutes);
    } else {
      lcd.printf("Alarm: %d:0%d", alarmHours, alarmMinutes);
    }
  }else if (alarmNum == 2) {

      lcd.printf("!!! ALARM !!!");

  }else if (alarmNum == 3) {
    if (alarmMinutes >= 10) {
      lcd.printf("Snooze: %d:%d", alarmHours, alarmMinutes);
    } else {
      lcd.printf("Snooze: %d:0%d", alarmHours, alarmMinutes);
    }
  }
};


