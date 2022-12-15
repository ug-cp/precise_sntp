/*
  precise_sntp example

  This example gets the time once and print the to serial.
  Between polling time check_millis_overflow() is called to ensure to get/print
  a correct time.

  Author: Daniel Mohr
  Date: 2022-12-15
*/

#include <Ethernet.h>
#include <EthernetUdp.h>

#include <precise_sntp.h>

#define SERIAL_BAUD_RATE 115200
#define SERIAL_TIMEOUT 1000
uint8_t mac[] = {0x02, 0x74, 0x72, 0x69, 0x67, 0x00};

EthernetUDP udp;

precise_sntp sntp(udp);

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
  sntp.force_update_iburst();
}

void loop() {
  sntp.check_millis_overflow();
  Serial.print("epoch: ");
  Serial.println(sntp.dget_epoch(), 1);
  delay(1000);
}
