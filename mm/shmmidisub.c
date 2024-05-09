#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <assert.h>

int main(int argc, char **argv)
{
    int oflags=O_RDWR;
    int i;

    char * name = "akai.mpd218.00";
    int fd = shm_open(name, oflags, 0644);

    fprintf(stderr, "[shm] fd: %d\n", fd);
    assert (fd>0);

    struct stat sb;
    fstat(fd, &sb);
    off_t length = sb.st_size;

    u_char * ptr = (u_char *) mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    fprintf(stderr, "[shm] addr: %p [0..%lu]\n", ptr, length-1);
    assert (ptr);

    int nctr = 0;
    int state = 0;
    for(;;)
    {
        printf("%d %d %d %d\n", ptr[0], ptr[1], ptr[2], ptr[3]);
    }

    close(fd);
    exit(0);
}
