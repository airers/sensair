/**
 * Singleton to manage the current program state
 * Manages the message bar at the bottom
 */
#ifndef _STATE_MANAGER_H_
#define _STATE_MANAGER_H_
#include <Arduino.h>
#include <RTClib.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include "EEPROMVariables.h"

// Defining constants

#define MICROCLIMATE_INDOORS 0
#define MICROCLIMATE_OUTDOORS 1

class StateManager {
private:
  void print2digits(int number) {
    if (number >= 0 && number < 10) {
      Serial.write('0');
    }
    Serial.print(number);
  }
public:
  uint8_t microclimate;
  RTC_DS1307 rtc;
  Adafruit_ST7735 * tft;
  

  void init(Adafruit_ST7735 * _tft) {
    microclimate = MICROCLIMATE_INDOORS;
    tft = _tft;
    rtc.begin();
    ROMVar::setCurrentTime(getTimeStamp());
    ROMVar::setNextMinuteTime(calculateNextMinute());
  }

  long calculateNextMinute() {
    DateTime currentDateTime = getDateTime();
    return currentDateTime.unixtime() + (60 - currentDateTime.second());
  }
  
  void setNextMinute() {
    ROMVar::setNextMinuteTime(calculateNextMinute());
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
    print2digits(now.day());
    Serial.print("/");
    print2digits(now.month());
    Serial.print("/");
    Serial.print(now.year());
    Serial.print(" ");
    print2digits(now.hour());
    Serial.print(":");
    print2digits(now.minute());
    Serial.print(":");
    print2digits(now.second());
    Serial.println();
  }
  
  long getTimeStamp() {
    return rtc.now().unixtime();
  }
  
  void setTime(long time) {
    rtc.adjust(DateTime(time));
    ROMVar::setCurrentTime(time);
    setNextMinute();
  }
  
  void startCount() {
    tft->fillRect(5,96,120,20, ST7735_BLACK);
    tft->setTextColor(ST7735_GREEN);
    tft->setCursor(5, 96);
    tft->print("Counting...");
  }
  
  void updateCount(uint16_t count) {
    tft->fillRect(5,106,120,10, ST7735_BLACK);
    tft->setTextColor(ST7735_GREEN);
    tft->setCursor(5, 106);
    tft->print(String(String(count) + " readings"));
  }
  
  void startSend() {
    tft->fillRect(5,96,120,20, ST7735_BLACK);
    tft->setTextColor(ST7735_GREEN);
    tft->setCursor(5, 96);
    tft->print("Sending...");
  }
  
  void updateSend(uint16_t left) {
    tft->fillRect(5,106,120,10, ST7735_BLACK);
    tft->setTextColor(ST7735_GREEN);
    tft->setCursor(5, 106);
    tft->print(String(String(left) + " left"));
  }
  
  void end() {
    tft->fillRect(5,96,120,20, ST7735_BLACK);
  }
};

#endif // _STATE_MANAGER_H_
