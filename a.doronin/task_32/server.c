#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/uppercase_socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int server_fd;
int client_fds[MAX_CLIENTS];

void handle_sigio(int sig)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int fd = client_fds[i];
        if (fd != -1)
        {
            // Чтение данных без блокировки
            while ((bytes_read = read(fd, buffer, BUFFER_SIZE - 1)) > 0)
            {
                buffer[bytes_read] = '\0';
                for (ssize_t j = 0; j < bytes_read; j++)
                    buffer[j] = toupper((unsigned char)buffer[j]);
                printf("FD=%d: %s", fd, buffer);
                fflush(stdout);
            }
            if (bytes_read == 0)
            {
                printf("Client FD=%d disconnected\n", fd);
                close(fd);
                client_fds[i] = -1;
            }
        }
    }
}

int main()
{
    struct sockaddr_un server_addr;

    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
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

    printf("Async server listening on %s\n", SOCKET_PATH);

    // Инициализируем клиентские FD
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_fds[i] = -1;

    // Настраиваем обработчик сигнала SIGIO
    struct sigaction sa;
    sa.sa_handler = handle_sigio;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGIO, &sa, NULL) == -1)
    {
        perror("sigaction");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Устанавливаем серверный FD как non-blocking
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC);
    fcntl(server_fd, F_SETOWN, getpid());

    // Основной цикл: принимаем новых клиентов
    while (1)
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("accept");
            }
            usleep(100000); // 0.1s пауза, чтобы не крутить CPU
            continue;
        }

        printf("New client connected: FD=%d\n", client_fd);

        // Non-blocking + SIGIO для клиента
        int cflags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, cflags | O_NONBLOCK | O_ASYNC);
        fcntl(client_fd, F_SETOWN, getpid());

        // Сохраняем клиента
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
            printf("Too many clients, closing FD=%d\n", client_fd);
            close(client_fd);
        }
    }

    // Закрытие
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (client_fds[i] != -1)
            close(client_fds[i]);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
