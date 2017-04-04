//Including library for TFT Screen
//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library
//#include <SPI.h>

//Including library for DHT22 (Temp and RH sensor)
#include <DHT.h>;

//Including library for GPS
#include <TinyGPS++.h>

//Including libraries to connect RTC by Wire and I2C
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

#include "classes/CommandProcessor.h"
#include "classes/CommandProcessor.cpp"
#include "classes/StateManager.h"
#include "classes/FileProcessor.h"
#include "classes/EEPROMVariables.h"


//Defining pins for Sharp sensor
#define MEASURE_PIN A0
#define LED_POWER_PIN 8

// Datasheet: Duration before measuring the ouput signal (after switching on LED): 280 µs
#define SAMPLING_TIME   280
// Datasheet: Duration of the whole excitation pulse: 320 µs; Duration before switching off LED: 40 µs
#define DELTA_TIME      40
// Datasheet: Pulse Cycle: 10ms; Remaining time: 10,000 - 320 = 9680 µs
#define SLEEP_TIME      9680

#define BLUETOOTH_RX 7
#define BLUETOOTH_TX 9

#define BAUD_RATE     9600

//Defining pins for TFT Screen
#define TFT_CS     6
#define TFT_RST    8
#define TFT_DC     7
#define TFT_SCLK 5
#define TFT_MOSI 4


//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
SoftwareSerial btSerial(BLUETOOTH_RX, BLUETOOTH_TX);
StateManager stateManager;
FileProcessor fileProcessor;

//Defining pins for DHT22
#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal

//Defining pins for GPS
SoftwareSerial gpsSerial(3, 2); //RX=pin 10, TX=pin 11
TinyGPSPlus gps;//This is the GPS object that will pretty much do all the grunt work with the NMEA data

unsigned int samplingTime = 280; //Datasheet: Duration before measuring the ouput signal (after switching on LED): 280 µs
unsigned int deltaTime = 40; //Datasheet: Duration of the whole excitation pulse: 320 µs; Duration before switching off LED: 40 µs
unsigned int sleepTime = 9680; //Datasheet: Pulse Cycle: 10ms; Remaining time: 10,000 - 320 = 9680 µs

//Variables for DHT22
int chk;
float hum;
float temp;

// Required for the file operations
// long currentTime;
// long nextMinuteTime;

long calculateNextMinute() {
  DateTime currentDateTime(ROMVar::getCurrentTime());
  return currentDateTime.unixtime() + (60 - currentDateTime.second());
}


void setup() {
  Serial.begin(BAUD_RATE); //Setting the speed of communication in bits per second; Arduino default: 9600
  btSerial.begin(BAUD_RATE);
  gpsSerial.begin(BAUD_RATE);//This opens up communications to the GPS

  Wire.begin();
  dht.begin();

  stateManager.init();
  fileProcessor.init();

  ROMVar::setCurrentTime(stateManager.getTimeStamp());
  ROMVar::setNextMinuteTime(calculateNextMinute());

  pinMode(LED_POWER_PIN ,OUTPUT); //Configures the digital pin as an output (to set it at 0V and 5V per cycle; turning on and off the LED
  pinMode(10, OUTPUT); //Configures the pin of the SD card reader as an output


//  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
//  tft.fillScreen(ST7735_BLACK);

  /**
   * Tests show that it takes about 1.9ms to read a single line from a file
   * 2.4ms to do conversion of data
   * 109.5ms to write a single line to the BT buffer
   * This means that when reading from a file of 1440 elements,
   * it wil take 2.7s to scan the entire file.
   * This will result in reading loses of 2 seconds.
   * This is not a problem, just something to take note.
   */
  // uint16_t count = fileProcessor.countPackets2(1490830040);
  // Serial.println(count);
  // count = fileProcessor.countPackets2(1490830040);
  // Serial.println(count);
  // count = fileProcessor.countPackets2(1490830040);
  // Serial.println(count);
  // count = fileProcessor.countPackets2(1490830040);
  // Serial.println(count);
  // fileProcessor.startSendingData(1490830040, 100);
  Serial.println(fileProcessor.getFirstReading());
  Serial.write(C_S);
  Serial.write(C_T);
  Serial.write(C_A);
  Serial.write(C_R);
  Serial.write(C_T);
  Serial.write(C_LR);
}


void loop() {
  // Process the incoming bluetooth packets
  {
    byte type = 0;
    uint8_t len = 0;
    btSerial.listen();
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

      Serial.println(len);
      CommandProcessor::processPacket(type, len, data, btSerial, stateManager, fileProcessor);
    }
  }

  // Process GPS packets
  {
    // gpsSerial.listen();
    // while(gpsSerial.available()) { //While there are characters to come from the GPS
    //   gps.encode(gpsSerial.read());//This feeds the serial NMEA data into the library one char at a time
    // }

    // {
    //   //Get the latest info from the gps object which it derived from the data sent by the GPS unit

    //   // Serial.println("Satellite Count:");
    //   // Serial.println(gps.satellites.value());
    //   // Serial.println("Latitude:");
    //   // Serial.println(gps.location.lat(), 6);
    //   // Serial.println("Longitude:");
    //   // Serial.println(gps.location.lng(), 6);
    // }
  }
  {
    long loopTime = stateManager.getTimeStamp();
    long currentTime = ROMVar::getCurrentTime();
    long nextMinuteTime = ROMVar::getNextMinuteTime();

    if ( loopTime > currentTime ) {
      currentTime = loopTime;

      float voMeasured = 0;
      float calcVoltage = 0;
      float dustDensity = 0;

      digitalWrite(LED_POWER_PIN ,LOW); //Turning on the LED; sinking current (i.e. light the LED connected through a series reistor to 5V)
      delayMicroseconds(SAMPLING_TIME); //Duration of sampling

      voMeasured = analogRead(MEASURE_PIN); //Reading the voltage measured

      delayMicroseconds(DELTA_TIME); //Completing excitation pulse
      digitalWrite(LED_POWER_PIN ,HIGH); //Turning off the LED
      delayMicroseconds(SLEEP_TIME); //Delay before next reading

      calcVoltage = voMeasured*(5.0/1024); //0-5V mapped to 0 - 1023 integer values for real voltage value
      dustDensity = 0.17*calcVoltage-0.1; //Datasheet: Calibration curve
      //Read data and store it to variables hum and temp
      hum = dht.readHumidity();
      temp = dht.readTemperature();

      // TODO: Not hardcode the microclimate and location
      fileProcessor.pushData(dustDensity, 1.35432101, 103.8765432, 0);
    }
    if ( currentTime >= nextMinuteTime ) {
      long prevMinuteTime = nextMinuteTime - 60;

      // Average past minute readings & save as previous minute
      fileProcessor.openAppropiateFile(prevMinuteTime);
      fileProcessor.storeAverageData(prevMinuteTime, stateManager.microclimate);
      nextMinuteTime = calculateNextMinute();
    }

    ROMVar::setCurrentTime(currentTime);
    ROMVar::setNextMinuteTime(nextMinuteTime);
  }

  // Send packets if there's something requesting it
  fileProcessor.sendSomePackets(btSerial);

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

//  {
//
//    //Printing data onto TFT
//    tft.setTextColor(ST7735_fWHITE);
//    tft.setCursor(20,10);
//    tft.setTextSize(2);
//    tft.println("SENSAIR");
//    tft.setTextSize(1);
//
//    tft.setCursor(0, 40);
//    tft.drawLine(0, 30, tft.width()-1, 30, ST7735_WHITE); //draw line separator
//    tft.setTextColor(ST7735_YELLOW);
//    tft.print(" PM2.5 Conc :");
//    tft.setCursor(80, 40);
//    tft.setTextColor(ST7735_GREEN);
////    tft.println((float)dustDensity);
//
//    tft.setCursor(0, 50);
//    tft.setTextColor(ST7735_YELLOW);
//    tft.print(" Humidity(%) :");
//    tft.setCursor(80, 50);
//    tft.setTextColor(ST7735_GREEN);
//    tft.println((float)hum);
//
//    tft.setCursor(0, 60);
//    tft.setTextColor(ST7735_YELLOW);
//    tft.print(" Temp :");
//    tft.setCursor(80, 60);
//    tft.setTextColor(ST7735_GREEN);
//    tft.println((float)temp);
//
//    tft.drawLine(0, 70, tft.width()-1, 70, ST7735_WHITE); //draw line separator
//
//    delay(60000);
//
//    tft.fillRect(80,40,40,18,ST7735_BLACK);
//    tft.fillRect(80,50,40,18,ST7735_BLACK);
//    tft.fillRect(80,60,40,18,ST7735_BLACK);
//  }

  delay(50);
}
