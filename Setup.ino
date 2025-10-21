void setup() {
	Serial.begin(115200);
	Serial.println("Avvio...");

	/************  Setup PIN  **************/
	pinMode(PIN_LED_BLU, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT); 
	pinMode(PIN_LED_WHITE, OUTPUT);
	pinMode(PIN_BL, OUTPUT);
	pinMode(BUTTON_PREV, INPUT_PULLUP); 
	pinMode(BUTTON_NEXT, INPUT_PULLUP); 
	pinMode(BUTTON_SELECT, INPUT_PULLUP); 
	pinMode(PIN_DHT, INPUT); 
	pinMode(PIN_PHOTO,INPUT);   //analogico
	/****************************************/

	/************ Init display *****************/  
  tft.init();
  tft.setRotation(0);  // Orientamento del display
  tft.setSwapBytes(true);
  digitalWrite(PIN_BL,HIGH); //retroilluminazione accesa

	////////////////// PULSANTI //////////////////////////
	/*imposta debounde*/
	buttonPrev.attachClick(onPrevClick);
	buttonNext.attachClick(onNextClick);
	buttonSelect.attachClick(onSelectClick);
	//long press
	buttonNext.attachLongPressStart(onNextLong);
  buttonPrev.attachLongPressStart(onPrevLong);
	buttonSelect.attachLongPressStart(onSelectLong);

	// Inizializza il WiFiManager
	WiFiManager wm;
	Serial.println("MacAddress: ");
	Serial.print(String(WiFi.macAddress()));

	// Avvia il WiFiManager e cerca di collegarsi alla rete salvata
	if (!wm.autoConnect("ESP32_AP")) {
		printDebug("Impossibile connettersi alla rete WiFi. Solo funzioni locali",false,SERIAL_DEBUG,TFT_DEBUG);
	}

	// Inizia la connessione WiFi

	// Attendi che l'ESP32 si connetta alla rete WiFi
	#ifdef DEBUG_WIFI
		Serial.println("Connessione a WiFi");
	#endif	

	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
	}
  
  
	IP = WiFi.localIP().toString();

	mqtt_nodeID=String(WiFi.macAddress());
	mqtt_nodeID.replace(":", "");
	
	mqtt_pass_str = base64encode(xorEncryptDecrypt(mqtt_nodeID + suffisso, chiave));

	// e imposta i puntatori alle buffer interne di String
	mqtt_user = mqtt_nodeID.c_str();
	mqtt_pass = mqtt_pass_str.c_str();

	#ifdef DEBUG_MQTT
		Serial.println("************************************");
		Serial.print("mqtt_nodeID: ");
		Serial.println(mqtt_nodeID);
		Serial.print("XOR: ");
		Serial.println(xorEncryptDecrypt(mqtt_nodeID + suffisso, chiave));
		Serial.print("base64: ");
		Serial.println(mqtt_pass_str);
		Serial.println("************************************");
	#endif

	// Se la connessione è riuscita, stampa l'indirizzo IP
	tft.fillScreen(TFT_BLACK);
	drawText(0, 1, 3, TFT_YELLOW, "WiFi connesso");
	drawText(0, 30, 3, TFT_YELLOW, WiFi.localIP().toString().c_str());
	drawText(0, 60, 3, TFT_GREEN, ("Ver.: " + String(currentVersion)).c_str());
	drawText(0, 150, 3, TFT_RED, String("ID:"+mqtt_nodeID).c_str());

	// Configurazione OTA
	ArduinoOTA.setHostname(OTA_HOSTNAME); // Nome del dispositivo OTA
	ArduinoOTA.setPassword(OTA_PASSWORD); // Password per l'aggiornamento OTA
	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}
		#ifdef DEBUG_OTHER
			Serial.println("Avvio aggiornamento " + type);
		#endif
		}
	)
	.onEnd([]() {
		#ifdef DEBUG_OTHER
			Serial.println("\nAggiornamento completato!");
		#endif
		}
	)
	.onProgress([](unsigned int progress, unsigned int total) {
		#ifdef DEBUG_OTHER
			Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));
		#endif
		}
	)
	.onError([](ota_error_t error) {
		#ifdef DEBUG_OTHER
			Serial.printf("Errore[%u]: ", error);
			if (error == OTA_AUTH_ERROR) printDebug("Autenticazione fallita",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
				else if (error == OTA_BEGIN_ERROR) printDebug("OTA:Avvio fallito",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
					else if (error == OTA_CONNECT_ERROR) printDebug("OTA:Connessione fallita",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
						else if (error == OTA_RECEIVE_ERROR) printDebug("OTA: Ricezione fallita",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
							else if (error == OTA_END_ERROR) printDebug("OTA: Fine fallita",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
		#endif
		}
	);

	ArduinoOTA.begin(); // Avvia il servizio OTA
	#ifdef DEBUG_OTHER
		Serial.println("Pronto per aggiornamenti OTA");
	#endif

// legge i dati salvati
	preferences.begin("Dati", true);
		scenari[3].ledWhite=preferences.getInt("dutycycle_W", 50);
		scenari[3].ledRed=preferences.getInt("dutycycle_R", 50);
		scenari[3].ledBlue=preferences.getInt("dutycycle_B", 50);
		scenari[3].timer=preferences.getInt("timeDuration", 8); // in ore
		scenari[3].luminosita=preferences.getInt("lumPhoto", 30);
		scenari[3].ciclo=preferences.getInt("tipoTimer", 0);
		scenari[3].umidita_H=preferences.getInt("umidita_H", 0);
		scenari[3].umidita_L=preferences.getInt("umidita_L", 0);
		timerAttivo=preferences.getInt("timerAttivo",1);
    cicloContinuo=preferences.getInt("cicloContinuo",0);
		scenarioAttivo=preferences.getInt("scenarioAttivo", 0);
		durataAlba=preferences.getInt("durataAlba",1);
    durataTramonto=preferences.getInt("durataTramonto",0);
		startHour=preferences.getInt("startHour", 0);
		displayAlwaysOn=preferences.getInt("displayAlwaysOn", 0);
		timerDelay=preferences.getInt("timerDelay", 5);
		email=preferences.getString("email", "");
		disclaimer=preferences.getInt("disclaimer",0);
	preferences.end();

	// Ottieni le previsioni meteo
	getDailyWeatherForecast();

	
	/************ Connessione al Server MQTT *****************/
	connectServer();

	TimeManager::begin();
	// Messaggio MQTT di riavvio
	lastResetDay=TimeManager::dateDDMMYYYY();
	if(connessoMqtt){client.publish(String(mqtt_nodeID+String(out_topic)+"DR").c_str(), (TimeManager::dateTimeISO()+" ver.:"+String(currentVersion)).c_str(), true);}		
	/******************* Inizializzazione delle pubblicazioni e delle sottoscrizioni ***********************/
	checkForUpdate();
	timerOn=false;

	selectDHT(); //seleziona il modello di DHT
	//imposta timer
	task_meteo=timer.every(4*MILLIS_PER_ORA,aggiornaMeteo);	  //ogni 4 ore aggiorna meteo, UTC
	task_sensors=timer.every(60000,pubblica);									//ogni 1 munuto pubblicazione sensori e aggiornamento display orario
	task_goMain=timer.in(TIMER_BL*60000,goMain);					    //dopo 5 minuti riporta al mainScreen, spegne retroilluminazione, verifica che lo stato del timer per accendere o spegnere i led manuali , verifica che se il timer è attivo i led siano accesi o li riaccende

	// Inizializza sistema fading
  sunLight.begin();
  sunLight.setMaxValues(MAX_WHITE, MAX_RED, MAX_BLU);
  sunLight.setTransitionDuration(durataAlba*60, durataTramonto*60);
  sunLight.setTransitionCallback(onTransitionComplete);
  stato=0;
  sensorsRead();
  pubblicaValoriScenario(scenarioAttivo,true);
  displayStato(stato);
}
