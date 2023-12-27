#include <iostream>
#include <string>
#include <vector>

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <WinSock2.h>
#include <Windows.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#define MP make_pair
#define SRV_PORT 123

using namespace std;

SOCKET connection; // Сокеты и ники клиентов
int nClients = 0;

enum Type { Login, Greeting, CommonMessage, PrivateMessage, UserCome, UserGone };

struct Message {
	Type type;
	char from[20];
	char to[20];
	char message[2048];
};

DWORD WINAPI getMessage(LPVOID sckt) {
	int len = 0;
	Message msg;

	// Цикл общения с сервером
	while (true) {
		len = recv(connection, (char*)&msg, sizeof(msg), 0);
		if (len == SOCKET_ERROR) {
			cout << "Ошибка recv! \n" << WSAGetLastError();
			closesocket(connection);
			WSACleanup();
			return -1;
		}

		if ((string)msg.from != "__SERVER__")
			cout << '(' << (string)msg.from << ") : ";
		cout << (string)msg.message << endl;

	}

}



int main() {
	setlocale(LC_ALL, "rus");
	// Оформление окна
	cout << "\t Клиент\n";
	cout << "PRIVATE - начать приватное общение с пользователем \n";
	cout << "exit - выйти из чата \n";
	for (int i = 0; i < 30; i++)
		cout << "-";
	cout << endl;

	// Инициализируем библиотеку сокетов
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws)) {
		cout << "Ошибка WSAStartup! \n" << WSAGetLastError();
		return -1;
	}

	// Создание сокета
	if ((connection = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		cout << "Ошибка socket! \n" << WSAGetLastError();
		WSACleanup();
		return -1;
	}

	// Указание адреса и порта сервера
	sockaddr_in srvAddr;
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	srvAddr.sin_port = htons(SRV_PORT);

	// Установление соединения
	if (connect(connection, (sockaddr*)&srvAddr, sizeof(srvAddr))) {
		cout << "Ошибка connect! \n" << WSAGetLastError();
		closesocket(connection);
		WSACleanup();
		return -1;
	}

	cout << "Клиент подключен к серверу! \n";

	Message msg;
	// Получаем предложение ввести логин
	int len = recv(connection, (char*)&msg, sizeof(msg), 0);
	if (len == SOCKET_ERROR) {
		cout << "Ошибка recv! \n" << WSAGetLastError();
		closesocket(connection);
		WSACleanup();
		return -1;
	}
	cout << (string)msg.message << endl;

	string nickname;
	Message clMsg;

	if (msg.type == Login) {
		getline(cin, nickname);
		clMsg.type = Login;
		strcpy(clMsg.from, nickname.c_str());
		send(connection, (char*)&clMsg, sizeof(clMsg), 0);
		Sleep(10);
	}

	//Создаем новый поток для получения сообщений
	DWORD threadId;
	CreateThread(NULL, NULL, getMessage, NULL, NULL, &threadId);

	// Продолжаем писать сообщения
	string cMess;
	while (true) {
		//getline(cin, cMess); // Считываем перевод строки
		getline(cin, cMess);
		if (cMess == "PRIVATE") {
			clMsg.type = PrivateMessage;
			cout << "Введите никнейм адресата: ";
			getline(cin, nickname);
			strcpy(clMsg.to, nickname.c_str());
			getline(cin, cMess);
		}
		else
			clMsg.type = CommonMessage;

		strcpy(clMsg.message, cMess.c_str());
		send(connection, (char*)&clMsg, sizeof(clMsg), 0);

		if (cMess == "exit") {
			cout << "Exit... \n";
			break;
		}

		Sleep(10);
	}

	closesocket(connection);
	WSACleanup();
	return 0;
}
