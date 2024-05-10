#include "ofApp.h"

void ofApp::setupyc(){
    ycfd=shm_open(reface, O_RDWR, 0644);
    fprintf(stderr, "[shm] ycfd: %d\n", ycfd);
    if(ycfd>0){
        struct stat sby;
        fstat(ycfd,&sby);
        yclen=sby.st_size;
        yc=(u_char *)mmap(NULL, yclen, PROT_READ|PROT_WRITE, MAP_SHARED, ycfd, 0);
        fprintf(stderr, "[shm] addr: %p [0..%lu]\n", yc, yclen-1);
        if(yc){
            cout<<"[yc] all in order.\n";
        }else{
            cout<<"[error] the end is here!\n";
        }
    }
}

void ofApp::setupsndsys(){
    ofSoundStreamSettings settings;
	settings.numOutputChannels = 2;
	settings.sampleRate = 44100;
	settings.bufferSize = 512;
	settings.numBuffers = 4;
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
    vmy=40.;
}

void ofApp::color12(int c){
    int b=ofMap(c&0xf,0,0xf,0,255);
    int g=ofMap((c&0xf0)>>4,0,0xf,0,255);
    int r=ofMap((c&0xf00)>>8,0,0xf,0,255);
    ofSetColor(r,g,b);
}

void ofApp::cyclevm(){
    char code=M[pc];
    int pos,src,dst,xpc,arg1,arg2,arg3,arg4;
    int ma1,ma2;
    float hh,r0;
    switch(code){
        case '.':
            pc=0;
            break;
        case 'n':
            pc=(pc+1)%MEMLEN;
            M[pc]=AB[activenote];
            pc=(pc+1)%MEMLEN;
            break;
        case 'g':
            pc=(pc+1)%MEMLEN;
            M[pc]=AB[floor(ofMap(notevelo,0,127,0,MEMLEN))];
            pc=(pc+1)%MEMLEN;
            break;
        case '^': // ramp up wraparound
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                M[pc]=AB[(pos+1)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'A': // ramp up till limit
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            M[pc]=AB[arg2>=arg1?0:arg2+1];
            pc=(pc+1)%MEMLEN;
            break;
        case 'v': // ramp down wraparound
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                M[pc]=AB[pos==0?MEMLEN-1:pos-1];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'V': // ramp down from limit
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            M[pc]=AB[arg2==0||arg2>arg1?arg1:arg2-1];
            pc=(pc+1)%MEMLEN;
            break;
        case 'L': // delay loop
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
        case '1':
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                rootf1=ofMap(pos,0,MEMLEN-1,rootflo,rootfhi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '2':
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                rootf2=ofMap(pos,0,MEMLEN-1,rootflo,rootfhi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'f':
            pc=(pc+1)%MEMLEN;
            pos=AB.find(M[pc]);
            if(pos!=-1){
                f5times=pos; // invoking with 0 causes system hang :D
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'c': // cp OR patch/connect
            pc=(pc+1)%MEMLEN;
            src=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            dst=AB.find(M[pc]);
            if(src!=-1&&dst!=-1){
                M[dst]=M[src];
                color12(cablehues[pc%NHUES]);
                hh=ofMap(pc,0,MEMLEN-1,vmy+36.,vmy+69.);
                ofDrawLine(vmx+src*16,hh,vmx+10+src*16,hh);
                ofDrawLine(vmx+dst*16,hh,vmx+10+dst*16,hh);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '=': // conditional assignment
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
        case '+':
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
        case '*':
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
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                M[pc]=AB[(arg1%arg2)%MEMLEN];
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'i':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                gain1=ofMap(arg1,0,MEMLEN-1,gainlo,gainhi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'j':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                gain2=ofMap(arg1,0,MEMLEN-1,gainlo,gainhi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '/':
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
        case 'm':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                mgain=ofMap(arg1,0,MEMLEN-1,mglo,mghi);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'p':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                w1[w1typ]->command(0,idx2freq(arg1,rootf1));
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'q':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                w2[w2typ]->command(0,idx2freq(arg1,rootf2));
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '3':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                w1typ=arg1%NWAVS;
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '4':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                w2typ=arg1%NWAVS;
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'z':
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
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]); // addr of the input
            pc=(pc+1)%MEMLEN;
            xpc=pc; // the location of the fixed output constant
            pc=(pc+1)%MEMLEN;
            if(arg1!=-1&&arg2!=-1){
                arg3=AB.find(M[arg1]); // actual input
                M[pc]=arg3>0?M[xpc]:'0'; // hi/lo binary output
            }
            pc=(pc+1)%MEMLEN;
            break;
        case 'x':
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg3=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg3!=-1){
                modmat[arg1%MMROWS][arg2%MMCOLS]=arg3>0;
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '5': // lfo1 <freq:bichar> <gain:char>
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg3=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg3!=-1){
                r0=(float)arg1+ofMap(arg2,0,MEMLEN,0.,1.);
                lfo1[lfo1typ]->command(0,r0);
                lfo1[lfo1typ]->command(1,arg3);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '6': // lfo2 <freq> <gain>
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg2=AB.find(M[pc]);
            pc=(pc+1)%MEMLEN;
            arg3=AB.find(M[pc]);
            if(arg1!=-1&&arg2!=-1&&arg3!=-1){
                r0=(float)arg1+ofMap(arg2,0,MEMLEN,0.,1.);
                lfo2[lfo2typ]->command(0,r0);
                lfo2[lfo2typ]->command(1,arg3);
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '7': // lfo1 <type>
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                lfo1typ=arg1%NLFOWAVS;
            }
            pc=(pc+1)%MEMLEN;
            break;
        case '8': // lfo2 <type>
            pc=(pc+1)%MEMLEN;
            arg1=AB.find(M[pc]);
            if(arg1!=-1){
                lfo2typ=arg1%NLFOWAVS;
            }
            pc=(pc+1)%MEMLEN;
            break;
        default:
            pc=(pc+1)%MEMLEN;
    }
}

void ofApp::initsynth(){
    // audio wavs
    w1[0] = new tri(110, 0.4);
	w2[0] = new tri(111, 0.4);
    w1[1] = new squ(110, 0.4);
	w2[1] = new squ(111, 0.4);
    w1[2] = new rsaw(110, 0.4);
	w2[2] = new rsaw(111, 0.4);
    w1[3] = new syn(110, 0.4);
	w2[3] = new syn(111, 0.4);
    w1[4] = new noyz(110, 0.1);
	w2[4] = new noyz(110, 0.1);
    w1typ = 0;
    w2typ = 0;
    // lfo wavs
    lfo1[0] = new lfosyn(2, 10);
	lfo2[0] = new lfosyn(2, 10);
    lfo1[1] = new lfosqu(1, 10);
	lfo2[1] = new lfosqu(1, 10);
    lfo1typ = 0;
    lfo2typ = 0;
    // init lfo scopes
    lfobufsz=84;
    lfoctr=0;
    for(int i=0;i<lfobufsz;i++){
        lfo1scope.push_back(0.);
        lfo2scope.push_back(0.);
    }
    //
    rootf1 = 52.0;
    rootf2 = 52.0;
    rootflo=40.0;
    rootfhi=80.0;
    activenote=0;
    notevelo=0;
    // osc gains
    gainhi=0.95;
    gainlo=0.00;
    gain1=0.4;
    gain2=0.4;
    mgain=0.0;
    mglo=0.00;
    mghi=0.20;
}

void ofApp::setup(){
    ofSetWindowTitle("amUrtayantra"); // amUrtayantra it is!
    ofSetVerticalSync(true);
    ofBackground(0,0,0);

    setupyc();
    setupsndsys();
    initvm();
    initsynth();

    fnt.load("OCRA",10,true,true);
}

//--------------------------------------------------------------
void ofApp::update(){
    // "lastBuffer" is shared between update() and audioOut(), which are called
	// on two different threads. This lock makes sure we don't use lastBuffer
	// from both threads simultaneously (see the corresponding lock in audioOut())
	unique_lock<mutex> lock(audioMutex);

	// the x coordinates are evenly spaced on a grid from 0 to the window's width
	// the y coordinates map each audio sample's range (-1 to 1) to the height
	// of the window

	waveform.clear();
	for(size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
		float sample = lastBuffer.getSample(i, 0);
		float x = ofMap(i, 0, lastBuffer.getNumFrames(), 0, ofGetWidth());
		float y = ofMap(sample, -1, 1, 0, ofGetHeight());
		waveform.addVertex(x, y);
	}

	rms = lastBuffer.getRMSAmplitude();

    // record lfo variation
    lfo1scope.push_back(lfo1[lfo1typ]->y);
    lfo1scope.erase(lfo1scope.begin());
    lfo2scope.push_back(lfo2[lfo2typ]->y);
    lfo2scope.erase(lfo2scope.begin());
}

//--------------------------------------------------------------
float ofApp::idx2freq(int note,float basefreq) {
    int octave = note / 12;
    int key = note % 12;
    return basefreq * pow(2.,(float)octave) * tuning[key];
}

//--------------------------------------------------------------
void ofApp::modgain(bool mmij, lfo * oo, phasor * ww){
    if(mmij){
        ww->modulate(1, ofMap(oo->y,-oo->ampref,oo->ampref,0.,1.));
    }
}

void ofApp::modgain(bool mmij, int arg, phasor * ww){
    if(mmij){
        ww->modulate(1, (float)arg/(float)MEMLEN);
    }
}

void ofApp::xmod(){
    int mmi=mmctr/MMCOLS;
    int mmj=mmctr%MMCOLS;
    if(mmi==0){
        if(mmj==1){
            lfo2[lfo2typ]->modulate(0, modmat[mmi][mmj] ? lfo1[lfo1typ]->y : 0); // L1 mod L2f
        }else if(mmj==3){
            lfo2[lfo2typ]->modulate(1, modmat[mmi][mmj] ? lfo1[lfo1typ]->y : 0); // L1 mod L2g
        }else if(mmj==4){
            w1[w1typ]->modulate(0, modmat[mmi][mmj] ? lfo1[lfo1typ]->y : 0); // L1 mod w1f
        }else if(mmj==5){
            w2[w2typ]->modulate(0, modmat[mmi][mmj] ? lfo1[lfo1typ]->y : 0); // L1 mod w2f
        }else if(mmj==6){
            modgain(modmat[mmi][mmj], lfo1[lfo1typ], w1[w1typ]); // L1 mod w1g
        }else if(mmj==7){
            modgain(modmat[mmi][mmj], lfo1[lfo1typ], w2[w2typ]); // L1 mod w2g
        }
    }else if(mmi==1){
        if(mmj==0){
            lfo1[lfo1typ]->modulate(0, modmat[mmi][mmj] ? lfo2[lfo2typ]->y : 0); // L2 mod L1f
        }else if(mmj==2){
            lfo1[lfo1typ]->modulate(1, modmat[mmi][mmj] ? lfo2[lfo2typ]->y : 0); // L2 mod L1g
        }else if(mmj==4){
            w1[w1typ]->modulate(0, modmat[mmi][mmj] ? lfo2[lfo2typ]->y : 0); // L2 mod w1f
        }else if(mmj==5){
            w2[w2typ]->modulate(0, modmat[mmi][mmj] ? lfo2[lfo2typ]->y : 0); // L2 mod w2f
        }else if(mmj==6){
            modgain(modmat[mmi][mmj], lfo2[lfo2typ], w1[w1typ]); // L2 mod w1g
        }else if(mmj==7){
            modgain(modmat[mmi][mmj], lfo2[lfo2typ], w2[w2typ]); // L2 mod w2g
        }
    }else if(mmi==2){
        if(mmj==0){
            lfo1[lfo1typ]->modulate(0, modmat[mmi][mmj] ? activenote : 0); // nn mod L1f
        }else if(mmj==1){
            lfo2[lfo2typ]->modulate(0, modmat[mmi][mmj] ? activenote : 0); // nn mod L2f
        }else if(mmj==2){
            lfo1[lfo1typ]->modulate(1, modmat[mmi][mmj] ? activenote : 0); // nn mod L1g
        }else if(mmj==3){
            lfo2[lfo2typ]->modulate(1, modmat[mmi][mmj] ? activenote : 0); // nn mod L2g
        }else if(mmj==4){
            w1[w1typ]->modulate(0, modmat[mmi][mmj] ? activenote : 0); // nn mod w1f
        }else if(mmj==5){
            w2[w2typ]->modulate(0, modmat[mmi][mmj] ? activenote : 0); // nn mod w2f
        }else if(mmj==6){
            modgain(modmat[mmi][mmj], activenote, w1[w1typ]); // nn mod w1g
        }else if(mmj==7){
            modgain(modmat[mmi][mmj], activenote, w2[w2typ]); // nn mod w2g
        }
    }else if(mmi==3){
        if(mmj==0){
            lfo1[lfo1typ]->modulate(0, modmat[mmi][mmj] ? notevelo : 0); // nv mod L1f
        }else if(mmj==1){
            lfo2[lfo2typ]->modulate(0, modmat[mmi][mmj] ? notevelo : 0); // nv mod L2f
        }else if(mmj==2){
            lfo1[lfo1typ]->modulate(1, modmat[mmi][mmj] ? notevelo : 0); // nv mod L1g
        }else if(mmj==3){
            lfo2[lfo2typ]->modulate(1, modmat[mmi][mmj] ? notevelo : 0); // nv mod L2g
        }else if(mmj==4){
            w1[w1typ]->modulate(0, modmat[mmi][mmj] ? notevelo : 0); // nv mod w1f
        }else if(mmj==5){
            w2[w2typ]->modulate(0, modmat[mmi][mmj] ? notevelo : 0); // nv mod w2f
        }else if(mmj==6){
            modgain(modmat[mmi][mmj], notevelo, w1[w1typ]); // nv mod w1g
        }else if(mmj==7){
            modgain(modmat[mmi][mmj], notevelo, w2[w2typ]); // nv mod w2g
        }
    }
    mmctr=(mmctr+1)%(MMROWS*MMCOLS);
}

void ofApp::audioOut(ofSoundBuffer &outBuffer) {
	int sr = outBuffer.getSampleRate();
	for(size_t i = 0; i < outBuffer.getNumFrames(); i++) {
		// waveshaping
		float d = gain1 * w1[w1typ]->y;
		float e = gain2 * w2[w2typ]->y;
		float outl = mgain * d;
        float outr = mgain * e;
		// write out
		outBuffer.getSample(i, 0) = outl+outr;
		outBuffer.getSample(i, 1) = outl+outr;
        // modulation
        xmod(); // here or in draw?
		// updation
		w1[w1typ]->update(sr);
		w2[w2typ]->update(sr);
        lfo1[lfo1typ]->update(sr);
        lfo2[lfo2typ]->update(sr);
	}

	unique_lock<mutex> lock(audioMutex);
	lastBuffer = outBuffer;
}

//--------------------------------------------------------------
void ofApp::exit(){
    close(ycfd);
}

void ofApp::rndrmem(float y){
    ofSetColor(0,230,0);
    char q[2];
    float d=6;
    float cw=16,ch=18;
    vmx=(ofGetWidth()-MEMLEN*cw)/2.;
    float x=vmx;
    for(int i=0;i<MEMLEN;i++){
        if(ec==i){
            ofSetColor(180,120,0);
            ofDrawLine(x+i*cw,y+ch,x+i*cw+d,y+ch-d);
            ofDrawLine(x+i*cw+d,y+ch-d,x+i*cw+2*d,y+ch);
        }
        if(pc==i){
            ofSetColor(0,120,120);
            ofDrawLine(x+i*cw,y-ch-2,x+i*cw+d,y-ch-2+d);
            ofDrawLine(x+i*cw+d,y-ch-2+d,x+i*cw+2*d,y-ch-2);
        }
        ofSetColor(0,240,0);
        sprintf(q,"%c",M[i]);
        fnt.drawString(q,x+i*cw,y+3);
        ofSetColor(70,70,70);
        sprintf(q,"%c",AB[i]);
        fnt.drawString(q,x+i*cw,y+3+ch+12);
    }
}

void ofApp::rndryc(float y){
    int d=0;
    float w=20,h=200;
    float x=(ofGetWidth()-w*(yclen-1))/2;
    ofNoFill();
    for(int i=0;i<yclen-1;i++,d++){
        float v=ofMap(yc[i],0,127,0.,h);
        ofDrawLine(x+d*w,y+h-v,x+(d+1)*w,y+h-v);
    }
}

void ofApp::rndrlfos(float x,float y){
    char q[32];
    float xgap=256.;
    float ch=18.;
    float scopeh=64.,ygap=27.;

    // log it
    fnt.drawString("--- LFO1 ---",x,y);
    sprintf(q,"f=%05.2f, g=%.2f",lfo1[lfo1typ]->freq,lfo1[lfo1typ]->amp);
    fnt.drawString(q,x,y+ch);
    sprintf(q,"          -%.2f",lfo1[lfo1typ]->amp);
    fnt.drawString(q,x,y+ch+scopeh+ygap);
    fnt.drawString("--- LFO2 ---",x+xgap,y);
    sprintf(q,"f=%05.2f, g=%.2f",lfo2[lfo2typ]->freq,lfo2[lfo2typ]->amp);
    fnt.drawString(q,x+xgap,y+ch);
    sprintf(q,"          -%.2f",lfo2[lfo2typ]->amp);
    fnt.drawString(q,x+xgap,y+ch+scopeh+ygap);

    // scopes
    ofSetColor(23*0.66,202*0.66,232*0.66);
    float l1mx=lfo1[lfo1typ]->ampref;
    float l2mx=lfo2[lfo2typ]->ampref;
    float k=2.0;
    for(int i=0;i<lfobufsz;i++){
        float h1=ofMap(lfo1scope[i],-l1mx,l1mx,-scopeh/2,scopeh/2);
        float h2=ofMap(lfo2scope[i],-l2mx,l2mx,-scopeh/2,scopeh/2);
        ofDrawLine(x+k*i,y+ygap+scopeh/2,x+k*i,y+ygap+scopeh/2+h1);
        ofDrawLine(x+xgap+k*i,y+ygap+scopeh/2,x+xgap+k*i,y+ygap+scopeh/2+h2);
    }
}

void ofApp::rndrmodmat(float x,float y){
    ofSetColor(23,202,232);
    float cw=16,ch=18;
    char q[2];
    fnt.drawString("src",x,y+ch);
    fnt.drawString("dst",x+cw,y);
    ofDrawLine(x,y,x+cw*3,y+ch*1);
    for(int i=0;i<MMROWS;i++){
        for(int j=0;j<MMCOLS;j++){
            sprintf(q,"%d",modmat[i][j]);
            fnt.drawString(q,x+(j+2)*cw*2,y+(i+2)*ch);
        }
    }
    // row desc
    fnt.drawString("L1 0",x,y+ch*2);
    fnt.drawString("L2 1",x,y+ch*3);
    fnt.drawString("NN 2",x,y+ch*4);
    fnt.drawString("NV 3",x,y+ch*5);
    ofDrawLine(x+cw*3,y+ch,x+cw*3,y+ch*5+5);
    // col desc
    fnt.drawString("0",x+cw*4,y+ch);
    fnt.drawString("1",x+cw*6,y+ch);
    fnt.drawString("2",x+cw*8,y+ch);
    fnt.drawString("3",x+cw*10,y+ch);
    fnt.drawString("4",x+cw*12,y+ch);
    fnt.drawString("5",x+cw*14,y+ch);
    fnt.drawString("6",x+cw*16,y+ch);
    fnt.drawString("7",x+cw*18,y+ch);
    //
    fnt.drawString("L1f",x+cw*4,y);
    fnt.drawString("L2f",x+cw*6,y);
    fnt.drawString("L1g",x+cw*8,y);
    fnt.drawString("L2g",x+cw*10,y);
    fnt.drawString("w1f",x+cw*12,y);
    fnt.drawString("w2f",x+cw*14,y);
    fnt.drawString("w1g",x+cw*16,y);
    fnt.drawString("w2g",x+cw*18,y);
    ofDrawLine(x+cw*3,y+ch+3,x+cw*19,y+ch+3);
}

void ofApp::kbsweep(){
    int somenoteon=0;
    for(int i=0;i<yclen;i++){
        if(yc[i]>0){
            activenote=i;
            notevelo=yc[i];
            somenoteon=1;
            break;
        }
    }
    if(!somenoteon){
        notevelo=0;
    }
}

void ofApp::f5(float x,float y,int times){
    for(int i=0;i<times;i++){
        ofBackground(0,0,0);
        ofSetColor(0,240,0);
        // monophonic key sweeping
        kbsweep();
        // rndr
        rndryc(ofGetHeight()-200-30);
        rndrmem(y);
        rndrmodmat(35,110);
        rndrlfos(440,110);
        ofSetColor(0,240,0);
        ofSetLineWidth(1 + (rms * 30.));
        waveform.draw();
        // cycling the vm
        cyclevm();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    f5(vmx,vmy,f5times);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    cout<<key<<endl;
    int tmp;
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
    if(key==3684) return; // lalt
    if(key==3685) return; // ralt
    if(key==13)   return; // enter
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