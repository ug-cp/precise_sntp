---
author: Daniel Mohr
date: 2023-01-18
url: https://github.com/ug-cp/precise_sntp
license: BSD 3-Clause License
---

# precise_sntp -- precise Simple Network Time Protocol

This library is an implementation of the Simple Network Time Protocol (SNTPv4)
for the arduino platform.

In contrast to other implementations this library allows sub-second precision.
Further you can set the (S)NTP server to use by IP or by DNS name.

As all (S)NTP implementations for arduino it is simplified.

Since in SNTP no mitigation algorithms (clock filter, clock adjust, ...) are
used, a precision of sub-milliseconds is not meaningful.
Therefore we concentrate here on getting the time with a precision of a
few milliseconds.

The returned epoch in different formats is always calculated using `millis()`.
`millis()` will overflow after about 50 days and the returned epoch is not
correct anymore. To handle this you have to call `update`,
`update_adapt_poll_period` or `check_millis_overflow` at least every 24 days.
This should work for about 8926 years, then the check counter will overflow.

Since no clock adjust is done, the time is not always continuous. Every time
the time is updated from a time server the time could jump.

* home: [github.com/ug-cp/precise_sntp](https://github.com/ug-cp/precise_sntp)
* mirror: [gitlab.com/ug-cp/precise_sntp](https://gitlab.com/ug-cp/precise_sntp)
* Arduino Library Manager: [www.arduino.cc/reference/en/libraries/precise_sntp](https://www.arduino.cc/reference/en/libraries/precise_sntp/)

## Install

To use/install this library, you can open the Library Manager in the
Arduino IDE and install it from there. This is the recommended way.

Or if you use [Arduino CLI](https://arduino.github.io/arduino-cli/) the
recommended way is `arduino-cli lib install precise_sntp`

Or to install this library use `Add .ZIP Library ...` in your Arduino IDE
to add the zip archive `....zip` you can download from
this repository. Or you can use archives from
[latest release](https://github.com/ug-cp/precise_sntp/releases/latest).

Or just copy the content of this repository to your Arduino libraries, e. g.:

```shell
git clone https://github.com/ug-cp/precise_sntp.git ~/Arduino/libraries/precise_sntp
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

Maybe cou can use `force_update_iburst()` in the setup routine to speed up the
initial synchronization.

```c
void setup() {
  sntp.force_update_iburst();
}
```

## Tested

It was tested on SAMD21 (Arduino MKR1000 using Ethernet and
Arduino MKR WiFi 1010 using WI-FI) -- see [examples](examples).

## Examples

In the folder [examples](examples) you can find some examples.

## License, Copyright: BSD 3-Clause License

Copyright (c) 2022, 2023 Daniel Mohr and
                         University of Greifswald, Institute of Physics,
                         Colloidal (dusty) plasma group

All rights reserved.

see: [LICENSE](LICENSE)
