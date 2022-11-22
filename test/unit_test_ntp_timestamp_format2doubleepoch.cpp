/*
  Author: Daniel Mohr
  Date: 2022-11-08
*/

#include <Arduino.h>
#include <ArduinoUnitTests.h>

#include <precise_sntp.h>
#include <precise_sntp_ntp_timestamp_format2doubleepoch.h>

unittest(test_ntp_timestamp_seconds2epoch) {
  struct ntp_timestamp_format_struct value;
  value.seconds = 0x89ABCDEFUL;
  value.fraction = 0x01234567UL;
  uint32_t result = 100749167;
  assertEqual(result, ntp_timestamp_seconds2epoch(value));
  value.seconds = 0xFECDBA98;
  value.fraction = 0x76543210;
  result = 2065906712;
  assertEqual(result, ntp_timestamp_seconds2epoch(value));
}

unittest(test_ntp_timestamp_format2doubleepoch) {
  struct ntp_timestamp_format_struct value;
  value.seconds = 0x89ABCDEFUL;
  value.fraction = 0x01234567UL;
  double result = 100749167.004;
  double cal_result = ntp_timestamp_format2doubleepoch(value);
  assertEqualFloat(result, cal_result, 1e-6);
  value.seconds = 0xFECDBA98;
  value.fraction = 0x76543210;
  result = 2065906712.462;
  cal_result = ntp_timestamp_format2doubleepoch(value);
  assertEqualFloat(result, cal_result, 1e-6);
}

unittest_main()
