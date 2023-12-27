#include <iostream>
#include <string>

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <WinSock2.h>
#include <Windows.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#define BUF_SIZE 2048

#define SRV_PORT 666
#define SRV_ADDR "127.0.0.1"

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


int main() {
	setlocale(LC_ALL, "rus");
	// Оформление окна
	cout << "\t Клиент\n";
	cout << "Типы взаимодействий с сервером: " << endl;
	cout << "1: Эхо-сервер" << endl;
	cout << "2: Деканат" << endl;
	cout << "Команда выхода: exit \n";
	for (int i = 0; i < 30; i++)
		cout << "-";
	cout << endl;

	int type;
	cout << "Выберите тип взаимодействия с сервером: ";
	cin >> type;
	string str;
	getline(cin, str); // Считываем перевод строки

	// Инициализируем библиотеку сокетов
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws)) {
		cout << "Ошибка WSAStartup! \n" << WSAGetLastError();
		return -1;
	}

	// Создание сокета
	SOCKET s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		cout << "Ошибка socket! \n" << WSAGetLastError();
		WSACleanup();
		return -1;
	}

	// Указание адреса и порта сервера
	sockaddr_in srvAddr;
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(SRV_PORT);

	HOSTENT* hst;

	// Преобразование IP-адреса из символьного в сетевой формат
	if (inet_addr(SRV_ADDR) != INADDR_NONE)
		srvAddr.sin_addr.s_addr = inet_addr(SRV_ADDR);
	else {
		// Попытка получить IP-адрес по доменному имени
		if (hst = gethostbyname(SRV_ADDR))
			((unsigned long*)&srvAddr.sin_addr)[0] =
			((unsigned long**)hst->h_addr_list)[0][0];
		else {
			cout << "Неверный сетевой адрес! \n" << WSAGetLastError();
			closesocket(s);
			WSACleanup();
			return -1;
		}
	}

	// Установление соединения
	if (connect(s, (sockaddr*)&srvAddr, sizeof(srvAddr))) {
		cout << "Ошибка connect! \n" << WSAGetLastError();
		closesocket(s);
		WSACleanup();
		return -1;
	}

	int len = 0;
	char buf[BUF_SIZE + 1] = { 0 };
	// Цикл получения сообщений
	while (true) {
		len = recv(s, &buf[0], BUF_SIZE, 0);
		if (len == SOCKET_ERROR) {
			cout << "Ошибка recv! \n" << WSAGetLastError();
			closesocket(s);
			WSACleanup();
			return -1;
		}

		// Вывод сообщения от сервера
		buf[len] = '\0';
		cout << "(Сервер): " << buf << endl;


		Message msg;
		msg.type = type;

		if (type == 1) {
			cout << "Введите сообщение: " << endl;
			// Ввод сообщения пользователя
			cin.getline(msg.message, 255);
			if (string(msg.message) == "exit") {
				// Отправка сообщения
				send(s, (char*)&msg, sizeof(msg), 0);
				cout << "Exit... \n";
				break;
			}
		}
		if (type == 2) {
			cout << "Введите фамилию: ";
			cin.getline(msg.st.fam, 20);
			if (string(msg.st.fam) == "exit") {
				// Отправка сообщения
				send(s, (char*)&msg, sizeof(msg), 0);
				cout << "Exit... \n";
				break;
			}
			cout << "Введите четыре оценки: ";
			for (int i = 0; i < 4; i++)
				cin >> msg.st.vals[i];
			getline(cin, str); // Перевод строки
		}

		// Отправка сообщения
		send(s, (char*)&msg, sizeof(msg), 0);

	}

	closesocket(s);
	WSACleanup();
	return 0;
}
