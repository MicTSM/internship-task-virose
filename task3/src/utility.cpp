#include "main.h"

esp_now_peer_info_t peer_info;

uint8_t mac_addresses[MAC_ADDRESS_TOTAL][MAC_ADDRESS_LENGTH] = {
    {0x24, 0x0A, 0xC4, 0x0A, 0x21, 0x11},  // BISMA
    {0x24, 0x0A, 0xC4, 0x0A, 0x21, 0x10},  // JSON
    {0x24, 0x0A, 0xC4, 0x0A, 0x20, 0x11},  // FARUG
    {0x24, 0x0A, 0xC4, 0x0A, 0x10, 0x10},  // Fauzan Firdaus
    {0x24, 0x0A, 0xC4, 0x0A, 0x10, 0x11},  // Africha Sekar wangi
    {0x24, 0x0A, 0xC4, 0x0A, 0x11, 0x10},  // Rafaina Erin Sadia
    {0x24, 0x0A, 0xC4, 0x0A, 0x11, 0x11},  // Antonius Michael Yordanis Hartono
    {0x24, 0x0A, 0xC4, 0x0A, 0x12, 0x10},  // Dinda Sofi Azzahro
    {0x24, 0x0A, 0xC4, 0x0A, 0x12, 0x11},  // Muhammad Fahmi Ilmi
    {0x24, 0x0A, 0xC4, 0x0A, 0x13, 0x10},  // Dhanishara Zaschya Putri Syamsudin
    {0x24, 0x0A, 0xC4, 0x0A, 0x13, 0x11},  // Irsa Fairuza
    {0x24, 0x0A, 0xC4, 0x0A, 0x14, 0x10},  // Revalinda Bunga Nayla Laksono

};

const char *mac_names[MAC_ADDRESS_TOTAL] = {
    "BISMA",                               // 0
    "JSON",                                // 1
    "FARUG",                               // 2
    "Fauzan Firdaus",                      // 3
    "Africha Sekar Wangi",                 // 4
    "Rafaina Erin Sadia",                  // 5
    "Antonius Michael Yordanis Hartono",   // 6
    "Dinda Sofi Azzahro",                  // 7
    "Muhammad Fahmi Ilmi",                 // 8
    "Dhanishara Zaschya Putri Syamsudin",  // 9
    "Irsa Fairuza",                        // 10
    "Revalinda Bunga Nayla Laksono",       // 11
};

esp_err_t mulai_esp_now(int index_mac_address) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    /* Init ESP-NOW */
    esp_err_t result = esp_now_init();
    if (result != ESP_OK)
        return result;

    /* Set callback function to handle received data */
    result = esp_now_register_recv_cb(callback_data_esp_now);
    if (result != ESP_OK)
        return result;

    result = esp_now_register_send_cb(callback_pengiriman_esp_now);
    //     if (result != ESP_OK)
    //         return result;

    /* Set MAC Address */
    uint8_t mac[MAC_ADDRESS_LENGTH];
    memcpy(mac, mac_addresses[index_mac_address], MAC_ADDRESS_LENGTH);
    result = esp_wifi_set_mac(WIFI_IF_STA, mac);
    if (result != ESP_OK)
        return result;

    /* Initialize peer_info and set fields*/
    memset(&peer_info, 0, sizeof(esp_now_peer_info_t));
    peer_info.channel = 0;
    peer_info.encrypt = false;

    /* Add All MAC to peer list  */
    for (int i = 0; i < MAC_ADDRESS_TOTAL; i++) {
        memcpy(peer_info.peer_addr, mac_addresses[i], MAC_ADDRESS_LENGTH);
        result = esp_now_add_peer(&peer_info);
        if (result != ESP_OK)
            return result;
    }

    return ESP_OK;
}

int cari_mac_index(const uint8_t *mac) {
    for (int i = 0; i < MAC_ADDRESS_TOTAL; i++) {
        // Compare the MAC address
        if (memcmp(mac, mac_addresses[i], MAC_ADDRESS_LENGTH) == 0)
            return i;
    }

    // if not found return -1
    return -1;
}

String mac_index_to_names(int mac_index) {
    if ((mac_index < 0 || mac_index >= MAC_ADDRESS_TOTAL) || (mac_index == -1)) {
        return "Unknown";
    }
    return mac_names[mac_index];
}

void callback_data_esp_now(const uint8_t *mac, const uint8_t *data, int len) {
    int index_mac_asal = cari_mac_index(mac);
    process_perintah(data, len, index_mac_asal);
}
void callback_pengiriman_esp_now(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.printf("\nStatus pengiriman ESP-NOW: %s\n", esp_err_to_name(status));
}
void callback_data_serial(const uint8_t *data, int len) {
    process_perintah(data, len);
}
    
void baca_serial(void (*callback)(const uint8_t *data, int len)) {
    // TODO 2: implementasi kode buat nerima perintah dari serial
    // pastikan ada minimal 4 byte: header (3) + length (1)
    if (Serial.available() < 4) return;

    // cek header 0xFF 0xFF 0x00
    if (Serial.read() != 0xFF) return;
    if (Serial.read() != 0xFF) return;
    if (Serial.read() != 0x00) return;

    // ambil panjang data
    int panjang = Serial.read();
    if (panjang <= 0 || panjang > 50) return;

    // tunggu hingga semua byte data tersedia (dengan timeout agar tak hang)
    unsigned long start = millis();
    while (Serial.available() < panjang && millis() - start < 200) {
        delay(1);
    }
    if (Serial.available() < panjang) return;  // timeout, batal

    uint8_t buffer[50];
    Serial.readBytes(buffer, panjang);

    if (callback) callback(buffer, panjang);
}    
void process_perintah(const uint8_t* data, int len, int index_mac_asal) {
    // TODO 3: implementasi kode buat processing perintah
    if (!data || len < 2) return;

    uint8_t cmd = data[0];
    uint8_t tujuan = data[1];

    String teks = "";
    for (int i = 2; i < len; i++) teks += char(data[i]);

    String namaAsal   = (index_mac_asal < 0) ? "Laptop" : mac_index_to_names(index_mac_asal);
    String namaTujuan = mac_index_to_names(tujuan);
    String namaKu     = mac_index_to_names(mac_index_ku);

    uint8_t pkt[64];

    switch (cmd) {
        case HALO:  // 0x00
            if (index_mac_asal < 0) {
                String msg = "Halo Wahai " + namaTujuan + " Panggil Aku " + namaKu;
                pkt[0] = HALO; pkt[1] = tujuan;
                memcpy(pkt + 2, msg.c_str(), msg.length());
                esp_now_send(mac_addresses[tujuan], pkt, 2 + msg.length());
                Serial.println("[Serial->ESP] " + msg);
            } else {
                String reply = "Halo Juga Bung " + namaAsal + " Call me " + namaKu;
                pkt[0] = JAWAB; pkt[1] = index_mac_asal;
                memcpy(pkt + 2, reply.c_str(), reply.length());
                esp_now_send(mac_addresses[index_mac_asal], pkt, 2 + reply.length());
                Serial.println("[ESP->Balasan] " + reply);
            }
            break;

        case CEK:   // 0x01
            if (index_mac_asal < 0) {
                String msg = namaTujuan + " I am " + namaKu + " apa kamu selamat dari ETS?";
                pkt[0] = CEK; pkt[1] = tujuan;
                memcpy(pkt + 2, msg.c_str(), msg.length());
                esp_now_send(mac_addresses[tujuan], pkt, 2 + msg.length());
                Serial.println("[Serial->ESP] " + msg);
            } else {
                String reply = "Iya Aku " + namaAsal + " Disini Raja " + namaKu;
                pkt[0] = JAWAB; pkt[1] = index_mac_asal;
                memcpy(pkt + 2, reply.c_str(), reply.length());
                esp_now_send(mac_addresses[index_mac_asal], pkt, 2 + reply.length());
                Serial.println("[ESP->Balasan] " + reply);
            }
            break;

        case JAWAB: // 0x02
            if (index_mac_asal >= 0) {
                Serial.print("[JAWAB dari ");
                Serial.print(namaAsal);
                Serial.print("] ");
                Serial.println(teks);
            }
            break;

        default:
            Serial.printf("[CMD tidak dikenal] 0x%02X\n", cmd);
            break;
    }
}
