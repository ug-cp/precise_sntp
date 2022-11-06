/*
  Author: Daniel Mohr
  Date: 2022-11-06

  For more information look at the README.md.

  We try here to follow: https://www.rfc-editor.org/rfc/rfc5905
*/

#include <precise_sntp.h>

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
#define htons(i) ( (((i)<<8) & 0xF0) | \
		   (((i)>>8) & 0x0F) )
/*
#define htons(i) (uint16_t) (( ((uint16_t) (i)) << 8) | \
                             ( ((uint16_t) (i)) >> 8))
*/
#endif

#ifndef ntohs
#define ntohs htons
#endif

#define NTP_PACKET_SIZE 48

struct ntp_short_format_struct { // 4 bytes
  uint16_t seconds;
  uint16_t fraction;
};
struct ntp_packet_struct { // 48 bytes, all in network byte order
  byte leap_version_mode; // leap indicator, version number, mode
  byte stratum;
  byte poll; // poll exponent
  byte precision; // precision exponent
  struct ntp_short_format_struct rootdelay; // root delay
  struct ntp_short_format_struct rootdisp; // root dispersion
  byte refid[4]; // reference ID
  struct ntp_timestamp_format_struct reftime; // reference timestamp
  struct ntp_timestamp_format_struct org; // origin timestamp
  struct ntp_timestamp_format_struct rec; // receive timestamp
  struct ntp_timestamp_format_struct xmt; // transmit timestamp
};
union ntp_packet_union {
  byte as_bytes[NTP_PACKET_SIZE];
  struct ntp_packet_struct as_ntp_packet;
};

static inline void ntp_short_format_ntoh(ntp_short_format_struct *t) {
  t->seconds = ntohs(t->seconds);
  t->fraction = ntohs(t->fraction);
}

static inline void ntp_timestamp_format_ntoh(ntp_timestamp_format_struct *t) {
  t->seconds = ntohl(t->seconds);
  t->fraction = ntohl(t->fraction);
}

static inline void ntp_timestamp_format_hton(ntp_timestamp_format_struct *t) {
  t->seconds = htonl(t->seconds);
  t->fraction = htonl(t->fraction);
}

precise_sntp::precise_sntp(UDP &udp) {
  _udp = &udp;
  IPAddress ntpip(192, 168, 178, 1);
  _ntp_server_ip = ntpip;
  memset(_ntp_local_clock.as_bytes, 0, 8);
}

precise_sntp::precise_sntp(UDP &udp, IPAddress ntp_server_ip) {
  _udp = &udp;
  _ntp_server_ip = ntp_server_ip;
  memset(_ntp_local_clock.as_bytes, 0, 8);
}

bool precise_sntp::update() {
  Serial.println("update");
  union ntp_packet_union ntp_packet;
  memset(ntp_packet.as_bytes, 0, 48);
  // set leap=3 (no warning), version=4, mode=3 (client):
  ntp_packet.as_ntp_packet.leap_version_mode = 0xE3;
  ntp_packet.as_ntp_packet.stratum = 0; // stratum=0 (unspecified or invalid)
  ntp_packet.as_ntp_packet.poll = 6; // poll=6 (default min poll interval)
  ntp_packet.as_ntp_packet.precision = 0xEC;
  const struct ntp_timestamp_format_struct t1 = _get_local_clock();
  ntp_packet.as_ntp_packet.xmt = t1;
  ntp_timestamp_format_hton(&(ntp_packet.as_ntp_packet.xmt));
  _udp->begin(_localport);
  _udp->beginPacket(_ntp_server_ip, 123);
  _udp->write(ntp_packet.as_bytes, 48);
  _udp->endPacket();
  unsigned long start_waiting = millis();
  int packetSize;
  while (((packetSize = _udp->parsePacket()) != NTP_PACKET_SIZE) &&
	 (start_waiting + 1000)) {
  }
  const struct ntp_timestamp_format_struct t4 = _get_local_clock();
  if (packetSize == NTP_PACKET_SIZE) {
    _udp->read(ntp_packet.as_bytes, NTP_PACKET_SIZE);
    // adapt byte order
    ntp_short_format_ntoh(&ntp_packet.as_ntp_packet.rootdelay);
    ntp_short_format_ntoh(&ntp_packet.as_ntp_packet.rootdisp);
    ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.reftime);
    ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.org);
    ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.rec);
    ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.xmt);
    // check
    if ((t1.seconds != ntp_packet.as_ntp_packet.org.seconds) ||
	(t1.fraction != ntp_packet.as_ntp_packet.org.fraction)){
      return false;
    }
    // go on
    const struct ntp_timestamp_format_struct t2 = ntp_packet.as_ntp_packet.rec;
    const struct ntp_timestamp_format_struct t3 = ntp_packet.as_ntp_packet.xmt;
    Serial.print("t1: ");
    Serial.print(t1.seconds);
    Serial.print(".");
    Serial.println(t1.fraction);
    Serial.print("t2: ");
    Serial.print(t2.seconds);
    Serial.print(".");
    Serial.println(t2.fraction);
    Serial.print("t3: ");
    Serial.print(t3.seconds);
    Serial.print(".");
    Serial.println(t3.fraction);
    Serial.print("t4: ");
    Serial.print(t4.seconds);
    Serial.print(".");
    Serial.println(t4.fraction);
    // using the own clock, we can calculate here some statistics, e. g.:
    // offset of B relative to A:
    uint64_t T1 = (((uint64_t) t1.seconds) << 32) + t1.fraction;
    uint64_t T2 = (((uint64_t) t2.seconds) << 32) + t2.fraction;
    uint64_t T3 = (((uint64_t) t3.seconds) << 32) + t3.fraction;
    uint64_t T4 = (((uint64_t) t4.seconds) << 32) + t4.fraction;
    int64_t theta = 0.5 * (((int64_t) T2 - (int64_t) T1) +
			   ((int64_t) T3 - (int64_t) T4));
    Serial.print("theta [ms]: ");
    Serial.println((int16_t) (((theta >> 16) * 1000) >> 16));
    Serial.print("theta [us]: ");
    Serial.println((int16_t) (((theta >> 16) * 1000000) >> 16));
    if (abs(theta) > (((uint64_t) 1)<<32)) {
      // large error, we will use the transmit timestamp of the server
      Serial.println("large error, we will use the transmit timestamp of the server");
      _ntp_local_clock.as_timestamp.seconds = ntp_packet.as_ntp_packet.xmt.seconds;
      _ntp_local_clock.as_timestamp.fraction = ntp_packet.as_ntp_packet.xmt.fraction;
      _last_update = millis();
    } else {
      int64_t my_local_clock = (((uint64_t) _ntp_local_clock.as_timestamp.seconds) << 32) + _ntp_local_clock.as_timestamp.fraction;
      my_local_clock = my_local_clock + theta;
      _ntp_local_clock.as_timestamp.seconds = (uint32_t) (my_local_clock >> 32);
      _ntp_local_clock.as_timestamp.fraction = (uint32_t) (my_local_clock & 0x00000000FFFFFFFFULL);
    }

    Serial.print(" reftime ");
    Serial.println(ntp_packet.as_ntp_packet.reftime.seconds);
    Serial.print(" org ");
    Serial.println(ntp_packet.as_ntp_packet.org.seconds);
    Serial.print(" rec ");
    Serial.println(ntp_packet.as_ntp_packet.rec.seconds);
    Serial.print(" xmt ");
    Serial.println(ntp_packet.as_ntp_packet.xmt.seconds);
    
    uint32_t epoch = _ntp_local_clock.as_timestamp.seconds - 2208988800UL;
    uint16_t epoch_milli = (uint16_t) (((float) 1000) * (((float) (_ntp_local_clock.as_timestamp.fraction >> 22)) / ((float) (1<<10))));
    Serial.print(epoch);
    Serial.print(".");
    if (epoch_milli < 100) {
      Serial.print("0");
      if (epoch_milli < 10) {
	Serial.print("0");
      }
    }
    Serial.print(epoch_milli);
    double fepoch = (double) epoch + ((double) _ntp_local_clock.as_timestamp.fraction) / ((double) 4294967295UL); // ~= seconds + fraction / (2**32)
    Serial.print(" ");
    Serial.print(fepoch);
  }
  return true;
}

struct ntp_timestamp_format_struct precise_sntp::_get_local_clock() {
  struct ntp_timestamp_format_struct now = _ntp_local_clock.as_timestamp;
  const unsigned long off = millis() - _last_update;
  const unsigned long off_seconds = off / 1000;
  now.seconds += off_seconds;
  // milli * 2**32 / 1e3 = fraction
  const uint32_t fraction = ((((uint32_t) (off - 1000 * off_seconds)) * (1<<16)) / 1000) * (1<<16);
  if (__builtin_uaddl_overflow(now.fraction, fraction, &now.fraction)) {
    now.seconds++;
  }
  return now;
}

time_t precise_sntp::get_epoch() {
  const struct ntp_timestamp_format_struct now = _get_local_clock();
  return now.seconds - 2208988800UL;
}
