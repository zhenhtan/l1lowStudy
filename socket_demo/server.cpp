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
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::perror("setsockopt");
        close(server_fd);
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(kPort);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 1) < 0) {
        std::perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "server listening on port " << kPort << std::endl;

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    const int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
        std::perror("accept");
        close(server_fd);
        return 1;
    }

    std::cout << "client connected: " << inet_ntoa(client_addr.sin_addr)
              << ":" << ntohs(client_addr.sin_port) << std::endl;

    char buffer[kBufferSize] = {};
    const ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received < 0) {
        std::perror("recv");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    buffer[received] = '\0';
    std::cout << "server received: " << buffer << std::endl;

    const std::string response = std::string("server reply: hello, client. I got -> ") + buffer;
    if (send(client_fd, response.c_str(), response.size(), 0) < 0) {
        std::perror("send");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
