#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace {
constexpr int kPort = 9090;
constexpr int kBufferSize = 1024;
}

int main()
{
    const int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        std::perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(kPort);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        std::perror("inet_pton");
        close(client_fd);
        return 1;
    }

    if (connect(client_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::perror("connect");
        close(client_fd);
        return 1;
    }

    const std::string message = "hello from client";
    if (send(client_fd, message.c_str(), message.size(), 0) < 0) {
        std::perror("send");
        close(client_fd);
        return 1;
    }

    char buffer[kBufferSize] = {};
    const ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received < 0) {
        std::perror("recv");
        close(client_fd);
        return 1;
    }

    buffer[received] = '\0';
    std::cout << "client received: " << buffer << std::endl;

    close(client_fd);
    return 0;
}
