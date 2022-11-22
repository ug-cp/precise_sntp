/*
  Author: Daniel Mohr
  Date: 2022-11-07
*/

#include <Arduino.h>
#include <ArduinoUnitTests.h>

#include <precise_sntp_htonl_htons.h>

unittest(test_htons) {
  uint16_t value = 0xABCD;
  uint16_t result = 0xCDAB;
  assertEqual(result, htons(value));
  assertEqual(value, ntohs(htons(value)));
}

unittest(test_htonl) {
  uint32_t value = 0x01234567UL;
  uint32_t result = 0x67452301UL;
  assertEqual(result, htonl(value));
  assertEqual(value, ntohl(htonl(value)));
}

unittest_main()
