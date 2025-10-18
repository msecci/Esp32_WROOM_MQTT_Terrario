/********** Sottoscrizioni *****************/
/********* Da client Android verso ESP32****************/
extern void restoreDisplayAfterTransition();
void sottoscrizioni(String path){
	client.subscribe(String(path+"S").c_str());   //Accende/spegne LED
	client.subscribe(String(path+"U").c_str());   //aggiornamenti
	client.subscribe(String(path+"I").c_str());  	// Id dello scenario
	client.subscribe(String(path+"R").c_str());		// Riavvio (riavvio=1)
  client.subscribe(String(path+"T").c_str());		// Test luminosità led
	client.subscribe(String(path+"Q").c_str());		// Reinvio VS (generale)
	client.subscribe(String(path+"A").c_str());		// Parametri setup cicli alba/tramonto
	client.subscribe(String(path+"B").c_str());		// Timer attivo/disattivo
	client.subscribe(String(path+"C").c_str());		// Durata accensione manuale
	client.subscribe(String(path+"D").c_str());		// Display sempre acceso
	client.subscribe(String(path+"Z").c_str());		// Parametri setup cicli e timer
	client.subscribe(String(path+"Y").c_str());		// Reinvio VS (scenario)
	client.subscribe(String(path+"N").c_str());		// Invio email
	client.subscribe(String(path+"X").c_str());		// Invio dati scenario custom
	#ifdef DEBUG_MQTT
		Serial.println("Sottoscrizione: "+path);
	#endif
	return;
}

void azionamento(String messaggio, String topic){
	#ifdef DEBUG_MQTT
		Serial.println("Messaggio: ");
		Serial.println(messaggio);
		Serial.print("topic: ");
		Serial.println(topic);
		Serial.print("topic len: ");
		Serial.println((topic.length()));
		Serial.print("topic sub: ");
		Serial.println((topic.substring(topic.length()-1)));
	#endif
	int result;
	const char* Cstr=messaggio.c_str();

  switch (topic.charAt(topic.length()-1)){

		case 'X': {// Parametri setup scenario custom
			int w, r, b, lum, cyc, tim, umiH, umiL;
			int n = sscanf(Cstr, " %d , %d , %d , %d , %d , %d , %d, %d",
										&w, &r, &b, &lum, &cyc, &tim, &umiL, &umiH);
			if (n == 8) {
				//const int SCENARIO_COUNT = sizeof(scenari) / sizeof(scenari[0]);
				//if (id < 0) id = 0;
				//if (id >= SCENARIO_COUNT) id = SCENARIO_COUNT - 1;
				scenari[3].ledWhite         = w;
				scenari[3].ledRed           = r;
				scenari[3].ledBlue          = b;
				scenari[3].luminosita       = lum;
				scenari[3].ciclo            = cyc;
				scenari[3].timer            = tim;
				scenari[3].umidita_L				= umiL;
				scenari[3].umidita_H				= umiH;
				pubblicaValoriScenario(scenarioAttivo,true);
			} else {
				#ifdef DEBUG_MQTT
					Serial.println(String("Parse X failed, got ") + String(n) + " fields: " + String(Cstr));
				#endif
			}
		}break;

		case 'Z': { // Parametri setup cicli e timer	{}
			
			int a, t, sh, td, ta, cc, ao;
			int n = sscanf(Cstr, " %d , %d , %d , %d , %d , %d , %d ",
                              &a, &t, &sh, &ta, &td, &cc, &ao);
			if (n == 7) {
				durataAlba      = a;
				durataTramonto  = t;
				startHour       = sh;
				timerAttivo     = ta;
				timerDelay      = td;
				cicloContinuo   = cc;
				displayAlwaysOn	= ao;
				sunLight.setTransitionDuration(durataAlba*60, durataTramonto*60);
				pubblicaValoriCicli(true);
			} else {
				#ifdef DEBUG_MQTT
					Serial.println(String("Parse Z failed, got ") + n + " fields: " + Cstr);
				#endif
			}
		}break;

		case 'A': { // Parametri setup cicli 
			
			int a, t, sh, td, ta, cc, ao;
			int n = sscanf(Cstr, " %d , %d , %d , %d",
                              &a, &t, &sh, &cc);
			if (n == 4) {
				durataAlba      = a;
				durataTramonto  = t;
				startHour       = sh;
				cicloContinuo   = cc;
				sunLight.setTransitionDuration(durataAlba*60, durataTramonto*60);
				pubblicaValoriCicli(true);
			} else {
				#ifdef DEBUG_MQTT
					Serial.println(String("Parse Z failed, got ") + n + " fields: " + Cstr);
				#endif
			}
		}break;

		case 'B':
			// aggiorna timer
			timerAttivo=messaggio.toInt();
			pubblicaValoriCicli(true);
			pubblicaValoriAmbientali();
			stato=0;
			displayStato(stato);
		break;

		case 'C':
			// durata accesione manuale
			timerDelay=messaggio.toInt();
			pubblicaValoriCicli(true);
			stato=0;
			displayStato(stato);
		break;

		case 'D':
			// display always On
			displayAlwaysOn=messaggio.toInt();
			pubblicaValoriCicli(true);
			stato=0;
			displayStato(stato);
		break;
		
	case 'N': {
    // Cstr deve essere una C-string null-terminata con formato: email,numero
    char email_buf[128];
    int c;

    // Legge fino alla virgola (max 127 char) e poi un intero, spazi tollerati
    int n = sscanf(Cstr, " %127[^,] , %d", email_buf, &c);
    if (n == 2) {
      disclaimer = c;                // 0 o 1
      email = String(email_buf);     // se 'email' è una String globale/di classe
    } else {
      // parsing fallito: gestisci se serve
      Serial.println("Parsing N fallito");
    }
    pubblicaValoriCicli(true);
    stato = 0;
    #ifdef DEBUG_MQTT
      Serial.print("Topic N: ");
      Serial.println(email);
    #endif
    displayStato(stato);
	}break;

		
		case 'U':
			if(messaggio.toInt()==1){
					checkForUpdate();
					if(connessoMqtt){client.publish(String(mqtt_nodeID+String(in_topic)+"U").c_str(), String(0).c_str(), true);}		//controllo aggiornamenti
			}		
		break;

		case 'S':
			//accende/spegne tutti i led
			comandoManualeLed(messaggio.toInt());
			#ifdef DEBUG_MQTT
				Serial.print("timerOn: ");
				Serial.println(timerOn);
				Serial.print("ledOn: ");
				Serial.println(ledOn);
			#endif
		break; 

		case 'I':
			//id dello scenario
			scenarioAttivo=messaggio.toInt();
			pubblicaValoriScenario(scenarioAttivo,true);
			aggiornaLuminositaScenarioAttivo();
			stato=0;
			displayStato(stato);
		break;

		case 'R': //Riavvio
		  if(messaggio.toInt()==1){
				Serial.print("topic+messaggio: ");
				Serial.println(messaggio);
				#ifdef DEBUG_MQTT
					Serial.println("Riavvio ...");
				#endif
				if(connessoMqtt){client.publish(String(mqtt_nodeID+String(out_topic)+"R").c_str(), String(2).c_str(), true);}		
				delay(5000);
				ESP.restart();
			}
		break;

		case 'Q': // Reinvia di dati di pubblicaValoriCicli senza salvare
		  if(messaggio.toInt()==1){
				pubblicaValoriCicli(false);
			}
		break;

		case 'Y': // Reinvia di dati di pubblicaValoriScenari per lo scenario custom (3) senza salvare
		  if(messaggio.toInt()==1){
				pubblicaValoriScenario(3,false);
			}
		break;

		case 'T': { // test luminosità led
    	// il messaggio contiene i valori di 3 led separati da virgola
      // se i led sono accesi viene salvato il valore attuale e sostituito da quello in test per 5 secondi
      // se sono spenti si accendono con il valore in test per 5 secondi (attenzione timer bloccanti)
      int test_W, test_R, test_B;
      int prev_W, prev_R, prev_B;
      bool wasOn = ledOn;
			tft.fillScreen(ST77XX_YELLOW);
  		drawText(0,0,4,ST77XX_BLACK,"Test LED");
      result = sscanf(Cstr,"%d,%d,%d",&test_W,&test_R,&test_B);
      if (result == 3) {
        sunLight.getCurrentPercentages(prev_W, prev_R, prev_B); // leggi valori correnti
        if (wasOn) {
          sunLight.turnOff();
          ledOn = false;
          if (stato == 8) {
            restoreDisplayAfterTransition();
          }
        }
        sunLight.turnOn(test_W, test_R, test_B); // prova valori proposti
        delay(5000);
        if (wasOn) {
          sunLight.turnOn(prev_W, prev_R, prev_B); // ripristina valori precedenti
          ledOn = true;
        } else {
          sunLight.turnOff();
          ledOn = false;
          if (stato == 8) {
            restoreDisplayAfterTransition();
          }
        }
      }
			displayStato(0);

    }break;



	}
}


////Gestione coda messaggi MQTT /////////////
// ======= CODA MESSAGGI MQTT =======

const int QSIZE = 8;
volatile int qHead = 0, qTail = 0;
Msg queueMsg[QSIZE];

void enqueueMsg(const char* topic, const char* buf, unsigned len) {
  int next = (qHead + 1) % QSIZE;
  if (next == qTail) {
    Serial.println("⚠️ Coda MQTT piena, messaggio scartato");
    return;
  }
  queueMsg[qHead].topic = String(topic);
  queueMsg[qHead].payload = String(buf).substring(0, len);
  qHead = next;
}
/////////////////////////////////////////////

void connectServer() {
  
  int numRipetizioni = NUM_RIPETIZIONI; // tentativi di riconnessione
  // Imposta server e parametri MQTT (PRIMA della connect)
  //client.setServer(mqtt_server, convertCtoI(mqtt_port));
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);    // <-- FONDAMENTALE
  client.setKeepAlive(45);         // 30–60s è ok
  client.setSocketTimeout(10);     // evita blocchi lunghi su socket
  client.setBufferSize(1024);      // aumenta se i payload >256B

  #ifdef DEBUG_MQTT
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.println(mqtt_port);
  #endif

  if (client.connected()) {
    #ifdef DEBUG_MQTT
      Serial.println("Client MQTT già connesso");
    #endif
    connessoMqtt = true;
    return;
  }


  while (!client.connected() && numRipetizioni > 0) {
    numRipetizioni--;

    #ifdef DEBUG_MQTT
      Serial.println();
      Serial.print("Connecting to MQTT...:");
      Serial.print(numRipetizioni);
      Serial.print("   mqtt_user:  "); Serial.print(mqtt_user);
      Serial.print("   mqtt_pass:"); Serial.println(mqtt_pass);
    #endif

    if (client.connect(("ESP-"+mqtt_nodeID).c_str(), mqtt_user, mqtt_pass)) {  //id client, username, password
      connessoMqtt = true;
			printDebug("Attivato collegamento MQTT", MQTT_DEBUG, SERIAL_DEBUG, false);

      // backoff leggero e non bloccante
      for (int i = 0; i < 20; ++i) { delay(50); yield(); } // ~1000ms totali
      sottoscrizioni(mqtt_nodeID+String(in_topic));   
      client.publish(String(mqtt_nodeID+String(in_topic)+"R").c_str(), String(0).c_str(), true);		//azzera riavvio
      return; // connesso
    } else {
        #ifdef DEBUG_MQTT
          Serial.print("failed with state: ");
          Serial.print(client.state());
          Serial.print("  nodo ID:  ");
          Serial.print(mqtt_nodeID);
          Serial.print("  USER: "); 
          Serial.print(mqtt_user);
          Serial.print("   PASSWORD: "); 
          Serial.println(mqtt_pass);
        #endif
      connessoMqtt = false;

      // backoff leggero e non bloccante
      for (int i = 0; i < 10; ++i) { delay(50); yield(); } // ~500ms totali
    }
  }
  if(!connessoMqtt){
		printDebug("Server MQTT non disponibile. Solo funzioni locali", false, SERIAL_DEBUG, false);
  }
  return;
}

/*________________________ START CALLBACK  _____________________________*/ 
void callback(char* topic, byte* payload, unsigned int length) {

  enqueueMsg(topic, (const char*)payload, length);
  
	#ifdef DEBUG_MQTT
		Serial.print("Message arrived [");
		Serial.print(topic);
		Serial.print("] ");
	#endif
}

/*callback notifying us of the need to save config*/
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void pubblicaValoriScenario(int scenario, bool tipo){
	String valori;
	// IDscenario,   led bianco,   led rosso,   led blu,  soglia luminosita,    durata timer in ore ,     tipo timer,    minuto di avvio,     minuto di fine,   soglia umidità

	Serial.print("****************** Scenario Attivo dentro pubblicaValoriScenario: ");
	Serial.println(scenario);

	if (scenari[scenario].ciclo==0){
		if(startHour==0){
			startTotalMinutes=sunriseTotalMinutes;
		}else{
			startTotalMinutes = startHour*60;
		}
		endTotalMinutes = startTotalMinutes + (scenari[scenario].timer*60);
	}else{
		startTotalMinutes=sunriseTotalMinutes;
		endTotalMinutes=sunsetTotalMinutes;
	}

	valori=String(scenario)+","+scenari[scenario].ledWhite+","+scenari[scenario].ledRed+","+scenari[scenario].ledBlue+","+scenari[scenario].luminosita+","+scenari[scenario].ciclo+","+scenari[scenario].timer+","+startTotalMinutes+","+endTotalMinutes+","+scenari[scenario].umidita_L+","+scenari[scenario].umidita_H;
	if(connessoMqtt){
		client.publish(String(mqtt_nodeID+String(out_topic)+"VS").c_str(), 	valori.c_str(), true);	
	}
	if(tipo){
		if(scenario==3){
			preferences.begin("Dati", false);    
				preferences.putInt("dutycycle_W",   scenari[3].ledWhite);
				preferences.putInt("dutycycle_R",   scenari[3].ledRed);
				preferences.putInt("dutycycle_B",   scenari[3].ledBlue);
				preferences.putInt("lumPhoto",      scenari[3].luminosita);
				preferences.putInt("timeDuration",  scenari[3].timer);
				preferences.putInt("tipoTimer",     scenari[3].ciclo);
				preferences.putInt("umidita_H",     scenari[3].umidita_H);
				preferences.putInt("timerAttivo",   timerAttivo);
				preferences.putInt("umidita_L",     scenari[3].umidita_L);
			preferences.end();
		}else{
			preferences.begin("Dati", false);    
				preferences.putInt("scenarioAttivo",scenarioAttivo);
			preferences.end();
		}
	}
	

	#ifdef DEBUG_MQTT
		Serial.print("Valori Scenari: ");
		Serial.println(valori);
	#endif

	return;
}

void pubblicaValoriAmbientali(){
	String valori;
	// LedOn,    Temperatura,   Umidità,   Luminosità,   Minuti attuali

	valori=String(ledOn)+","+String(Temperatura)+","+String(Umidita)+","+String(Luminosita)+","+String(nowTotalMinutes);
	if(connessoMqtt){
		client.publish(String(mqtt_nodeID+String(out_topic)+"VT").c_str(), 	valori.c_str(), true);	
	}

	#ifdef DEBUG_MQTT
		Serial.print("Valori Ambientali: ");
		Serial.println(valori);
	#endif

	return;
}

void pubblicaValoriCicli(bool tipo){
	String valori;
	// Durata Alba,   Durata Tramonto,   Ora Avvio,   Stato Timer,    Ritardo Timer Manuale,   Ciclo Continuo attivato, Display sempre acceso, email,  disclaimer

	valori=String(durataAlba)+","+String(durataTramonto)+","+String(startHour)+","+(timerAttivo ? "1" : "0")
					+","+String(timerDelay)+","+String(cicloContinuo)+","+String(displayAlwaysOn)+","+email+","+String(disclaimer);
					
	if(connessoMqtt){client.publish(String(mqtt_nodeID+String(out_topic)+"VC").c_str(), 	valori.c_str(), true);}

	if(tipo){
		preferences.begin("Dati", false);    
	  	preferences.putInt("durataAlba",   durataAlba);
			preferences.putInt("durataTramonto", durataTramonto);
			preferences.putInt("startHour",startHour);
			preferences.putInt("timerAttivo",   timerAttivo);
		  preferences.putInt("timerDelay", timerDelay);
			preferences.putInt("cicloContinuo", cicloContinuo);
			preferences.putInt("displayAlwaysOn", displayAlwaysOn);
			preferences.putInt("disclaimer", disclaimer);
			preferences.putString("email", email);
		preferences.end();
	}

	#ifdef DEBUG_MQTT
		Serial.print("Valori Cicli: ");
		Serial.println(valori);
	#endif

	return;
}
