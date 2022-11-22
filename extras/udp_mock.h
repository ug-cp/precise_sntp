/*
  Author: Daniel Mohr
  Date: 2022-11-22

  Since src/precise_sntp.cpp is compiled together with the unittest
  test/unit_test_htonl_htons.cpp, we have to make at least the compiling
  working.

  For the unittest this file mocks the part of Udp.h we need.
*/

class UDP {
 public:
  virtual uint8_t begin(uint16_t) = 0;
  virtual int beginPacket(IPAddress ip, uint16_t port) = 0;
  virtual int beginPacket(const char *host, uint16_t port) = 0;
  virtual size_t write(const uint8_t *buffer, size_t size) = 0;
  virtual int endPacket() = 0;
  virtual int parsePacket() = 0;
  virtual int read(unsigned char* buffer, size_t len) = 0;
  virtual int read(char* buffer, size_t len) = 0;
};
