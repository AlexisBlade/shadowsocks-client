#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>

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

#define SERVER_IP "38.180.134.199"
#define SERVER_PORT 8388
#define PASSWORD "Indigo2017"
#define METHOD "aes-256-gcm"

#ifdef _WIN32
int inet_pton(int af, const char *src, void *dst) {
    struct sockaddr_storage ss;
    int size = sizeof(ss);
    char src_copy[INET6_ADDRSTRLEN+1];

    ZeroMemory(&ss, sizeof(ss));
    strncpy(src_copy, src, INET6_ADDRSTRLEN+1);
    src_copy[INET6_ADDRSTRLEN] = 0;

    if (WSAStringToAddressA(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
        switch (af) {
            case AF_INET:
                *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
                return 1;
            case AF_INET6:
                *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
                return 1;
        }
    }
    return 0;
}

void set_system_proxy(const char *proxy_address) {
    INTERNET_PER_CONN_OPTION_LIST option_list;
    INTERNET_PER_CONN_OPTION option[3];
    unsigned long list_size = sizeof(option_list);

    option[0].dwOption = INTERNET_PER_CONN_FLAGS;
    option[0].Value.dwValue = PROXY_TYPE_DIRECT | PROXY_TYPE_PROXY;

    option[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    option[1].Value.pszValue = (char *)proxy_address;

    option[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    option[2].Value.pszValue = "local";

    option_list.dwSize = sizeof(option_list);
    option_list.pszConnection = NULL;
    option_list.dwOptionCount = 3;
    option_list.dwOptionError = 0;
    option_list.pOptions = option;

    if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, list_size)) {
        printf("Failed to set proxy settings\n");
    } else {
        printf("Proxy settings set successfully\n");
    }
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
}

void unset_system_proxy() {
    INTERNET_PER_CONN_OPTION_LIST option_list;
    INTERNET_PER_CONN_OPTION option;
    unsigned long list_size = sizeof(option_list);

    option.dwOption = INTERNET_PER_CONN_FLAGS;
    option.Value.dwValue = PROXY_TYPE_DIRECT;

    option_list.dwSize = sizeof(option_list);
    option_list.pszConnection = NULL;
    option_list.dwOptionCount = 1;
    option_list.dwOptionError = 0;
    option_list.pOptions = &option;

    if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, list_size)) {
        printf("Failed to unset proxy settings\n");
    } else {
        printf("Proxy settings unset successfully\n");
    }
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
}
#endif

int main() {
    if (sodium_init() < 0) {
        fprintf(stderr, "libsodium initialization failed\n");
        return 1;
    }

    printf("Libsodium initialized successfully\n");

    // Setting up the encryption key
    unsigned char key[crypto_aead_aes256gcm_KEYBYTES];
    if (crypto_pwhash(key, sizeof key, PASSWORD, strlen(PASSWORD),
                      (const unsigned char *) "shadowsocks", crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT) != 0) {
        fprintf(stderr, "crypto_pwhash failed\n");
        return 1;
    }

    printf("Encryption key setup successfully\n");

    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
    printf("WSAStartup initialized successfully\n");

    // Set system proxy
    set_system_proxy("127.0.0.1:1080");
    printf("System proxy set to 127.0.0.1:1080\n");
    #endif

    // Create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    printf("Socket created successfully\n");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error occurred\n");
        return 1;
    }

    printf("Server address setup successfully\n");

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Connected to the server successfully\n");

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

    printf("Message encrypted successfully\n");

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

    // Receive response from the server
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

    #ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();

    // Unset system proxy
    unset_system_proxy();
    printf("System proxy unset\n");
    #else
    close(sockfd);
    #endif

    printf("Socket closed successfully\n");

    printf("Press Enter to exit...");
    getchar();  // Wait for Enter key press before exiting
    return 0;
}
