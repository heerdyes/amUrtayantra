#include "ofApp.h"

void ofApp::setupyc(){
    kbfd=shm_open(reface, O_RDWR, 0644);
    fprintf(stderr, "[shm][yc] fd: %d\n", kbfd);
    if(kbfd>0){
        struct stat sby;
        fstat(kbfd,&sby);
        kblen=sby.st_size;
        kb=(u_char *)mmap(NULL, kblen, PROT_READ|PROT_WRITE, MAP_SHARED, kbfd, 0);
        fprintf(stderr, "[shm] addr: %p [0..%lu]\n", kb, kblen-1);
        if(kb){
            cout<<"[yc] all in order.\n";
        }else{
            cout<<"[error] the end is here!\n";
        }
    }
}

void ofApp::setupdm(){
    kbfd=shm_open(deepmind, O_RDWR, 0644);
    fprintf(stderr, "[shm][dm] fd: %d\n", kbfd);
    if(kbfd>0){
        struct stat sby;
        fstat(kbfd,&sby);
        kblen=sby.st_size;
        kb=(u_char *)mmap(NULL, kblen, PROT_READ|PROT_WRITE, MAP_SHARED, kbfd, 0);
        fprintf(stderr, "[shm] addr: %p [0..%lu]\n", kb, kblen-1);
        if(kb){
            cout<<"[dm] all in order.\n";
        }else{
            cout<<"[error] the end is here!\n";
        }
    }
}

void ofApp::setupsndsys(){
    ofSoundStreamSettings settings;
	settings.numOutputChannels = NOUTCH;
	settings.sampleRate = SMPLRATE;
	settings.bufferSize = BUFSZ;
	settings.numBuffers = NBUF;
	settings.setOutListener(this);
	soundStream.setup(settings);
}

void ofApp::initvm(){
    for(int i=0;i<MEMLEN;i++){
        M[i]='.';
    }
    // registers
    pc=0;
    ec=0;
    f5times=2;
    lcjmp=10;
    lcdown=0;
    vmx=30.;
    vmy=55.;
}

void ofApp::initlogo(){
    for(int i=0;i<NOSCS;i++){
        float x=ofRandom(0,ofGetWidth());
        float y=ofRandom(0,ofGetHeight());
        tx[i]=new trtl(x,y,1,0);
    }
    txctr=0;
    tpc=0;
}

void ofApp::color12(int c){
    int b=ofMap(c&0xf,0,0xf,0,255);
    int g=ofMap((c&0xf0)>>4,0,0xf,0,255);
    int r=ofMap((c&0xf00)>>8,0,0xf,0,255);
    ofSetColor(r,g,b);
}

void ofApp::cyclevm(){
    char code=M[pc];
    int pos,src,dst,xpc,arg1,arg2,arg3,arg4,arg5;
    int ma1,ma2;
    float hh,r0,cw=18.,ch=20.;
    float cvargs[3]={0.,0.,0.};
    switch(code){
        case '.':
            pc=0;
            break;
        case 'n':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            M[pc]=AB[activenote];
            pc=(pc+1)%MEMLEN;
            break;
        case 'g':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            r0=((float)notevelo / 128.) * (float)MEMLEN;
            M[pc]=AB[floor(r0)];
            pc=(pc+1)%MEMLEN;
            break;
        case '^': // ramp up wraparound
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                M[pc]=AB[(pos+1)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '>': // ++ if M[arg1] multiple of arg2: > 'addr' 'num' '_'
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg3=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg2!=0&&arg3!=-1){
                if(AB.find(M[arg1])%arg2==0){
                    M[pc]=AB[(arg3+1)%MEMLEN];
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '<': // ++ if M[arg1] multiple of arg2: > 'addr' 'num' '_'
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg3=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg2!=0&&arg3!=-1){
                if(AB.find(M[arg1])%arg2==0){
                    M[pc]=AB[arg3==0?(MEMLEN-1):(arg3-1)];
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'A': // ramp up till limit
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            M[pc]=AB[arg2>=arg1?0:arg2+1];
            pc=(pc+1)%MEMLEN;
            break;
        case 'v': // ramp down wraparound
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                M[pc]=AB[pos==0?MEMLEN-1:pos-1];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'V': // ramp down from limit
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            M[pc]=AB[arg2==0||arg2>arg1?arg1:arg2-1];
            pc=(pc+1)%MEMLEN;
            break;
        case 'L': // delay loop
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pos=(pc+1)%MEMLEN;
            arg1=AB.find(M[pos]);
            xpc=(pc+2)%MEMLEN;
            arg2=AB.find(M[xpc]);
            if(arg2==0){
                M[xpc]=M[pos];
                pc=(pc+3)%MEMLEN;
            }else{
                M[xpc]=AB[arg2-1];
            }
            break;
        case '2': // harmonix tuning
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                roothx=ofMap(arg1,0,MEMLEN-1,rootflo,rootfhi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '3': // fine tuning keys
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1){
                finetuning[arg1%12]=ofMap(arg2,0,MEMLEN-1,tunlo,tunhi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'f':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                f5times=pos; // invoking with 0 causes system hang :D
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'c': // cp OR patch/connect
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            src=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            dst=AB.find(M[pc]);
            if(src!=-1&&dst!=-1){
                M[dst]=M[src];
                color12(cablehues[pc%NHUES]);
                hh=vmy+40.;
                r0=(vmx+5+src*cw + vmx+5+dst*cw) / 2.;
                ofDrawLine(vmx+src*cw,hh,vmx+10+src*cw,hh);
                ofDrawLine(vmx+dst*cw,hh,vmx+10+dst*cw,hh);
                ofDrawLine(vmx+5+src*cw,hh,r0,hh+30);
                ofDrawLine(vmx+5+dst*cw,hh,r0,hh+30);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '=': // conditional assignment
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]); // addr of recipient
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]); // value to be assigned
            pc=(pc+1)%MEMLEN;
            arg3=AB.find(M[pc]); // if M[arg3] is non-zero the assignment occurs
            if(arg1!=-1&&arg2!=-1&&arg3!=-1){
                if(AB.find(M[arg3])>0){
                    M[arg1]=AB[arg2];
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'b': // jnz
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&M[arg1]!='0'){
                pc=arg2;
            }else{
                pc=(pc+1)%MEMLEN;
            }
            break;
        case 'B': // unconditional jump
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                pc=arg1;
            }
            break;
        case '+':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                M[pc]=AB[(arg1+arg2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '#':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            ma1=AB.find(M[arg1]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            ma2=AB.find(M[arg2]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                M[pc]=AB[(ma1+ma2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '-':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                M[pc]=arg1<arg2?AB[MEMLEN-(arg2-arg1)]:AB[arg1-arg2];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '_':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                arg1=AB.find(M[arg1]);
                arg2=AB.find(M[arg2]);
                M[pc]=arg1<arg2?AB[MEMLEN-(arg2-arg1)]:AB[arg1-arg2];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '*':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                M[pc]=AB[(arg1*arg2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'X':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            ma1=AB.find(M[arg1]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            ma2=AB.find(M[arg2]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                M[pc]=AB[(ma1*ma2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '%':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1&&arg2!=0){
                M[pc]=AB[(arg1%arg2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '5':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                arg1=AB.find(M[arg1]);
                arg2=AB.find(M[arg2]);
                if(arg2!=0) M[pc]=AB[(arg1%arg2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '/':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1&&arg2!=0){
                M[pc]=AB[(arg1/arg2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '\\':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                arg1=AB.find(M[arg1]);
                arg2=AB.find(M[arg2]);
                if(arg2!=0) M[pc]=AB[(arg1/arg2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'm':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                mgain=ofMap(arg1,0,MEMLEN-1,mglo,mghi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'z':
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg3=M[pc]; // erstwhile zstate
            xpc=pc;
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg3!=-1){
                if(arg3=='0'){
                    if(arg2<arg1){
                        M[pc]=AB[arg2+1];
                        M[xpc]='0';
                    }else if(arg2==arg1){
                        M[pc]=AB[arg2-1];
                        M[xpc]='1';
                    }else{
                        M[pc]=AB[0];
                        M[xpc]=0;
                    }
                }else if(arg3=='1'){
                    if(arg2==0){
                        M[pc]=AB[1];
                        M[xpc]='0';
                    }else if(arg2<arg1){
                        M[pc]=AB[arg2-1];
                        M[xpc]='1';
                    }else{
                        M[pc]=AB[arg1-1];
                        M[xpc]='1';
                    }
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'r': // rndgen
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                M[pc]=AB[round(ofRandom(arg1,arg2))];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'e': // TODO: implement audio rate envelopes at once!
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,5*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            xpc=pc; // estate
            pc=(pc+1)%MEMLEN;
            arg4=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg4!=-1){
                if(M[xpc]=='0'){
                    if((MEMLEN-arg4)<arg1){
                        M[pc]=AB[MEMLEN-1];
                        M[xpc]='1';
                    }else{
                        M[pc]=AB[arg4+arg1];
                        M[xpc]='0';
                    }
                }else if(M[xpc]=='1'){
                    if(arg4<arg2){
                        M[pc]='0';
                        M[xpc]='2';
                    }else{
                        M[pc]=AB[arg4-arg2];
                        M[xpc]='1';
                    }
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 't': // binarizer/thresholder
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]); // addr of the input
            pc=(pc+1)%MEMLEN;
            xpc=pc; // the location of the fixed output constant
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1){
                arg3=AB.find(M[arg1]); // actual input
                M[pc]=arg3>0?M[xpc]:'0'; // hi/lo binary output
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'h': // harmonix coefficients
            pos=pc;
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                if(arg1%2==0){ // quadra
                    ofSetColor(248,248,248); ofDrawRectangle(vmx+pos*cw,vmy-ch+5,6*cw-3,ch+5);
                    pc=(pc+1)%MEMLEN;
                    arg1=AB.find(M[pc]);
                    pc=(pc+1)%MEMLEN;
                    arg2=AB.find(M[pc]); // k2 = <arg1>.<arg2>
                    pc=(pc+1)%MEMLEN;
                    arg3=AB.find(M[pc]);
                    pc=(pc+1)%MEMLEN;
                    arg4=AB.find(M[pc]); // k1 = <arg3>.<arg4>
                    if(arg1!=-1&&arg2!=-1&&arg3!=-1&&arg4!=-1){
                        // k2=4 gives freq = 63^2 * 4 + 63*94 = 21798
                        // which is already more than the limit
                        cvargs[0]=ofMap(arg1,0,MEMLEN,0.,4.) + (float)arg2/(float)MEMLEN;
                        cvargs[1]=(float)arg3 + (float)arg4/(float)MEMLEN;
                        hxq->command(2,2,cvargs); // quadratic coefficients
                    }
                }else{
                    ofSetColor(248,248,248); ofDrawRectangle(vmx+pos*cw,vmy-ch+5,(2+NCOEFF)*cw-3,ch+5);
                    for(int i=0;i<NCOEFF;i++){
                        pc=(pc+1)%MEMLEN;
                        arg1=AB.find(M[pc]);
                        if(arg1!=-1){
                            cvargs[0]=i;
                            cvargs[1]=arg1;
                            hxa->command(2,2,cvargs); // setrot(i,arg1)
                        }
                    }
                    hxa->command(4,0,cvargs); // sethxfreqgain()
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'w': // harmonix parametric array
            pos=pc;
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                if(arg1%2==0){ // [quadra] K <g0> <g1> <g2>
                    ofSetColor(248,248,248); ofDrawRectangle(vmx+pos*cw,vmy-ch+5,7*cw-3,ch+5);
                    pc=(pc+1)%MEMLEN;
                    arg1=AB.find(M[pc]);
                    pc=(pc+1)%MEMLEN;
                    arg2=AB.find(M[pc]); // g0 = <arg1>.<arg2>
                    pc=(pc+1)%MEMLEN;
                    arg3=AB.find(M[pc]); // g1 = <arg3>
                    pc=(pc+1)%MEMLEN;
                    arg4=AB.find(M[pc]);
                    pc=(pc+1)%MEMLEN;
                    arg5=AB.find(M[pc]); // g2 = <arg4>.<arg5>
                    if(arg1!=-1&&arg2!=-1&&arg3!=-1&&arg4!=-1&&arg5!=-1){
                        cvargs[0]=(float)arg1 + (float)arg2/(float)MEMLEN;
                        cvargs[1]=arg3;
                        cvargs[2]=(float)arg4 + (float)arg5/(float)MEMLEN;
                        hxq->command(3,3,cvargs); // exp sin coefficients
                    }
                }else{ // [acobuf] harmonix vol/weight array: NCOEFF params
                    ofSetColor(248,248,248); ofDrawRectangle(vmx+pos*cw,vmy-ch+5,(2+NCOEFF)*cw-3,ch+5);
                    for(int i=0;i<NCOEFF;i++){
                        pc=(pc+1)%MEMLEN;
                        arg1=AB.find(M[pc]);
                        if(arg1!=-1){
                            cvargs[0]=i;
                            cvargs[1]=arg1;
                            hxa->command(3,2,cvargs); // setvol(i,arg1)
                        }
                    }
                    hxa->command(4,0,cvargs); // sethxfreqgain()
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'I': // harmonix main gain
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                hxgain=ofMap(arg1,0,MEMLEN,0.,hxgainlim);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'i': // gains
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1){
                cvargs[0]=ofMap(arg2,0,MEMLEN,0.,1.);
                if(arg1%2==0){ // [quadra] gain
                    hxq->command(1,1,cvargs);
                }else{ // [acobuf] gain
                    hxa->command(1,1,cvargs);
                }
                // space for more additive sub-engines
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'p': // note frequency: p <notenum>
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,3*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1){
                cvargs[0]=idx2freq(arg2,roothx);
                if(arg1%2==0){
                    hxq->command(0,1,cvargs); // quadra frequency
                }else{
                    hxa->command(0,1,cvargs); // acobuf frequency
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'D': // overdrive
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,2*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1){
                cvargs[0]=ofMap(arg2,0,MEMLEN,1.,OVERDRIVE_LIMIT);
                if(arg1%2==0){
                    hxq->command(5,1,cvargs); // quadra
                }else{
                    hxa->command(5,1,cvargs); // acobuf
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 's': // stereo channel mix coefficients
            ofSetColor(248,248,248); ofDrawRectangle(vmx+pc*cw,vmy-ch+5,4*cw-3,ch+5);
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg3=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg3!=-1){
                cvargs[0]=(float)arg2/(float)MEMLEN;
                cvargs[1]=(float)arg3/(float)MEMLEN;
                if(arg1%2==0){
                    hxq->command(6,2,cvargs); // passing stereo kL, kR to quadra
                }else{
                    hxa->command(6,2,cvargs); // passing stereo kL, kR to acobuf
                }
            }
            pc=(pc+1)%MEMLEN;
            break;
        default:
            pc=(pc+1)%MEMLEN;
    }
}

void ofApp::initsynth(){
    // additive buffer coefficient synthesis
    hxa=new acobuf(NHARM,111.,0.);
    hxq=new quadra(NHARM,111.,0.);
    roothx=52.;
    //
    rootflo=40.0;
    rootfhi=80.0;
    tunlo=-7.0;
    tunhi=7.0;
    activenote=0;
    notevelo=0;
    mglo=0.00;
    mghi=0.95;
    cout<<"[initsynth] master gain limit = "<<mghi<<endl;
    hxgainlim=0.94;
    hxgain=0.66*hxgainlim;
    cout<<"[initsynth] harmonix gain limit = "<<hxgainlim<<endl;
}

void ofApp::initcam(){
    camw=1280;
    camh=720;
    vdo.setVerbose(true);
    vdo.setup(camw,camh);
    plane.set(ofGetWidth(),ofGetHeight());
    plane.mapTexCoords(0,0,ofGetWidth(),ofGetHeight());
    camfnt.load("OCRA",8);
    asciiChars=string("    ,./:;`~'\"+=()[]%%{}**");
    t=0.0;
    dt=0.01;
}

void ofApp::initshdr(){
    ofDisableArbTex();
    if(ofIsGLProgrammableRenderer()){
        shdr[0].load("gl3/hypnorgb");
        shdr[1].load("gl3/lips");
        shdr[2].load("gl3/quantum");
        shdr[3].load("gl3/ringhypnosystem");
        shdr[4].load("gl3/trigonomagnetism");
        shdr[5].load("gl3/vdofx");
        shdr[6].load("gl3/madkam");
        shdr[7].load("gl3/tmp");
        shdr[8].load("gl3/dimensionwarp");
    }else{
        shdr[0].load("gl2/hypnorgb");
        shdr[1].load("gl2/lips");
        shdr[2].load("gl2/quantum");
        shdr[3].load("gl2/ringhypnosystem");
        shdr[4].load("gl2/trigonomagnetism");
        shdr[5].load("gl2/vdofx");
        shdr[6].load("gl2/madkam");
        shdr[7].load("gl2/tmp");
        shdr[8].load("gl2/dimensionwarp");
    }
    curshdr=0;
}

void ofApp::rndrcam(ofPixelsRef & pixelsRef){
    vdo.getTexture().bind();
    int curshdrcopy=curshdr;
    shdr[curshdrcopy].begin();
    //
    shdr[curshdrcopy].setUniform1f("u_time", t);
    shdr[curshdrcopy].setUniform2f("u_res", ofGetWidth(), ofGetHeight());
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2,ofGetHeight()/2);
    ofRotateRad(PI);
    plane.draw();
    ofPopMatrix();
    //
    shdr[curshdrcopy].end();
    vdo.getTexture().unbind();
}

void ofApp::setup(){
    ofSetWindowTitle("amUrtayantra"); // amUrtayantra it is! although fullscreen ensures you'll never see the name
    ofSetVerticalSync(true);
    ofBackground(0,0,0);

    // use only one kbd at a time!
    setupyc();    // yamaha reface yc
    //setupdm();    // behringer deepmind 12
    setupsndsys();
    initvm();
    initsynth();
    initlogo();
    initshdr();
    initcam();

    fnt.load("OCRA",14,true,true);
}

//--------------------------------------------------------------
void ofApp::update(){
    // lock the buffer
    unique_lock<mutex> lock(audioMutex);

	waveform.clear();
	for(size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
        // graphing
		float sample = lastBuffer.getSample(i, 0);
		float x = ofMap(i, 0, lastBuffer.getNumFrames(), 0, ofGetWidth());
		float y = ofMap(sample, -1, 1, 0, ofGetHeight());
		waveform.addVertex(x, y);
	}
	rms = lastBuffer.getRMSAmplitude();

    // cam update
    vdo.update();
    // the passage of time
    t+=dt;
}

//--------------------------------------------------------------
float ofApp::idx2freq(int note,float basefreq) {
    int octave = note / 12;
    int key = note % 12;
    return basefreq * pow(2.,(float)octave) * tuning[key] + finetuning[key];
}

void ofApp::audioOut(ofSoundBuffer &outBuffer) {
	int sr = outBuffer.getSampleRate();
	for(size_t i = 0; i < outBuffer.getNumFrames(); i++) {
		// waveshaping
        float eg = mgain * hxgain;
        float lmix = eg * (hxa->lmix(i) + hxq->lmix(i));
        float rmix = eg * (hxa->rmix(i) + hxq->rmix(i));
		// guards/limits
		if(lmix>0.99){
            lmix=mghi;
        }else if(lmix<-0.99){
            lmix=-mghi;
        }
        if(rmix>0.99){
            rmix=mghi;
        }else if(rmix<-0.99){
            rmix=-mghi;
        }
		// write out
		outBuffer.getSample(i, 0) = lmix;
		outBuffer.getSample(i, 1) = rmix;
        // harmonic engines update
        hxa->update(sr);
        hxq->update(sr);
	}

	unique_lock<mutex> lock(audioMutex);
	lastBuffer = outBuffer;
}

//--------------------------------------------------------------
void ofApp::exit(){
    close(kbfd);
}

void ofApp::rndrmem(float y){
    ofSetColor(0,230,0);
    char q[2];
    float d=6;
    float cw=18.,ch=20.;
    vmx=(ofGetWidth()-MEMLEN*cw)/2.;
    float x=vmx;
    for(int i=0;i<MEMLEN;i++){
        if(ec==i){
            ofSetColor(240,160,0);
            ofDrawLine(x+i*cw,y+ch,x+i*cw+d,y+ch-d);
            ofDrawLine(x+i*cw+d,y+ch-d,x+i*cw+2*d,y+ch);
        }
        if(pc==i){
            ofSetColor(0,220,220);
            ofDrawLine(x+i*cw,y-ch-2,x+i*cw+d,y-ch-2+d);
            ofDrawLine(x+i*cw+d,y-ch-2+d,x+i*cw+2*d,y-ch-2);
        }
        ofSetColor(0,240,0);
        sprintf(q,"%c",M[i]);
        fnt.drawString(q,x+i*cw,y+3);
        ofSetColor(144,144,144);
        sprintf(q,"%c",AB[i]);
        fnt.drawString(q,x+i*cw,y+3+ch+12);
    }
}

void ofApp::rndrkb(float y){
    int d=0;
    float w=20,h=200;
    float x=(ofGetWidth()-w*(kblen-1))/2;
    ofNoFill();
    for(int i=0;i<kblen-1;i++,d++){
        float v=ofMap(kb[i],0,127,0.,h);
        ofDrawLine(x+d*w,y+h-v,x+(d+1)*w,y+h-v);
    }
}

void ofApp::kbsweep(){
    int somenoteon=0;
    for(int i=0;i<kblen;i++){
        if(kb[i]>0){
            activenote=i;
            notevelo=kb[i];
            somenoteon=1;
            break;
        }
    }
    if(!somenoteon){
        notevelo=0;
    }
}

void ofApp::trtlwalk(){
    for(int i=0;i<NOSCS;i++){
        tx[i]->rndr();
        tx[i]->sens(tx[(i+1)%NOSCS]);
    }
    for(int i=0;i<NOSCS;i++){
        tx[i]->walk();
    }
}

void ofApp::rndrhxser(hxser * H,float x,float y,float w,float h){
    // render harmonix
    ofSetColor(23,202,232);
    float gk=3000.;
    for(int i=0;i<NHARM;i++){
        syn * hrm=H->hx[i];
        float dx=ofMap(hrm->freq,0.,20000.,0.,w);
        float gy=gk*hrm->gain;
        ofDrawLine(x+dx,y+h-gy,x+dx,y+h);
    }
}

void ofApp::f5(float x,float y,int times,ofPixelsRef & pixelsRef){
    // shadercam
    rndrcam(pixelsRef);
    for(int i=0;i<times;i++){
        ofSetColor(0,240,0);
        // monophonic key sweeping
        kbsweep();
        // rndr
        rndrkb(ofGetHeight()-200-30);
        rndrmem(y);
        // logo
        trtlwalk();
        // oscope
        ofSetColor(0,240,0);
        ofSetLineWidth(1 + (rms * 30.));
        waveform.draw();
        // harmonic rendering
        rndrhxser(hxq,50.,80.,800.,300.);
        rndrhxser(hxa,0.5*ofGetWidth(),80.,800.,300.);
        // cycling the vm
        cyclevm();
    }
}

void ofApp::loadprogram(int pid){
    if(pid<0){
        cout<<"program #"<<pid<<" unavailable!\n";
        return;
    }
    for(int i=0;i<MEMLEN;i++){
        M[i]=prgms[pid%NPRGMS][i];
    }
    ec=0;
    pc=0;
}

void ofApp::loadshader(int pid){
    if(pid<0){
        cout<<"shader #"<<pid<<" unavailable!\n";
        return;
    }
    // shader select
    curshdr=pid%NGLSL;
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPixelsRef pixelsRef = vdo.getPixels();
    f5(vmx,vmy,f5times,pixelsRef);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    int tmp;
    if(DEBUG){
        cout<<key<<endl;
    }
    if(key==1)    return;
    if(key==2)    return;
    if(key==4)    return;
    if(key==3680) { // lshift
        lsdown=1;
        return;
    }
    if(key==3681) return; // rshift
    if(key==3682) { // lctrl
        lcdown=1;
        return;
    }
    if(key==3683) return; // rctrl
    if(key==3684) { // lalt
        ladown=1;
        return;
    }
    if(key==3685) return; // ralt
    if(key==13){ // enter: dump the tape
        for(int i=0;i<MEMLEN;i++){
            cout<<M[i];
        }
        cout<<endl;
        return;
    }
    if(key==57356){ // larrow
        if(lcdown){
            tmp=ec-lcjmp;
            ec=tmp<0?0:tmp;
        }else{
            ec=ec==0?MEMLEN-1:ec-1;
        }
        return;
    }
    if(key==57358){ // rarrow
        if(lcdown){
            ec=(ec+lcjmp)%MEMLEN;
        }else{
            ec=(ec+1)%MEMLEN;
        }
        return;
    }
    if(key==57357){ // uarrow
        if(lsdown){
            vmy-=5.;
        }else{
            int pos=AB.find(M[ec]);
            if(pos!=-1){
                M[ec]=AB[(pos+1)%MEMLEN];
            }
        }
        return;
    }
    if(key==57359){ // darrow
        if(lsdown){
            vmy+=5.;
        }else{
            int pos=AB.find(M[ec]);
            if(pos!=-1){
                M[ec]=AB[pos==0?MEMLEN-1:pos-1];
            }
        }
        return;
    }
    if(key==57362){ // home
        ec=0;
        return;
    }
    if(key==57363){ // end
        ec=MEMLEN-1;
        return;
    }
    if(key==8){
        ec=ec==0?MEMLEN-1:ec-1;
        M[ec]='.';
        return;
    }
    if(key==127){ // !clear tape!
        for(int i=0;i<MEMLEN;i++){
            M[i]='.';
        }
        ec=0;
        pc=0;
        return;
    }
    // program (patchgram) memory [0-9] check prgms str arr
    if(key>=48&&key<=57&&ladown){
        loadprogram(key-48);
        return;
    }
    // extended program memory [a-z] :D
    if(key>=97&&key<=122&&ladown){
        loadprogram(key-97+10);
        return;
    }
    // shader memory [0-9] check glsl
    if(key>=48&&key<=57&&lcdown){
        loadshader(key-48);
        return;
    }
    // extended shader memory [a-z] :D
    if(key>=97&&key<=122&&lcdown){
        loadshader(key-97+10);
        return;
    }
    if(key==114&&ladown){ // alt+r -> randomly constituted program
        for(int i=0;i<MEMLEN;i++){
            int idx=(int)ofRandom(0,1024);
            M[i]=AB[idx%MEMLEN];
        }
        return;
    }
    // time reset
    if(key==32&&lcdown){ // ctrl+space
        t=0.0;
        return;
    }
    M[ec]=key;
    ec=(ec+1)%MEMLEN;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key==3682){ // lctrl
        lcdown=0;
        return;
    }
    if(key==3680){ // lshift
        lsdown=0;
        return;
    }
    if(key==3684){ // lalt
        ladown=0;
        return;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
