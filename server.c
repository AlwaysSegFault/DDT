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

int main() {
    // Инициализация окна
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Сервер - Простая мультиплеерная игра");

    // Позиция квадрата для сервера
    Position serverPosition = {screenWidth / 4.0f, screenHeight / 2.0f};
    // Позиция квадрата для клиента
    Position clientPosition = {screenWidth / 1.5f, screenHeight / 2.0f};
    const float squareSize = 50.0f;

    // Настроим сервер для UDP
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);  // Создаем UDP сокет
    if (server_fd < 0) {
        printf("Ошибка создания сокета\n");
        return -1;
    }

    // Адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Ошибка биндинга\n");
        return -1;
    }

    printf("Сервер запущен. Ожидание данных от клиента...\n");

    // Главный игровой цикл
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // Обработка ввода для сервера (движение квадрата сервера)
        if (IsKeyDown(KEY_W)) serverPosition.y -= 5.0f;
        if (IsKeyDown(KEY_S)) serverPosition.y += 5.0f;
        if (IsKeyDown(KEY_A)) serverPosition.x -= 5.0f;
        if (IsKeyDown(KEY_D)) serverPosition.x += 5.0f;

        // Получаем данные от клиента (позиция клиента)
        int bytesReceived = recvfrom(server_fd, &clientPosition, sizeof(Position), 0,
                                      (struct sockaddr*)&client_addr, &client_addr_len);

        if (bytesReceived > 0) {
            // Отправляем только свою позицию (позицию сервера) обратно клиенту
            sendto(server_fd, &serverPosition, sizeof(Position), 0,
                   (struct sockaddr*)&client_addr, client_addr_len);
        }

        // Начало рисования
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Отображаем квадраты
        DrawText("Сервер", 10, 10, 20, DARKGRAY);
        DrawRectangleV((Vector2){serverPosition.x, serverPosition.y}, (Vector2){squareSize, squareSize}, BLUE);  // Синий квадрат для сервера
        DrawRectangleV((Vector2){clientPosition.x, clientPosition.y}, (Vector2){squareSize, squareSize}, RED);  // Красный квадрат для клиента

        EndDrawing();
    }

    // Закрытие соединения
    close(server_fd);
    CloseWindow();

    return 0;
}

