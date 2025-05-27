#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <winsock2.h>

using namespace std;

#define SERVER "127.0.0.1"
#define PORT 8888

string ID_CLIENT = "112";

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

class ClientProgram{
	private:
		WSADATA wsa;
	    SOCKET client_socket;
	    struct sockaddr_in server;
	    int recv_size;
	    char messageRecv[1000];
	    string messageSend;
	    
	public:
		
		void clientInitial(){
			cout << "Memulai Winsock...\n";
		    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		        cerr << "WSAStartup gagal. Error code: " << WSAGetLastError() <<endl;
		        return;
		    }
		    
		    cout << "Winsock terisntall."<<endl;
		
		    client_socket = socket(AF_INET, SOCK_STREAM, 0);
		    if (client_socket == INVALID_SOCKET) {
		        cerr << "gagal membuat socket. Error code: " << WSAGetLastError() <<endl;
		        WSACleanup();
		        return;
		    }
		    cout << "Socket berhasil dibuat."<<endl;
		
		    server.sin_addr.s_addr = inet_addr(SERVER);
		    server.sin_family = AF_INET;
		    server.sin_port = htons(PORT);
		
		    if (connect(client_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
		        cerr << "Connection gagal. Error code: " << WSAGetLastError() <<endl;
		        closesocket(client_socket);
		        WSACleanup();
		        return;
		    }
		    
		    cout << "\nterhubung ke server.\n"<<endl;
		    
		    send(client_socket, ID_CLIENT.c_str(), ID_CLIENT.length(), 0);
		    
		    clientMainProses();
		    
		    closesocket(client_socket);
		    WSACleanup();	
		}
		
		void clientMainProses(){
			while(1){
		    	cout<<"----------------------------------\n"<<endl;
		    	cout << "Request: ";
		   		getline(cin, messageSend);
		   		
		   		messageSend = toUpper(messageSend);
		   		
		   		string temp;
		   		
		   		stringstream ss(messageSend);
		   		getline(ss,temp,' ');
		   		
		   		if(temp == "ADD_LOG"){
		   			messageSend += " "+ getCurrentTime();
				}
		
		    	send(client_socket, messageSend.c_str(), messageSend.length(), 0);
		    
		    	recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
		    
		    	if(recv_size == SOCKET_ERROR){
		        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
		    	}else {
		        	messageRecv[recv_size] = '\0';
		    	}
		    
		    	cout << "Pesan dari server: \n" << messageRecv <<endl;
		    	
		    	if(messageSend == "SHUTDOWN"){
		    		break;
				}
			}
		}		
};

int main() {
	
	ClientProgram Cp;
	
	Cp.clientInitial();

    return 0;
}