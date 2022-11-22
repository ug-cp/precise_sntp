/*
  Author: Daniel Mohr
  Date: 2022-11-08
*/

#include <Arduino.h>
#include <ArduinoUnitTests.h>

#include <precise_sntp.h>
#include <precise_sntp_ntp_local_clock_union2uint64.h>

unittest(test_ntp_local_clock_union2uint64) {
  union ntp_local_clock_union value;
  value.as_timestamp.seconds = 0x01234567UL;
  value.as_timestamp.fraction = 0x89ABCDEFUL;
  uint64_t result = 0x123456789ABCDEF;
  assertEqual(result, _ntp_local_clock_union2uint64(value));
}

unittest_main()
