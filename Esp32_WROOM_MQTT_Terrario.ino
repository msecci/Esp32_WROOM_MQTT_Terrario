/** @file Esp32_WROOM_MQTT_Terrario.ino
 *  @brief contiele loop e dichiarazioni
 * @ board: ESP32 S3 DEV MODULE
 */
 
float currentVersion=2.29;

/*__________________________Parametri operativi (modificare secondo esigenze)____________________*/
#define MAX_WHITE        	1300  //massimo valore per LED BIANCO (attenzione all'assorbimento) max 4096
#define MAX_BLU          	1500	//massimo valore per LED BLU (attenzione all'assorbimento) max 4096
#define MAX_RED            800  //massimo valore per LED ROSSO (attenzione all'assorbimento) max 4096     
#define HYSTERESIS        	10 //valore per isteresi su luminosità
//#define URL               	"http://msecci.freeddns.org:63451/Terrario/firmware_test.bin"    //path aggiornamento firmware di test
#define URL               	"http://msecci.freeddns.org:63451/Terrario-S3/firmware.bin"    //path aggiornamento firmware
#define URLVER            	"http://msecci.freeddns.org:63451/Terrario-S3/version.txt"     //path verifica aggiornamento firmware
#define URL_APK             "http://msecci.freeddns.org:63451/Terrario-S3/app.apk"         //path aggiornamento app.apk
//#define URLVER_APK         	"http://msecci.freeddns.org:63451/Terrario/versionApk.txt"  //path verifica aggiornamento apk
#define TEMP_ALERT        	45   //temperatura rilevata da dht per far scattare l'allarme e spegnere tutti i led. Si ripristina con <OK>
#define ORA_DI_AVVIO_TIMER   8   //Ora di avvio di default del timer
#define TIMER_DELAY         60   //minuti per spegnimento timer dopo accensione con OK 
#define TIMER_BL             5   //minuti per spegnimento di BackLight
#define TIME_SUNRISE        10   //durata del ciclo alba (in minuti)
#define TIME_SUNSET          8   //durata del ciclo tramonto (in minuti)
#define NUM_RIPETIZIONI      3   // numero delle ripetizione nella connessione MQTT

/*________________________ INCLUDE LIBRARIES  _____________________________*/
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>  //Esp32
//#include <LovyanGFX.hpp>
#include <SPI.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <Preferences.h>	// Memorizzazione dati
#include <OneButton.h>
#include "icone.h"			// Array delle icone
#include "arduino-timer-custom.h"
#include "SunriseSunset.h"
#include "qrcode.h" // Assicurati che questo file sia nella tua cartella di sketch!
#include "MqttQueue.h"
#include "icone32.h"			// Array delle icone 32 px
#include "TimeManager.h"
#include <esp_sntp.h>   // usa le API non deprecate


//Timer
#define MILLIS_PER_ORA      3600000UL

/*__________________________ MQTT TOPICS   _____________________________*/
#define in_topic     "/to_terrario/"               //topic per ricezione  da parte dell'ESP (subscriber)
#define out_topic    "/from_terrario/"             //topic per trasmissione messaggio dall'ESP (publisher)
#define BUFFER_SIZE  300
#define mqtt_port   61884
#define mqtt_server  "msecci.freeddns.org"
#define chiave        "AsD.123"
#define suffisso      "MosUsr"

/*__________________ DEBUG _____________________*/
//#define DEBUG_MQTT
//#define DEBUG_MENU
//#define DEBUG_WIFI
#define DEBUG_OTHER
#define DEBUG_WEATHER
//#define DEBUG_TIMER
//#define WM_DEBUG false      //Disattiva debug wifimanager
#define SERIAL_DEBUG true     //abilita l'uscita seriale nella funzione printDebug
#define TFT_DEBUG true        //abilita l'uscita su display nella funzione printDebug
#define MQTT_DEBUG false       //abilita l'uscita su mqtt nella funzione printDebug

/*______________________ SENSOR DEFINITIONS (input) _____________________*/
#define PIN_DHT        17
#define BUTTON_PREV    13
#define BUTTON_NEXT    14
#define BUTTON_SELECT  15
#define PIN_PHOTO      35
#define PIN_BL         21
#define PIN_FAN        26


/*_____________________ OUTPUT DEFINITIONS  (output) _____________________*/
#define PIN_LED_BLU      22
#define PIN_LED_RED      25 
#define PIN_LED_WHITE    27
#define PIN_LED_GREEN    32
#define PIN_LED_UV       33

/*_____________DEFINIZIONE DI ALTRI COLORI_______________*/
#define TFT_BLACK     0x0000
#define TFT_BLUE      0x001F
#define TFT_RED       0xF800
#define TFT_GREEN     0x07E0
#define TFT_CYAN      0x07FF
#define TFT_MAGENTA   0xF81F
#define TFT_YELLOW    0xFFE0
#define TFT_WHITE     0xFFFF
#define TFT_ORANGE    0xFD20
#define TFT_GRAY      0x8430
#define TFT_DARKGRAY  0x52AA

// dimensioni display
const int screenWidth = 320;
const int screenHeight = 170;

struct Scenario{	//Struttura scenari
  String  nome;           // nome dello scenario
  const uint16_t* icona;  // puntatore all'icona
  int   ledWhite;         // valore luminosità led bianco
  int   ledRed;           // valore luminosità led rosso
  int   ledBlue;          // valore luminosità led blu
  int   luminosita;       // soglia luminosità ambientale per spegnere i led
  int   timer;            // ore durata accensione timer
  int   xIco;             // X dell'icona
  int   yIco;             // y dell'icona
  int   xText;            // X del testo
  int   yText;            // y del testo
  int   ciclo;            // 1= ciclo Alba/Tramonto attivo; 0= disattivo
  int   umidita_H;        // soglia umidità superiore per alert
  int   umidita_L;        // soglia umidità inferiore per alert
};

struct DecodeMeteo{	//Struttura decodeMeteo
  const uint16_t* icona;     //puntatore nome all'array contenente l'icona
  const uint16_t* icona32;   //puntatore nome all'array contenente l'icona a 32 px
  String decoted;            //stringa del meteo
};

struct matrice{ //posizione testo e icona della lista delle previsioni giornaliere
  int x_g;
  int y_g;
  int x_i;
  int y_i;
};

matrice posizioneIcone[]{
  //x,y giorno, x,y icona
  {   8,  58, 125,  47},
  {   8, 100, 125,  89},
  {   8, 142, 125, 131},
  { 168,  16, 285,   5},
  { 168,  58, 285,  47},
  { 168, 100, 285,  89},
  { 168, 142, 285, 131}
};

Scenario scenari[]{
  //nome,icona,led_white,led_red,led_blue,luminosità,timer,x_icona,y_icona,x_testo,y_testo,ciclo,umidità
  {"Tropicale",     icoTropicale,     30, 20, 60, 60, 12,  0,  0, 35,  8, 0, 95,80},
  {"Mediterraneo",  icoTemperato,     50, 20, 30, 70,  0,  0, 30, 35, 38, 1, 85,70},
  {"Desertico",     icoSoleggiato,    80, 25, 30, 60, 12,  0, 60, 35, 68, 0, 30,10},
  {"Custom",        icoCustom,        80, 30, 30, 70,  10,  0, 90, 35, 98, 0, 85,70}
};

/*________________________ GLOBAL VARIABLES  _____________________________*/
char message_buff[100];
float Temperatura=0;
float Umidita=0;
int Luminosita=0;
String lati;
String longi;
long utcOffset = 0; 		//in ore
String mqtt_nodeID;
String IP;

int lumPhoto=0;
const char* versionURL = URLVER;
//const char* versionURL_Apk = URLVER_APK;
int stato;                	// stato
uint8_t currentIndex = 0;
uint8_t oldCurrentIndex = 0;
String mqtt_user_str;
String mqtt_pass_str;
const char* mqtt_user;
const char* mqtt_pass;
bool connessoMqtt=false;    // stato della connessione al server MQTT
bool conferma=false;        // pulsante premuto nella form di conferma: false="NO"; true="SI"
int scenarioAttivo=1;       // indice dello scenario attualmente attivo. E' memorizzato in preferences
int tipoConferma;           // 0= reset wifi; 1= reset ESP
int hysteresis=0;           // variabile di isteresi
bool displayAlwaysOn=false; // true= display sempre acceso
String email;               // email del profilo
bool disclaimer;            // autorizzazione disclaimer
// Stato interno
bool albaEseguita = false;
bool tramontoEseguito = false;
bool timerDisattivoEseguito=false;
bool isSunriseMode = true;


//Timer

int timeDuration;                   	            // Durata in ore della temporizzazione. Se scenario=mediterraneo (indece 1) il timeDuration viene calcolato come (int)((endTotalMinuter-startTotalMinutes)/60)
bool ledOn=false;                   	            // True se si è forzata l'accensione
bool timerOn=false;                  	            // Timer attivo dalla schedulazione
bool lastTimerOn=false;                           // Ultimo stato del timer per verificare se c'è stato un cambiamento
String lastResetDay;                              // Indica la data dell'avvio (al momento del cambio data effettua il controllo degli aggiornamenti)
int nowTotalMinutes = 0;                          // Ora attuale in minuti
int startHour=ORA_DI_AVVIO_TIMER;                 // Ora avvio timer (ore)
int startTotalMinutes = ORA_DI_AVVIO_TIMER*60;    // Ora inizio timer in minuti
int endTotalMinutes = 0;                          // Ora fine timer in minuti
bool timerAttivo=true;                            // Attiva/Blocca l'utilizzo del timer
bool cicloContinuo=false;                         // Attiva il ciclo continuo Alba/Tramnto
bool lastCicloContinuo=false;                     // Stato precedente del ciclo continuo
uint8_t timerDelay=TIMER_DELAY;                   // Durata del timer dopo accensione manuale dei led (minuti)
uint8_t durataAlba=TIME_SUNRISE;                  // Durata del cicli Alba (minuti)
uint8_t durataTramonto=TIME_SUNSET;               // Durata del ciclo Tramonto (minuti)
int sunriseTotalMinutes;                          // Ora dell'alba in minuti
int sunsetTotalMinutes;                           // Ora del tramonto in minuti



Timer<> timer = timer_create_default();

// variabili per previsioni meteo
JsonDocument doc;
JsonArray timeArrayJson;
JsonArray weathercode;
JsonArray temperatureMax;
JsonArray temperatureMin;
JsonArray precipitationSum;
JsonArray sunrise;
JsonArray sunset;
int currentDay = 0;  // Giorno attualmente visualizzato
int totalDays = 7;
int numDays = 7;
float temperatureCurrent;


//flag for saving data
bool shouldSaveConfig = true;

const char* OTA_HOSTNAME = "ESP32-OTA"; // Nome del dispositivo OTA
const char* OTA_PASSWORD = "admin";    	// Password per l'aggiornamento OTA (opzionale)


/*__________________________ ISTANZIA OGGETTI_______________________________________*/
// Puntatori ai singoli task
Timer<>::Task* task_goMain = nullptr;
Timer<>::Task* task_sensors = nullptr;
Timer<>::Task* task_meteo = nullptr;
Timer<>::Task* task_manualLed = nullptr;

SunriseSunset sunLight(PIN_LED_WHITE, PIN_LED_RED, PIN_LED_BLU); 

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wm;
TFT_eSPI tft = TFT_eSPI();
WiFiUDP ntpUDP;

Preferences preferences;

DHT dht11(PIN_DHT, DHT11);
DHT dht22(PIN_DHT, DHT22);

DHT* activeDHT = nullptr;

//debouncer
OneButton buttonPrev(BUTTON_PREV, true);
OneButton buttonNext(BUTTON_NEXT, true);
OneButton buttonSelect(BUTTON_SELECT, true);

void restoreDisplayAfterTransition();

void loop() {
  client.loop();
  buttonPrev.tick();
  buttonNext.tick();
  buttonSelect.tick();
  timer.tick();
  sunLight.update();
  TimeManager::loop();
  //Serial.println(analogRead(PIN_PHOTO));
 
  // Processa eventuali comandi MQTT in coda
  while (qTail != qHead) {
    auto &m = queueMsg[qTail];
    azionamento(m.payload, m.topic); // esegui il comando ricevuto
    qTail = (qTail + 1) % QSIZE;
  }

  // Serial.print(analogRead(PIN_PHOTO));
  // Serial.print("   ");
  // Serial.println(Luminosita);

  // gestisce attivazione/disattivazione ciclo continuo
  if (cicloContinuo != lastCicloContinuo) {
    lastCicloContinuo = cicloContinuo;
    if (cicloContinuo) {
      auto &sc = scenari[scenarioAttivo];
      sunLight.turnOn(sc.ledWhite, sc.ledRed, sc.ledBlue);
      isSunriseMode = true;
      sunLight.startSunrise();
      ledOn = true;
      stato = 8;
      displayStato(8);
    } else {
      sunLight.stopTransition();
      sunLight.turnOff();
      ledOn = false;
      timerOn = false;
      albaEseguita = false;
      tramontoEseguito = false;
      onTransitionComplete(false);
    }
  }

  if (!cicloContinuo) {
    gestisciTimerLuci(nowTotalMinutes, startTotalMinutes, endTotalMinutes);
  }

  if (stato == 8 && sunLight.isTransitionActive()) {
    updateTransitionBar(sunLight.getTransitionProgress());
  }

  if (stato == 8 && sunLight.isTransitionActive()) {
    updateTransitionBar(sunLight.getTransitionProgress());
  }


  // con timer attivo, spegne/riaccende in base alla luminosità
  if (!cicloContinuo && timerAttivo && timerOn) {      // <-- aggiunto timerAttivo
    if (Luminosita >= scenari[scenarioAttivo].luminosita && ledOn) {
      ledOn = false;
      Serial.print("Troppa luce");
      sunLight.turnOff();
      if (stato == 8) {
        restoreDisplayAfterTransition();
      }
      pubblicaValoriAmbientali();
    } else if (Luminosita <= scenari[scenarioAttivo].luminosita - HYSTERESIS && !ledOn) {
      ledOn = true;
      sunLight.turnOn(scenari[scenarioAttivo].ledWhite, scenari[scenarioAttivo].ledRed, scenari[scenarioAttivo].ledBlue);
      Serial.println("Ripristino luce");
      pubblicaValoriAmbientali();
    }
  }
}

void aggiornaLuminositaScenarioAttivo() {
  if (!(ledOn || cicloContinuo)) {
    return;
  }

  auto &sc = scenari[scenarioAttivo];
  sunLight.turnOn(sc.ledWhite, sc.ledRed, sc.ledBlue);
}

void onTransitionComplete(bool alba){
  if (cicloContinuo) {
    if (alba) {
      // alba terminata, avvia tramonto
      isSunriseMode = false;
      ledOn = false;
      sunLight.startSunset();
      stato = 8;
      displayStato(8);
    } else {
      // tramonto terminato, avvia alba
      auto &sc = scenari[scenarioAttivo];
      sunLight.turnOn(sc.ledWhite, sc.ledRed, sc.ledBlue);
      isSunriseMode = true;
      ledOn = true;
      sunLight.startSunrise();
      stato = 8;
      displayStato(8);
    }
  } else {
    restoreDisplayAfterTransition();
  }
}

void restoreDisplayAfterTransition() {
  stato = 0;
  displayStato(0);
  // riattiva il timer di spegnimento display
  timer.cancel(task_goMain);
  task_goMain = timer.every(TIMER_BL * 60000, goMain);
}
