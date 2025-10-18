// SunriseSunset.h
#ifndef SUNRISE_SUNSET_H
#define SUNRISE_SUNSET_H

#include <Arduino.h>

class SunriseSunset {
private:
    // Pin dei LED
    int pin_bianco;
    int pin_rosso;
    int pin_blu;
    
    // Configurazione PWM
    static const int FREQ_PWM = 5000;
    static const int RISOLUZIONE_PWM = 12; // 12 bit = 0-4095
    
    // Valori massimi per protezione LED (0-4095)
    int max_bianco; 
    int max_rosso;
    int max_blu;
    
    // Durata transizioni in millisecondi
    unsigned long durata_alba;
    unsigned long durata_tramonto;
    
    // Variabili per transizioni NON-BLOCCANTI
    bool transizione_attiva;
    bool is_alba;
    unsigned long timestamp_inizio;
    unsigned long ultimo_aggiornamento;
    static const unsigned long INTERVALLO_AGGIORNAMENTO = 16; // ~60fps
    
    int valori_iniziali[3];  // bianco, rosso, blu
    int valori_target[3];    // valori obiettivo
    int valori_attuali[3];   // valori correnti
    
    // Callback per fine transizione (opzionale)
    void (*callback_fine_transizione)(bool alba_completata) = nullptr;
    
    // Funzioni private
    bool aggiornaTransizione();
    void calcolaValoriTransizione(float progresso);
    void aggiornaAlba(float progresso);
    void aggiornaTramonto(float progresso);
    float easeInOut(float t);
    void applicaValoriLED();
    void completaTransizione();
    
public:
    // Costruttore - I canali non sono più necessari con ESP32 Core 3.0+
    SunriseSunset(int pin_w = 3, int pin_r = 4, int pin_b = 1);
    
    // Inizializzazione
    void begin();
    
    // Configurazione valori massimi LED (0-4095)
    void setMaxValues(int max_w, int max_r, int max_b);
    
    // Configurazione durata transizioni (in secondi)
    void setTransitionDuration(unsigned long alba_sec, unsigned long tramonto_sec);
    
    // Controllo LED con valori percentuali (0-100)
    void setLedValues(int bianco_percent, int rosso_percent, int blu_percent);
    
    // Accensione con valori percentuali
    void turnOn(int bianco_percent, int rosso_percent, int blu_percent);
    
    // Spegnimento immediato
    void turnOff();
    
    // Transizioni NON-BLOCCANTI
    bool startSunrise();     // Ritorna false se già in transizione
    bool startSunset();      // Ritorna false se già in transizione
    void stopTransition();
    
    // Stato
    bool isTransitionActive() const;
    float getTransitionProgress() const;  // 0.0 - 1.0
    unsigned long getRemainingTime() const; // millisecondi rimanenti
    
    void getCurrentValues(int& bianco, int& rosso, int& blu) const;
    void getCurrentPercentages(int& bianco_percent, int& rosso_percent, int& blu_percent) const;
    void getTargetPercentages(int& bianco_percent, int& rosso_percent, int& blu_percent) const;
    
    // Callback per eventi (opzionale)
    void setTransitionCallback(void (*callback)(bool alba_completata));
    
    // Aggiornamento NON-BLOCCANTE - chiamare nel loop principale
    void update();
    
    // Forza aggiornamento immediato LED (bypassa throttling)
    void forceUpdate();
};

#endif