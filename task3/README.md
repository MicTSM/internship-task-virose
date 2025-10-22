# Task 3 - Komunikasi ESP-NOW

Dokumentasi implementasi untuk tugas ESP-NOW pada modul ESP-Programming.

## Deskripsi Proyek

Proyek ini mengimplementasikan komunikasi antar ESP32 menggunakan protokol ESP-NOW. Kode yang dibuat fokus pada pengisian fungsi `baca_serial()` dan `process_perintah()` di file `utility.cpp` sesuai dengan TODO yang diberikan, tanpa mengubah struktur file lainnya.

## Fitur Utama

1. **Identifikasi Board** - Mencetak nama board berdasarkan MAC address yang telah dikonfigurasi
2. **Pembacaan Data Serial** - Menerima perintah dari laptop melalui komunikasi serial dengan format paket biner
3. **Pemrosesan Perintah** - Menangani tiga jenis perintah (HALO, CEK, JAWAB) baik dari serial maupun ESP-NOW

## Struktur File

- `src/main.cpp` - Program utama (tidak dimodifikasi)
- `src/utility.cpp` - Implementasi fungsi komunikasi (TODO 2 dan 3)
- `src/main.h` - Header file dengan definisi enum dan deklarasi fungsi

## Format Komunikasi Serial

Struktur paket yang dikirim dari laptop ke ESP32:

```
HEADER   LENGTH   DATA
3 byte   1 byte   N byte
```

Komponen paket:

- **HEADER**: `0xFF 0xFF 0x00` (penanda awal paket)
- **LENGTH**: Jumlah byte pada bagian DATA
- **DATA**: Payload berisi perintah

Format DATA:

- `[CMD][TUJUAN]` untuk perintah dari Serial
- `[CMD][TUJUAN][STRING...]` untuk data ESP-NOW

Definisi Command:
| CMD | Fungsi | Keterangan |
|-----|--------|------------|
| `0x00` | HALO | Mengirim sapaan |
| `0x01` | CEK | Mengirim pengecekan |
| `0x02` | JAWAB | Balasan dari ESP lain |

## Implementasi

### TODO 1: Mencetak Identitas Board

Bagian ini sudah tersedia di template `main.cpp`:

```cpp
Serial.println(mac_index_to_names(mac_index_ku));
```

Fungsi ini mencetak nama board sesuai dengan index MAC yang dikonfigurasi pada variabel `mac_index_ku`.

### TODO 2: Fungsi `baca_serial()`

```cpp
void baca_serial(void (*callback)(const uint8_t *data, int len)) {
    if (Serial.available() < 4) return;
    if (Serial.read() != 0xFF) return;
    if (Serial.read() != 0xFF) return;
    if (Serial.read() != 0x00) return;

    int panjang = Serial.read();
    if (panjang <= 0 || panjang > 50) return;

    unsigned long start = millis();
    while (Serial.available() < panjang && millis() - start < 200) {
        delay(1);
    }
    if (Serial.available() < panjang) return;

    uint8_t buffer[50];
    Serial.readBytes(buffer, panjang);
    if (callback) callback(buffer, panjang);
}
```

Cara kerja:

1. Validasi minimal 4 byte tersedia (header + length)
2. Verifikasi header `0xFF 0xFF 0x00`
3. Baca panjang data dan validasi (maksimal 50 byte)
4. Tunggu hingga semua data tersedia dengan timeout 200ms untuk menghindari blocking
5. Baca data ke buffer dan panggil callback untuk pemrosesan lebih lanjut

### TODO 3: Fungsi `process_perintah()`

```cpp
void process_perintah(const uint8_t* data, int len, int index_mac_asal) {
    if (!data || len < 2) return;

    uint8_t cmd = data[0];
    uint8_t tujuan = data[1];

    String teks = "";
    for (int i = 2; i < len; i++) teks += char(data[i]);

    String namaAsal   = (index_mac_asal < 0) ? "Laptop" : mac_index_to_names(index_mac_asal);
    String namaTujuan = mac_index_to_names(tujuan);
    String namaKu     = mac_index_to_names(mac_index_ku);

    uint8_t pkt[100];

    switch (cmd) {
        case HALO:
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

        case CEK:
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

        case JAWAB:
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
```

Logika pemrosesan:

- Parameter `index_mac_asal` bernilai `-1` jika perintah berasal dari Serial (laptop), atau bernilai index MAC jika dari ESP-NOW
- Untuk perintah HALO dan CEK dari Serial: membuat pesan dan mengirimkan via ESP-NOW ke board tujuan
- Untuk perintah HALO dan CEK dari ESP-NOW: membalas dengan perintah JAWAB ke pengirim
- Untuk perintah JAWAB: mencetak pesan balasan ke Serial Monitor
- Buffer paket menggunakan ukuran 100 byte untuk menampung pesan yang lebih panjang

## Alur Komunikasi

Berikut alur komunikasi lengkap sistem:

1. Laptop mengirim paket serial ke board A
2. Board A menerima via `baca_serial()` yang memanggil `process_perintah(data, len, -1)`
3. Board A mengirim perintah ke board B melalui ESP-NOW
4. Board B menerima via callback ESP-NOW yang memanggil `process_perintah(data, len, index_A)`
5. Board B membalas dengan perintah JAWAB ke board A
6. Board A menampilkan balasan di Serial Monitor

## Pengujian

Contoh data HEX untuk testing melalui Serial:

| Aksi            | Data (HEX)          | Keterangan                                          |
| --------------- | ------------------- | --------------------------------------------------- |
| HALO ke index 1 | `FF FF 00 02 00 01` | Mengirim sapaan ke board dengan index 1 (JSON)      |
| CEK ke index 0  | `FF FF 00 02 01 00` | Mengirim pengecekan ke board dengan index 0 (BISMA) |

Contoh output di Serial Monitor:

```
[Serial->ESP] Halo Wahai JSON Panggil Aku Muhammad Fahmi Ilmi
[ESP->Balasan] Halo Juga Bung BISMA Call me Muhammad Fahmi Ilmi
[JAWAB dari BISMA] Iya Aku BISMA Disini Raja Muhammad Fahmi Ilmi
```

## Konfigurasi Hardware

Spesifikasi yang digunakan:

- Board: ESP32 Dev Module
- Baudrate: 115200
- Protocol: ESP-NOW

Pastikan variabel `mac_index_ku` di `main.cpp` sudah diatur sesuai dengan index MAC address yang telah ditentukan.

## Troubleshooting

Beberapa masalah umum dan solusinya:

**Tidak ada respons dari board**

- Periksa header paket sudah benar (`FF FF 00`)
- Pastikan mengirim data dalam format biner, bukan ASCII
- Cek koneksi serial dan baudrate

**Error "Send FAIL" pada ESP-NOW**

- Peer belum ditambahkan ke daftar peer
- MAC address tujuan salah atau tidak valid
- Board tujuan belum diinisialisasi

**Tidak menerima balasan**

- Board tujuan mungkin belum aktif atau tidak terhubung
- Periksa konfigurasi MAC address di kedua board

## Catatan

Implementasi ini mengikuti template yang diberikan dari modul pembelajaran. Kode fokus pada pengisian fungsi `baca_serial()` dan `process_perintah()` tanpa mengubah struktur file lainnya.
