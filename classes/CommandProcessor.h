/**
 * Class to process the packets and commands
 * that come in through the bluetooth channel.
 */
#ifndef _COMMAND_PROCESSOR_H_
#define _COMMAND_PROCESSOR_H_

#include <Arduino.h>
#include "StateManager.h"


#define CMD_FALSE                 0
#define CMD_TRUE                  1
#define CMD_CONNECTION_CHECK      2
#define CMD_CONNECTION_ACK        3
#define CMD_GET_TIME              4
#define CMD_SET_TIME              5
#define CMD_TIME_PACKET           6
#define CMD_GET_READINGS          7
#define CMD_READING_COUNT         8
#define CMD_READY_TO_RECEIVE      9
#define CMD_READING_PACKET        10
#define CMD_READINGS_RECEIVED     11
#define CMD_SET_MICROCLIMATE      12
#define CMD_GET_MICROCLIMATE      13
#define CMD_MICROCLIMATE_PACKET   14

typedef union {
    char bytes[4];
    long data;
} long_u;

typedef union {
    char bytes[2];
    uint8_t data;
} uint16_u;


class CommandProcessor {
private:

public:
  static void processPacket(byte type, uint8_t len, byte bytes[], SoftwareSerial &btSerial, StateManager &stateManager);

  static long decodeLong(byte data [], int start);
  static uint8_t decodeInt8(byte data [], int start);
  static uint16_t decodeInt16(byte data [], int start);

  static void decodePacket();
  static void encodePacket();

  static void init();
};

#endif // _COMMAND_PROCESSOR_H_
