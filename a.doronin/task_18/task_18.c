#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void print_file_info(const char *filename)
{
    struct stat st;
    struct passwd *pw;
    struct group *gr;
    char mode[11];
    char time_str[20];
    const char *basename;

    if (lstat(filename, &st) == -1)
    {
        perror("lstat");
        return;
    }

    mode[0] = S_ISDIR(st.st_mode) ? 'd' : S_ISREG(st.st_mode) ? '-'
                                                              : '?';

    mode[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    mode[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    mode[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';

    mode[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    mode[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    mode[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';

    mode[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    mode[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    mode[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    mode[10] = '\0';

    pw = getpwuid(st.st_uid);
    gr = getgrgid(st.st_gid);

    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);

    basename = strrchr(filename, '/');
    if (basename != NULL)
    {
        basename++;
    }
    else
    {
        basename = filename;
    }

    printf("%s %lu %-10.10s %-10.10s ",
           mode,
           st.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?");

    if (S_ISREG(st.st_mode))
    {
        printf("%8ld ", (long)st.st_size);
    }
    else
    {
        printf("%8ld ", (long)st.st_size);
    }

    printf("%s %s\n", time_str, basename);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_file_info(".");
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            print_file_info(argv[i]);
        }
    }

    return 0;
}