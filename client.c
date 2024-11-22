#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 1080

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};  // буфер для передачи данных
    int guess;

    // Проверка аргументов командной строки
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <Server IP>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Создание сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    // Заполнение структуры с адресом сервера
    serv_addr.sin_family = AF_INET;          // используем IPv4
    serv_addr.sin_port = htons(PORT);        // преобразуем порт в порядок байтов

    // Преобразование IP-адреса из текстового в бинарное
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Неверный адрес/ Адрес не поддерживается\n");
        close(sock);
        return EXIT_FAILURE;
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Не удалось установить соединение");
        close(sock);
        return EXIT_FAILURE;
    }

    // Основной цикл игры
    while (1) {
        printf("Введите своё число (1-100): ");
        if (scanf("%d", &guess) != 1 || guess < 1 || guess > 100) {
            fprintf(stderr, "Введите корректное число в диапазоне от 1 до 100.\n");
            while(getchar() != '\n'); // очистка буфера ввода
            continue;
        }

        // Отправка предположения на сервер
        sprintf(buffer, "%d", guess);
        send(sock, buffer, strlen(buffer), 0);

        // Получение ответа от сервера
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, sizeof(buffer), 0);
        
        if (valread < 0) {
            perror("Ошибка чтения от сервера");
            break;
        }

        printf("Сервер: %s\n", buffer);

        // Проверка, угадано ли число
        if (strcmp(buffer, "Правильно!") == 0) {
            printf("Вы угадали число!\n");
            break;
        }
    }

    close(sock);  // Закрытие сокета клиента
    return EXIT_SUCCESS;
}
