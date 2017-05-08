# Sensair

This project contains the Arduino code for the sensor device codenamed Sensair.

This implements the custom data storage and transfer protocols to allow storage of large (for the Arduino) amounts of data and bluetooth communication with the Viewair Android application.

Sensair requires the following modules:
- microcontroller (Teensy++ 2.0)
- Bluetooth (HC-05)
- GPS (??)
- RTC (DS1307)
- SD Card reader
- Sharp PM2.5 detector

## Issues

All components still untested.


## Tests

Since Arduino has no testing interface to speak of, here are a list of manual tests to run to ensure the thing is still working. For the most part, I suppose we'll have to rely on the Serial output to do any debugging.

**File processing**

- It writes once per minute
- Ensure the writing format is working

**Bluetooth**

- Can connect to phone
- Can sync clock
- Sending data works (this tests a lot)
   - File reading
   - File iteration logic
   - Bluetooth sending protocol
   - Android App working

**GPS**

- Can connect **BROKEN**

**TFT**

- Displays stuff **BROKEN**
