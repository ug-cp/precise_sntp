/*
  Author: Daniel Mohr
  Date: 2022-11-08
*/

#define _ntp_local_clock_union2uint64(x) \
  ((((uint64_t) x.as_timestamp.seconds) << 32) + x.as_timestamp.fraction)
