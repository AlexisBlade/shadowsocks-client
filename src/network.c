#include "network.h"

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

void print_hex(const char *label, const unsigned char *data, size_t length) {
    printf("%s: ", label);
    for (size_t i = 0; i < length; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int initialize_sodium() {
    if (sodium_init() < 0) {
        fprintf(stderr, "libsodium initialization failed\n");
        return -1;
    }
    printf("Libsodium initialized successfully\n");
    return 0;
}

int setup_encryption_key(unsigned char *key) {
    if (crypto_pwhash(key, crypto_aead_aes256gcm_KEYBYTES, PASSWORD, strlen(PASSWORD),
                      (const unsigned char *) "shadowsocks", crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT) != 0) {
        fprintf(stderr, "crypto_pwhash failed\n");
        return -1;
    }
    printf("Encryption key setup successfully\n");
    print_hex("Encryption key", key, crypto_aead_aes256gcm_KEYBYTES);
    return 0;
}

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    printf("Socket created successfully\n");
    return sockfd;
}

int connect_to_server(int sockfd, const char *server_ip, int server_port) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error occurred\n");
        return -1;
    }
    printf("Server address setup successfully\n");
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return -1;
    }
    printf("Connected to the server successfully\n");
    return 0;
}
