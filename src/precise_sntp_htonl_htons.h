/*
  Author: Daniel Mohr
  Date: 2022-11-07

  These macros convert values between host and network byte order.

  It is assumed that host byte order is little-endian (most arduino platforms).
  Network byte order is big-endian.
*/

#pragma once

#ifndef htonl
#define htonl(i) ( (((i)<<24) & 0xFF000000UL) | \
                   (((i)<< 8) & 0x00FF0000UL) | \
                   (((i)>> 8) & 0x0000FF00UL) | \
                   (((i)>>24) & 0x000000FFUL) )
#endif

#ifndef ntohl
#define ntohl htonl
#endif

#ifndef htons
#define htons(i) ( (((i)<<8) & 0xFF00) | \
		   (((i)>>8) & 0x00FF) )
#endif

#ifndef ntohs
#define ntohs htons
#endif
