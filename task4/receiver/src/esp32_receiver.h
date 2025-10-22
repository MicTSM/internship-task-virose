// Header file ESP32-Receiver
#ifndef ESP32_RECEIVER_H
#define ESP32_RECEIVER_H

#include <Arduino.h>      // String, Serial
#include <esp_now.h>      // ESP-NOW
#include <WiFi.h>         // WiFi functions
#include <SPIFFS.h>       // File system
#include <ArduinoJson.h>  // JSON parsing

// Variabel global
extern String receivedPackets[];
extern int packetsCount;
extern int totalPackets;
extern bool allPacketsReceived;

// Fungsi-fungsi
void onDataReceived(const uint8_t *mac, const uint8_t *data, int len);
bool parsePacket(String packet, int &id, int &total, String &data);
String reassembleFile();
void saveToSPIFFS(String fileContent);
void displayOutput(String jsonContent);
void resetTransfer();

#endif
