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

long calculateNextMinute() {
  DateTime currentDateTime(currentTime);
  return currentDateTime.unixtime() + (60 - currentDateTime.second());
}

void processLine(char * buffer) {
  int i = 0;
  // Move to first comma
  while(buffer[i] != ',') i++;
  i++;
  // Extract timestamp from second comma to third comma
  int startUnix = i;
  while(buffer[i] != ',') i++;
  int endUnix = i;
  i++;
  char unix[endUnix - startUnix + 1];
  for ( int j = startUnix, itr = 0; j < endUnix; j++, itr++ ) {
    unix[itr] = buffer[j];
  }
  unix[endUnix - startUnix] = NULL;
  long unixTimestamp = atol(unix);
  // delay(10000);
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
   * This means that when reading from a file of 1440 elements,
   * it wil take 2.7s to scan the entire file.
   * This will result in reading loses of 2 seconds.
   * This is not a problem, just something to take note.
   */
  long readingIterator = 1490816240;

  char * filename = FileProcessor::timestampToFilename(readingIterator);
  if ( SD.exists(filename) ) {
    Serial.print(filename);
    Serial.println(" exists");

    long start = millis();
    File currentFile = SD.open(filename, FILE_READ);
    char buffer[100];
    if ( currentFile ) {
      int lines = 0;
      while ( currentFile.available() ) {
        char r = NULL;
        int i = 0;
        while ( r != '\n' ) {
          if ( !currentFile.available() ) break;
          r = currentFile.read();
          buffer[i] = r;
          i++;
        }
        buffer[i] = NULL;
        // Serial.println(buffer);
        // Process a line
        processLine(buffer);
        lines ++ ;
      }
      Serial.print("Lines: ");
      Serial.println(lines);

    }
    long duration = millis() - start;
    Serial.println(duration);

  } else {
    Serial.print(filename);
    Serial.println(" does not exist.");
  }
}


void loop() {
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

    fileProcessor.pushData(dustDensity, 1.35, 103.8, 0);
    // Average past minute readings & save as previous minute
    // fileProcessor.openAppropiateFile(currentTime);
    // fileProcessor.storeAverageData(currentTime, stateManager.microclimate);
    // nextMinuteTime = calculateNextMinute();
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
