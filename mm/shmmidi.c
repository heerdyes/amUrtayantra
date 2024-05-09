#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        printf("usage: ./shmmidi </dev/midi>\n");
        exit(0);
    }
    int oflags = O_RDWR | O_CREAT;
    FILE * mpd;
    unsigned char buf[1];
    int state = 0;

    off_t l = 16;
    char * name = "akai.mpd218.00";
    int fd = shm_open(name, oflags, 0644);

    ftruncate(fd, l);
    fprintf(stderr, "[shm] fd: %d\n", fd);
    assert(fd>0);

    u_char * ptr = (u_char *) mmap(NULL, l, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    fprintf(stderr, "[shm] addr: %p [0..%lu]\n", ptr, l-1);
    fprintf(stderr, "[shm] path: /dev/shm/%s\n", name);
    assert(ptr);

    // init mem
    for(int i=0; i<l; i++)
    {
        ptr[i] = 0;
    }

    // midi
    printf("[midi] opening akai mpd218 (%s) ...\n", argv[1]);
    mpd = fopen(argv[1], "rb");
    int p, v, note;

    for(;;)
    {
        fread(buf, sizeof(buf), 1, mpd);
        //printf("%d\n", buf[0]);
        if (buf[0] == 144)
        {
            fread(buf, sizeof(buf), 1, mpd);
            p = buf[0];
            fread(buf, sizeof(buf), 1, mpd);
            v = buf[0];
            if (p >= 36 && p <= 51)
            {
                //printf("on p: %d, v: %d\n", p, v);
                ptr[p-36] = v;
            }
            else
            {
                fprintf(stderr, "exceeded pad range!\n");
            }
        }
        else if (buf[0] == 128)
        {
            fread(buf, sizeof(buf), 1, mpd);
            p = buf[0];
            fread(buf, sizeof(buf), 1, mpd);
            if (p >= 36 && p <= 51)
            {
                //printf("off p: %d, v: %d\n", p, v);
                ptr[p-36] = 0;
            }
            else
            {
                fprintf(stderr, "exceeded pad range!\n");
            }
        }
    }
    fclose(mpd);

    close(fd);
    exit(0);
}
