/*
  Author: Daniel Mohr
  Date: 2022-11-06

  For more information look at the README.md.
*/

#pragma once

#include <Arduino.h>
#include "Udp.h"

// if PRECISE_SNTP_DEBUG exists debugging output to serial console is done
// #define PRECISE_SNTP_DEBUG

struct ntp_timestamp_format_struct { // 8 bytes
  uint32_t seconds;
  uint32_t fraction;
};

struct timestamp_format { // 8 bytes
  uint32_t seconds;
  uint32_t fraction;
};

union ntp_local_clock_union {
  byte as_bytes[8];
  struct ntp_timestamp_format_struct as_timestamp;
};

class precise_sntp {
 public:

  /*
    minimal initialization of the class
  */
  precise_sntp(UDP &udp);

  /*
    initialization of the class with a specific time server
  */
  precise_sntp(UDP &udp, IPAddress ntp_server_ip);

  /*
    Get time from server.

    returns an error code:

    0: success
    1: poll policy does not allow fast updates, skip communication with server
    2: cannot use local port _localport
    3: cannot start connection
    4: problems writing data
    5: packet was not send
    6: got no answer from server
    7: sanity check fail, answer from server is bogus
    8: sanity check fail, server is not syncronized
  */
  uint8_t update();

  /*
    Force get time from server, should only be used in local networks with
    local time server for debugging purpose.

    returns an error code:

    0: success
    1: poll policy does not allow fast updates, skip communication with server
    2: cannot use local port _localport
    3: cannot start connection
    4: problems writing data
    5: packet was not send
    6: got no answer from server
    7: sanity check fail, answer from server is bogus
  */
  uint8_t force_update();

  struct ntp_timestamp_format_struct _get_local_clock();
  time_t get_epoch(); // seconds
  double dget_epoch(); // seconds with milliseconds as fraction
  timestamp_format tget_epoch(); // seconds + fraction of the second
 private:
  IPAddress _ntp_server_ip;
  UDP* _udp;
  uint16_t _localport = 1234;
  union ntp_local_clock_union _ntp_local_clock;
  unsigned long _last_clock_update = 0;
  unsigned long _last_update = 0;
  uint8_t _poll_exponent = 1;
};
