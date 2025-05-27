#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <winsock2.h>
#include <fstream>
#include "../json.hpp"

using namespace std;
using json = nlohmann::json;

#define SERVER "127.0.0.1"
#define PORT 8888

//done
string getCurrentTime() {
    time_t now = time(0); // ambil waktu sekarang dalam format epoch (detik)
    tm* localTime = localtime(&now); // ubah jadi struct waktu lokal

    stringstream ss;
    ss << put_time(localTime, "%Y-%m-%d %H:%M:%S"); // format waktu jadi string
    return ss.str();
}

//done
string getDate() {
    time_t now = time(0); // ambil waktu sekarang dalam format epoch (detik)
    tm* localTime = localtime(&now); // ubah jadi struct waktu lokal

    stringstream ss;
    ss << put_time(localTime, "%Y_%m_%d"); // format waktu jadi string
    return ss.str();
}


//pindah ke server
void saveLogsToJSON(const string& name, const string& timestamp, const string& action, const string& filename) {
    json entry;
    entry["name"] = name;
    entry["timestamp"] = timestamp;
    entry["action"] = action;

    // Cek apakah file sudah ada dan load jika ya
    json fullData;
    ifstream inFile(filename);
    if (inFile.is_open()) {
        try {
            inFile >> fullData;
        } catch (...) {
            // Kosongkan jika file rusak/format salah
            fullData = json::array();
        }
        inFile.close();
    }

    // Tambahkan entry baru
    fullData.push_back(entry);

    // Simpan kembali ke file
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << fullData.dump(4); // indented 4 spasi
        outFile.close();
        cout << "Data tersimpan di file " << filename << endl;
    } else {
        cout << "Gagal menulis ke file " << filename << endl;
    }
}


// dijaiin class 
int main() {
	
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server;
    int recv_size;
    char messageRecv[100];
    string messageSend,phars = "0";
    char buffer[2048];
    
    cout << "Memulai Winsock...\n";
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "WSAStartup gagal. Error code: " << WSAGetLastError() <<endl;
        return 1;
    }
    
    cout << "Winsock terisntall."<<endl;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        cerr << "gagal membuat socket. Error code: " << WSAGetLastError() <<endl;
        WSACleanup();
        return 1;
    }
    cout << "Socket berhasil dibuat."<<endl;

    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        cerr << "Connection gagal. Error code: " << WSAGetLastError() <<endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    
    cout << "terhubung ke server.\n"<<endl;




// dari server cm kirirm message biar gak perlu kondisi pilihan
    bool status = 1;
    while(status){
        int pilihan;
    	cout << "\n[ Menu Admin Log Viewer ]\n";
        cout << "1. Memasukan Log\n";
        cout << "2. Tampilkan semua log\n";
        cout << "3. Cari berdasarkan nama\n";
        cout << "4. Donwload Log [JSON]\n";
        cout << "0. Keluar\n";
        cout << "Pilih: ";
        cin >> pilihan;
        cin.ignore();
        
        //ganti pake 1 message dari server
        switch (pilihan) {
            case 1:
                while (true) {
                    string rfid;
                    cout << "Scan RFID: ";
                    getline(cin, rfid);
                    if (rfid == "exit") {
                        cout << "Keluar dari mode scan.\n";
                        break;
                    } else {
                        messageSend = rfid + " " + getCurrentTime();
                        send(client_socket, messageSend.c_str(), messageSend.size(), 0);

                        memset(messageRecv, 0, sizeof(messageRecv));
                        if ((recv_size = recv(client_socket, messageRecv, 100, 0)) == SOCKET_ERROR) {
                            cout << "Receive failed: " << WSAGetLastError() << endl;
                        } else {
                            messageRecv[recv_size] = '\0';
                            cout << "Result : " << messageRecv << endl;
                        }
                    }
                }
                break;
            case 2:
                messageSend = "SHOW_LOG 1";
                send(client_socket, messageSend.c_str(), messageSend.size(), 0);
                char buffer[1024];
                while (true) {
                    int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (bytesReceived <= 0) {
                        cout << "Koneksi terputus atau error.\n";
                        break;
                    }
                    buffer[bytesReceived] = '\0'; // null terminator

                    string data(buffer);

                    if (data == "#END") {
                        cout << "[Info] Semua log diterima.\n";
                        break; // keluar dari loop karena server sudah selesai kirim
                    }

                    cout << "[LOG] " << data << endl;
                }
                break;
            case 3: {
                string name;
                cout << "Keyword : ";
                getline(cin, name);
                messageSend = "SEARCH_LOG " + name;
                send(client_socket, messageSend.c_str(), messageSend.size(), 0);
                char buffer[1024];
                while (true) {
                    int bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (bytesReceived <= 0) {
                        cout << "Koneksi terputus atau error.\n";
                        break;
                    }
                    buffer[bytesReceived] = '\0'; // null terminator

                    string data(buffer);

                    if (data == "#END") {
                        cout << "[Info] Semua log diterima.\n";
                        break; // keluar dari loop karena server sudah selesai kirim
                    }

                    cout << "[LOG] " << data << endl;
                }
                break;
            }
            case 4: {
                messageSend = "DOWNLOAD_LOG 1";
                send(client_socket, messageSend.c_str(), messageSend.size(), 0);
                char buffer[1024];
                bool state = 1;
                while (state) {
                    char buffer[1024];
                    int recv_size = recv(client_socket, buffer, sizeof(buffer), 0);
                    buffer[recv_size] = '\0';
                    if (buffer == "END") {
                        cout << "[Info] Semua log sudah tersimpan.\n";
                        state = 0; // keluar dari loop karena server sudah selesai kirim
                    } else{
                        json received = json::parse(buffer);
                        string name = received["name"];
                        string timestamp = received["timestamp"];
                        string action = received["action"];
                        string filename = "LOG_" + getDate() + ".json";

                        saveLogsToJSON(name, timestamp, action, filename);
                    }
                    
                }
                break;
            }
            case 0: {
                messageSend = "EXIT 1";
                send(client_socket, messageSend.c_str(), messageSend.size(), 0);
                status = 0;
                break;
            }
        }
    }
    closesocket(client_socket);
    WSACleanup();
    
    return 0;
}
