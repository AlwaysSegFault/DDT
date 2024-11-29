#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <raylib.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    float x, y;
    int player_id;
} PlayerData;

int main(void) {
    // Инициализация RayLib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "UDP Game Client");
    SetTargetFPS(60);

    int sockfd;
    struct sockaddr_in server_addr;
    PlayerData player;
    player.x = SCREEN_WIDTH / 2;
    player.y = SCREEN_HEIGHT / 2;
    player.player_id = -1;  // Изначально ID = -1

    // Создание сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Инициализация адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Получаем свой уникальный ID
    while (player.player_id == -1) {
        sendto(sockfd, &player, sizeof(PlayerData), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        recvfrom(sockfd, &player, sizeof(PlayerData), 0, NULL, NULL);  // Получаем свой ID
    }

    while (!WindowShouldClose()) {
        // Управление персонажем
        if (IsKeyDown(KEY_RIGHT)) player.x += 5.0f;
        if (IsKeyDown(KEY_LEFT)) player.x -= 5.0f;
        if (IsKeyDown(KEY_UP)) player.y -= 5.0f;
        if (IsKeyDown(KEY_DOWN)) player.y += 5.0f;

        // Отправка данных о игроке на сервер
        sendto(sockfd, &player, sizeof(PlayerData), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Отображение экрана
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircle(player.x, player.y, 20, BLUE);  // Отображение квадратика
        EndDrawing();
    }

    close(sockfd);
    CloseWindow();
    return 0;
}

