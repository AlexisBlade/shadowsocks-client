#ifndef NETWORK_H
#define NETWORK_H

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

void print_hex(const char *label, const unsigned char *data, size_t length);
void set_system_proxy(const char *proxy_address);
void unset_system_proxy();
int initialize_sodium();
int setup_encryption_key(unsigned char *key);
int create_socket();
int connect_to_server(int sockfd, const char *server_ip, int server_port);

#endif
