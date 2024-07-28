#include "network.h"

int main() {
    // Sodium init
    if (initialize_sodium() < 0) {
        return 1;
    }

    // Crypto key gen
    unsigned char key[crypto_aead_aes256gcm_KEYBYTES];
    if (setup_encryption_key(key) < 0) {
        return 1;
    }

    // Windows Sockets init
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

    // Network Socket creation 
    int sockfd = create_socket();
    if (sockfd < 0) {
        return 1;
    }
    
    // Connection to server
    if (connect_to_server(sockfd, SERVER_IP, SERVER_PORT) < 0) {
        return 1;
    }

    char *message = "Hello, Shadowsocks!";
    unsigned char ciphertext[1024]; // array for encrypted text
    unsigned long long ciphertext_len; // for length of ciphertext
    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES]; // array for nonce

    randombytes_buf(nonce, sizeof nonce); // generation random bytes for nonce array (for unique encryption)

    // message encryption
    if (crypto_aead_aes256gcm_encrypt(ciphertext, &ciphertext_len,
                                      (const unsigned char *)message, strlen(message),
                                      NULL, 0, NULL, nonce, key) != 0) {
        fprintf(stderr, "encryption failed\n");
        return 1;
    }

    printf("Message encrypted successfully\n");
    print_hex("Nonce", nonce, sizeof nonce);
    print_hex("Ciphertext", ciphertext, ciphertext_len);

    // sending nonce via sockfd by send function
    if (send(sockfd, nonce, sizeof nonce, 0) == -1) {
        perror("send nonce");
        return 1;
    }
    printf("Nonce sent successfully\n");

    // sending crypted text via sockfd by send function
    if (send(sockfd, ciphertext, ciphertext_len, 0) == -1) {
        perror("send ciphertext");
        return 1;
    }
    printf("Ciphertext sent successfully\n");

    // Wait for a response from the server
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
}
