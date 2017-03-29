/**
 * Class to process data in SD cards
 */
#ifndef _FILE_PROCESSOR_H_
#define _FILE_PROCESSOR_H_

#include <Arduino.h>
#include <SD.h>
#include "RTClib.h"


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
  //TODO: Accuracy calculations
  float lonMin, lonMax, latMin, latMax;
  double elevationTotal;
  uint8_t readingCount;

  File currentDayFile;
public:
  static char* timestampToFilename(long timestamp) {
    char* filename = "YYMMMDD.txt";
    DateTime dateTime(timestamp);
    {
      String yearS(dateTime.year());
      filename[0] = yearS[2];
      filename[1] = yearS[3];
    }
    {
      char* l;
      int index = dateTime.month() - 1;
      {l = "JFMAMJJASOND"; filename[2] = l[index];}
      {l = "AEAPAUUUECOE"; filename[3] = l[index];}
      {l = "NBRRYNLGPTVC"; filename[4] = l[index];}
    }
    {
      String dayS(dateTime.day());
      if ( dayS.length() == 1 ) {
        filename[5] = '0';
        filename[6] = dayS[0];
      } else {
        filename[5] = dayS[0];
        filename[6] = dayS[1];
      }
    }
    return filename;
  }
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

  void pushData (float reading, double lat, double lon, float elevation)  {
    readingTotal += reading;
    latTotal += lat;
    lonTotal += lon;
    elevationTotal += elevation;
    readingCount++;
    // Accuracy
    float latF = lat;
    float lonF = lat;
    if ( latMin == 0 && latMax == 0 ) latMin = latMax = latF;
    if ( lonMin == 0 && lonMax == 0 ) lonMin = lonMax = lonF;
    if ( latMin > latF ) latMin = latF;
    if ( latMax < latF ) latMax = latF;
    if ( lonMin > lonF ) lonMin = lonF;
    if ( lonMax < lonF ) lonMax = lonF;
  }

  void openAppropiateFile(long timestamp)  {
    if ( !cardAvailable ) return;
    char * filename = FileProcessor::timestampToFilename(timestamp);
    currentDayFile = SD.open(filename, FILE_WRITE);
    if ( currentDayFile ) {
      Serial.print("Storing data in ");
      Serial.println(filename);
    }
  }

  void storeAverageData(long minuteTime, uint8_t microclimate) {
    double readingAvg = readingTotal / readingCount;
    double latAvg = latTotal / readingCount;
    double lonAvg = lonTotal / readingCount;
    double elevationAvg = elevationAvg / readingCount;
    float accuracy = ((latMax-latMin) * (latMax-latMin)) + ((lonMax-lonMin) * (lonMax-lonMin));
    readingTotal = latTotal = lonTotal = elevationTotal = 0.0;
    lonMin = lonMax = latMin = latMax = 0.0;
    readingCount = 0;

    if ( currentDayFile ) {
      DateTime now(minuteTime);
      currentDayFile.print(now.hour(), DEC);
      currentDayFile.print(":");
      currentDayFile.print(now.minute(), DEC);
      currentDayFile.print(":");
      currentDayFile.print(now.second(), DEC);
      currentDayFile.print(",");

      currentDayFile.print(now.unixtime());
      currentDayFile.print(",");

      currentDayFile.print(readingAvg);
      currentDayFile.print(",");
      currentDayFile.print(microclimate);
      currentDayFile.print(",");
      currentDayFile.print(latAvg);
      currentDayFile.print(",");
      currentDayFile.print(lonAvg);
      currentDayFile.print(",");
      currentDayFile.print(elevationAvg);
      currentDayFile.print(",");
      currentDayFile.println(accuracy);
      currentDayFile.close();
    }
  }
};

#endif // _FILE_PROCESSOR_H_
