#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256

SOCKET server, client;

void error(const char* msg) {
    printf("%s: %d\n", msg, WSAGetLastError());
    exit(1);
}

void setup_server() {
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        error("WSAStartup failed.");
    }

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        error("Server socket creation failed.");
    }

    u_short port = 8080;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 0;
    addr.sin_port = htons(port);

    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        error("Bind failed.");

    if (listen(server, 8) == SOCKET_ERROR) {
        error("Listen failed.");
    }

    printf("Server is listening on port %d...\n", port);
    client = accept(server, NULL, NULL);
    if (client == INVALID_SOCKET) {
        error("Failed on Accept.");
    }

    printf("Client connected!\n");

    // Set the socket to non-blocking mode
    u_long mode = 1; // 1 to enable non-blocking mode
    if (ioctlsocket(client, FIONBIO, &mode) == SOCKET_ERROR) {
        error("Failed to set non-blocking mode.");
    }
}


char buffer[BUFFER_SIZE];
int n;
void update_server() {
    memset(buffer, 0, BUFFER_SIZE);


    n = recv(client, buffer, BUFFER_SIZE, 0);
    if (n == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        if (error_code == WSAEWOULDBLOCK) {
            return;
        }
        else {
            error("Error on Read.");
        }
    }

    printf("Client : %s\n", buffer);

    //fgets(buffer, BUFFER_SIZE, stdin);

    //n = send(client, buffer, strlen(buffer), 0);
    //if (n == SOCKET_ERROR)
    //    error("Error on Send.");
}

void close_server() {

    closesocket(client);
    closesocket(server);

    WSACleanup();
}