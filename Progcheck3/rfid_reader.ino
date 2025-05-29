#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Serial.println("Tempelkan kartu RFID Anda...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Buat string kosong untuk menampung hasil konversi
  String uidString = "";

  // Konversi tiap byte UID jadi 2 digit hex string
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uidString += "0"; // tambahkan leading zero agar selalu 2 digit
    }
    uidString += String(rfid.uid.uidByte[i], HEX);
  }

  // Buat huruf jadi uppercase supaya konsisten
  uidString.toUpperCase();

  Serial.println(uidString);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1500);
}
