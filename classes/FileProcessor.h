/**
 * Class to process data in SD cards
 */
#ifndef _FILE_PROCESSOR_H_
#define _FILE_PROCESSOR_H_

#include <Arduino.h>
#include <SD.h>
#include "RTClib.h"
#include "EEPROMVariables.h"

// #define CMD_READING_COUNT         8


//Defining the pin for the SD Card reader
#define SD_PIN 10

class FileProcessor {
private:
  bool cardAvailable;

  // Reading totals and accuracy calculations are now
  // stored in the EEPROM Memory
  // float readingTotal, latTotal, lonTotal, eleTotal;
  // float latMin, latMax, lonMin, lonMax;
  uint8_t readingCount;  // 4 btytes

  // Used for writing files
  File currentDayFile;
public:
  // Used for reading files
  // If the iterator is 0, it means the program is not sending files.
  long readingIterator = 0; //1490816240;
  uint8_t packetsToSend = 0;

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
    float zero = 0;
    ROMVar::setAccuracyVars(zero,zero,zero,zero);
    ROMVar::setAverageVars(zero,zero,zero,zero);
    readingIterator = 0;
    packetsToSend = 0;
    cardAvailable = false;
    if (!SD.begin(SD_PIN)) {
      // -Serial.println("SD Card Inaccessible");
    } else {
      // -Serial.println("SD Card Accessed");
      cardAvailable = true;
    }
  }

  void pushData (float reading, float lat, float lon, float elevation)  {
    float readingTotal, latTotal, lonTotal, eleTotal;
    float latMin, latMax, lonMin, lonMax;
    ROMVar::getAverageVars(readingTotal, latTotal, lonTotal, eleTotal);
    ROMVar::getAccuracyVars(lonMin, lonMax, latMin, latMax);

    readingTotal += reading;
    latTotal += lat;
    lonTotal += lon;
    eleTotal += elevation;
    // Accuracy
    if ( latMin == 0 && latMax == 0 ) latMin = latMax = lat;
    if ( lonMin == 0 && lonMax == 0 ) lonMin = lonMax = lon;
    if ( latMin > lat ) latMin = lat;
    if ( latMax < lat ) latMax = lat;
    if ( lonMin > lon ) lonMin = lon;
    if ( lonMax < lon ) lonMax = lon;

    readingCount++;

    ROMVar::setAverageVars(readingTotal, latTotal, lonTotal, eleTotal);
    ROMVar::setAccuracyVars(lonMin, lonMax, latMin, latMax);

  }

  void openAppropiateFile(long timestamp)  {
    if ( !cardAvailable ) return;
    char * filename = FileProcessor::timestampToFilename(timestamp);
    currentDayFile = SD.open(filename, FILE_WRITE);
    if ( currentDayFile ) {
      // -Serial.print("Storing data in ");
      // -Serial.println(filename);
    }
  }

  void storeAverageData(long minuteTime, uint8_t microclimate) {
    float readingTotal, latTotal, lonTotal, eleTotal;
    float latMin, latMax, lonMin, lonMax;
    ROMVar::getAverageVars(readingTotal, latTotal, lonTotal, eleTotal);
    ROMVar::getAccuracyVars(lonMin, lonMax, latMin, latMax);

    float readingAvg = readingTotal / readingCount;
    float latAvg = latTotal / readingCount;
    float lonAvg = lonTotal / readingCount;
    float elevationAvg = elevationAvg / readingCount;
    float accuracy = ((latMax-latMin) * (latMax-latMin)) + ((lonMax-lonMin) * (lonMax-lonMin));
    float zero = 0;
    ROMVar::setAccuracyVars(zero,zero,zero,zero);
    ROMVar::setAverageVars(zero,zero,zero,zero);
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
      Serial.println(timestamp);
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

  uint16_t countPackets(long from) {
    // // -Serial.print("Start counting ");
    // ROMVar::printFreeRam();
    // -Serial.println("Counting packets:");
    uint16_t count = 0;
    long startTime = millis();
    long countTimeIterator = from;
    char * filename = FileProcessor::timestampToFilename(countTimeIterator);
    char * buffer = (char*)malloc(100);
    char * temp = (char*)malloc(15);
    while (SD.exists(filename)) {
      // -Serial.print("Reading file:");
      // -Serial.print(countTimeIterator);
      // -Serial.print(" ");
      // -Serial.println(filename);
      // ROMVar::printFreeRam();

      Serial.write(C_F);
      Serial.write(C_LR);

      Serial.print(countTimeIterator);
      Serial.write(C_SP);
      Serial.println(filename);

      File currentFile = SD.open(filename, FILE_READ);
      bool matched = false;
      if ( currentFile ) {
        // -Serial.println("File read");
        Serial.write(C_R);
        Serial.write(C_LR);
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
            getSplitSection(temp, buffer, 1);
            long timestamp = atol(temp);
            Serial.println(timestamp);
            if ( timestamp >= countTimeIterator ) {
              Serial.write(C_M);
              Serial.write(C_LR);
              matched = true;
              count++;
            }

          }
        }
        currentFile.close();
        free(&currentFile);
      } else {
        break;
      }

      // // -Serial.print("End counting");

      countTimeIterator = FileProcessor::getStartOfDay(countTimeIterator) + 86400;
      filename = FileProcessor::timestampToFilename(countTimeIterator);
    } // end while (SD.exists(filename));

    free(temp);
    free(buffer);
    free(filename);
    // long duration = millis() - startTime;
    // -Serial.print("Duration: ");
    // -Serial.println(duration);

    // ROMVar::printFreeRam();

    return count;
  }

  void startSendingData(long from, uint16_t count) {
    readingIterator = from;
    packetsToSend = count;
  }
  void sendSomePackets(SoftwareSerial &btSerial) {
    if ( readingIterator == 0 || packetsToSend == 0 ) {
      return;
    }

    char * filename = FileProcessor::timestampToFilename(readingIterator);
    if ( SD.exists(filename) ) {
      // -Serial.print("Sending packets from: ");
      Serial.write(C_S);
      Serial.write(C_SP);
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
            packetsToSend--;
            if ( packetsToSend == 0 || lines >= 5 ) {
              break;
            }
          };
        }
        if ( packetsToSend == 0 ) {
          readingIterator = 0;
        }
        if ( lines == 0 ) {
          // -Serial.print("No more packets ");
          readingIterator = FileProcessor::getStartOfDay(readingIterator) + 86400;
          // -Serial.println(readingIterator);
        }
        currentFile.close();
        free(&currentFile);
        free(buffer);
      }
      // long duration = millis() - startTime;
      // // -Serial.print("Duration: ");
      // // -Serial.println(duration);

    } else {
      // -Serial.print(filename);
      // -Serial.println(" does not exist.");
      readingIterator = 0;
    }
  }

};


#endif // _FILE_PROCESSOR_H_
