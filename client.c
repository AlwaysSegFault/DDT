#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "000";
    char buffer[1024] = {0};

    // Создаем сокет
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    // Настройка адреса сервера
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Преобразование IP-адреса
    if (inet_pton(AF_INET, "172.28.117.177", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address or Address not supported\n");
        return -1;
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection failed\n");
        return -1;
    }

    // Отправка сообщения серверу
    send(sock, message, strlen(message), 0);
    printf("Message sent\n");

    // Чтение ответа от сервера
    read(sock, buffer, 1024);
    printf("Received: %s\n", buffer);

    close(sock);
    return 0;
}
