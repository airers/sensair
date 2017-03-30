//Including libraries to connect RTC by Wire and I2C
#include <Wire.h>
#include "RTClib.h"
#include <SoftwareSerial.h>

#include "classes/CommandProcessor.h"
#include "classes/CommandProcessor.cpp"
#include "classes/StateManager.h"
#include "classes/FileProcessor.h"



//Defining pins for Sharp sensor
#define MEASURE_PIN A0
#define LED_POWER_PIN 8

unsigned int SAMPLING_TIME = 280; //Datasheet: Duration before measuring the ouput signal (after switching on LED): 280 µs
unsigned int deltaTime = 40; //Datasheet: Duration of the whole excitation pulse: 320 µs; Duration before switching off LED: 40 µs
unsigned int sleepTime = 9680; //Datasheet: Pulse Cycle: 10ms; Remaining time: 10,000 - 320 = 9680 µs

#define BLUETOOTH_RX 7
#define BLUETOOTH_TX 9

SoftwareSerial btSerial(BLUETOOTH_RX, BLUETOOTH_TX);
StateManager stateManager;
FileProcessor fileProcessor;

// Required for the file operations
long currentTime;
long nextMinuteTime;

long readingIterator = 1490816240;

long calculateNextMinute() {
  DateTime currentDateTime(currentTime);
  return currentDateTime.unixtime() + (60 - currentDateTime.second());
}

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

bool processLine(char * buffer) {
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

    for ( int a = 0 ; a < 25; a++ ) {
      Serial.print(packet[a]);
      Serial.print(" ");
    }
    Serial.println();
    free(packet);
    free(temp);
    return true;
  } else {
    free(temp);
    return false;
  }
}


void sendSomePackets() {
  if ( readingIterator == 0 ) {
    return;
  }
  Serial.print("Sending packets from: ");
  Serial.println(readingIterator);
  char * filename = FileProcessor::timestampToFilename(readingIterator);
  if ( SD.exists(filename) ) {
    Serial.print(filename);
    Serial.println(" exists");

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
        if ( processLine(buffer) ) {
          lines++;
        };
        if ( lines >= 5 ) {
          break;
        }
      }
      if ( lines == 0 ) {
        Serial.println("No more packets");
        readingIterator = 0;
      }
      currentFile.close();
      Serial.print("Lines: ");
      Serial.println(lines);
      free(buffer);
    }
    long duration = millis() - startTime;
    Serial.print("Duration: ");
    Serial.println(duration);

  } else {
    Serial.print(filename);
    Serial.println(" does not exist.");
  }
}

void setup() {
  Serial.begin(9600); //Setting the speed of communication in bits per second; Arduino default: 9600
  btSerial.begin(9600);
  Wire.begin();

  stateManager.init();
  fileProcessor.init();

  currentTime = stateManager.getTimeStamp();
  nextMinuteTime = calculateNextMinute();

  pinMode(LED_POWER_PIN ,OUTPUT); //Configures the digital pin as an output (to set it at 0V and 5V per cycle; turning on and off the LED
  pinMode(10, OUTPUT); //Configures the pin of the SD card reader as an output

  /**
   * Tests show that it takes about 1.9ms to read a single line from a file
   * 2.4ms to do conversion of data
   * 109.5ms to write a single line to the BT buffer
   * This means that when reading from a file of 1440 elements,
   * it wil take 2.7s to scan the entire file.
   * This will result in reading loses of 2 seconds.
   * This is not a problem, just something to take note.
   */

}


void loop() {
  sendSomePackets();
  long loopTime = stateManager.getTimeStamp();
  if ( loopTime > currentTime ) {
    currentTime = loopTime;

    float voMeasured = 0;
    float calcVoltage = 0;
    float dustDensity = 0;

    digitalWrite(LED_POWER_PIN ,LOW); //Turning on the LED; sinking current (i.e. light the LED connected through a series reistor to 5V)
    delayMicroseconds(SAMPLING_TIME); //Duration of sampling

    voMeasured = analogRead(MEASURE_PIN); //Reading the voltage measured

    delayMicroseconds(deltaTime); //Completing excitation pulse
    digitalWrite(LED_POWER_PIN ,HIGH); //Turning off the LED
    delayMicroseconds(sleepTime); //Delay before next reading

    calcVoltage = voMeasured*(5.0/1024); //0-5V mapped to 0 - 1023 integer values for real voltage value
    dustDensity = 0.17*calcVoltage-0.1; //Datasheet: Calibration curve

    fileProcessor.pushData(dustDensity, 1.35432101, 103.8765432, 0);
    // Average past minute readings & save as previous minute
    // fileProcessor.openAppropiateFile(currentTime);
    // fileProcessor.storeAverageData(currentTime, stateManager.microclimate);
  }
  if ( currentTime >= nextMinuteTime ) {
    long prevMinuteTime = nextMinuteTime - 60;
    Serial.print("Averaging Readings for: ");
    Serial.println(prevMinuteTime);

    // Average past minute readings & save as previous minute
    fileProcessor.openAppropiateFile(prevMinuteTime);
    fileProcessor.storeAverageData(prevMinuteTime, stateManager.microclimate);
    nextMinuteTime = calculateNextMinute();
  }


  // Process the incoming bluetooth packets
  byte type = 0;
  uint8_t len = 0;
  while ( btSerial.available() ) {
    type = btSerial.read();
    if ( btSerial.available() ) {
      len = btSerial.read();
    }
    byte data [len];
    int i = 0;
    while ( i < len && btSerial.available() ) {
      data[i] = btSerial.read();
      i++;
    }

    CommandProcessor::processPacket(type, len, data, btSerial, stateManager);

  }

  /**
   * Arduino date format
   * 1490821200
   * 80 32 220 88
   * Wed, 29 Mar 2017 21:00:00 GMT
   *
   * Android date format
   * 1490793754
   * 88 219 181 26
   */


  delay(200); //Time interval before each printed reading
}
