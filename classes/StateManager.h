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
  uint8_t state;
  uint8_t microclimate;
  RTC_DS1307 rtc;

  static StateManager _singleton;
  StateManager();
public:
  static StateManager instance();
  // Reads incoming data packets from bluetooth
  void processPackets();
  // Processes the current state
  void stateProcessing();

  // Microclimate Getter/Setter
  uint8_t getMicroclimate();
  void setMicroclimate(uint8_t _microclimate);

  // Time Getter/Setter
  DateTime getDateTime();
  long getTimeStamp();
  void setTime(long time);
};

#endif // _STATE_MANAGER_H_
