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
    FILE * dm;
    unsigned char buf[1];
    int state = 0;

    int bot=12,top=108;
    off_t l = top-bot+1;
    char * name = "behringer.deepmind.12";
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
    printf("[midi] opening behringer deepmind 12 (%s) ...\n", argv[1]);
    dm = fopen(argv[1], "rb");
    int p, v, note;

    for(;;)
    {
        fread(buf, sizeof(buf), 1, dm);
        int hi=buf[0]>>4;
        int lo=buf[0]&0xf;
        if(hi==0x9) // note_on
        {
            fread(buf,sizeof(buf),1,dm);
            p=buf[0];
            fread(buf,sizeof(buf),1,dm);
            v=buf[0];
            if (p >= bot && p <= top) ptr[p-bot] = v;
            else fprintf(stderr, "exceeded keyboard range!\n");
        }
        else if(hi==0x8) // note_off
        {
            fread(buf,sizeof(buf),1,dm);
            p=buf[0];
            fread(buf,sizeof(buf),1,dm);
            v=buf[0];
            if (p >= bot && p <= top) ptr[p-bot] = 0;
            else fprintf(stderr, "exceeded keyboard range!\n");
        }
        else if(buf[0]==0xd0) // pressure
        {
            fread(buf,sizeof(buf),1,dm);
            int cp=buf[0];
            // TODO later
        }
        else if(buf[0]==0xb0) // control change
        {
            fread(buf,sizeof(buf),1,dm);
            int ccid=buf[0];
            fread(buf,sizeof(buf),1,dm);
            int ccv=buf[0];
            // TODO later
        }
    }
    fclose(dm);

    close(fd);
    exit(0);
}
