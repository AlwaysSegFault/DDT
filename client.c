#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

volatile int can_input = 0; // Флаг, чтобы включать ввод только в нужное время

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(sock, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            printf("Disconnected from server\n");
            break;
        }

        printf("\n%s\n", buffer); // Отображаем сообщение от сервера
        fflush(stdout);           // Принудительный вывод

        // Если сообщение от сервера — это не победа или ошибка, разрешаем ввод
        if (strstr(buffer, "Invalid move") == NULL && 
            strstr(buffer, "wins") == NULL) {
            can_input = 1;
        }
    }
    pthread_exit(NULL);
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Создаем сокет
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    // Настройка адреса сервера
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Преобразование IP-адреса
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to server\n");
    fflush(stdout);

    // Создаем поток для приема сообщений
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void *)&sock) != 0) {
        perror("Thread creation failed");
        return -1;
    }

    while (1) {
        while (!can_input) {
            usleep(100000); // Ждём, пока ввод не станет доступным
        }

        printf("Your move (1-9): ");
        fflush(stdout); // Принудительно выводим подсказку
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Убираем символ новой строки

        send(sock, buffer, strlen(buffer), 0);
        can_input = 0; // Блокируем ввод до следующего разрешения от сервера
    }

    close(sock);
    return 0;
}
