/**
 * Singleton to manage the current program state
 */
#ifndef _STATE_MANAGER_H_
#define _STATE_MANAGER_H_
#include <Arduino.h>

#define STATE_IDLE    0
#define STATE_SENDING 1

class StateManager {
private:
  uint8_t state;
  static StateManager _singleton;
  StateManager();
public:
  static StateManager instance();
  // Reads incoming data packets from bluetooth
  void processPackets();
  // Processes the current state
  void stateProcessing();
};

#endif // _STATE_MANAGER_H_
