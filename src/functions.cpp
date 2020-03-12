#include <Arduino.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <time.h>
#include "functions.h"

char* jsontime(const struct tm *timeptr)
{
  static char result[26];
  sprintf(result, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
    1900 + timeptr->tm_year,
    timeptr->tm_mon + 1,
    timeptr->tm_mday, timeptr->tm_hour,
    timeptr->tm_min, timeptr->tm_sec);
  return result;
}
// Helper routine to dump a byte array as hex values to Serial.
void dump_byte_array (byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        //Serial.print(buffer[i], HEX);
    }
}
// Reads card info to User struct
void read_card (MFRC522 mfrc522, MFRC522::MIFARE_Key key, User &user) {
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);
    for (byte i = 1; i < 6; i++) {

        // Authenticate using key A
        status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, ((i+1)*4)-1, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
            //Serial.print(F("PCD_Authenticate() failed: "));
            //Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        // Read
        switch (i) {
            case 1: {
                        for (byte j = 0; j < 3; j++) {
                            status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(i*4+j, buffer, &size);
                            if (status != MFRC522::STATUS_OK) {
                                //Serial.print(F("MIFARE_Read() failed: "));
                                //Serial.println(mfrc522.GetStatusCodeName(status));
                            }
                            if (buffer[0] == 0) break;
                            for (byte k = 0; k < 16 && buffer[k] != '\0'; k++) {
                                //Serial.write(buffer[k]);
                                user.name[k+j*16] = buffer[k];
                            }
                        }
                        break;
                    }
            case 2: {
                        for (byte j = 0; j < 3; j++) {
                            status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(i*4+j, buffer, &size);
                            if (status != MFRC522::STATUS_OK) {
                                //Serial.print(F("MIFARE_Read() failed: "));
                                //Serial.println(mfrc522.GetStatusCodeName(status));
                            }
                            if (buffer[0] == 0) break;
                            for (byte k = 0; k < 16 && buffer[k] != '\0'; k++) {
                                //Serial.write(buffer[k]);
                                user.fatherName[k+j*16] = buffer[k];
                            }
                        }
                        break;
                    }
            case 3: {
                        status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(i*4, buffer, &size);
                        if (status != MFRC522::STATUS_OK) {
                            //Serial.print(F("MIFARE_Read() failed: "));
                            //Serial.println(mfrc522.GetStatusCodeName(status));
                        }
                        for (byte k = 0; k < 16 && buffer[k] != '\0'; k++) {
                            //Serial.write(buffer[k]);
                            user.id[k] = buffer[k];
                        }
                        break;
                    }
            case 4: {
                        status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(i*4, buffer, &size);
                        if (status != MFRC522::STATUS_OK) {
                            //Serial.print(F("MIFARE_Read() failed: "));
                            //Serial.println(mfrc522.GetStatusCodeName(status));
                        }
                        for (byte k = 0; k < 16 && buffer[k] != '\0'; k++) {
                            //Serial.write(buffer[k]);
                            user.createdDate[k] = buffer[k];
                        }
                        break;
                    }
            case 5: {
                        status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(i*4, buffer, &size);
                        if (status != MFRC522::STATUS_OK) {
                            //Serial.print(F("MIFARE_Read() failed: "));
                            //Serial.println(mfrc522.GetStatusCodeName(status));
                        }
                        for (byte k = 0; k < 16 && buffer[k] != '\0'; k++) {
                            //Serial.write(buffer[k]);
                            user.expireDate[k] = buffer[k];
                        }
                        break;
                    }
        }
    }
}

// Prints card info directly to Serial
void print_card (MFRC522 mfrc522, MFRC522::MIFARE_Key key) {
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);
    for (byte i = 1; i < 6; i++) {

        // Authenticate using key A
        status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, ((i+1)*4)-1, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
            //Serial.print(F("PCD_Authenticate() failed: "));
            //Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        // Read
        //Serial.print(F("Data in sector ")); //Serial.print(i); //Serial.println(F(":"));
        switch (i) {
            case 1:
            case 2: {
                        for (byte j = 0; j < 3; j++) {
                            status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(i*4+j, buffer, &size);
                            if (status != MFRC522::STATUS_OK) {
                                //Serial.print(F("MIFARE_Read() failed: "));
                                //Serial.println(mfrc522.GetStatusCodeName(status));
                            }
                            if (buffer[0] == 0) break;
                            for (byte k = 0; k < 16 && buffer[k] != '\0'; k++) {
                                Serial.write(buffer[k]);
                            }
                        }
                        break;
                    }
            case 3:
            case 4:
            case 5: {
                        status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(i*4, buffer, &size);
                        if (status != MFRC522::STATUS_OK) {
                            //Serial.print(F("MIFARE_Read() failed: "));
                            //Serial.println(mfrc522.GetStatusCodeName(status));
                        }
                        for (byte k = 0; k < 16 && buffer[k] != '\0'; k++) {
                            Serial.write(buffer[k]);
                        }
                        break;
                    }
        }
        //Serial.println();
    }
}

// Writes card info from User struct
void write_card (MFRC522 mfrc522, MFRC522::MIFARE_Key key, User user) {
    MFRC522::StatusCode status;
    byte buffer[18];
    for (byte i = 1; i < 6; i++) {

        // Authenticate using key B
        status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, ((i+1)*4)-1, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
            //Serial.print(F("PCD_Authenticate() failed: "));
            //Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        // Write
        //Serial.print(F("Writing data into sector ")); //Serial.print(i);
        //Serial.println(F("..."));
        switch (i) {
            case 1: {
                        for (byte j = 0; j < 3; j++) {
                            for (byte k = 0; k < 16; k++) {
                                buffer[k] = user.name[k+j*16];
                            }
                            status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(i*4+j, buffer, 16);
                            if (status != MFRC522::STATUS_OK) {
                                //Serial.print(F("MIFARE_Write() failed: "));
                                //Serial.println(mfrc522.GetStatusCodeName(status));
                            }
                        }
                        break;
                    }
            case 2: {
                        for (byte j = 0; j < 3; j++) {
                            for (byte k = 0; k < 16; k++) {
                                buffer[k] = user.fatherName[k+j*16];
                            }
                            status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(i*4+j, buffer, 16);
                            if (status != MFRC522::STATUS_OK) {
                                //Serial.print(F("MIFARE_Write() failed: "));
                                //Serial.println(mfrc522.GetStatusCodeName(status));
                            }
                        }
                        break;
                    }
            case 3: {
                        for (byte k = 0; k < 16; k++) {
                            buffer[k] = user.id[k];
                        }
                        status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(i*4, buffer, 16);
                        if (status != MFRC522::STATUS_OK) {
                            //Serial.print(F("MIFARE_Write() failed: "));
                            //Serial.println(mfrc522.GetStatusCodeName(status));
                        }
                        break;
                    }
            case 4: {
                        for (byte k = 0; k < 16; k++) {
                            buffer[k] = user.createdDate[k];
                        }
                        status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(i*4, buffer, 16);
                        if (status != MFRC522::STATUS_OK) {
                            //Serial.print(F("MIFARE_Write() failed: "));
                            //Serial.println(mfrc522.GetStatusCodeName(status));
                        }
                        break;
                    }
            case 5: {
                        for (byte k = 0; k < 16; k++) {
                            buffer[k] = user.expireDate[k];
                        }
                        status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(i*4, buffer, 16);
                        if (status != MFRC522::STATUS_OK) {
                            //Serial.print(F("MIFARE_Write() failed: "));
                            //Serial.println(mfrc522.GetStatusCodeName(status));
                        }
                        break;
                    }
        }
    }
    //Serial.println("Data written");
}

// Sends GET request to url
void https_get (HTTPClient &https, const uint8_t fingerprint[], String url, String &res) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);
    if (https.begin(*client, url)) {
        int httpCode = https.GET();
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                res = https.getString();
            }
        } else {
            //Serial.print("[HTTPS] GET failed, error: ");
            //Serial.println(https.errorToString(httpCode).c_str());
        }
        https.end();
    } else {
        //Serial.println("[HTTPS] Unable to connect");
    }
}

// API function for card creation
void scapi_create (HTTPClient &https, const uint8_t fingerprint[], User user) {
    String res = "";
    String createUrl = String("https://scapi.now.sh/api/create?name=") + String((char*)user.name) + String("&fatherName=") + String((char*)user.fatherName) + String("&id=") + String((char*)user.id) + String("&createdDate=") + String((char*)user.createdDate) + String("&expireDate=") + String((char*)user.expireDate);
    https_get(https, fingerprint, createUrl, res);
    //Serial.print("scapi: ");
    //Serial.println(res);
}

// API function for card logging
void scapi_log (HTTPClient &https, const uint8_t fingerprint[], User user) {
    String res = "";
    //String time_string = "2010-02-02"; // TODO: Get Current TIme
    time_t now = time(nullptr);
    struct tm * timeinfo;
    timeinfo = localtime ( &now );
    String updateUrl = String("https://scapi.now.sh/api/update?searchid=") + String((char*)user.id) + String("&log=") + String(jsontime(timeinfo));
    https_get(https, fingerprint, updateUrl, res);
    //Serial.print("scapi: ");
    //Serial.println(res);
}

// API function for card updating
void scapi_update (HTTPClient &https, const uint8_t fingerprint[], User user, String queries) {
    String res = "";
    String updateUrl = String("https://scapi.now.sh/api/update?searchid=") + String((char*)user.id) + String("&") + queries; 
    https_get(https, fingerprint, updateUrl, res);
    //Serial.print("scapi: ");
    //Serial.print(res);
}

// API function for card checking
bool scapi_check (HTTPClient &https, const uint8_t fingerprint[], User &user) {
    String res = "";
    https_get(https, fingerprint, "https://scapi.now.sh/api/check", res);
    //Serial.print("scapi: ");
    //Serial.println(res);
    if (res == "") {
        return false;
    } else {
        const size_t capacity = JSON_OBJECT_SIZE(5) + 276;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, res);

        const char* name = doc["name"];
        const char* fatherName = doc["fatherName"];
        const int id = doc["id"];
        char id_str[49];
        sprintf(id_str, "%d", id);
        const char* createdDate = doc["createdDate"];
        const char* expireDate = doc["expireDate"];
        strcpy((char*)user.name, name);
        strcpy((char*)user.fatherName, fatherName);
        strcpy((char*)user.id, id_str);
        strcpy((char*)user.createdDate, createdDate);
        strcpy((char*)user.expireDate, expireDate);

        return true;
    }
}
bool scapi_verify (HTTPClient& https, const uint8_t fingerprint[], User user) {
    String res = "";
    https_get(https, fingerprint, String(String("https://scapi.now.sh/api/find?searchid=") + String((char*)user.id)), res);
    //Serial.print("scapi: ");
    //Serial.println(res);
    if (res == "") {
        return false;
    } else {
        return true;
    }
}

// scom functions to give commands to slave arduino
// str1: 1st row of lcd, str2: 2nd row of lcd
void scom_lcd (String str1) {
    Serial.flush();
    Serial.print("scom_lcd>\n");
    Serial.print(str1);
    Serial.write('\n');
    //Serial.write(String(String("scom_lcd>\n") + str1 + String("\n") + str2 + String("\n")).c_str());
}

// status: error || correct
void scom_feedback (String status) {
    Serial.flush();
    Serial.print("scom_feedback>\n");
    Serial.print(status);
    Serial.write('\n');
    //Serial.write(String(String("scom_feedback>\n") + status + String("\n")).c_str());
}
