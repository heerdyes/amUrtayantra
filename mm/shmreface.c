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
        printf("usage: ./shmreface </dev/midiX>\n");
        exit(0);
    }
    int oflags = O_RDWR | O_CREAT;
    FILE * yc;
    unsigned char buf[1];
    int state = 0;

    int lo=24,hi=108;
    off_t l = hi-lo+1;
    char * name = "yamaha.refaceyc.00";
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
    printf("[midi] opening yamaha reface yc (%s) ...\n", argv[1]);
    yc = fopen(argv[1], "rb");
    int p, v, note;

    for(;;)
    {
        fread(buf, sizeof(buf), 1, yc);
        if(buf[0]==0xfe) continue;
        if (buf[0] == 0x90) // reface note_on and note_off are both 0x90 lol
        {
            fread(buf, sizeof(buf), 1, yc);
            p = buf[0];
            fread(buf, sizeof(buf), 1, yc);
            v = buf[0];
            if (p >= lo && p <= hi) ptr[p-lo] = v;
            else fprintf(stderr, "exceeded pad range!\n");
        }
    }
    fclose(yc);

    close(fd);
    exit(0);
}
