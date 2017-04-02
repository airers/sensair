/**
 * Class to process the packets and commands
 * that come in through the bluetooth channel.
 */
#ifndef _COMMAND_PROCESSOR_H_
#define _COMMAND_PROCESSOR_H_

#include <Arduino.h>
#include "StateManager.h"
#include "FileProcessor.h"
#include "datatypes.h"


class CommandProcessor {
private:
public:
  // static long timestampForSending;
  static void processPacket(byte type, uint8_t len, byte bytes[], SoftwareSerial &btSerial, StateManager &stateManager, FileProcessor &fileProcessor);

  static long decodeLong(byte data [], int start);
  static uint8_t decodeInt8(byte data [], int start);
  static uint16_t decodeInt16(byte data [], int start);

  static void decodePacket();
  static void encodePacket();

  static void init();
};

#endif // _COMMAND_PROCESSOR_H_
