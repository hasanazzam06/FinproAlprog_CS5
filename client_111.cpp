// Final project Algoritma Pemrograman
// Case Study 5
// project Arduino-Based RFID Reader: Real-Time Intelligent Acquisition System untuk Data Kehadiran Lab 

/*
==============================================================
 Sistem Pencatatan Kehadiran Lab - CLIENT (dengan Arduino RFID)
==============================================================

Deskripsi:
Program client ini melakukan dua tugas utama:
1. Berinteraksi dengan server melalui socket TCP untuk mengirim perintah,
   seperti mencatat kehadiran, mencari log, atau mengekspor data.
2. Terhubung ke Arduino RFID melalui port serial untuk membaca UID kartu
   RFID secara otomatis, lalu mengirimkannya sebagai log ke server dalam
   format perintah ADD_LOG.

Fitur Utama:
- Koneksi TCP ke server (Winsock)
- Pembacaan UID dari Arduino RFID melalui COM port
- Thread terpisah untuk membaca UID secara paralel
- Format log: "ADD_LOG <UID> <timestamp>"
- Timestamp otomatis saat menerima UID
- Interaksi user untuk perintah manual ke server
- Mendukung multiclient (multithreaded server-side)
*/


#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <atomic>
#include <mutex>

using namespace std;

#define SERVER "127.0.0.1"
#define PORT 8888

// ID khusus setiap client, digunakan sebagai identifikasi unik saat terkoneksi ke server.
string ID_CLIENT = "111"; // Pastikan tidak duplikat antar client.

mutex ioMutex; // Untuk mengamankan akses ke terminal (cout & cin)

// Fungsi untuk mendapatkan timestamp saat ini dalam format:
// YYYY-MM-DD->HH:MM:SS
string getCurrentTime() {
    time_t now = time(0);
    tm* localTime = localtime(&now);
    stringstream ss;
    ss << put_time(localTime, "%Y-%m-%d->%H:%M:%S");
    return ss.str();
}

// Fungsi konversi string menjadi huruf kapital seluruhnya.
string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// =======================
// Class ClientProgram
// Program utama sisi client yang bertugas:
// - Membuat koneksi ke server melalui socket
// - Membaca data UID dari Arduino melalui serial port
// - Mengirim request ke server dan menerima respon
// =======================
class ClientProgram {
private:
	// Komponen koneksi socket
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server;
    int recv_size;
    char messageRecv[1000];
    string messageSend;

	// Komponen pembacaan RFID
    atomic<bool> keepReading;
    thread readerThread;
    HANDLE hSerial = INVALID_HANDLE_VALUE;

public:
	// Fungsi untuk membuka koneksi ke port serial Arduino.
    // Dikonfigurasi pada COM8 dengan baudrate 9600.(Ganti tergantung penggunaan user)
    HANDLE openSerialPort(const char* portName) {
        HANDLE handle = CreateFileA(portName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (handle == INVALID_HANDLE_VALUE) {
            lock_guard<mutex> lock(ioMutex);
            cerr << "Gagal membuka port serial " << portName << "\n";
            return handle;
        }

        DCB dcb = { 0 };
        dcb.DCBlength = sizeof(dcb);
        if (!GetCommState(handle, &dcb)) {
            lock_guard<mutex> lock(ioMutex);
            cerr << "Gagal mendapatkan state port serial\n";
            CloseHandle(handle);
            return INVALID_HANDLE_VALUE;
        }

        dcb.BaudRate = CBR_9600;
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity   = NOPARITY;

        if (!SetCommState(handle, &dcb)) {
            lock_guard<mutex> lock(ioMutex);
            cerr << "Gagal mengatur state port serial\n";
            CloseHandle(handle);
            return INVALID_HANDLE_VALUE;
        }

        COMMTIMEOUTS timeouts = { 0 };
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        if (!SetCommTimeouts(handle, &timeouts)) {
            lock_guard<mutex> lock(ioMutex);
            cerr << "Gagal mengatur timeout port serial\n";
            CloseHandle(handle);
            return INVALID_HANDLE_VALUE;
        }

        return handle;
    }

	// Fungsi yang berjalan dalam thread terpisah.
    // Membaca UID dari Arduino, kemudian mengirimkan ke server dalam format:
    // "ADD_LOG <uid> <timestamp>"
    void readUIDFromArduino(SOCKET server_socket) {
        const char* portName = "COM8"; // Ubah sesuai port Arduino yg digunakkan
        hSerial = openSerialPort(portName);
        
        if (hSerial == INVALID_HANDLE_VALUE) return;

        char buffer[256];
        DWORD bytesRead;

        while (keepReading) {
            if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    string uid = string(buffer);
                    uid.erase(remove(uid.begin(), uid.end(), '\r'), uid.end());
                    uid.erase(remove(uid.begin(), uid.end(), '\n'), uid.end());

                    if (!uid.empty()) {
                    	// jika ada data dari arduino kirim ke server untuk diporses
                        string messageSend = "ADD_LOG " + uid + " " + getCurrentTime();
                        send(server_socket, messageSend.c_str(), messageSend.length(), 0);
                    }
                }
            }
            Sleep(300);
        }

        CloseHandle(hSerial);
    }

	// Fungsi utama inisialisasi socket client.
    // Mengatur Winsock, membuat socket, dan connect ke server.
    // Setelah itu memulai thread pembacaan UID dan masuk ke proses utama.
    void clientInitial() {
        {
            lock_guard<mutex> lock(ioMutex);
            cout << "Memulai Winsock...\n";
        }

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            lock_guard<mutex> lock(ioMutex);
            cerr << "WSAStartup gagal. Error code: " << WSAGetLastError() << endl;
            return;
        }

        {
            lock_guard<mutex> lock(ioMutex);
            cout << "Winsock terinstall.\n";
        }

        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == INVALID_SOCKET) {
            lock_guard<mutex> lock(ioMutex);
            cerr << "Gagal membuat socket. Error code: " << WSAGetLastError() << endl;
            WSACleanup();
            return;
        }

        {
            lock_guard<mutex> lock(ioMutex);
            cout << "Socket berhasil dibuat.\n";
        }

        server.sin_addr.s_addr = inet_addr(SERVER);
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);

        if (connect(client_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
            lock_guard<mutex> lock(ioMutex);
            cerr << "Koneksi gagal. Error code: " << WSAGetLastError() << endl;
            closesocket(client_socket);
            WSACleanup();
            return;
        }

        {
            lock_guard<mutex> lock(ioMutex);
            cout << "\nTerhubung ke server.\n";
        }

		// Kirim ID client saat pertama terkoneksi.
        send(client_socket, ID_CLIENT.c_str(), ID_CLIENT.length(), 0);

		// Mulai thread untuk baca UID dari Arduino.
        keepReading = true;
        readerThread = thread(&ClientProgram::readUIDFromArduino, this, client_socket);
		
		 // Masuk ke loop komunikasi utama dengan server.
        clientMainProses();

		// Cleanup setelah keluar dari loop
        keepReading = false;
        if (readerThread.joinable()) readerThread.join();

        closesocket(client_socket);
        WSACleanup();
    }

	// Fungsi komunikasi utama client dengan server.
    // Menerima input dari user, mengirim ke server, dan menampilkan respon.
    void clientMainProses() {
    	recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
			
		if(recv_size == SOCKET_ERROR){
	        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
	    }else {
		       	messageRecv[recv_size] = '\0';
		}
		   	
		cout << "Pesan dari server: \n" << messageRecv <<endl;
		   	
        while (true) {
            {
                lock_guard<mutex> lock(ioMutex);
                cout << "----------------------------------\n";
                cout << "Request: ";
            }

            getline(cin, messageSend);
			if (messageSend == "\0"){
				continue;
			}
			
            messageSend = toUpper(messageSend);

            send(client_socket, messageSend.c_str(), messageSend.length(), 0);

            recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
            if (recv_size == SOCKET_ERROR) {
                lock_guard<mutex> lock(ioMutex);
                cout << "Gagal menerima pesan. Error code: " << WSAGetLastError() << endl;
                break;
            }

            messageRecv[recv_size] = '\0';

            {
                lock_guard<mutex> lock(ioMutex);
                cout << "Pesan dari server: \n" << messageRecv << endl;
            }

            if (messageSend == "SHUTDOWN") break;
        }
    }
};

// Fungsi utama program Client
// Inisialisasi sistem dan menjalankan loop utama
int main() {
	// Inisialisasi objek utama client
    ClientProgram Cp;
    
    // Menjalankan proses client (koneksi ke server dan pembacaan dari Arduino)
    Cp.clientInitial();
    return 0;
}