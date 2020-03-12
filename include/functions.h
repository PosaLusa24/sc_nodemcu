#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <MFRC522.h>
#include <ESP8266HTTPClient.h>

struct User {
    byte name[49];
    byte fatherName[49];
    byte id[17];
    byte createdDate[17];
    byte expireDate[17];
};

// RFID
void dump_byte_array (byte*, byte);
void read_card (MFRC522, MFRC522::MIFARE_Key, User&);
void print_card (MFRC522, MFRC522::MIFARE_Key);
void write_card (MFRC522, MFRC522::MIFARE_Key, User);

// WIFI
void https_get (HTTPClient&, const uint8_t*, String, String&);
void scapi_create (HTTPClient&, const uint8_t*, User);
void scapi_log (HTTPClient&, const uint8_t*, User);
void scapi_update (HTTPClient&, const uint8_t*, User, String);
bool scapi_check (HTTPClient&, const uint8_t*, User&);
bool scapi_verify (HTTPClient&, const uint8_t*, User);

// SCOM
void scom_lcd (String);
void scom_feedback (String);

#endif
