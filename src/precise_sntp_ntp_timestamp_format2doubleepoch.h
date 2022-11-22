/*
  Author: Daniel Mohr
  Date: 2022-11-08
*/

#define ntp_timestamp_seconds2epoch(x) (x.seconds - 2208988800UL)

#define ntp_timestamp_format2doubleepoch(x) \
  (((double) ntp_timestamp_seconds2epoch(x)) + \
   ((double) ((uint16_t) (((x.fraction >> 16) * 1000) >> 16))) / 1000.0)
