// Copied from Bing.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

#define SERVER_PORT 7
#define BUFFER_SIZE 1024

int main(int , char *[]) {
    /* if (argc != 4) {
        std::cout << "Usage: echoclient <address> <port> <message>" << std::endl;
        return 0;
    } */
    const char* ip = "::1";
    int sockfd;
    struct sockaddr_in6 server_addr;
    char buffer[] = "Test";

    // Create a UDP socket
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&enable,sizeof(enable)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERVER_PORT);
    inet_pton(AF_INET6, ip, reinterpret_cast<void *>(&server_addr.sin6_addr) );

    if (connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send message to server
    send(sockfd, buffer, strlen(buffer), 0);

    // Receive echoed message from server
    struct sockaddr_in6 client_addr;
    client_addr.sin6_family = AF_INET6;
    socklen_t buffer_len = sizeof(client_addr);
    auto n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, reinterpret_cast<sockaddr *>(&client_addr), &buffer_len);
    if (n < 0) {
        perror("recv failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    char tempbuffer[1024] { 0 };
    inet_ntop(client_addr.sin6_family, reinterpret_cast<void *>(&client_addr.sin6_addr), tempbuffer, std::size(tempbuffer));
    auto port = htons(client_addr.sin6_port);
    std::cout << "Server echoed: " << buffer << ", address: " << tempbuffer << ", port: " << port << std::endl;

    // Close the socket
    close(sockfd);
    return 0;
}