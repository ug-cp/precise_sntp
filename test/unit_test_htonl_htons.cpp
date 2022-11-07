/*
  Author: Daniel Mohr
  Date: 2022-11-07
*/

#include <Arduino.h>
#include <ArduinoUnitTests.h>

#include <precise_sntp_htonl_htons.h>

unittest(test_htons) {
  uint16_t value = 0xab;
  uint16_t result = 0xba;
  assertEqual(htons(value), result);
  assertEqual(ntohs(htons(value)), value);
}

unittest(test_htonl) {
  uint32_t value = 0xabcd;
  uint32_t result = 0xdcba;
  assertEqual(htonl(value), 0xdcba);
  assertEqual(ntohl(htonl(value)), value);
}

unittest_main()
