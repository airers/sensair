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
}


void loop() {

  long loopTime = stateManager.getTimeStamp();
  if ( loopTime > currentTime ) {
    currentTime = loopTime;
    // Get reading
    Serial.print("Getting Reading at: ");
    Serial.println(currentTime);

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

  }
  if ( currentTime >= nextMinuteTime ) {
    Serial.print("Averaging Readings for: ");
    Serial.println(nextMinuteTime - 60);

    // Average past minute readings & save as previous minute
    fileProcessor.storeAverageData(nextMinuteTime - 60, stateManager.microclimate);

    nextMinuteTime = calculateNextMinute();
  }



//
//  Serial.print("Dust Density: ");
//  Serial.print(dustDensity); // unit: mg/m3
//  Serial.println(";");

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

//    while ( btSerial.available() ) { // Flush the buffer
//      Serial.println(btSerial.read());
//    }
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
//   long_u timeLong;
//   timeLong.data = stateManager.getTimeStamp();
//  //
//   Serial.println(timeLong.data);
  // Serial.print((uint8_t)timeLong.bytes[0]);
  // Serial.print(" ");
  // Serial.print((uint8_t)timeLong.bytes[1]);
  // Serial.print(" ");
  // Serial.print((uint8_t)timeLong.bytes[2]);
  // Serial.print(" ");
  // Serial.println((uint8_t)timeLong.bytes[3]);

  //Printing readings to Serial Monitor
  // btSerial.print("Date: ");
  // btSerial.print(now.day(), DEC);
  // btSerial.print("/");
  // btSerial.print(now.month(), DEC);
  // btSerial.print("/");
  // btSerial.print(now.year(), DEC);
  // btSerial.print(";");
  //
  // btSerial.print(" Time: ");
  // btSerial.print(now.hour(), DEC);
  // btSerial.print(":");
  // btSerial.print(now.minute(), DEC);
  // btSerial.print(":");
  // btSerial.print(now.second(), DEC);
  // btSerial.print(";");
  //
  // btSerial.print(" Timestamp: ");
  // btSerial.print(now.unixtime());
  // btSerial.print(";");
  //
  // btSerial.print(" Raw Signal Value (0-1023): ");
  // btSerial.print(voMeasured);
  // btSerial.print(";");
  //
  // btSerial.print(" Voltage: ");
  // btSerial.print(calcVoltage);
  // btSerial.print(";");
  //
//  btSerial.print("Dust Density: ");
//  btSerial.print(dustDensity); // unit: mg/m3
//  btSerial.println(";");


  delay(200); //Time interval before each printed reading

  //Printing readings to an SD card file
  // File dataFile = SD.open("LOG.TXT", FILE_WRITE); //Writes a LOG file if one is not available
  // if (dataFile) {
//    DateTime now = stateManager.getDateTime();
//    dataFile.print("Date: ");
//    dataFile.print(now.year(), DEC);
//    dataFile.print("/");
//    dataFile.print(now.month(), DEC);
//    dataFile.print("/");
//    dataFile.print(now.day(), DEC);
//    dataFile.print(";");
//
//    dataFile.print(" Time: ");
//    dataFile.print(now.hour(), DEC);
//    dataFile.print(":");
//    dataFile.print(now.minute(), DEC);
//    dataFile.print(":");
//    dataFile.print(now.second(), DEC);
//    dataFile.print(";");
//
//    dataFile.print(" Timestamp: ");
//    dataFile.print(now.unixtime());
//    dataFile.print(";");
//
//    dataFile.print(" RawSignalValue(0-1023): ");
//    dataFile.print(voMeasured);
//    dataFile.print(";");
//
//    dataFile.print(" Voltage: ");
//    dataFile.print(calcVoltage);
//    dataFile.print(";");
//
//    dataFile.print(" DustDensity: ");
//    dataFile.print(dustDensity); // unit: mg/m3
//    dataFile.println(";");
//    dataFile.close();

  // } else {
    // Serial.println("SD Card Error");
  // }

}
