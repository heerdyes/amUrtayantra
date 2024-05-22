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
        printf("usage: ./deepmindtest </dev/midiX>\n");
        exit(0);
    }
    FILE * dm;
    unsigned char buf[1];

    // midi
    printf("[midi] opening behringer deepmind 12 (%s) ...\n", argv[1]);
    dm = fopen(argv[1], "rb");
    int p, v, note, cp, ch;

    for(;;)
    {
        fread(buf, sizeof(buf), 1, dm);
        if(buf[0]==0xfe) continue;
        int hi=buf[0]>>4;
        int lo=buf[0]&0xf;
        printf("hi: 0x%x, lo: 0x%x\n",hi,lo);
        if(hi==0x9) // note_on
        {
            fread(buf,sizeof(buf),1,dm);
            p=buf[0];
            fread(buf,sizeof(buf),1,dm);
            v=buf[0];
            printf("on p=%d, v=%d, ch=%d\n",p,v,lo);
        }
        else if(hi==0x8) // note_off
        {
            fread(buf,sizeof(buf),1,dm);
            p=buf[0];
            fread(buf,sizeof(buf),1,dm);
            v=buf[0];
            printf("off p=%d, v=%d, ch=%d\n",p,v,lo);
        }
        else if(buf[0]==0xd0) // pressure
        {
            fread(buf,sizeof(buf),1,dm);
            cp=buf[0];
            printf("cp=%d\n",cp);
        }
        else if(buf[0]==0xb0) // control change
        {
            fread(buf,sizeof(buf),1,dm);
            int ccid=buf[0];
            fread(buf,sizeof(buf),1,dm);
            int ccv=buf[0];
            printf("ccid=%x, ccv=%d\n",ccid,ccv);
        }
    }
    fclose(dm);
    exit(0);
}
