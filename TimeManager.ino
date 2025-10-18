namespace {
  Preferences prefs;
  bool prefsOpened = false;

  const char* NVS_NS   = "tmgr";
  const char* KEY_EPOCH= "lastEpoch";
  const char* KEY_MS   = "lastMillis";

  bool     _begun = false;
  uint32_t _resyncIntervalMs = 6UL * 60UL * 60UL * 1000UL;
  uint32_t _nextResyncAtMs   = 0;

  void ensurePrefs() {
    if (!prefsOpened) { prefsOpened = prefs.begin(NVS_NS, false); }
  }

  void onTimeSync(struct timeval *tv) {
    ensurePrefs();
    prefs.putULong64(KEY_EPOCH, (uint64_t)time(nullptr));
    prefs.putUInt   (KEY_MS,    millis());
  }

  bool restoreCachedTime() {
    ensurePrefs();
    uint64_t savedEpoch = prefs.getULong64(KEY_EPOCH, 0ULL);
    uint32_t savedMs    = prefs.getUInt(KEY_MS, 0U);
    if (!savedEpoch || !savedMs) return false;

    uint64_t est = savedEpoch + (uint64_t)((millis() - savedMs) / 1000UL);
    struct timeval tv = { (time_t)est, 0 };
    settimeofday(&tv, nullptr);
    return (est > 1700000000ULL);
  }

  // nomi giorni in italiano (tm_wday: 0=Dom, ... 6=Sab)
  const char* itDays[7] = {
    "Domenica","Lunedi","Martedi","Mercoledi","Giovedi","Venerdi","Sabato"
  };
}

namespace TimeManager {

void begin(const char* tz, const char* s1, const char* s2, const char* s3,
           uint32_t firstSyncTimeoutMs, uint32_t resyncIntervalMs) {
  if (_begun) return;
  _begun = true;

  _resyncIntervalMs = resyncIntervalMs;

  setenv("TZ", tz, 1);
  tzset();

  esp_sntp_set_time_sync_notification_cb(onTimeSync);
  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, s1);
  esp_sntp_setservername(1, s2);
  esp_sntp_setservername(2, s3);
  esp_sntp_set_sync_interval(resyncIntervalMs);
  esp_sntp_init();

  uint32_t start = millis();
  while ((millis() - start) < firstSyncTimeoutMs && time(nullptr) < 1700000000) {
    delay(50);
  }
  if (time(nullptr) < 1700000000) {
    restoreCachedTime();
  }
  _nextResyncAtMs = millis() + _resyncIntervalMs;
}

bool timeIsValid() {
  return time(nullptr) >= 1700000000;
}

time_t nowUTC() {
  return time(nullptr);
}

tm localTime() {
  time_t now = time(nullptr);
  tm out;
  localtime_r(&now, &out);
  return out;
}

String timeHHMMSS() {
  tm t = localTime();
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  return String(buf);
}

String timeHHMM() {
  tm t = localTime();
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", t.tm_hour, t.tm_min);
  return String(buf);
}

String dateDDMMYYYY() {
  tm t = localTime();
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d-%02d-%04d", t.tm_mday, 1 + t.tm_mon, 1900 + t.tm_year);
  return String(buf);
}

int dayOfWeekNum() {
  tm t = localTime();
  return t.tm_wday; // 0=Dom ... 6=Sab
}

String dayOfWeekIT() {
  tm t = localTime();
  // itDays è già definito nello scope anonimo in cima al file
  //extern const char* itDays[7];
  return String(itDays[t.tm_wday]);
}

int minutesOfDay() {
  tm t = localTime();
  return t.tm_hour * 60 + t.tm_min;
}

String dateYYYYMMDD() {
  tm t = localTime();
  char buf[16];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday);
  return String(buf);
}

String dateTimeISO() {
  tm t = localTime();
  char buf[24];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday,
           t.tm_hour, t.tm_min, t.tm_sec);
  return String(buf);
}

time_t epochFromISODate(const String& iso, uint8_t hour, uint8_t minute) {
  // iso atteso: "YYYY-MM-DD"
  if (iso.length() < 10) return 0;
  int y = iso.substring(0,4).toInt();
  int m = iso.substring(5,7).toInt();
  int d = iso.substring(8,10).toInt();

  tm t = {};
  t.tm_year = y - 1900;
  t.tm_mon  = m - 1;
  t.tm_mday = d;
  t.tm_hour = hour;
  t.tm_min  = minute;
  t.tm_sec  = 0;
  t.tm_isdst = -1; // **KEY**: lascia calcolare a mktime se applicare la DST

  // mktime usa il fuso impostato (TZ)
  time_t epochLocal = mktime(&t);
  return epochLocal;
}


String weekdayName(time_t epoch) {
  if (!epoch) return String("?");
  tm t;
  localtime_r(&epoch, &t);
  // tm_wday: 0=Dom … 6=Sab
  return String(itDays[t.tm_wday]);
}

String weekdayNameFromISO(const String& isoDate) {
  return weekdayName(epochFromISODate(isoDate));
}

void loop() {
  if ((int32_t)(millis() - _nextResyncAtMs) >= 0) {
    esp_sntp_stop();
    delay(10);
    esp_sntp_init();
    _nextResyncAtMs = millis() + _resyncIntervalMs;
  }
}

} // namespace
