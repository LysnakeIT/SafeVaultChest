//
// Created by lilian on 30/04/2024.
//
#include <DHT.h>

// Configuration des pins
const int pinDHT = 2;        // Pin où le capteur DHT22 est connecté
const int pinPIR = 3;        // Pin où le capteur de mouvement PIR est connecté
const int seuilVariationTemp = 5;  // Seuil de variation de température (en degrés Celsius)
const int seuilVariationHum = 10;  // Seuil de variation d'humidité (en pourcentage)

// Variables pour stocker les dernières mesures valides
float derniereTemp = 0.0;
float derniereHum = 0.0;
bool accesLegitime = false;  // La variable pour vérifier l'accès légitime au coffre

// Création de l'instance du capteur DHT
DHT dht(pinDHT, DHT22);

void setup() {
    Serial.begin(9600);
    dht.begin();
    pinMode(pinPIR, INPUT);
}

void loop() {
    delay(2000); // Délai entre les mesures

    float humidite = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Vérifier si les valeurs lues sont valides
    if (isnan(humidite) || isnan(temperature)) {
        Serial.println("Erreur de lecture du capteur DHT");
        return;
    }

    envoyerDataAffichage();

    // Lire l'état du capteur de mouvement
    int mouvement = digitalRead(pinPIR);

    // Vérifier les conditions d'alerte
    if (!accesLegitime && (abs(temperature - derniereTemp) > seuilVariationTemp || abs(humidite - derniereHum) > seuilVariationHum || mouvement)) {
        if (mouvement) {
            envoyerAlerte("Mouvement détecté sans accès légitime!");
        }
        if (abs(temperature - derniereTemp) > seuilVariationTemp) {
            envoyerAlerte("Changement brusque de température détecté!");
        }
        if (abs(humidite - derniereHum) > seuilVariationHum) {
            envoyerAlerte("Changement brusque d'humidité détecté!");
        }
    }

    // Mettre à jour les dernières valeurs valides
    derniereTemp = temperature;
    derniereHum = humidite;
}

void envoyerDataAffichage() {
    // TODO Implémentez la fonctionnalité d'envoi des données d'affichage ici.
}

void envoyerAlerte(String message) {
    // TODO Implémentez la fonctionnalité d'envoi d'alerte ici.
}
