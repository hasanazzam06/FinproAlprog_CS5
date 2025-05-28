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


//memebuat sting jadi kapital
string toUpper(string str) {

}

//return tgl hari ini
string getDate() {

}


//template string header list log
string templateLog(){

}


//template string header list database
string templateDatabase(){

}

//class 1 log
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

		}
		
		//return atribut
		string getRFID() const { return RFID; }
    	string getAction() const { return action; }
    	string getTime() const { return time; }
    	string getName() const { return name;}
    	
    	//return semua atribut
    	string toString() const {

    	}
    	
    	//fungsi exportdan baca json
    	friend void to_json(json& j, const Log& l);
    	friend void from_json(const json& j, Log& l);
};

//fungsi export json
void to_json(json& j, const Log& l){

}
//fungsi baca json
void from_json(const json& j, Log& l) {

}


//class ke 2 user database
class User{
	private:
		//atribut class
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
 
    	}
		
		friend void to_json(json& j, const User& u);
		friend void from_json(const json& j, User& u);
		
};

// mirirp kayak sebelumnya tapi khusus class database bukan log
void to_json(json& j, const User& u){

}
void from_json(const json& j, User& u) {

}


//class 3 class main program log manajer
class LogManager{
	private:
		// atribut class
		vector<Log> logs;
		vector<User> datas;
		string ID_client;
		unordered_map<string, string> lastActionByRFID;
		unordered_map<string, string> rfidToName;
	
	public:
		//metdho class
		LogManager(string ID){

		}
		
// method umum
		
		// mencari action terakhir dari suatu rfid
		void loadAction(){

		}
		
		// function shorting metode insertion
		void insertionSort(vector<Log>& listLog) {

		}
		
		//nulis data ke biner dengan cara menambahkan data dikagir file, bukan buat ulang
		int exportBiner(string datasLog, const char nameFile[]){

		}
		
		//membaca file biner dan menyimpan ke vector Log
		vector<Log> loadFileBinery(const char nameFile[]){

		}
		
		// mencarai pasangan nama dan rfid
		string SearchRFID(string rfid) {

		}
		
		//membaca file databse json dan menyimpanya ke vector user
		int readFromJSON(vector<User>& db) {

		}
		
		// menyimpan vector log ke file json
		int exportToJSON(const vector<Log>& logs, const string& filename) {

		}

//method menu
		
		//fungsi add log untuk menambah log dengan paramter rfid 
		string addLog(string datasLog){

		}
		
		//fungsi mencari log dengan kata kunci tertentu bisa rfid taupun nama. bisa juga untuk mencari list log di hari ini
		string searchLogs(string param){
			
		}
		
		// menampilkan list log yg sudah terururt
		string listLogs(){
			
		}
		
		// fungsi untuk export log ke file json/ fokus ke menu beda dengan fungsi sebelumnya
		string exportJSON(){

		}
		
		// menampilkan list database
		string database(){

		}
		
		//menghapus semua list log saat ini		
		string clear(){

		}
		
		// menampilkkan list perintah yg tersedia
		string help() {

		}
		
		
		// method main yg memproses semua menu yg akan dipilih user	
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


//class 3 syetem server dan cliengt
class AttendanceSystem{
	private:
		// atribut
		vector<Log> logsGlobal;
		
	public:
		
		// fungsi yang digunakkan untuk menangani suatu client, nantinya setiap clianet yg terhubung  server akan menjalankan fungsi ini untuk tiap clientnya
		void clientPort(SOCKET client_socket){
			
		}
		
		
		// membuat socket untuk server
		SOCKET makeSocket(){
			
		}
		
		
		// metod server proses, functin yg akan stanby untuk mendengarkan dan menerima clinet, setlah itu akan mmenjalankan thread agar mendukung mutithreading
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

// main function
int main() {
	//intialisasi class uatama
	AttendanceSystem As;
	
	//menjalankan server dan stdanbya menunggu client
	As.ServerProses();	
    return 0;
}
