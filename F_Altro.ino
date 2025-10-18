void saveParameters(bool messaggio) {
	//Salva parametri modificabili dal menu ESP
  preferences.begin("Dati", false);    
    preferences.putInt("timerAttivo",   timerAttivo);
    preferences.putInt("cicloContinuo", cicloContinuo);
    preferences.putInt("scenarioAttivo",scenarioAttivo);
  preferences.end();

  pubblicaValoriCicli(false);   //riaggiorna mqtt senza salvare

  if(messaggio){
    tft.fillScreen(ST77XX_BLACK);
    drawText(0, 1, 3, ST77XX_WHITE, "Parametri salvati");
    delay(1000);
  }
}


/*________________________ RESET  _____________________________*/
void software_Reset(){
	#ifdef DEBUG_WIFI
		Serial.println("SW resetting........");
	#endif
	ESP.restart(); 
}

/********* controllo aggiornamenti firmware OTA *************/
void updateFirmware(const char* url) {
	HTTPClient http;
	http.begin(url); // URL del firmware
	
	int httpCode = http.GET();
	if (httpCode == HTTP_CODE_OK) {
		int contentLength = http.getSize();
		if (contentLength > 0) {
			if (Update.begin(contentLength)) {
				printDebug("OTA:Avvio aggiornamento...",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
				WiFiClient& client = http.getStream();
				size_t written = Update.writeStream(client);
				if (written == contentLength) {
					printDebug("OTA:Scrittura completata",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
				}else{
					printDebug("EOTA:rrore durante la scrittura",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
				}
				if (Update.end()) {
					printDebug("OTA:Aggiornamento completato",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
					if (Update.isFinished()) {
						printDebug("OTA:Riavvio...",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
						ESP.restart();
					} else {
						printDebug("OTA:Aggiornamento non completato",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
					}
				} else {
					printDebug("OTA:Errore durante l'aggiornamento",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
				}
			} else {
				printDebug("OTA:Spazio insufficiente per l'aggiornamento",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
			}
		} else {
			printDebug("OTA:Nessun contenuto disponibile",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
		}
	} else {
			printDebug("OTA:Errore durante il download",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
	}
	printDebug("OTA:Errore nell'aggiornamento",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
  http.end();
}

void checkForUpdate() {

  // controllo versione firmware
  { //blocco per isolare la chiamata http
    HTTPClient http;
    http.setUserAgent(mqtt_nodeID);
    http.begin(versionURL);
    int httpCode = http.GET();
    if (httpCode == 200) {
        String newVersion = http.getString();
        float serverVersion = newVersion.toFloat();
        digitalWrite(PIN_BL,HIGH);
        //if(connessoMqtt){client.publish(String(mqtt_nodeID+String(out_topic)+"V").c_str(), 	String(serverVersion).c_str(), true);}
        if (serverVersion > currentVersion) {
						printDebug("Nuovo firmware disponibile: "+newVersion+"\n Avvio aggiornamento...",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
						updateFirmware(URL);
        } else {
            printDebug("Firmware gia' aggiornato. \n Vers.: "+String(currentVersion),MQTT_DEBUG,SERIAL_DEBUG,false);
        }
    } else {
        printDebug("Errore nel controllo della versione.",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
    }
    http.end();
  }

/*
  { //blocco per isolare la chiamata http
    // controllo versione apk. Legge la versione e la pubblica
    HTTPClient http;
    http.setUserAgent(mqtt_nodeID);
    http.begin(versionURL_Apk);
    int httpCode = http.GET();
    if (httpCode == 200) {
        String newVersion = http.getString();
       // if(connessoMqtt){client.publish(String(mqtt_nodeID+String(out_topic)+"VA").c_str(), 	String(newVersion).c_str(), true);}
        printDebug("Versione apk: "+String(newVersion),MQTT_DEBUG,SERIAL_DEBUG,false);
    } else {
        printDebug("Errore nel controllo della versione Apk.",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
    }
    http.end();
  }
*/
}

String xorEncryptDecrypt(String message, String key) {
  String result = "";
  for (int i = 0; i < message.length(); i++) {
    char c = message[i] ^ key[i % key.length()];
    result += c;
  }
  return result;
}

String base64encode(String input) {
  const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String encoded = "";
  int i = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];
  
  int inputLen = input.length();
  int j = 0;

  while (inputLen--) {
    char_array_3[i++] = *(input.c_str() + j++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; i < 4 ; i++)
        encoded += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for(int j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (int j = 0; j < i + 1; j++)
      encoded += base64_chars[char_array_4[j]];

    while((i++ < 3))
      encoded += '=';
  }

  return encoded;
}

void selectDHT(){
  unsigned long start = millis();
  dht22.begin();
  delay(1000);  // ok, breve
  Serial.println("Bloccato DHT22 (prova lettura)");
  bool letturaOk = false;
  float t2 = NAN;

  while (millis() - start < 3000) {  // tenta per massimo 3 secondi
    t2 = dht22.readTemperature();
    if (!isnan(t2)) {
      letturaOk = true;
      break;
    }
    delay(200);  // breve attesa tra i tentativi
  }

  if (letturaOk) {
    activeDHT = &dht22;
    Serial.println("✅ Rilevato sensore: DHT22");}
  else {
    start = millis();
    dht11.begin();
    delay(1000);  // ok, breve
    Serial.println("Bloccato DHT11 (prova lettura)");
    bool letturaOk = false;
    float t1 = NAN;

    while (millis() - start < 3000) {  // tenta per massimo 3 secondi
      t1 = dht11.readTemperature();
      if (!isnan(t1)) {
        letturaOk = true;
        break;
      }
      delay(200);  // breve attesa tra i tentativi
    }

    if (letturaOk) {
      activeDHT = &dht11;
        Serial.println("✅ Rilevato sensore: DHT11");}
    else {   
      Serial.println("❌ DHT non rilevato o bloccato");
    }
  }
}

void sensorsRead(){
	if (client.connected()) {
		#ifdef DEBUG_MQTT
			Serial.println("Client MQTT connesso");
		#endif
	}else{
		#ifdef DEBUG_MQTT
     	Serial.println("Client MQTT disconnesso. Provo la riconnessione");
   	#endif
    connectServer();
	}
  // Sincronizza il tempo
  TimeManager::loop();
  // Aggiorna minuti attuali
  nowTotalMinutes = TimeManager::minutesOfDay();

  // RESET giornaliero al cambio di giorno
  if (TimeManager::dateDDMMYYYY() != lastResetDay) {
      albaEseguita = false;
      tramontoEseguito = false;
      lastResetDay = TimeManager::dateDDMMYYYY();
      checkForUpdate();
      Serial.println("🔄 Reset giornaliero eseguito.");
  }


  // assegna ora di avvio (in minuti)
 /* if(startHour==0){   // Avvio all'alba
    startTotalMinutes=sunriseTotalMinutes;
  }else{
    if(scenari[scenarioAttivo].ciclo==1){
      startTotalMinutes=sunriseTotalMinutes;
    }else{
      startTotalMinutes = startHour*60;
    }
    
  }*/

  if (scenari[scenarioAttivo].ciclo==0){
		if(startHour==0){
			startTotalMinutes=sunriseTotalMinutes;
		}else{
			startTotalMinutes = startHour*60;
		}
		endTotalMinutes = startTotalMinutes + (scenari[scenarioAttivo].timer*60);
	}else{
		startTotalMinutes=sunriseTotalMinutes;
		endTotalMinutes=sunsetTotalMinutes;
	}
/*
  // Assegna ora di fine (in minuti)
  if(scenari[scenarioAttivo].ciclo==0){  //timer programmato
    endTotalMinutes = startTotalMinutes + scenari[scenarioAttivo].timer*60; 
  }else{                                  //timer Alba/Tramonto
    endTotalMinutes=sunsetTotalMinutes;
  }
*/
	#ifdef DEBUG_TIMER
		Serial.print("⏰ Ora attuale (min): ");
		Serial.println(nowTotalMinutes);
		Serial.print("▶️ Start (min): ");
		Serial.println(startTotalMinutes);
		Serial.print("⛔ End (min): ");
		Serial.println(endTotalMinutes);
		Serial.print("Ora Attuale (min): ");
		Serial.println(nowTotalMinutes);
		Serial.print("Alba eseguita? ");
		Serial.println(albaEseguita ? "SI" : "NO");
		Serial.print("Tramonto eseguito? ");
		Serial.println(tramontoEseguito ? "SI" : "NO");
	#endif

	if (activeDHT != nullptr){
		Temperatura=activeDHT->readTemperature();	
		Umidita=activeDHT->readHumidity(); 
	}
	Luminosita=map(analogRead(PIN_PHOTO),150,4095,100,0);
  pubblicaValoriAmbientali();

  return;
}

// ---- parse boolean-ish tokens like "true"/"false"/"1"/"0" ----
static inline int parseBoolToken(const char* s) {
  if (!s || !*s) return 0;
  char c = s[0];
  if (c=='1' || c=='t' || c=='T' || c=='y' || c=='Y') return 1;
  if (c=='0' || c=='f' || c=='F' || c=='n' || c=='N') return 0;
  return atoi(s);
}


