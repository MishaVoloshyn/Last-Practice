#include <winsock2.h>
#include <iostream>
#include <vector>
#include <cctype>
#include <string>

using namespace std;

#define MAX_CLIENTS 30
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib ")
#pragma warning(disable:4996)

SOCKET server_socket;

vector<string> menuNames = { "mucflurry", "latte", "chikenburger", "free potato", "cola", "coffee", "cherry pie" };
vector<double> menuPrices = { 3.50, 1.25, 3.50, 2.40, 1.15, 1.25, 2.45 };
vector<int> menuCookTimes = { 5, 2, 7, 4, 1, 3, 5 };

vector<pair<string, int>> orders_queue;

int main() {
    system("title Server");

    puts("Start server... DONE.");
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d", WSAGetLastError());
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d", WSAGetLastError());
        return 2;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d", WSAGetLastError());
        return 3;
    }

    listen(server_socket, MAX_CLIENTS);

    puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

    fd_set readfds;
    SOCKET client_socket[MAX_CLIENTS] = {};
    int currentClientIndex = 0;
    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            SOCKET s = client_socket[i];
            if (s > 0) {
                FD_SET(s, &readfds);
                currentClientIndex = i;
            }
        }

        if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            printf("select function call failed with error code : %d", WSAGetLastError());
            return 4;
        }

        SOCKET new_socket;
        sockaddr_in address;
        int addrlen = sizeof(sockaddr_in);
        if (FD_ISSET(server_socket, &readfds)) {
            if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
                perror("accept function error");
                return 5;
            }

            cout << "New client connected" << endl;

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET s = client_socket[i];

            if (FD_ISSET(s, &readfds)) {
                char client_message[DEFAULT_BUFLEN];

                int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
                client_message[client_message_length] = '\0';

                for (int i = 0; i < client_message_length; ++i) {
                    client_message[i] = tolower(client_message[i]);
                }

                vector<string> items;
                char* token = strtok(client_message, " ");
                while (token != NULL) {
                    items.push_back(token);
                    token = strtok(NULL, " ");
                }

                int allTime = 0;
                double allMoney = 0;
                for (const string& item : items)
                {
                    auto it = find(menuNames.begin(), menuNames.end(), item);
                    if (it != menuNames.end())
                    {
                        int index = distance(menuNames.begin(), it);
                        allTime += menuCookTimes[index];
                        allMoney += menuPrices[index];
                        orders_queue.push_back({ item, menuCookTimes[index] });
                    }
                }

                if (allTime == 0)
                {
                    char txt[200] = "Извините, у нас нет такой позиции. Выберите что-то другое";
                    send(s, txt, 200, 0);
                }
                else
                {
                    string mes = to_string(allTime) + " " + to_string(allMoney);
                    send(s, mes.c_str(), mes.length(), 0);
                }

            }
        }
    }

    WSACleanup();
}
