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
    int nknobs=6*3,ncp=1,nch=1; // 6*3 knobs, 16 pads, 1 channel pressure slot, 1 channel slot
    int lo=36,hi=83;

    off_t l = nknobs+(hi-lo+1)+ncp+nch;
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

    for(;;)
    {
        int p, v;
        fread(buf, sizeof(buf), 1, mpd);
        // split into hi and lo nibbles
        int nibhi=buf[0]>>4;
        int niblo=buf[0]&0xf;
        if (nibhi == 0x9)
        {
            fread(buf, sizeof(buf), 1, mpd);
            p = buf[0];
            fread(buf, sizeof(buf), 1, mpd);
            v = buf[0];
            if (p >= lo && p <= hi)
            {
                ptr[p-lo+nknobs] = v;
                ptr[l-1]=niblo;
            }
            else fprintf(stderr, "exceeded pad range!\n");
        }
        else if (nibhi == 0x8)
        {
            fread(buf, sizeof(buf), 1, mpd);
            p = buf[0];
            fread(buf, sizeof(buf), 1, mpd);
            if (p >= lo && p <= hi)
            {
                ptr[p-lo+nknobs] = 0;
                ptr[l-1]=niblo;
            }
            else fprintf(stderr, "exceeded pad range!\n");
        }
        else if(buf[0]==0xd0)
        {
            fread(buf,sizeof(buf),1,mpd);
            int cp=buf[0];
            ptr[l-2]=cp;
        }
        else if(buf[0]==0xb0)
        {
            fread(buf,sizeof(buf),1,mpd);
            int ccid=buf[0];
            fread(buf,sizeof(buf),1,mpd);
            int ccv=buf[0];
            if(ccid>=0xc&&ccid<=0x1b) ptr[ccid-0xa]=ccv;
            else
            {
                switch(ccid)
                {
                    case 0x3: ptr[0]=ccv; break;
                    case 0x9: ptr[1]=ccv; break;
                    default: fprintf(stderr,"illegal ccid: 0x%x\n",ccid);
                }
            }
        }
    }
    fclose(mpd);

    close(fd);
    exit(0);
}
