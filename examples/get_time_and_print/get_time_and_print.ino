/*
  precise_sntp example

  This is a basic example. It gets the time and print it to serial.

  Author: Daniel Mohr
  Date: 2022-11-06
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
  Ethernet.init(5); // MKR ETH shield
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
  Serial.println();
  sntp.update();
  Serial.println();
  Serial.print(sntp.get_epoch());
  Serial.println(" running");
  delay(1000);
  Serial.print(sntp.get_epoch());
  Serial.println(" running");
  delay(1000);
  Serial.print(sntp.get_epoch());
  Serial.println(" running");
  delay(1000);
  Serial.print(sntp.get_epoch());
  Serial.print(" running ");
  static int sleep_time = 1234;
  Serial.print("(sleep = ");
  Serial.print(sleep_time);
  Serial.println(")");
  delay(sleep_time);
  if (sleep_time < 60000) {
    sleep_time = 2 * sleep_time;
  }
}
