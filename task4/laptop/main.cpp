// Program Laptop - File Transfer ESP-NOW
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <serial/serial.h>

using namespace std;

// Class untuk handle file dan fragmentasi
class FileHandler {
private:
    string filename;
    vector<char> binaryContent;
    const int PACKET_SIZE = 200; // 200 bytes data + 50 bytes header = 250 total
    
public:
    FileHandler(string file) : filename(file) {}
    
    // Load file dalam mode binary
    bool loadFile() {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr << "Error: Tidak dapat membuka file " << filename << endl;
            return false;
        }
        
        // Dapatkan ukuran file
        file.seekg(0, ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, ios::beg);
        
        // Baca file ke memory
        binaryContent.resize(fileSize);
        file.read(binaryContent.data(), fileSize);
        file.close();
        
        cout << "File dimuat: " << fileSize << " bytes" << endl;
        return true;
    }
    
    // Pecah file menjadi paket-paket
    vector<vector<char>> fragmentFile() {
        vector<vector<char>> fragments;
        
        for (size_t i = 0; i < binaryContent.size(); i += PACKET_SIZE) {
            size_t chunkSize = min((size_t)PACKET_SIZE, binaryContent.size() - i);
            vector<char> chunk(binaryContent.begin() + i, binaryContent.begin() + i + chunkSize);
            fragments.push_back(chunk);
        }
        
        cout << "File dipecah menjadi " << fragments.size() << " paket" << endl;
        return fragments;
    }
};

// Class untuk handle pengiriman via serial
class SerialSender {
private:
    serial::Serial serialPort;
    
    // Hitung checksum untuk validasi data (cek data tidak rusak)
    string calculateChecksum(const vector<char>& data) {
        unsigned long hash = 0;
        
        // Loop setiap byte, buat hash unik dari data
        for (char c : data) {
            hash = hash * 31 + (unsigned char)c;
        }
        
        // Convert angka ke hexadecimal string
        char hex[20];
        sprintf(hex, "%lx", hash);
        return string(hex);
    }
    
public:
    SerialSender() {}
    
    // Connect ke serial port
    bool connect(const string& port) {
        try {
            serialPort.setPort(port);
            serialPort.setBaudrate(115200);
            serial::Timeout timeout = serial::Timeout::simpleTimeout(1000);
            serialPort.setTimeout(timeout);
            serialPort.open();
            
            if (serialPort.isOpen()) {
                cout << "Terhubung ke " << port << endl;
                return true;
            }
        } catch (serial::IOException& e) {
            cerr << "Error: " << e.what() << endl;
        }
        return false;
    }
    
    // Kirim semua paket ke ESP32-Bridge
    void sendPackets(vector<vector<char>>& fragments) {
        if (!serialPort.isOpen()) {
            cerr << "Error: Port serial tidak terbuka" << endl;
            return;
        }
        
        cout << "\nMengirim " << fragments.size() << " paket..." << endl;
        
        for (size_t i = 0; i < fragments.size(); i++) {
            // Buat header: id|total|length|checksum|
            string header = to_string(i + 1) + "|" + 
                          to_string(fragments.size()) + "|" + 
                          to_string(fragments[i].size()) + "|" +
                          calculateChecksum(fragments[i]) + "|";
            
            // Gabungkan header + data
            string packet = header + string(fragments[i].begin(), fragments[i].end());
            packet += "\n";
            
            // Kirim via serial
            serialPort.write(packet);
            
            cout << "  Paket " << (i + 1) << "/" << fragments.size() << " terkirim" << endl;
        }
        
        cout << "Semua paket berhasil dikirim" << endl;
    }
};

int main() {
    cout << "Sistem Transfer File - ESP-NOW" << endl;
    cout << "===============================" << endl;
    
    // Load file data.json
    FileHandler handler("data.json");
    if (!handler.loadFile()) {
        return 1;
    }
    
    // Pecah file jadi paket-paket
    vector<vector<char>> fragments = handler.fragmentFile();
    
    // Setup serial communication
    SerialSender sender;
    
    cout << "\nMasukkan port COM (contoh: COM3): ";
    string port;
    cin >> port;
    
    // Connect ke ESP32-Bridge
    if (!sender.connect(port)) {
        return 1;
    }
    
    // Kirim semua paket
    sender.sendPackets(fragments);
    
    cout << "\nProgram selesai" << endl;
    return 0;
}
