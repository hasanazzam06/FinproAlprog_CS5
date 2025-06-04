// Final project Algoritma Pemrograman
// Case Study 5
// project Arduino-Based RFID Reader: Real-Time Intelligent Acquisition System untuk Data Kehadiran Lab 

/*
=========================================
 Sistem Pencatatan Kehadiran Lab - SERVER
=========================================
Deskripsi:
Server multithreaded ini menggunakan socket TCP berbasis Winsock (Windows)
untuk menerima log kehadiran RFID dari banyak client secara bersamaan.
Setiap koneksi client dilayani dalam thread terpisah agar tidak saling mengganggu.
Data log disimpan ke dalam file biner, bisa diekspor ke JSON, dan dapat dicari
atau ditampilkan sesuai permintaan client.

Fitur Utama:
- Socket TCP port 8888
- Multithreading: setiap client = satu thread
- Penyimpanan log ke file biner dan ekspor JSON
- Pencarian dan pengurutan log kehadiran
- Komunikasi dua arah real-time
*/

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cctype> 
#include <algorithm>
#include <thread>
#include <winsock2.h>
#include "json.hpp"
 
using json = nlohmann::json;
using namespace std;

#define PORT 8888

// Mengubah seluruh huruf dalam string menjadi huruf kapital.
string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}


// Mengembalikan tanggal hari ini dalam format YYYY-MM-DD.
string getDate() {
    time_t now = time(0); // ambil waktu sekarang dalam format epoch (detik)
    tm* localTime = localtime(&now); // ubah jadi struct waktu lokal

    stringstream ss;
    ss << put_time(localTime, "%Y-%m-%d"); // format waktu jadi string
    return ss.str();
}


// Return header string tabel untuk daftar log (RFID, Name, Action, Time).
string templateLog(){
	ostringstream oss;
    		
    oss<<left<<setw(10) << "RFID"
    	<<setw(15) << "Name"
    	<< setw(10) << "Action"
   		<<"Time\n";
   		
    return oss.str();
}

// Return header string tabel untuk daftar database pengguna (RFID, Name).
string templateDatabase(){
	ostringstream oss;
    		
    oss<<left<<setw(10) << "RFID"
    	<<setw(15) << "Name\n";
   		
    return oss.str();
}

// =======================
// Class Log: Mewakili satu entri log kehadiran RFID
// =======================
class Log{
	private:
		//atribut classs
		string RFID;
		string time;
		string name;
		string action;
		
	public:
		
		//method class
		Log() = default; 
		
		Log(string param){
 			stringstream ss(param);
 			getline(ss, RFID,' ');
 			getline(ss, time,' ');
 			getline(ss, action,' ');
 			getline(ss, name);	
		}
		
		//return atribut
		string getRFID() const { return RFID; }
    	string getAction() const { return action; }
    	string getTime() const { return time; }
    	string getName() const { return name;}
    	
    	//return semua atribut
    	string toString() const {
    		ostringstream oss;
    		
    		oss<<left<<setw(10) << RFID
    			<<setw(15) << name
    			<< setw(10) << action
    			<<time<<"\n";
    			
        	return oss.str();
    	}
    	
    	//import fungsi exportdan baca json ke dlam class
    	friend void to_json(json& j, const Log& l);
    	friend void from_json(const json& j, Log& l);
};

// Mengonversi objek Log ke format JSON.
void to_json(json& j, const Log& l){
    j = json{{"rfid", l.RFID}, {"action", l.action}, {"time", l.time}, {"name",l.name}};
}

// Mengonversi data JSON menjadi objek Log.
void from_json(const json& j, Log& l) {
    j.at("rfid").get_to(l.RFID);
    j.at("action").get_to(l.action);
    j.at("time").get_to(l.time);
    j.at("name").get_to(l.name);
}

// =======================
// Class User: Mewakili satu database User RFID
// =======================
class User{
	
	//atribut class
	private:
		string name;
		string RFID;

	public:
		
		//method class mirip sama class log	
		User() = default;
		 
		User(string n, string m){
			name = n;
			RFID = m;
		}
		
		string getName() const{ return name; }
		string getRFID() const{ return RFID; }
		
    	string toString() const {
    		ostringstream oss;
    		
    		oss<<left<<setw(10) << RFID
    			<<setw(15) << name<<"\n";
    			
        	return oss.str();
    	}
		
		friend void to_json(json& j, const User& u);
		friend void from_json(const json& j, User& u);
		
};

// Mengonversi objek User ke format JSON.
void to_json(json& j, const User& u){
    j = json{{"rfid", u.RFID}, {"name", u.name}};
}
// Mengonversi data JSON menjadi objek User.
void from_json(const json& j, User& u) {
    j.at("rfid").get_to(u.RFID);
    j.at("name").get_to(u.name);
}


// =======================
// Class LogManager
// Bertanggung jawab mengelola log kehadiran,
// menambah, menyimpan, mencari, dan menampilkan log.
// =======================
class LogManager{
	
	// Atribut class menyimpan data log, data user, dan mapping untuk mempercepat proses
	private:
		vector<Log> logs;                      // List log aktivitas
		vector<User> datas;                    // List user database
		string ID_client;                      // ID client yang terhubung
		unordered_map<string, string> lastActionByRFID;// Menyimpan aksi terakhir tiap RFID
		unordered_map<string, string> rfidToName;       // Mapping RFID ke nama user
	
	public:
		// Konstruktor class LogManager
		// Inisialisasi ID client, memuat data log dari file biner,
		// memuat aksi terakhir dan data user dari file JSON,
		// dan membuat mapping RFID ke nama user	
		LogManager(string ID){
			ID_client = ID;
			string logFile = "logsBiner"+ID_client+".bin";
			logs = loadFileBinery(logFile.c_str());
			loadAction();
			readFromJSON(datas);
		
			for(const auto& data:datas){
				rfidToName[data.getRFID()] = data.getName();
			}
		}

		// =====================  METHODS UNIVERSAL =====================		
		
		// Memuat aksi terakhir dari setiap RFID ke dalam map lastActionByRFID		
		void loadAction(){
		    for (const auto& log : logs) {
		        lastActionByRFID[log.getRFID()] = log.getAction();
		    }
		}
		
		// Mengurutkan vector log menggunakan metode insertion sort
		void insertionSort(vector<Log>& listLog) {
    		int n = listLog.size();
    
    		for (int i = 1; i < n; i++) {
    	
       		Log key = listLog[i];
        	int j = i - 1;
        
        	while (j >= 0 && listLog[j].getTime() > key.getTime()) {
            	listLog[j + 1] = listLog[j];
            	j--;
        	}
        	listLog[j + 1] = key;
    		}
		}
		
		// Menyimpan data log ke file biner dengan menambahkan (bukan menghapus data sebelumnya) data baru
		int exportBiner(string datasLog, const char nameFile[]){
			
			char temp[100];
			strcpy(temp,datasLog.c_str());
			
			FILE* file = fopen(nameFile, "ab");
			
			if(file){
				fwrite(temp, sizeof(temp),1,file);
				
				fclose(file);
				
				return 1;	
			}
			
			return 0;
		}
		
		// Membaca data log dari file biner dan mengembalikannya sebagai vector<Log>
		vector<Log> loadFileBinery(const char nameFile[]){
			char data[100];
			string param;
			
			vector<Log> readLogs;
 			
 			cout<<"proses pembacaan data ....."<<endl;
 			
			FILE* file = fopen(nameFile, "rb");
			
			if(file){
				
				while(fread(data, sizeof(data),1, file)){
					param = data;
					
					Log readLog(param);
					readLogs.push_back(readLog);
				}
				
				fclose(file);	
			}
			
			cout<<"history logs berhasil terbaca.\n"<<endl;
			return readLogs;
		}
		
		// Mencari pasangan nama dan rfid.
		string SearchRFID(string rfid) {
    		auto it = rfidToName.find(rfid);
    		if (it != rfidToName.end()) {
        		return it->second; // kembalikan nama sebenarnya
    		}
    		return "Tidak dikenal";
		}
		
		// Membaca data user dari file JSON dan menyimpannya ke vector<User>
		int readFromJSON(vector<User>& db) {
    		ifstream file("dataBase.json");
    		if (!file.is_open()) return 0;

    		json j;
    		file >> j;
    		file.close();

   		 	db = j.get<vector<User>>(); // otomatis pakai from_json()
   		 	return 1;
		}
		
		// Menyimpan vector log ke file JSON
		int exportToJSON(const vector<Log>& logs, const string& filename) {
    		ofstream file(filename);
   			if (!file.is_open()) return 0;

    		json j = logs; // otomatis pakai to_json()
    		file << setw(4) << j << endl; // buat tampil rapi
    		file.close();
    		return 1;
		}

	
		// ===================== MENU METHODS =====================
		
		// Menambahkan entri log baru ke dalam sistem.
		// Param: datasLog -> string berisi data RFID.
		// Output: string konfirmasi status penambahan. 
		string addLog(string datasLog){
			string message;
			
			string temp, action;
			stringstream ss(datasLog);
			getline(ss,temp,' ');
						
		    if (lastActionByRFID.find(temp) == lastActionByRFID.end() || lastActionByRFID[temp] == "Keluar") {
		        action = "masuk";
		    } else {
		        action = "Keluar";
    		}
    		
    		datasLog += " "+ action;
		    lastActionByRFID[temp] = action;
		    
			datasLog+=" "+ SearchRFID(temp);
		
			Log newLog(datasLog);
			logs.push_back(newLog);
			
			string nameFile = "logsBiner"+ID_client+".bin";
			
			if(exportBiner(datasLog, nameFile.c_str())){
				message = "Add log berhasil -> " + newLog.toString() +"\n";
			}else{
				message = "gagal menyimpan log\n";
			}
			
		 	return message;
		}
		
		// Mencari log berdasarkan parameter tertentu (RFID, nama, atau keyword spesifik).
		// Bisa juga digunakan untuk mencari  list log hari ini (Key: "today").
		// Param: param -> keyword pencarian.
		// Output: string hasil pencarian.
		string searchLogs(string param){
			string key, message, temp, today;
			
			stringstream ss(param);
			getline(ss,key,' ');
			
			vector<Log> tempSearch;
			
			message = "List Log dengan key : "+key+"\n";
			
			today = getDate();
			
			for(const auto& log:logs){
				temp = toUpper(log.getName());
				if(log.getRFID().find(key) != string::npos){
					//message += log.getAction() + " " + log.getTime() + "\n";
					tempSearch.push_back(log);
				}else if(temp.find(key) != string::npos){
					//message += log.getAction() + " " + log.getTime() + "\n";
					tempSearch.push_back(log);
				}else if(key == "TODAY" && log.getTime().find(today) != string::npos){
					tempSearch.push_back(log);
				}
			}
			
			message += templateLog();
			
			for(const auto& log:tempSearch){
				message += log.toString();
			}
			
			if(tempSearch.empty()){
				message = "Tidak Ada history log dengan kata kunci : "+ key +"\n";
			}
			
			return message;
		}
		
		// Menampilkan seluruh log yang sudah terurut berdasarkan waktu.
		// Output: string list log yang sudah diformat.
		string listLogs(){
			string message;
			
		 	vector<Log> temp = logs;
		 	
		 	insertionSort(temp);
		
			message = "Daftar Logs:\n" + templateLog();
			
			for(const auto& log:logs){
				message += log.toString();
			}
			
			if (logs.empty()) {
		        message = "Tidak Ada history log\n";
		    }
		 	
		 	return message;
		}
		
		// Mengekspor seluruh data log ke file JSON dengan nama khusus per ID client.
		// Output: string pesan sukses atau error.
		string exportJSON(){
			string message;
			
			string logFile = "logs"+ID_client+".json";
			exportToJSON(logs, logFile);
			
			message = "berhasil export file";
			return message;
		}
		
		// Menampilkan data pengguna dari database (RFID dan nama).
		// Output: string list user.
		string database(){
			string message;
			message = "List anggota yg terdaftar\n";
			
			message += templateDatabase();
			
			for(const auto& data:datas){
				message+= data.toString();
			}
			
			if(datas.empty()){
				message = "tidak ada Anggota yg terdaftar";
			}
			
			return message;
		}
		
		// Menghapus seluruh log yang tersimpan di memori dan file biner.
		// Output: string pemberitahuan penghapusan.		
		string clear(){
			string message;
			logs.clear();
			
			string nameFile = "logsBiner"+ID_client+".bin";
			
			FILE* file = fopen(nameFile.c_str(), "wb");
			
		    if (file) {
		        fclose(file);
		    }
			
			message = "seluruh history log sudah terhapus.....";
			return message;
		}
		
		// Menampilkan daftar perintah/menu yang tersedia untuk client.
		// Output: string berisi deskripsi command yang tersedia.
		string help() {
		    string message =
		        "List Request:\n\n"
		        "- ADD_LOG <RFID> (IN/OUT) (TIMESTAMP)      -> menambah log baru\n"
		        "- SEARCH_LOG <key/RFID/NAME/TODAY>         -> menampilkan list log berdasarkan RFID\n"
		        "- LIST_LOGS                                -> menampilkan list log Lokal/Global berdasarkan waktu\n"
		        "- EXPORT_JSON                              -> menyimpan data log Lokal/Global ke file JSON\n"
		        "- DATABASE                                 -> menampilkan data anggota\n"
		        "- CLEAR                                    -> Menghapus semua history log\n"
		        "- HELP                                     -> menampilkan list Request\n"
		        "- SHUTDOWN                                 -> mengakhiri program\n";
		    
		    return message;
		}
		
		// Fungsi utama untuk memproses input dari client.
		// Param: recvMessage -> string dari client berisi perintah dan opsional data.
		// Output: string hasil pemrosesan perintah.		
		string processRequest(string recvMessage){
			string request, param, sendMessage;
			
			stringstream ss(recvMessage);
		    getline(ss,request,' ');
		    getline(ss,param);
		    
		    if(request == "ADD_LOG"){
		    	sendMessage = addLog(param);
			}else if(request == "SEARCH_LOG"){
				sendMessage = searchLogs(param);
			}else if(request == "LIST_LOGS"){
				sendMessage = listLogs();
			}else if(request == "EXPORT_JSON"){
				sendMessage = exportJSON();
			}else if(request == "DATABASE"){
				sendMessage = database();
			}else if(request == "CLEAR"){
				sendMessage = clear();
			}else if(request == "HELP"){
				sendMessage = help();
			}else if(request == "SHUTDOWN"){
				sendMessage = "Terimkaish program telah selesai";
			}else{
				sendMessage = "Request tidak terdaftar\n";
			}
		    
		    return sendMessage;
		}		
};

// =======================
// Class AttendanceSystem
// Sistem utama yang mengatur komunikasi client-server,
// proses pembacaan RFID, dan integrasi LogManager.
// =======================
class AttendanceSystem{
	public:
		
		// Fungsi untuk menangani koneksi tiap client.
		// Fungsi ini akan dijalankan pada thread terpisah untuk setiap client yang terhubung.
		void clientPort(SOCKET client_socket){
			int  recv_size;
		    char messageRecv[100], ID_client[5];
		    
		    recv_size = recv(client_socket, ID_client, sizeof(ID_client) - 1, 0);
		    
		    if(recv_size == SOCKET_ERROR){
		       	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
		   	}else {
		       	ID_client[recv_size] = '\0';
		   	}

		    cout << "\nkoneksi terhubung dengan ID_client: " << ID_client << endl;
		    
		    LogManager mainPros(ID_client);
		    
		    string menu = mainPros.help();
		    send(client_socket, menu.c_str(), menu.length(), 0);
		    
		    
		    while(1){
		    	recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
		    
		    	if(recv_size == SOCKET_ERROR){
		        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
		        	break;
		    	}else {
		        	messageRecv[recv_size] = '\0';
		    	}
		    
		    	cout << "Pesan dari Client("<<ID_client<<") : " << messageRecv <<endl;
		    
		    	string message = mainPros.processRequest(messageRecv);
		    
		    	send(client_socket, message.c_str(), message.length(), 0);
		    	
		    	if(message == "program telah selesai, menutup server....."){
		    		break;
				}
			}
			
			closesocket(client_socket);
			
		}
		
		// Fungsi untuk membuat socket server.
		SOCKET makeSocket(){
			WSADATA wsa;
		    SOCKET listen_socket;
		    struct sockaddr_in server;
		    
		    
		    cout << "Memulai Winsock...\n";
		    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		        cerr << "WSAStartup gagal. Error code: " << WSAGetLastError() <<endl;
		        return INVALID_SOCKET;
		    }
		    cout << "Winsock terisntall.\n";
		
		    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
		    if (listen_socket == INVALID_SOCKET) {
		        cerr <<"gagal membuat socket. Error code: " << WSAGetLastError() <<endl;
		        WSACleanup();
		        return INVALID_SOCKET;
		    }
		    cout << "Socket berhasil dibuat."<<endl;
		
		    server.sin_family = AF_INET;
		    server.sin_addr.s_addr = INADDR_ANY;
		    server.sin_port = htons(PORT);
		
		    if (bind(listen_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		        cerr << "Bind gagal. Error code: " << WSAGetLastError() <<endl;
		        closesocket(listen_socket);
		        WSACleanup();
		        return INVALID_SOCKET;
		    }
		    cout << "Bind berhasil."<<endl;
		    
		    listen(listen_socket, 3);
		    cout << "menunggu koneksi..."<<endl;
		    
		    return listen_socket;
		}
		
		// Metode utama server yang akan standby menunggu koneksi client.
		// Setelah client terkoneksi, server akan membuat thread baru untuk setiap client (multithreading).
		void ServerProses(){
			struct sockaddr_in client;
			int c = sizeof(struct sockaddr_in);
		
			SOCKET listen_socket = makeSocket();
			if (listen_socket == INVALID_SOCKET) {
		        cerr <<"gagal membuat socket. Error code: " << WSAGetLastError() <<endl;
		        WSACleanup();
		        return;
		    }
		    
		    while(1){
		    	SOCKET client_socket = accept(listen_socket, (struct sockaddr*)&client, &c);
		    
		    	if (client_socket == INVALID_SOCKET) {
		        	cerr << "Accept gagal. Error code: " << WSAGetLastError() <<endl;
		        	closesocket(listen_socket);
		        	WSACleanup();
		        	return;
		    	}
		    	
		    	thread t(&AttendanceSystem::clientPort, this,client_socket);
		    	t.detach();
			}
		
		    closesocket(listen_socket);
		    WSACleanup();	
		}
		
};

// Fungsi utama program Server
// Inisialisasi sistem dan menjalankan loop utama
int main() {
	// Inisialisasi objek utama sistem absensi
	AttendanceSystem As;
	
	// Menjalankan proses server dan standby menunggu client untuk terhubung
	As.ServerProses();	
    return 0;
}
