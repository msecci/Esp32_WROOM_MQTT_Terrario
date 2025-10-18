//___________________SELECT____________________
void onSelectClick() {
  //reset del timer per il menu principale e accende BL
  timer.cancel(task_goMain);
  task_goMain=timer.every(TIMER_BL*60000,goMain); 
  if (digitalRead(PIN_BL)==LOW){
    digitalWrite(PIN_BL,HIGH);
    return;
  }

  #ifdef DEBUG_MENU
    Serial.print("OK Stato: ");
    Serial.print(stato);
    Serial.print("   currentIndex: ");
    Serial.println(currentIndex);
  #endif

  switch (stato){
      case 0: //Accende/spegne i Led manualmente (timer disattivo o fuori periodo)
        #ifdef DEBUG_TIMER
          Serial.print("ledOn: ");
          Serial.print(ledOn);
          Serial.print("  timerOn: ");
          Serial.println(timerOn);
        #endif
        comandoManualeLed(!ledOn);

      break;

      case 1:  //weatherScreen
      //////////////////////////////////////////////////////////////////da implementare
        stato=7; // seleziona giorno e vai previsioni
        displayStato(stato);
      break;

      case 2: //setupScreen
        stato=3;
        currentIndex=scenarioAttivo;
        oldCurrentIndex=currentIndex;
        displayStato(stato);

      break;

      case 3: //Lista scorrevole scenari
        /////////////////////////// da inserire salvataggio indice scenario attivo
        scenarioAttivo=currentIndex;
        pubblicaValoriScenario(scenarioAttivo,true);
        aggiornaLuminositaScenarioAttivo();
        tft.fillScreen(ST77XX_BLACK);
        drawText(0,40,3,ST77XX_YELLOW,"Scenario attivo:");
        drawText(0,100,3,ST77XX_BLUE,String(scenari[scenarioAttivo].nome).c_str());
        stato=0;
        displayStato(stato);
  
      break;

      case 4:  //conferma reset WiFi
        if(conferma){
          switch(tipoConferma){
            case 0: //Riavvio WiFi
              tft.fillScreen(ST77XX_BLACK);
              tft.setTextSize(2); 
              tft.setTextColor(ST77XX_YELLOW);
              tft.setCursor(10,0);  
              tft.println("...Avvio reset WiFi..."); 
              tft.println(); 
              tft.println("- Connettere WiFi:ESP32_AP"); 
              tft.println("- Aprire browser all'ind.:"); 
              tft.println(); 
              tft.println("        192.168.4.1"); 
              tft.println(); 
              tft.println("- Inserire credenz. WiFi"); 
              tft.println("- Save  & close browser");
              tft.println("- Lanciare l'applicazione.");  
              delay(1000);
              wm.resetSettings();
              software_Reset();
          break;

          case 1: //Riavvio ESP             
              tft.fillScreen(ST77XX_BLACK);
              drawText(0, 0, 3, ST77XX_RED, "...RIAVVIO...");
              delay(1000);
           		ESP.restart();
          break;
          }
        }else{
          stato=0;
          displayStato(stato);
        }
      break;

      case 5: //QRCode Nodo
        stato=0;
        displayStato(stato);
      break;

      case 6: //QRCode App
        stato=0;
        displayStato(stato);
      break;

      case 7: // weather giornaliero
        stato=1;
        displayStato(stato);
      break;

      case 9: //reset dopo allarme
        if (Temperatura < TEMP_ALERT){
          //Riattiva i timer
          task_goMain=timer.every(300000,goMain);						//ogni 5 minuti riporta al mainScreen e spegne retroilluminazione e verifica che lo stato del il timer per accendere o spegnere i led
          stato=0;
          displayStato(0);
        }else{
          printDebug(" ALLARME ancora attivo. ATTENDERE e premere <OK> per reset",MQTT_DEBUG,SERIAL_DEBUG,TFT_DEBUG);
        }
      break;
    }
    #ifdef DEBUG_MENU
      Serial.print("EXIT OK Stato: ");
      Serial.print(stato);
      Serial.print("   currentIndex: ");
      Serial.println(currentIndex);
    #endif
} 

//____________________PREV_____________________
void onPrevClick() {
  timer.cancel(task_goMain);
  task_goMain=timer.every(TIMER_BL*60000,goMain); //reset del timer per il menu principale
  if (digitalRead(PIN_BL)==LOW){
    digitalWrite(PIN_BL,HIGH);
    return;
  }

  #ifdef DEBUG_MENU
    Serial.print("PREV Stato: ");
    Serial.print(stato);
    Serial.print("   currentIndex: ");
    Serial.println(currentIndex);
  #endif
  switch (stato){
    case 0: //Main Screen
      stato=2;
      displayStato(2);
    break;

    case 1:  //weatherScreen
      {
      // --- de-seleziona quello corrente (bianco) ---
      int prevIdx = currentDay;
      String isoPrev = timeArrayJson[prevIdx].as<String>();
      String giornoPrev = TimeManager::weekdayNameFromISO(isoPrev);
      drawText(posizioneIcone[prevIdx].x_g, posizioneIcone[prevIdx].y_g, 2, ST77XX_WHITE,giornoPrev.c_str());

      // --- calcola il nuovo indice con wrap all’indietro ---
      int nextIdx = (currentDay - 1);
      if (nextIdx < 0) nextIdx = totalDays - 1;

      // --- seleziona il nuovo (rosso) ---
      String isoNext = timeArrayJson[nextIdx].as<String>();
      String giornoNext = TimeManager::weekdayNameFromISO(isoNext);
      drawText(posizioneIcone[nextIdx].x_g, posizioneIcone[nextIdx].y_g, 2, ST77XX_RED, giornoNext.c_str());

      // aggiorna la variabile di stato
      currentDay = nextIdx;
      }
    break;    
      
    case 2: //setupScreen
      stato=0;
      displayStato(stato);
    break;

    case 3: //Lista correvole scenari
      oldCurrentIndex=currentIndex;
      if(currentIndex==0){  
        currentIndex=3;
      }else{
        if (currentIndex >0) {
          currentIndex--;
        };
      }
      displayStato(stato); 
    break;
  
    case 4: //form di conferma
      conferma=false;
      drawButton(80,110,40,25,ST77XX_GRAY,ST77XX_RED,"NO");
      drawButton(200,110,40,25,ST77XX_GRAY,ST77XX_WHITE,"SI");
    break;
    case 5: //QRCode Nodo
      stato=6;
      displayStato(stato);
    break;

    case 6: //QRCode App
      stato=5;
      displayStato(stato);
    break;

    case 7:  //weatherScreen
      if (currentDay >0) {
        currentDay--;
        displayStato(stato);
      };

      if(currentDay==0){  
        currentDay=totalDays-1;
        displayStato(stato);
      }
    break;

  }
  #ifdef DEBUG_MENU
    Serial.print("EXIT PREV Stato: ");
    Serial.print(stato);
    Serial.print("   currentIndex: ");
    Serial.println(currentIndex);
  #endif
}

//____________________NEXT_____________________
void onNextClick() {
  timer.cancel(task_goMain);
  task_goMain=timer.every(TIMER_BL*60000,goMain); //reset del timer per il menu principale
  if (digitalRead(PIN_BL)==LOW){
    digitalWrite(PIN_BL,HIGH);
    return;
  }

  #ifdef DEBUG_MENU
    Serial.print("NEXT Stato: ");
    Serial.print(stato);
    Serial.print("   currentIndex: ");
    Serial.println(currentIndex);
  #endif
  switch (stato){
    case 0: //Main Screen
      stato=1; //weather menu
      displayStato(stato);
    break;

    case 1:  //lista giorni

      {
        // --- de-seleziona quello precedente (bianco) ---
      int prevIdx = currentDay;
      String isoPrev = timeArrayJson[prevIdx].as<String>();                 // "YYYY-MM-DD"
      String giornoPrev = TimeManager::weekdayNameFromISO(isoPrev);         // "Lunedì", ...
      drawText(posizioneIcone[prevIdx].x_g,posizioneIcone[prevIdx].y_g,2, ST77XX_WHITE,giornoPrev.c_str());

      // --- calcola il prossimo indice con wrap ---
      int nextIdx = (currentDay + 1);
      if (nextIdx >= totalDays) nextIdx = 0;

      // --- seleziona il nuovo (rosso) ---
      String isoNext = timeArrayJson[nextIdx].as<String>();
      String giornoNext = TimeManager::weekdayNameFromISO(isoNext);
      drawText(posizioneIcone[nextIdx].x_g,posizioneIcone[nextIdx].y_g,2, ST77XX_RED,giornoNext.c_str());

      // aggiorna la variabile di stato
      currentDay = nextIdx;
    break;
      }
    case 2: //setupScreen
      stato=5;
      displayStato(stato);
    break;

    case 3: //Lista correvole scenari  
      oldCurrentIndex=currentIndex;
      if(currentIndex==3){  
        currentIndex=0;
      }else{
        if (currentIndex<3) {
          currentIndex++;
        };
      }
      displayStato(stato); 
    break;

    case 4: //form di conferma
      conferma=true;
      drawButton(80,110,40,25,ST77XX_GRAY,ST77XX_WHITE,"NO");
      drawButton(200,110,40,25,ST77XX_GRAY,ST77XX_RED,"SI");
    break;

    case 5: //QRCode Nodo
      stato=6;
      displayStato(stato);
    break;

    case 6: //QRCode App
      stato=5;
      displayStato(stato);
    break;

    case 7:  //weatherScreen
      if (currentDay < totalDays) {
        currentDay++;
        displayStato(stato);
      };

      if(currentDay==(totalDays-1)){  
        currentDay=0;
        displayStato(stato);
      }
    break;
  }
  
  #ifdef DEBUG_MENU
    Serial.print("EXIT NEXT Stato: ");
    Serial.print(stato);
    Serial.print("   currentIndex: ");
    Serial.println(currentIndex);
  #endif
}

/////////////////SELECT///////////////////
void onSelectLong(){
  switch (stato){
    case 8: //disattiva ciclo contionuo
      cicloContinuo=false;
      saveParameters(false);
    break;

    case 3: // switch ciclo attivo/spento
      cicloContinuo=!cicloContinuo;
      if(cicloContinuo){
        drawText(280,140,3,ST77XX_GREEN,"CL");    //ciclo continuo attivo
      }else{
        drawText(280,140,3,ST77XX_RED,"CL");  //ciclo continuo spento
      }
      saveParameters(false);
      delay(300);
      stato=0;
      displayStato(stato);
    break;
  }
}

  //////////PREV////////////////
void onPrevLong(){
  switch (stato){
    case 0:  //Riavvio dell'ESP
      tipoConferma=1;
      formConferma("ATTENZIONE","Riavvio?");
    break;
    case 1: // da lista giorni vai a main
      stato=0;
      displayStato(stato);
    break;

    case 2: // da setup modifica vai a setup
      stato=0;
      displayStato(stato);
    break;

    case 3: // da setup modifica vai a setup
      stato=2;
      displayStato(stato);
    break;

    case 5: // da setup modifica vai a setup
      stato=0;
      displayStato(stato);
    break;

    case 6: // da setup modifica vai a setup
      stato=0;
      displayStato(stato);
    break;

    case 7: // da setup modifica vai a setup
      stato=0;
      displayStato(stato);
    break;
  }
}


////////////////NEXT//////////////////////
void onNextLong(){
  switch (stato){
    case 0: //Main Screen reset WIFI
      tipoConferma=0;
      formConferma("ATTENZIONE","Reset del WiFi?");
    break;
  
    case 3: // switch Timer attivo/spento
      timerAttivo=!timerAttivo;
      if(timerAttivo){
        drawText(227,140,3,ST77XX_GREEN,"TM");    //timer attivo
      }else{
        drawText(227,140,3,ST77XX_RED,"TM");  //timer spento
      }
      saveParameters(false);
      printDebug(" TM pressione lunga",false,SERIAL_DEBUG,TFT_DEBUG);
      //delay(300);
      stato=3;
      //displayStato(stato);
    break;
    
  }
}

