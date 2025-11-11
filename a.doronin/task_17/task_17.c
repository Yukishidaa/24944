#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define MAX_LINE_LENGTH 40
#define BELL '\a'

static struct termios orig_term;

void restore_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

void setup_terminal(void)
{
    struct termios new_terminal;

    if (tcgetattr(STDIN_FILENO, &orig_term) == -1)
    {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    atexit(restore_terminal);

    new_terminal = orig_term;
    new_terminal.c_lflag &= ~(ICANON | ECHO);
    new_terminal.c_cc[VMIN] = 1;
    new_terminal.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal) == -1)
    {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void erase_last_char(char *line, int *pos)
{
    if (*pos > 0)
    {
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
        line[*pos] = '\0';
    }
    else
    {
        printf("%c", BELL);
        fflush(stdout);
    }
}

void kill_line(char *line, int *pos)
{
    while (*pos > 0)
    {
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
    }
    line[0] = '\0';
}

int is_space(char c)
{
    return (c == ' ' || c == '\t');
}

void erase_last_word(char *line, int *pos)
{
    int end_pos = *pos;
    while (*pos > 0 && is_space(line[*pos - 1]))
    {
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
    }

    while (*pos > 0 && !is_space(line[*pos - 1]))
    {
        (*pos)--;
        printf("\b \b");
        fflush(stdout);
    }

    line[*pos] = '\0';

    if (end_pos == *pos)
    {
        printf("%c", BELL);
        fflush(stdout);
    }
}

int is_printable(char c)
{
    return (c >= 32 && c <= 126);
}

int find_wrap_position(const char *line, int current_pos)
{
    if (current_pos <= MAX_LINE_LENGTH)
        return -1;

    for (int i = MAX_LINE_LENGTH - 1; i >= 0; i--)
    {
        if (is_space(line[i]))
        {
            return i + 1;
        }
    }

    return MAX_LINE_LENGTH;
}

void perform_word_wrap(char *line, int *pos, int *column)
{
    int wrap_pos = find_wrap_position(line, *pos);

    if (wrap_pos != -1 && wrap_pos < *pos)
    {
        for (int i = 0; i < *column; i++)
        {
            printf("\b \b");
        }

        for (int i = 0; i < wrap_pos; i++)
        {
            printf("%c", line[i]);
        }

        printf("\n");
        for (int i = wrap_pos; i < *pos; i++)
        {
            printf("%c", line[i]);
        }

        int remaining_chars = *pos - wrap_pos;
        for (int i = 0; i < remaining_chars; i++)
        {
            line[i] = line[wrap_pos + i];
        }
        line[remaining_chars] = '\0';

        *pos = remaining_chars;
        *column = remaining_chars;

        fflush(stdout);
    }
}

void check_line_wrap(char *line, int *pos, int *column)
{
    if (*column >= MAX_LINE_LENGTH)
    {
        perform_word_wrap(line, pos, column);
    }
}

int main(void)
{
    char line[MAX_LINE_LENGTH * 2] = {0};
    int pos = 0;
    int column = 0;
    char c;

    setup_terminal();

    printf("Line editor (max %d characters per line with word wrap)\n", MAX_LINE_LENGTH);
    printf("Controls:\n");
    printf("  Backspace or Ctrl+H - erase a character\n");
    printf("  Ctrl+U - erase the entire line\n");
    printf("  Ctrl+W - erase the previous word\n");
    printf("  Ctrl+D at the start of the line - exit\n");
    printf("  Other control characters - bell sound\n");
    printf("========================================\n");

    while (1)
    {
        if (read(STDIN_FILENO, &c, 1) != 1)
        {
            break;
        }

        if (c == 0x7F || c == 0x08)
        {
            erase_last_char(line, &pos);
            if (column > 0)
                column--;
        }
        else if (c == 0x15)
        {
            kill_line(line, &pos);
            column = 0;
        }
        else if (c == 0x17)
        {
            erase_last_word(line, &pos);
            column = pos;
        }
        else if (c == 0x04)
        {
            if (pos == 0)
            {
                printf("\nExit...\n");
                break;
            }
            else
            {
                printf("%c", BELL);
                fflush(stdout);
            }
        }
        else if (c == '\n' || c == '\r')
        {
            printf("\n");
            pos = 0;
            column = 0;
            line[0] = '\0';
        }
        else if (is_printable(c))
        {
            if (pos < MAX_LINE_LENGTH * 2 - 1)
            {
                line[pos++] = c;
                line[pos] = '\0';
                printf("%c", c);
                fflush(stdout);
                column++;
                check_line_wrap(line, &pos, &column);
            }
            else
            {
                printf("%c", BELL);
                fflush(stdout);
            }
        }
        else
        {
            printf("%c", BELL);
            fflush(stdout);
        }
    }

    return 0;
}