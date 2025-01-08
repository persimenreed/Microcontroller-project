/**
 * @file   buttons.cpp
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#include "buttons.h"
#include "mbed.h"


// INITIALIZING BUTTON TO USE IN ALL FILES
InterruptIn *init_button_hum_temp() {
  static InterruptIn button_hum_temp(D4, PullDown);
  return &button_hum_temp;
}


InterruptIn *init_button_set_coordinates() {
  static InterruptIn button_set_coordinates(D1, PullDown);
  return &button_set_coordinates;
}


InterruptIn *init_button_weather() {
  static InterruptIn button_set_weather(D8, PullDown);
  return &button_set_weather;
}

InterruptIn *init_button_news() {
  static InterruptIn button_set_news(D7, PullDown);
  return &button_set_news;
}

InterruptIn *init_button_alarm() {
  static InterruptIn button_set_alarm(D2, PullDown);
  return &button_set_alarm;
}

InterruptIn *init_button_mute_alarm() {
  static InterruptIn button_mute_alarm(D11, PullDown);
  return &button_mute_alarm;
}

InterruptIn *init_button_snooze_alarm() {
  static InterruptIn button_snooze_alarm(D12, PullDown);
  return &button_snooze_alarm;
}


