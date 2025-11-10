#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main()
{
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {

        close(pipefd[1]); // close write

        while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE - 1)) > 0)
        {
            buffer[bytes_read] = '\0';

            for (int i = 0; i < bytes_read; i++)
            {
                buffer[i] = toupper(buffer[i]);
            }

            printf("Child process: %s", buffer);
            fflush(stdout);
        }

        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(pipefd[0]);
        char *messages[] = {
            "Hello World!\n",
            "I love C!!!\n",
            "I love OS!!!!\n",
            "I love linux!!!\n"};

        int num_messages = sizeof(messages) / sizeof(messages[0]);

        for (int i = 0; i < num_messages; i++)
        {
            printf("The parent send a message: %s", messages[i]);
            write(pipefd[1], messages[i], strlen(messages[i]));
            sleep(3);
        }

        close(pipefd[1]);
        wait(NULL);
        printf("The parent process is completed.\n");
    }

    return 0;
}