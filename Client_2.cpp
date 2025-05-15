#include <iostream>
#include <string>
#include <winsock2.h>

using namespace std;

#define SERVER "127.0.0.1"
#define PORT 8888

int main() {
	// Kerjakan Program anda disini
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server;
    int recv_size;
    char messageRecv[100];
    string messageSend,phars = "0";
    
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
    
    while(1){
    	cout<<"----------------------------------\n"<<endl;
    	cout << "Request: ";
   		getline(cin, messageSend);

    	send(client_socket, messageSend.c_str(), messageSend.length(), 0);
    
    	recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
    
    	if(recv_size == SOCKET_ERROR){
        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
    	}else {
        	messageRecv[recv_size] = '\0';
    	}
    
    	cout << "Pesan dari server: " << messageRecv <<endl;
    	
    	if(messageSend == "SHUTDOWN"){
    		break;
    		
		}
	}
    
    closesocket(client_socket);
    WSACleanup();
    
    return 0;
}