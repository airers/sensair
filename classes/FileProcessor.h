/**
 * Class to process data in SD cards
 */
#ifndef _FILE_PROCESSOR_H_
#define _FILE_PROCESSOR_H_

#include <Arduino.h>
#include <SD.h>

class Reading { // 25 bytes
  float reading; // 4 bytes
  double lat; // 8 bytes
  double lon; // 8 bytes
  float elevation; // 4 bytes
  uint_8_t microclimate; // 1 byte
}
class FileProcessor {
private:
  static FileProcessor * _singleton;

  const int chipSelect = 10; //Defining the pin for the SD Card reader
  bool cardAvailable;

  // Storing 60 readings is not feasible,
  // will take up 1500 bytes.

  double readingTotal;
  double latTotal;
  double lonTotal;
  double elevationTotal;
  uint_8_t readingCount;

  File currentDayFile;

  FileProcessor();

public:
  static FileProcessor * instance();

  void openAppropiateFile();
  void pushData (float reading, double lat, double lon, float elevation);
  void storeAverageData();

};

#endif // _FILE_PROCESSOR_H_
