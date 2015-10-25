#include "hibike_message.h"

uint8_t checksum(uint8_t data[], int length) {
  uint8_t chk = data[0];
  for (int i=1; i<length; i++) {
    chk ^= data[i];
  }
  return chk;
}


int send_message(message_t *msg) {
  uint8_t data[msg->payload_length+ MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES+CHECKSUM_BYTES];
  message_to_byte(data, msg);
  data[msg->payload_length+MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES] = checksum(data, msg->payload_length+MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES);
  uint8_t written = Serial.write(data, msg->payload_length+ MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES+CHECKSUM_BYTES);
  if (written != msg->payload_length+ MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES+CHECKSUM_BYTES) {
    return -1;
  }
  return 0;
}


// Returns 0 on success, -1 on error (ex. no message)
// If checksum does not match, empties the incoming buffer. 
int read_message(message_t *msg) {
  if (!Serial.available()) {
    return -1;
  }

  uint8_t data[MAX_PAYLOAD_SIZE+MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES]; 
  Serial.readBytes((char*) data, MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES);
  uint8_t length = data[MESSAGEID_BYTES];
  Serial.readBytes((char*) data+MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES, length);

  uint8_t chk = checksum(data, length+
    MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES+CHECKSUM_BYTES);
  int expected_chk_ind = MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES+length;
  Serial.readBytes((char*) data+expected_chk_ind, 1);

  if (chk != data[expected_chk_ind]) {
    // Empty incoming buffer
    while (Serial.available() > 0) {
      Serial.read();
    }
    return -1;
  }

  msg->messageID = data[0];
  msg->payload_length = data[MESSAGEID_BYTES];
  for (int i = 0; i < length; i++){
    msg->payload[i] = data[i+MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES];
  }

  return 0;
}


void message_to_byte(uint8_t *data, message_t *msg) {
  data[0] = msg->messageID;
  data[1] = msg->payload_length;
  for (int i = 0; i < msg->payload_length; i++){
    data[i+MESSAGEID_BYTES+PAYLOAD_SIZE_BYTES] = msg->payload[i];
  }
}
