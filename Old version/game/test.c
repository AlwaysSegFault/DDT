#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define SCREEN_WIDTH GetMonitorWidth(0)
#define SCREEN_HEIGHT GetMonitorHeight(0)
#define TILE_SIZE 96
#define MAP_WIDTH 501
#define MAP_HEIGHT 501
#define NUM_ROCKS 10000
#define NUM_GRASS 30000
#define NUM_TREES 10000
#define NUM_GROUND 5000

typedef struct {
    float x;
    float y;
    float speed;
} Character;

typedef struct {
    int x;
    int y;
    int type;
} Rock;

typedef struct {
    int x;
    int y;
    int type;
} Grass;

typedef struct {
    int x;
    int y;
    int type;
} Ground;

typedef struct {
    int x;
    int y;
} Tree;

typedef struct {
    float x;
    float y;
    int type;
    int layerType; 
} RenderableObject;


bool isGameRunning = false;
int occupied[MAP_WIDTH][MAP_HEIGHT] = {0};
Grass grass[NUM_GRASS];
Rock rocks[NUM_ROCKS];
Tree trees[NUM_TREES];
Ground ground[NUM_GROUND];
Texture2D characterTexture;


bool CheckCollisionWithObjects(Character character, Rock rocks[], int numRocks, Tree trees[], int numTrees);
bool CheckCollisionRectangles(Rectangle rect1, Rectangle rect2);
void DrawCharacter(Character character);
void DrawCollisionDebug(Tree trees[], int numTrees, Rock rocks[], int numRocks, Character character);

void DrawCharacter(Character character) {
    float characterWidth = TILE_SIZE * 0.7;
    float characterHeight = TILE_SIZE * 0.7;

    Rectangle sourceRec = {0.0f, 0.0f, (float)characterTexture.width, (float)characterTexture.height}; 
    Rectangle destRec = {character.x - characterWidth / 2, character.y - characterHeight / 2, characterWidth, characterHeight};  
    Vector2 origin = {characterTexture.width / 2.0f, characterTexture.height / 2.0f};  

    DrawTexturePro(characterTexture, sourceRec, destRec, origin, 0.0f, WHITE);  
}

Character CreateCharacter() {
    Character character = {MAP_WIDTH / 2.0f * TILE_SIZE, MAP_HEIGHT / 2.0f * TILE_SIZE, 200.0f};
    return character;
}

Camera2D SetupCamera(Character character) {
    Camera2D camera = {0};
    camera.target = (Vector2){character.x, character.y};
    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    camera.zoom = 1.0f;
    return camera;
}

void UpdateCharacter(Character *character, float deltaTime, Rock rocks[], int numRocks, Tree trees[], int numTrees) {
    float newX = character->x;
    float newY = character->y;

    float moveX = 0.0f;
    float moveY = 0.0f;

    if (IsKeyDown(KEY_W)) moveY -= character->speed * deltaTime;
    if (IsKeyDown(KEY_S)) moveY += character->speed * deltaTime;
    if (IsKeyDown(KEY_A)) moveX -= character->speed * deltaTime;
    if (IsKeyDown(KEY_D)) moveX += character->speed * deltaTime;

    float length = sqrtf(moveX * moveX + moveY * moveY); 

    if (length != 0.0f) { 
        moveX /= length; 
        moveY /= length; 
    }

    newX += moveX * character->speed * deltaTime;
    newY += moveY * character->speed * deltaTime;

    Character tempCharacter = { newX, character->y, character->speed };
    if (!CheckCollisionWithObjects(tempCharacter, rocks, numRocks, trees, numTrees)) {
        character->x = newX;
    }

    tempCharacter = (Character){ character->x, newY, character->speed };
    if (!CheckCollisionWithObjects(tempCharacter, rocks, numRocks, trees, numTrees)) {
        character->y = newY;
    }
}

void CustomeUpdateCamera(Camera2D *camera, Character character) {
    camera->target = (Vector2){character.x, character.y};
}

void DrawMap(Ground ground[], int numGround, Grass grass[], int numGrass, Rock rocks[], int numRocks, Tree trees[], int numTrees, Texture2D groundTexture1, Texture2D groundTexture2, Texture2D grassTexture1, Texture2D grassTexture2, Texture2D grassTexture3, Texture2D grassTexture4, Texture2D rockTexture1, Texture2D rockTexture2, Texture2D treeTexture, Character character) {
    for (int i = 0; i < numGround; i++) {
        int groundX = ground[i].x * TILE_SIZE;
        int groundY = ground[i].y * TILE_SIZE;
        DrawTextureEx(ground[i].type == 0 ? groundTexture1 : groundTexture2, (Vector2){groundX, groundY}, 0.0f, (float)TILE_SIZE / groundTexture1.width * 2, WHITE);
    }

    for (int i = 0; i < numGrass; i++) {
        int grassX = grass[i].x * TILE_SIZE;
        int grassY = grass[i].y * TILE_SIZE;
        DrawTextureEx((grass[i].type == 0 ? grassTexture1 : (grass[i].type == 1 ? grassTexture2 : (grass[i].type == 2 ? grassTexture3 : grassTexture4))), (Vector2){grassX, grassY}, 0.0f, (float)TILE_SIZE / grassTexture1.width, WHITE);
    }
    
    for (int i = 0; i < numRocks; i++) {
        int rockX = rocks[i].x * TILE_SIZE;
        int rockY = rocks[i].y * TILE_SIZE;

        if ((rockY + TILE_SIZE * 0.25f) < character.y) { 
            DrawTextureEx(rocks[i].type == 0 ? rockTexture1 : rockTexture2, (Vector2){rockX, rockY}, 0.0f, (float)TILE_SIZE / rockTexture1.width, WHITE);
        }
    }

    for (int i = 0; i < numTrees; i++) {
        int treeX = trees[i].x * TILE_SIZE;
        int treeY = trees[i].y * TILE_SIZE;

        if ((treeY + TILE_SIZE * 2.35f) < character.y) { 
            DrawTextureEx(treeTexture, (Vector2){treeX, treeY}, 0.0f, (float)TILE_SIZE / treeTexture.width * 2, WHITE);
        }
    }

    DrawCharacter(character);

    for (int i = 0; i < numRocks; i++) {
        int rockX = rocks[i].x * TILE_SIZE;
        int rockY = rocks[i].y * TILE_SIZE;
        
        if ((rockY + TILE_SIZE * 0.25f) >= character.y) {
            DrawTextureEx(rocks[i].type == 0 ? rockTexture1 : rockTexture2, (Vector2){rockX, rockY}, 0.0f, (float)TILE_SIZE / rockTexture1.width, WHITE);
        }
    }

    for (int i = 0; i < numTrees; i++) {
        int treeX = trees[i].x * TILE_SIZE;
        int treeY = trees[i].y * TILE_SIZE;

        if ((treeY + TILE_SIZE * 2.35f) >= character.y) { 
            DrawTextureEx(treeTexture, (Vector2){treeX, treeY}, 0.0f, (float)TILE_SIZE / treeTexture.width * 2, WHITE);
        }
    }
    
}

void GenerateRocks(Rock rocks[], int numRocks, int occupied[MAP_WIDTH][MAP_HEIGHT]) {
    for (int i = 0; i < numRocks; i++) {
        int x, y;
        do {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;
        } while (occupied[x][y]); 
        rocks[i].x = x;
        rocks[i].y = y;
        rocks[i].type = rand() % 2;
        occupied[x][y] = 1; 
    }
}

void GenerateGrass(Grass grass[], int numGrass, int occupied[MAP_WIDTH][MAP_HEIGHT]) {
    for (int i = 0; i < numGrass; i++) {
        int x, y;
        do {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;
        } while (occupied[x][y]); 
        grass[i].x = x;
        grass[i].y = y;
        grass[i].type = rand() % 4;
        occupied[x][y] = 1; 
    }
}

void GenerateTrees(Tree trees[], int numTrees, int occupied[MAP_WIDTH][MAP_HEIGHT]) {
    for (int i = 0; i < numTrees; i++) {
        int x, y;
        int isValid;
        do {
            isValid = 1;
            x = rand() % (MAP_WIDTH - 1); 
            y = rand() % (MAP_HEIGHT - 2); 
            for (int offsetY = 0; offsetY < 3; offsetY++) {
                for (int offsetX = 0; offsetX < 2; offsetX++) {
                    if (occupied[x + offsetX][y + offsetY]) {
                        isValid = 0;
                        break;
                    }
                }
                if (!isValid) break;
            }
        } while (!isValid);
        trees[i].x = x;
        trees[i].y = y;
        for (int offsetY = 0; offsetY < 3; offsetY++) {
            for (int offsetX = 0; offsetX < 2; offsetX++) {
                occupied[x + offsetX][y + offsetY] = 1;
            }
        }
    }
}

void GenerateGround(Ground ground[], int numGround, int occupied[MAP_WIDTH][MAP_HEIGHT]) {
    for (int i = 0; i < numGround; i++) {
        int x, y;
        int isValid;
        do {
            isValid = 1;
            x = rand() % (MAP_WIDTH - 2); 
            y = rand() % (MAP_HEIGHT - 1); 
            
            for (int offsetY = 0; offsetY <2; offsetY++) {
                for (int offsetX = 0; offsetX < 3; offsetX++) {
                    if (x + offsetX >= MAP_WIDTH || y + offsetY >= MAP_HEIGHT || occupied[x + offsetX][y + offsetY]) {
                        isValid = 0;
                        break;
                    }
                }
                if (!isValid) break;
            }
        } while (!isValid);
        ground[i].x = x;
        ground[i].y = y;
        ground[i].type = rand() % 2;
        for (int offsetY = 0; offsetY < 2; offsetY++) {
            for (int offsetX = 0; offsetX < 3; offsetX++) {
                if (x + offsetX < MAP_WIDTH && y + offsetY < MAP_HEIGHT) {
                    occupied[x + offsetX][y + offsetY] = 1;
                }
            }
        }
    }
}

bool CheckCollisionWithObjects(Character character, Rock rocks[], int numRocks, Tree trees[], int numTrees) {
    Rectangle characterRect = {
        character.x - TILE_SIZE * 0.3f,
        character.y - TILE_SIZE * 0.3f,
        TILE_SIZE * 0.3f,
        TILE_SIZE * 0.6f
    };

    for (int i = 0; i < numRocks; i++) {
        Rectangle rockRect = {
            rocks[i].x * TILE_SIZE + TILE_SIZE * 0.15f,
            rocks[i].y * TILE_SIZE + TILE_SIZE * 0.25f,
            TILE_SIZE * 0.65f,
            TILE_SIZE * 0.45f
        };
        
        if (CheckCollisionRectangles(characterRect, rockRect)) {
            return true; 
        }
    }

    for (int i = 0; i < numTrees; i++) {
        Rectangle treeCollisionRect = {
            trees[i].x * TILE_SIZE + TILE_SIZE * 0.65f,
            trees[i].y * TILE_SIZE + TILE_SIZE * 2.35f,
            TILE_SIZE * 0.65f,
            TILE_SIZE * 0.45f
        };

        if (CheckCollisionRectangles(characterRect, treeCollisionRect)) {
            return true; 
        }
    }

    return false; 
}

bool CheckCollisionRectangles(Rectangle rect1, Rectangle rect2) {
    return (rect1.x < rect2.x + rect2.width &&
            rect1.x + rect1.width > rect2.x &&
            rect1.y < rect2.y + rect2.height &&
            rect1.y + rect1.height > rect2.y);
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "DDT");
    SetTargetFPS(60);
    srand(time(NULL));

    Color backgroundColor = (Color){214, 142, 8, 255};
    
    occupied[MAP_WIDTH / 2][MAP_HEIGHT / 2] = 1;

    const char *loadingText = "Don't Die Together";
    int loadingTextLength = strlen(loadingText);
    int currentLetter = 0;
    float loadingTime = 0.0f;
    const float letterDisplayInterval = 0.1f;
    bool isLoading = true;
    
    characterTexture = LoadTexture("texture/character.png");

    Texture2D groundTexture1 = LoadTexture("texture/ground1.png");
    Texture2D groundTexture2 = LoadTexture("texture/ground2.png");
    Texture2D grassTexture1 = LoadTexture("texture/grass1.png");
    Texture2D grassTexture2 = LoadTexture("texture/grass2.png");
    Texture2D grassTexture3 = LoadTexture("texture/grass3.png");
    Texture2D grassTexture4 = LoadTexture("texture/grass4.png");
    Texture2D rockTexture1 = LoadTexture("texture/rock1.png");
    Texture2D rockTexture2 = LoadTexture("texture/rock2.png");
    Texture2D treeTexture = LoadTexture("texture/tree.png");

    Texture2D menuTexture = LoadTexture("texture/menu.png");
    Texture2D exitTexture = LoadTexture("texture/exit.png");
    Texture2D infoTexture = LoadTexture("texture/info.png");
    Texture2D animationPlayTexture = LoadTexture("texture/animation_play-button.png");
    int playFrameWidth = animationPlayTexture.width / 3;
    int currentPlayFrame = 0;

    Texture2D animationExitTexture = LoadTexture("texture/animation_exit-button.png");
    int exitFrameWidth = animationExitTexture.width / 3;
    int currentExitFrame = 0;

    float playButtonAnimationTime = 0.0f;
    float exitButtonAnimationTime = 0.0f;
    const float animationDisplayDuration = 0.5f;
    const float frameDuration = animationDisplayDuration / 3.0f;
    float playFrameTimeAccumulator = 0.0f;
    float exitFrameTimeAccumulator = 0.0f;

    bool playButtonPressed = false;
    bool exitButtonPressed = false;

    Character character;
    Camera2D camera;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        BeginDrawing();
        ClearBackground(backgroundColor);

    if (isLoading) {
        loadingTime += deltaTime;
        if (loadingTime >= letterDisplayInterval) {
            loadingTime = 0.0f;
            if (currentLetter < loadingTextLength) {
                currentLetter++;
            } else {
                isLoading = false;
            }
        }

        int textX = SCREEN_WIDTH / 2 - MeasureText(loadingText, 40) / 2;
        int textY = SCREEN_HEIGHT / 2 - 20;

        int offsetX = 0;
        const int spacing = 5;

        for (int i = 0; i < currentLetter; i++) {
            char currentChar[2] = {loadingText[i], '\0'}; 
            DrawText(currentChar, textX + offsetX, textY, 40, BLACK);
            offsetX += MeasureText(currentChar, 40) + spacing; 
        }
    } else if (!isGameRunning) {
            float scaleMenu = 8.0f; 
            int menuScaledWidth = menuTexture.width * scaleMenu;
            int menuScaledHeight = menuTexture.height * scaleMenu;
            int menuX = (SCREEN_HEIGHT - menuScaledHeight) / 2;  
            int menuY = (SCREEN_HEIGHT - menuScaledHeight) / 2; 

            float scaleInfoAndExit = 4.0f;
            int exitScaledWidth = exitTexture.width * scaleInfoAndExit;
            int exitScaledHeight = exitTexture.height * scaleInfoAndExit;
            int exitX = SCREEN_WIDTH - ((SCREEN_HEIGHT - menuScaledHeight) / 2) - exitScaledWidth;  
            int exitY = SCREEN_HEIGHT - ((SCREEN_HEIGHT - menuScaledHeight) / 2) - exitScaledHeight; 

            int infoScaledWidth = exitTexture.width * scaleInfoAndExit;
            int infoScaledHeight = exitTexture.height * scaleInfoAndExit;
            int infoX = SCREEN_WIDTH - ((SCREEN_HEIGHT - menuScaledHeight) / 2) - infoScaledWidth;  
            int infoY = (SCREEN_HEIGHT - menuScaledHeight) / 2; 

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePoint = GetMousePosition();
                int buttonPlayWidth = 36 * scaleMenu;
                int buttonPlayHeight = 20 * scaleMenu;
                int buttonPlayX = menuX + (14 * scaleMenu);
                int buttonPlayY = menuY + (63 * scaleMenu);

                int buttonExitWidth = 36 * scaleInfoAndExit;
                int buttonExitHeight = 20 * scaleInfoAndExit;
                int buttonExitX = exitX + (6 * scaleInfoAndExit);
                int buttonExitY = exitY + (6 * scaleInfoAndExit);

                if (CheckCollisionPointRec(mousePoint, (Rectangle){buttonPlayX, buttonPlayY, buttonPlayWidth, buttonPlayHeight})) {
                    playButtonPressed = true;
                    playButtonAnimationTime = animationDisplayDuration;
                    playFrameTimeAccumulator = 0.0f;
                    currentPlayFrame = 0;
                }
                if (CheckCollisionPointRec(mousePoint, (Rectangle){buttonExitX, buttonExitY, buttonExitWidth, buttonExitHeight})) {
                    exitButtonPressed = true;
                    exitButtonAnimationTime = animationDisplayDuration;
                    exitFrameTimeAccumulator = 0.0f;
                    currentExitFrame = 0;
                }
            }

            if (playButtonPressed) {
                playButtonAnimationTime -= deltaTime;
                playFrameTimeAccumulator += deltaTime;

                if (playFrameTimeAccumulator >= frameDuration) {
                    currentPlayFrame++;
                    if (currentPlayFrame > 2) {
                        currentPlayFrame = 0;
                    }
                    playFrameTimeAccumulator = 0.0f;
                }

                if (playButtonAnimationTime <= 0.0f) {
                    playButtonPressed = false;
                    isGameRunning = true;

                    character = CreateCharacter();
                    camera = SetupCamera(character);

                    GenerateGrass(grass, NUM_GRASS, occupied);
                    GenerateRocks(rocks, NUM_ROCKS, occupied);
                    GenerateTrees(trees, NUM_TREES, occupied);
                    GenerateGround(ground, NUM_GROUND, occupied);
                }
            }

            if (exitButtonPressed) {
                exitButtonAnimationTime -= deltaTime;
                exitFrameTimeAccumulator += deltaTime;

                if (exitFrameTimeAccumulator >= frameDuration) {
                    currentExitFrame++;
                    if (currentExitFrame > 2) {
                        currentExitFrame = 0;
                    }
                    exitFrameTimeAccumulator = 0.0f;
                }

                if (exitButtonAnimationTime <= 0.0f) {
                    exitButtonPressed = false;
                    CloseWindow();
                    return 0;
                }
            }

            if (playButtonPressed) {
                Rectangle sourceRec = { playFrameWidth * currentPlayFrame, 0, playFrameWidth, animationPlayTexture.height };
                Rectangle destRec = { menuX, menuY, playFrameWidth * scaleMenu, animationPlayTexture.height * scaleMenu };
                DrawTexturePro(animationPlayTexture, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawTextureEx(menuTexture, (Vector2){menuX, menuY}, 0.0f, scaleMenu, WHITE);
            }

            if (exitButtonPressed) {
                Rectangle sourceRec = { exitFrameWidth * currentExitFrame, 0, exitFrameWidth, animationExitTexture.height };
                Rectangle destRec = { exitX, exitY, exitFrameWidth * scaleInfoAndExit, animationExitTexture.height * scaleInfoAndExit };
                DrawTexturePro(animationExitTexture, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawTextureEx(exitTexture, (Vector2){exitX, exitY}, 0.0f, scaleInfoAndExit, WHITE);
            }
            DrawTextureEx(infoTexture, (Vector2){infoX, infoY}, 0.0f, scaleInfoAndExit, WHITE);
        } else {
            UpdateCharacter(&character, deltaTime, rocks, NUM_ROCKS, trees, NUM_TREES);
            CustomeUpdateCamera(&camera, character);

            BeginMode2D(camera);
            DrawMap(ground, NUM_GROUND, grass, NUM_GRASS, rocks, NUM_ROCKS, trees, NUM_TREES, groundTexture1, groundTexture2, grassTexture1, grassTexture2, grassTexture3, grassTexture4, rockTexture1, rockTexture2, treeTexture, character);

            EndMode2D();
        }

        EndDrawing();
    }

    UnloadTexture(groundTexture1);
    UnloadTexture(groundTexture2);
    UnloadTexture(grassTexture1);
    UnloadTexture(grassTexture2);
    UnloadTexture(grassTexture3);
    UnloadTexture(grassTexture4);
    UnloadTexture(rockTexture1);
    UnloadTexture(rockTexture2);
    UnloadTexture(treeTexture);
    UnloadTexture(menuTexture);
    UnloadTexture(exitTexture);
    UnloadTexture(infoTexture);
    UnloadTexture(animationPlayTexture);
    UnloadTexture(animationExitTexture);

    CloseWindow();
    return 0;
}


