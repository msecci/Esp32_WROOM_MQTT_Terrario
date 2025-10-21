// coordinate della barra di avanzamento transizione
const int TRANS_BAR_X = 20;
const int TRANS_BAR_Y = 80;
const int TRANS_BAR_W = 280;
const int TRANS_BAR_H = 20;

// ultimo valore disegnato per la barra di transizione
static int lastFillWidth = 0;

void mainScreen() {

    currentIndex=0;
    tft.fillScreen(TFT_BLACK);
    drawText(0,0,4,TFT_WHITE,"Principale");
    tft.fillScreen(TFT_BLACK);
    updateTime();

    // Giorno
    tft.setTextSize(2);
    if (TimeManager::dayOfWeekNum() == 0) { // Domenica
      drawText(10,44,2,TFT_RED, TimeManager::dayOfWeekIT().c_str());
    } else {
      drawText(10,44,2,TFT_GREEN, TimeManager::dayOfWeekIT().c_str());
    }
 
    // Data
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    /*time_t epochTime = timeClient.getEpochTime();  // Ottieni epoch time
    struct tm *ptm = gmtime((time_t *)&epochTime);
    int giorno = ptm->tm_mday;
    int mese = ptm->tm_mon + 1;  // Mesi da 0 a 11, quindi aggiungiamo 1
    int anno = ptm->tm_year + 1900;  // Anno parte da 1900
    String data=String(giorno)+"-"+String(mese)+"-"+String(anno);
    //drawText(5,68,2,TFT_WHITE,data.c_str());*/
    drawText(5,68,2,TFT_WHITE,TimeManager::dateDDMMYYYY().c_str());

    // Scenario
    tft.setTextColor(TFT_WHITE);
    drawText(40,145,2,TFT_WHITE,(scenari[scenarioAttivo].nome).c_str());
    tft.pushImage(2, 138, 32, 32, scenari[scenarioAttivo].icona);

    // Tempo residuo
    int minutiRes;
    if(nowTotalMinutes<startTotalMinutes){
        minutiRes=endTotalMinutes-startTotalMinutes;
    }else{
        minutiRes=endTotalMinutes-nowTotalMinutes;
    }
  
    int oreRes=(int)(minutiRes/60);
    int minRes=minutiRes-(oreRes*60);
    drawText(34, 110, 2, TFT_WHITE,
         (String(oreRes < 0 ? 0 : oreRes) + ":" +
          (minRes < 10 ? "0" : "") + String(minRes < 0 ? 0 : minRes)).c_str());
    //drawText(34,110,2,TFT_WHITE,(String(oreRes<0 ? 0 : oreRes)+":"+String(minRes<0 ? 0 : minRes)).c_str());
    if(scenari[scenarioAttivo].ciclo==0){
      tft.pushImage(0, 92, 32, 32, icoResiduo);
    }else{
      tft.pushImage(0, 92, 32, 32, icoAlba);
    }

    // Temperatura
    drawText(195,10,3,TFT_WHITE,"^C");
    tft.pushImage(155, 5, 32, 32, icoTermometro);
    // Umidità
    tft.pushImage(92, 90, 32, 32, icoUmidita);

    // Current. T.
    drawText(230,145,3,TFT_WHITE,String(temperatureCurrent).c_str());

    // Min. T.
    drawText(249,72,2,TFT_CYAN,(String(temperatureMin[currentDay].as<float>())).c_str());

    // Max T.
    drawText(249,100,2,TFT_ORANGE,(String(temperatureMax[currentDay].as<float>())).c_str());

    //Icona Meteo
    drawMeteo(256,0,weatherConvert(currentDay).icona,64,64);    

    //orizzontale y1
    tft.drawLine(0, 40, 130, 40, TFT_WHITE); 

    //orizzontale y2
    tft.drawLine(0, 85, 240, 85, TFT_WHITE); 

    //orizzontale y3
    tft.drawLine(0, 130, 320, 130, TFT_WHITE); 

    //orizzontale y4
    tft.drawLine(240, 64, 320, 64, TFT_WHITE); 

    //verticale x1
    tft.drawLine(92, 85, 92, 130, TFT_WHITE); 

    //verticale x2
    tft.drawLine(130, 0, 130, 85, TFT_WHITE); 
     
    //verticale x3
    tft.drawLine(240, 0, 240, 130, TFT_WHITE); 

    //verticale x5
    tft.drawLine(220, 130, 220, 170, TFT_WHITE); 
}

void setupScreen() {

  tft.fillScreen(TFT_BLACK);
  drawText(0,0,4,TFT_WHITE,"Setup");
  tft.fillScreen(TFT_BLACK);
  
  for (int i=0;i<4;i++){
    drawText(scenari[i].xText,scenari[i].yText,2,TFT_WHITE,(scenari[i].nome).c_str());
    tft.pushImage(scenari[i].xIco, scenari[i].yIco, 32, 32, scenari[i].icona);
  }
  
  // Evidenzia scenario attivo
  drawText(scenari[scenarioAttivo].xText,scenari[scenarioAttivo].yText,2,TFT_GREEN,(scenari[scenarioAttivo].nome).c_str());
  parametriScenario(scenarioAttivo);

  if(connessoMqtt){drawText(1,140,3,TFT_GREEN,String(mqtt_nodeID).c_str());
  }else{drawText(1,140,3,TFT_RED,String(mqtt_nodeID).c_str());
  }
  drawText(223,145,2,TFT_WHITE,"ver");
  drawText(263,145,2,TFT_WHITE,String(currentVersion).c_str());

  //orizzontale y1
  tft.drawLine(210, 90, 320, 90, TFT_WHITE); 
  //orizzontale y2
  tft.drawLine(0, 130, 320, 130, TFT_WHITE); 
  //verticale 1
  tft.drawLine(210, 0, 210, 130, TFT_WHITE);
  //verticale 2
  tft.drawLine(220, 131, 220, 170, TFT_WHITE);  
  //verticale 3
  tft.drawLine(270, 90, 270, 130, TFT_WHITE); 

}

void parametriScenario(int indicescenario){  // mostra i parametri dello scenario
  //cancella finestra luminosita
	 tft.fillRect(211, 91, 59, 39, TFT_BLACK); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore
	//cancella finestra timer
	 tft.fillRect(271, 91, 50, 39, TFT_BLACK); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore
	//cancella finestra slider
	 tft.fillRect(211, 0, 109, 89, TFT_BLACK); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore
  
  // Durata complessiva del timer nello scenario attivo
  drawText(280,105,2,TFT_WHITE,(String(scenari[indicescenario].timer).c_str()));
  
  // Luminosità interna dello scenario attivo
  drawText(230,105,2,TFT_WHITE,(String(scenari[indicescenario].luminosita).c_str()));

  //Slider LED
  //x,y,lenght,height,value,min,max,trackcolor,thumbcolor
  drawSlider(218,10,97,15,scenari[indicescenario].ledWhite,0,100,TFT_WHITE,TFT_WHITE);  //Led WHITE
  drawSlider(218,35,97,15,scenari[indicescenario].ledRed,0,100,TFT_RED,TFT_RED);	      //Led RED
  drawSlider(218,60,97,15,scenari[indicescenario].ledBlue,0,100,TFT_BLUE,TFT_BLUE);	    //Led BLUE
}

void weatherScreen() {
  // Indice in range
  if (currentDay < 0) currentDay = 0;
  if (currentDay >= totalDays) currentDay = totalDays - 1;

  // Data ISO dal daily di Open-Meteo (è già locale con timezone=auto)
  String date = timeArrayJson[currentDay].as<String>(); // "YYYY-MM-DD"
  String day   = date.substring(8, 10);
  String month = date.substring(5, 7);
  String year  = date.substring(0, 4);

  // Giorno della settimana affidabile via TimeManager (usa TZ + DST corretti)
  String giorno = TimeManager::weekdayNameFromISO(date);

  #ifdef DEBUG_WEATHER
    Serial.println("Previsioni per: " + date + "  ->  " + giorno);
  #endif

  tft.fillScreen(TFT_BLACK);
  // Giorno (rosso se Domenica)
  tft.setTextSize(2);
  if (giorno == "Domenica") {
    drawText(2,14,2,TFT_RED,   giorno.c_str());
  } else {
    drawText(2,14,2,TFT_GREEN, giorno.c_str());
  }

  // Data "DD-MM-YYYY"
  drawText(118,14,2,TFT_WHITE, (String(day + "-" + month + "-" + year)).c_str());

  // Meteo del giorno selezionato
  drawText(1,135,2,TFT_WHITE, (weatherConvert(currentDay).decoted).c_str());
  drawMeteo(256,0, weatherConvert(currentDay).icona, 64,64);

  // Alba/Tramonto: stringhe locali tipo "YYYY-MM-DDTHH:MM"
  String sunriseHHMM = sunrise[currentDay].as<String>().substring(11,16);
  String sunsetHHMM  = sunset[currentDay].as<String>().substring(11,16);
  drawText(40,93,2, TFT_WHITE, sunriseHHMM.c_str());
  tft.pushImage(5, 90, 32, 32, icoAlba);
  drawText(165,95,2, TFT_WHITE, sunsetHHMM.c_str());
  tft.pushImage(130, 90, 32, 32, icoTramonto);

  // Precipitazioni / Temperature
  drawText(120,50,2, TFT_WHITE, (String(precipitationSum[currentDay].as<float>()) + " mm").c_str());
  tft.pushImage(5, 45, 32, 32, icoPrecipitazioni);
  drawText(245,72, 2, TFT_CYAN,   String(temperatureMin[currentDay].as<float>()).c_str());
  drawText(245,100,2, TFT_ORANGE, String(temperatureMax[currentDay].as<float>()).c_str());

  // linee
  tft.drawLine(0, 40, 240, 40, TFT_WHITE);
  tft.drawLine(240, 65, 320, 65, TFT_WHITE);
  tft.drawLine(0, 80, 240, 80, TFT_WHITE);
  tft.drawLine(0, 125, 320, 125, TFT_WHITE);
  tft.drawLine(120, 80, 120, 125, TFT_WHITE);
  tft.drawLine(240, 0, 240, 65,  TFT_WHITE);
  tft.drawLine(240, 0, 240, 125, TFT_WHITE);
}


void transitionScreen(bool isAlba){
  // mantieni retroilluminazione attiva durante la transizione
  digitalWrite(PIN_BL, HIGH);
  // disattiva il timer che spegne il display
  timer.cancel(task_goMain);
  // resetta la larghezza precedente della barra
  lastFillWidth = 0;
  tft.fillScreen(TFT_BLACK);
  uint16_t color = isAlba ? TFT_YELLOW : TFT_ORANGE;
  drawText(0,20,4,color, isAlba ? "Alba" : "Tramonto");
  tft.drawRect(TRANS_BAR_X, TRANS_BAR_Y, TRANS_BAR_W, TRANS_BAR_H, TFT_WHITE);
  tft.fillRect(TRANS_BAR_X + 1, TRANS_BAR_Y + 1, TRANS_BAR_W - 2, TRANS_BAR_H - 2, TFT_BLACK);
}

void displayStato(int screen){
  switch(screen){
    case 0:
      mainScreen();
    break;

    case 1:
      //weatherScreen();
      weatherList(currentDay);
    break;

    case 2:
      setupScreen();
    break;

    case 3: //menu Setup con evidenziato scenario selezionato

      if(oldCurrentIndex!=currentIndex){  //camcella evidenziazione precedente
        tft.setTextColor(TFT_WHITE);
        tft.fillRect(scenari[oldCurrentIndex].xText,scenari[oldCurrentIndex].yText, 175, 15, TFT_BLACK);
        drawText(scenari[oldCurrentIndex].xText,scenari[oldCurrentIndex].yText,2,TFT_WHITE,(scenari[oldCurrentIndex].nome).c_str());
      }
      //sfondo evidenziato
      tft.fillRect(scenari[currentIndex].xText,scenari[currentIndex].yText, 175, 15, TFT_WHITE);

      //scrivi testo su evidenziato
      if(scenarioAttivo!=currentIndex){
        drawText(scenari[currentIndex].xText,scenari[currentIndex].yText,2,TFT_BLACK,(scenari[currentIndex].nome).c_str());
      }

      // Evidenzia scenario attivo
      drawText(scenari[scenarioAttivo].xText,scenari[scenarioAttivo].yText,2,TFT_GREEN,(scenari[scenarioAttivo].nome).c_str());

      //evidenzia i parametri dello scenario selezionato
      parametriScenario(currentIndex);

      //Cancella ver e predispone per Timer attivo e ciclo continuo
      tft.fillRect(221,131,100,39,TFT_BLACK);
      //verticale 
      tft.drawLine(270, 131, 270, 170, TFT_WHITE); 
      // Legge timer attivo e ciclo continuo
      if(timerAttivo){
        drawText(227,140,3,TFT_GREEN,"TM"); //timer attivo
      }else{
        drawText(227,140,3,TFT_RED,"TM");  //timer spento
      }
      if(cicloContinuo){
        drawText(280,140,3,TFT_GREEN,"CL"); //timer attivo
      }else{
        drawText(280,140,3,TFT_RED,"CL");  //timer spento
      }    
    break;

    case 4:
      //Riscrive timer e luminosità in bianco
      tft.fillRect(211, 91, 59, 39, TFT_BLACK); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore
	    tft.fillRect(271, 91, 50, 39, TFT_BLACK); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore

      drawText(280,105,3,TFT_WHITE,(String(scenari[scenarioAttivo].timer)+"h").c_str());
      drawText(230,105,3,TFT_WHITE,String(scenari[scenarioAttivo].luminosita).c_str());

    break;

    case 5: //QRCode ID Nodo
      //const char* txtConv=mqtt_nodeID.c_str();
      drawQRCode(mqtt_user, 150, 10, 5,"ID Nodo");  // URL, X, Y, scala
    break;

    case 6: //QRCode app.apk
      drawQRCode("http://msecci.freeddns.org:63451/Terrario/app.apk", 150, 10, 5,"App");  // URL, X, Y, scala
    break;

    case 7:
      weatherScreen();
    break;
    case 8:
      transitionScreen(isSunriseMode);
    break;
  }

  return;
}

void updateTransitionBar(float progress){
  int targetWidth = (int)((TRANS_BAR_W - 2) * progress);
  uint16_t color = isSunriseMode ? TFT_YELLOW : TFT_ORANGE;
  while (lastFillWidth < targetWidth) {
    tft.drawFastVLine(TRANS_BAR_X + 1 + lastFillWidth, TRANS_BAR_Y + 1,
                      TRANS_BAR_H - 2, color);
    lastFillWidth++;
  }
}

void updateTime(){
  //Mostra dati cancellando sole le zone necessarie
  
  // Ora
  tft.fillRect(0, 0, 130, 40, TFT_ORANGE); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore
  drawText(5,5,4,TFT_WHITE,TimeManager::timeHHMM().c_str());

  // Temperatura
  tft.fillRect(137, 47, 240-145, 85-55, TFT_BLACK); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore
  drawText(140,50,3,TFT_WHITE,String(Temperatura).c_str());

  // Umidità
  tft.fillRect(125, 92, 238-128, 130-100, TFT_BLACK); //Sfondo x,y punto di partenza; lunghezza, altezza in pixel, colore
  drawText(128,95,3,TFT_WHITE,(String(Umidita)+"%").c_str());
  return;
}

void weatherList(int currentDay_local){
  // Clamp dell’indice selezionato (se lo usi per evidenziare)
  if (currentDay_local < 0) currentDay_local = 0;
  if (currentDay_local >= totalDays) currentDay_local = totalDays - 1;

  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 159, 42, TFT_MAGENTA);
  drawText(40,10,3,TFT_YELLOW,"METEO");
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  // Oggi (0=Dom … 6=Sab), utile se vuoi colorare “oggi” diverso
  int idxToday = TimeManager::dayOfWeekNum();

  // Quanti giorni mostrare (max 7 ma non oltre quelli disponibili)
  int daysToShow = min(7, (int)timeArrayJson.size());

  for (int n = 0; n < daysToShow; n++) {
    // Prendi la data ISO dal JSON (es. "2025-09-01")
    String isoDate = timeArrayJson[n].as<String>();

    // Nome del giorno affidabile con TZ + DST (fix mktime + tm_isdst = -1)
    String giorno = TimeManager::weekdayNameFromISO(isoDate);

    #ifdef DEBUG_WEATHER
      Serial.print("n="); Serial.print(n);
      Serial.print(" iso="); Serial.print(isoDate);
      Serial.print(" dow="); Serial.println(giorno);
      Serial.print(" idxToday="); Serial.println(idxToday);
      Serial.print(" currentDay="); Serial.println(currentDay);
      Serial.print(" meteo="); Serial.println(weatherConvert(n).decoted);
      Serial.println();
    #endif

    // Colore: selezionato (currentDay_local) verde, altrimenti bianco
    // Se vuoi evidenziare “oggi” in un altro colore, aggiungi una regola
    uint16_t color = (n == currentDay_local) ? TFT_GREEN : TFT_WHITE;
    // Esempio opzionale: oggi in giallo
    // if (TimeManager::dayOfWeekNumFromISO(isoDate) == idxToday) color = TFT_YELLOW;

    drawText(posizioneIcone[n].x_g, posizioneIcone[n].y_g, 2, color, giorno.c_str());
	  drawMeteo(posizioneIcone[n].x_i,posizioneIcone[n].y_i,weatherConvert(n).icona32,32,32);
  }

  // griglia
  tft.drawLine(0, 42, 320, 42, TFT_WHITE);
  tft.drawLine(0, 84, 320, 84, TFT_WHITE);
  tft.drawLine(0, 126, 320, 126, TFT_WHITE);
  tft.drawLine(120, 42, 120, 170, TFT_WHITE);
  tft.drawLine(160, 0, 160, 170, TFT_WHITE);
  tft.drawLine(280, 0, 280, 1170, TFT_WHITE);
}

void drawQRCode(const char* text, int x, int y, int scale, String label) {
  QRCode qrcode;
  tft.fillScreen(TFT_WHITE);
  drawText(1,0,3,TFT_BLUE,label.c_str());
  uint8_t qrcodeData[qrcode_getBufferSize(3)];  // Versione 3 = 29x29

  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, text);

  for (uint8_t yy = 0; yy < qrcode.size; yy++) {
    for (uint8_t xx = 0; xx < qrcode.size; xx++) {
      int color = qrcode_getModule(&qrcode, xx, yy) ? TFT_BLACK : TFT_WHITE;
      tft.fillRect(x + xx * scale, y + yy * scale, scale, scale, color);
    }
  }
}

void drawButton(int x, int y, int w, int h, uint16_t color, uint16_t textColor, String text) {
  tft.drawRoundRect(x, y, w, h, 5, color);
	//tft.fillRect(x, y, w, h, color);
  tft.setTextColor(textColor);
  tft.setTextSize(2); // Imposta la dimensione del testo (regola a piacere)

  int16_t textWidth = tft.textWidth(text.c_str());
  int16_t textHeight = tft.fontHeight();

  // Calcola le coordinate per centrare il testo all'interno del pulsante
  int centerX = x + (w - textWidth) / 2;
  int centerY = y + (h - textHeight) / 2;

  tft.setCursor(centerX, centerY);
  tft.print(text);
}

void drawSlider(int x, int y, int length, int height, int value, int minValue, int maxValue, uint16_t trackColor, uint16_t thumbColor) {
  // Disegna la traccia dello slider (una linea orizzontale)
  int trackY = y + height / 2; // Centro verticale della traccia
  tft.drawFastHLine(x, trackY, length, trackColor);

  // Calcola la posizione dell'indicatore (thumb)
  float ratio = (float)(value - minValue) / (maxValue - minValue);
  int thumbX = x + (int)(ratio * length);
  int thumbY = y;
  int thumbSize = height; // Rende l'altezza dell'indicatore uguale all'altezza dello slider

  // Disegna l'indicatore (un rettangolo)
  tft.fillRect(thumbX - thumbSize / 2, thumbY, thumbSize, thumbSize, thumbColor);
  tft.drawRect(thumbX - thumbSize / 2, thumbY, thumbSize, thumbSize, TFT_WHITE); // Bordo opzionale
}

// Disegna l'icona passata come variabile
void drawMeteo(int x,int y,const uint16_t *bitmap, int px, int py ){
    tft.pushImage(x, y, px, py, bitmap);
	return;
}

void printDebug(String testo, bool mqtt, bool seriale, bool display){
	if (seriale){
		Serial.print("Debug: ");
		Serial.println(testo);
	}
	if(display){
		digitalWrite(PIN_BL,HIGH);
		tft.fillScreen(TFT_BLACK);
		tft.setTextColor(TFT_YELLOW, TFT_BLACK);
		tft.setCursor(0, 0);
		tft.setTextSize(2);
		tft.println(testo);
		delay(1000);
		displayStato(stato);

	}

	if(mqtt){
		if(connessoMqtt){client.publish(String(mqtt_nodeID+String(out_topic)+"DG").c_str(), testo.c_str(), true);}		// Testo di debug sull'App
	}
	return;
}

void formConferma(String titolo, String testo){
  conferma=false;
  stato=4; // per azioni nella selezione dei pulsanti
  // Disegna bordi e pulsanti
  tft.fillRect(60, 25, 200, 120, TFT_BLACK);
  tft.drawLine(60, 25, 260, 25, TFT_WHITE); 
  tft.drawLine(60, 145, 260, 145, TFT_WHITE); 
  tft.drawLine(60, 25, 60, 145, TFT_WHITE); 
  tft.drawLine(260, 25, 260, 145, TFT_WHITE); 
  // Scrivi titolo e Testo     
  drawText(72,29,3,TFT_YELLOW,titolo.c_str());
  drawText(67,70,2,TFT_YELLOW,testo.c_str());


  //Pulsanti NO,SI
  drawButton(80,110,40,25,TFT_GRAY,TFT_RED,"NO");
  drawButton(200,110,40,25,TFT_GRAY,TFT_WHITE,"SI");
  
  return;
}

void drawText(int x, int y, int dimensione, uint16_t colore, const char *testo) {
  // Imposta la dimensione del testo
  tft.setTextSize(dimensione);
  // Imposta il colore del testo
  tft.setTextColor(colore);

  // Calcola la larghezza e l'altezza del testo
  int16_t w = tft.textWidth(testo);
  int16_t h = tft.fontHeight();

  // Gestione del posizionamento
  int finalX = x;
  int finalY = y;

  if (x == 0 && y == 0) {
    // Entrambi sono 0, centra il testo su entrambi gli assi
    finalX = (screenWidth - w) / 2;
    finalY = (screenHeight - h) / 2;
  } else if (x == 0) {
    // Solo x è 0, centra il testo sull'asse X
    finalX = (screenWidth - w) / 2;
    finalY = y; // Usa la y fornita
  } else if (y == 0) {
    // Solo y è 0, centra il testo sull'asse Y
    finalX = x; // Usa la x fornita
    finalY = (screenHeight - h) / 2;
  } else {
    // Nessuno è 0, usa i valori x e y forniti
    finalX = x;
    finalY = y;
  }

    // Posiziona il cursore e stampa il testo
  tft.setCursor(finalX, finalY);
  tft.print(testo);
}