//
// Created by lilian on 30/04/2024.
//
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>


// Configuration des pins pour les capteurs DHT et PIR
const int pinDHT = 2; // Pin où le capteur DHT22 est connecté
const int pinPIR = 3; // Pin où le capteur de mouvement PIR est connecté

// Configuration des pins pour le module RFID
const int RST_PIN = 9; // Pin de réinitialisation du module RFID
const int SS_PIN = 10; // Pin de sélection du module RFID

String UID = "";    // Stocke l'ID de la carte autorisée
byte lock = 0;      // État de verrouillage (0 pour verrouillé, 1 pour déverrouillé)

// Création des instances servo, de l'écran LCD et du lecteur RFID
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adresse I2C de l'écran LCD
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);            
  servo.write(70);               // Définir la position initiale du servo à 70 degrés (verrouillé)
  lcd.init();                    
  lcd.backlight();               // Allumer le rétroéclairage de l'écran LCD
  servo.attach(3);               // Attacher le servo à la broche 3
  SPI.begin();                   // Initialiser la communication SPI
  rfid.PCD_Init();               
}

void processCard() {
  lcd.clear();                   
  lcd.setCursor(0, 0);           
  lcd.print("Scannage");         
  Serial.print("Tag ID :");    
  String ID = "";                

  // Lire l'ID de la carte et construire la chaîne d'identification
  for (byte i = 0; i < rfid.uid.size; i++) {
    lcd.print(".");              
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ")); // Ajouter un zéro initial si nécessaire
    ID.concat(String(rfid.uid.uidByte[i], HEX)); // Ajouter l'octet au format hexadécimal
    delay(300);                  
  }

  ID.toUpperCase();              // Convertir la chaîne d'identification en majuscules

  // Vérifier si l'ID scanné correspond à l'ID stocké et à l'état de verrouillage
  if (ID.substring(1) == UID && lock == 0) {
    servo.write(70);             
    lcd.clear();                 
    lcd.setCursor(0, 0);         
    lcd.print("Porte verrouillée"); 
    delay(1500);                 
    lcd.clear();                 
    lock = 1;                    // Mettre à jour l'état de verrouillage à verrouillé
  } else if (ID.substring(1) == UID && lock == 1) {
    servo.write(160);            
    lcd.clear();                 
    lcd.setCursor(0, 0);         
    lcd.print("Porte ouverte"); 
    delay(1500);                 
    lcd.clear();                 
    lock = 0;                    // Mettre à jour l'état de verrouillage à déverrouillé
  } else {
    lcd.clear();                 
    lcd.setCursor(0, 0);         
    lcd.print("Carte incorrecte !");    
    delay(1500);                 
    lcd.clear();                 
  }
}

void loop() {
  lcd.setCursor(4, 0);           
  lcd.print("Bienvenue !");      
  lcd.setCursor(1, 1);           
  lcd.print("Présentez votre carte"); 

  // Vérifier si une nouvelle carte RFID est présente
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Tenter de lire le numéro de série de la carte
  if (!rfid.PICC_ReadCardSerial())
    return;

  // Appeler la fonction pour traiter la carte
  processCard();
}


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

String code4chiffre = "1234"; // Code à 4 chiffres pour l'authentification

// Configuration des seuils de variation de température et d'humidité
const int seuilVariationTemp = 5; // Seuil de variation de température (en degrés Celsius)
const int seuilVariationHum = 10; // Seuil de variation d'humidité (en pourcentage)

// Variables pour stocker les dernières mesures valides
float derniereTemp = 0.0;
float derniereHum = 0.0;
bool accesLegitime = false; // La variable pour vérifier l'accès légitime au coffre

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