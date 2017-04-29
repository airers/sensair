/**
 * Singleton to manage the current program state
 */
#ifndef _STATE_MANAGER_H_
#define _STATE_MANAGER_H_
#include <Arduino.h>
#include <RTClib.h>

// Defining constants

#define MICROCLIMATE_INDOORS 0
#define MICROCLIMATE_OUTDOORS 1

class StateManager {
private:
public:
  uint8_t microclimate;
  RTC_DS1307 rtc;

  void init() {
    microclimate = MICROCLIMATE_INDOORS;
    rtc.begin();
  }

  // Microclimate Getter/Setter
  uint8_t getMicroclimate() {
    return microclimate;
  }
  void setMicroclimate(uint8_t _microclimate) {
    microclimate = _microclimate;
  }

  // Time Getter/Setter
  DateTime getDateTime() {
    return rtc.now();
  }

  void printNow() {
    DateTime now = getDateTime();
    Serial.print(now.day());
    Serial.print("/");
    Serial.print(now.month());
    Serial.print("/");
    Serial.print(now.year());
    Serial.print(" ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.print(now.second());
    Serial.println();
  }
  long getTimeStamp() {
    return rtc.now().unixtime();
  }
  void setTime(long time) {
    rtc.adjust(DateTime(time));
  }
};

#endif // _STATE_MANAGER_H_
