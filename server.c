#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *response = "Hello from server";

    // Создаем сокет
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Привязываем сокет к порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Ожидаем подключения
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    // Принимаем входящее соединение
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Чтение данных от клиента
    read(new_socket, buffer, 1024);
    printf("Received: %s\n", buffer);

    // Отправка ответа клиенту
    send(new_socket, response, strlen(response), 0);
    printf("Response sent\n");

    close(new_socket);
    close(server_fd);
    return 0;
}
