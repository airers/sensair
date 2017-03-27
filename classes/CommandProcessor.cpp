#include "CommandProcessor.h"

void CommandProcessor::processPacket(byte type, uint8_t len, byte bytes[], SoftwareSerial &btSerial) {
  uint8_t packet_len = 0;
  switch ( type ) {
    case CMD_CONNECTION_CHECK:
      Serial.println("Received connection check");
      btSerial.write(CMD_CONNECTION_ACK);
      btSerial.write(packet_len);
      btSerial.print("\r\n");
    break;
    case CMD_SET_TIME:
      // Set time
    case CMD_GET_TIME:
      // Send time packet
    break;
    case CMD_SET_MICROCLIMATE:
      // Set microclimate
    case CMD_GET_MICROCLIMATE:
      // Send microclimate packet
    break;
    case CMD_GET_READINGS:
      // Count readings
      // Send reading count
    case CMD_READY_TO_RECEIVE:
      // Set state to packet sending mode
      // Send packets (not here, in main loop)
    case CMD_READINGS_RECEIVED:
      // Set state back to idle
    default:
      // Error!
      Serial.println(" :Wrong command");
    break;
  }
}

long CommandProcessor::decodeLong(byte data [], int start) {
  long_u ret;
  ret.bytes[0] = data[start];
  ret.bytes[1] = data[start + 1];
  ret.bytes[2] = data[start + 2];
  ret.bytes[3] = data[start + 3];
  return ret.data;
}

uint8_t CommandProcessor::decodeInt8(byte data [], int start) {
  return data[start];
}

uint16_t CommandProcessor::decodeInt16(byte data [], int start) {
  uint16_u ret;
  ret.bytes[0] = data[start];
  ret.bytes[1] = data[start + 1];
  return ret.data;
}
