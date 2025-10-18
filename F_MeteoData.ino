
void getDailyWeatherForecast() {
  // 1) Posizione (rimane com’è)
  getLocation();

  // 2) API Open-Meteo: con timezone=auto le date/ore sono già locali
  String url =
    "https://api.open-meteo.com/v1/forecast?latitude=" + lati +
    "&longitude=" + longi +
    "&current=temperature_2m"  // se ti serve la corrente
    "&daily=weathercode,temperature_2m_max,temperature_2m_min,precipitation_sum,sunrise,sunset"
    "&timezone=auto";

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    // 3) Parse JSON
    deserializeJson(doc, payload);
    JsonObject daily = doc["daily"];

    // NIENTE utcOffset: non serve per il day-of-week
    temperatureCurrent = doc["current"]["temperature_2m"].as<float>();

    timeArrayJson     = daily["time"].as<JsonArray>();               // ["YYYY-MM-DD", ...]
    weathercode       = daily["weathercode"].as<JsonArray>();
    temperatureMax    = daily["temperature_2m_max"].as<JsonArray>();
    temperatureMin    = daily["temperature_2m_min"].as<JsonArray>();
    precipitationSum  = daily["precipitation_sum"].as<JsonArray>();
    sunrise           = daily["sunrise"].as<JsonArray>();            // ["YYYY-MM-DDTHH:MM", ...]
    sunset            = daily["sunset"].as<JsonArray>();             // ["YYYY-MM-DDTHH:MM", ...]

    // 4) Esempio: converti alba[0]/tramonto[0] in minuti (HH:MM -> int)
    // (sono già orari locali grazie a timezone=auto)
    auto hhmmToMinutes = [](const String& isoDateTime) -> int {
      // isoDateTime: "YYYY-MM-DDTHH:MM"
      int tIndex = isoDateTime.indexOf('T');
      if (tIndex == -1 || tIndex + 5 >= (int)isoDateTime.length()) return 0;
      String hh = isoDateTime.substring(tIndex + 1, tIndex + 3);
      String mm = isoDateTime.substring(tIndex + 4, tIndex + 6);
      return hh.toInt() * 60 + mm.toInt();
    };

    if (sunrise.size() > 0) {
      sunriseTotalMinutes = hhmmToMinutes(sunrise[0].as<String>());
    }
    if (sunset.size() > 0) {
      sunsetTotalMinutes = hhmmToMinutes(sunset[0].as<String>());
    }

    // 5) Debug robusto con day-of-week corretto via TimeManager
    #ifdef DEBUG_WEATHER
      Serial.println(F("=== Daily forecast (local) ==="));
      Serial.print(F("Current temp: ")); Serial.println(temperatureCurrent);

      for (int i = 0; i < timeArrayJson.size(); i++) {
        String iso = timeArrayJson[i].as<String>();              // "YYYY-MM-DD"
        String dow = TimeManager::weekdayNameFromISO(iso);       // "Lunedì", ...
        Serial.print(F("\nGiorno: ")); Serial.print(iso);
        Serial.print(F(" (")); Serial.print(dow); Serial.println(F(")"));
        Serial.print(F("  Meteo code: ")); Serial.println(weathercode[i].as<int>());
        Serial.print(F("  T MAX: ")); Serial.print(temperatureMax[i].as<float>()); Serial.println(F(" °C"));
        Serial.print(F("  T MIN: ")); Serial.print(temperatureMin[i].as<float>()); Serial.println(F(" °C"));
        Serial.print(F("  Prec: ")); Serial.print(precipitationSum[i].as<float>()); Serial.println(F(" mm"));
        Serial.print(F("  Alba: ")); Serial.println(sunrise[i].as<String>());   // "YYYY-MM-DDTHH:MM"
        Serial.print(F("  Tramonto: ")); Serial.println(sunset[i].as<String>()); // idem
      }
    #endif

  } else {
    #ifdef DEBUG_WEATHER
      Serial.println("Errore HTTP Open-Meteo: " + String(httpCode));
    #endif
  }

  http.end();
}


void getLocation() {
	// URL del servizio IP-API
	String url = "http://ip-api.com/json/";

	// Inizializza HTTPClient
	HTTPClient http;
	http.begin(url);

	// Esegui la richiesta HTTP
	int httpCode = http.GET();

	// Verifica se la richiesta è andata a buon fine
	if (httpCode == HTTP_CODE_OK) {
		String payload = http.getString(); // Ottieni la risposta JSON
		#ifdef DEBUG_WEATHER
			Serial.println("Risposta API:");
			Serial.println(payload);
		#endif	

		// Analizza la risposta JSON
		JsonDocument doc;
		deserializeJson(doc, payload);

		// Estrai latitudine e longitudine
		float latitude = doc["lat"].as<float>();
		float longitude = doc["lon"].as<float>();

		lati=String(latitude, 6);
		longi= String(longitude, 6);

		// Stampa i dati
		#ifdef DEBUG_WEATHER
			Serial.println("Latitudine: " + lati);
			Serial.println("Longitudine: " + longi);
		#endif
	} else {
		#ifdef DEBUG_WEATHER
			Serial.println("Errore nella richiesta HTTP: " + String(httpCode));
		#endif
	}

	// Chiudi la connessione
	http.end();
}

DecodeMeteo weatherConvert(int currentDay){
  DecodeMeteo datiMeteo;
  switch (weathercode[currentDay].as<int>()){
    case 0:
      datiMeteo.decoted="Cielo sereno";
      datiMeteo.icona=icoSole;
      datiMeteo.icona32=icoSole32;
    break;

    case 1:
      datiMeteo.decoted="Principalmente sereno";
      datiMeteo.icona=icoQuasiSereno;
      datiMeteo.icona32=icoQuasiSereno32;
    break;

    case 2:
      datiMeteo.decoted="Parzialmente nuvoloso";
      datiMeteo.icona=icoSemiCoperto;
      datiMeteo.icona32=icoSemiCoperto32;
    break;

    case 3:
      datiMeteo.decoted="Coperto";
      datiMeteo.icona=icoCoperto;
      datiMeteo.icona32=icoCoperto32;
    break;

    case 45:
      datiMeteo.decoted="Nebbia";    
      datiMeteo.icona=icoNebbia;
      datiMeteo.icona32=icoNebbia32;
    break;
    case 48:
      datiMeteo.decoted="Nebbia con brina";
      datiMeteo.icona=icoNebbia;
      datiMeteo.icona32=icoNebbia32;
    break;
    case 51:
      datiMeteo.decoted="Pioviggine leggera";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 53:
      datiMeteo.decoted="Pioviggine moderata";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 55:
      datiMeteo.decoted="Pioviggine intensa";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 56:
      datiMeteo.decoted="Pioviggine gelata leggera";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 57:
      datiMeteo.decoted="Pioviggine gelata intensa";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 61:
      datiMeteo.decoted="Pioggia leggera";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 65:
      datiMeteo.decoted="Pioggia moderata";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 66:
      datiMeteo.decoted="Pioggia gelata leggera";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 67:
      datiMeteo.decoted="Pioggia gelata intensa";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 71:
      datiMeteo.decoted="Neve leggera";
      datiMeteo.icona=icoNeve;
      datiMeteo.icona32=icoNeve32;
    break;
    case 73:
      datiMeteo.decoted="Neve moderata";
      datiMeteo.icona=icoNeve;
       datiMeteo.icona32=icoNeve32;
    break;
    case 75:
      datiMeteo.decoted="Neve intensa";
      datiMeteo.icona=icoNeve;
       datiMeteo.icona32=icoNeve32;
    break;
    case 77:
      datiMeteo.decoted="Granelli di neve";
      datiMeteo.icona=icoNeve;
       datiMeteo.icona32=icoNeve32;
    break;
    case 80:
      datiMeteo.decoted="Rovesci di pioggia leggeri";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 81:
      datiMeteo.decoted="Rovesci di pioggia moderati";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 82:
      datiMeteo.decoted="Rovesci di pioggia violenti";
      datiMeteo.icona=icoPioggia;
      datiMeteo.icona32=icoPioggia32;
    break;
    case 85:
      datiMeteo.decoted="Rovesci di neve leggeri";
      datiMeteo.icona=icoNeve;
       datiMeteo.icona32=icoNeve32;
    break;
    case 86:
      datiMeteo.decoted="Rovesci di neve intensi";
      datiMeteo.icona=icoNeve;
       datiMeteo.icona32=icoNeve32;
    break;
    case 95:
      datiMeteo.decoted="Temporale leggero o moderato";
      datiMeteo.icona=icoTemporale;
      datiMeteo.icona32=icoTemporale32;
    break;
    case 96:
      datiMeteo.decoted="Temporale con leggera grandine";
      datiMeteo.icona=icoTemporale;
      datiMeteo.icona32=icoTemporale32;
    break;
    case 99:
      datiMeteo.decoted="Temporale con forte grandine";
      datiMeteo.icona=icoTemporale;
      datiMeteo.icona32=icoTemporale32;
    break;

    default:
      datiMeteo.decoted="Non disponibile";
	  datiMeteo.icona=icoNonDisponibile;
      datiMeteo.icona32=icoNonDisponibile32;
    break;

  }
  return datiMeteo;
}

String convertiOra(int tempo){
  int ore=tempo/60;
  int minuti=tempo-(ore*60);
  
  String stringa;
  String minuti_str;
  String ore_str;

  if(minuti>10){
    minuti_str="0"+String(minuti);
  }else{
    minuti_str=String(minuti);
  }

   if(ore<10){
    ore_str="0"+String(ore);
  }else{
    ore_str=String(ore);
  }




  return ore_str+":"+minuti_str;
}
