/*
  Author: Daniel Mohr
  Date: 2022-11-07

  Since src/precise_sntp.cpp is compiled together with this unittest
  test/unit_test_htonl_htons.cpp, we have to make at least the compiling
  working.

  This file will include Udp.h (from arduino core) for normal processing.

  For the unittest it is overwritten to mock Udp.h.
*/

#include <Udp.h>
