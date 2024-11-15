#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

char board[3][3];
int current_turn = 0; // 0 - сервер (X), 1 - клиент (O)

void initialize_board() {
    int count = 1;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            board[i][j] = '1' + count - 1;
            count++;
        }
    }
}

void print_board() {
    printf("\n");
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            printf(" %c ", board[i][j]);
            if (j < 2) printf("|");
        }
        printf("\n");
        if (i < 2) printf("---|---|---\n");
    }
    printf("\n");
}

int check_winner() {
    // Проверка строк, столбцов и диагоналей
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) return board[i][0];
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) return board[0][i];
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) return board[0][0];
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) return board[0][2];
    return 0;
}

void *handle_client(void *socket_desc) {
    int client_socket = *(int *)socket_desc;
    char buffer[1024] = {0};

    while (1) {
        print_board();

        // Проверка победителя
        int winner = check_winner();
        if (winner) {
            char winner_message[50];
            sprintf(winner_message, "Player %c wins!", winner);
            send(client_socket, winner_message, strlen(winner_message), 0);
            printf("%s\n", winner_message);
            break;
        }

        if (current_turn == 1) {
            memset(buffer, 0, sizeof(buffer));
            read(client_socket, buffer, sizeof(buffer));
            int move = atoi(buffer) - 1;
            int row = move / 3, col = move % 3;

            if (board[row][col] != 'X' && board[row][col] != 'O') {
                board[row][col] = 'O';
                current_turn = 0;
            } else {
                send(client_socket, "Invalid move", strlen("Invalid move"), 0);
            }
        } else {
            printf("Your move (1-9): ");
            fgets(buffer, sizeof(buffer), stdin);
            int move = atoi(buffer) - 1;
            int row = move / 3, col = move % 3;

            if (board[row][col] != 'X' && board[row][col] != 'O') {
                board[row][col] = 'X';
                current_turn = 1;
                send(client_socket, buffer, strlen(buffer), 0);
            } else {
                printf("Invalid move. Try again.\n");
            }
        }
    }

    close(client_socket);
    free(socket_desc);
    pthread_exit(NULL);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    initialize_board();

    // Создаем сокет
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Привязываем сокет к порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Ожидаем подключения
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            perror("Accept");
            continue;
        }
        printf("New connection established\n");

        pthread_t client_thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_socket;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_sock) != 0) {
            perror("Thread creation failed");
            free(new_sock);
        }
    }

    close(server_fd);
    return 0;
}
