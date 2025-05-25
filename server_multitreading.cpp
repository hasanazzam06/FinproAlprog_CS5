#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cctype> 
#include <thread>
#include <winsock2.h>
#include "json.hpp"
 
using json = nlohmann::json;
using namespace std;

#define PORT 8888

class Log{
	private:
		string RFID;
		string action;
		string time;
		
	public:
		Log() = default; 
		
    	Log(string rfid, string act, string t)
        : RFID(rfid), action(act), time(t) {}
		
		Log(string param){
			
 			stringstream ss(param);
 			getline(ss, RFID,' ');
 			getline(ss, action,' ');
 			getline(ss, time,' ');
		}
		
		string getRFID() const { return RFID; }
    	string getAction() const { return action; }
    	string getTime() const { return time; }
    	
    	string toString() const {
        return "RFID: " + RFID + ", Action: " + action + ", Time: " + time;
    	}
    	
    	friend void to_json(json& j, const Log& l);
    	friend void from_json(const json& j, Log& l);
};

void to_json(json& j, const Log& l){
    j = json{{"rfid", l.RFID}, {"action", l.action}, {"time", l.time}};
}

void from_json(const json& j, Log& l) {
    j.at("rfid").get_to(l.RFID);
    j.at("action").get_to(l.action);
    j.at("time").get_to(l.time);
}

int exportToJSON(const vector<Log>& logs, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) return 0;

    json j = logs; // otomatis pakai to_json()
    file << setw(4) << j << endl; // buat tampil rapi
    file.close();
    return 1;
}

class User{
	private:
		string name;
		string RFID;
		string status;
		
	public:
		User(string n,string m, string o, string p){
			name = n;
			RFID = o;
			status = p;
		}
		
		string getName() const{ return name; }
		string getRFID() const{ return RFID; }
		string getStatus() const{return status;}
};

class LogManager{
	private:
		vector<Log> logs;
		vector<User> datas;
		string ID_client;
	
	public:
		
		LogManager(string ID){
			ID_client = ID;
			string logFile = "logsBiner"+ID_client+".bin";
			logs = readFileBiner(logFile.c_str());
			//datas = readFileBiner()
		}
		
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

		int exportBiner(string param, const char nameFile[]){
			int status=0;
			
			char temp[100];
			strcpy(temp,param.c_str());
			
			FILE* file = fopen(nameFile, "ab");
			
			if(file){
				fwrite(temp, sizeof(temp),1,file);
				
				fclose(file);
				
				status = 1;	
			}
			
			return status;
		}
		
		vector<Log> readFileBiner(const char nameFile[]){
			int status=0;
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
				status = 1;	
			}
			
			cout<<"history logs berhasil terbaca.\n"<<endl;
			return readLogs;
		}
		
		string addLog(string datasLog){
			string message;
			
			Log newLog(datasLog);
			logs.push_back(newLog);
			
			string nameFile = "logsBiner"+ID_client+".bin";
			
			if(exportBiner(datasLog,nameFile.c_str())){
				message = "Add log berhasil -> " + newLog.toString() +"\n";
			}else{
				message = "gagal menyimpan log\n";
			}
			
		 	return message;
		}
		
		string searchLogs(string param){
			string key, message;
			
			stringstream ss(param);
			getline(ss,key,' ');
			
			message = "List Log dengan RFID: "+key+"\n";
			
			for(const auto& log:logs){
				if(log.getRFID() == key){
					message += log.getAction() + " " + log.getTime() + "\n";
				}
			}
			
			if(logs.empty()){
				message = "Tidak Ada history log dengan RFID"+ key +"\n";
			}
			
			return message;
		}
		
		string listLogs(){
			string message;
			
		 	vector<Log> temp = logs;
		 	
		 	insertionSort(temp);
		 	
			message = "Daftar Logs:\n";
			for(const auto& log:temp){
				message += log.getRFID() + " " + log.getAction() + " " + log.getTime() + "\n";
			}
			
			if (logs.empty()) {
		        message = "Tidak Ada history log\n";
		    }
		 	
		 	return message;
		}
		
		string exportJSON(){
			string message;
			
			string logFile = "logs"+ID_client+".json";
			exportToJSON(logs, logFile);
			
			message = "berhasil export file";
			return message;
		}
		
		string database(){
			string message;
			message = "List anggota yg terdaftar";
			
			for(const auto& data:datas){
				message+=data.getName() + " " + data.getRFID();
			}
			
			if(datas.empty()){
				message = "tidak ada Anggota yg terdaftar";
			}
			
			return message;
		}
		
		string clear(){
			string message;
			logs.clear();
			
			FILE* file = fopen("logsBiner.bin", "wb");
			
		    if (file) {
		        fclose(file);
		    }
			
			message = "seluruh history log sudah terhapus.....";
			return message;
		}
		
		string help() {
		    string message =
		        "List Request:\n\n"
		        "- ADD_LOG <RFID> <IN/OUT> (TIMESTAMP)      -> menambah log baru\n"
		        "- SEARCH_LOG <RFID>                        -> menampilkan list log berdasarkan RFID\n"
		        "- LIST_LOGS                                -> menampilkan list log Lokal/Global berdasarkan waktu\n"
		        "- EXPORT_JSON                              -> menyimpan data log Lokal/Global ke file JSON\n"
		        "- DATABASE                                 -> menampilkan data anggota\n"
		        "- CLEAR                                    -> Menghapus semua history log\n"
		        "- HELP                                     -> menampilkan list Request\n"
		        "- SHUTDOWN                                 -> mengakhiri program\n";
		    
		    return message;
		}
			
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

class AttendanceSystem{
	private:
		vector<Log> logsGlobal;
		
	public:
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

int main() {
	AttendanceSystem As;
	
	As.ServerProses();	
    return 0;
}
