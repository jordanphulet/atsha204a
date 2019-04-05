# include "ATSHA204A_jhulet.h"

ATSHA204A::ATSHA204A(uint8_t i2c_addr): i2c_addr(i2c_addr) {
  Wire.begin();
}

void ATSHA204A::readSerialNumber(uint8_t* serialNumber) {
  // serial number is a subset of the first 32 bytes of the config
  uint8_t readResponse[32];
  commandRead(ATSHA204A_ZONE_ENCODING_CONFIG, 0, readResponse);

  uint8_t offset = 0;
  // first word
  memcpy(serialNumber, readResponse, 4);

  // third word
  memcpy(serialNumber + 4, readResponse + 8, 4);

  // first byte of fourth word
  serialNumber[8] = readResponse[12];
}

void ATSHA204A::printBinaryByte(uint8_t value, uint8_t maxValue) {
  if (maxValue > 64 && value < 64) {
    Serial.print("0");
  }
  if (maxValue > 32 && value < 32) {
    Serial.print("0");
  }
  if (maxValue > 16 && value < 16) {
    Serial.print("0");
  }
  if (maxValue > 8 && value < 8) {
    Serial.print("0");
  }
  if (maxValue > 4 && value < 4) {
    Serial.print("0");
  }
  if (maxValue > 2 && value < 2) {
    Serial.print("0");
  }
  Serial.print(value, BIN);
}

void ATSHA204A::printSlotConfig(uint8_t* slotBytes) {
  // print the bytes
  Serial.print("[ ");
  printHexByte(slotBytes[0]);
  printHexByte(slotBytes[1]);
  Serial.print("]: ");

  // WriteConfig
  printBinaryByte(slotBytes[0] >> 4, 16);
  Serial.print(" ");

  // WriteKey
  uint8_t writeKey = slotBytes[0] & 0x0F;
  if (writeKey < 10) {
    Serial.print(" ");
  }
  Serial.print(writeKey);
  Serial.print(" ");

  // IsSecret
  if ((1 << 7) & slotBytes[1]) {
    Serial.print("S");
  } else {
    Serial.print("s");
  }

  // EncryptRead
  if ((1 << 6) & slotBytes[1]) {
    Serial.print("E");
  } else {
    Serial.print("e");
  }

  // LimitedUse
  if ((1 << 5) & slotBytes[1]) {
    Serial.print("L");
  } else {
    Serial.print("l");
  }

  // CheckOnly
  if ((1 << 4) & slotBytes[1]) {
    Serial.print("C");
  } else {
    Serial.print("c");
  }
  Serial.print(" ");

  // ReadKey
  uint8_t readKey = slotBytes[1] & 0x0F;
  if (readKey < 10) {
    Serial.print(" ");
  }
  Serial.print(readKey);
  Serial.print(" ");

  Serial.println();


  /*

  uint8_t read_key = configBytes[slot_begin + 1] & 0x0F;
  if (read_key < 10) {
    Serial.print(" ");
  }
  Serial.print(read_key);
  Serial.print(" ");

  Serial.println();
  */
}

void ATSHA204A::printConfig() {
  uint8_t readResponse[32];

  // read device data
  Serial.println("device data:");
  commandRead(ATSHA204A_ZONE_ENCODING_CONFIG, 0, readResponse);
  for (uint8_t i = 0; i < 5; i++) {
    uint8_t offset = i * 4;
    printHexByte(readResponse[offset]);
    printHexByte(readResponse[offset + 1]);
    printHexByte(readResponse[offset + 2]);
    printHexByte(readResponse[offset + 3]);
    Serial.println();
  }

  // read slot config
  Serial.println("slot config:");
  for (uint8_t i = 5; i < 8; i++) {
    uint8_t offset = i * 4;
    printSlotConfig(readResponse + offset);
    printSlotConfig(readResponse + offset + 2);
  }

  commandRead(ATSHA204A_ZONE_ENCODING_CONFIG, 1, readResponse);
  for (uint8_t i = 0; i < 5; i++) {
    uint8_t offset = i * 4;
    printSlotConfig(readResponse + offset);
    printSlotConfig(readResponse + offset + 2);
  }

  // read use flags
  Serial.println("use flags:");
  for (uint8_t i = 5; i < 8; i++) {
    uint8_t offset = i * 4;
    printHexByte(readResponse[offset]);
    printHexByte(readResponse[offset + 1]);
    Serial.println();
    printHexByte(readResponse[offset + 2]);
    printHexByte(readResponse[offset + 3]);
    Serial.println();
  }

  commandRead(ATSHA204A_ZONE_ENCODING_CONFIG, 2, readResponse, true, 0);
  printHexByte(readResponse[0]);
  printHexByte(readResponse[1]);
  Serial.println();
  printHexByte(readResponse[2]);
  printHexByte(readResponse[3]);
  Serial.println();

  // read key use data
  Serial.println("key use data:");
  for (uint8_t i = 0; i < 4; i++) {
    commandRead(ATSHA204A_ZONE_ENCODING_CONFIG, 2, readResponse, true, 1 + i);
    for (uint8_t j = 0; j < 4; j++) {
      printHexByte(readResponse[j]);
      Serial.println();
    }
  }

  // user data and lock data
  Serial.println("user data and lock data:");
  commandRead(ATSHA204A_ZONE_ENCODING_CONFIG, 2, readResponse, true, 5);
  printHexByte(readResponse[0]);
  printHexByte(readResponse[1]);
  printHexByte(readResponse[2]);
  printHexByte(readResponse[3]);
  Serial.println();
}

uint8_t ATSHA204A::commandRandom(uint8_t* response) {
  return command(
    ATSHA204A_OPCODE_RANDOM,
    0x00,
    0x0000,
    0, 0,
    ATSHA204A_RESP_SIZE_RANDOM, response,
    ATSHA204A_CMD_DELAY_RANDOM
  );
}

uint8_t ATSHA204A::commandRead(uint8_t zoneEncoding, uint8_t slot, uint8_t* response) {
  return commandRead(zoneEncoding, slot, response, false, 0);
}

uint8_t ATSHA204A::commandRead(uint8_t zoneEncoding, uint8_t slot, uint8_t* response, bool read4Bytes, uint8_t offset) {
  uint8_t zone = read4Bytes ? 0: 0b10000000;
  // set zone encoding bits
  zone |= zoneEncoding;

  uint16_t address = slot << 3;
  address += offset;
  address <<= 8;

  return command(
    ATSHA204A_OPCODE_READ,
    zone,
    address,
    0, 0,
    read4Bytes ? 4: 32, response,
    ATSHA204A_CMD_DELAY_READ
  );
}

uint8_t ATSHA204A::commandWrite(uint8_t zoneEncoding, uint8_t slot, uint8_t* data) {
  // only writing 32 bytes in the clear
  uint8_t zone = 0b10000000;
  // set zone encoding bits
  zone |= zoneEncoding;

  uint16_t address = slot << 11;
  uint8_t response;
  return command(
    ATSHA204A_OPCODE_WRITE,
    zone,
    address,
    32, data,
    1, &response,
    ATSHA204A_CMD_DELAY_WRITE
  );
}

uint8_t ATSHA204A::commandMac(uint8_t keySlot, uint8_t* response) {
  // mode
  //   7: 0
  //   6: include SN bits
  //   5: include 64 OTP bits
  //   4: include 88 OTP bits (overrides 6 if set)
  //   3: 0
  //   2: if 0 or 1 are set, must match TempKey.SourceFlag (0=random, 1=input)
  //   1: 1st 32 bytes from from TempKey, otherwise from data slot
  //   0: 2nd 32 bytes from from TempKey, otherwise from challenge parameter 

  uint8_t mode = 0b01000001; // include SN, no OTP bits, key from slot, not using challenge param, temp key random
  return command(
    ATSHA204A_OPCODE_MAC,
    mode,
    keySlot << 8,
    0, 0,
    ATSHA204A_RESP_SIZE_MAC, response,
    ATSHA204A_CMD_DELAY_MAC
  );
}

uint8_t ATSHA204A::commandNonce(bool passThrough, uint8_t* numIn, uint8_t* response) {
  // in passthrough mode the response is 0x00, otherwise it is the 32 byte output of the RNG
  uint8_t responseSize = passThrough ? 1 : 32;

  // in passthorough mode the data is the 32 byte input, otherwise the 20 byte
  // input is used in the SHA which also includes the output of the RNG
  uint8_t dataSize = passThrough ? 32 : 20;

  // always update the seed if not in passthrough mode
  uint8_t mode = passThrough ? 0x03 : 0x00;

  return command(
    ATSHA204A_OPCODE_NONCE,
    mode,
    0x0000,
    dataSize, numIn,
    responseSize, response,
    ATSHA204A_CMD_DELAY_NONCE
  );
}

uint8_t ATSHA204A::commandLock(bool lockData) {
  // never checking CRC
  uint8_t zone = 0b10000000;
  if (lockData) {
    zone += 1;
  }

  uint8_t response[1];
  return command(
    ATSHA204A_OPCODE_LOCK,
    zone,
    0x0000,
    0, 0,
    1, response,
    ATSHA204A_CMD_DELAY_LOCK
  );
}

uint8_t ATSHA204A::command(
  uint8_t  opcode,
  uint8_t  param1,
  uint16_t param2,
  uint8_t  dataSize,     uint8_t* data,
  uint8_t  responseSize, uint8_t* response,
  uint8_t  cmdDelay
) {
  // send command
  uint8_t err = sendCommand(opcode, param1, param2, dataSize, data);
  if (err != 0) {
    return err;
  }

  // delay for command completion
  delay(cmdDelay);

  // get response
  return getResponse(responseSize, response);
}

uint8_t ATSHA204A::sendCommand(uint8_t opcode, uint8_t param1, uint16_t param2, uint8_t dataSize, uint8_t* data) {
  uint8_t requestSize = ATSHA204A_COUNT_SIZE + ATSHA204A_OPCODE_SIZE + ATSHA204A_PARAM1_SIZE + ATSHA204A_PARAM2_SIZE + dataSize + ATSHA204A_CRC_SIZE; 
  uint8_t request[requestSize];

  // copy count
  request[0] = requestSize;

  // copy opcode
  request[ATSHA204A_COUNT_SIZE] = opcode;

  // copy param1
  request[ATSHA204A_COUNT_SIZE + ATSHA204A_OPCODE_SIZE] = param1;

  // copy param2
  request[ATSHA204A_COUNT_SIZE + ATSHA204A_OPCODE_SIZE + ATSHA204A_PARAM1_SIZE] = param2 >> 8;
  request[ATSHA204A_COUNT_SIZE + ATSHA204A_OPCODE_SIZE + ATSHA204A_PARAM1_SIZE + 1] = param2 & 0xFF;

  // copy data
  for (uint8_t i = 0; i < dataSize; i++) {
    request[i + ATSHA204A_COUNT_SIZE + ATSHA204A_OPCODE_SIZE + ATSHA204A_PARAM1_SIZE + ATSHA204A_PARAM2_SIZE] = data[i];
  }

  // add CRC to request
  uint8_t crcDataSize = requestSize - ATSHA204A_CRC_SIZE;
  uint8_t* crc = request + requestSize - ATSHA204A_CRC_SIZE;
  calculateCrc(crcDataSize, request, crc);

  #if ATSHA_DEBUG
  Serial.print("SENDING: ");
  for (int i = 0; i < requestSize; i++) {
    printHexByte(request[i]);
  }
  Serial.println();
  #endif

  uint8_t bytesWritten = 0;
  while(bytesWritten < requestSize) {
    Wire.beginTransmission(i2c_addr);
    Wire.write(ATSHA204A_FLAG_CMD);

    uint8_t bytesToWrite = min(ATSHA204A_MAX_PACKET_SIZE - 1, requestSize - bytesWritten);

    bytesWritten += Wire.write(request + bytesWritten, bytesToWrite);
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
      Serial.println("NACK ERROR!");
      return ATSHA204A_ERR_NACK;
    }
  }

  // OK
  return ATSHA204A_ERR_OK;
}

void ATSHA204A::printHexByte(uint8_t data) {
  Serial.print("0x");
  if (data < 0x10) {
    Serial.print("0");
  }
  Serial.print(data, HEX);
  Serial.print(" ");
}

bool ATSHA204A::wireWait() {
  uint8_t retries = 3;
  while (!Wire.available()) {
    retries--;
    if (retries < 0) {
      return false;
    }
    delay(20);
  }
  return true;
}

uint8_t ATSHA204A::getResponse(uint8_t dataSize, uint8_t* data) {
  // trivial case
  if (dataSize < 1) {
    return ATSHA204A_ERR_OK;
  }

  uint8_t retries = 5;
  while(true) {
    retries--;
    // send transmit flag
    delay(3);
    Wire.beginTransmission(i2c_addr);
    Wire.write(0x88);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      break;
    }
    else if (retries < 0) {
      Serial.println("RESPONSE TRANSMISSION FAILED ERROR!");
      Serial.println(err);
      return ATSHA204A_ERR_TRANSMISSION_FAILED;
    }
  }

  // get count
  Wire.requestFrom(i2c_addr, 1);
  if (!wireWait()) {
    Serial.println("COUNT NO BYTES AVAILABLE ERROR!");
    return ATSHA204A_ERR_NO_BYTES_AVAILABLE;
  }

  // initialize response buffer
  uint8_t count = Wire.read();
  uint8_t response[count];
  response[0] = count;

  // get all the bytes
  uint8_t bytesReceived = 1;
  while (bytesReceived < count) {
    uint8_t newBytes = Wire.requestFrom(i2c_addr, count - bytesReceived);
    if (!wireWait()) {
      Serial.println("RESPONSE NO BYTES AVAILABLE ERROR!");
      return ATSHA204A_ERR_NO_BYTES_AVAILABLE;
    }
    for (int i = 0; i < newBytes; i++) {
      response[i + bytesReceived] = Wire.read();
    }
    bytesReceived += newBytes;
  }

  #if ATSHA_DEBUG
  Serial.print("RESPONSE: ");
  for (uint8_t i = 0; i < count; i++) {
    printHexByte(response[i]);
  }
  Serial.println();
  #endif

  // check CRC
  uint8_t crc[2];
  calculateCrc(count - 2, response, crc);
  if (crc[0] != response[count - 2] || crc[1] != response[count-1]) {
    Serial.println("BAD CRC ERROR!");
    return ATSHA204A_ERR_BAD_CRC;
  }

  // copy data
  for (int i = 0; i < dataSize; i++) {
    if (i + 1 >= count) {
      break;
    }
    data[i] = response[i + 1];
  }

  // OK
  return ATSHA204A_ERR_OK;
}

uint8_t ATSHA204A::wake() {
  // hold SDA low
  Wire.beginTransmission(0x00);
  Wire.endTransmission();

  // delay for device wakeup
  delay(3);

  // send transmit flag
  Wire.beginTransmission(i2c_addr);
  Wire.write(0x88);
  uint8_t err = Wire.endTransmission();
  if (err != 0) {
    Serial.println("WAKE END TRANSMISSION FAILED ERROR!");
    return ATSHA204A_ERR_TRANSMISSION_FAILED;
  }

  // get 0x11 response
  uint8_t response[2];
  err = getResponse(2, response);
  if (err != 0) {
    return err;
  }

  // OK
  return ATSHA204A_ERR_OK;
}

uint8_t ATSHA204A::sleep() {
  Wire.beginTransmission(i2c_addr);
  Wire.write(0xCC);
  uint8_t err = Wire.endTransmission();
  if (err != 0) {
    Serial.println("SLEEP END TRANSMISSION FAILED ERROR!");
    return ATSHA204A_ERR_TRANSMISSION_FAILED;
  }

  // OK
  return ATSHA204A_ERR_OK;
}

void ATSHA204A::calculateCrc(uint8_t length, uint8_t *data, uint8_t *crc) {
  uint8_t counter;
  uint16_t crc_register = 0;
  uint16_t polynom = 0x8005;
  uint8_t shift_register;
  uint8_t data_bit, crc_bit;

  for (counter = 0; counter < length; counter++) {
    for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
      data_bit = (data[counter] & shift_register) ? 1 : 0;
      crc_bit = crc_register >> 15;

      // Shift CRC to the left by 1.
      crc_register <<= 1;

      if ((data_bit ^ crc_bit) != 0)
        crc_register ^= polynom;
    }
  }

  crc[0] = (uint8_t) (crc_register & 0x00FF);
  crc[1] = (uint8_t) (crc_register >> 8);
}
