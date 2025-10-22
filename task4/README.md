# File Transfer ESP-NOW

Program untuk transfer file JSON dari laptop ke ESP32 menggunakan protokol ESP-NOW. File dipecah jadi paket-paket kecil, dikirim via serial ke ESP32-Bridge, terus diteruskan ke ESP32-Receiver via ESP-NOW. Di receiver, paket-paket disusun ulang dan disimpan ke SPIFFS.

## Struktur Project

```
task4/
├── laptop/              # Program laptop (C++)
│   ├── data.json        # File input
│   ├── CMakeLists.txt   # Build config
│   ├── main.cpp         # Source code
│   └── build/           # Hasil build
│
├── bridge/              # ESP32-Bridge
│   └── src/
│       ├── esp32_bridge.cpp
│       └── esp32_bridge.h
│
└── receiver/            # ESP32-Receiver
    └── src/
        ├── esp32_receiver.cpp
        └── esp32_receiver.h
```

## Cara Kerja

1. Program laptop baca `data.json` dalam mode binary
2. File dipecah jadi paket-paket (200 bytes data + 50 bytes header)
3. Kirim paket ke ESP32-Bridge lewat serial (USB)
4. ESP32-Bridge meneruskan paket via ESP-NOW ke Receiver
5. ESP32-Receiver susun ulang semua paket
6. Simpan file lengkap ke SPIFFS
7. Tampilkan isi file di serial monitor

> **Note:** ESP-NOW hanya bisa kirim max 250 bytes per paket, jadi data dipecah dulu.

## Requirements

### Laptop

- CMake >= 3.15
- C++ Compiler (MSVC/GCC/Clang)
- Git (to download serial library)

### ESP32

- PlatformIO or Arduino IDE
- ArduinoJson library (for receiver)

## Build & Run

### 1. Build Program Laptop

```bash
cd laptop

# Download library serial
git clone https://github.com/wjwwood/serial.git

# Build
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

File executable: `laptop/build/Release/task4.exe` (Windows)

### 2. Jalankan Program

**Pastikan ESP32-Bridge sudah disambungkan via USB!**

```bash
# Copy data.json dulu
copy laptop\data.json laptop\build\Release\data.json

# Jalankan
cd laptop\build\Release
.\task4.exe
```

Nanti program akan:

- Load file `data.json`
- Pecah jadi paket-paket
- Minta input port COM (cth: COM5)
- Kirim semua paket ke ESP32

## Upload ESP32

### PlatformIO

```bash
# Upload Receiver dulu
cd receiver
pio run --target upload

# Upload Bridge
cd bridge
pio run --target upload
```

### Konfigurasi MAC Address

**Sangat Penting!** Sebelum upload Bridge, harus mengetahui MAC address Receiver dulu.

**Cara:**

1. Upload Receiver, buka serial monitor (115200 baud)
2. Catat MAC address yang muncul, contoh: `30:C9:22:32:C8:70`
3. Edit `bridge/src/esp32_bridge.cpp` baris 4:
   ```cpp
   uint8_t receiverMac[] = {0x30, 0xC9, 0x22, 0x32, 0xC8, 0x70};
   ```
4. Upload ulang Bridge

## Format Data

### File JSON (`data.json`)

```json
{
  "nama": "Antonius Michael Yordanis Hartono",
  "jurusan": "Sains Data",
  "umur": 18,
  "deskripsi": "Saya adalah orang yang ontime dan suka membantu..."
}
```

### Protokol Paket

Format: `id|total|length|checksum|data`

Contoh:

```
1|2|200|a3f5d2|{"nama":"Antonius...
2|2|125|b8e9c1|...Hartono"}
```

- **id**: nomor paket (mulai dari 1)
- **total**: jumlah total paket
- **length**: panjang data (bytes)
- **checksum**: hash untuk validasi data
- **data**: isi file (binary)

### Output di Serial Monitor

```
[KONTEN FILE YANG DITERIMA]
NAMA: Antonius Michael Yordanis Hartono
JURUSAN: Sains Data
UMUR: 18
DESKRIPSI DIRI: Saya adalah orang yang ontime dan suka membantu...
```

## Fitur yang Diimplementasi

### Program Laptop

- **OOP**: 2 class (FileHandler untuk baca file, SerialSender untuk kirim data)
- **CMakeLists**: build system menggunakan CMake + library eksternal
- **Serial Communication**: menggunakan library `wjwwood/serial` untuk komunikasi serial real
- **Binary Mode**: baca file dalam mode binary agar data tidak rusak
- **Fragmentasi**: pecah file jadi 200 bytes per paket
- **Checksum**: validasi data menggunakan simple hash

### Program ESP32

- **Header File**: pisah .h dan .cpp agar rapi
- **Protokol Custom**: paket memiliki struktur `id|total|length|checksum|data`
- **ESP-NOW**: komunikasi wireless tanpa WiFi router
- **SPIFFS**: simpan file di flash memory ESP32
- **Reassembly**: susun ulang paket jadi file lengkap

## Troubleshooting

### Build Error

Kalau build gagal, coba clean rebuild:

```bash
cd laptop
rm -rf build serial
git clone https://github.com/wjwwood/serial.git
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

**Windows:** Pastikan Visual Studio + C++ tools sudah terinstall.

### ESP32 Tidak Ter-detect

- Cabut-sambung ulang USB
- Cek driver (CP2102 / CH340)
- Tekan tombol BOOT saat upload
- Cek di Device Manager (Ports)

### Data Tidak Lengkap / Rusak

- Pastikan MAC address Bridge sudah benar
- Cek baud rate 115200
- ESP32 jangan terlalu jauh (max ~100m)
- Restart kedua ESP32

## Catatan Teknis

- Baud rate: **115200**
- ESP-NOW limit: **250 bytes** (hardware)
- Ukuran paket data: **200 bytes** (250 - 50 header)
- Mode baca file: **binary** (`rb`)
- Mode tulis SPIFFS: **binary** (`wb`)
- Range ESP-NOW: ~100 meter

### Kenapa Pakai Binary Mode?

Agar data tidak rusak saat transfer, karena:

- Transfer byte-by-byte exact
- Tidak ada encoding issue
- Bisa support file apa saja (tidak hanya text)
- File bisa disusun ulang dengan benar

---
