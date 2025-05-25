#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype> 

using namespace std;
   		
class Log{
	private:
		string RFID;
		string action;
		string time;
		
	public:
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
};

vector<Log> logs;

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

string getCurrentTime() {
    time_t now = time(0); // ambil waktu sekarang dalam format epoch (detik)
    tm* localTime = localtime(&now); // ubah jadi struct waktu lokal
    
    stringstream ss;
    ss << put_time(localTime, "%Y-%m-%d->%H:%M:%S"); // format waktu jadi string
    return ss.str();
    
}

string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

int exportBiner(string param){
	int status=0;
	
	char temp[100];
	strcpy(temp,param.c_str());
	
	FILE* file = fopen("logsBiner.bin", "ab");
	
	if(file){
		fwrite(temp, sizeof(temp),1,file);
		
		fclose(file);
		status = 1;	
	}
	
	return status;
}

int readFileBiner(){
	int status=0;
	char data[100];
	string param;
	
	FILE* file = fopen("logsBiner.bin", "rb");
	
	if(file){
		
		while(fread(data, sizeof(data),1, file)){
			param = data;
			
			Log readLog(param);
			logs.push_back(readLog);
		}
		
		fclose(file);
		status = 1;	
	}
	
	cout<<"proses dara ....."<<endl;
	return status;
}

string addLog(string param){
	string message;
	
	Log newLog(param);
	logs.push_back(newLog);
	
	if(exportBiner(param)){
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
	
	return message;
}

string database(){
	string message;
	message = "List anggota yg terdaftar";
	
	
	return message;
}

string help() {
    string message =
        "List Request:\n\n"
        "- ADD_LOG <RFID> <IN/OUT> (TIMESTAMP)      -> menambah log baru\n"
        "- SEARCH_LOG <RFID>                        -> menampilkan list log berdasarkan RFID\n"
        "- LIST_LOGS                                -> menampilkan list log Lokal/Global berdasarkan waktu\n"
        "- EXPORT_JSON  (blom ada)                  -> menyimpan data log Lokal/Global ke file JSON\n"
        "- DATABASE     (blom ada)                  -> menampilkan data anggota\n"
        "- CLEAR        (blom ada)                  -> Menghapus semua history log\n"
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
	}else if(request == "EXPORT_BINER"){
		sendMessage = database();
	}else if(request == "HELP"){
		sendMessage = help();
	}else if(request == "SHUTDOWN"){
		sendMessage = "Terimkaish program telah selesai";
	}else{
		sendMessage = "Request tidak terdaftar\n";
	}
    
    return sendMessage;
}
	
int main() {
    string messageSend;
    
    readFileBiner();
    
    while(1){
    	
   	 cout << "\nRequest: ";
   		getline(cin, messageSend);
   		
   		messageSend = toUpper(messageSend);
   		
   		string temp;
   		
   		stringstream ss(messageSend);
   		getline(ss,temp,' ');
   		
   		if(temp == "ADD_LOG"){
   			messageSend += " "+ getCurrentTime();
		}
		
    	cout<<processRequest(messageSend);
    	
    	if(temp == "SHUTDOWN"){
    		break;
		}
	}
    return 0;
}
