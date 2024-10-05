#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256

void error(const char *msg) {
    printf("%s: %d\n", msg, WSAGetLastError());
    exit(1);
}

int main() {
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        error("WSAStartup failed.");
    }

    SOCKET client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        error("Client socket creation failed.");
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        error("Connection to the server failed.");
    }

    printf("Connected to the server!\n");

    char buffer[BUFFER_SIZE];
    int n;

    while (1) {
        // Clear buffer and get user input
        memset(buffer, 0, BUFFER_SIZE);
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Send message to the server
        n = send(client_socket, buffer, strlen(buffer), 0);
        if (n == SOCKET_ERROR) {
            error("Error sending data.");
        }

        // // Receive response from the server
        // memset(buffer, 0, BUFFER_SIZE);
        // n = recv(client_socket, buffer, BUFFER_SIZE, 0);
        // if (n == SOCKET_ERROR) {
        //     error("Error receiving data.");
        // }

        // printf("Server: %s\n", buffer);
    }

    // Close the socket
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
