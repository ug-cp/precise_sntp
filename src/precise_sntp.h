/*
  Author: Daniel Mohr
  Date: 2022-11-06

  For more information look at the README.md.
*/

#pragma once

#include <Arduino.h>
#include <Udp.h>

struct ntp_timestamp_format_struct { // 8 bytes
  uint32_t seconds;
  uint32_t fraction;
};

union ntp_local_clock_union {
  byte as_bytes[8];
  struct ntp_timestamp_format_struct as_timestamp;
};

class precise_sntp {
 public:
  precise_sntp(UDP &udp);
  precise_sntp(UDP &udp, IPAddress ntp_server_ip);
  bool update();
  struct ntp_timestamp_format_struct _get_local_clock();
  time_t get_epoch();
 private:
  IPAddress _ntp_server_ip;
  UDP* _udp;
  uint16_t _localport = 1234;
  union ntp_local_clock_union _ntp_local_clock;
  unsigned long _last_update = 0;
};
