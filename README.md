# Sensair

Code for the arduino sensor device module thing.

This implements the custom data storage and transfer protocols to allow storage of large (for the arduino) amounts of data and bluetooth communication with the Viewair Android application.

The Arduino device requires the following modules:
- Bluetooth (HC-05)
- GPS (??)
- RTC
- SD Card
- Sharp PM2.5 detector

## Issues

The arduino nano cannot support the TFT library. It's out of memory. Plans have been made to get hold of the Teensy 2.0 ++ device that allows more memory.


## Tests

Since arduino has no testing interface to speak of, here are a list of manual tests to run to ensure the thing is still working. For the most part, I suppose we'll have to rely on the Serial output to do any debugging.

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
