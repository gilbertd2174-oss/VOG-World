#pragma once
#include "Parameters.h"
#include <vector>
#include <cstdint>
namespace davoe {
constexpr float pi=3.14159265358979323846f;
struct Envelope {
 enum Stage { Idle,Attack,Decay,Sustain,Release } stage=Idle;
 float value=0, releaseStep=0;
 void on(){stage=Attack;}
 void off(float ms,float sr){if(stage!=Idle&&stage!=Release){stage=Release;releaseStep=value/std::max(1.f,ms*.001f*sr);}}
 void reset(){stage=Idle;value=0;}
 float tick(float a,float d,float s,float sr){
  switch(stage){
   case Attack:value+=1.f/std::max(1.f,a*.001f*sr);if(value>=1){value=1;stage=Decay;}break;
   case Decay:value-=(1-s)/std::max(1.f,d*.001f*sr);if(value<=s){value=s;stage=Sustain;}break;
   case Sustain:value=s;break;
   case Release:value-=releaseStep;if(value<=0){value=0;stage=Idle;}break;
   default:break;
  }return value;
 }
};
inline float blep(float t,float dt){
 if(t<dt){t/=dt;return t+t-t*t-1;}
 if(t>1-dt){t=(t-1)/dt;return t*t+t+t+1;}
 return 0;
}
inline float oscillator(float phase,float dt,int shape,float pw){
 if(shape==0)return 2*phase-1-blep(phase,dt);
 if(shape==1){float q=phase-pw;if(q<0)q+=1;return (phase<pw?1.f:-1.f)+blep(phase,dt)-blep(q,dt);}
 // Triangle uses a finite band-limited odd-harmonic series, avoiding corner aliasing.
 if(shape==2){float y=0;for(int h=1;h<=15;h+=2){if(h*dt>=.48f)break; y+=(((h-1)/2)%2?-1.f:1.f)*std::sin(2*pi*phase*h)/(float)(h*h);}return y*.81056947f;}
 return std::sin(2*pi*phase);
}
struct Filter {
 float s1=0,s2=0;
 void reset(){s1=s2=0;}
 float tick(float x,float hz,float res,int mode,float sr){
  float g=std::tan(pi*clamp(hz,20.f,sr*.42f)/sr),k=2.f-1.9f*res;
  float a=1.f/(1.f+g*(g+k));float v1=a*(s1+g*(x-s2));float v2=s2+g*v1;
  s1=2*v1-s1;s2=2*v2-s2;
  return mode==0?v2:(mode==1?x-k*v1-v2:v1);
 }
};
struct Voice {
 int note=-1,channel=0;uint64_t age=0;float velocity=0,phase1=0,phase2=.23f,phase3=.47f,phase4=.71f,phaseSub=0,pitch=60,pan=0;
 Envelope amp,env;Filter filter;
 bool active()const{return amp.stage!=Envelope::Idle;}
 void start(int n,int ch,float vel,uint64_t stamp,bool legato,float sr,const Values& p){
  bool was=active()&&amp.stage!=Envelope::Release;note=n;channel=ch;velocity=vel;age=stamp;
  if(!legato||!was){pitch=(float)n;phase1=0;phase2=.23f;phase3=.47f;phase4=.71f;phaseSub=0;filter.reset();amp.reset();env.reset();}
  if(!legato||!was){amp.on();env.on();}
  pan=((n*37%101)/100.f-.5f)*p[Spread];(void)sr;
 }
 void release(float sr,const Values& p){amp.off(p[AmpR],sr);env.off(p[FilterR],sr);}
};
struct DelayLine {
 std::vector<float> data;int pos=0;
 void prepare(int size){data.assign((size_t)std::max(4,size),0);pos=0;}
 void reset(){std::fill(data.begin(),data.end(),0);pos=0;}
 float read(float samples)const{float at=(float)pos-clamp(samples,1.f,(float)data.size()-2);if(at<0)at+=(float)data.size();int a=(int)at;int b=(a+1)%(int)data.size();float f=at-a;return data[(size_t)a]+f*(data[(size_t)b]-data[(size_t)a]);}
 void push(float x){data[(size_t)pos]=x;if(++pos==(int)data.size())pos=0;}
};
struct Biquad {
 float b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 void peak(float hz,float db,float q,float sr){float A=std::pow(10.f,db/40.f),w=2*pi*std::min(hz,sr*.4f)/sr,alpha=std::sin(w)/(2*q),c=std::cos(w),norm=1/(1+alpha/A);b0=(1+alpha*A)*norm;b1=-2*c*norm;b2=(1-alpha*A)*norm;a1=b1;a2=(1-alpha/A)*norm;}
 float tick(float x){float y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}
 void reset(){z1=z2=0;}
};
struct Effects {
 DelayLine chorus[2],delay[2],pre[2],comb[2][4],allpass[2][2];Biquad eq[2][7],tone[2][3];
 float damp[2][4]{},low[2]{},high[2]{},previous[2]{},dcIn[2]{},dcOut[2]{};
 float phase=0,verbPhase=0,fxPhase=0,envelope=0,reduction=0;
 float attack=.99f,release=.999f,compMakeup=1,distGain=1,toneGain=1;
 float on[5]={1,1,0,1,1};
 void prepare(float sr){for(int c=0;c<2;c++){
  chorus[c].prepare((int)(sr*.06f));delay[c].prepare((int)(sr*1.1f));pre[c].prepare((int)(sr*.4f));
  for(int k=0;k<4;k++)comb[c][k].prepare((int)(sr*.09f));
  allpass[c][0].prepare((int)(sr*.005f)+c*13);allpass[c][1].prepare((int)(sr*.0017f)+c*7);
 }reset();}
 void update(const Values& p,float sr){
  attack=std::exp(-1/(sr*p[CompAttack]*.001f));release=std::exp(-1/(sr*p[CompRelease]*.001f));compMakeup=std::pow(10.f,p[CompMakeup]/20.f);
  distGain=1+p[DistDrive]*24;toneGain=std::pow(10.f,p[DistLevel]/20.f);
  const float hz[]={100,200,400,800,1600,3200,6400};
  for(int c=0;c<2;c++){for(int k=0;k<7;k++)eq[c][k].peak(hz[k],p[(size_t)(Eq100+k)],p[EqWidth],sr);
   tone[c][0].peak(120,p[DistBass],.6f,sr);tone[c][1].peak(1000,p[DistMid],.7f,sr);tone[c][2].peak(6000,p[DistTreble],.6f,sr);}
 }
 void reset(){phase=verbPhase=fxPhase=envelope=reduction=0;for(int c=0;c<2;c++){chorus[c].reset();delay[c].reset();pre[c].reset();low[c]=high[c]=previous[c]=dcIn[c]=dcOut[c]=0;for(auto& b:eq[c])b.reset();for(auto& b:tone[c])b.reset();for(int k=0;k<4;k++){comb[c][k].reset();damp[c][k]=0;}for(auto& a:allpass[c])a.reset();}}
 float distort(float x,int mode)const{
  float d=x*distGain;
  if(mode==0)return std::tanh(d+.2f)-std::tanh(.2f);
  if(mode==1)return std::tanh(d*3.f)*(.7f+.3f*std::tanh(std::abs(d)*4.f));
  return d/(1+std::abs(d));
 }
 void tick(float& l,float& r,const Values& p,float sr,double bpm,float wheel){
  const Param switches[]={CompOn,EqOn,DistOn,EchoOn,VerbOn};for(int k=0;k<5;k++)on[k]+=(p[switches[k]]-on[k])*.002f;
  float peak=std::max(std::abs(l),std::abs(r)),coef=peak>envelope?attack:release;envelope=coef*envelope+(1-coef)*peak;
  float db=20*std::log10(std::max(1.e-8f,envelope));float gr=std::max(0.f,db-p[CompThreshold])*(1-1/p[CompRatio]);
  reduction=gr*on[0];float cg=std::pow(10.f,-gr/20.f)*compMakeup;float blend=p[CompMix]*on[0];l*=1+blend*(cg-1);r*=1+blend*(cg-1);
  phase+=.24f/sr;if(phase>=1)phase-=1;
  verbPhase+=p[VerbRate]/sr;if(verbPhase>=1)verbPhase-=1;
  fxPhase+=(p[FxSync]>.5f?(float)bpm/60.f:p[FxRate])/sr;if(fxPhase>=1)fxPhase-=std::floor(fxPhase);
  int shape=(int)p[FxShape];float mod=shape==0?std::sin(2*pi*fxPhase):(shape==1?1-4*std::abs(fxPhase-.5f):(fxPhase<.5f?1.f:-1.f));
  mod*=p[FxDepth]*(p[FxWheel]>.5f?wheel:1.f);
  float input[]={l,r},out[2]{};
  float echoMs=p[EchoSync]>.5f?(float)(45000.0/std::max(20.0,bpm)):p[DelayTime];echoMs=clamp(echoMs,40,1000);
  const float times[]={.0297f,.0371f,.0411f,.0437f};
  for(int c=0;c<2;c++){
   float x=input[c],equal=x;for(auto& b:eq[c])equal=b.tick(equal);x+=(equal-x)*on[1];
   float wetDist=(distort((x+previous[c])*.5f,(int)p[DistMode])+distort(x,(int)p[DistMode]))*.5f;previous[c]=x;
   float dc=wetDist-dcIn[c]+.995f*dcOut[c];dcIn[c]=wetDist;dcOut[c]=dc;
   for(auto& b:tone[c])dc=b.tick(dc);dc*=toneGain;x+=(dc-x)*p[DistMix]*on[2];
   float t=sr*(.018f+(.004f+.002f*mod)*std::sin(2*pi*(phase+c*.27f)));
   float ch=chorus[c].read(t);chorus[c].push(x);x=x*(1-.4f*p[Chorus])+ch*p[Chorus]*.65f;
   float wow=1+.003f*p[EchoWow]*std::sin(2*pi*(phase*.73f+c*.13f));float tapeTime=sr*.001f*echoMs*wow;
   float echo=delay[c].read(tapeTime);int heads=(int)p[EchoHeads];
   if(heads==1||heads==3)echo+=delay[c].read(tapeTime/3)*.65f;
   if(heads==2||heads==3)echo+=delay[c].read(tapeTime*2/3)*.65f;
   echo/=heads==3?2.3f:(heads==0?1.f:1.65f);
   float fb=clamp(p[Feedback]+mod*.1f,0,.9f);delay[c].push(std::tanh(x*on[3]+echo*fb));x+=echo*p[DelayMix]*on[3];
   high[c]+=(1-std::exp(-2*pi*p[VerbHigh]/sr))*(x-high[c]);float hp=x-high[c];
   low[c]+=(1-std::exp(-2*pi*p[VerbLow]/sr))*(hp-low[c]);pre[c].push(low[c]*on[4]);
   float preTime=sr*.001f*p[PreDelay];float feed=pre[c].read(std::max(1.f,preTime));
   float early=pre[c].read(preTime+sr*.017f)*.5f+pre[c].read(preTime+sr*.031f)*.3f;
   float wet=0;for(int k=0;k<4;k++){auto& d=comb[c][k];float sec=(times[k]+c*.0011f)*(.8f+.5f*p[Space]);float len=sr*sec;
    float modulation=sr*.00035f*p[VerbDepth]*std::sin(2*pi*(verbPhase+k*.23f+c*.17f));float y=d.read(len+modulation);
    damp[c][k]+=(.8f-.75f*p[VerbDamp])*(y-damp[c][k]);float decay=std::pow(.001f,sec/p[VerbDecay]);
    d.push(feed*.2f+damp[c][k]*decay);wet+=y*.25f;}
   for(auto& a:allpass[c]){float y=a.data[(size_t)a.pos];float z=y-wet*.5f;a.push(wet+y*.5f);wet=z;}
   float vm=clamp(p[Reverb]+mod*.08f,0,.8f)*on[4];out[c]=x*(1-.25f*vm)+(wet*1.7f+early*p[Early]) *vm;
  }l=out[0];r=out[1];
 }
};
class Engine {
 float sr=48000,smoothCoeff=.001f,lfoPhase=0,lfoHeld=0;uint32_t rng=0x9e3779b9;uint64_t serial=0;
 Values current=defaults(),target=defaults();std::array<Voice,16> voices{};
 std::array<std::array<float,128>,16> held{},down{};std::array<bool,16> pedal{};
 std::array<float,16>bends{},mods{};Effects effects;
 bool arpWas=false,monoWas=false;double arpSamples=0,gateSamples=0;int arpStep=0,arpNote=-1,arpChannel=0;
 float random(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (float)(rng&0xffffff)/8388608.f-1;}
 void start(int note,int ch,float vel){
  Voice* selected=nullptr;
  if(target[Mono]>.5f)selected=&voices[0];
  else{for(auto& v:voices)if(!v.active()){selected=&v;break;}
   if(!selected)selected=&*std::min_element(voices.begin(),voices.end(),[](const Voice&a,const Voice&b){bool ar=a.amp.stage==Envelope::Release,br=b.amp.stage==Envelope::Release;return ar!=br?ar:a.age<b.age;});}
  selected->start(note,ch,vel,++serial,target[Mono]>.5f&&target[Glide]>0,sr,current);
 }
 void releaseNote(int n,int ch){for(auto& v:voices)if(v.note==n&&v.channel==ch)v.release(sr,current);}
 void releaseAll(){for(auto& v:voices)v.release(sr,current);arpNote=-1;}
 int collect(std::array<int,384>& notes,std::array<int,384>& chans)const{
  int count=0;int oct=(int)target[ArpOctaves];for(int o=0;o<oct;o++)for(int n=0;n<128;n++)for(int c=0;c<16;c++)if(held[(size_t)c][(size_t)n]>0&&n+12*o<=127&&count<384){notes[(size_t)count]=n+12*o;chans[(size_t)count]=c;count++;}return count;
 }
 void arpTick(double bpm){
  if(gateSamples>0&&--gateSamples<=0&&arpNote>=0){releaseNote(arpNote,arpChannel);arpNote=-1;}
  if(--arpSamples>0)return;
  std::array<int,384> notes{},chans{};int count=collect(notes,chans);if(count==0){arpSamples=0;return;}
  const double beats[]={1,.5,.25,1.0/3,1.0/6};double length=sr*60.0/std::max(20.0,bpm)*beats[(size_t)(int)target[ArpRate]];
  arpSamples+=length;gateSamples=length*target[ArpGate];
  int mode=(int)target[ArpMode],index=arpStep%count;
  if(mode==1)index=count-1-index;
  else if(mode==2&&count>1){int p=arpStep%(2*count-2);index=p<count?p:2*count-2-p;}
  else if(mode==3)index=std::min(count-1,(int)((random()+1)*.5f*count));
  if(arpNote>=0)releaseNote(arpNote,arpChannel);
  arpNote=notes[(size_t)index];arpChannel=chans[(size_t)index];
  int base=arpNote;while(base>0&&held[(size_t)arpChannel][(size_t)base]==0)base-=12;
  start(arpNote,arpChannel,std::max(.1f,held[(size_t)arpChannel][(size_t)std::max(0,base)]));
  arpStep=(arpStep+1)%1000000;
 }
public:
 void prepare(double sampleRate){sr=(float)sampleRate;smoothCoeff=1-std::exp(-1.f/(sr*.015f));effects.prepare(sr);current=target;panic();}
 float gainReduction()const{return effects.reduction;}
 void setParameters(const Values& p){for(size_t i=0;i<p.size();i++)target[i]=std::isfinite(p[i])?clamp(p[i],specs[i].lo,specs[i].hi):specs[i].initial;
  effects.update(target,sr);
  bool arp=target[ArpOn]>.5f,mono=target[Mono]>.5f;
  if(arp!=arpWas||mono!=monoWas){releaseAll();arpStep=0;arpSamples=0;gateSamples=0;if(!arp)for(int c=0;c<16;c++)for(int n=0;n<128;n++)if(held[(size_t)c][(size_t)n]>0)start(n,c,held[(size_t)c][(size_t)n]);}
  if(arpWas&&current[ArpHold]>.5f&&target[ArpHold]<.5f){for(int c=0;c<16;c++)for(int n=0;n<128;n++)if(down[(size_t)c][(size_t)n]==0&&!pedal[(size_t)c])held[(size_t)c][(size_t)n]=0;}
  arpWas=arp;monoWas=mono;
 }
 void noteOn(int note,float velocity,int channel=0){if(note<0||note>127||channel<0||channel>15)return;
  if(velocity<=0){noteOff(note,channel);return;}
  if(arpWas&&target[ArpHold]>.5f){bool any=false;for(const auto& c:down)for(float v:c)any=any||v>0;if(!any)for(auto& c:held)c.fill(0);}
  held[(size_t)channel][(size_t)note]=down[(size_t)channel][(size_t)note]=clamp(velocity,0,1);
  if(target[FxKeyReset]>.5f)effects.fxPhase=0;
  if(!arpWas)start(note,channel,velocity);
 }
 void noteOff(int note,int channel=0){if(note<0||note>127||channel<0||channel>15)return;down[(size_t)channel][(size_t)note]=0;
  if(pedal[(size_t)channel]||(arpWas&&target[ArpHold]>.5f))return;
  held[(size_t)channel][(size_t)note]=0;
  if(!arpWas){releaseNote(note,channel);if(target[Mono]>.5f&&voices[0].note==note){for(int n=127;n>=0;--n)if(held[(size_t)channel][(size_t)n]>0){start(n,channel,held[(size_t)channel][(size_t)n]);break;}}}
 }
 void controller(int number,int value,int ch=0){if(ch<0||ch>15)return;
  if(number==1)mods[(size_t)ch]=value/127.f;
  if(number==64){bool was=pedal[(size_t)ch];pedal[(size_t)ch]=value>=64;if(was&&value<64){for(int n=0;n<128;n++)if(down[(size_t)ch][(size_t)n]==0){if(!(arpWas&&target[ArpHold]>.5f))held[(size_t)ch][(size_t)n]=0;if(!arpWas)releaseNote(n,ch);}}}
  if(number==120||number==123)panic();
 }
 void pitchWheel(int value,int ch=0){if(ch>=0&&ch<16)bends[(size_t)ch]=clamp((value-8192)/8192.f,-1,1);}
 void panic(){for(auto& v:voices){v.amp.reset();v.env.reset();v.filter.reset();v.note=-1;}for(auto& c:held)c.fill(0);for(auto& c:down)c.fill(0);pedal.fill(false);bends.fill(0);mods.fill(0);effects.reset();arpSamples=gateSamples=0;arpStep=0;arpNote=-1;}
 int activeVoices()const{int count=0;for(auto& v:voices)if(v.active())count++;return count;}
 void sample(float& left,float& right,double hostBpm=0){
  for(size_t i=0;i<current.size();i++){bool discrete=specs[i].step==1&&specs[i].skew==1;current[i]=discrete?target[i]:current[i]+smoothCoeff*(target[i]-current[i]);}
  if(arpWas)arpTick(hostBpm>0?hostBpm:target[Tempo]);
  lfoPhase+=current[LfoRate]/sr;if(lfoPhase>=1){lfoPhase-=1;lfoHeld=random();}
  int shape=(int)target[LfoShape];float lfo=shape==0?std::sin(2*pi*lfoPhase):(shape==1?1-4*std::abs(lfoPhase-.5f):(shape==2?(lfoPhase<.5f?1.f:-1.f):lfoHeld));
  left=right=0;
  for(auto& v:voices){if(!v.active())continue;
   if(target[Mono]>.5f&&current[Glide]>0)v.pitch+=(v.note-v.pitch)*(1-std::exp(-1.f/(sr*.001f*std::max(1.f,current[Glide]))));else v.pitch=(float)v.note;
   float pitch=v.pitch+bends[(size_t)v.channel]*current[BendRange]+lfo*(current[LfoPitch]+mods[(size_t)v.channel]*.5f);
   float hz1=midiHz(pitch+12*target[Octave1]),hz2=midiHz(pitch+12*target[Octave2]+current[Detune]*.01f);
   float dt1=std::min(.45f,hz1/sr),dt2=std::min(.45f,hz2/sr);
   float dt3=std::min(.45f,midiHz(pitch+12*target[Octave3]+current[Detune3]*.01f)/sr),dt4=std::min(.45f,midiHz(pitch+12*target[Octave4]+current[Detune4]*.01f)/sr);
   float x=oscillator(v.phase1,dt1,(int)target[Wave1],clamp(current[Pulse1]+lfo*current[LfoPWM],.05f,.95f))*current[Level1];
   x+=oscillator(v.phase2,dt2,(int)target[Wave2],clamp(current[Pulse2]-lfo*current[LfoPWM],.05f,.95f))*current[Level2];
   x+=oscillator(v.phase3,dt3,(int)target[Wave3],clamp(current[Pulse3]+lfo*current[LfoPWM],.05f,.95f))*current[Level3];
   x+=oscillator(v.phase4,dt4,(int)target[Wave4],clamp(current[Pulse4]-lfo*current[LfoPWM],.05f,.95f))*current[Level4];
   v.phase3+=dt3;v.phase3-=std::floor(v.phase3);v.phase4+=dt4;v.phase4-=std::floor(v.phase4);
   x+=std::sin(2*pi*v.phaseSub)*current[Sub]+random()*current[Noise];
   v.phase1+=dt1;v.phase1-=std::floor(v.phase1);v.phase2+=dt2;v.phase2-=std::floor(v.phase2);v.phaseSub+=dt1*.5f;v.phaseSub-=std::floor(v.phaseSub);
   float e=v.env.tick(current[FilterA],current[FilterD],current[FilterS],sr);
   float cutoff=current[Cutoff]*std::exp2(e*current[EnvAmount]+lfo*current[LfoFilter]+current[KeyTrack]*(v.note-60)/12.f);
   x=std::tanh(x*(1+current[Drive]*5))*.5f;x=v.filter.tick(x,cutoff,current[Resonance],(int)target[FilterMode],sr);
   x*=v.amp.tick(current[AmpA],current[AmpD],current[AmpS],sr)*v.velocity*(1-current[LfoAmp]*(.5f+.5f*lfo))*.22f;
   float pan=v.pan;left+=x*std::sqrt(.5f-pan*.8f);right+=x*std::sqrt(.5f+pan*.8f);
  }
  float wheel=0;for(float m:mods)wheel=std::max(wheel,m);
  effects.tick(left,right,current,sr,hostBpm>0?hostBpm:target[Tempo],wheel);float gain=std::pow(10.f,current[Master]/20.f);
  left=std::tanh(left*gain);right=std::tanh(right*gain);
  if(!std::isfinite(left)||!std::isfinite(right)){panic();left=right=0;}
 }
};
}
