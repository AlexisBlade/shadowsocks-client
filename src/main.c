#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#define SERVER_IP "38.180.134.199"
#define SERVER_PORT 8388
#define PASSWORD "Indigo2017"
#define METHOD "aes-256-gcm"


int main() {
    if (sodium_init() < 0) {
        fprintf(stderr, "libsodium initialization failed\n");
        return 1;
    }

    // Setting up the encryption key
    unsigned char key[crypto_aead_aes256gcm_KEYBYTES];
    if (crypto_pwhash(key, sizeof key, PASSWORD, strlen(PASSWORD),
                      (const unsigned char *) "shadowsocks", crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT) != 0) {
        fprintf(stderr, "crypto_pwhash failed\n");
        return 1;
    }

    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
    #endif

    // Create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error occurred\n");
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    // Sending a test message (this part should be replaced with actual Shadowsocks protocol)
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

    send(sockfd, nonce, sizeof nonce, 0);
    send(sockfd, ciphertext, ciphertext_len, 0);

    #ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
    #else
    close(sockfd);
    #endif

    return 0;
}
