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
        printf("usage: ./refacetest </dev/midiX>\n");
        exit(0);
    }
    FILE * yc;
    unsigned char buf[1];

    // midi
    printf("[midi] opening yamaha reface yc (%s) ...\n", argv[1]);
    yc = fopen(argv[1], "rb");
    int p, v, note;
    int lo=24,hi=108;

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
            if(v==0) printf("off p: %d\n", p);
            else printf("on p: %d, v: %d\n", p, v);
        }
    }
    fclose(yc);
    exit(0);
}
