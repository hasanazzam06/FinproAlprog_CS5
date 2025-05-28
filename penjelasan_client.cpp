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


// id khusus setiap client agar tidak terjadi duplikat dokumen
string ID_CLIENT = "112";

// mendapatkan detail waktu saat ini sebagai timestamp log tahun -> detik
string getCurrentTime() {


// membuat semua string menjadi kapital agar pesan tidak case senstive
string toUpper(string str) {

}

//class main program
class ClientProgram{
	private:
		//atribut class
		WSADATA wsa;
	    SOCKET client_socket;
	    struct sockaddr_in server;
	    int recv_size;
	    char messageRecv[1000];
	    string messageSend;
	    
	public:
		
		// memulai menjalankan socket client dan mencoba connect ke server
		void clientInitial(){
			
		}
		
		
		// proses main program, setelah terhubug ke server dan dapat mulai komunikasi, sesaui list perintah yg ada
		void clientMainProses(){
			
			recv_size = recv(client_socket, messageRecv, sizeof(messageRecv) - 1, 0);
			
			if(recv_size == SOCKET_ERROR){
		        	cout << "gagal menerima pesan error code" <<  WSAGetLastError()<<endl;
		    }else {
		        	messageRecv[recv_size] = '\0';
		   	}
		   	
		   	cout << "Pesan dari server: \n" << messageRecv <<endl;
			
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