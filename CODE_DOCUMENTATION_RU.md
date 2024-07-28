# Документация кода

## Оглавление
- [Общее описание](#общее-описание)
- [Описание файла config.h](#описание-файла-configh)
- [Описание файла network.h](#описание-файла-networkh)
- [Описание файла main.c](#описание-файла-mainc)
- [Описание файла network.c](#описание-файла-networkc)

## Общее описание

Этот проект представляет собой VPN-клиент, работающий по протоколу Shadowsocks, который позволяет пользователю обходить блокировки интернет-ресурсов в стране. Основные компоненты проекта включают функции для работы с сетевыми соединениями, шифрованием данных и настройкой системного прокси. Проект использует библиотеку libsodium для обеспечения криптографических операций.

## Описание файла `config.h`

Файл `config.h` содержит конфигурационные параметры для подключения к серверу:

```c
#ifndef CONFIG_H
#define CONFIG_H

#define SERVER_IP "your-ip"    // IP-адрес сервера
#define SERVER_PORT 8388              // Порт сервера
#define PASSWORD "your-password"         // Пароль для шифрования
#define METHOD "aes-256-gcm"          // Метод шифрования

#endif
```

## Описание файла `network.h`

Файл `network.h` объявляет функции и включает необходимые библиотеки для работы с сетью и шифрованием:

```c
#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include "config.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

void print_hex(const char *label, const unsigned char *data, size_t length); // Печать данных в шестнадцатеричном формате
void set_system_proxy(const char *proxy_address); // Установка системного прокси
void unset_system_proxy(); // Отключение системного прокси
int initialize_sodium(); // Инициализация библиотеки libsodium
int setup_encryption_key(unsigned char *key); // Настройка ключа шифрования
int create_socket(); // Создание сокета для сетевого взаимодействия
int connect_to_server(int sockfd, const char *server_ip, int server_port); // Подключение к серверу

#endif
```

## Описание файла `main.c`

Файл `main.c` содержит основную функцию программы, которая выполняет следующие задачи:

1. **Инициализация библиотеки Sodium**:
    ```c
    if (initialize_sodium() < 0) {
        return 1;
    }
    ```

2. **Генерация ключа шифрования**:
    ```c
    unsigned char key[crypto_aead_aes256gcm_KEYBYTES];
    if (setup_encryption_key(key) < 0) {
        return 1;
    }
    ```

3. **Инициализация Windows Sockets (только для Windows)**:
    ```c
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
    printf("WSAStartup initialized successfully\n");
    set_system_proxy("127.0.0.1:1080");
    printf("System proxy set to 127.0.0.1:1080\n");
    #endif
    ```

4. **Создание сетевого сокета**:
    ```c
    int sockfd = create_socket();
    if (sockfd < 0) {
        return 1;
    }
    ```

5. **Подключение к серверу**:
    ```c
    if (connect_to_server(sockfd, SERVER_IP, SERVER_PORT) < 0) {
        return 1;
    }
    ```

6. **Шифрование и отправка сообщения**:
    ```c
    char *message = "Hello, Shadowsocks!";
    unsigned char ciphertext[1024];
    unsigned long long ciphertext_len;
    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];

    randombytes_buf(nonce, sizeof nonce);

    if (crypto_aead_aes256gcm_encrypt(ciphertext, &ciphertext_len,
                                      (const unsigned char *)message, strlen(message),
                                      NULL, 0, NULL, nonce, key) != 0) {
        fprintf(stderr, "encryption failed\n");
        return 1;
    }
    printf("Message encrypted successfully\n");
    print_hex("Nonce", nonce, sizeof nonce);
    print_hex("Ciphertext", ciphertext, ciphertext_len);

    if (send(sockfd, nonce, sizeof nonce, 0) == -1) {
        perror("send nonce");
        return 1;
    }
    printf("Nonce sent successfully\n");

    if (send(sockfd, ciphertext, ciphertext_len, 0) == -1) {
        perror("send ciphertext");
        return 1;
    }
    printf("Ciphertext sent successfully\n");
    ```

7. **Ожидание ответа от сервера**:
    ```c
    printf("Waiting for response from server...\n");
    unsigned char buffer[1024];
    int len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Received response from server: %s\n", buffer);
    } else if (len == 0) {
        printf("Connection closed by peer\n");
    } else {
        perror("recv");
    }
    ```

8. **Закрытие сокета**:
    ```c
    #ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
    unset_system_proxy();
    printf("System proxy unset\n");
    #else
    close(sockfd);
    #endif

    printf("Socket closed successfully\n");
    printf("Press Enter to exit...");
    getchar();
    return 0;
    ```

## Описание файла `network.c`

Файл `network.c` реализует функции, объявленные в `network.h`:

1. **Реализация inet_pton для Windows**:
    ```c
    #ifdef _WIN32
    int inet_pton(int af, const char *src, void *dst) {
        // Реализация функции
    }
    #endif
    ```

2. **Установка системного прокси на Windows**:
    ```c
    void set_system_proxy(const char *proxy_address) {
        // Реализация функции
    }
    ```

3. **Отключение системного прокси на Windows**:
    ```c
    void unset_system_proxy() {
        // Реализация функции
    }
    ```

4. **Печать данных в шестнадцатеричном формате**:
    ```c
    void print_hex(const char *label, const unsigned char *data, size_t length) {
        // Реализация функции
    }
    ```

5. **Инициализация библиотеки libsodium**:
    ```c
    int initialize_sodium() {
        // Реализация функции
    }
    ```

6. **Настройка ключа шифрования**:
    ```c
    int setup_encryption_key(unsigned char *key) {
        // Реализация функции
    }
    ```

7. **Создание сокета для сетевого взаимодействия**:
    ```c
    int create_socket() {
        // Реализация функции
    }
    ```

8. **Подключение к серверу с использованием указанного IP-адреса и порта**:
    ```c
    int connect_to_server(int sockfd, const char *server_ip, int server_port) {
        // Реализация функции
    }
    ```

### Важные аспекты

1. **Платформозависимость**: Часть функций реализована специально для Windows (`inet_pton`, управление системным прокси), что обеспечивает корректную работу программы в этой операционной системе.
2. **Криптография**: Использование библиотеки libsodium для генерации ключей и шифрования данных. Это обеспечивает высокий уровень безопасности передаваемых данных.
3. **Сетевые операции**: Создание и управление сокетами, подключение к серверу, что позволяет программе обмениваться данными с удалённым сервером.
