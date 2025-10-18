extern void restoreDisplayAfterTransition();
void comandoManualeLed(int messaggio) {
  // Consenti controllo manuale solo se il timer è disattivato o fuori periodo
  if (timerAttivo && timerOn) {
    #ifdef DEBUG_TIMER
      Serial.println("[manual] Ignorato: timer nel periodo attivo");
    #endif
    return;
  }

  switch (messaggio) {
    case 0:   // comando spegnimento manuale
      if (ledOn) {
        ledOn = false;
        sunLight.turnOff();
        if (stato == 8) {
          restoreDisplayAfterTransition();
        }
        timer.cancel(task_manualLed);
        timerOn = false;
        albaEseguita = false;
        tramontoEseguito = false;
      }
      break;

    case 1:   // comando accensione manuale con autospegnimento
      if (!ledOn) {
        ledOn = true;
        auto &sc = scenari[scenarioAttivo];
        sunLight.turnOn(sc.ledWhite, sc.ledRed, sc.ledBlue);
        timer.cancel(task_manualLed);
        task_manualLed = timer.in(timerDelay * 60000UL, spegniLedTemporizzato); // auto-off (minuti → ms)
      }
      break;
  }

    //client.publish(String(mqtt_nodeID + String(out_topic) + "FS").c_str(), ledOn ? "1" : "0", true);
    pubblicaValoriAmbientali();

}


bool spegniLedTemporizzato(void*){
  ledOn=false;
  sunLight.turnOff();
  if (stato == 8) {
    restoreDisplayAfterTransition();
  }
  pubblicaValoriAmbientali();
  return false;
}

bool pubblica(void*){
  sensorsRead();
  //verificaTimer();
  if(stato==0){updateTime();}
  if (Temperatura > TEMP_ALERT){
    sunLight.turnOff();
    ledOn=false;
    stato=9; //allarme
    timer.cancel(task_goMain); //Disattiva timer
    printDebug(" ATTENZIONE!! Rilevata sovratemperatura. <OK> per reset",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
  }
  return true;
}

/*bool spegniBL(void*){
  if (!displayAlwaysOn) {
    digitalWrite(PIN_BL,LOW); 
  }
  return true;
}*/

bool goMain(void*){
  currentIndex=0;
  stato=0;
  if (displayAlwaysOn) {
    digitalWrite(PIN_BL,HIGH); 
  } else {
    digitalWrite(PIN_BL, LOW); 
  } 
  displayStato(0); 
  return true;
}

bool aggiornaMeteo(void*){
  getDailyWeatherForecast();
  //timeClient.setTimeOffset(utcOffset);
  stato=0;
  displayStato(0);
  return true;
}

// --- Helper: finestra oraria [start, end) con gestione mezzanotte ---
inline bool isInWindow(int nowMin, int startMin, int endMin) {
  if (startMin == endMin) return false;                // finestra nulla = disattiva
  if (startMin <  endMin) return (nowMin >= startMin && nowMin < endMin);
  // finestra che attraversa le 24:00 (es. 22:00→06:00)
  return (nowMin >= startMin) || (nowMin < endMin);
}

// --- (Opzionale) Helper per evitare out-of-bounds su scenari ---
#ifndef SCENARIO_COUNT
#define SCENARIO_COUNT 4
#endif
inline int clampScenario(int idx) {
  if (idx < 0) return 0;
  if (idx >= SCENARIO_COUNT) return SCENARIO_COUNT - 1;
  return idx;
}

// Usa le tue variabili globali:
//  - bool timerAttivo, timerOn, albaEseguita, tramontoEseguito, timerDisattivoEseguito, ledOn;
//  - int scenarioAttivo;
//  - struct { int ledWhite, ledRed, ledBlue; ... } scenari[];
//  - oggetto sunLight con turnOn(), startSunrise(), startSunset();
//  - printDebug(...), comandoManualeLed(int).

void gestisciTimerLuci(int nowTotalMinutes, int startTotalMinutes, int endTotalMinutes) {
  const bool inWindow = isInWindow(nowTotalMinutes, startTotalMinutes, endTotalMinutes);

  if (timerAttivo) {
    // riattivo timer → sblocco guardia di spegnimento
    timerDisattivoEseguito = false;

    if (inWindow) {
      // dentro finestra → avvia ALBA una sola volta quando entri
        if (!timerOn && !albaEseguita) {
          printDebug("Azione ALBA eseguita!", MQTT_DEBUG, SERIAL_DEBUG, false);
          auto &sc = scenari[clampScenario(scenarioAttivo)];
          sunLight.turnOn(sc.ledWhite, sc.ledRed, sc.ledBlue);  // preset livelli target
          sunLight.startSunrise();                               // transizione alba
          timer.cancel(task_manualLed);                          // annulla eventuale auto-off manuale
          isSunriseMode = true;
          stato = 8;
          displayStato(8);
          ledOn = true;
          albaEseguita = true;
          tramontoEseguito = false;
          timerOn = true;                                       // ora siamo "dentro" il ciclo diurno
        }
      // già dentro (timerOn==true) → non fare altro (no retrigger)
    } else {
        // fuori finestra → avvia TRAMONTO una sola volta quando esci
        if (timerOn && !tramontoEseguito) {
          printDebug("Azione TRAMONTO eseguita!", MQTT_DEBUG, SERIAL_DEBUG, false);
          sunLight.startSunset();           // transizione tramonto
          isSunriseMode = false;
          stato = 8;
          displayStato(8);
          ledOn = false;
          tramontoEseguito = true;
          albaEseguita = false;
          timerOn = false;                  // ora siamo "fuori" dal ciclo diurno
        }
      // già fuori (timerOn==false) → non fare altro
    }

  } else {
    // timer disabilitato → spegni e azzera stato, così al riattivo riparte ALBA se sei nella finestra
    if (!timerDisattivoEseguito) {
      printDebug("Disattivato Timer!", MQTT_DEBUG, SERIAL_DEBUG, false);
      comandoManualeLed(0);               // spegne i LED
      ledOn = false;
      timerOn = false;                    // **importante**: permette ALBA al riattivo
      albaEseguita = false;
      tramontoEseguito = false;
      timerDisattivoEseguito = true;      // evita ripetizioni
    }
  }
}
