#include "StateManager.h"

// StateManager::StateManager() {
//   state = STATE_IDLE;
//   microclimate = MICROCLIMATE_INDOORS;
//   rtc.begin();
// }

// StateManager * StateManager::_singleton = NULL;
// StateManager * StateManager::inst() {
//   if ( !_singleton ) _singleton = new StateManager();
//   return _singleton;
// }


// uint8_t StateManager::getMicroclimate() {
//   return microclimate;
// }
// void StateManager::setMicroclimate(uint8_t _microclimate) {
//   microclimate = _microclimate;
// }


// static DateTime StateManager::getDateTime() {
//   return rtc.now();
// }
// static long StateManager::getTimeStamp() {
//   return rtc.now().unixtime();
// }
// void StateManager::setTime(uint32_t time) {
//   rtc.adjust(DateTime(time));
// }
