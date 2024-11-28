#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_PLAYERS 10
#define BUFFER_SIZE 256

// Структура для хранения позиции квадрата
typedef struct {
    float x;
    float y;
} Position;

// Структура для игрока
typedef struct {
    Position position;
    Color color;
    int active;  // Флаг активности игрока
} Player;

Player players[MAX_PLAYERS];
int playerCount = 0; // Количество игроков, полученных от сервера

#define SERVER_IP "127.0.0.1"  // Задаем IP сервера

int main() {
    // Инициализация окна
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Клиент - Мультиплеерная игра");

    // Настроим клиент для UDP
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // Создаем UDP-сокет
    if (sockfd < 0) {
        printf("Ошибка создания сокета\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  // Используем конкретный IP-адрес

    // Позиция клиента 
    Position clientPosition = {screenWidth / 2.0f, screenHeight / 2.0f};
    const float squareSize = 50.0f;

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

        // Получаем позиции всех игроков от сервера
        int bytesReceived = recvfrom(sockfd, players, sizeof(players), 0,
                                      (struct sockaddr*)&server_addr, &server_addr_len);

        if (bytesReceived > 0) {
            playerCount = bytesReceived / sizeof(Player);
        }

        // Рисование
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Рисуем квадраты всех активных игроков (кроме себя)
        for (int i = 0; i < playerCount; i++) {
            if (players[i].active && !(players[i].position.x == clientPosition.x && players[i].position.y == clientPosition.y)) {
                DrawRectangleV((Vector2){players[i].position.x, players[i].position.y}, (Vector2){squareSize, squareSize}, players[i].color);
            }
        }

        // Отрисовка квадрата самого клиента
        DrawRectangleV((Vector2){clientPosition.x, clientPosition.y}, (Vector2){squareSize, squareSize}, (Color){255, 0, 0, 255});  // Например, красный

        EndDrawing();
    }

    // Закрытие соединения
    close(sockfd);
    CloseWindow();

    return 0;
}
