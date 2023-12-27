#include <iostream>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

#define PORT 666 // Порт сервера
#define SRV_ADDR "127.0.0.1"
#define BUF_SIZE 1024

using namespace std;

int main() {
    setlocale(LC_ALL, "rus");

    // Оформление окна
    cout << "\t UDP-Клиент\n";
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


    HOSTENT* hst;
    sockaddr_in srvAddr;
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(PORT);

    // Определение IP-адреса сервера
    if (inet_addr(SRV_ADDR)) {
        srvAddr.sin_addr.s_addr = inet_addr(SRV_ADDR);
    }
    else {
        if (hst = gethostbyname(SRV_ADDR)) {
            srvAddr.sin_addr.s_addr = ((unsigned long**)hst->h_addr_list)[0][0];
        }
        else {
            cout << "Unknown Host: " << WSAGetLastError() << "\n";
            closesocket(s);
            WSACleanup();
            return -1;
        }
    }

    // Обмен сообщениями с сервером
    string msg;
    while (true) {
        cout << "Введите сообщение: " << endl;
        getline(cin, msg);
        if (msg == "exit") {
            break;
        }

        // Передача сообщения
        sendto(s, (char*)&msg[0], msg.size(), 0, (sockaddr*)&srvAddr, sizeof(srvAddr));

        // Прием ответа
        sockaddr_in ansAddr;
        char buf[BUF_SIZE] = { 0 };
        int len = sizeof(ansAddr);
        int bsz = recvfrom(s, buf, sizeof(buf), 0, (sockaddr*)&ansAddr, &len);
        if (bsz == SOCKET_ERROR) {
            cout << "Ошибка приема сообщения! \n" << WSAGetLastError() << "\n";
            closesocket(s);
            WSACleanup();
            return -1;
        }
        buf[bsz] = '\0';
        cout << "Answer: " << buf << endl;
    }
    closesocket(s);
    WSACleanup();
    return 0;
}
