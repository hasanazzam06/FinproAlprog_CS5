#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <winsock2.h>
#include <fstream>
#include "json.hpp"
#include <algorithm>

using namespace std;
using json = nlohmann::json;

#define PORT 8888



class Log{
	private:
		string RFID;
		string timestamp;
        string name;
		string action;
	public:
        Log(string param){
            stringstream ss(param);
            getline(ss, RFID,' ');
            getline(ss, name, ' ');
            getline(ss, timestamp,' ');
            getline(ss, action,' ');
        }

        Log(string rfid, string name, string time, string action)
        : RFID(rfid), name(name), timestamp(time), action(action) {}
		
		string getRFID() const { return RFID; }
    	string getTime() const { return timestamp; }
    	string getAction() const { return action; }
    	string getName() const { return name; }
    	string toString() const {
        return "RFID: " + getRFID() + ", Name: " + getName() + ", Timestamp: " +  getTime() + ", Action: " + getAction();
    	}
};

vector<Log> logs;

unordered_map<string, string> rfidToName;

struct LogRaw {
    char rfid[6];
    char time[21];
    char action[10];
    char name[30];
};

void loadDatabaseFast(const string& filename = "database.json") {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file.\n";
        return;
    }

    json db;
    file >> db;

    for (const auto& entry : db) {
        string rfid = entry["rfid"].get<string>();
        string name = entry["name"].get<string>();
        
        rfidToName[rfid] = name;
    }
}

string SearchRFID(string message) {
    auto it = rfidToName.find(message);
    if (it != rfidToName.end()) {
        return it->second; // kembalikan nama sebenarnya
    }
    return "Tidak dikenal";
}

unordered_map<string, string> lastActionByRFID;


void insertionSort(std::vector<Log>& listLog) {
    int n = listLog.size();
    for (int i = 1; i < n; i++) {
        Log key = listLog[i];
        int j = i - 1;
        while (j >= 0 && listLog[j].getTime() > key.getTime()) {
            listLog[j + 1] = listLog[j]; // geser ke kanan
            j--;
        }
        listLog[j + 1] = key; // masukkan di posisi yang benar
    }
}

void loadAction(){
    for (const auto& log : logs) {
        lastActionByRFID[log.getRFID()] = log.getAction();
    }
}

string addLog(string param){
    string message;

    // Ambil RFID dan waktu dari param
    stringstream ss(param);
    string rfid, date, time, timestamp;
    ss >> rfid >> date >> time;
    timestamp = date + ";" + time;

    string action;
    if (lastActionByRFID.find(rfid) == lastActionByRFID.end() || lastActionByRFID[rfid] == "Keluar") {
        action = "Masuk";
    } else {
        action = "Keluar";
    }

    lastActionByRFID[rfid] = action;
    string name = SearchRFID(rfid);

    Log newLog(rfid, name, timestamp, action); 

    logs.push_back(newLog);

    message = "Add log berhasil -> " + newLog.toString();
    return message;
}

// string getLogs(string param){
// 	string type,message = "";
//  	stringstream ss(param);
//  	getline(ss, type,' ');
 	
//  	vector<Log> temp = logs;
 	
//  	insertionSort(temp);
 	
//  	for(const auto& log:temp){
//  		message += log.getRFID() + " " + log.getName() + " " + log.getTime() + " " + log.getAction() + "\n";
// 	}
	
// 	if (message.empty()) {
//         message = "Tidak ada log.\n";
        
        
//     }
 	
//  	return message;
// }

// vector<Log> readLogsFromBinary(const string& filename) {
//     vector<Log> result;
//     ifstream file(filename, ios::binary);
//     if (!file.is_open()) {
//         cerr << "Gagal membuka file.\n";
//         return result;
//     }

//     LogRaw raw;
    
//     while (file.read(reinterpret_cast<char*>(&raw), sizeof(LogRaw))) {
//         string rfid(raw.rfid);
//         string time(raw.time);
//         string action(raw.action);
//         string name(raw.name);
//         Log newLog(rfid, name, time, action); 
//         result.push_back(newLog);
//     }

//     return result;
// }

void loadFileBinery(){
    ifstream file("logs.bin", ios::binary);
    if (!file.is_open()) {
        cerr << "Gagal membuka file.\n";
    }

    LogRaw raw;
    
    while (file.read(reinterpret_cast<char*>(&raw), sizeof(LogRaw))) {
        string rfid(raw.rfid);
        string time(raw.time);
        string action(raw.action);
        string name(raw.name);
        Log newLog(rfid, name, time, action); 
        logs.push_back(newLog);
    }
}

void sendAllLogs(SOCKET sock) {

    for (const auto& log : logs) {
        string logString = log.getName() + " " + log.getTime() + " " + log.getAction();
        send(sock, logString.c_str(), logString.size(), 0);
        Sleep(50); // beri jeda kecil jika perlu
    }
    string endSignal = "#END";
    send(sock, endSignal.c_str(), endSignal.length(), 0);
    cout << "Semua log telah dikirim.\n";
}


void appendToBinary(const Log& log, const string& filename) {
    ofstream file(filename, ios::binary | ios::app);
    if (!file.is_open()) {
        cerr << "Gagal membuka file.\n";
        return;
    }

    LogRaw raw{};
    strncpy(raw.rfid, log.getRFID().c_str(), sizeof(raw.rfid) - 1);
    strncpy(raw.time, log.getTime().c_str(), sizeof(raw.time) - 1);
    strncpy(raw.action, log.getAction().c_str(), sizeof(raw.action) - 1);
    strncpy(raw.name, log.getName().c_str(), sizeof(raw.name) - 1);

    file.write(reinterpret_cast<char*>(&raw), sizeof(LogRaw));
    file.close();
}


string handleScan(string param) {
    string result = addLog(param);
    appendToBinary(logs.back(), "logs.bin");
    cout << result << endl;
    return "Berhasil Ditambahkan";
}

vector<Log> searchLogByName(const vector<Log>& logs, const string& keyword) {
    vector<Log> result;

    for (const auto& log : logs) {
        string name = log.getName();
        // Ubah ke lowercase untuk pencocokan yang tidak case-sensitive
        string lowerName = name;
        string lowerKeyword = keyword;
        transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);
        cout << lowerKeyword;

        if (lowerName.find(lowerKeyword) != string::npos) {
            result.push_back(log);
        }
    }

    return result;
}

void sendSearching(SOCKET sock, const vector<Log>& allLogs, const string& keyword) {
    vector<Log> hasil = searchLogByName(allLogs, keyword);

    for (const auto& log : hasil) {
        string logString = log.getName() + " " + log.getTime() + " " + log.getAction();
        send(sock, logString.c_str(), logString.size(), 0);
        Sleep(50); // beri jeda kecil jika perlu
    }
    string endSignal = "#END";
    send(sock, endSignal.c_str(), endSignal.length(), 0);
    cout << "Semua log telah dikirim.\n";
}




void downloadLogs(SOCKET sock){
    for (const auto& log : logs) {
        json j;
        j["name"] = log.getName();
        j["timestamp"] = log.getTime();
        j["action"] = log.getAction();

        string sendMessage = j.dump(); // ubah menjadi string JSON
        send(sock, sendMessage.c_str(), sendMessage.size(), 0);
        Sleep(50); // beri jeda kecil jika perlu
    }
    string endSignal = "END";
    send(sock, endSignal.c_str(), endSignal.length(), 0);
    cout << "Semua log telah dikirim.\n";
}

string exportBiner(string param){
	string type,message;
 	stringstream ss(param);
	getline(ss, type,' ');
	
	return message;
}


bool processRequest(string recvMessage, SOCKET sock){
	string request, param,sendMessage;
	
	stringstream ss(recvMessage);
    getline(ss,request,' ');
    getline(ss,param);
    
    if(request == "SHOW_LOG"){
        sendAllLogs(sock);
        return 1;
    }else if(request == "SEARCH_LOG"){
        cout << param;
        sendSearching(sock, logs, param);
        return 1;
    }else if(request == "EXIT"){
        return 0;
    }else if(request == "DOWNLOAD_LOG"){
        downloadLogs(sock);
        return 1;
    }else{
        sendMessage = handleScan(recvMessage);
        send(sock, sendMessage.c_str(), sendMessage.length(), 0);
        return 1;
    }
	// }else if(request == "GET_SHORTING_LOGS"){
	// 	sendMessage = addLogs(param);
	// }else if(request == "SEARCH_LOG"){
	// 	sendMessage = addLogs(param);
	// }else if(request == "EXPORT_JSON"){
	// 	sendMessage = addLogs(param);
	// }else if(request == "EXPORT_BINER"){
	// 	sendMessage = addLogs(param);
	// }else if(request == "SHUTDOWN"){
	// 	sendMessage = addLogs(param);
	// }else{
	// 	sendMessage = addLogs(param);
	// }
}

int main() {
    loadDatabaseFast();
    loadFileBinery();
    loadAction();
    
    
    WSADATA wsa;
    SOCKET listen_socket, client_socket;
    struct sockaddr_in server, client;
    int c, recv_size;
    char messageRecv[100];
    string messageSend;
    
    cout << "Memulai Winsock...\n";
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "WSAStartup gagal. Error code: " << WSAGetLastError() <<endl;
        return 1;
    }
    cout << "Winsock terisntall.\n";

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET) {
        cerr <<"gagal membuat socket. Error code: " << WSAGetLastError() <<endl;
        WSACleanup();
        return 1;
    }
    cout << "Socket berhasil dibuat."<<endl;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(listen_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        cerr << "Bind gagal. Error code: " << WSAGetLastError() <<endl;
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }
    cout << "Bind berhasil."<<endl;

    listen(listen_socket, 3);
    cout << "menunggu koneksi..."<<endl;

    c = sizeof(struct sockaddr_in);
    
    client_socket = accept(listen_socket, (struct sockaddr*)&client, &c);
    
    if (client_socket == INVALID_SOCKET) {
        cerr << "Accept gagal. Error code: " << WSAGetLastError() <<endl;
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }
    cout << "koneksi terhubung.\n\n";
    
    bool status = 1;
    while(status){
    	recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
    
    	if(recv_size == SOCKET_ERROR){
        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
    	} else {
        	messageRecv[recv_size] = '\0';
            cout << "Pesan dari server: " << messageRecv <<endl;
            status = processRequest(messageRecv, client_socket);
        }
	}
    
    closesocket(client_socket);
    closesocket(listen_socket);
    WSACleanup();

    return 0;
}
