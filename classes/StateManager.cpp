#include "StateManager.h"

StateManager::StateManager() {
  state = STATE_IDLE;
  microclimate = MICROCLIMATE_INDOORS;
  rtc.begin();
}

StateManager::instance() {
  if ( !_singleton ) _singleton = new StateManager();
  return _singleton;
}


uint8_t StateManager::getMicroclimate() {
  return microclimate;
}
void StateManager::setMicroclimate(uint8_t _microclimate) {
  microclimate = _microclimate;
}


DateTime StateManager::getDateTime() {
  return rtc.now();
}
long StateManager::getTimeStamp() {
  return rtc.now();
}
void StateManager::setTime(uint32_t time) {
  rtc.adjust(DateTime(time));
}
