#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Servo.h>

// Configuration des pins pour les capteurs DHT et PIR et de l'alarme
const int pinDHT = 2; // Pin où le capteur DHT11 est connecté
const int pinPIR = 3; // Pin où le capteur de mouvement PIR est connecté
const int pinAlarme = 7; // Pin où le buzzer est connecté

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
String code4chiffre = "1234"; // Code à 4 chiffres pour l'authentification PIN
String CLOSE_CODE = "9999";  // Code PIN pour refermer le coffre

// Configuration des seuils de variation de température et d'humidité
const float seuilMaximumTemp = 25.0; // Seuil de variation de température (en degrés Celsius)
const float seuilMaximumHum = 60.0; // Seuil de variation d'humidité (en pourcentage)

int seuilMouvementPrecedent = LOW; // Etat précédent du capteur de mouvement

bool alarmeActive = false; // La variable pour vérifier si l'alarme est activée
bool accesLegitime = false; // La variable pour vérifier l'accès légitime au coffre

// Création de l'instance du capteur DHT, du module RFID et du servo-moteur
Servo servo;
MFRC522 rfid(SS_PIN, RST_PIN);
DHT dht(pinDHT, DHT11);

void setup() {
    Serial.begin(9600);
    servo.write(120); // Position de fermeture du servo 
    SPI.begin();
    rfid.PCD_Init();
    dht.begin();
    servo.attach(5); 
    pinMode(pinAlarme, OUTPUT);
    pinMode(pinPIR, INPUT);
}

void loop() {
    delay(2000); // Délai entre les mesures
    afficherMessage("Action requise : Passez votre badge RFID");
    if (authentificationRFID() || accesLegitime) {
        Serial.println("RFID validé, veuillez entrer votre code PIN");
        if (verificationPIN()) {
            afficherMessage("L'accès est autorisé, ouverture du coffre.");
            servo.write(20); // Ouvrir le coffre
            accesLegitime = true;
        }
    } else if (!accesLegitime) {
        surveillerCapteurs();
    }
}

/**
* Fonction pour l'authentification RFID en comparant le code UID de la carte avec le code autorisé.
* Si l'authentification est réussie, l'accès est autorisé.
* @return true si l'authentification est réussie, false sinon.
*/
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

/**
* Fonction pour la vérification du code PIN saisi par l'utilisateur. 
* Si le code PIN est correct, l'accès est autorisé.
* Si le code de fermeture est saisi, le coffre est fermé.
* @return true si le code PIN est correct, fasle si le code de fermeture est saisi et false si le code PIN est incorrect.
*/
bool verificationPIN() {
    String codeSaisi = "";
    char touche = clavier.getKey(); // Lire la touche du clavier
    while (touche != '#') { // Lire jusqu'à ce que '#' soit pressé
        if (touche != NO_KEY && touche != '#') {
            codeSaisi += touche;
        }
        touche = clavier.getKey();
        delay(100); // Délai pour gérer la vitesse de saisie
    }
    if (accesLegitime && codeSaisi == CLOSE_CODE) {
        fermerCoffre();
        return false;
    }
    return codeSaisi == code4chiffre;
}

void fermerCoffre() {
    servo.write(120); // Position de fermeture du servo
    accesLegitime = false; // Réinitialiser l'accès légitime
    afficherMessage("Coffre fermé et sécurisé.");
}

/**
* Fonction pour surveiller les capteurs de température, d'humidité et de mouvement.
* Si les conditions d'alerte sont remplies, l'alarme est activée.
* Si l'accès est légitime, l'alarme est désactivée.
*/
void surveillerCapteurs() {
    float humidite = dht.readHumidity(); // Lire l'humidité
    float temperature = dht.readTemperature(); // Lire la température
    int mouvement = digitalRead(pinPIR); // Lire l'état du capteur de mouvement

    // Vérifier les conditions d'alerte
    if (temperature > seuilMaximumTemp || humidite > seuilMaximumHum || (seuilMouvementPrecedent == LOW && mouvement == HIGH)) {
        alarmeActive = true;
        digitalWrite(pinAlarme, HIGH); // Activer l'alarme
        for (int i = 0; i < 10; i++) {
            tone(pinAlarme, 1000); // Activer le buzzer
            delay(500);
            noTone(pinAlarme); // Désactiver le buzzer
            delay(500);
        }
        servo.write(120); // Fermer le coffre
        if (seuilMouvementPrecedent == LOW && mouvement == HIGH) {
            afficherMessage("Mouvement détecté sans accès légitime !");
        }
        if (temperature > seuilMaximumTemp) {
            afficherMessage("Changement brusque de température détecté !");
        }
        if (humidite > seuilMaximumHum) {
            afficherMessage("Changement brusque d'humidité détecté !");
        }
    } else {
        alarmeActive = false;
        digitalWrite(pinAlarme, LOW); // Désactiver l'alarme
    }
}

/**
* Fonction pour afficher les messages d'alerte sur le moniteur série.
* @param message - Le message d'alerte à afficher
*/
void afficherMessage(String message) {
    Serial.println(message);
}