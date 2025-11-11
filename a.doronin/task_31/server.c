#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/uppercase_socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main()
{
    int server_fd, client_fd, max_fd, activity;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    int client_fds[MAX_CLIENTS];

    unlink(SOCKET_PATH);

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s\n", SOCKET_PATH);

    for (int i = 0; i < MAX_CLIENTS; i++)
        client_fds[i] = -1;

    fd_set read_fds;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] > 0)
                FD_SET(client_fds[i], &read_fds);
            if (client_fds[i] > max_fd)
                max_fd = client_fds[i];
        }

        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1)
            {
                perror("accept");
                continue;
            }

            printf("New client connected: FD=%d\n", client_fd);

            int added = 0;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_fds[i] == -1)
                {
                    client_fds[i] = client_fd;
                    added = 1;
                    break;
                }
            }
            if (!added)
            {
                printf("Too many clients, connection refused\n");
                close(client_fd);
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = client_fds[i];
            if (fd != -1 && FD_ISSET(fd, &read_fds))
            {
                ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
                if (bytes_read <= 0)
                {
                    if (bytes_read == 0)
                        printf("Client FD=%d disconnected\n", fd);
                    else
                        perror("read");

                    close(fd);
                    client_fds[i] = -1;
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    for (ssize_t j = 0; j < bytes_read; j++)
                        buffer[j] = toupper((unsigned char)buffer[j]);
                    printf("FD=%d: %s", fd, buffer);
                    fflush(stdout);
                }
            }
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
        if (client_fds[i] != -1)
            close(client_fds[i]);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
