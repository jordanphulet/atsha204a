#ifndef ATSHA204A_JHULET_H
#define ATSHA204A_JHULET_H

#include "Wire.h"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// error codes
#define ATSHA204A_ERR_OK                  ((uint8_t) 0x00)
#define ATSHA204A_ERR_BAD_CRC             ((uint8_t) 0x01)
#define ATSHA204A_ERR_BAD_RESPONSE_SIZE   ((uint8_t) 0x02)
#define ATSHA204A_ERR_TRANSMISSION_FAILED ((uint8_t) 0x03)
#define ATSHA204A_ERR_NO_BYTES_AVAILABLE  ((uint8_t) 0x04)
#define ATSHA204A_ERR_NACK                ((uint8_t) 0x05)
#define ATSHA204A_ERR_WAKE_FAILED         ((uint8_t) 0x06)

// zone definitions
#define ATSHA204A_ZONE_ENCODING_CONFIG ((uint8_t) 0x00)
#define ATSHA204A_ZONE_ENCODING_OTP    ((uint8_t) 0x01)
#define ATSHA204A_ZONE_ENCODING_DATA   ((uint8_t) 0x02)

// flags
#define ATSHA204A_FLAG_CMD ((uint8_t) 0x03)

// message parts sizes
#define ATSHA204A_COUNT_SIZE  ((uint8_t) 1)
#define ATSHA204A_OPCODE_SIZE ((uint8_t) 1)
#define ATSHA204A_PARAM1_SIZE ((uint8_t) 1)
#define ATSHA204A_PARAM2_SIZE ((uint8_t) 2)
#define ATSHA204A_CRC_SIZE    ((uint8_t) 2)

// stay under the buffer size in the Wire library
#define ATSHA204A_MAX_PACKET_SIZE ((uint8_t) 18)

// opcodes
#define ATSHA204A_OPCODE_READ   0x02
#define ATSHA204A_OPCODE_MAC    0x08
#define ATSHA204A_OPCODE_WRITE  0x12
#define ATSHA204A_OPCODE_NONCE  0x16
#define ATSHA204A_OPCODE_LOCK   0x17
#define ATSHA204A_OPCODE_RANDOM 0x1B

// command delays
#define ATSHA204A_CMD_DELAY_READ   4
#define ATSHA204A_CMD_DELAY_MAC    35
#define ATSHA204A_CMD_DELAY_WRITE  42
#define ATSHA204A_CMD_DELAY_NONCE  60
#define ATSHA204A_CMD_DELAY_LOCK   24
#define ATSHA204A_CMD_DELAY_RANDOM 50

// data sizes
#define ATSHA204A_DATA_SIZE_MAC 32

// response sizes
#define ATSHA204A_RESP_SIZE_MAC 32
#define ATSHA204A_RESP_SIZE_RANDOM 32

#define ATSHA_DEBUG 1

class ATSHA204A {
  public:

  ATSHA204A(uint8_t i2c_addr);

  uint8_t wake();
  uint8_t sleep();

  // helper comands
  void readSerialNumber(uint8_t* serialNumber);
  void printConfig();

  // basic commands
  uint8_t commandRandom(uint8_t* respons);
  uint8_t commandRead(uint8_t zoneEncoding, uint8_t slot, uint8_t* response);
  uint8_t commandRead(uint8_t zoneEncoding, uint8_t slot, uint8_t* response, bool read4Bytes, uint8_t offset);
  uint8_t commandWrite(uint8_t zoneEncoding, uint8_t slot, uint8_t* data);
  uint8_t commandMac(uint8_t keySlot, uint8_t* response);
  uint8_t commandNonce(bool passThrough, uint8_t* numIn, uint8_t* response);
  uint8_t commandLock(bool lockData);

  private:

  int i2c_addr;

  uint8_t command(
    uint8_t  opcode,
    uint8_t  param1,
    uint16_t param2,
    uint8_t  dataSize,     uint8_t* data,
    uint8_t  responseSize, uint8_t* response,
    uint8_t  cmdDelay
  );
  uint8_t sendCommand(uint8_t opcode, uint8_t param1, uint16_t param2, uint8_t dataSize, uint8_t* data);
  uint8_t getResponse(uint8_t dataSize, uint8_t* data);
  void calculateCrc(uint8_t length, uint8_t *data, uint8_t *crc);
  bool wireWait();

  // utilities
  void printSlotConfig(uint8_t* slotBytes);
  void printBinaryByte(uint8_t value, uint8_t maxValue);
  void printHexByte(uint8_t data);
};

#endif
