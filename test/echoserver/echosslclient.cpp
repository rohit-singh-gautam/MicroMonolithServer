// This code is copied from Bing Copilot
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>


int main() {
    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd <= 0) {
        std::cout << "Unable to create socket \n";
        return 0;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4833);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(client_fd, (struct sockaddr*)&addr, sizeof(addr));

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);
    SSL_connect(ssl);

    const char *msg = "Hello, server!";
    SSL_write(ssl, msg, strlen(msg));

    char buf[1024];
    int bytes = SSL_read(ssl, buf, sizeof(buf));
    if (bytes > 0) {
        buf[bytes] = 0;
        std::cout << "Received: " << buf << std::endl;
    }

    SSL_free(ssl);
    close(client_fd);
    SSL_CTX_free(ctx);
    return 0;
}