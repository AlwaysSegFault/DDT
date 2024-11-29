#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <raylib.h>

#define SERVER_PORT 12345
#define MAX_PLAYERS 16
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    float x, y;
    int player_id;
    int is_active;
} PlayerData;

PlayerData players[MAX_PLAYERS];

void *server_loop(void *arg) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[sizeof(PlayerData)];

    // Инициализация массива игроков
    memset(players, 0, sizeof(players));

    // Создание сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Инициализация адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Привязка сокета
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    while (1) {
        // Получение данных от клиента
        recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);

        PlayerData *incoming_player = (PlayerData *)buffer;
        int player_id = -1;

        // Находим первого свободного игрока, если есть
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (players[i].is_active == 0) {
                player_id = i;
                break;
            }
        }

        // Если нет свободного места, перезаписываем данные игрока (если это уже активный игрок)
        if (player_id == -1) {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (players[i].player_id == incoming_player->player_id) {
                    player_id = i;
                    break;
                }
            }
        }

        if (player_id == -1) {
            // Если игрок не найден и нет места, игнорируем запрос
            continue;
        }

        // Обновляем данные игрока
        players[player_id] = *incoming_player;
        players[player_id].is_active = 1;

        // Отправка обновлений всем игрокам
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (players[i].is_active) {
                sendto(sockfd, &players[i], sizeof(PlayerData), 0, (struct sockaddr *)&client_addr, client_len);
            }
        }
    }

    close(sockfd);
    return NULL;
}

int main(void) {
    // Инициализация RayLib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "UDP Game Server");
    SetTargetFPS(60);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_loop, NULL);

    // Игровой цикл
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Отображение игроков
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (players[i].is_active) {
                DrawCircle(players[i].x, players[i].y, 20, BLUE);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

