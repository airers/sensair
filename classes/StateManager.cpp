#include "StateManager.h"

StateManager::StateManager() {
  state = STATE_IDLE;
}

StateManager::instance() {
  if ( !_singleton ) _singleton = new StateManager();
  return _singleton;
}
