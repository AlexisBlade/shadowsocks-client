#include "network.h"

// Function to print data in hexadecimal format
void print_hex(const char *label, const unsigned char *data, size_t length) {
    printf("%s", label);
    for (size_t i = 0; i < length; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    printf("Starting Shadowsocks client...\n");

    // Initialize the Sodium library
    if (initialize_sodium() < 0) {
        return 1;
    }

    // Generate the encryption key
    unsigned char key[crypto_aead_aes256gcm_KEYBYTES];
    if (setup_encryption_key(key) < 0) {
        return 1;
    }
    printf("Encryption key setup successfully\n");
    print_hex("Encryption key: ", key, crypto_aead_aes256gcm_KEYBYTES);

    // Initialize Windows Sockets (only for Windows)
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
    printf("WSAStartup initialized successfully\n");

    // Set system proxy to localhost:1080
    set_system_proxy("http=127.0.0.1:1080");
    printf("System proxy set to 127.0.0.1:1080\n");
    #endif

    // Create the network socket
    int sockfd = create_socket();
    if (sockfd < 0) {
        return 1;
    }
    printf("Socket created successfully\n");

    // Connect to the server
    if (connect_to_server(sockfd, SERVER_IP, SERVER_PORT) < 0) {
        return 1;
    }
    printf("Connected to the server successfully\n");

    // Encrypt and send a test message
    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];
    randombytes_buf(nonce, sizeof nonce);
    printf("Nonce: ");
    print_hex("Nonce: ", nonce, sizeof nonce);

    const char *message = "Hello Shadowsocks";
    unsigned char ciphertext[128];
    unsigned long long ciphertext_len;

    if (crypto_aead_aes256gcm_encrypt(ciphertext, &ciphertext_len, (const unsigned char *)message, strlen(message), NULL, 0, NULL, nonce, key) != 0) {
        fprintf(stderr, "Encryption failed\n");
        return 1;
    }
    printf("Ciphertext: ");
    print_hex("Ciphertext: ", ciphertext, ciphertext_len);

    // Send nonce
    if (send(sockfd, nonce, sizeof nonce, 0) < 0) {
        perror("send nonce");
        return 1;
    }
    printf("Nonce sent successfully\n");

    // Send ciphertext
    if (send(sockfd, ciphertext, ciphertext_len, 0) < 0) {
        perror("send ciphertext");
        return 1;
    }
    printf("Ciphertext sent successfully\n");

    // Wait for response (for example purposes, not needed for Shadowsocks)
    char buffer[128];
    int bytes_received = recv(sockfd, buffer, sizeof buffer, 0);
    if (bytes_received < 0) {
        perror("recv");
        return 1;
    }
    printf("Received response: ");
    print_hex("Received response: ", (unsigned char *)buffer, bytes_received);

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
