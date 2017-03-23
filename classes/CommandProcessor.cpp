#include "CommandProcessor.h"

CommandProcessor::processPacket(byte [] data) {
  int byteCount = sizeOf(data);
  if ( byteCount < 2 ) {
    return;
  }
  uint8_t command = data[0];
  uint8_t length = data[1];
  if ( byteCount < length ) {
    return;
  }

  switch ( command ) {
    case CMD_CONNECTION_CHECK:

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
    break;
  }
}

long CommandProcessor::decodeLong(byte [] data, int start) {

}

uint8_t CommandProcessor::decodeInt8(byte [] data, int start) {

}

uint16_t CommandProcessor::decodeInt16(byte [] data, int start) {

}
