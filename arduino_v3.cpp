/*
Created by lilian on 30/04/2024.

Capteur DHT22 :
Pin de données : pinDHT (Pin 2)

Capteur de mouvement PIR :
Pin de sortie : pinPIR (Pin 3)

Module RFID :
Pin de réinitialisation : RST_PIN (Pin 9)
Pin de sélection : SS_PIN (Pin 10)

Servo moteur :
Pin de contrôle : Pin 5

Écran LCD I2C :
Adresse I2C : 0x27 (SDA et SCL connectés aux pins correspondants de l'Arduino)

Récepteur infrarouge :
Pin de données : RECV_PIN (Pin 4)
 */
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

// Configuration des pins pour les capteurs DHT et PIR
const int pinDHT = 2; // Pin où le capteur DHT22 est connecté
const int pinPIR = 3; // Pin où le capteur de mouvement PIR est connecté

// Configuration des pins pour le module RFID
const int RST_PIN = 9; // Pin de réinitialisation du module RFID
const int SS_PIN = 10; // Pin de sélection du module RFID

String UID = ""; // Stocke l'ID de la carte autorisée
byte lock = 0;   // État de verrouillage (0 pour verrouillé, 1 pour déverrouillé)

// Création des instances servo, de l'écran LCD et du lecteur RFID
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adresse I2C de l'écran LCD
MFRC522 rfid(SS_PIN, RST_PIN);

// Configuration du récepteur IR
#define RECV_PIN 4
IRrecv irrecv(RECV_PIN);
decode_results results;

String inputCodeIR = "";          // Stocke le code PIN saisi via la télécommande IR
const String correctPIN = "1234"; // Code PIN correct pour déverrouiller

void setup()
{
  Serial.begin(9600);
  servo.write(70); // Définir la position initiale du servo à 70 degrés (verrouillé)
  lcd.init();
  lcd.backlight();     // Allumer le rétroéclairage de l'écran LCD
  servo.attach(5);     // Attacher le servo à la broche 5
  SPI.begin();         // Initialiser la communication SPI
  rfid.PCD_Init();     // Initialiser le lecteur RFID
  irrecv.enableIRIn(); // Initialiser le récepteur IR
}

void processCard()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scannage");
  Serial.print("Tag ID :");
  String ID = "";

  // Lire l'ID de la carte et construire la chaîne d'identification
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    lcd.print(".");
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ")); // Ajouter un zéro initial si nécessaire
    ID.concat(String(rfid.uid.uidByte[i], HEX));                // Ajouter l'octet au format hexadécimal
    delay(300);
  }

  ID.toUpperCase(); // Convertir la chaîne d'identification en majuscules

  // Vérifier si l'ID scanné correspond à l'ID stocké et à l'état de verrouillage
  if (ID.substring(1) == UID && lock == 0)
  {
    servo.write(70);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Porte verrouillée");
    delay(1500);
    lcd.clear();
    lock = 1; // Mettre à jour l'état de verrouillage à verrouillé
  }
  else if (ID.substring(1) == UID && lock == 1)
  {
    servo.write(160);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Porte ouverte");
    delay(1500);
    lcd.clear();
    lock = 0; // Mettre à jour l'état de verrouillage à déverrouillé
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Carte incorrecte !");
    delay(1500);
    lcd.clear();
  }
}

void processIR()
{
  if (irrecv.decode(&results))
  {
    // Convertir le code IR reçu en caractère et l'ajouter au code saisi
    char c = (char)results.value;
    inputCodeIR += c;
    lcd.setCursor(0, 1);
    lcd.print(inputCodeIR);

    // Si le code PIN est complet, vérifier s'il est correct
    if (inputCodeIR.length() >= 4)
    {
      if (inputCodeIR == correctPIN)
      {
        if (lock == 1)
        {
          servo.write(160);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Porte ouverte");
          delay(1500);
          lcd.clear();
          lock = 0; // Mettre à jour l'état de verrouillage à déverrouillé
        }
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Code incorrect !");
        delay(1500);
        lcd.clear();
      }
      inputCodeIR = ""; // Réinitialiser le code saisi
    }
    irrecv.resume(); // Recevoir le prochain code IR
  }
}

void surveillerCapteurs()
{
  float humidite = dht.readHumidity();       // Lire l'humidité
  float temperature = dht.readTemperature(); // Lire la température

  // Vérifier si les valeurs lues sont valides
  if (isnan(humidite) || isnan(temperature))
  {
    Serial.println("Erreur de lecture du capteur DHT");
    return;
  }

  envoyerDataAffichage(temperature, humidite);

  int mouvement = digitalRead(pinPIR); // Lire l'état du capteur de mouvement

  // Vérifier les conditions d'alerte
  if (!accesLegitime && (abs(temperature - derniereTemp) > seuilVariationTemp || abs(humidite - derniereHum) > seuilVariationHum || mouvement))
  {
    if (mouvement)
    {
      envoyerAlerte("Mouvement détecté sans accès légitime!");
    }
    if (abs(temperature - derniereTemp) > seuilVariationTemp)
    {
      envoyerAlerte("Changement brusque de température détecté!");
    }
    if (abs(humidite - derniereHum) > seuilVariationHum)
    {
      envoyerAlerte("Changement brusque d'humidité détecté!");
    }
  }

  derniereTemp = temperature;
  derniereHum = humidite;
}

void envoyerDataAffichage(float temperature, float humidite)
{
  // TODO Implémentez la fonctionnalité d'envoi des données d'affichage ici.
}

void envoyerAlerte(String message)
{
  // TODO Implémentez la fonctionnalité d'envoi d'alerte ici.
}

void loop()
{
  lcd.setCursor(4, 0);
  lcd.print("Bienvenue !");
  lcd.setCursor(1, 1);
  lcd.print("Présentez votre carte");

  // Vérifier si une nouvelle carte RFID est présente
  if (rfid.PICC_IsNewCardPresent())
  {
    // Tenter de lire le numéro de série de la carte
    if (rfid.PICC_ReadCardSerial())
    {
      // Appeler la fonction pour traiter la carte
      processCard();
    }
  }

  // Vérifier les entrées de la télécommande IR
  processIR();

  // Surveiller les capteurs
  surveillerCapteurs();
}
