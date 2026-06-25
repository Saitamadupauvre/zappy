
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HOST "127.0.0.1"
#define PORT 8080

static char *recv_line(int fd)
{
    static char buf[4096];
    char c;
    int i = 0;

    while (read(fd, &c, 1) > 0 && c != '\n' && i < 4095)
        buf[i++] = c;
    buf[i] = '\0';
    return buf;
}

static void send_cmd(int fd, const char *cmd)
{
    send(fd, cmd, strlen(cmd), 0);
    printf("CLIENT > %s", cmd);
}

int main(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(PORT),
    };
    inet_pton(AF_INET, HOST, &addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    // handshake
    printf("SERVER > %s\n", recv_line(fd));   // WELCOME
    send_cmd(fd, "GRAPHIC\n");

    // read entire initial state dump
    char *line;
    while ((line = recv_line(fd)) && strlen(line) > 0) {
        printf("SERVER > %s\n", line);
    }

    // burst commands: send multiple requests without waiting
    const char *burst[] = {
        "msz\n",
        "sgt\n",
        NULL
    };

    for (int i = 0; burst[i]; i++)
        send_cmd(fd, burst[i]);

    // read burst responses
    while ((line = recv_line(fd)) && strlen(line) > 0)
        printf("SERVER > %s\n", line);

    close(fd);
    return 0;
}
