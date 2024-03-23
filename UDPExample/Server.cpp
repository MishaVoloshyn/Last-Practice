#include <winsock2.h>
#include <iostream>
#include <vector>
#include <map>
#include <cctype>
#include <sstream>

#include <string>

using namespace std;

#define MAX_CLIENTS 30
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib") 
#pragma warning(disable:4996) 

SOCKET server_socket;

struct MenuItem {
    string name;
    double price;
    int cookTime; 
};

map<string, MenuItem> menu = {
    {"burger", {"burger", 2.50, 5}},
    {"tea", {"tea", 1.25, 1}},
    {"coffee", {"coffee", 1.50, 1}},
    {"pancake", {"pancake", 2.20, 2}},
    {"sprite", {"sprite", 1.15, 1}},
    {"cola", {"cola", 1.25, 1}},
    {"sandwich", {"sandwich", 2.45, 2}}
};

vector<pair<string, int>> orders_queue; // Очередь заказов 

int main() {
    system("title Server");

    puts("Start server... DONE.");
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d", WSAGetLastError());
        return 1;
    }

    // create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d", WSAGetLastError());
        return 2;
    }

    // prepare the sockaddr_in structure
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // bind socket
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

        int t = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET s = client_socket[i];
           
            if (FD_ISSET(s, &readfds)) {
                char client_message[DEFAULT_BUFLEN];

                int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
                client_message[client_message_length] = '\0';
                string message = client_message;
                
                // Обработка заказа
                istringstream stream(message);
                string word;
                vector<string> items;

                for (int i = 0; i < menu.size(); i++)
                {
                    while (stream >> word) 
                    {
                        items.push_back(word);
                    }
                }
                
                int totalCookTime = 0;
                double totalPrice = 0;
                for (const string& item : items) 
                {

                    auto it = menu.find(item);
                    if (it != menu.end()) 
                    {
                        totalCookTime += it->second.cookTime;
                        totalPrice += it->second.price;
                        orders_queue.push_back({ item, it->second.cookTime });
                    }
                }

                if (totalCookTime == 0)
                {
                    string txt = "Извините, у нас нет такой позиции. Выберите что-то другое";
                    send(s, txt.c_str(), txt.length(), 0);
                }
                else
                {
                    string mes = to_string(totalCookTime) + " " + to_string(totalPrice);
                    send(s, mes.c_str(), mes.length(), 0);
                }

            }
        }
    }

    WSACleanup();
}