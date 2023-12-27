#include <iostream> 
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS  
#include <WinSock2.h>
#include <Windows.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#define PORT 666 // Порт сервера
#define BUF_SIZE 1024

using namespace std;

string greeting = "Hello, Student";

int main() {
	setlocale(LC_ALL, "rus");

	// Оформление окна
	cout << "\t UDP-Сервер\n";
	for (int i = 0; i < 30; i++)
		cout << "-";
	cout << endl;

	// Инициализируем библиотеку 
	char buff[1024];
	if (WSAStartup(0x0202, (WSADATA*)&buff[0])) {
		cout << "Ошибка инициализации! \n" << WSAGetLastError();
		return -1;
	}

	// Создание сокета UDP
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET) {
		cout << "Ошибка создания сокета! \n" << WSAGetLastError();
		return -1;
	}

	// Создание структуры локального адреса	сокета
	sockaddr_in sAddr;

	// Заполняем структуру адреса для клиента
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;	// ~ clntSin.sin_addr.S_un.S_addr
	sAddr.sin_port = htons(PORT);



	// Связываем сокет с адресом и номером порта
	if (bind(s, (sockaddr*)&sAddr, sizeof(sAddr))) {
		cout << "Ошибка привязки сокета! \n" << WSAGetLastError();
		return -1;
	}

	char buf[BUF_SIZE] = { 0 };
	int k = 1;
	// Цикл обработки пакетов
	while (true) {
		sockaddr_in from;
		int len = sizeof(from);
		int bsz = recvfrom(s, &buf[0], BUF_SIZE - 1, 0, (sockaddr*)&from, &len);
		if (bsz == SOCKET_ERROR) {
			cout << "Ошибка приема сообщения! \n" << WSAGetLastError();
			return -1;
		}

		// Определяем параметры клиента
		HOSTENT* hst;
		hst = gethostbyaddr((char*)&from.sin_addr, 4, AF_INET);
		cout << "New Datagram: \n" << ((hst) ? hst->h_name : "Unknown host") << endl
			<< inet_ntoa(from.sin_addr) << endl << ntohs(from.sin_port) << endl;

		buf[bsz] = '\0';
		cout << "Datagram: " << buf << endl;

		string msg = "Datagramm " + to_string(k) + " is received: " + buf;
		k++;
		sendto(s, (char*)&msg[0], msg.size(), 0, (sockaddr*)&from, sizeof(from));
	}
	return 0;
}
