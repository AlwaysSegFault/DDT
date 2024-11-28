#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 256

// Структура для хранения позиции квадрата
typedef struct {
    float x;
    float y;
} Position;

#define SERVER_IP "172.28.123.89"  // Задаем IP сервера

int main() {
    // Инициализация окна
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Клиент - Простая мультиплеерная игра");

    // Позиция квадрата для клиента
    Position clientPosition = {screenWidth / 1.5f, screenHeight / 2.0f};
    const float squareSize = 50.0f;

    // Настроим клиент для UDP
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // Создаем UDP сокет
    if (sockfd < 0) {
        printf("Ошибка создания сокета\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  // Используем конкретный IP-адрес

    // Начальный клиентский квадратик
    Position serverPosition = {0, 0};  // Позиция сервера, которую мы будем обновлять

    // Главный игровой цикл
    SetTargetFPS(60);  // Устанавливаем FPS
    while (!WindowShouldClose()) {
        // Обработка ввода для клиента (движение квадратом клиента)
        if (IsKeyDown(KEY_W)) clientPosition.y -= 5.0f;
        if (IsKeyDown(KEY_S)) clientPosition.y += 5.0f;
        if (IsKeyDown(KEY_A)) clientPosition.x -= 5.0f;
        if (IsKeyDown(KEY_D)) clientPosition.x += 5.0f;

        // Отправка позиции квадрата клиента на сервер
        sendto(sockfd, &clientPosition, sizeof(Position), 0, 
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        // Получаем позицию сервера
        int bytesReceived = recvfrom(sockfd, &serverPosition, sizeof(Position), 0,
                                      (struct sockaddr*)&server_addr, &server_addr_len);

        if (bytesReceived > 0) {
            // Начало рисования
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText("Клиент", 10, 10, 20, DARKGRAY);
            DrawRectangleV((Vector2){clientPosition.x, clientPosition.y}, (Vector2){squareSize, squareSize}, RED);   // Красный квадрат для клиента
            DrawRectangleV((Vector2){serverPosition.x, serverPosition.y}, (Vector2){squareSize, squareSize}, BLUE);  // Синий квадрат для сервера

            EndDrawing();
        }
    }

    // Закрытие соединения
    close(sockfd);
    CloseWindow();

    return 0;
}

