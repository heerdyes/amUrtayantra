#pragma once

#include "ofMain.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MEMLEN 94
#define NHUES 33
#define NWAVS 5
#define NLFOWAVS 2
#define NOSCS 9
#define MMROWS 4
#define MMCOLS 22
#define NPRGMS 5

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
		void rndryc(float y);
		void audioOut(ofSoundBuffer &outBuffer);
		float idx2freq(int note,float basefreq);
		void kbsweep();
		void setupmpd();
		void setupyc();
		void setupsndsys();
		void initvm();
		void cyclevm();
		void rndrmem(float y);
		void f5(float x,float y,int times);
		void color12(int c);
		void initsynth();
		void rndrmodmat(float x,float y);
		void rndrlfos(float x,float y);
		void modgain(bool mmij, lfo * oo, phasor * ww);
		void modgain(bool mmij, int arg, phasor * ww);
		void xmod();
		void clrmodmat();
		void loadprogram(int pid);

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
		float mgain;
		float mglo,mghi;
		// natural (just) scale tuning
		// TODO: make this configurable later
		float tuning[12] = {1.0, 17.0/16.0, 9.0/8.0, 19.0/16.0, 5.0/4.0, 4.0/3.0, 17.0/12.0, 3.0/2.0, 19.0/12.0, 5.0/3.0, 85.0/48.0, 15.0/8.0};
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
		
		// midi
		u_char * yc;
		off_t yclen;
		const char * reface="yamaha.refaceyc.00";
		int ycfd;
		int activenote;
		int notevelo;

		// vm
		char M[MEMLEN];
		// programs (patches) [0-9]
		string prgms[NPRGMS]={
			"n,vQg0 =3,5c19 P,c3nmd!Q~2 c1gfae4520 =z05I0 c,-..............................................",
			"mAI0n[g0c73P[c5c10m11\\12y13Fvmcti#t4\\ c,l#t8y c=o#tcF c$r~2...................................",
			"raZLPLc35I0mZ~3f4z806Azc 16cckqcnrntg0 c.a....................................................",
			"AZ[#2|%#6|H#a|T i00i10i20i30p0'p1$p2Gp3Sc2uc6xca,ce;nvg0 c&ic&lc&oc&rf4 mf...................c",
			"m.p0Ii00`01raZIce4 x0d150arg0csocs7 i1r`14 x1e1617c..........................................."
		};
		// alfabet
		string AB="0123456789abcdefghijklmnopqrstuvwxyz,./;'[]-=\\` )!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ<>?:\"{}_+|~";
		string CMD=".ng^AvV><L1!fc=b+#-*X%iI/mpP`~zretx5678";
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
};
