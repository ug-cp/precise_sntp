/*
  Author: Daniel Mohr
  Date: 2022-12-09

  For more information look at the README.md.
*/

#pragma once

#include <Arduino.h>
#include <Udp.h>

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

    The named ntp server pool.ntp.org is used.

    Example:

    #include <EthernetUdp.h>
    #include <precise_sntp.h>
    EthernetUDP udp;
    precise_sntp sntp(udp);
    void setup() {}
    void loop() {
    sntp.update();
    }
  */
  precise_sntp(UDP &udp);

  /*
    initialization of the class with a specific time server

    Example:

    #include <EthernetUdp.h>
    #include <IPAddress.h>
    #include <precise_sntp.h>
    EthernetUDP udp;
    precise_sntp sntp(udp, IPAddress(192, 168, 178, 1));
    void setup() {}
    void loop() {
    sntp.update();
    }
  */
  precise_sntp(UDP &udp, IPAddress ntp_server_ip);

  /*
    initialization of the class with a specific time server name

    Example:

    #include <EthernetUdp.h>
    #include <precise_sntp.h>
    EthernetUDP udp;
    precise_sntp sntp(udp, "pool.ntp.org");
    void setup() {}
    void loop() {
    sntp.update();
    }
  */
  precise_sntp(UDP &udp, const char* ntp_server_name);

  /*
    Set the used poll exponent range.

    In NTPv4 the poll exponent ranges from 4 (16 s) to 17 (36 h).

    The poll exponent tau defines the allowed poll interval of 2^tau seconds.

    The method `update` does not do faster polls.

    The method `update_adapt_poll_period` does not do faster polls and also
    adapt the poll period by incrementing or decrementing the used poll
    exponent in the defined range.
   */
  void set_poll_exponent_range(uint8_t min_poll, uint8_t max_poll);

  /*
    This checks if millis overflow.

    This method is called from update() and update_adapt_poll_period().
    Therefore it is not necessary to call this function in addition.

    The returned epoch in other methods is always calculated using millis().
    millis() will overflow after about 50 days and the returned epoch is not
    correct anymore. To handle this check_millis_overflow() tries to count
    the overflows of millis().

    If you do not call update() or update_adapt_poll_period() at least every
    24 days, you should call check_millis_overflow() at least every 24 days.

    This should work for about 8926 years, then the check counter will overflow.
  */
  void check_millis_overflow();

  /*
    Get time from server.

    After the first contact the poll policy of the server is used.
    The smallest allowed poll exponent is used. This means as often
    as allowed we get the time from the server.

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
    Get time from server and adapt the poll period.

    After the first contact the poll policy of the server is used.
    The smallest allowed poll exponent is used. On successful polls
    the poll exponent iss incremented. On unsuccessful polls the poll
    exponent is decremented.

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
  uint8_t update_adapt_poll_period();

  /*
    Returns the actual time as epoch (unix timestamp) in seconds.
   */
  time_t get_epoch(); // seconds

  /*
    Returns the actual time as epoch (unix timestamp) in seconds
    with a precision of about milliseconds.
  */
  double dget_epoch(); // seconds with milliseconds as fraction

  /*
    Returns the actual time as epoch (unix timestamp) in seconds
    and fractions of the second. This is the full calculated precision.
    But since the local clock is not adjusted this precision reflects
    not the reality. You could expect a precision of a few milliseconds.
  */
  timestamp_format tget_epoch(); // seconds + fraction of the second

  /*
    returns true if clock was once updated and
    the next update time is not reached
  */
  bool is_synchronized();

  /*
    This is intended to speed the initial synchronization.
    Use it for example in the setup routine.

    It does n times getting the time from the server with a delay of d
    milliseconds between. The first times the transmit timestamp of the
    ntp server is used.

    Since no mitigation algorithms (clock filter, clock adjust, ...) are
    done, it does not make sense to do a high number of calls. Therefore
    n is 2 as default and not the typical 8 used by NTP implementations.

    The return value is combined results of all calls. In the least significant
    4 Bits is the return value of the first call. In the next 4 Bits the
    return value of the second call. And so on.

    In each 4 Bits the value can be interpreted like the return error codes
    from force_update.

    Therefore n should not be larger than 15.
    If it is larger, it will be ignored and 8 is used.
   */
  uint64_t force_update_iburst(uint8_t n=2, uint16_t d=2000);

  /*
    Force get time from server, should only be used in local networks with
    local time server for debugging purpose.

    If use_transmit_timestamp is set to true, the transmit timestamp of the
    server is used. This is not optimal.
    If use_transmit_timestamp is set to false (the default), the averaged
    offset theta from ntp server is used to decide whether to use the
    transmit timestamp for theta > 1 second or to correct the time using
    the offset theta. The latter case make sense and is after initial
    time setting the expected behavior.

    returns an error code:

    0: success
    2: cannot use local port _localport
    3: cannot start connection
    4: problems writing data
    5: packet was not send
    6: got no answer from server
    7: sanity check fail, answer from server is bogus
    8: sanity check fail, server is not syncronized
  */
  uint8_t force_update(bool use_transmit_timestamp=false);

  /*
    Returns the ntp local clock in ntp timestamp format.
  */
  struct ntp_timestamp_format_struct _get_local_clock();

  /*
    return the millis when the last update from the time server was successful
  */
  unsigned long get_last_update();

 private:
  IPAddress _ntp_server_ip;
  const char* _ntp_server_name;
  UDP* _udp;
  uint16_t _localport = 1234;
  union ntp_local_clock_union _ntp_local_clock;
  unsigned long _last_clock_update = 0;
  unsigned long _last_update = 0;
  unsigned long _next_update_period = 0;
  uint8_t _poll_exponent = 1;
  bool _is_synced = false;
  uint8_t _min_poll_exponent = 6; // 4 is NTPv4 minimal poll exponent (16 s)
  uint8_t _max_poll_exponent = 10; // 17 is NTPv4 maximal poll exponent (36 h)
  uint16_t _millis_overflow_count = 0;
};
