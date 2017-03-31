#include "CommandProcessor.h"

long CommandProcessor::timestampForSending = 0;
void CommandProcessor::processPacket(byte type,
  uint8_t len, byte bytes[],
  SoftwareSerial &btSerial,
  StateManager &stateManager,
  FileProcessor &fileProcessor) {

  uint8_t packet_len = 0;
  switch ( type ) {
    case CMD_CONNECTION_CHECK:
    {
      // Serial.println("Received connection check");
      btSerial.write(CMD_CONNECTION_ACK);
      packet_len = 4;
      btSerial.write(packet_len);
      long_u timestamp;
      timestamp.data = stateManager.getTimeStamp();
      btSerial.write(timestamp.bytes[0]);
      btSerial.write(timestamp.bytes[1]);
      btSerial.write(timestamp.bytes[2]);
      btSerial.write(timestamp.bytes[3]);
      btSerial.print("\r\n");
    }
    break;
    case CMD_SET_TIME:
    {
      Serial.println("Setting time");
      Serial.println(len);
      if ( len >= 4 ) {
        long_u timestamp;
        timestamp.bytes[0] = bytes[0];
        timestamp.bytes[1] = bytes[1];
        timestamp.bytes[2] = bytes[2];
        timestamp.bytes[3] = bytes[3];
        Serial.println(timestamp.data);
        stateManager.setTime(timestamp.data);
      }
    }
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
        Serial.println("Getting readings");
        Serial.println(len);
        if ( len >= 4 ) {
          long_u timestamp;
          timestamp.bytes[0] = bytes[0];
          timestamp.bytes[1] = bytes[1];
          timestamp.bytes[2] = bytes[2];
          timestamp.bytes[3] = bytes[3];
          Serial.println(timestamp.data);
          stateManager.setTime(timestamp.data);
          uint16_u readingCount;
          Serial.print("Getting reading count: ");
          Serial.print(timestamp.data);
          Serial.println(" ");
          readingCount.data = fileProcessor.countPackets(timestamp.data);
          btSerial.write(CMD_READING_COUNT);
          packet_len = 2;
          btSerial.write(packet_len);
          btSerial.write(readingCount.bytes[0]);
          btSerial.write(readingCount.bytes[1]);
          btSerial.print("\r\n");
          Serial.println(readingCount.data);
          CommandProcessor::timestampForSending = timestamp.data;
        }

      // Send reading count
      break;
    case CMD_READY_TO_RECEIVE:
      // Set state to packet sending mode
      // Send packets (not here, in main loop)
      fileProcessor.startSendingData(CommandProcessor::timestampForSending);
      CommandProcessor::timestampForSending = 0;
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
