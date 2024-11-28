#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>  // Для флага O_NONBLOCK

#define PORT 12345
#define MAX_PLAYERS 10
#define BUFFER_SIZE 256
#define UPDATE_INTERVAL 16 // Интервал обновления в миллисекундах (около 60 FPS)

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
    struct sockaddr_in addr; // Адрес игрока (для отправки данных)
} Player;

// Массив из 10 предустановленных цветов
Color colorPalette[10] = {
    (Color){255, 0, 0, 255},     // Красный
    (Color){0, 255, 0, 255},     // Зеленый
    (Color){0, 0, 255, 255},     // Синий
    (Color){255, 255, 0, 255},   // Желтый
    (Color){0, 255, 255, 255},   // Циан
    (Color){255, 0, 255, 255},   // Магента
    (Color){128, 128, 128, 255}, // Серый
    (Color){255, 165, 0, 255},   // Оранжевый
    (Color){0, 0, 0, 255},       // Черный
    (Color){255, 255, 255, 255}  // Белый
};

Player players[MAX_PLAYERS];  // Массив для хранения игроков
int playerCount = 0; // Количество подключенных игроков

// Функция для установки сокета в неблокирующий режим
void setSocketNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return;
    }
    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1) {
        perror("fcntl(F_SETFL)");
    }
}

int main() {
    // Инициализация окна
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Сервер - Мультиплеерная игра");

    // Настроим сервер для UDP
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);  // Создаем UDP сокет
    if (server_fd < 0) {
        printf("Ошибка создания сокета\n");
        return -1;
    }

    // Установим сокет в неблокирующий режим
    setSocketNonBlocking(server_fd);

    // Адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Ошибка биндинга\n");
        return -1;
    }

    printf("Сервер запущен. Ожидание данных от клиентов...\n");

    // Инициализация первого игрока (квадрат для сервера)
    players[0].position = (Position){screenWidth / 4.0f, screenHeight / 2.0f};
    players[0].color = colorPalette[0]; // Присваиваем первый цвет
    players[0].active = 1; // Серверный игрок активен
    playerCount = 1;  // Один игрок (сервер) по умолчанию

    // Главный игровой цикл
    SetTargetFPS(60); // Устанавливаем FPS

    int lastUpdateTime = GetTime() * 1000; // Время последнего обновления (в миллисекундах)

    while (!WindowShouldClose()) {
        int currentTime = GetTime() * 1000; // Текущее время в миллисекундах
        int deltaTime = currentTime - lastUpdateTime;

        // Перемещение квадратика сервера
        if (IsKeyDown(KEY_W)) players[0].position.y -= 5.0f;
        if (IsKeyDown(KEY_S)) players[0].position.y += 5.0f;
        if (IsKeyDown(KEY_A)) players[0].position.x -= 5.0f;
        if (IsKeyDown(KEY_D)) players[0].position.x += 5.0f;

        // Если прошло достаточно времени (интервал обновления), обработаем данные от клиентов
        if (deltaTime >= UPDATE_INTERVAL) {
            // Обработка позиции всех существующих игроков
            for (int i = 1; i < playerCount; i++) {
                int bytesReceived = recvfrom(server_fd, &players[i].position, sizeof(Position), 0,
                                              (struct sockaddr*)&client_addr, &client_addr_len);

                if (bytesReceived > 0) {
                    // Каждый игрок получает уникальный цвет из массива
                    if (i < 10 && players[i].color.r == 0 && players[i].color.g == 0 && players[i].color.b == 0) {
                        players[i].color = colorPalette[i]; // Присваиваем уникальный цвет
                    }
                    players[i].active = 1; // Отметим игрока как активного
                    players[i].addr = client_addr; // Сохраняем адрес клиента для отправки данных
                } else {
                    players[i].active = 0; // Если позиция не обновилась, помечаем игрока как неактивного
                }
            }

            // Обработка подключения новых клиентов
            if (playerCount < MAX_PLAYERS) {
                int bytesReceived = recvfrom(server_fd, &players[playerCount].position, sizeof(Position), 0,
                                              (struct sockaddr*)&client_addr, &client_addr_len);
                if (bytesReceived > 0) {
                    // Присваиваем новый уникальный цвет игроку
                    players[playerCount].color = colorPalette[playerCount % 10]; // Используем модуль для гарантии
                    players[playerCount].active = 1; // Новый игрок активен
                    players[playerCount].addr = client_addr; // Сохраняем адрес нового клиента
                    playerCount++;
                }
            }

            // Отправляем обновленные данные всем клиентам, кроме того, кто отправил запрос
            for (int i = 0; i < playerCount; i++) {
                if (players[i].active) {
                    for (int j = 0; j < playerCount; j++) {
                        // Не отправляем клиенту его собственные данные
                        if (i != j && players[j].active) {
                            sendto(server_fd, &players[j], sizeof(players[j]), 0, 
                                   (struct sockaddr*)&players[i].addr, client_addr_len);
                        }
                    }
                }
            }

            // Обновляем время последнего обновления
            lastUpdateTime = currentTime;
        }

        // Рисование
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Рисуем квадраты всех активных игроков
        for (int i = 0; i < playerCount; i++) {
            if (players[i].active) {
                DrawRectangleV((Vector2){players[i].position.x, players[i].position.y}, (Vector2){50, 50}, players[i].color);
            }
        }

        EndDrawing();
    }

    // Закрытие соединения
    close(server_fd);
    CloseWindow();

    return 0;
}
