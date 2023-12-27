#include <iostream>
#include <string>
#include <vector>

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <WinSock2.h>
#include <Windows.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#define MP make_pair
#define LISTEN_PORT 123
#define CAPACITY 100	// Вместимость сервера
#define SERVERNAME "__SERVER__"

using namespace std;

vector < pair <SOCKET, string> > connections; // Сокеты и ники клиентов
int nClients = 0;

enum Type { Login, Greeting, CommonMessage, PrivateMessage, UserCome, UserGone };

struct Message {
	Type type;
	char from[20];
	char to[20];
	char message[2048];
};

DWORD WINAPI connectToClient(LPVOID clSock) {
	SOCKET sClient = ((SOCKET*)clSock)[0];

	// Предлагаем клиенту представиться
	Message login;
	login.type = Login;
	strcpy(login.from, SERVERNAME);
	strcpy(login.message, "Введите никнейм: \0");
	send(sClient, (char*)&login, sizeof(login), 0);

	Message getNickname;
	int len = 0;
	if ((len = recv(sClient, (char*)&getNickname, sizeof(getNickname), 0)) == SOCKET_ERROR) {
		cout << "Клиент не отвечает! \n";
		nClients--;
		closesocket(sClient);
		return -1;
	}

	string nickname = getNickname.from;
	// Увеличиваем число клиентов онлайн
	nClients++;
	connections.push_back(MP(sClient, nickname));
	cout << "Новое подключение! \n";
	cout << nClients << " пользователей" << endl;
	int idx = nClients - 1;

	// Создаем и посылаем приветствие клиенту
	Message greeting;
	greeting.type = Greeting;
	strcpy(greeting.from, SERVERNAME);
	string hello = "Добро пожаловать в чат, ";
	hello += nickname;
	strcpy(greeting.message, hello.c_str());
	send(sClient, (char*)&greeting, sizeof(greeting), 0);


	// Создаем и посылаем всем клиентам сообщение о прибытии текущего клиента
	Message userCome;
	userCome.type = UserCome;
	strcpy(userCome.from, nickname.c_str());
	string uCmMsg = nickname;
	uCmMsg += " зашел в чат";
	strcpy(userCome.message, uCmMsg.c_str());
	for (int i = 0; i < connections.size(); i++) {
		if (i == idx)
			continue;
		send(connections[i].first, (char*)&userCome, sizeof(userCome), 0);
	}

	// Цикл обработки сообщений
	Message msg;
	while (true) {
		// Получаем очередное сообщение от клиента
		len = recv(sClient, (char*)&msg, sizeof(msg), 0);
		if (len == SOCKET_ERROR || (string)msg.message == "exit") {
			break;
			/*
			cout << "Ошибка recv! \n" << WSAGetLastError();
			closesocket(sClient);
			WSACleanup();
			return -1;
			*/
		}


		// Если это обычное сообщение, то посылаем его всем клиентам
		if (msg.type == CommonMessage) {
			for (int i = 0; i < connections.size(); i++) {
				if (i == idx)
					continue;
				send(connections[i].first, (char*)&msg, sizeof(msg), 0);
			}
		}

		// Если это приватное сообщение, то посылаем его указанному клиенту
		if (msg.type == PrivateMessage) {
			string to = msg.to;
			for (int i = 0; i < connections.size(); i++)
				if (connections[i].second == to)
					send(connections[i].first, (char*)&msg, sizeof(msg), 0);
		}
	}

	// Клиент вышел
	nClients--;
	// Создаем и посылаем всем клиентам сообщение о прибытии текущего клиента
	Message userGone;
	userGone.type = UserGone;
	strcpy(userGone.from, nickname.c_str());
	string uGnMsg = nickname;
	uGnMsg += " покинул чат";

	cout << uGnMsg << endl;
	cout << nClients << " пользователей" << endl;

	strcpy(userGone.message, uGnMsg.c_str());
	for (int i = 0; i < connections.size(); i++) {
		if (i == idx)
			continue;
		send(connections[i].first, (char*)&userGone, sizeof(userGone), 0);
	}
	closesocket(sClient);
	return 0;
}

int main() {
	setlocale(LC_ALL, "rus");
	// Оформление окна
	cout << "\t Сервер\n";
	for (int i = 0; i < 30; i++)
		cout << "-";
	cout << endl;

	// Инициализируем библиотеку сокетов
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws)) {
		cout << "Ошибка WSAStartup! \n" << WSAGetLastError();
		return -1;
	}

	// Создание прослушивающего сокета
	SOCKET sListener;
	if ((sListener = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		cout << "Ошибка socket! \n" << WSAGetLastError();
		WSACleanup();
		return -1;
	}

	// Создаем локальный адрес сервера
	sockaddr_in srvAdr;
	srvAdr.sin_family = AF_INET;
	srvAdr.sin_port = htons(LISTEN_PORT);
	srvAdr.sin_addr.s_addr = inet_addr("127.0.0.1");


	// Связываем сокет с адресом сервера
	if (bind(sListener, (sockaddr*)&srvAdr, sizeof(srvAdr))) {
		cout << "Ошибка bind! \n" << WSAGetLastError();
		closesocket(sListener);
		WSACleanup();
		return -1;
	}

	// Инициализируем слушающий сокет
	if (listen(sListener, CAPACITY)) {
		cout << "Ошибка listen! \n" << WSAGetLastError();
		closesocket(sListener);
		WSACleanup();
		return -1;
	}

	cout << "Сервер готов к работе" << endl;

	// Создаем сокет и адрес клиента
	SOCKET sNewClient;		// Сокет клиента
	sockaddr_in adrClient;	// Адрес клиента (заполнится автоматически при вызове)
	int len = sizeof(adrClient);

	//while (true) {
	while (true) {
		// Принимаем входящее соединение
		if ((sNewClient = accept(sListener, (sockaddr*)&adrClient, &len)) == INVALID_SOCKET) {
			cout << "Ошибка accept! \n" << WSAGetLastError();
			break;
		}

		//Создаем новый поток для обслуживания клиента
		DWORD threadId;
		int number = connections.size();
		CreateThread(NULL, NULL, connectToClient, &sNewClient, NULL, &threadId);
	}

	closesocket(sListener);
	WSACleanup();
	return 0;
}