/*
  precise_sntp example

  This is a basic example. It gets the time and print it to serial.

  Author: Daniel Mohr
  Date: 2022-11-22
*/

#include <Ethernet.h>
#include <EthernetUdp.h>

#include <precise_sntp.h>

#define SERIAL_BAUD_RATE 115200
#define SERIAL_TIMEOUT 1000
uint8_t mac[] = {0x02, 0x74, 0x72, 0x69, 0x67, 0x00};

EthernetUDP udp;

precise_sntp sntp(udp);
// precise_sntp sntp(udp, IPAddress(192, 168, 178, 1));
// precise_sntp sntp(udp, "pool.ntp.org");

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.setTimeout(SERIAL_TIMEOUT);
  while (!Serial);
  Serial.println("-- start --");
  Ethernet.init(5);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      while (true) {
        Serial.println("Ethernet shield not found");
        delay(1000);
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable not connected");
    }
  }
}

void loop() {
  sntp.update();
  Serial.print("epoch: ");
  Serial.print(sntp.get_epoch());
  Serial.print(" ");
  Serial.print(sntp.dget_epoch(), 3);
  Serial.print(" ");
  Serial.print(sntp.tget_epoch().seconds);
  Serial.print("."); // this is not a decimal point!
  Serial.println(sntp.tget_epoch().fraction);
  delay(1000);
}
