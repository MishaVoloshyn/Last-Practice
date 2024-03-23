#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;
bool Wait = false;

// Очередь для хранения сообщений от клиентов
queue<string> messages_queue;

bool isOrderInProgress = false;

DWORD WINAPI Sender(void* client_socket_ptr)
{

    while (true)
    {
        char query[DEFAULT_BUFLEN];
        cin.getline(query, DEFAULT_BUFLEN);
        send(client_socket, query, strlen(query), 0);

    }
}
DWORD WINAPI Receiver(void* param)
{
    while (true)
    {
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        if (result == SOCKET_ERROR) {
            return 1;
        }
        response[result] = '\0';

        int totalCookTime = 0;
        double price = 0;

        char* spacePos = strchr(response, ' '); 
        bool isZero = false;

        if (spacePos != nullptr)
        {
            *spacePos = '\0'; 
            spacePos++; 

            totalCookTime = atoi(response); 
            price = atof(spacePos); 

           
            if (totalCookTime == 0 || price == 0)
            {
                isZero = true; 
            }

            if (totalCookTime > 0)
            {
                cout << "Время ожидания составит " << totalCookTime << " секунд, с вас - " << price << "$" << endl;
            }
            else
            {
                cout << "Ваш заказ готов!" << endl;
            }

            if (isZero)
            {
                cout << "Извините, данной позиции нет в меню. Пожалуйста, выберите что-то другое." << endl;
            }
            
        }
  
    }

}

int main()
{
    system("title Client");
    setlocale(LC_CTYPE, "Russian");
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (client_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }
        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 4;
    }

    cout << "Добро пожаловать в наше кафе! Что желаете заказать?" << endl;
    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);

    return 0;
}