//Including library for TFT Screen
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library


// Including library for DHT22 (Temp and RH sensor)
#include <DHT.h>

//Including library for GPS
// #include <TinyGPS++.h>

//Including libraries to connect RTC by Wire and I2C
// #include <Wire.h>
// #include <RTClib.h>
#include <SoftwareSerial.h>
//
// #include "classes/CommandProcessor.h"
// #include "classes/CommandProcessor.cpp"
#include "classes/StateManager.h"
// #include "classes/FileProcessor.h"
#include "classes/EEPROMVariables.h"

// Temp used for testing
#include <SD.h>
#include <SPI.h>
// #include <DS1307RTC.h>
// #include <Time.h>
#include <Wire.h>

//Defining pins for DHT22
#define DHTPIN 17     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal

//Defining pins for Sharp sensor
#define MEASURE_PIN A0
#define LED_POWER_PIN 14

// Datasheet: Duration before measuring the ouput signal (after switching on LED): 280 µs
#define SAMPLING_TIME   280
// Datasheet: Duration of the whole excitation pulse: 320 µs; Duration before switching off LED: 40 µs
#define DELTA_TIME      40
// Datasheet: Pulse Cycle: 10ms; Remaining time: 10,000 - 320 = 9680 µs
#define SLEEP_TIME      9680

#define BLUETOOTH_RX 15
#define BLUETOOTH_TX 16

#define BAUD_RATE     9600

//Defining pins for TFT Screen


// LED pins
#define POWER_LED 6
#define WORKING_LED 5

// Pins on Screen
// CLK   SCLK    12
// SDA   MOSI    11
// RS    RS/DC   10
// RST   RST     9
// CS    CS      8

// #define TFT_SCLK   21
// #define TFT_MOSI   22
#define TFT_SCLK   12
#define TFT_MOSI   11
#define TFT_DC     10
#define TFT_RST    9
#define TFT_CS     8

#define BTSerial Serial1



Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
SoftwareSerial btSerial(BLUETOOTH_RX, BLUETOOTH_TX);
StateManager stateManager;
// FileProcessor fileProcessor;

//Defining pins for GPS
// SoftwareSerial gpsSerial(3, 2); //RX=pin 10, TX=pin 11
// TinyGPSPlus gps;//This is the GPS object that will pretty much do all the grunt work with the NMEA data

// unsigned int samplingTime = 280; //Datasheet: Duration before measuring the ouput signal (after switching on LED): 280 µs
// unsigned int deltaTime = 40; //Datasheet: Duration of the whole excitation pulse: 320 µs; Duration before switching off LED: 40 µs
// unsigned int sleepTime = 9680; //Datasheet: Pulse Cycle: 10ms; Remaining time: 10,000 - 320 = 9680 µs


long calculateNextMinute() {
  DateTime currentDateTime(ROMVar::getCurrentTime());
  return currentDateTime.unixtime() + (60 - currentDateTime.second());
}

void setup() {
  pinMode(POWER_LED, OUTPUT);
  digitalWrite(POWER_LED, LOW);

  pinMode(WORKING_LED, OUTPUT);
  digitalWrite(WORKING_LED, HIGH);

  Serial.begin(BAUD_RATE); //Setting the speed of communication in bits per second; Arduino default: 9600
  // Serial1.begin(BAUD_RATE);

  btSerial.begin(BAUD_RATE);
  // gpsSerial.begin(BAUD_RATE);//This opens up communications to the GPS

  Wire.begin();
  dht.begin();

  stateManager.init();
  stateManager.printNow();

  // fileProcessor.init();

  // pinMode(LED_POWER_PIN ,OUTPUT); //Configures the digital pin as an output (to set it at 0V and 5V per cycle; turning on and off the LED
  // pinMode(10, OUTPUT); //Configures the pin of the SD card reader as an output

  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);

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
  // Serial.println(fileProcessor.getFirstReading());

  // pinMode(20, INPUT_PULLUP); //Configures the pin of the SD card reader as an output
  if (!SD.begin(20)) {
    Serial.println("NO SD");
  } else {
    Serial.println("SD Available");
    File currentDayFile = SD.open("TEST.TXT", FILE_WRITE);
    currentDayFile.println("Testing stuff");
    currentDayFile.close();
  }


  Serial.println("Start");

  ROMVar::setCurrentTime(stateManager.getTimeStamp());
  ROMVar::setNextMinuteTime(calculateNextMinute());
  // Triple blink to indicate ready
  digitalWrite(WORKING_LED, LOW);
  delay(75);
  digitalWrite(WORKING_LED, HIGH);
  delay(50);
  digitalWrite(WORKING_LED, LOW);
  delay(75);
  digitalWrite(WORKING_LED, HIGH);
  delay(50);
  digitalWrite(WORKING_LED, LOW);
  delay(75);
  digitalWrite(WORKING_LED, HIGH);
  delay(50);
  digitalWrite(WORKING_LED, LOW);
  digitalWrite(POWER_LED, HIGH);

}


void loop() {
  // Process the incoming bluetooth packets
  {
    byte type = 0;
    uint8_t len = 0;
    bool read = false;
    if ( btSerial.available() ) {
      Serial.println("Received packet");
    }
    while(btSerial.available()) {
      Serial.print(btSerial.read());
      Serial.print(" ");
      read = true;
    }
    if ( read )
    Serial.println();

    btSerial.write(CMD_CONNECTION_ACK);
    int packet_len = 4;
    btSerial.write(packet_len);
    long_u timestamp;
    timestamp.data = stateManager.getTimeStamp();
    btSerial.write(timestamp.bytes[0]);
    btSerial.write(timestamp.bytes[1]);
    btSerial.write(timestamp.bytes[2]);
    btSerial.write(timestamp.bytes[3]);
    btSerial.print("\r\n");

    // while ( BTSerial.available() ) {
    //   type = BTSerial.read();
    //   if ( BTSerial.available() ) {
    //     len = BTSerial.read();
    //   }
    //   byte data [len];
    //   int i = 0;
    //   while ( i < len && BTSerial.available() ) {
    //     data[i] = BTSerial.read();
    //     i++;
    //   }
    //
    //   Serial.println(i);
    //   // CommandProcessor::processPacket(type, len, data, btSerial, stateManager, fileProcessor);
    // }
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
  float hum;
  float temp;
  float dustDensity = 0;

  {
    // Variables for DHT22

    long loopTime = stateManager.getTimeStamp();
    long currentTime = ROMVar::getCurrentTime();
    long nextMinuteTime = ROMVar::getNextMinuteTime();

    if ( loopTime > currentTime ) {
      currentTime = loopTime;
      ROMVar::setCurrentTime(currentTime);

      float voMeasured = 0;
      float calcVoltage = 0;
      digitalWrite(LED_POWER_PIN ,LOW); //Turning on the LED; sinking current (i.e. light the LED connected through a series reistor to 5V)
      delayMicroseconds(SAMPLING_TIME); //Duration of sampling

      voMeasured = analogRead(MEASURE_PIN); //Reading the voltage measured

      delayMicroseconds(DELTA_TIME); //Completing excitation pulse
      digitalWrite(LED_POWER_PIN ,HIGH); //Turning off the LED
      delayMicroseconds(SLEEP_TIME); //Delay before next reading

      calcVoltage = voMeasured*(5.0/1024); //0-5V mapped to 0 - 1023 integer values for real voltage value
      dustDensity = 0.17*calcVoltage-0.1; //Datasheet: Calibration curve
      //Read data and store it to variables hum and temp
      hum = dht.readHumidity(); // Relative Humidity in %
      temp = dht.readTemperature(); // Temperature in deg C
      Serial.print("Hum: ");
      Serial.print(hum);
      Serial.print("% Temp: ");
      Serial.print(temp);
      Serial.print("C");
      Serial.print(" Dust: ");
      Serial.println(dustDensity);
      stateManager.printNow();

      {
        //Printing data onto TFT
         tft.setTextColor(ST7735_WHITE);
         tft.setCursor(20,10);
         tft.setTextSize(2);
         tft.println("SENSAIR");
         tft.setTextSize(1);

         tft.fillRect(80,40,40,10,ST7735_BLACK);

         tft.setCursor(0, 40);
         tft.drawLine(0, 30, tft.width()-1, 30, ST7735_WHITE); //draw line separator
         tft.setTextColor(ST7735_YELLOW);
         tft.print(" PM2.5 Conc :");
         tft.setCursor(80, 40);
         tft.setTextColor(ST7735_GREEN);
         tft.println((float)dustDensity);

         tft.fillRect(80,50,40,10,ST7735_BLACK);

         tft.setCursor(0, 50);
         tft.setTextColor(ST7735_YELLOW);
         tft.print(" Humidity(%) :");
         tft.setCursor(80, 50);
         tft.setTextColor(ST7735_GREEN);
         tft.println((float)hum);

         tft.fillRect(80,60,40,10,ST7735_BLACK);

         tft.setCursor(0, 60);
         tft.setTextColor(ST7735_YELLOW);
         tft.print(" Temp :");
         tft.setCursor(80, 60);
         tft.setTextColor(ST7735_GREEN);
         tft.println((float)temp);

         tft.drawLine(0, 75, tft.width()-1, 75, ST7735_WHITE); //draw line separator
      }

      // TODO: Not hardcode the microclimate and location
      // fileProcessor.pushData(dustDensity, 1.35432101, 103.8765432, 0);
    }
    if ( currentTime >= nextMinuteTime ) {
      long prevMinuteTime = nextMinuteTime - 60;

      // Average past minute readings & save as previous minute
      // fileProcessor.openAppropiateFile(prevMinuteTime);
      // fileProcessor.storeAverageData(prevMinuteTime, stateManager.microclimate);
      nextMinuteTime = calculateNextMinute();
      ROMVar::setNextMinuteTime(nextMinuteTime);

    }


    //

  }

  // Send packets if there's something requesting it
  // fileProcessor.sendSomePackets(btSerial);

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


  // tmElements_t tm;
  //
  // time_t unixtime = RTC.get();
  // Serial.println(unixtime);
  // if (RTC.read(tm)) {
  //   Serial.print("Ok, Time = ");
  //   print2digits(tm.Hour);
  //   Serial.write(':');
  //   print2digits(tm.Minute);
  //   Serial.write(':');
  //   print2digits(tm.Second);
  //   Serial.print(", Date (D/M/Y) = ");
  //   Serial.print(tm.Day);
  //   Serial.write('/');
  //   Serial.print(tm.Month);
  //   Serial.write('/');
  //   Serial.print(tmYearToCalendar(tm.Year));
  //   Serial.println();
  // } else {
  //   if (RTC.chipPresent()) {
  //     Serial.println("The DS1307 is stopped.  Please run the SetTime");
  //     Serial.println("example to initialize the time and begin running.");
  //     Serial.println();
  //   } else {
  //     Serial.println("DS1307 read error!  Please check the circuitry.");
  //     Serial.println();
  //   }
  //   delay(9000);
  // }

  delay(50);
}
