#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define SPEED 35
#define screenWidth 1920
#define screenHeight 1080
#define DIST 300
#define squareSize 70.0f
#define SERVER_IP "172.28.123.89"  

Rectangle clientRect = {(screenWidth - squareSize) / 2 + DIST, (screenHeight - squareSize) / 2, squareSize, squareSize};
Rectangle serverRect = {0, 0, squareSize, squareSize};

void move_client_position() {
    if (clientRect.x < 0) clientRect.x = 0;
    if (clientRect.x > screenWidth - squareSize) clientRect.x = screenWidth - squareSize;
    if (clientRect.y < 0) clientRect.y = 0;
    if (clientRect.y > screenHeight - squareSize) clientRect.y = screenHeight - squareSize;
}

void handleCollision() {
    if (CheckCollisionRecs(clientRect, serverRect)) {
        // Перемещаем клиентский квадрат так, чтобы он не пересекался с сервером
        if (clientRect.x + squareSize > serverRect.x && clientRect.x < serverRect.x) {
            clientRect.x = serverRect.x - squareSize;
        }
        else if (clientRect.x < serverRect.x + squareSize && clientRect.x + squareSize > serverRect.x + squareSize) {
            clientRect.x = serverRect.x + squareSize;
        }
        else if (clientRect.y + squareSize > serverRect.y && clientRect.y < serverRect.y) {
            clientRect.y = serverRect.y - squareSize;
        }
        else if (clientRect.y < serverRect.y + squareSize && clientRect.y + squareSize > serverRect.y + squareSize) {
            clientRect.y = serverRect.y + squareSize;
        }
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "Клиент - Простая мультиплеерная игра");
    ToggleFullscreen(); 

    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd < 0) {
        printf("Ошибка создания сокета\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);    

    SetTargetFPS(60);  
    
    while (!WindowShouldClose()) {
        float prevx = clientRect.x;
        float prevy = clientRect.y;
        
        if (IsKeyDown(KEY_W)) clientRect.y -= SPEED;
        if (IsKeyDown(KEY_S)) clientRect.y += SPEED;
        if (IsKeyDown(KEY_A)) clientRect.x -= SPEED;
        if (IsKeyDown(KEY_D)) clientRect.x += SPEED;

        handleCollision();  // Проверка на коллизию с сервером
        
        move_client_position();   

        sendto(sockfd, &clientRect, sizeof(Rectangle), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        int bytesReceived = recvfrom(sockfd, &serverRect, sizeof(Rectangle), 0,
                                      (struct sockaddr*)&server_addr, &server_addr_len);

        if (bytesReceived > 0) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangle((screenWidth - 10) / 2, 0, 10, screenHeight, WHITE);
            DrawRectangleRec(clientRect, RED);
            DrawRectangleRec(serverRect, BLUE);
            EndDrawing();
        }
    }

    close(sockfd);
    CloseWindow();

    return 0;
}

