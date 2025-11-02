#include "main.h"

esp_now_peer_info_t peer_info;

// Daftar alamat MAC perangkat transmitter
uint8_t mac_addresses[MAC_ADDRESS_TOTAL][MAC_ADDRESS_LENGTH] = {
    {0x24, 0x0A, 0xC4, 0x0A, 0x10, 0x10},  // Fauzan Firdaus
    {0x24, 0x0A, 0xC4, 0x0A, 0x10, 0x11},  // Africha Sekar wangi
    {0x24, 0x0A, 0xC4, 0x0A, 0x11, 0x10},  // Rafaina Erin Sadia
    {0x24, 0x0A, 0xC4, 0x0A, 0x11, 0x11},  // Antonius Michael Yordanis Hartono
    {0x24, 0x0A, 0xC4, 0x0A, 0x12, 0x10},  // Dinda Sofi Azzahro
    {0x24, 0x0A, 0xC4, 0x0A, 0x12, 0x11},  // Muhammad Fahmi Ilmi
    {0x24, 0x0A, 0xC4, 0x0A, 0x13, 0x10},  // Dhanishara Zaschya Putri Syamsudin
    {0x24, 0x0A, 0xC4, 0x0A, 0x13, 0x11},  // Irsa Fairuza
    {0x24, 0x0A, 0xC4, 0x0A, 0x14, 0x10},  // Revalinda Bunga Nayla Laksono
    {0x24, 0x0A, 0xC4, 0x0A, 0x21, 0x11},  // BISMA
    {0x24, 0x0A, 0xC4, 0x0A, 0x21, 0x10},  // JSON
    {0x24, 0x0A, 0xC4, 0x0A, 0x20, 0x11},  // FARUG
};

// Nama pemilik perangkat sesuai urutan MAC address
const char* mac_names[MAC_ADDRESS_TOTAL] = {
    "Fauzan Firdaus",                      // 0
    "Africha Sekar Wangi",                 // 1
    "Rafaina Erin Sadia",                  // 2
    "Antonius Michael Yordanis Hartono",   // 3
    "Dinda Sofi Azzahro",                  // 4
    "Muhammad Fahmi Ilmi",                 // 5
    "Dhanishara Zaschya Putri Syamsudin",  // 6
    "Irsa Fairuza",                        // 7
    "Revalinda Bunga Nayla Laksono",       // 8
    "BISMA",                               // 9
    "JSON",                                // 10
    "FARUG",                               // 11
};

// Inisialisasi ESP-NOW dan konfigurasi receiver
esp_err_t mulai_esp_now(int index_mac_address) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    /* Init ESP-NOW */
    esp_err_t result = esp_now_init();
    if (result != ESP_OK) {
        Serial.printf("[ERROR] Failed to initialize ESP-NOW: %s\n", esp_err_to_name(result));
        return result;
    }

    /* Set callback function to handle received data */
    result = esp_now_register_recv_cb(callback_data_esp_now);
    if (result != ESP_OK) {
        Serial.printf("[ERROR] Failed to register receive callback: %s\n", esp_err_to_name(result));
        return result;
    }

    result = esp_now_register_send_cb(callback_pengiriman_esp_now);
    if (result != ESP_OK) {
        Serial.printf("[WARNING] Failed to register send callback: %s\n", esp_err_to_name(result));
        // Not critical for receiver, continue
    }

    /* Set MAC Address */
    uint8_t mac[MAC_ADDRESS_LENGTH];
    memcpy(mac, mac_addresses[index_mac_address], MAC_ADDRESS_LENGTH);
    result = esp_wifi_set_mac(WIFI_IF_STA, mac);
    if (result != ESP_OK) {
        Serial.printf("[ERROR] Failed to set MAC address: %s\n", esp_err_to_name(result));
        return result;
    }

    Serial.print("[INFO] Receiver MAC Address set to: ");
    for (int i = 0; i < MAC_ADDRESS_LENGTH; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < MAC_ADDRESS_LENGTH - 1) Serial.print(":");
    }
    Serial.printf(" (%s)\n", mac_names[index_mac_address]);

    /* Initialize peer_info and set fields*/
    memset(&peer_info, 0, sizeof(esp_now_peer_info_t));
    peer_info.channel = 0;
    peer_info.encrypt = false;

    /* Add All MAC to peer list  */
    for (int i = 0; i < MAC_ADDRESS_TOTAL; i++) {
        memcpy(peer_info.peer_addr, mac_addresses[i], MAC_ADDRESS_LENGTH);
        result = esp_now_add_peer(&peer_info);
        if (result != ESP_OK) {
            Serial.printf("[ERROR] Failed to add peer %s: %s\n", mac_names[i], esp_err_to_name(result));
            return result;
        }
    }

    Serial.println("[INFO] ESP-NOW initialized successfully");
    Serial.println("[INFO] Receiver ready to receive commands...");
    return ESP_OK;
}

// Mencari index MAC address dalam array
int cari_mac_index(const uint8_t* mac) {
    for (int i = 0; i < MAC_ADDRESS_TOTAL; i++) {
        // Compare the MAC address
        if (memcmp(mac, mac_addresses[i], MAC_ADDRESS_LENGTH) == 0)
            return i;
    }

    // if not found return -1
    return -1;
}

// Konversi index MAC address menjadi nama pemilik
String mac_index_to_names(int mac_index) {  
    if ((mac_index < 0 || mac_index >= MAC_ADDRESS_TOTAL) || (mac_index == -1)) {  
        return "Unknown";  
    }
    return mac_names[mac_index];
}

// Callback ketika menerima data melalui ESP-NOW
void callback_data_esp_now(const uint8_t* mac, const uint8_t* data, int len) { 
    int index_mac_asal = cari_mac_index(mac);  // Mencari index MAC address pengirim dalam array
    String sender_name = mac_index_to_names(index_mac_asal);  // Mengkonversi index menjadi nama pengirim
    
    Serial.printf("[INFO] Data diterima dari %s, len: %d\n", sender_name.c_str(), len);
    
    process_perintah(data, len, index_mac_asal); 
}

// Callback untuk status pengiriman data (opsional untuk receiver)
void callback_pengiriman_esp_now(const uint8_t* mac_addr, esp_now_send_status_t status) {  
    if (status == ESP_NOW_SEND_SUCCESS) {  
        Serial.println("[INFO] Data berhasil dikirim");  
    } else {  // Jika pengiriman gagal
        Serial.printf("[WARNING] Pengiriman gagal: %s\n", esp_err_to_name(status));  
    }
}

// Proses perintah yang diterima dan kirim ke Serial USB untuk Webots
void process_perintah(const uint8_t* data, int len, int index_mac_address_asal) { 
    if (len > 0) {
        char key = (char)data[0];  // Mengkonversi byte pertama data menjadi karakter
        Serial.write(key);  // Menulis karakter ke Serial USB untuk dikirim ke Webots
        Serial.flush();  // Memastikan semua data terkirim dengan mem-flush buffer Serial
        
        Serial.printf("[INFO] Perintah '%c' diteruskan ke Serial USB\n", key);  
    } else { 
        Serial.println("[WARNING] Data kosong, diabaikan"); 
    }
}
