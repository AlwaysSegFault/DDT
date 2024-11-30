#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>  // Для работы с потоками
#include <sys/select.h>  // Для работы с таймаутами

#define PORT 12345
#define SPEED 35
#define CLIENT_TIMEOUT 2  // Таймаут для клиента в секундах
#define screenWidth 1920
#define screenHeight 1080
#define squareSize 70.0f
#define DIST 300
#define RAD 50
#define GATES 240

typedef struct {
    float x;
    float y;
} Position;

int server_fd;
struct sockaddr_in server_addr, client_addr;
socklen_t client_addr_len = sizeof(client_addr);
Rectangle serverRect = {(screenWidth - squareSize) / 2 - DIST, (screenHeight - squareSize) / 2, squareSize, squareSize};
Rectangle clientRect = {(screenWidth - squareSize) / 2 + DIST, (screenHeight - squareSize) / 2, squareSize, squareSize};
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
            bytesReceived = recvfrom(server_fd, &clientRect, sizeof(Rectangle), 0, 
                                     (struct sockaddr*)&client_addr, &client_addr_len);
            if (bytesReceived > 0) {
                clientConnected = 1;  // Клиент подключился
                // Отправляем серверные координаты обратно клиенту
                sendto(server_fd, &serverRect, sizeof(Rectangle), 0, 
                       (struct sockaddr*)&client_addr, client_addr_len);
            }
        } else {
            // Таймаут, клиент не прислал данные
            clientRect.x = -1;  // Убираем квадрат клиента
            clientRect.y = -1;  // Убираем квадрат клиента
            clientConnected = 0;  // Клиент отсоединился
        }

        // Перезагружаем таймаут для следующего цикла
        timeout.tv_sec = CLIENT_TIMEOUT;
        timeout.tv_usec = 0;
    }

    return NULL;
}

void move_server() {
    // Пределы поля
    if (serverRect.x < 0) serverRect.x = 0;
    if (serverRect.x > screenWidth - squareSize) serverRect.x = screenWidth - squareSize;
    if (serverRect.y < 0) serverRect.y = 0;
    if (serverRect.y > screenHeight - squareSize) serverRect.y = screenHeight - squareSize;
}

bool checkCollision(Rectangle rect1, Rectangle rect2, Rectangle rect3, Rectangle rect4, Rectangle rect5){
  return CheckCollisionRecs(rect1, rect2) || CheckCollisionRecs(rect1, rect3) || CheckCollisionRecs(rect1, rect4) || CheckCollisionRecs(rect1, rect5);
}

void handleCollision() {
    if (CheckCollisionRecs(serverRect, clientRect)) {
        // Перемещаем клиентский квадрат так, чтобы он не пересекался с сервером
        if (serverRect.x + squareSize > clientRect.x && serverRect.x < clientRect.x) {
            serverRect.x = clientRect.x - squareSize;
        }
        else if (serverRect.x < clientRect.x + squareSize && serverRect.x + squareSize > clientRect.x + squareSize) {
            serverRect.x = clientRect.x + squareSize;
        }
        else if (serverRect.y + squareSize > clientRect.y && serverRect.y < clientRect.y) {
            serverRect.y = clientRect.y - squareSize;
        }
        else if (serverRect.y < clientRect.y + squareSize && serverRect.y + squareSize > clientRect.y + squareSize) {
            serverRect.y = clientRect.y + squareSize;
        }
    }
}

Rectangle rect1 = {0, (screenHeight - GATES) / 2 - 5, 25, 10};
Rectangle rect2 = {0, (screenHeight + GATES) / 2 - 5 , 25, 10};
Rectangle rect3 = {screenWidth - 25, (screenHeight - GATES) / 2 - 5, 25, 10};
Rectangle rect4 = {screenWidth - 25, (screenHeight + GATES) / 2 - 5, 25, 10};


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
        float prevx = serverRect.x;
        float prevy = serverRect.y;
        
        if (IsKeyDown(KEY_W)) serverRect.y -= SPEED;
        if (IsKeyDown(KEY_S)) serverRect.y += SPEED;
        if (IsKeyDown(KEY_A)) serverRect.x -= SPEED;
        if (IsKeyDown(KEY_D)) serverRect.x += SPEED;
        
        if(checkCollision(serverRect, rect1, rect2, rect3, rect4)){
          serverRect.x = prevx;
          serverRect.y = prevy;
        }
        
        handleCollision();
        move_server();  // Проверка на выход за пределы экрана

        BeginDrawing();
        ClearBackground(BLACK);
        
        //Рисуем элементы поля
        DrawCircle(screenWidth/2, screenHeight/2, 375, WHITE);
        DrawCircle(screenWidth/2, screenHeight/2, 365, BLACK);
        DrawRectangle((screenWidth - 10) / 2, 0, 10, screenHeight / 2 - RAD - 10, WHITE);
        DrawRectangle((screenWidth - 10) / 2, screenHeight / 2 + RAD + 10, 10, screenHeight, WHITE);
        DrawRectangleRec(rect1, WHITE);
        DrawRectangleRec(rect2, WHITE);
        DrawRectangleRec(rect3, WHITE);
        DrawRectangleRec(rect4, WHITE);
        
        DrawRectangleRec(serverRect, BLUE);
        if (clientConnected) {
            DrawRectangleRec(clientRect, RED);
        }
        EndDrawing();
    }

    close(server_fd);
    CloseWindow();

    return 0;
}

