#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 256
#define SPEED 20
#define screenWidth 1920
#define screenHeight 1080
// Структура для хранения позиции квадрата
typedef struct {
    float x;
    float y;
} Position;

#define SERVER_IP "172.28.123.89"  // Задаем IP сервера

Position clientPosition = {screenWidth / 1.5f, screenHeight / 2.0f};
const float squareSize = 50.0f;

void move_client_position() {
    // Пределы поля
    if (clientPosition.x < 0) clientPosition.x = 0;
    if (clientPosition.x > screenWidth - squareSize) clientPosition.x = screenWidth - squareSize;
    if (clientPosition.y < 0) clientPosition.y = 0;
    if (clientPosition.y > screenHeight - squareSize) clientPosition.y = screenHeight - squareSize;
}

// Проверка на коллизию между клиентом и сервером
int check_collision(Position serverPos) {
    return (clientPosition.x < serverPos.x + squareSize &&
            clientPosition.x + squareSize > serverPos.x &&
            clientPosition.y < serverPos.y + squareSize &&
            clientPosition.y + squareSize > serverPos.y);
}

void handle_collision(Position serverPos) {
    if (check_collision(serverPos)) {
        // Останавливаем движение клиента по направлениям
        if (clientPosition.x < serverPos.x) clientPosition.x = serverPos.x + squareSize;
        else if (clientPosition.x > serverPos.x) clientPosition.x = serverPos.x - squareSize;

        if (clientPosition.y < serverPos.y) clientPosition.y = serverPos.y + squareSize;
        else if (clientPosition.y > serverPos.y) clientPosition.y = serverPos.y - squareSize;
    }
}

int main() {

    // Инициализация полноэкранного окна
    InitWindow(screenWidth, screenHeight, "Клиент - Простая мультиплеерная игра");
    ToggleFullscreen();  // Включаем полноэкранный режим

    // Позиция квадрата для клиента
    

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
        if (IsKeyDown(KEY_W)) clientPosition.y -= SPEED;
        if (IsKeyDown(KEY_S)) clientPosition.y += SPEED;
        if (IsKeyDown(KEY_A)) clientPosition.x -= SPEED;
        if (IsKeyDown(KEY_D)) clientPosition.x += SPEED;

        move_client_position();  // Проверка на выход за пределы экрана
        handle_collision(serverPosition);  // Проверка коллизии с сервером

        sendto(sockfd, &clientPosition, sizeof(Position), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        int bytesReceived = recvfrom(sockfd, &serverPosition, sizeof(Position), 0,
                                      (struct sockaddr*)&server_addr, &server_addr_len);

        if (bytesReceived > 0) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangle((screenWidth - 10) / 2, 0, 10, screenHeight, WHITE);
            DrawRectangleV((Vector2){clientPosition.x, clientPosition.y}, (Vector2){squareSize, squareSize}, RED);
            DrawRectangleV((Vector2){serverPosition.x, serverPosition.y}, (Vector2){squareSize, squareSize}, BLUE);
            EndDrawing();
        }
    }


    // Закрытие соединения
    close(sockfd);
    CloseWindow();

    return 0;
}
