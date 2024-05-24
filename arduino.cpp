//
// Created by lilian on 30/04/2024.
//
#include <DHT.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

// Configuration des pins pour les capteurs DHT et PIR et de l'alarme
const int pinDHT = 2; // Pin où le capteur DHT22 est connecté
const int pinPIR = 3; // Pin où le capteur de mouvement PIR est connecté
const int pinAlarme = 7; // Pin où l'alarme est connectée

// Configuration des pins pour le module RFID
const int RST_PIN = 9; // Pin de réinitialisation du module RFID
const int SS_PIN = 10; // Pin de sélection du module RFID

// Configuration du clavier matriciel 4x4 pour l'authentification
const byte ligneClavier = 4; // Ligne du clavier matriciel
const byte colonneClavier = 4; // Colonne du clavier matriciel
char touchesClavier[ligneClavier][colonneClavier] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte pinLignesClavier[ligneClavier] = {5, 6, 7, 8};    // Pins des lignes du clavier matriciel
byte pinColonnesClavier[colonneClavier] = {11, 12, 13, A0};  // Pins des colonnes du clavier matriciel
Keypad clavier = Keypad(makeKeymap(touchesClavier), pinLignesClavier, pinColonnesClavier, ligneClavier, colonneClavier);

String codeUIDcarte = ""; // Code UID de la carte autorisée
String code4chiffre = "1234"; // Code à 4 chiffres pour l'authentification

// Configuration des seuils de variation de température et d'humidité
const int seuilVariationTemp = 5; // Seuil de variation de température (en degrés Celsius)
const int seuilVariationHum = 10; // Seuil de variation d'humidité (en pourcentage)

// Variables pour stocker les dernières mesures valides
float derniereTemp = 0.0;
float derniereHum = 0.0;
bool alarmeActive = false; // La variable pour vérifier si l'alarme est activée
bool accesLegitime = false; // La variable pour vérifier l'accès légitime au coffre

// Création de l'instance du capteur DHT et du module RFID
MFRF522 rfid(SS_PIN, RST_PIN);
DHT dht(pinDHT, DHT22);
IRrecv irrecv(pinIR);
decode_results resultats;

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init();
    dht.begin();
    irrecv.enableIRIn(); // Démarrer la réception IR
    pinMode(pinAlarme, OUTPUT);
    pinMode(pinPIR, INPUT);
}

void loop() {
    delay(2000); // Délai entre les mesures
    if (authentificationRFID()) {
        Serial.println("RFID validé, veuillez entrer votre code PIN:");
        if (verificationPIN()) {
            envoyerAlerte("Accès autorisé!");
        }
    } else {
        envoyerAlerte("Accès non autorisé!");
    }
    surveillerCapteurs();
    if (irrecv.decode(&resultats)) {
        if (resultats.value == 0xFFA25D) { // Code de la télécommande IR
            alarmeActive = false;
            digitalWrite(pinAlarme, LOW); // Désactiver l'alarme
            irrecv.resume(); // Recevoir la prochaine valeur
        }
    }
}

bool authentificationRFID() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
        return false;

    String ID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
        ID.concat(String(rfid.uid.uidByte[i], HEX));
    }
    ID.toUpperCase();

    return ID == codeUIDcarte;
}

bool verificationPIN() {
    String codeSaisi = "";
    char touche = clavier.getKey();
    while (touche != '#') { // Lire jusqu'à ce que '#' soit pressé
        if (touche != NO_KEY && touche != '#') {
            codeSaisi += touche;
        }
        touche = clavier.getKey();
        delay(100); // Délai pour gérer la vitesse de saisie
    }
    return codeSaisi == code4chiffre;
}

void surveillerCapteurs() {
    float humidite = dht.readHumidity(); // Lire l'humidité
    float temperature = dht.readTemperature(); // Lire la température

    // Vérifier si les valeurs lues sont valides
    if (isnan(humidite) || isnan(temperature)) {
        Serial.println("Erreur de lecture du capteur DHT");
        return;
    }

    envoyerDataAffichage(temperature, humidite);

    int mouvement = digitalRead(pinPIR); // Lire l'état du capteur de mouvement

    // Vérifier les conditions d'alerte
    if (!accesLegitime && (abs(temperature - derniereTemp) > seuilVariationTemp || abs(humidite - derniereHum) > seuilVariationHum || mouvement)) {
        alarmeActive = true;
        digitalWrite(pinAlarme, HIGH); // Activer l'alarme
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

    derniereTemp = temperature;
    derniereHum = humidite;
}

void envoyerDataAffichage(float temperature, float humidite) {
    // TODO Implémentez la fonctionnalité d'envoi des données d'affichage ici.
}

void envoyerAlerte(String message) {
    // TODO Implémentez la fonctionnalité d'envoi d'alerte ici.
}