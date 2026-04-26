#include <SoftwareSerial.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define FIREBASE_HOST "attendance-management-sy-12b7f-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "1d4d09832f5fc70348fec28760eb5c1e9f088a87"
#define RXD2 16  
#define TXD2 17    
#define SS_PIN 2  
#define RST_PIN 15  

SoftwareSerial sim800l(RXD2, TXD2);
FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  sim800l.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  Serial.println("Initializing SIM800L, RFID, and LCD...");
  delay(1000);

  sim800l.println("AT");
  delay(1000);
  sim800l.println("AT+CGATT=1");
  delay(1000);
  sim800l.println("AT+CSTT=\"internet\",\"your-user\",\"your-pass\"");
  delay(1000);
  sim800l.println("AT+CIICR");
  delay(1000);
  sim800l.println("AT+CIFSR");  
  delay(1000);

  // Configure Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String tagID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      tagID += String(rfid.uid.uidByte[i], HEX);
    }
    rfid.PICC_HaltA();

    Serial.println("RFID Tag ID: " + tagID);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RFID: " + tagID);

    String path = "/attendance/" + tagID;
    FirebaseJson json;
    json.set("RFID", tagID);
    json.set("Timestamp", millis());

    if (Firebase.updateNode(fbdo, path, json)) {
      Serial.println("Data sent to Firebase: " + tagID);
      lcd.setCursor(0, 1);
      lcd.print("Sent to Firebase");
    } else {
      Serial.println("Firebase push failed: " + fbdo.errorReason());
      lcd.setCursor(0, 1);
      lcd.print("Firebase Error");
    }
  }
  delay(1000);  
}