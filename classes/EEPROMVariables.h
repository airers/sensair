/**
 * Class to process data in SD cards
 */
#ifndef _EEPROM_VARIABLES_H_
#define _EEPROM_VARIABLES_H_

#include <Arduino.h>
#include <EEPROM.h>
#include "datatypes.h"

#define CURRENT_TIME      0
#define NEXT_MINUTE_TIME  4
class ROMVar {
private:
  static void setLong(int address, long data) {
    long_u write;
    write.data = data;
    EEPROM.write(address    , write.bytes[0]);
    EEPROM.write(address + 1, write.bytes[1]);
    EEPROM.write(address + 2, write.bytes[2]);
    EEPROM.write(address + 3, write.bytes[3]);
  }
  static long getLong(int address) {
    long_u read;
    read.bytes[0] = EEPROM.read(address);
    read.bytes[1] = EEPROM.read(address + 1);
    read.bytes[2] = EEPROM.read(address + 2);
    read.bytes[3] = EEPROM.read(address + 3);
    return read.data;
  }
public:

  // Writing takes 9ms
  // Reading takes 4ms

  static void setCurrentTime(long time) {
    setLong(CURRENT_TIME, time);
  }

  static long getCurrentTime() {
    return getLong(CURRENT_TIME);
  }

  static void setNextMinuteTime(long time) {
    setLong(NEXT_MINUTE_TIME, time);
  }

  static long getNextMinuteTime() {
    return getLong(NEXT_MINUTE_TIME);
  }
};



#endif // _EEPROM_VARIABLES_H_
