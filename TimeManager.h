namespace TimeManager {
  void begin(
    const char* tz = "CET-1CEST,M3.5.0,M10.5.0/3",
    const char* s1 = "0.it.pool.ntp.org",
    const char* s2 = "1.pool.ntp.org",
    const char* s3 = "time.cloudflare.com",
    uint32_t   firstSyncTimeoutMs = 10000,
    uint32_t   resyncIntervalMs   = 6UL * 60UL * 60UL * 1000UL
  );
  bool   timeIsValid();
  time_t nowUTC();
  tm     localTime();

  String timeHHMMSS();      // "HH:MM:SS"
  String dateYYYYMMDD();    // "YYYY-MM-DD"
  String dateTimeISO();     // "YYYY-MM-DD HH:MM:SS"
  int    dayOfWeekNum();     // oggi, 0=Dom ... 6=Sab
  String dayOfWeekIT();      // nome giorno di oggi in IT
  int    minutesOfDay();     // minuti trascorsi da mezzanotte (locale)
  String timeHHMM();        // "HH:MM"
  String dateDDMMYYYY();    // "DD-MM-YYYY"

  // Nuove utility per il meteo / parsing
  time_t epochFromISODate(const String& isoDateYYYYMMDD, uint8_t hour = 12, uint8_t minute = 0);
  String weekdayName(time_t epoch);                 // "Lunedì", "Martedì", ...
  String weekdayNameFromISO(const String& isoDate); // comodo: da "YYYY-MM-DD" → nome giorno

  void loop();
}
