#pragma once

#include "ofMain.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MEMLEN 95
#define NHUES 33
#define NWAVS 5
#define NLFOWAVS 2
#define NOSCS 9
#define MMROWS 4
#define MMCOLS 22
#define NPRGMS 27
#define NHARM 64
#define NCOEFF 4

#define SMPLRATE 44100
#define BUFSZ 512
#define NBUF 4
#define NOUTCH 2
#define SPDLIM 4
#define NGLSL 9

class trtl{
public:
	float x,y,a;
	float slen,swid;
	float step,turn;
	float df2,db2;
	float turncoeff,stepcoeff;

	trtl(float xx,float yy,float st,float tn){
		x=xx;
		y=yy;
		a=90.;
		slen=6.;
		swid=3.;
		step=st;
		turn=tn;
		turncoeff=0.011;
		stepcoeff=0.05;
	}

	void fd(float r){
		float rads=a*PI/180.;
		float x2=x+r*cos(rads);
		float y2=y-r*sin(rads);
		if(x2>ofGetWidth()){ x2=x2-ofGetWidth(); }
		if(x2<0){ x2=x2+ofGetWidth(); }
		if(y2>ofGetHeight()){ y2=y2-ofGetHeight(); }
		if(y2<0){ y2=y2+ofGetHeight(); }
		ofDrawLine(x,y,x2,y2);
		x=x2;
		y=y2;
	}

	void bk(float r){ fd(-r); }

	void lt(float phi){ a+=phi; }

	void rt(float phi){ lt(-phi); }

	void sens(trtl * q){
		float k=PI/180.;
		float fx=x+slen*cos(a*k);
		float fy=y-slen*sin(a*k);
		df2=powf(q->x-fx,2.)+powf(q->y-fy,2.);
		db2=powf(q->x-x,2.)+powf(q->y-y,2.);
		// feedback
		float dfb=sqrt(df2)-sqrt(db2);
		turn+=turncoeff*dfb;
		float db=sqrt(db2);
		float tmp=stepcoeff*db;
		step=tmp>SPDLIM?SPDLIM:tmp;
	}

	void walk(){
		fd(step);
		lt(turn);
	}

	void rndr(){
		float k=PI/180.;
		float plx=x+swid/2*cos((a+90)*k);
		float ply=y-swid/2*sin((a+90)*k);
		float prx=x+swid/2*cos((a-90)*k);
		float pry=y-swid/2*sin((a-90)*k);
		float pfx=x+slen*cos(a*k);
		float pfy=y-slen*sin(a*k);
		ofSetColor(23,202,232);
		ofDrawLine(plx,ply,prx,pry);
		ofDrawLine(plx,ply,pfx,pfy);
		ofDrawLine(prx,pry,pfx,pfy);
	}
};

class phasor{
public:
	virtual void update(int sr) = 0;

	virtual void command(int ccode, float cval) = 0;

	virtual void modulate(int mcode, float mval) = 0;

	float freq, gain, phase, phasestep, y;
	float freqref, gainref;
};

class lfo{
public:
	virtual void update(int sr) = 0;

	virtual void command(int ccode, float cval) = 0;

	virtual void modulate(int mcode, float mval) = 0;

	float freq, amp, y;
	float freqref, ampref;
	float t, dt;
};

// lfo phasors
class lfosyn : public lfo {
public:
	lfosyn(float f, float g) {
		freq = f;
		amp = g;
		freqref=f;
		ampref=g;
		y = 0;
		phase=0.0;
	}

	void update(int sr) {
		phasestep=TWO_PI*freq/sr;
		phase+=phasestep;
		y = amp * sin(phase);
		if(phase>TWO_PI) phase-=TWO_PI;
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				freq=cv;
				freqref=freq;
				break;
			case 1:
				amp=cv;
				ampref=amp;
				break;
		}
	}

	void modulate(int mc, float mv){
		switch(mc){
			case 0:
				freq=freqref+mv;
				break;
			case 1:
				amp=ampref+mv;
				break;
		}
	}

	float phase,phasestep;
};

class lfosqu : public lfo {
public:
	lfosqu(float f, float g) {
		freq = f;
		amp = g;
		freqref=f;
		ampref=g;
		y = 0;
		yy=0;
		phase=0.0;
	}

	void update(int sr) {
		phasestep=TWO_PI*freq/sr;
		phase+=phasestep;
		yy = sin(phase);
		y=yy>0?amp:-amp;
		if(phase>TWO_PI) phase-=TWO_PI;
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				freq=cv;
				freqref=freq;
				break;
			case 1:
				amp=cv;
				ampref=amp;
				break;
		}
	}

	void modulate(int mc, float mv){
		switch(mc){
			case 0:
				freq=freqref+mv;
				break;
			case 1:
				amp=ampref+mv;
				break;
		}
	}

	float phase,phasestep;
	float yy;
};

// audio phasors
class rsaw : public phasor {
public:
	rsaw(float f, float g) {
		freq = f;
		gain = g;
		freqref=f;
		gainref=g;
		y = 0;
		delta=0.0;
		deltaref=0.0;
		yy=0;
	}

	void update(int sr) {
		phasestep = 2.0 * freq / sr; // forget not the struggle! A2 ?= 110.25 Hz
		float bot=phasestep*(1.-delta);
		float top=phasestep*(1.+delta);
		float dy=ofRandom(bot,top);
		yy = yy+dy;
		if (yy >= 1) {
			yy = -1;
		}
		y=yy*gain;
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				freq=cv<33?33:cv;
				freqref=freq;
				break;
			case 1:
				gain=cv>1?1:(cv<0?0:cv);
				gainref=gain;
				break;
			case 2:
				delta=cv;
				deltaref=delta;
				break;
		}
	}

	void modulate(int mc, float mv){
		float tmp;
		switch(mc){
			case 0:
				freq=freqref+mv;
				freq=freq<33?33:freq;
				break;
			case 1:
				tmp=gainref*mv;
				gain=tmp>1?1:(tmp<0?0:tmp);
				break;
			case 2:
				delta=deltaref+mv;
				break;
		}
	}

	float delta,yy;
	float deltaref;
};

class squ : public phasor {
public:
	squ(float f, float g) {
		freq = f;
		gain = g;
		freqref=f;
		gainref=g;
		y =0.;
		y_=0;
		yy=1.;
		phase=0.0;
	}

	void update(int sr) {
		phasestep=TWO_PI*freq/sr;
		phase+=phasestep;
		yy=sin(phase);
		y=yy>0?gain:-gain;
		if(phase>TWO_PI) phase-=TWO_PI;
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				freq=cv<33?33:cv;
				freqref=freq;
				break;
			case 1:
				gain=cv>1?1:(cv<0?0:cv);
				gainref=gain;
				break;
		}
	}

	void modulate(int mc, float mv){
		float tmp;
		switch(mc){
			case 0:
				freq=freqref+mv;
				freq=freq<33?33:freq;
				break;
			case 1:
				tmp=gainref*mv;
				gain=tmp>1?1:(tmp<0?0:tmp);
				break;
		}
	}

	float y_,yy;
};

class tri : public phasor {
public:
	tri(float f, float g) {
		freq = f;
		gain = g;
		freqref=f;
		gainref=g;
		y = 0;
		ydir = 1;
		yy=0.;
	}

	void update(int sr) {
		phasestep = 2.0 * freq / sr;
		yy += 2.*ydir*phasestep;
		if (yy >= 1) {
			ydir = -1;
		}else if(yy <= -1){
			ydir = 1;
		}
		y=yy*gain;
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				freq=cv<33?33:cv;
				freqref=freq;
				break;
			case 1:
				gain=cv>1?1:(cv<0?0:cv);
				gainref=gain;
				break;
		}
	}

	void modulate(int mc, float mv){
		float tmp;
		switch(mc){
			case 0:
				tmp=freqref+mv;
				freq=tmp<33?33:tmp;
				break;
			case 1:
				tmp=gainref*mv;
				gain=tmp>1?1:(tmp<0?0:tmp);
				break;
		}
	}

	int ydir;
	float yy;
};

class syn : public phasor {
public:
	syn(float f, float g) {
		freq = f;
		gain = g;
		freqref=f;
		gainref=g;
		y = 0;
		phase = 0.0;
		yy=0.;
	}

	void update(int sr) {
		phasestep = 2.0 * freq * PI / sr;
		phase += phasestep;
		yy = sin(phase);
		if(phase > TWO_PI) phase-=TWO_PI;
		y=yy*gain;
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				freq=cv<33?33:cv;
				freqref=freq;
				break;
			case 1:
				gain=cv>1?1:(cv<0?0:cv);
				gainref=gain;
				break;
		}
	}

	void modulate(int mc, float mv){
		float tmp;
		switch(mc){
			case 0:
				freq=freqref+mv;
				freq=freq<33?33:freq;
				break;
			case 1:
				tmp=gainref*mv;
				gain=tmp>1?1:(tmp<0?0:tmp);
				break;
		}
	}

	float yy;
};

class noyz : public phasor {
public:
	noyz(float f, float g) {
		freq = f;
		gain = g;
		freqref=f;
		gainref=g;
		y = 0;
		phase = 0.0;
		yy=0.;
	}

	void update(int sr) {
		yy = ofRandom(0.99,-0.99);
		y=yy*gain;
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				freq=cv<33?33:cv;
				freqref=freq;
				break; // deep dive into noise color later
			case 1:
				gain=cv>1?1:(cv<0?0:cv);
				gainref=gain;
				break;
		}
	}

	void modulate(int mc, float mv){
		float tmp;
		switch(mc){
			case 0:
				freq=freqref+mv;
				freq=freq<33?33:freq;
				break;
			case 1:
				tmp=gainref*mv;
				gain=tmp>1?1:(tmp<0?0:tmp);
				break;
		}
	}

	float yy;
};

// harmonic series synth
class hxsyn {
public:
	hxsyn(int n, float f, float g, float kf, float kg) {
		nhx=n;
		F=f;
		G=g;
		kF=kf;
		kG=kg;
		hx=new syn*[nhx];
		for(int i=0;i<NCOEFF;i++){
			rot[i]=0;
			vol[i]=MEMLEN-1;
		}
		for(int i=0;i<nhx;i++){
			float fi=F+i*rot[i%NCOEFF];
			float gi=(1.0/(float)NHARM) * ((float)vol[i%NCOEFF] / (float)MEMLEN);
			hx[i]=new syn(fi,gi);
		}
		for(int i=0;i<BUFSZ;i++){
			buf[i]=0.;
		}
		uctr=0;
		y=0.;
		yy=0.;
	}

	void update(int sr) {
		yy=0.;
		for(int i=0;i<nhx;i++){
			yy+=hx[i]->y;
			hx[i]->update(sr);
		}
		y=G*yy;
		buf[uctr]=y;
		uctr=(uctr+1)%BUFSZ;
	}

	void sethxfreqgain(){
		for(int i=0;i<nhx;i++){
			float fi=F+i*rot[i%NCOEFF];
			float gi=(1.0/(float)NHARM) * ((float)vol[i%NCOEFF] / (float)MEMLEN);
			hx[i]->command(0, fi);
			hx[i]->command(1, gi);
		}
	}

	void command(int cc, float cv){
		switch(cc){
			case 0:
				F=cv<33?33:cv;
				Fref=F;
				sethxfreqgain();
				break;
			case 1:
				G=cv>1?1:(cv<0?0:cv);
				Gref=G;
				break;
		}
	}

	void setrot(int pos,int vlu){
		if(pos>=0&&pos<NCOEFF){
			rot[pos]=vlu;
		}
	}

	void setvol(int pos,int vlu){
		if(pos>=0&&pos<NCOEFF){
			vol[pos]=vlu;
		}
	}

	// modulation not yet activated
	void modulate(int mc, float mv){
		float tmp;
		switch(mc){
			case 0:
				F=Fref+mv;
				F=F<33?33:F;
				break;
			case 1:
				tmp=Gref*mv;
				G=tmp>1?1:(tmp<0?0:tmp);
				break;
		}
	}

	float yy,y;
	int nhx,uctr;
	float G,F,Gref,Fref;
	float kF,kG;
	syn ** hx;
	float buf[BUFSZ];
	int rot[NCOEFF];
	int vol[NCOEFF];
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void rndrcc(float x,float y,float r);
		void rndrmpd(float x,float y);
		void rndrkb(float y);
		void audioOut(ofSoundBuffer &outBuffer);
		float idx2freq(int note,float basefreq);
		void kbsweep();
		void setupmpd();
		void setupyc();
		void setupdm();
		void setupsndsys();
		void initvm();
		void cyclevm();
		void rndrmem(float y);
		void f5(float x,float y,int times,ofPixelsRef & pixelsRef);
		void color12(int c);
		void initsynth();
		void rndrmodmat(float x,float y);
		void rndrlfos(float x,float y);
		void modgain(bool mmij, lfo * oo, phasor * ww);
		void modgain(bool mmij, int arg, phasor * ww);
		void xmod();
		void clrmodmat();
		void loadprogram(int pid);
		void loadshader(int pid);
		void initlogo();
		void trtlwalk();
		void initcam();
		void rndrcam(ofPixelsRef & pixelsRef);
		void initshdr();

		// audio
		std::mutex audioMutex;
		ofSoundStream soundStream;
		ofSoundBuffer lastBuffer;
		ofPolyline waveform;
		float rms;

		// synth
		phasor * w[NOSCS][NWAVS];
		lfo * lfo1[NLFOWAVS];
		lfo * lfo2[NLFOWAVS];
		vector<float> lfo1scope;
		vector<float> lfo2scope;
		int lfobufsz;
		int lfoctr;
		int wtyp[NOSCS];
		int lfo1typ,lfo2typ;
		float rootf[NOSCS];
		float rootflo,rootfhi;
		float gain[NOSCS];
		float gainhi,gainlo;
		float hxgainlim,hxgain;
		float mgain;
		float mglo,mghi;
		// natural (just) scale tuning
		// TODO: make this configurable later
		float tuning[12]     = {1.0, 17.0/16.0, 9.0/8.0, 19.0/16.0, 5.0/4.0, 4.0/3.0, 17.0/12.0, 3.0/2.0, 19.0/12.0, 5.0/3.0, 85.0/48.0, 15.0/8.0};
		float finetuning[12] = {0.0,       0.0,     0.0,       0.0,     0.0,     0.0,       0.0,     0.0,       0.0,     0.0,       0.0,      0.0};
		float tunlo,tunhi;
		bool modmat[MMROWS][MMCOLS] = {
			//                     |<---    w[0-8]f    --->|   |<---    w[0-8]g    --->|
			// L1f, L2f, L1g, L2g, 0, 1, 2, 3, 4, 5, 6, 7, 8   0, 1, 2, 3, 4, 5, 6, 7, 8
			{    0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0 }, // L1, [0] [2] -> X
			{    0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0 }, // L2, [1] [3] -> X
			{    0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0 }, // notenmbr
			{    0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0 }  // notevelo
			//   0    1    2    3  4  5  6  7  8  9 10 11 12  13 14 15 16 17 18 19 20 21
		}; // this is bool because i can't fix buffer overflow error that comes with int lol
		int mmctr;
		// harmonic synthesis
		float roothx;
		hxsyn * hxs;
		
		// midi
		u_char * kb;
		off_t kblen;
		const char * reface   = "yamaha.refaceyc.00";
		const char * deepmind = "behringer.deepmind.12";
		int kbfd;
		int activenote;
		int notevelo;

		// vm
		char M[MEMLEN];
		// programs (patches) [0-9]
		// sooner or later these are going to go into a file ;-P
		string prgms[NPRGMS]={
			"^q1qDc13iq0g0c19ccanopqoc1mcknfa`q3 c1x 39N 3aSmM..............................................",
			"mHI0n[g0c73P[c5c10m11\\12y13Fvmcti#t4\\ c,l#t8y c=o#tcF c$r~2...................................",
			"raZLPLc35I0mZ~3f4z806Azc 16cckqcnrntg0 c.a....................................................",

			"AZ[#2|%#6|H#a|T i00i10i20i30p0'p1$p2Gp3Sc2uc6xca,ce;nvg0 c&ic&lc&oc&rf4 mM...................c",
			"mMp0Ii00`01raZIce4 x0d150arg0csocs7 i1r`14 x1e1617c...........................................",
			"n,g0r020#17,p0,cbei0.m.`03i1p10s11rp1.cb.i20`24 c3-f8Vc7 x041570zc&EAh1.......................",

			"x141i00mG`02i10`13630en,g0p0,cnsfa cp6 x051500Z cpep1, cn% /020cpC cE=`24 i2g x3f1............",
			"600r x101 x0d1`04i0mmHi10i20nvg0 c;1/020cv.71nv `12i1zp1vc`* `22i2dp2BraZBcPL.................",
			"^? i0omM p0[n[cdb `0>c1kf4 x0d150)z x10160_u i10`13p1[cd%e5a20g0 =C0F/020 cDM cO  x151........",

			"g0/020c13/020c5ai00i10i20c1ic5lccop0tp1[p2%nt#=|[#)|% c=,c);c$]`03`13`23mmfb10p11p12p........c",
			"n,g0mhf5B............................V85+,c)c1[A81p1)c!#c-$i10c!C/030c3IcKD`11c!Sb;...........",
			"V87i70c24V00cb5g0cgap70c2lcgm`7xc2ufanx c/v...................................................",

			"n&g0v2X5|4p36c5bc9ci30c5km5e3620 =u03 cvlf4`33 c5=r093 c%\\ x041555Z..........................2",
			"A82%251c24`21c2bc6ci20c2km9p2xc2snx cyte6920g0 =]0\\ c-lf7rrtt12t cCFc2E.......................",
			"n,g0j0mTc.5q,c1cf6k1839wI%C=^=ctre9620 =,03>t4C c`q<t6% c^pXt~\">E3I cIo>ta8 cQk<t83 cYl.......2",

			"mMj0q,n,g0c75c93h26if6^j%jzicnpcrjvr%rm6cz. c;i `02i00 p0,c7( x0d156zte8820 =P09 cQ% x14163mz.",
			" x041x151550s630hn,g0P,cimI0/020ckt cvrm,~1fa 10t11t12t13s14s15s16s17s18sA61cR] x2f1x2g1x2i1...",
			"^\\p\\0c13e4520cc4i\\0c1hf2`\\3c1pcciAZp/p%0 cz. =b0;rdlf x0f1c$*5a0r..............................",

			"j0mZk16g7g0ca1A91V66cg5cj6rgmgct7^4>y8L/L97 c/' c]8nz/z1z c$^qzc*Efae590 =N0a.................",
			"nog0j0c35mMqoc1ck1y23^=>m4d/d91cqscuhVf0-z0y c;] c-i i00c3&`02f8 p0oc1J x04153 r x0e1i10c3:`14.",
			"n)g0j0mZk336jq)c1eAe7c35w=fIj#k~]f6Aa2>.8f%f43 c[- c\\9Vg9<*f6%686 cCE cGb.....................z",

			"k1360j0mJg0c 6qonochfJZf4Aaa>rdu%uf0 cvxcz4ea620 =`0a..........................................",
			"k1364j0mJg0c 6q[n[chfJZf4Aa9>rdG%Gf4 cvxcz4ea620 =`0a i00`04p0[chE ca* x0d1553tVa5 cXS.........",
			"n-g0mF B.vb>a5( ce\\w~~B~^B cpm BA....j0c3/k14(bq-c1)B9....`01i00c3Fp0-c1Lf8 x0d1585tx101606r...",

			"n,g0j0k>>>>cn5q,c1fea620=m03f3wB>``^TX,~B c'vm\"J\"<,2> c$w<,4> cC7cC8cC9cCa....................2",
			"nqg0j0kMOMMcn5qqc1fea620=m03f3ww&``^gX,~w c'vm\"J\"<,2& c$w<,4M cC7#C~OcK8#C3McR9#CnMcYa........2",
			"nrg0j0kz#^&cn5qrc1fea620=m03f3w(W``^SX,~( c'vm\"J\"<,2W c$w<,4z cC7#C2#cK8#C4^cR9#C6&cYa........2"
		};
		// alfabet
		string AB="0123456789abcdefghijklmnopqrstuvwxyz,./;'[]-=\\` )!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ<>?:\"{}_+|~";
		int pc;
		int ec;
		ofTrueTypeFont fnt;
		int f5times;
		int cablehues[NHUES]={
			0x00f,0x0f0,0xf00, 0x01e,0x1e0,0x10e, 0x02d,0x2d0,0x20d, 0x03c,0x3c0,0x30c,
			0x04b,0x4b0,0x40b, 0x05a,0x5a0,0x50a, 0x069,0x690,0x609, 0x078,0x780,0x708,
			0x087,0x870,0x807, 0x096,0x960,0x906, 0x0a5,0xa50,0xa05
		};

		// "mAI0n[g0c73P[c5c10m11\12y13Fvmcti#t4\ c,l#t8y c=o#tcF c$r~2..................................."
		//  0123456789abcdefghijklmnopqrstuvwxyz,./;'[]-=\` )!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ<>?:"{}_+|~

		// key gestures
		int lcdown;
		int lcjmp;
		int lsdown;
		bool ladown; // left alt down

		// gui
		float vmx,vmy;

		// logo
		trtl * tx[NOSCS];
		int txctr;
		int tpc; // turtle vm

		// asciicam
		ofVideoGrabber vdo;
		int camw,camh;
		string asciiChars;
		ofTrueTypeFont camfnt;

		// shading
		ofShader shdr[NGLSL];
		ofPlanePrimitive plane;
		int curshdr;
		float t,dt; // time arrives finally!

		// !programmer! //
		bool DEBUG=false; // cout filter
};
