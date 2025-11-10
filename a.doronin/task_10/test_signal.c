#include <stdlib.h>
#include <signal.h>

int main()
{
    raise(SIGINT); // signal interrupt
    return 0;
}