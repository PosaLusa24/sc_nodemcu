#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <time.h>
#include "functions.h"

#define RST_PIN         D1
#define SS_PIN          D2
#define BUTTON          D8
// #define BUZZER          D5
// #define LED_RED         D6
// #define LED_YELLOW      D7
// #define LED_GREEN       D8

// Sector #1, 4-7
// Sector #2, 8-11
// Sector #3, 12-15
// Sector #4, 32-19
// Sector #5, 20-23

/**
 * Card specifications
 * Sector 1, Addr 4-6   : name
 * Sector 2, Addr 8-10  : fatherName
 * Sector 3, Addr 12    : id
 * Sector 4, Addr 32-18 : createdDate
 * Sector 5, Addr 20-22 : expireDate
 */

// WIFI
ESP8266WiFiMulti wifiMulti;
HTTPClient https;
const uint8_t fingerprint[20] = {0x1b, 0x91, 0x76, 0xfa, 0x59, 0xd1, 0xc6, 0x08, 0xcc, 0x14, 0x8b, 0xcd, 0x82, 0x5e, 0x88, 0xff, 0x07, 0x25, 0x77, 0x0e};

// RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Cosmetics
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON, OUTPUT);

    // WIFI
    wifiMulti.addAP("MMA", "megamega");
    while (wifiMulti.run() != WL_CONNECTED) {
        delay(1000);
    }

    // RFID
    SPI.begin();
    mfrc522.PCD_Init();
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
    
    configTime(6.5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    while (!time(nullptr)) {
        delay(500);
    }
    // LCD 
    // analogWrite(6, 0);
    // lcd.begin(16, 2);

    //Serial.println("Ready");
    //Serial.println();
}

void loop() {
    if ( ! WiFi.status() == WL_CONNECTED) return;
    User user {};

    scom_lcd("Scan Card");

    if (digitalRead(BUTTON) && scapi_check(https, fingerprint, user)) {
        scom_lcd("Write Card");
        while (1) {
            delay(100);
            if ( ! mfrc522.PICC_IsNewCardPresent()) continue;
            if ( ! mfrc522.PICC_ReadCardSerial()) continue;
            write_card(mfrc522, key, user);
            scapi_update(https, fingerprint, user, "fileToWrite=false");

            scom_lcd("Card Written");
            scom_feedback("Y");
            delay(3000);

            break;
        }
    } else {
        if ( ! mfrc522.PICC_IsNewCardPresent()) return;
        if ( ! mfrc522.PICC_ReadCardSerial()) return;

        read_card(mfrc522, key, user);
        if (!scapi_verify(https, fingerprint, user)) {
            scom_lcd("Unregistered");
            scom_feedback("N");
            delay(3000);
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return;
        }
        //print_card(mfrc522, key);
        scapi_log(https, fingerprint, user);

        scom_lcd("Card Logged");
        scom_feedback("Y");
        delay(3000);
        /*for (byte k = 0; k < 49 && user.name[k] != '\0'; k++) {
        Serial.write(user.name[k]);
        }
        Serial.println();
        for (byte k = 0; k < 49 && user.fatherName[k] != '\0'; k++) {
        Serial.write(user.fatherName[k]);
        }
        Serial.println();
        for (byte k = 0; k < 17 && user.id[k] != '\0'; k++) {
        Serial.write(user.id[k]);
        }
        Serial.println();
        for (byte k = 0; k < 17 && user.createdDate[k] != '\0'; k++) {
        Serial.write(user.createdDate[k]);
        }
        Serial.println();
        for (byte k = 0; k < 17 && user.expireDate[k] != '\0'; k++) {
        Serial.write(user.expireDate[k]);
        }
        Serial.println();*/
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}


