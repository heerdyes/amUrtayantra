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
        printf("usage: ./akaitest </dev/midiX>\n");
        exit(0);
    }
    FILE * mpd;
    unsigned char buf[1];

    // midi
    printf("[midi] opening akai mpd218 (%s) ...\n", argv[1]);
    mpd = fopen(argv[1], "rb");
    int p, v, note, cp, ch;

    for(;;)
    {
        fread(buf, sizeof(buf), 1, mpd);
        if(buf[0]==0xfe) continue;
        int hi=buf[0]>>4;
        int lo=buf[0]&0xf;
        printf("hi: 0x%x, lo: 0x%x\n",hi,lo);
        if(hi==0x9) // note_on
        {
            fread(buf,sizeof(buf),1,mpd);
            p=buf[0];
            fread(buf,sizeof(buf),1,mpd);
            v=buf[0];
            printf("on p=%d, v=%d, ch=%d\n",p,v,lo);
        }
        else if(hi==0x8) // note_off
        {
            fread(buf,sizeof(buf),1,mpd);
            p=buf[0];
            fread(buf,sizeof(buf),1,mpd);
            v=buf[0];
            printf("off p=%d, v=%d, ch=%d\n",p,v,lo);
        }
        else if(buf[0]==0xd0) // pressure
        {
            fread(buf,sizeof(buf),1,mpd);
            cp=buf[0];
            printf("cp=%d\n",cp);
        }
        else if(buf[0]==0xb0)
        {
            fread(buf,sizeof(buf),1,mpd);
            int ccid=buf[0];
            fread(buf,sizeof(buf),1,mpd);
            int ccv=buf[0];
            printf("ccid=%x, ccv=%d\n",ccid,ccv);
        }
    }
    fclose(mpd);
    exit(0);
}
