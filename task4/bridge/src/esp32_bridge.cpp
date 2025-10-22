#include "esp32_bridge.h"

// MAC Address ESP32-Receiver (ganti sesuai MAC receiver kamu)
uint8_t receiverMac[] = {0x30, 0xC9, 0x22, 0x32, 0xC8, 0x70};

// Callback dipanggil setelah ESP-NOW mengirim data
void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.print("Status pengiriman: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Berhasil" : "Gagal");
}

void setup() {
  // Inisialisasi serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32-Bridge");
  
  // Set WiFi mode ke Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  // Tampilkan MAC address ESP32-Bridge
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Inisialisasi ESP-NOW gagal");
    return;
  }
  Serial.println("ESP-NOW berhasil diinisialisasi");
  
  // Daftarkan callback
  esp_now_register_send_cb(onDataSent);
  
  // Tambahkan ESP32-Receiver sebagai peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA; 
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Gagal menambahkan peer");
    return;
  }
  
  Serial.println("Peer berhasil ditambahkan");
  Serial.print("Target MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", receiverMac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println("\n\nSiap menerima data dari laptop...\n");
}

void loop() {
  // Cek data dari serial (laptop)
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    
    // Debug: tampilkan data yang diterima
    Serial.print("Terima: ");
    Serial.print(data.length());
    Serial.println(" bytes");
    
    // Kirim via ESP-NOW (max 250 bytes)
    if (data.length() > 0 && data.length() <= 250) {
      esp_err_t result = esp_now_send(receiverMac, (uint8_t*)data.c_str(), data.length());
      
      // Tunggu sebentar agar ESP-NOW selesai kirim
      delay(50);
    }
  }
  
  delay(10);
}
