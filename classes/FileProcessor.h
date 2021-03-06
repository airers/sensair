/**
 * Class to process data in SD cards
 */
#ifndef _FILE_PROCESSOR_H_
#define _FILE_PROCESSOR_H_

#include <Arduino.h>
#include <SD.h>
// #include "RTClib.h"
#include "EEPROMVariables.h"

#include "Globals.h"



//Defining the pin for the SD Card reader
#define SD_PIN 20
#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
// Takes about 1040ms to send this many
#define READINGS_PER_PACKET 30

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
  long firstReading; // Timestamp for the first reading
  
  char * buffer = "                                                                                          ";
  char * temp = "               ";
public:
  // Used for reading files
  // If the iterator is 0, it means the program is not sending files.
  long readingIterator = 0; //1490816240;
  // The position in a file to start from so don't have to process
  // the packets that are not used
  long fileIterator = 0;
  
  // If the iterator is 0, it means the program is not counting packets.
  long countReadingIterator = 0;
  uint16_u sendCount;
  uint16_t packetsToSend = 0;

  static long getStartOfDay(long timestamp) {
    DateTime dateTime(timestamp);
    long seconds = dateTime.second();
    long minutes = dateTime.minute();
    long hours = dateTime.hour();
    return timestamp - (seconds) - (minutes * 60) - (hours * SECONDS_IN_HOUR);
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
    firstReading = ROMVar::getFirstReading();
    if (SD.begin(SD_PIN)) {
      cardAvailable = true;
      Serial.println("SD Working");
    } else {
      Serial.println("SD ERROR");
    }
  }

  bool isSendingData() {
    return readingIterator != 0;
  }
  
  bool isCounting() {
    return countReadingIterator != 0;
  }
  
  bool getCardAvailable() {
    return cardAvailable;
  }

  long setFirstReading(long time) {
    firstReading = time;
    ROMVar::setFirstReading(firstReading);
  }

  long getFirstReading() {
    return firstReading;
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

      Serial.print("Stored data: ");
      Serial.print(now.unixtime());
      Serial.print(" ");
      Serial.println(readingAvg);

      if ( firstReading <= 0 ) {
        setFirstReading(minuteTime);
      }
      ROMVar::setLatestReading(minuteTime);
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

/**
 * Counts the packets from a given timetamp to the latest timestamp
 * @method countPackets2
 * @param  from          start timestamp
 * @return               [description]
 */
  void countPackets2(long from) {
    stopCountingAndSending();
    countReadingIterator = from;
    Serial.print("Count Reading Iterator: ");
    Serial.println(countReadingIterator);
    sendCount.data = 0;
    Globals::stateManager->startCount();
  }

  void stopCountingAndSending() {
    if ( countReadingIterator != 0 && readingIterator != 0 ) {
      countReadingIterator = 0;
      readingIterator = 0;
      packetsToSend = 0;
      Globals::stateManager->end();
    }
  }
  
  void startSendingData(long from, uint16_t count) {
    stopCountingAndSending();
    fileIterator = 0;
    readingIterator = from;
    packetsToSend = count;
    Globals::stateManager->startSend();  
  }

  bool processLine(char * buffer, SoftwareSerial &btSerial) {
    // Line example:
    // 12:41:42,1490058484,1023.01,2,1.2952337,103.7858645,14.525,5.4246
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

      uint8_t command = CMD_READING_PACKET;
      btSerial.write(command);
      uint8_t packet_len = 25;
      btSerial.write(packet_len);
      for ( int a = 0 ; a < 25; a++ ) {
        btSerial.write(packet[a]);
      }
      btSerial.print("\r\n");
      free(packet);
      return true;
    } else {
      return false;
    }
  }

  void countSomePackets(SoftwareSerial &btSerial) {
    if ( countReadingIterator == 0 ) {
      return;
    } 
    long latestReading = ROMVar::getLatestReading();
    
    char * filename = FileProcessor::timestampToFilename(countReadingIterator);
    Serial.write(C_C);
    Serial.write(C_SP);
    Serial.println(filename);
    if ( !SD.exists(filename) ) {
      countReadingIterator = FileProcessor::getStartOfDay(countReadingIterator) + SECONDS_IN_DAY;
      Serial.println(countReadingIterator);
      Serial.println(Globals::stateManager->getTimeStamp());
      if ( countReadingIterator <= latestReading && countReadingIterator <= Globals::stateManager->getTimeStamp() ) {
        return;
      } else {
        countReadingIterator = 0;
        Serial.println("End of readings");
        btSerial.write(CMD_READING_COUNT);
        uint8_t packet_len = 2;
        btSerial.write(packet_len);
        btSerial.write(sendCount.bytes[0]);
        btSerial.write(sendCount.bytes[1]);
        btSerial.print("\r\n");
        
        // tft.fillRect(5,96,120,20, ST7735_BLACK);
        Globals::stateManager->end();
        return;
      }
    }
    
    Serial.print("Counting... ");
    

    File currentFile = SD.open(filename, FILE_READ);
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
        // Process a line
        getSplitSection(temp, buffer, 1);
        long timestamp = atol(temp);

        if ( timestamp >= countReadingIterator) {
          sendCount.data++;
          countReadingIterator = timestamp + 1;
        }
      }
      currentFile.close();
      countReadingIterator = FileProcessor::getStartOfDay(countReadingIterator) + SECONDS_IN_DAY;
    
      // tft.fillRect(5,106,120,10, ST7735_BLACK);
      // tft.setTextColor(ST7735_GREEN);
      // tft.setCursor(5, 96);
      // tft.print("Counting...");
      // tft.setCursor(5, 106);
      // tft.print(String(String(sendCount.data) + " readings"));
      
      Globals::stateManager->updateCount(sendCount.data);
      
      btSerial.write(CMD_READING_COUNTING);
      uint8_t packet_len = 2;
      btSerial.write(packet_len);
      btSerial.write(sendCount.bytes[0]);
      btSerial.write(sendCount.bytes[1]);
      btSerial.print("\r\n");
    }
    Serial.println(sendCount.data);
  }

  void sendSomePackets(SoftwareSerial &btSerial) {
    if ( readingIterator == 0 || packetsToSend == 0 ) {
      return;
    }

    long latestReading = ROMVar::getLatestReading();
    char * filename = FileProcessor::timestampToFilename(readingIterator);
    if ( !SD.exists(filename) ) { // Skip if file does not exist
      readingIterator = FileProcessor::getStartOfDay(readingIterator) + SECONDS_IN_DAY;
      fileIterator = 0;
      if ( readingIterator <= latestReading ) {
      } else {
        Globals::stateManager->end();
        readingIterator = 0;
      }
      return;
    }

    // -Serial.print("Sending packets from: ");
    Serial.write(C_S);
    Serial.write(C_SP);
    Serial.println(filename);

    // long startTime = millis();
    File currentFile = SD.open(filename, FILE_READ);
    char * buffer = (char*)malloc(90);
    if ( currentFile ) {
      int lines = 0;
      currentFile.seek(fileIterator); // Skip to the part from the previous read cycle
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
          if ( packetsToSend == 0 || lines >= READINGS_PER_PACKET ) {
            fileIterator = currentFile.position();
            break;
          }
        };
      }
      Globals::stateManager->updateSend(packetsToSend);
      Serial.println(packetsToSend);
      if ( packetsToSend == 0 ) { // Done sending files
        Globals::stateManager->end();
        readingIterator = 0; // Reset the iterator to indicate nothing is sending
      } else if ( lines < READINGS_PER_PACKET ) { // Reached end of file
        Serial.println("EOF");
        readingIterator = FileProcessor::getStartOfDay(readingIterator) + SECONDS_IN_DAY;
        fileIterator = 0;
      }
      currentFile.close();
      free(buffer);

      // long duration = millis() - startTime;
      // Serial.print("Duration: ");
      // Serial.println(duration);

    }
  }


};


#endif // _FILE_PROCESSOR_H_
