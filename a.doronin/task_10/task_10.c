#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments %s\n\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        pid_t w = waitpid(pid, &status, 0);

        if (w == -1)
        {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
        {
            printf("\nПотомок завершился с кодом %d\n\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("\nПотомок убит сигналом %d\n\n", WTERMSIG(status));
        }
    }

    return 0;
}