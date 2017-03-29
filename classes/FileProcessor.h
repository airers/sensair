/**
 * Class to process data in SD cards
 */
#ifndef _FILE_PROCESSOR_H_
#define _FILE_PROCESSOR_H_

#include <Arduino.h>
#include <SD.h>

//Defining the pin for the SD Card reader
#define SD_PIN 10

class FileProcessor {
private:
  bool cardAvailable;
  int state;
  // Storing 60 readings is not feasible,
  // will take up 1500 bytes.
  double readingTotal;
  double latTotal;
  double lonTotal;
  double elevationTotal;
  uint8_t readingCount;

  File currentDayFile;
public:
  FileProcessor() {

  }
  void init() {
    state = STATE_IDLE;
    cardAvailable = false;
    if (!SD.begin(SD_PIN)) {
      Serial.println("SD Card Inaccessible");
    } else {
      Serial.println("SD Card Accessed");
      cardAvailable = true;
    }
  }
  void openAppropiateFile()  {
    if ( !cardAvailable ) return;
    // Get the correct time
    // Find the month folder
    // Find the day file
    currentDayFile = SD.open("temp.txt", FILE_WRITE);
  }
  void pushData (float reading, double lat, double lon, float elevation)  {
    // if ( !cardAvailable ) return;
    readingTotal += reading;
    latTotal += lat;
    lonTotal += lon;
    elevationTotal += elevation;
    readingCount ++;
    Serial.print(readingTotal);
    Serial.print(", ");

    Serial.print(latTotal);
    Serial.print(", ");

    Serial.print(lonTotal);
    Serial.print(", ");
    Serial.print(elevationTotal);
    Serial.print(", ");
    Serial.print(readingCount);

  }
  void storeAverageData(long minuteTime, uint8_t microclimate) {
    double readingAvg = readingTotal / readingCount;
    double latAvg = latTotal / readingCount;
    double lonAvg = lonTotal / readingCount;
    double elevationAvg = elevationAvg / readingCount;
    readingTotal = latTotal = lonTotal = elevationTotal = 0.0;
    readingCount = 0;

    // if ( currentDayFile ) {
      // DateTime now = StateManager.instance()->getDateTime();
      DateTime now(minuteTime);
      Serial.print(now.year(), DEC);
      Serial.print("/");
      Serial.print(now.month(), DEC);
      Serial.print("/");
      Serial.print(now.day(), DEC);
      Serial.print(",");

      Serial.print(now.hour(), DEC);
      Serial.print(":");
      Serial.print(now.minute(), DEC);
      Serial.print(":");
      Serial.print(now.second(), DEC);
      Serial.print(",");

      Serial.print(now.unixtime());
      Serial.print(",");

      Serial.print(readingAvg);
      Serial.print(",");
      Serial.print(microclimate);
      Serial.print(",");
      Serial.print(latAvg);
      Serial.print(",");
      Serial.print(lonAvg);
      Serial.print(",");
      Serial.print(elevationAvg);
      Serial.print(",");
      Serial.println("0");
    // }
  }
};

#endif // _FILE_PROCESSOR_H_
