/**
 * Singleton to manage the current program state
 */
#ifndef _STATE_MANAGER_H_
#define _STATE_MANAGER_H_
#include <Arduino.h>

// Defining constants
#define STATE_IDLE    0
#define STATE_SENDING 1

#define MICROCLIMATE_INDOORS 0
#define MICROCLIMATE_OUTDOORS 0

class StateManager {
private:
public:
  uint8_t state;
  uint8_t microclimate;
  RTC_DS1307 rtc;

  void init() {
    state = STATE_IDLE;
    microclimate = MICROCLIMATE_INDOORS;
    rtc.begin();
  }
  // static StateManager * inst();
  // Reads incoming data packets from bluetooth
  void processPackets();
  // Processes the current state
  void stateProcessing();

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
  long getTimeStamp() {
    return rtc.now().unixtime();
  }
  void setTime(long time) {
    rtc.adjust(DateTime(time));
  }
};

#endif // _STATE_MANAGER_H_
