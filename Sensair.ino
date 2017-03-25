//Including libraries to connect RTC by Wire and I2C
#include <Wire.h>
#include "RTClib.h"
#include <SD.h>

#include "classes/CommandProcessor.h"
#include "classes/StateManager.h"

RTC_DS1307 rtc; //Defining the RTC

//Defining pins for Sharp sensor
int measurePin = A0;
int ledPower = 8;

unsigned int samplingTime = 280; //Datasheet: Duration before measuring the ouput signal (after switching on LED): 280 µs
unsigned int deltaTime = 40; //Datasheet: Duration of the whole excitation pulse: 320 µs; Duration before switching off LED: 40 µs
unsigned int sleepTime = 9680; //Datasheet: Pulse Cycle: 10ms; Remaining time: 10,000 - 320 = 9680 µs

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

void setup(){

  Serial.begin(9600); //Setting the speed of communication in bits per second; Arduino default: 9600
  Wire.begin();
  rtc.begin();

  pinMode(ledPower,OUTPUT); //Configures the digital pin as an output (to set it at 0V and 5V per cycle; turning on and off the LED
  //pinMode(10, OUTPUT); //Configures the pin of the SD card reader as an output

  Serial.println("Test Start"); //To indicate the start of a test interval
}

void loop(){

  DateTime now = rtc.now(); //Retrieving current date and time

  digitalWrite(ledPower,LOW); //Turning on the LED; sinking current (i.e. light the LED connected through a series reistor to 5V)
  delayMicroseconds(samplingTime); //Duration of sampling

  voMeasured = analogRead(measurePin); //Reading the voltage measured

  delayMicroseconds(deltaTime); //Completing excitation pulse
  digitalWrite(ledPower,HIGH); //Turning off the LED
  delayMicroseconds(sleepTime); //Delay before next reading

  calcVoltage = voMeasured*(5.0/1024); //0-5V mapped to 0 - 1023 integer values for real voltage value
  dustDensity = 0.17*calcVoltage-0.1; //Datasheet: Calibration curve

  //Printing readings to Serial Monitor
  Serial.print("Date: ");
  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.year(), DEC);
  Serial.print(";");

  Serial.print(" Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print(";");

  Serial.print(" Timestamp: ");
  Serial.print(now.unixtime());
  Serial.print(";");

  Serial.print(" Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
  Serial.print(";");

  Serial.print(" Voltage: ");
  Serial.print(calcVoltage);
  Serial.print(";");

  Serial.print(" Dust Density: ");
  Serial.print(dustDensity); // unit: mg/m3
  Serial.println(";");

  delay(1000); //Time interval before each printed reading

  //Printing readings to an SD card file
  File dataFile = SD.open("LOG.TXT", FILE_WRITE); //Writes a LOG file if one is not available
   if (dataFile) {
  dataFile.print("Date: ");
  dataFile.print(now.year(), DEC);
  dataFile.print("/");
  dataFile.print(now.month(), DEC);
  dataFile.print("/");
  dataFile.print(now.day(), DEC);
  dataFile.print(";");

  dataFile.print(" Time: ");
  dataFile.print(now.hour(), DEC);
  dataFile.print(":");
  dataFile.print(now.minute(), DEC);
  dataFile.print(":");
  dataFile.print(now.second(), DEC);
  dataFile.print(";");

  dataFile.print(" Timestamp: ");
  dataFile.print(now.unixtime());
  dataFile.print(";");

  dataFile.print(" RawSignalValue(0-1023): ");
  dataFile.print(voMeasured);
  dataFile.print(";");

  dataFile.print(" Voltage: ");
  dataFile.print(calcVoltage);
  dataFile.print(";");

  dataFile.print(" DustDensity: ");
  dataFile.print(dustDensity); // unit: mg/m3
  dataFile.println(";");
  dataFile.close();

  }

    else {
    Serial.println("SD Card Error");
  }

}
