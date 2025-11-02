#include "main.h"

const int mac_index_ku = ANTONIUS_MICHAEL_YORDANIS_HARTONO;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("ESP32 Receiver Initializing...");
    Serial.println("========================================");
    
    esp_err_t result = mulai_esp_now(mac_index_ku);
    
    if (result != ESP_OK) {
        Serial.printf("[ERROR] Failed to initialize ESP-NOW: %s\n", esp_err_to_name(result));
        Serial.println("[ERROR] Restarting in 5 seconds...");
        delay(5000);
        ESP.restart();
    }
    
    Serial.println("========================================");
    Serial.println("Receiver ready!");
    Serial.println("Waiting for commands from Transmitter...");
    Serial.println("========================================\n");
}

void loop() {
    delay(10);
}