
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HOST "127.0.0.1"
#define PORT 8080
#define TEAM "t1"

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
    send_cmd(fd, TEAM "\n");
    printf("SERVER > %s\n", recv_line(fd));   // CLIENT-NUM
    printf("SERVER > %s\n", recv_line(fd));   // X Y

    // send commands without waiting (spec allows up to 10)
    const char *cmds[] = {
        "Take\n",
        "Set\n",
        "Inventory\n",
        "Look\n",
        "Forward\n",
        "Right\n",
        "Forward\n",
        "Left\n",
        "Inventory\n",
        "Broadcast hello world\n",
        "Look\n",
        NULL
    };

    for (int i = 0; cmds[i]; i++)
        send_cmd(fd, cmds[i]);

    // for (volatile int o = 0; o < 10000000; o++) {}
    //read responses
    char *line;
    while ((line = recv_line(fd)) && strlen(line) > 0)
        printf("SERVER > %s\n", line);

    close(fd);
    return 0;
}
