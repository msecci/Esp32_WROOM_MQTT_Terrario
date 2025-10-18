// SunriseSunset.cpp
#include "SunriseSunset.h"

SunriseSunset::SunriseSunset(int pin_w, int pin_r, int pin_b) {
    pin_bianco = pin_w;
    pin_rosso = pin_r;
    pin_blu = pin_b;
    
    // Valori di default
    max_bianco = 3000;
    max_rosso = 2500;
    max_blu = 2000;
    durata_alba = 30000;      // 30 secondi
    durata_tramonto = 45000;  // 45 secondi
    
    // Stato iniziale
    transizione_attiva = false;
    is_alba = false;
    timestamp_inizio = 0;
    ultimo_aggiornamento = 0;
    
    // Inizializzazione array
    for (int i = 0; i < 3; i++) {
        valori_iniziali[i] = 0;
        valori_target[i] = 0;
        valori_attuali[i] = 0;
    }
}

void SunriseSunset::begin() {
    // Configurazione canali PWM con la nuova API ESP32 Core 3.0+
    ledcAttach(pin_bianco, FREQ_PWM, RISOLUZIONE_PWM);
    ledcAttach(pin_rosso, FREQ_PWM, RISOLUZIONE_PWM);
    ledcAttach(pin_blu, FREQ_PWM, RISOLUZIONE_PWM);
    
    // Spegnimento iniziale
    turnOff();
}

void SunriseSunset::setMaxValues(int max_w, int max_r, int max_b) {
    max_bianco = constrain(max_w, 0, 4095);
    max_rosso = constrain(max_r, 0, 4095);
    max_blu = constrain(max_b, 0, 4095);
}

void SunriseSunset::setTransitionDuration(unsigned long alba_sec, unsigned long tramonto_sec) {
    durata_alba = alba_sec * 1000UL;       // Converti in millisecondi
    durata_tramonto = tramonto_sec * 1000UL;
}

void SunriseSunset::setLedValues(int bianco_percent, int rosso_percent, int blu_percent) {
    // Converti percentuali in valori PWM
    valori_target[0] = map(constrain(bianco_percent, 0, 100), 0, 100, 0, max_bianco);
    valori_target[1] = map(constrain(rosso_percent, 0, 100), 0, 100, 0, max_rosso);
    valori_target[2] = map(constrain(blu_percent, 0, 100), 0, 100, 0, max_blu);
    
    // Se non in transizione, applica immediatamente
    if (!transizione_attiva) {
        for (int i = 0; i < 3; i++) {
            valori_attuali[i] = valori_target[i];
        }
        applicaValoriLED();
    }
}

void SunriseSunset::turnOn(int bianco_percent, int rosso_percent, int blu_percent) {
    setLedValues(bianco_percent, rosso_percent, blu_percent);
}

void SunriseSunset::turnOff() {
    stopTransition();
    
    for (int i = 0; i < 3; i++) {
        valori_target[i] = 0;
        valori_attuali[i] = 0;
    }
    
    applicaValoriLED();
}

bool SunriseSunset::startSunrise() {
    if (transizione_attiva) {
        return false; // Transizione già in corso
    }
    
    // Imposta valori iniziali (spento)
    valori_iniziali[0] = 0;
    valori_iniziali[1] = 0;
    valori_iniziali[2] = 0;
    
    // Avvia transizione
    transizione_attiva = true;
    is_alba = true;
    timestamp_inizio = millis();
    ultimo_aggiornamento = 0;
    
    // Imposta valori iniziali
    for (int i = 0; i < 3; i++) {
        valori_attuali[i] = valori_iniziali[i];
    }
    applicaValoriLED();
    
    return true;
}

bool SunriseSunset::startSunset() {
    if (transizione_attiva) {
        return false; // Transizione già in corso
    }
    
    // Imposta valori iniziali (attuali)
    for (int i = 0; i < 3; i++) {
        valori_iniziali[i] = valori_attuali[i];
    }
    
    // Valori finali (spento)
    int target_backup[3];
    for (int i = 0; i < 3; i++) {
        target_backup[i] = valori_target[i];
        valori_target[i] = 0;
    }
    
    // Avvia transizione
    transizione_attiva = true;
    is_alba = false;
    timestamp_inizio = millis();
    ultimo_aggiornamento = 0;
    
    return true;
}

void SunriseSunset::stopTransition() {
    if (transizione_attiva) {
        transizione_attiva = false;
    }
}

bool SunriseSunset::isTransitionActive() const {
    return transizione_attiva;
}

float SunriseSunset::getTransitionProgress() const {
    if (!transizione_attiva) {
        return 0.0;
    }
    
    unsigned long tempo_trascorso = millis() - timestamp_inizio;
    unsigned long durata_totale = is_alba ? durata_alba : durata_tramonto;
    
    return min(1.0f, (float)tempo_trascorso / durata_totale);
}

unsigned long SunriseSunset::getRemainingTime() const {
    if (!transizione_attiva) {
        return 0;
    }
    
    unsigned long tempo_trascorso = millis() - timestamp_inizio;
    unsigned long durata_totale = is_alba ? durata_alba : durata_tramonto;
    
    if (tempo_trascorso >= durata_totale) {
        return 0;
    }
    
    return durata_totale - tempo_trascorso;
}

void SunriseSunset::getCurrentValues(int& bianco, int& rosso, int& blu) const {
    bianco = valori_attuali[0];
    rosso = valori_attuali[1];
    blu = valori_attuali[2];
}

void SunriseSunset::getCurrentPercentages(int& bianco_percent, int& rosso_percent, int& blu_percent) const {
    bianco_percent = map(valori_attuali[0], 0, max_bianco, 0, 100);
    rosso_percent = map(valori_attuali[1], 0, max_rosso, 0, 100);
    blu_percent = map(valori_attuali[2], 0, max_blu, 0, 100);
}

void SunriseSunset::getTargetPercentages(int& bianco_percent, int& rosso_percent, int& blu_percent) const {
    bianco_percent = map(valori_target[0], 0, max_bianco, 0, 100);
    rosso_percent = map(valori_target[1], 0, max_rosso, 0, 100);
    blu_percent = map(valori_target[2], 0, max_blu, 0, 100);
}

void SunriseSunset::setTransitionCallback(void (*callback)(bool alba_completata)) {
    callback_fine_transizione = callback;
}

void SunriseSunset::update() {
    if (!transizione_attiva) {
        return;
    }
    
    // Throttling per performance - aggiorna solo ogni INTERVALLO_AGGIORNAMENTO ms
    unsigned long ora = millis();
    if (ora - ultimo_aggiornamento < INTERVALLO_AGGIORNAMENTO) {
        return;
    }
    ultimo_aggiornamento = ora;
    
    // Aggiorna transizione
    if (aggiornaTransizione()) {
        completaTransizione();
    }
}

void SunriseSunset::forceUpdate() {
    if (transizione_attiva) {
        if (aggiornaTransizione()) {
            completaTransizione();
        }
    }
}

bool SunriseSunset::aggiornaTransizione() {
    unsigned long tempo_trascorso = millis() - timestamp_inizio;
    unsigned long durata_totale = is_alba ? durata_alba : durata_tramonto;
    
    if (tempo_trascorso >= durata_totale) {
        // Transizione completata
        for (int i = 0; i < 3; i++) {
            valori_attuali[i] = valori_target[i];
        }
        applicaValoriLED();
        return true; // Transizione completata
    }
    
    // Calcola progresso (0.0 - 1.0)
    float progresso = (float)tempo_trascorso / durata_totale;
    calcolaValoriTransizione(progresso);
    applicaValoriLED();
    
    return false; // Transizione in corso
}

void SunriseSunset::calcolaValoriTransizione(float progresso) {
    if (is_alba) {
        aggiornaAlba(progresso);
    } else {
        aggiornaTramonto(progresso);
    }
}

void SunriseSunset::aggiornaAlba(float progresso) {
    // Alba più naturale: tutti i colori crescono ma con tempi diversi
    // Rosso: 0% -> 100% gradualmente per tutta la transizione (più lento)
    // Bianco: 0% -> 100% dal 20% al 80% della transizione  
    // Blu: 0% -> 100% dal 60% al 100% della transizione
    
    float rosso_progress = 0.0f;
    float bianco_progress = 0.0f;
    float blu_progress = 0.0f;
    
    // Calcola progresso per rosso (0-100%) - crescita più graduale
    rosso_progress = progresso;
    
    // Calcola progresso per bianco (20-80%)
    if (progresso <= 0.2f) {
        bianco_progress = 0.0f;
    } else if (progresso <= 0.8f) {
        bianco_progress = (progresso - 0.2f) / 0.6f;
    } else {
        bianco_progress = 1.0f;
    }
    
    // Calcola progresso per blu (60-100%)
    if (progresso <= 0.6f) {
        blu_progress = 0.0f;
    } else {
        blu_progress = (progresso - 0.6f) / 0.4f;
    }
    
    // Applica i valori con easing
    valori_attuali[0] = (int)(valori_target[0] * easeInOut(bianco_progress));
    valori_attuali[1] = (int)(valori_target[1] * easeInOut(rosso_progress));
    valori_attuali[2] = (int)(valori_target[2] * easeInOut(blu_progress));
}

void SunriseSunset::aggiornaTramonto(float progresso) {
    // Tramonto graduale con transizioni sovrapposte (speculare all'alba)
    // Blu: 100% -> 0% nei primi 60% della transizione
    // Bianco: 100% -> 0% dal 20% all'80% della transizione
    // Rosso: 100% -> 0% dal 40% al 100% della transizione
    
    float blu_progress = 0.0f;
    float bianco_progress = 0.0f;
    float rosso_progress = 0.0f;
    
    // Calcola progresso per blu (0-60%)
    if (progresso <= 0.6f) {
        blu_progress = progresso / 0.6f;
    } else {
        blu_progress = 1.0f;
    }
    
    // Calcola progresso per bianco (20-80%)
    if (progresso <= 0.2f) {
        bianco_progress = 0.0f;
    } else if (progresso <= 0.8f) {
        bianco_progress = (progresso - 0.2f) / 0.6f;
    } else {
        bianco_progress = 1.0f;
    }
    
    // Calcola progresso per rosso (40-100%)
    if (progresso <= 0.4f) {
        rosso_progress = 0.0f;
    } else {
        rosso_progress = (progresso - 0.4f) / 0.6f;
    }
    
    // Applica i valori con easing (inverso per spegnimento)
    valori_attuali[0] = (int)(valori_iniziali[0] * (1.0f - easeInOut(bianco_progress)));
    valori_attuali[1] = (int)(valori_iniziali[1] * (1.0f - easeInOut(rosso_progress)));
    valori_attuali[2] = (int)(valori_iniziali[2] * (1.0f - easeInOut(blu_progress)));
}

float SunriseSunset::easeInOut(float t) {
    return t * t * (3.0f - 2.0f * t);
}

void SunriseSunset::applicaValoriLED() {
    // Applica i limiti di sicurezza
    int bianco = constrain(valori_attuali[0], 0, max_bianco);
    int rosso = constrain(valori_attuali[1], 0, max_rosso);
    int blu = constrain(valori_attuali[2], 0, max_blu);
    
    // Aggiorna PWM con la nuova API ESP32 Core 3.0+
    ledcWrite(pin_bianco, bianco);
    ledcWrite(pin_rosso, rosso);
    ledcWrite(pin_blu, blu);
}

void SunriseSunset::completaTransizione() {
    transizione_attiva = false;
    
    // Chiama callback se impostato
    if (callback_fine_transizione != nullptr) {
        callback_fine_transizione(is_alba);
    }
}