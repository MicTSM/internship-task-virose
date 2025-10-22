#include "esp32_receiver.h"

// Variabel global untuk penyimpanan paket
String receivedPackets[100];
int packetsCount = 0;
int totalPackets = 0;
bool allPacketsReceived = false;

void setup() {
  // Inisialisasi serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32-Receiver");
  
  // Inisialisasi SPIFFS (file system)
  if (!SPIFFS.begin(true)) {
    Serial.println("Inisialisasi SPIFFS gagal");
    return;
  }
  Serial.println("SPIFFS berhasil diinisialisasi");
  
  // Set WiFi mode ke Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  // Tampilkan MAC address (perlu didaftarkan di Bridge)
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Inisialisasi ESP-NOW gagal");
    return;
  }
  Serial.println("ESP-NOW berhasil diinisialisasi");
  
  // Daftarkan callback untuk terima data
  esp_now_register_recv_cb(onDataReceived);
  
  Serial.println("\nSiap menerima data dari ESP32-Bridge...\n");
}

void loop() {
  // Proses setelah semua paket diterima
  if (allPacketsReceived) {
    String completeFile = reassembleFile();
    saveToSPIFFS(completeFile);
    displayOutput(completeFile);
    resetTransfer();
    allPacketsReceived = false;
  }
  
  delay(100);
}

// Callback dipanggil saat ESP-NOW menerima data
void onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
  // Convert byte array ke String
  String receivedData = "";
  for (int i = 0; i < len; i++) {
    receivedData += (char)data[i];
  }
  
  int packetId, total;
  String packetData;
  
  // Parse paket: id|total|length|checksum|data
  if (parsePacket(receivedData, packetId, total, packetData)) {
    Serial.print("Paket ");
    Serial.print(packetId);
    Serial.print("/");
    Serial.println(total);
    
    // Simpan data paket
    receivedPackets[packetId - 1] = packetData;
    packetsCount++;
    totalPackets = total;
    
    // Cek semua paket sudah diterima
    if (packetsCount >= totalPackets) {
      Serial.println("\nSemua paket diterima\n");
      allPacketsReceived = true;
    }
  }
}

// Parse paket sesuai protokol: id|total|length|checksum|data
bool parsePacket(String packet, int &id, int &total, String &data) {
  // Cari posisi separator '|'
  int pipe1 = packet.indexOf('|');
  int pipe2 = packet.indexOf('|', pipe1 + 1);
  int pipe3 = packet.indexOf('|', pipe2 + 1);
  int pipe4 = packet.indexOf('|', pipe3 + 1);
  
  if (pipe1 == -1 || pipe2 == -1 || pipe3 == -1 || pipe4 == -1) {
    return false;
  }
  
  // Extract data dari paket
  id = packet.substring(0, pipe1).toInt();
  total = packet.substring(pipe1 + 1, pipe2).toInt();
  data = packet.substring(pipe4 + 1);
  
  return true;
}

// Susun ulang semua paket menjadi file lengkap
String reassembleFile() {
  Serial.println("Menyusun ulang file...");
  
  String completeFile = "";
  for (int i = 0; i < totalPackets; i++) {
    completeFile += receivedPackets[i];
  }
  
  Serial.print("File lengkap: ");
  Serial.print(completeFile.length());
  Serial.println(" bytes");
  
  return completeFile;
}

// Simpan file ke SPIFFS dalam mode binary
void saveToSPIFFS(String fileContent) {
  Serial.println("Menyimpan ke SPIFFS...");
  
  File file = SPIFFS.open("/received_data.json", "wb");
  if (!file) {
    Serial.println("Gagal membuka file");
    return;
  }
  
  file.print(fileContent);
  file.close();
  
  Serial.println("File tersimpan");
}

// Tampilkan isi JSON ke serial monitor
void displayOutput(String jsonContent) {
  Serial.println("\n================================");
  Serial.println("[KONTEN FILE YANG DITERIMA]");
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonContent);
  
  if (error) {
    Serial.println("Error parsing JSON");
    Serial.println(jsonContent);
    return;
  }
  
  // Tampilkan setiap field
  Serial.print("NAMA: ");
  Serial.println(doc["nama"].as<String>());
  
  Serial.print("JURUSAN: ");
  Serial.println(doc["jurusan"].as<String>());
  
  Serial.print("UMUR: ");
  Serial.println(doc["umur"].as<int>());
  
  Serial.print("DESKRIPSI DIRI: ");
  Serial.println(doc["deskripsi"].as<String>());
  
  Serial.println("================================\n");
}

// Reset untuk transfer berikutnya
void resetTransfer() {
  Serial.println("Reset\n");
  
  for (int i = 0; i < packetsCount; i++) {
    receivedPackets[i] = "";
  }
  
  packetsCount = 0;
  totalPackets = 0;
}
