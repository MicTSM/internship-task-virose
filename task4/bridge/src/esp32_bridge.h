// Header file ESP32-Bridge
#ifndef ESP32_BRIDGE_H
#define ESP32_BRIDGE_H

#include <Arduino.h>   // Serial, String
#include <esp_now.h>   // ESP-NOW
#include <WiFi.h>      // WiFi functions

// MAC Address ESP32-Receiver
extern uint8_t receiverMac[];

// Callback setelah kirim data
void onDataSent(const uint8_t *mac, esp_now_send_status_t status);

#endif
