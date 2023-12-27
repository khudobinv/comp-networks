#include <iostream>
#include <string>

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <WinSock2.h>
#include <Windows.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#define LISTENPORT 666
#define BUF_SIZE 2048

using namespace std;

struct Student {//структура данных для передачи
	char fam[20];
	int vals[4];
};

struct Message {//структура данных для сообщения
	int type;
	char message[256];
	Student st;
};

int nClients = 0;//начальное число клиентов

void printUsers() {
	if (nClients > 0)
		cout << nClients << " users online" << endl;
	else
		cout << "No users online" << endl;
	cout << endl;
}

// Прототип функции потока, обслуживающей подключившихся пользователей
DWORD WINAPI connectToClient(LPVOID clientSocket) {
	SOCKET cSocket = ((SOCKET*)clientSocket)[0];
	Message msg;
	char greeting[] = "Добрый день! Сервер готов к работе \r\n";
	int len = 0;

	// Отправляем сообщение клиенту
	send(cSocket, greeting, sizeof(greeting), 0);

	while ((len = recv(cSocket, (char*)&msg, sizeof(msg), 0)) != SOCKET_ERROR) {
		if (msg.type == 1) {
			cout << "(Пользователь): " << msg.message << endl << endl;
			string ans = "Получено сообщение: ";
			ans += msg.message;
			send(cSocket, ans.c_str(), ans.length(), 0);
		}
		if (msg.type == 2) {
			Student y = msg.st;

			if (string(y.fam) == "exit")
				break;

			cout << "Получены данные об успеваемости студента " << y.fam << endl;
			int mnv = 5;
			for (int i = 0; i < 4; i++)
				mnv = min(mnv, y.vals[i]);

			string ans;
			if (mnv <= 2)
				ans = "Имеет задолженность!";
			if (mnv == 3)
				ans = "Нет задолженностей, нет стипендии";
			if (mnv == 4)
				ans = "Нет задолженностей, есть стипендия: 1500 рублей";
			if (mnv >= 5)
				ans = "Нет задолженностей, есть стипендия: 2200 рублей";
			send(cSocket, ans.c_str(), ans.length(), 0);
		}
	}

	// Клиент вышел
	nClients--;
	cout << "Пользователь вышел" << endl;
	printUsers();
	closesocket(cSocket);
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

	// Создание сокета
	SOCKET sListener;
	if ((sListener = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		cout << "Ошибка socket! \n" << WSAGetLastError();
		WSACleanup();
		return -1;
	}

	// Создаем локальный адрес сервера
	sockaddr_in sLocal;
	sLocal.sin_family = AF_INET;
	sLocal.sin_port = htons(LISTENPORT);
	sLocal.sin_addr.s_addr = 0;

	// Связываем сокет с адресом сервера
	if (bind(sListener, (sockaddr*)&sLocal, sizeof(sLocal))) {
		cout << "Ошибка bind! \n" << WSAGetLastError();
		closesocket(sListener);
		WSACleanup();
		return -1;
	}

	// Инициализируем слушающий сокет
	if (listen(sListener, 0x100)) {
		cout << "Ошибка listen! \n" << WSAGetLastError();
		closesocket(sListener);
		WSACleanup();
		return -1;
	}

	cout << "Waiting for connection..." << endl;

	// Создаем сокет и адрес клиента
	SOCKET sClient;		// Сокет клиента
	sockaddr_in adrClient;	// Адрес клиента (заполнится автоматически при вызове)
	char buf[BUF_SIZE + 1] = { 0 };
	int len = sizeof(adrClient);

	// Цикл приема клиентов
	while (true) {
		// Принимаем входящее соединение
		if ((sClient = accept(sListener, (sockaddr*)&adrClient, &len)) == INVALID_SOCKET) {
			cout << "Ошибка accept! \n" << WSAGetLastError();
			closesocket(sListener);
			WSACleanup();
			return -1;
		}

		// Увеличиваем число клиентов онлайн
		nClients++;
		cout << "Новое подключение! \n";

		// Получаем сведения о подключившемся клиенте
		HOSTENT* hst = gethostbyaddr((char*)&adrClient.sin_addr.s_addr, 4, AF_INET);

		// Выводим сведения о подключившемся клиенте и общее число клиентов
		if (hst)
			cout << hst->h_name << ' ';
		cout << inet_ntoa(adrClient.sin_addr) << endl;
		printUsers();

		//Создаем новый поток для обслуживания клиента
		DWORD threadId;
		CreateThread(NULL, NULL, connectToClient, &sClient, NULL, &threadId);
	}

	closesocket(sListener);
	WSACleanup();
	return 0;
}