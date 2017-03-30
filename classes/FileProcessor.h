/**
 * Class to process data in SD cards
 */
#ifndef _FILE_PROCESSOR_H_
#define _FILE_PROCESSOR_H_

#include <Arduino.h>
#include <SD.h>
#include "RTClib.h"

// #define CMD_READING_COUNT         8


//Defining the pin for the SD Card reader
#define SD_PIN 10

class FileProcessor {
private:
  bool cardAvailable;

  // Store the reading totals
  double readingTotal;
  double latTotal;
  double lonTotal;

  // Accuracy calculations
  float lonMin, lonMax, latMin, latMax;
  double elevationTotal;
  uint8_t readingCount;

  // Used for writing files
  File currentDayFile;
public:
  // Used for reading files
  // If the iterator is 0, it means the program is not sending files.
  long readingIterator = 0; //1490816240;

  static long getStartOfDay(long timestamp) {
    DateTime dateTime(timestamp);
    long seconds = dateTime.second();
    long minutes = dateTime.minute();
    long hours = dateTime.hour();
    return timestamp - (seconds) - (minutes * 60) - (hours * 3600);
  }

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
    readingIterator = 0;
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
      currentDayFile.print(latAvg,8);
      currentDayFile.print(",");
      currentDayFile.print(lonAvg,8);
      currentDayFile.print(",");
      currentDayFile.print(elevationAvg,2);
      currentDayFile.print(",");
      currentDayFile.print(accuracy,2);
      currentDayFile.write("\n");
      currentDayFile.close();
    }
  }



  // ============= Reading and sending file ==================
  /**
   * Simulates doing a input.split(',')[section]
   * @param  output          output memory space (must be allocated)
   * @param  input           input memory (char*) assumes it's a string
   * @param  section         which section you want after "splitting"
   */
  void getSplitSection(char* output, char * input, int section ) {
    int start = 0;
    int end = 0;
    int i = 0;
    for (int s = 0; s < section + 1; s++ ) {
      i++; // Move one index up. Assumes the first char is not a comma
      start = !end?0:end+1; // Set the start as the previous end + 1
      while(input[i] && input[i] != ',') i++; // Move iterator to next pointer
      end = i; // Set the end index
      if (!input[i]) break; // Break on null terminator
    }
    if ( start == end ) return;
    memcpy(output, input + start, end - start); // Do actual copying
    output[end - start] = '\0'; // Null terminate the string
  }

  bool processLine(char * buffer, SoftwareSerial &btSerial) {
    // Line example:
    // 12:41:42,1490058484,1023.01,2,1.2952337,103.7858645,14.525,5.4246
    char * temp = (char*)malloc(15);
    getSplitSection(temp, buffer, 1);
    long timestamp = atol(temp);

    if ( timestamp >= readingIterator) {
      readingIterator = timestamp + 1;
      byte * packet = (byte*)malloc(25);
      memcpy(packet, &timestamp, 4);

      getSplitSection(temp, buffer, 2);
      float read = atof(temp);
      memcpy(packet + 4, &read, 4);

      getSplitSection(temp, buffer, 3);
      uint8_t mic = atoi(temp);
      packet[24] = mic;

      getSplitSection(temp, buffer, 4);
      double lat = atof(temp);
      memcpy(packet + 8, &lat, 4);

      getSplitSection(temp, buffer, 5);
      double lon = atof(temp);
      memcpy(packet + 12, &lon, 4);

      getSplitSection(temp, buffer, 6);
      float ele = atof(temp);
      memcpy(packet + 16, &ele, 4);

      getSplitSection(temp, buffer, 7);
      float acc = atof(temp);
      memcpy(packet + 20, &acc, 4);

      uint8_t command = 8;
      btSerial.write(command);
      uint8_t packet_len = 25;
      btSerial.write(packet_len);
      for ( int a = 0 ; a < 25; a++ ) {
        btSerial.write(packet[a]);
      }
      btSerial.print("\r\n");
      free(packet);
      free(temp);
      return true;
    } else {
      free(temp);
      return false;
    }
  }

  int countPackets(long from) {
    Serial.println("Counting packets:");
    int count = 0;
    long startTime = millis();
    long timeIterator = from;
    char * filename = FileProcessor::timestampToFilename(timeIterator);
    char * buffer = (char*)malloc(90);

    while (SD.exists(filename)) {
      Serial.print("Reading file:");
      Serial.print(timeIterator);
      Serial.print(" ");
      Serial.println(filename);

      File currentFile = SD.open(filename, FILE_READ);
      bool matched = false;
      if ( currentFile ) {
        while ( currentFile.available() ) {
          char r = '\0';
          int i = 0;
          while ( r != '\n' ) {
            if ( !currentFile.available() ) break;
            r = currentFile.read();
            buffer[i] = r;
            i++;
          }
          buffer[i] = '\0';
          if (matched) {
            count++;
          } else {
            char * temp = (char*)malloc(15);
            getSplitSection(temp, buffer, 1);
            long timestamp = atol(temp);
            if ( timestamp >= from ) {
              matched = true;
              count++;
            }
            free(temp);
          }
        }
        Serial.println("File read");
      }
      currentFile.close();
      timeIterator += 86400;
      filename = FileProcessor::timestampToFilename(timeIterator);

    }
    long duration = millis() - startTime;
    Serial.print("Duration: ");
    Serial.println(duration);
    return count;
  }

  void startSendingData(long from) {
    readingIterator = from;
  }
  void sendSomePackets(SoftwareSerial &btSerial) {
    if ( readingIterator == 0 ) {
      return;
    }

    char * filename = FileProcessor::timestampToFilename(readingIterator);
    if ( SD.exists(filename) ) {
      Serial.print("Sending packets from: ");

      Serial.println(filename);

      long startTime = millis();
      File currentFile = SD.open(filename, FILE_READ);
      char * buffer = (char*)malloc(90);
      if ( currentFile ) {
        int lines = 0;
        while ( currentFile.available() ) {
          char r = '\0';
          int i = 0;
          while ( r != '\n' ) {
            if ( !currentFile.available() ) break;
            r = currentFile.read();
            buffer[i] = r;
            i++;
          }
          buffer[i] = '\0';
          // Process a line
          if ( processLine(buffer, btSerial) ) {
            lines++;
          };
          if ( lines >= 5 ) {
            break;
          }
        }
        if ( lines == 0 ) {
          Serial.print("No more packets ");
          readingIterator = FileProcessor::getStartOfDay(readingIterator) + 86400;
          Serial.println(readingIterator);
        }
        currentFile.close();
        free(buffer);
      }
      // long duration = millis() - startTime;
      // Serial.print("Duration: ");
      // Serial.println(duration);

    } else {
      Serial.print(filename);
      Serial.println(" does not exist.");
      readingIterator = 0;
    }
  }

};


#endif // _FILE_PROCESSOR_H_
