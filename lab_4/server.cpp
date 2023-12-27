#include <iostream>  
#include <winsock2.h> 
#include <windows.h> 
#include <string> 

#pragma comment (lib, "Ws2_32.lib")  
#define SRV_PORT 1234  
#define BUF_SIZE 64  

using namespace std;

struct Student {
	char fam[20];
	int vals[4];
};

const string greeting = "Server is ready to work"; 

int main() {
	setlocale(LC_ALL, "rus");
	// Оформление окна
	cout << "\t Сервер\n";
	for (int i = 0; i < 30; i++) 
		cout << "-";
	cout << endl;

	// Инициализируем библиотеку 
	char buff[1024];
	if (WSAStartup(0x0202, (WSADATA*) &buff[0])) {
		cout << "Ошибка инициализации! \n" << WSAGetLastError();   // Ошибка!
		return -1;
	}

	SOCKET sListener, sNew;
	sockaddr_in sin, clntSin;

	sListener = socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(SRV_PORT);
	bind(sListener, (sockaddr*) &sin, sizeof(sin));
	
	int len;
	char buf[BUF_SIZE] = {0};
	string msg;
	listen(sListener, 3);
	while (true) {
		len = sizeof(clntSin);
		sNew = accept(sListener, (sockaddr*) &clntSin, &len);
		cout << "New connected client! "<<endl;
		msg = greeting; 
		while (true) {
			send(sNew, (char*) &msg[0], msg.size(), 0);
			len = recv(sNew, (char*) buf, BUF_SIZE, 0);
			if (len == SOCKET_ERROR) {
				break;
			}
			Student* y = (Student*) (&buf[0]);

			cout << (*y).fam << ": ";
			int mnv = 5;
			for (int i = 0; i < 4; i++)
				mnv = min(mnv, (*y).vals[i]);

			if (mnv <= 2) 
				msg = "Имеет задолженность!";
			if (mnv == 3) 
				msg = "Нет задолженностей, нет стипендии";
			if (mnv == 4)
				msg = "Нет задолженностей, есть стипендия: 1500 рублей";
			if (mnv >= 5)
				msg = "Нет задолженностей, есть стипендия: 2200 рублей";

			cout << msg << endl;
		}

		cout << "Client is lost \n";
		closesocket(sNew);
	}
	return 0;
}
