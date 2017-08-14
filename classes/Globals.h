#ifndef _GLOBAL_STUFF_H_
#define _GLOBAL_STUFF_H_

#include <Arduino.h>
#include "StateManager.h"


class Globals {
public:
  static StateManager * stateManager;
  
};

StateManager * Globals::stateManager = NULL;


#endif