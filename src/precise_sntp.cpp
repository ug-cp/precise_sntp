/*
  Author: Daniel Mohr
  Date: 2022-12-15

  For more information look at the README.md.

  We try here to follow: https://www.rfc-editor.org/rfc/rfc5905
*/

#include <precise_sntp.h>

#include <precise_sntp_htonl_htons.h>
#include <precise_sntp_ntp_local_clock_union2uint64.h>
#include <precise_sntp_ntp_timestamp_format2doubleepoch.h>

#define NTP_PACKET_SIZE 48
#define NTP_MIN_POLL_EXPONENT 4
#define NTP_MAX_POLL_EXPONENT 17

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
  _ntp_server_name = "pool.ntp.org";
  memset(_ntp_local_clock.as_bytes, 0, 8);
}

precise_sntp::precise_sntp(UDP &udp, IPAddress ntp_server_ip) {
  _udp = &udp;
  _ntp_server_ip = ntp_server_ip;
  _ntp_server_name = NULL;
  memset(_ntp_local_clock.as_bytes, 0, 8);
}

precise_sntp::precise_sntp(UDP &udp, const char* ntp_server_name) {
  _udp = &udp;
  _ntp_server_name = ntp_server_name;
  memset(_ntp_local_clock.as_bytes, 0, 8);
}

void precise_sntp::set_poll_exponent_range(uint8_t min_poll, uint8_t max_poll) {
  if (NTP_MIN_POLL_EXPONENT <= min_poll) {
    _min_poll_exponent = min_poll;
  } else {
    _min_poll_exponent = NTP_MIN_POLL_EXPONENT;
  }
  if (max_poll <= NTP_MAX_POLL_EXPONENT) {
    _max_poll_exponent = max_poll;
  } else {
    _max_poll_exponent = NTP_MAX_POLL_EXPONENT;
  }
}

void precise_sntp::check_millis_overflow() {
  static uint32_t last_check = 0;
  const uint32_t mtime = millis();
  if (mtime < last_check) {
    _millis_overflow_count++;
  }
  last_check = mtime;
}

uint8_t precise_sntp::update() {
  check_millis_overflow();
  if (_is_synced && (_next_update_period > millis() - _last_update)) {
    return 1;
  }
  return force_update();
}

uint8_t precise_sntp::update_adapt_poll_period() {
  const bool was_synchronized = (_is_synced) &&
    (_last_update + 2 * 1000 * (1 << _poll_exponent) > millis());
  const uint8_t old_poll_exponent = _poll_exponent;
  const uint8_t ret = update();
  if (ret == 1) {
    return ret;
  }
  if (was_synchronized && (ret == 0)) {
    if (old_poll_exponent < _max_poll_exponent) {
      _poll_exponent = old_poll_exponent + 1; // adapt poll period
      _next_update_period = 1000 * (1 << _poll_exponent);
    }
  } else if (ret > 1) {
    if (_min_poll_exponent < old_poll_exponent) {
      _poll_exponent = old_poll_exponent - 1; // adapt poll period
      if (was_synchronized) {
	_next_update_period = 1000 * (1 << _poll_exponent);
      }
    }
  }
  return ret;
}

uint64_t precise_sntp::force_update_iburst(uint8_t n, uint16_t d) {
  if (n > 15) {
    n = 8;
  }
  uint64_t ret = force_update(true);
  for (uint8_t i = 1; i < n; i++) {
    delay(d);
    ret += force_update() << (i * 4);
  }
  return ret;
}

uint8_t precise_sntp::force_update(bool use_transmit_timestamp) {
  _is_synced = false;
#ifdef PRECISE_SNTP_DEBUG
  Serial.println("update");
  Serial.print("_poll_exponent ");
  Serial.println(_poll_exponent);
#endif
  union ntp_packet_union ntp_packet;
  memset(ntp_packet.as_bytes, 0, NTP_PACKET_SIZE);
  // set leap=3 (no warning), version=4, mode=3 (client):
  ntp_packet.as_ntp_packet.leap_version_mode = 0xE3;
  ntp_packet.as_ntp_packet.stratum = 0; // stratum=0 (unspecified or invalid)
  ntp_packet.as_ntp_packet.poll = _poll_exponent; // poll=6 (default min poll interval)
  ntp_packet.as_ntp_packet.precision = 0xEC;
  const struct ntp_timestamp_format_struct t1 = get_local_clock();
  ntp_packet.as_ntp_packet.xmt = t1;
  ntp_timestamp_format_hton(&(ntp_packet.as_ntp_packet.xmt));
  if (_udp->begin(_localport) != 1) {
#ifdef PRECISE_SNTP_DEBUG
    Serial.println("_localport not working");
#endif
    return 2;
  }
  if (_ntp_server_name) {
    if (_udp->beginPacket(_ntp_server_name, 123) != 1) {
#ifdef PRECISE_SNTP_DEBUG
      Serial.println("cannot start connection");
#endif
      _next_update_period += 1000;
      return 3;
    }
  } else {
    if (_udp->beginPacket(_ntp_server_ip, 123) != 1) {
#ifdef PRECISE_SNTP_DEBUG
      Serial.println("cannot start connection");
#endif
      _next_update_period += 1000;
      return 3;
    }
  }
  if (_udp->write(ntp_packet.as_bytes, NTP_PACKET_SIZE) != NTP_PACKET_SIZE) {
#ifdef PRECISE_SNTP_DEBUG
    Serial.println("problems writing data");
#endif
    _next_update_period += 1000;
    return 4;
  }
  if (_udp->endPacket() != 1) {
#ifdef PRECISE_SNTP_DEBUG
    Serial.println("packet was not send");
#endif
    _next_update_period += 1000;
    return 5;
  }
  unsigned long start_waiting = millis();
  int packetSize;
  while (((packetSize = _udp->parsePacket()) != NTP_PACKET_SIZE) &&
	 (start_waiting + 1000)) {
  }
  if (packetSize != NTP_PACKET_SIZE) {
#ifdef PRECISE_SNTP_DEBUG
    Serial.println("got no answer from server");
#endif
    _next_update_period += 1000;
    return 6;
  }
  const struct ntp_timestamp_format_struct t4 = get_local_clock();
  _udp->read(ntp_packet.as_bytes, NTP_PACKET_SIZE);
  // adapt byte order (skipping not used values):
  // ntp_short_format_ntoh(&ntp_packet.as_ntp_packet.rootdelay);
  // ntp_short_format_ntoh(&ntp_packet.as_ntp_packet.rootdisp);
#ifdef PRECISE_SNTP_DEBUG
  ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.reftime);
#endif
  ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.org);
  ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.rec);
  ntp_timestamp_format_ntoh(&ntp_packet.as_ntp_packet.xmt);
  // sanity check
  if ((t1.seconds != ntp_packet.as_ntp_packet.org.seconds) ||
      (t1.fraction != ntp_packet.as_ntp_packet.org.fraction)) {
#ifdef PRECISE_SNTP_DEBUG
    Serial.println("sanity check fail, answer from server is bogus");
#endif
    _next_update_period += 1000;
    return 7;
  }
  if ((ntp_packet.as_ntp_packet.stratum < 1) ||
      (15 < ntp_packet.as_ntp_packet.stratum)) {
#ifdef PRECISE_SNTP_DEBUG
    Serial.println("sanity check fail, server is not syncronized");
#endif
    // handle stratum == 0 as KoD
    _next_update_period += 1000 * (1 << _min_poll_exponent);
    return 8;
  }
  // go on
  _last_update = millis();
  const struct ntp_timestamp_format_struct t2 = ntp_packet.as_ntp_packet.rec;
  const struct ntp_timestamp_format_struct t3 = ntp_packet.as_ntp_packet.xmt;
  if (ntp_packet.as_ntp_packet.poll < _min_poll_exponent) {
    _poll_exponent = _min_poll_exponent;
  } else if (_max_poll_exponent < ntp_packet.as_ntp_packet.poll) {
    _poll_exponent = _max_poll_exponent;
  } else {
    _poll_exponent = ntp_packet.as_ntp_packet.poll;
  }
  _next_update_period = 1000 * (1 << _poll_exponent);
#ifdef PRECISE_SNTP_DEBUG
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
#endif
  // using the own clock, we can calculate here some statistics, e. g.:
  // offset theta of B relative to A:
  const uint64_t T1 = (((uint64_t) t1.seconds) << 32) + t1.fraction;
  const uint64_t T2 = (((uint64_t) t2.seconds) << 32) + t2.fraction;
  const uint64_t T3 = (((uint64_t) t3.seconds) << 32) + t3.fraction;
  const uint64_t T4 = (((uint64_t) t4.seconds) << 32) + t4.fraction;
  // calculate offset theta from ntp server
  // theta = 0.5 * (T2+T3) - 0.5 * (T1+T4)
  const int64_t theta = 0.5 * (((int64_t) T2 - (int64_t) T1) +
			       ((int64_t) T3 - (int64_t) T4));
#ifdef PRECISE_SNTP_DEBUG
  Serial.print("theta [ms]: ");
  Serial.println((int16_t) (((theta >> 16) * 1000) >> 16));
  Serial.print("theta [us]: ");
  Serial.println((int16_t) (((theta >> 16) * 1000000) >> 16));
  // calculate the round-trip delay:
  const uint64_t delta = (T4 - T1) - (T3 - T2);
  Serial.print("delta [ms]: ");
  Serial.println((uint16_t) (((delta >> 16) * 1000) >> 16));
  Serial.print("delta [us]: ");
  Serial.println((uint16_t) (((delta >> 16) * 1000000) >> 16));
  Serial.print("poll: ");
  Serial.println(ntp_packet.as_ntp_packet.poll);
  Serial.print("statum: ");
  Serial.println(ntp_packet.as_ntp_packet.stratum);
#endif
  if ((((uint64_t) abs(theta)) > (((uint64_t) 1)<<32)) ||
      (use_transmit_timestamp)) {
    // large error, we will use the transmit timestamp of the server
#ifdef PRECISE_SNTP_DEBUG
    Serial.println("large error, we will use the transmit timestamp of the server");
#endif
    _ntp_local_clock.as_timestamp.seconds =
      ntp_packet.as_ntp_packet.xmt.seconds;
    _ntp_local_clock.as_timestamp.fraction =
      ntp_packet.as_ntp_packet.xmt.fraction;
    _last_clock_update = millis();
    _millis_overflow_count = 0;
  } else {
    const uint64_t my_local_clock =
      (int64_t) _ntp_local_clock_union2uint64(_ntp_local_clock) + theta;
    _ntp_local_clock.as_timestamp.seconds = (uint32_t) (my_local_clock >> 32);
    _ntp_local_clock.as_timestamp.fraction =
      (uint32_t) (my_local_clock & 0x00000000FFFFFFFFULL);
  }
#ifdef PRECISE_SNTP_DEBUG
  Serial.print(" reftime ");
  Serial.println(ntp_packet.as_ntp_packet.reftime.seconds);
  Serial.print(" org ");
  Serial.println(ntp_packet.as_ntp_packet.org.seconds);
  Serial.print(" rec ");
  Serial.println(ntp_packet.as_ntp_packet.rec.seconds);
  Serial.print(" xmt ");
  Serial.println(ntp_packet.as_ntp_packet.xmt.seconds);
  uint32_t epoch = _ntp_local_clock.as_timestamp.seconds - 2208988800UL;
  uint16_t epoch_milli =
    (uint16_t) (((float) 1000) *
		(((float) (_ntp_local_clock.as_timestamp.fraction >> 22)) /
		 ((float) (1<<10))));
  Serial.print(epoch);
  Serial.print(".");
  if (epoch_milli < 100) {
    Serial.print("0");
    if (epoch_milli < 10) {
      Serial.print("0");
    }
  }
  Serial.print(epoch_milli);
  double fepoch =
    (double) epoch +
    (((double) _ntp_local_clock.as_timestamp.fraction) /
     ((double) 4294967295UL)); // ~= seconds + fraction / (2**32)
  Serial.print(" ");
  Serial.println(fepoch);
#endif
  _is_synced = true;
  return 0;
}

struct ntp_timestamp_format_struct precise_sntp::get_local_clock() {
  const uint64_t mtime = (((uint64_t) _millis_overflow_count) << 32) + millis();
  uint64_t my_local_clock;
  my_local_clock =
    (int64_t) _ntp_local_clock_union2uint64(_ntp_local_clock) +
    ((((mtime - _last_clock_update) << 16) / 1000) << 16);
  struct ntp_timestamp_format_struct now;
  now.seconds = (uint32_t) (my_local_clock >> 32);
  now.fraction = (uint32_t) (my_local_clock & 0x00000000FFFFFFFFULL);
  return now;
}

time_t precise_sntp::get_epoch() {
  const struct ntp_timestamp_format_struct now = get_local_clock();
  return ntp_timestamp_seconds2epoch(now);
}

double precise_sntp::dget_epoch() {
  const struct ntp_timestamp_format_struct now = get_local_clock();
  return ntp_timestamp_format2doubleepoch(now);
}

timestamp_format precise_sntp::tget_epoch() {
  const struct ntp_timestamp_format_struct ntp_now = get_local_clock();
  struct timestamp_format now;
  now.seconds = ntp_timestamp_seconds2epoch(ntp_now);
  now.fraction = ntp_now.fraction;
  return now;
}
unsigned long precise_sntp::get_last_update() {
  return _last_update;
}

bool precise_sntp::is_synchronized() {
  return (_is_synced &&
	  (_last_update > millis() - 1000 * (1 << _poll_exponent)));
}
