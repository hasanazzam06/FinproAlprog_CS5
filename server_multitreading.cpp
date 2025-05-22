#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <winsock2.h>

using namespace std;

#define PORT 8888

//	ADD_LOG <RFID> <TIMESTAMP> <IN/OUT>
//   	GET_SHORTING_LOGS <LOCAL/GLOBAL>
//   	SEARCH_LOG <ID/RFID/NAME>
//  	EXPORT_JSON <LOCAL/GLOBAL>
//   	EXPORT_BINER <LOCAL/GLOBAL>
//   	SHUTDOWN
   		


string processRequest(string recvMessage){
	string request, param,sendMessage;
	
	stringstream ss(recvMessage);
    getline(ss,request,' ');
    getline(ss,param);
    
//    if(request == "ADD_LOG"){
//    	sendMessage = addLogs(param);
//	}else if(request == "GET_SHORTING_LOGS"){
//		sendMessage = addLogs(param);
//	}else if(request == "SEARCH_LOG"){
//		sendMessage = addLogs(param);
//	}else if(request == "EXPORT_JSON"){
//		sendMessage = addLogs(param);
//	}else if(request == "EXPORT_BINER"){
//		sendMessage = addLogs(param);
//	}else if(request == "SHUTDOWN"){
//		sendMessage = addLogs(param);
//	}else{
//		sendMessage = addLogs(param);
//	}
    
    sendMessage ="berhasil";
	return sendMessage;
}

void clientPort(SOCKET client_socket){
	int  recv_size;
    char messageRecv[100];
    
    while(1){
    	recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
    
    	if(recv_size == SOCKET_ERROR){
        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
    	}else {
        	messageRecv[recv_size] = '\0';
    	}
    
    	cout << "Pesan dari server: " << messageRecv <<endl;
    
    	string message = processRequest(messageRecv);
    
    	send(client_socket, message.c_str(), message.length(), 0);
    	
    	if(message == "program telah selesai, menutup server....."){
    		break;
		}
	}
	
	closesocket(client_socket);
	
}

int main() {
    
    
    WSADATA wsa;
    SOCKET listen_socket;
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
    
    while(1){
    	SOCKET client_socket = accept(listen_socket, (struct sockaddr*)&client, &c);
    
    	if (client_socket == INVALID_SOCKET) {
        	cerr << "Accept gagal. Error code: " << WSAGetLastError() <<endl;
        	closesocket(listen_socket);
        	WSACleanup();
        	return 1;
    	}
    	cout << "koneksi terhubung.\n\n";
    	
    	thread t(clientPort,client_socket);
    	t.detach();
	}
//    SOCKET client_socket = accept(listen_socket, (struct sockaddr*)&client, &c);
//    
//    if (client_socket == INVALID_SOCKET) {
//        cerr << "Accept gagal. Error code: " << WSAGetLastError() <<endl;
//        closesocket(listen_socket);
//        WSACleanup();
//        return 1;
//    }
//    cout << "koneksi terhubung.\n\n";
    
//    while(1){
//    	recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
//    
//    	if(recv_size == SOCKET_ERROR){
//        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
//    	}else {
//        	messageRecv[recv_size] = '\0';
//    	}
//    
//    	cout << "Pesan dari server: " << messageRecv <<endl;
//    
//    	string message = processRequest(storage,messageRecv);
//    
//    	send(client_socket, message.c_str(), message.length(), 0);
//    	
//    	if(message == "program telah selesai, menutup server....."){
//    		break;
//		}
//	}
    
//    closesocket(client_socket);
    closesocket(listen_socket);
    WSACleanup();

    return 0;
}
