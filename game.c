#include "raylib.h"

// Параметры окна
const int screenWidth = 500;
const int screenHeight = 300;
const int squareSize = 25;
Vector2 squarePosition;

const float speed = 400.0f; // Скорость движения квадрата
const int gridSize = 25;    // Размер ячеек сетки

// Функция для движения квадрата
void MoveSquare(float deltaTime) {
    if (IsKeyDown(KEY_RIGHT)) squarePosition.x += speed * deltaTime;
    if (IsKeyDown(KEY_LEFT)) squarePosition.x -= speed * deltaTime;
    if (IsKeyDown(KEY_UP)) squarePosition.y -= speed * deltaTime;
    if (IsKeyDown(KEY_DOWN)) squarePosition.y += speed * deltaTime;

    // Ограничение перемещения квадрата внутри окна
    if (squarePosition.x < 0) squarePosition.x = 0;
    if (squarePosition.y < 0) squarePosition.y = 0;
    if (squarePosition.x > screenWidth - squareSize) squarePosition.x = screenWidth - squareSize;
    if (squarePosition.y > screenHeight - squareSize) squarePosition.y = screenHeight - squareSize;
}

// Функция для отрисовки сетки
void DrawBackgroundGrid(int gridSize) {
    for (int x = 0; x < screenWidth; x += gridSize) {
        DrawLine(x, 0, x, screenHeight, LIGHTGRAY);
    }
    for (int y = 0; y < screenHeight; y += gridSize) {
        DrawLine(0, y, screenWidth, y, LIGHTGRAY);
    }
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Don't Die Together");
    SetTargetFPS(60);

    // Начальная позиция квадрата
    squarePosition = (Vector2){ screenWidth / 2.0f - squareSize / 2.0f, screenHeight / 2.0f - squareSize / 2.0f };

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        MoveSquare(deltaTime);

        // Отрисовка
        BeginDrawing();
        ClearBackground(RAYWHITE); // Очищаем экран

        // Рисуем сетку
        DrawBackgroundGrid(gridSize);

        // Рисуем квадрат
        DrawRectangleV(squarePosition, (Vector2){ squareSize, squareSize }, RED);
        DrawText("Example", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
