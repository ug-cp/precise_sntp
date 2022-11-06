---
author: Daniel Mohr
date: 2022-11-06
license: BSD 3-Clause License
---

# precise_sntp -- precise Simple Network Time Protocol

[[_TOC_]]

This library is an implementation of the Simple Network Time Protocol (SNTPv4)
for the arduino platform.

In contrast to other implementations this library allows sub-second precision.

As all (S)NTP implementation for arduino it is simplified.

Since in SNTP no mitigation algorithms (clock filter, clock adjust, ...) are
done, a precision of sub-milliseconds is not meaningful.
Therefore we concentrate here to get the time with a precision of a
few milliseconds. The returned epoch in different formats is always
calculated using `millis()`.

## Install

To install this library use `Add .ZIP Library ...` in your Arduino IDE
to add the zip archive `precise_sntp-main.zip` you can download from
this repository.

Or just copy the content of this repository to your Arduino libraries, e. g.:

## Usage

## Tested

It was tested on SAMD21 (Arduino MKR1000, Arduino MKR Zero and
Arduino MKR WiFi 1010).

## Examples

In the folder [examples](examples) you can find some examples.

## License, Copyright: BSD 3-Clause License

Copyright (c) 2022, Daniel Mohr and
                    University of Greifswald, Institute of Physics,
                    Colloidal (dusty) plasma group

All rights reserved.

see: [LICENSE](LICENSE)
