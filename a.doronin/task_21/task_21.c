#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int beep_count = 0;

void sigint_handler(int sig)
{
    signal(SIGINT, sigint_handler);
    beep_count++;
    printf("\r\aBEEP! \n");
    fflush(stdout);
}

void sigquit_handler(int sig)
{
    printf("\r\e[2K");
    printf("\nProgram finished.\n");
    printf("Count of signals: %d\n", beep_count);
    exit(0);
}

int main()
{
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);

    printf("Program started. PID: %d\n", getpid());
    printf("Press Ctrl+C for sound signal\n");
    printf("Press Ctrl+\\ to show statistics and exit\n");
    printf("========================================\n");

    while (1)
    {
        pause();
    }

    return 0;
}