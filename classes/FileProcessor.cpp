#include "FileProcessor.h"


FileProcessor::FileProcessor() {
  state = STATE_IDLE;
  cardAvailable = false;
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card Inaccessible");
  } else {
    Serial.println("SD Card Accessed");
    cardAvailable = true;
  }
}

FileProcessor * FileProcessor::instance() {
  if ( !_singleton ) _singleton = new FileProcessor();
  return _singleton;
}

void FileProcessor::openAppropiateFile() {
  if ( !cardAvailable ) return;
  // Get the correct time
  // Find the month folder
  // Find the day file
  currentDayFile = SD.open("temp.txt", FILE_WRITE);
}

void FileProcessor::pushData (float reading, double lat, double lon, float elevation) {
  if ( !cardAvailable ) return;
  readingTotal += reading;
  latTotal += lat;
  lonTotal += lon;
  elevationTotal += elevation;
  readingCount ++;
}
void FileProcessor::storeAverageData() {
  double readingAvg = readingTotal / readingCount;
  double latAvg = latTotal / readingCount;
  double lonAvg = lonTotal / readingCount;
  double elevationAvg = elevationAvg / readingCount;
  readingTotal = latTotal = lonTotal = elevationTotal = 0.0;
  readingCount = 0;

  if ( currentDayFile ) {
    DateTime now = StateManager.instance()->getDateTime();
    dataFile.print(now.year(), DEC);
    dataFile.print("/");
    dataFile.print(now.month(), DEC);
    dataFile.print("/");
    dataFile.print(now.day(), DEC);
    dataFile.print(",");

    dataFile.print(now.hour(), DEC);
    dataFile.print(":");
    dataFile.print(now.minute(), DEC);
    dataFile.print(":");
    dataFile.print(now.second(), DEC);
    dataFile.print(",");

    dataFile.print(now.unixtime());
    dataFile.print(",");

    dataFile.print(readingAvg);
    dataFile.print(",");
    dataFile.print(StateManager.instance()->getMicroclimate());
    dataFile.print(",");
    dataFile.print(latAvg);
    dataFile.print(",");
    dataFile.print(lonAvg);
    dataFile.print(",");
    dataFile.print(elevationAvg);
    dataFile.print(",");
    dataFile.println("0");

  }
}
