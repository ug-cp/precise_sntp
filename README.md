---
author: Daniel Mohr
date: 2022-12-08
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
few milliseconds.

The returned epoch in different formats is always calculated using `millis()`.
`millis()` will overflow after about 50 days and the returned epoch is not
correct anymore. To handle this you have to call `update`,
`update_adapt_poll_period` or `check_millis_overflow` at least every 24 days.
This should work for about 8926 years, then the check counter will overflow.

Since no clock adjust is done, the time is not always continuous. Every time
the time is updated from a time server the time could jump.

## Install

To install this library use `Add .ZIP Library ...` in your Arduino IDE
to add the zip archive `precise_sntp-main.zip` you can download from
this repository.

Or just copy the content of this repository to your Arduino libraries, e. g.:

```shell
git clone https://.../precise_sntp.git ~/Arduino/libraries/precise_sntp
```

## Usage

This library does not create/handle network. For example, you can do this
on your own by using the libraries `Ethernet.h` and `EthernetUdp.h`.

After including this library you have to create an instance `sntp` of
the class and to provide a EthernetUDP instance to handle udp sending
and receiving.

```c
#include <EthernetUdp.h>
#include <precise_sntp.h>
EthernetUDP udp;
precise_sntp sntp(udp);
```

Further you should call `sntp.update()` regularly to get or update the
time from a time server.

```c
void loop() {
  sntp.update();
}
```

## Tested

It was tested on SAMD21 (Arduino MKR1000 using Ethernet and
Arduino MKR WiFi 1010 using WI-FI) -- see examples.

## Examples

In the folder [examples](examples) you can find some examples.

## License, Copyright: BSD 3-Clause License

Copyright (c) 2022, Daniel Mohr and
                    University of Greifswald, Institute of Physics,
                    Colloidal (dusty) plasma group

All rights reserved.

see: [LICENSE](LICENSE)
