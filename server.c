#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>  // Для работы с потоками
#include <sys/select.h>  // Для работы с таймаутами

#define PORT 12345
#define BUFFER_SIZE 256
#define SPEED 20
#define CLIENT_TIMEOUT 5  // Таймаут для клиента в секундах
#define screenWidth 1920
#define screenHeight  1080

typedef struct {
    float x;
    float y;
} Position;




int server_fd;
struct sockaddr_in server_addr, client_addr;
socklen_t client_addr_len = sizeof(client_addr);
Position serverPosition = {400, 300};
Position clientPosition = {600, 300};  // Начальная позиция клиента
const float squareSize = 50.0f;
int clientConnected = 0;  // Флаг, показывающий подключение клиента

void* handle_client(void* arg) {
    int bytesReceived;
    fd_set read_fds;
    struct timeval timeout;
    timeout.tv_sec = CLIENT_TIMEOUT;
    timeout.tv_usec = 0;

    while (1) {
        // Подготовка для использования select для таймаута
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        int ready = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (ready > 0) {
            // Получаем данные от клиента
            bytesReceived = recvfrom(server_fd, &clientPosition, sizeof(Position), 0, 
                                     (struct sockaddr*)&client_addr, &client_addr_len);
            if (bytesReceived > 0) {
                clientConnected = 1;  // Клиент подключился
                // Отправляем серверные координаты обратно клиенту
                sendto(server_fd, &serverPosition, sizeof(Position), 0, 
                       (struct sockaddr*)&client_addr, client_addr_len);
            }
        } else {
            // Таймаут, клиент не прислал данные
            clientPosition.x = -1;  // Убираем квадрат клиента
            clientPosition.y = -1;  // Убираем квадрат клиента
            clientConnected = 0;  // Клиент отсоединился
        }

        // Перезагружаем таймаут для следующего цикла
        timeout.tv_sec = CLIENT_TIMEOUT;
        timeout.tv_usec = 0;
    }

    return NULL;
}

// Внесем изменения в обработку движения сервера и клиента, добавим проверку на коллизию

void move_server() {
    // Пределы поля
    if (serverPosition.x < 0) serverPosition.x = 0;
    if (serverPosition.x > screenWidth - squareSize) serverPosition.x = screenWidth - squareSize;
    if (serverPosition.y < 0) serverPosition.y = 0;
    if (serverPosition.y > screenHeight - squareSize) serverPosition.y = screenHeight - squareSize;
}

void move_client() {
    // Пределы поля
    if (clientPosition.x < 0) clientPosition.x = 0;
    if (clientPosition.x > screenWidth - squareSize) clientPosition.x = screenWidth - squareSize;
    if (clientPosition.y < 0) clientPosition.y = 0;
    if (clientPosition.y > screenHeight - squareSize) clientPosition.y = screenHeight - squareSize;
}

// Проверка на коллизию между сервером и клиентом
int check_collision() {
    return (serverPosition.x < clientPosition.x + squareSize &&
            serverPosition.x + squareSize > clientPosition.x &&
            serverPosition.y < clientPosition.y + squareSize &&
            serverPosition.y + squareSize > clientPosition.y);
}

void handle_collision() {
    if (check_collision()) {
        // Останавливаем движение сервера и клиента по направлениям
        // Отменяем движение, если есть пересечение
        if (serverPosition.x < clientPosition.x) serverPosition.x = clientPosition.x + squareSize;
        else if (serverPosition.x > clientPosition.x) serverPosition.x = clientPosition.x - squareSize;

        if (serverPosition.y < clientPosition.y) serverPosition.y = clientPosition.y + squareSize;
        else if (serverPosition.y > clientPosition.y) serverPosition.y = clientPosition.y - squareSize;

        if (clientPosition.x < serverPosition.x) clientPosition.x = serverPosition.x + squareSize;
        else if (clientPosition.x > serverPosition.x) clientPosition.x = serverPosition.x - squareSize;

        if (clientPosition.y < serverPosition.y) clientPosition.y = serverPosition.y + squareSize;
        else if (clientPosition.y > serverPosition.y) clientPosition.y = serverPosition.y - squareSize;
    }
}


int main() {
    InitWindow(screenWidth, screenHeight, "Сервер - Простая мультиплеерная игра");
    ToggleFullscreen();  // Включаем полноэкранный режим

    // Создание сокета
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        printf("Ошибка создания сокета\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Ошибка биндинга\n");
        return -1;
    }

    // Запуск обработчика клиента в отдельном потоке
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, NULL);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_W)) serverPosition.y -= SPEED;
    if (IsKeyDown(KEY_S)) serverPosition.y += SPEED;
    if (IsKeyDown(KEY_A)) serverPosition.x -= SPEED;
    if (IsKeyDown(KEY_D)) serverPosition.x += SPEED;

    move_server();  // Проверка на выход за пределы экрана
    handle_collision();  // Проверка коллизии

    BeginDrawing();
    ClearBackground(BLACK);
    int rectWidth = 10;
    
    
    DrawRectangle((screenWidth - rectWidth) / 2, 0, rectWidth, screenHeight, WHITE);
    DrawRectangle((int)serverPosition.x, (int)serverPosition.y, (int)squareSize, (int)squareSize, BLUE);
    if (clientConnected) {
        DrawRectangle((int)clientPosition.x, (int)clientPosition.y, (int)squareSize, (int)squareSize, RED);
    }
    EndDrawing();
}


    close(server_fd);
    CloseWindow();

    return 0;
}

