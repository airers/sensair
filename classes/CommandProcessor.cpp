#include "CommandProcessor.h"

// long CommandProcessor::timestampForSending =0;
void CommandProcessor::processPacket(byte type,
  uint8_t len, byte bytes[],
  SoftwareSerial &btSerial,
  StateManager &stateManager,
  FileProcessor &fileProcessor) {

  uint8_t packet_len = 0;
  switch ( type ) {
    case CMD_CONNECTION_CHECK:
    {
      // // -Serial.println("Received connection check");
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
      // -Serial.println("Setting time");
      // -Serial.println(len);
      if ( len >= 4 ) {
        long_u timestamp;
        timestamp.bytes[0] = bytes[0];
        timestamp.bytes[1] = bytes[1];
        timestamp.bytes[2] = bytes[2];
        timestamp.bytes[3] = bytes[3];
        // -Serial.println(timestamp.data);
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
        // -Serial.println("Getting readings");
        // -Serial.println(len);

        // ROMVar::printFreeRam();
        if ( len >= 4 ) {
          long_u timestamp;
          timestamp.bytes[0] = bytes[0];
          timestamp.bytes[1] = bytes[1];
          timestamp.bytes[2] = bytes[2];
          timestamp.bytes[3] = bytes[3];

          if ( timestamp.data < fileProcessor.getFirstReading() ) {
            timestamp.data = fileProcessor.getFirstReading();
          }
          // -Serial.println(timestamp.data);
          // stateManager.setTime(timestamp.data);
          uint16_u readingCount;
          Serial.write(C_R);
          Serial.write(C_D);
          Serial.write(C_SP);
          Serial.write(C_C);
          Serial.write(C_T);
          Serial.write(C_LR);
          Serial.print(timestamp.data);
          // -Serial.println(" ");
          readingCount.data = fileProcessor.countPackets(timestamp.data);
          btSerial.write(CMD_READING_COUNT);
          packet_len = 2;
          btSerial.write(packet_len);
          btSerial.write(readingCount.bytes[0]);
          btSerial.write(readingCount.bytes[1]);
          btSerial.print("\r\n");

          // Serial.println(readingCount.data);
        }

      // Send reading count
      break;
    case CMD_READY_TO_RECEIVE:
      // Serial.println("RTR");
      if ( len >= 6 ) {
        long_u timestamp;
        timestamp.bytes[0] = bytes[0];
        timestamp.bytes[1] = bytes[1];
        timestamp.bytes[2] = bytes[2];
        timestamp.bytes[3] = bytes[3];

        if ( timestamp.data < fileProcessor.getFirstReading() ) {
          timestamp.data = fileProcessor.getFirstReading();
        }

        uint16_u count;
        count.bytes[0] = bytes[4];
        count.bytes[1] = bytes[5];

        // Set state to packet sending mode
        // Send packets (not here, in main loop)
        fileProcessor.startSendingData(timestamp.data, count.data);
        // -Serial.println("Gonna send data");
        // Serial.println(timestamp.data);
        // Serial.println(count.data);
        // fileProcessor.startSendingData(1490830040, 100);
      }
      break;
    case CMD_READINGS_RECEIVED:
      // Set state back to idle
      break;
    default:
      // Error!
      // -Serial.print(type);
      // -Serial.println(" :Wrong command");
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
