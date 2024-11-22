#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT 1080
#define MAX_CONN 5

// Функция для логов сообщений от клиента
void log_message(const char* client_ip, const char* message) {
    printf("%s: %s\n", client_ip, message);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int number_to_guess, guess;
    char buffer[1024] = {0};   // буфер для приема данных
    char client_ip[INET_ADDRSTRLEN];  // строка для хранения IP-адреса клиента

    // создание сокета
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");   // ошибка создания сокета
        exit(EXIT_FAILURE);
    }

    // привязка сокета к порту
    address.sin_family = AF_INET;                  // используем IPv4
    address.sin_addr.s_addr = INADDR_ANY;          // принимаем подключения с ЛЮБОГО IP
    address.sin_port = htons(PORT);                // привязка к заданному порту

    // привязка сокета к указанному адресу и порту
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");   // ошибка привязки
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // прослушивание подключений
    if (listen(server_fd, MAX_CONN) < 0) {
        perror("listen");   // ошибка при режиме прослушивания
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Сервер запущен по порту %d\n", PORT);

    // генерация случайного числа вне цикла
    srand(time(NULL));

    // основной цикл сервера
    while (1) {
        printf("Ожидание соединения...\n");
        // ожидание подключения клиента
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept");   // ошибка при принятии подключения
            continue;  // продолжаем ожидание следующего подключения
        }

        // получение IP-адреса клиента в строке
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        log_message(client_ip, "Подключенный клиент");
        number_to_guess = rand() % 100 + 1;  // генерация нового числа для каждого клиента

        while (1) {
            memset(buffer, 0, sizeof(buffer));  // очистка буфера приемом
            int valread = recv(new_socket, buffer, sizeof(buffer), 0);
            if (valread <= 0) {
                log_message(client_ip, "Клиент отключен"); // логи отключения клиента
                break;  // выходим из внутреннего цикла
            }

            guess = atoi(buffer);  // преобразование строки в число
            printf("Полученное предположение: %d from %s\n", guess, client_ip);

            // проверка введенного числа
            if (guess == number_to_guess) {
                send(new_socket, "Правильно!", strlen("Правильно!"), 0);  // клиент угадал число
                log_message(client_ip, "Клиент угадал правильно");
                break;
            } else if (guess < number_to_guess) {
                send(new_socket, "Число больше", strlen("Число больше"), 0);  // загаданное число больше
            } else {
                send(new_socket, "Число меньше", strlen("Число меньше"), 0);  // загаданное число меньше
            }
        }

        close(new_socket);  // закрытие соединения с клиентом
    }

    close(server_fd);  // закрытие сокета сервера
    return 0;
}
