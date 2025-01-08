/**
 * @file   temphum.cpp
 * @author Per Simen Reed, Isak Rønningen, Hans Petter Grindheim
 */

#include "temphum.h"
#include "DFRobot_RGBLCD.h"
#include "HTS221Sensor.h"
#include "mbed.h"
#include "startfile.h"
#include "buttons.h"
#include "timescreen.h"

#define WAIT_TIME_MS 1000

// defining i2c and hts sensors and inputs
DevI2C i2c(PB_11, PB_10);
HTS221Sensor hts221(&i2c);

int temphum() {

  InterruptIn *button_hum_temp = init_button_hum_temp();

// connect to hts
  if (hts221.init(NULL) != 0) {
    printf("Init failed\n");
  }
  if (hts221.enable() != 0) {
    printf("Enable failed\n");
  }

  lcd.init();
  lcd.display();

  bool state = true;
  while (state) {

      float hum;
      float temp;

      hts221.get_humidity(&hum);
      hts221.get_temperature(&temp);

      // PRINT TEMPERATURE AND HUMIDITY
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.printf("Temp: %.1f C", temp);
      lcd.setCursor(0, 1);
      lcd.printf("Hum: %.1f %", hum);

      printf("Humidity: %.1f, Temperature: %.1f\n", hum, temp);



// SEND BACK TO MAIN FUNCTION IF BUTTON IS PRESSED AGAIN
      if(button_hum_temp->read()){
          state=false;
      }



      // BLINKING LED
    DigitalOut led1(LED1);
    led1 = !led1;
    thread_sleep_for(WAIT_TIME_MS);

    }
    return 2;
}
