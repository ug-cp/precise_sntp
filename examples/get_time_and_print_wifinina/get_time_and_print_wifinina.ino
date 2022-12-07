/*
  precise_sntp example

  This is a basic example. It gets the time and print it to serial.

  We use here WiFiNINA, which promised to provide NTP time.
  But it only returns this time with a precision of seconds.
  Further you can not define which NTP server is used and therefore you do
  not get a time without access to internet.

  Using precise_sntp you can define the used (S)NTP server and
  get sub-second precision.

  Author: Daniel Mohr
  Date: 2022-11-24
*/

#include <WiFiNINA.h>
#include <WiFiUdp.h>

#include <precise_sntp.h>

// in "arduino_secrets.h" should be defined:
// #define SECRET_SSID "foo"
// #define SECRET_PASS "bar"
#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

#define SERIAL_BAUD_RATE 115200
#define SERIAL_TIMEOUT 1000

WiFiUDP udp;

//precise_sntp sntp(udp);
precise_sntp sntp(udp, IPAddress(192, 168, 178, 1));
// precise_sntp sntp(udp, "pool.ntp.org");

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.setTimeout(SERIAL_TIMEOUT);
  while (!Serial);
  Serial.println("-- serial initialized --");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(300);
  }
  Serial.println("-- wifi initialized --");
  Serial.println("-- start --");
}

void loop() {
  sntp.update();
  Serial.print("WiFi.getTime: ");
  Serial.println(WiFi.getTime());
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
