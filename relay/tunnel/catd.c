#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int  fd, fdw;
    int  pid;
    char buffer[5];
    ssize_t nrd;

    fd = open("/dev/car/motor_tun", O_RDONLY);

    read(fd, buffer, 5);
    write(1, buffer, 4);

    close(fd);

    _exit(0);
}
