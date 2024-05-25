<<<<<<< Updated upstream
//
// Created by lilian on 30/04/2024.
//
=======
>>>>>>> Stashed changes
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Servo.h>

// Configuration des pins pour les capteurs DHT et PIR et de l'alarme
const int pinDHT = 2; // Pin où le capteur DHT22 est connecté
const int pinPIR = 3; // Pin où le capteur de mouvement PIR est connecté
const int pinAlarme = 7; // Pin où l'alarme est connectée

// Configuration des pins pour le module RFID
const int RST_PIN = 8; // Pin de réinitialisation du module RFID
const int SS_PIN = 9; // Pin de sélection du module RFID

// Configuration du clavier matriciel 4x4 pour l'authentification
const byte ligneClavier = 4; // Ligne du clavier matriciel
const byte colonneClavier = 4; // Colonne du clavier matriciel
char touchesClavier[ligneClavier][colonneClavier] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
};
byte pinLignesClavier[ligneClavier] = {22, 23, 24, 25};    // Pins des lignes du clavier matriciel
byte pinColonnesClavier[colonneClavier] = {26, 27, 28, 29};  // Pins des colonnes du clavier matriciel
Keypad clavier = Keypad(makeKeymap(touchesClavier), pinLignesClavier, pinColonnesClavier, ligneClavier, colonneClavier);

String codeUIDcarte = " D3 1C D0 24"; // Code UID de la carte autorisée
String code4chiffre = "1234"; // Code à 4 chiffres pour l'authentification

// Configuration des seuils de variation de température et d'humidité
const int seuilVariationTemp = 15; // Seuil de variation de température (en degrés Celsius)
const int seuilVariationHum = 15; // Seuil de variation d'humidité (en pourcentage)

// Variables pour stocker les dernières mesures valides
float derniereTemp = 0.0;
float derniereHum = 0.0;
bool alarmeActive = false; // La variable pour vérifier si l'alarme est activée
bool accesLegitime = false; // La variable pour vérifier l'accès légitime au coffre

// Création de l'instance du capteur DHT et du module RFID
Servo servo;
MFRC522 rfid(SS_PIN, RST_PIN);
DHT dht(pinDHT, DHT22);

void setup() {
    Serial.begin(9600);
<<<<<<< Updated upstream
    servo.write(70); 
    SPI.begin();
    rfid.PCD_Init();
    dht.begin();
    servo.attach(3); 
=======
    servo.write(70);
    SPI.begin();
    rfid.PCD_Init();
    dht.begin();
    servo.attach(3);
>>>>>>> Stashed changes
    pinMode(pinAlarme, OUTPUT);
    pinMode(pinPIR, INPUT);
}

void loop() {
    if (authentificationRFID()) {
        Serial.println("RFID validé, veuillez entrer votre code PIN:");
        if (verificationPIN()) {
            envoyerAlerte("Accès autorisé!");
        }
<<<<<<< Updated upstream
=======
    } else {
        envoyerAlerte("Accès non autorisé!");
        surveillerCapteurs();
>>>>>>> Stashed changes
    }
}

bool authentificationRFID() {

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
<<<<<<< Updated upstream
      envoyerAlerte("Accès non autorisé");
      return false;
    }

=======
        envoyerAlerte("Accès non autorisé");
        return false;
    }
>>>>>>> Stashed changes

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
<<<<<<< Updated upstream

    envoyerDataAffichage(temperature, humidite);

=======
>>>>>>> Stashed changes
    int mouvement = digitalRead(pinPIR); // Lire l'état du capteur de mouvement

    envoyerDataAffichage(temperature, humidite, mouvement);

    // Vérifier les conditions d'alerte
    if (!accesLegitime && (abs(temperature - derniereTemp) > seuilVariationTemp || abs(humidite - derniereHum) > seuilVariationHum || mouvement == HIGH)) {
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
    } else {
        alarmeActive = false;
        digitalWrite(pinAlarme, LOW); // Désactiver l'alarme
    }

    derniereTemp = temperature;
    derniereHum = humidite;
}

<<<<<<< Updated upstream
void envoyerDataAffichage(float temperature, float humidite) {
  Serial.println(temperature);
  Serial.println(humidite);
}

void envoyerAlerte(String message) {
  Serial.println(message);
}
=======
void envoyerDataAffichage(float temperature, float humidite, int mouvement) {
    Serial.println(temperature);
    Serial.println(humidite);
    Serial.println(mouvement);
}

void envoyerAlerte(String message) {
    Serial.println(message);
}
>>>>>>> Stashed changes
