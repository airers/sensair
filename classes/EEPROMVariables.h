/**
 * Class to process data in SD cards
 */
#ifndef _EEPROM_VARIABLES_H_
#define _EEPROM_VARIABLES_H_

#include <Arduino.h>
#include <EEPROM.h>
#include "datatypes.h"

// Required (stores first reading so device knows where to look for code)
#define FIRST_READING     0   // long (4 bytes)
#define LATEST_READING    4   // long (4 bytes)

// Volatile (does not need to be saved)
#define CURRENT_TIME      12  // long (4 bytes)
#define NEXT_MINUTE_TIME  16  // long (4 bytes)
#define READING_TOTAL     20  // float (4 bytes)
#define LAT_TOTAL         24  // float (4 bytes)
#define LON_TOTAL         28  // float (4 bytes)
#define ELE_TOTAL         32  // float (4 bytes)
#define LAT_MAX           36  // float (4 bytes)
#define LAT_MIN           40  // float (4 bytes)
#define LON_MAX           44  // float (4 bytes)
#define LON_MIN           48  // float (4 bytes)

class ROMVar {
private:
public:
  // Writing takes 9ms
  static void setLong(int address, long data) {
    long_u write;
    write.data = data;
    EEPROM.write(address    , write.bytes[0]);
    EEPROM.write(address + 1, write.bytes[1]);
    EEPROM.write(address + 2, write.bytes[2]);
    EEPROM.write(address + 3, write.bytes[3]);
  }

  // Reading takes 4ms
  static long getLong(int address) {
    long_u read;
    read.bytes[0] = EEPROM.read(address);
    read.bytes[1] = EEPROM.read(address + 1);
    read.bytes[2] = EEPROM.read(address + 2);
    read.bytes[3] = EEPROM.read(address + 3);
    return read.data;
  }

  static void setFloat(int address, float data) {
    float_u write;
    write.data = data;
    EEPROM.write(address    , write.bytes[0]);
    EEPROM.write(address + 1, write.bytes[1]);
    EEPROM.write(address + 2, write.bytes[2]);
    EEPROM.write(address + 3, write.bytes[3]);
  }

  static float getFloat(int address) {
    float_u read;
    read.bytes[0] = EEPROM.read(address);
    read.bytes[1] = EEPROM.read(address + 1);
    read.bytes[2] = EEPROM.read(address + 2);
    read.bytes[3] = EEPROM.read(address + 3);
    return read.data;
  }

  static void setFirstReading(long time) {
    setLong(FIRST_READING, time);
  }

  static long getFirstReading() {
    return getLong(FIRST_READING);
  }

  static void setLatestReading(long time) {
    setLong(LATEST_READING, time);
  }

  static long getLatestReading() {
    return getLong(LATEST_READING);
  }

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

  static void setAccuracyVars(float &lonMin, float &lonMax, float &latMin, float &latMax) {
    setFloat(LON_MIN, lonMin);
    setFloat(LON_MAX, lonMax);
    setFloat(LAT_MIN, latMin);
    setFloat(LAT_MAX, latMax);
  }

  static void getAccuracyVars(float &lonMin, float &lonMax, float &latMin, float &latMax) {
    lonMin = getFloat(LON_MIN);
    lonMax = getFloat(LON_MAX);
    latMin = getFloat(LAT_MIN);
    latMax = getFloat(LAT_MAX);
  }

  static void setAverageVars(float &readingTotal, float &latTotal, float &lonTotal, float &eleTotal) {
    setFloat(READING_TOTAL, readingTotal);
    setFloat(LAT_TOTAL, latTotal);
    setFloat(LON_TOTAL, lonTotal);
    setFloat(ELE_TOTAL, eleTotal);
  }

  static void getAverageVars(float &readingTotal, float &latTotal, float &lonTotal, float &eleTotal) {
    readingTotal = getFloat(READING_TOTAL);
    latTotal = getFloat(LAT_TOTAL);
    lonTotal = getFloat(LON_TOTAL);
    eleTotal = getFloat(ELE_TOTAL);
  }

  static void printFreeRam() {
    Serial.write(C_M);
    Serial.write(C_E);
    Serial.write(C_M);
    Serial.write(C_SP);
    Serial.println(freeRam());
  }

  static int freeRam ()  {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  }
};



#endif // _EEPROM_VARIABLES_H_
