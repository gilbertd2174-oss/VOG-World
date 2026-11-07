#pragma once
#include "SynthEngine.h"
#include "WorldParameters.h"
#include <memory>
namespace world {
struct SampleData{std::vector<float> left,right;double rate=48000;};
struct Effects{
 davoe::DelayLine echo[2],chorus[2],predelay[2],comb[2][4];float damping[2][4]{},length[2][4]{},feedback[2][4]{};float sr=48000,phase=0,gain=.4f;Values p=defaults();
 void prepare(float rate){sr=rate;for(int c=0;c<2;c++){echo[c].prepare((int)(sr*1.1f));chorus[c].prepare((int)(sr*.05f));predelay[c].prepare((int)(sr*.2f));for(int k=0;k<4;k++)comb[c][k].prepare((int)(sr*.08f));}reset();update(p);}
 void reset(){phase=0;for(int c=0;c<2;c++){echo[c].reset();chorus[c].reset();predelay[c].reset();for(int k=0;k<4;k++){comb[c][k].reset();damping[c][k]=0;}}}
 void update(const Values&v){p=v;gain=std::pow(10.f,p[Volume]/20);const float times[]={.0297f,.0371f,.0411f,.0437f};for(int c=0;c<2;c++)for(int k=0;k<4;k++){float sec=(times[k]+c*.0011f)*(.8f+p[Room]*.5f);length[c][k]=sec*sr;feedback[c][k]=std::pow(.001f,sec/(.6f+p[Room]*5));}}
 void sample(float&l,float&r){phase+=.3f/sr;if(phase>=1)phase-=1;float input[]={l,r},out[2]{};for(int c=0;c<2;c++){float x=input[c];float ch=chorus[c].read(sr*(.018f+.004f*std::sin(2*davoe::pi*(phase+c*.25f))));chorus[c].push(x);x=x*(1-p[Chorus]*.3f)+ch*p[Chorus]*.6f;float e=echo[c].read(sr*.001f*p[EchoTime]*(c? .997f:1));echo[c].push(std::tanh(x+e*p[Feedback]));x+=e*p[EchoMix];predelay[c].push(x);float feed=predelay[c].read(std::max(1.f,sr*.001f*p[PreDelay]));float wet=0;for(int k=0;k<4;k++){float y=comb[c][k].read(length[c][k]);damping[c][k]+=.3f*(y-damping[c][k]);comb[c][k].push(feed*.22f+damping[c][k]*feedback[c][k]);wet+=y*.25f;}out[c]=std::tanh((x*(1-.2f*p[Reverb])+wet*p[Reverb]*1.7f)*gain);}l=out[0];r=out[1];}
};
struct FastFilter{float g=0,k=2,a=1,s1=0,s2=0;int mode=0;void reset(){s1=s2=0;}void setup(float hz,float resonance,int m,float sr){g=std::tan(davoe::pi*davoe::clamp(hz,20,sr*.42f)/sr);k=2-1.9f*resonance;a=1/(1+g*(g+k));mode=m;}float tick(float x){float v1=a*(s1+g*(x-s2)),v2=s2+g*v1;s1=2*v1-s1;s2=2*v2-s2;return mode==2?v1:v2;}};
struct Voice{
 davoe::DelayLine string;davoe::Envelope amp;FastFilter drumNoise,bodyFilter,toneFilter,bodyFilterR,toneFilterR;std::shared_ptr<const SampleData> sampleData;double samplePosition=0;float lastRight=0;std::array<float,8>phases{};int note=-1,channel=0,model=0;uint64_t ageId=0;float vel=0,seconds=0,pan=0,lp=0,sr=48000;uint32_t rng=1;float baseHz=440,loopDecay=.99f,detuneRatio=1,drumAmp=1,noiseAmp=1,sweepAmp=1,drumRate=1,noiseRate=1,sweepRate=1;std::array<float,8>modalAmp{},modalRate{},harmonicWeight{};
 float noise(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (rng&0xffffff)/8388608.f-1;}
 void prepare(float rate){sr=rate;string.prepare((int)(sr/8)+32);reset();}
 void reset(){amp.reset();drumNoise.reset();bodyFilter.reset();toneFilter.reset();bodyFilterR.reset();toneFilterR.reset();sampleData.reset();samplePosition=0;lastRight=0;lp=0;}
 bool active()const{return amp.stage!=davoe::Envelope::Idle;}
 void configure(const Values&p){baseHz=davoe::midiHz((float)note+p[Transpose]+p[Tune]*.01f);loopDecay=std::pow(.001f,1/(std::max(8.f,baseHz)*p[Decay]*.001f));detuneRatio=std::exp2(p[Detune]/1200);float bodyHz=400+p[Body]*3300;bodyFilter.setup(bodyHz,.4f,2,sr);bodyFilterR.setup(bodyHz,.4f,2,sr);toneFilter.setup(p[FilterCutoff],p[FilterResonance],0,sr);toneFilterR.setup(p[FilterCutoff],p[FilterResonance],0,sr);for(int h=0;h<8;h++){modalRate[(size_t)h]=std::exp(-1/(sr*std::max(.04f,p[Decay]*.001f)/(1+h*.3f)));harmonicWeight[(size_t)h]=std::pow(.25f+p[Brightness]*.6f,(float)h)/(1+h*.2f);}drumNoise.setup(900+p[Brightness]*6500,.2f,2,sr);if(model==5)for(int h=0;h<8;h++)modalRate[(size_t)h]=std::exp(-1/(sr*std::max(.012f,p[Decay]*.0003f)/(1+h*.6f)));drumRate=std::exp(-1/(sr*std::max(.02f,p[Decay]*.0005f)));noiseRate=std::exp(-1/(sr*(.01f+p[Air]*.6f)));sweepRate=std::exp(-1/(sr*.025f));}
 void on(int n,int ch,float velocity,uint64_t id,const Values&p,std::shared_ptr<const SampleData> loaded){reset();note=n;channel=ch;vel=velocity;ageId=id;model=p[Source]>.5f?6:(int)p[Model];sampleData=loaded;samplePosition=0;seconds=0;phases.fill(0);modalAmp.fill(1);drumAmp=noiseAmp=sweepAmp=1;configure(p);pan=((n*37%101)/100.f-.5f)*p[Spread];rng=0x9e3779b9u+(uint32_t)(id*31+n*997);float hz=davoe::midiHz((float)n+p[Transpose]+p[Tune]*.01f);float period=sr/std::max(8.f,hz);if(model==0){string.pos=0;for(size_t i=0;i<string.data.size();i++){float t=(float)i/std::max(2.f,period);string.data[i]=noise()*(.4f+.6f*p[Strike])*(.5f+.5f*std::sin(davoe::pi*t));}}amp.on();}
 void off(const Values&p){amp.off(p[Release],sr);}
 float sample(const Values&p,float bend,float expression){if(!active())return 0;seconds+=1/sr;float vibrato=std::sin(2*davoe::pi*p[VibratoRate]*seconds)*(p[VibratoDepth]+expression*35)*.01f;float hz=baseHz*std::exp2((bend+vibrato)/12.f);float dt=std::min(.45f,hz/sr);float x=0;float xr=0;
 if(model==6){if(!sampleData||sampleData->left.size()<2){amp.reset();lastRight=0;return 0;}auto size=sampleData->left.size();if(samplePosition>=size-1){if(p[SampleLoop]>.5f)samplePosition=std::fmod(samplePosition,(double)(size-1));else{amp.reset();lastRight=0;return 0;}}size_t index=(size_t)samplePosition;float frac=(float)(samplePosition-index);x=sampleData->left[index]+frac*(sampleData->left[index+1]-sampleData->left[index]);xr=sampleData->right[index]+frac*(sampleData->right[index+1]-sampleData->right[index]);if(p[SampleLoop]<.5f){float fade=davoe::clamp((float)((size-1-samplePosition)/(sampleData->rate*.003)),0,1);x*=fade;xr*=fade;}samplePosition+=sampleData->rate/sr*std::exp2(((float)note+p[Transpose]-p[SampleRoot]+p[Tune]*.01f+bend+vibrato)/12.f);}
 else if(model==0){float delay=davoe::clamp(sr/std::max(8.f,hz)-.5f,2,(float)string.data.size()-2);float y=string.read(delay);float bright=.15f+.8f*p[Brightness];lp+=bright*(y-lp);string.push((y*.5f+lp*.5f)*loopDecay);x=y;}
 else if(model==1){for(int h=1;h<=6;h++){if(h*dt>=.48f)break;float weight=h==1?1.f:std::pow(p[Brightness]*.75f,(float)(h-1))/(float)h;x+=std::sin(2*davoe::pi*phases[0]*h)*weight;}x+=noise()*p[Air]*.35f*(.6f+.4f*std::sin(seconds*7));x*=.7f;}
 else if(model==2){x=davoe::oscillator(phases[0],dt,0,.5f)*.65f+std::sin(2*davoe::pi*phases[1])*.3f;x+=noise()*p[Air]*.12f;x=std::tanh(x*(1+p[Material]*2))*.8f;}
 else if(model==3){const float ratio[]={1,2,2.78f,3.95f,5.43f,6.9f,8.2f,10.3f};for(int h=0;h<8;h++){float r=(h+1)*(1-p[Material])+ratio[h]*p[Material];float f=hz*r;if(f>=sr*.45f)continue;phases[(size_t)h]+=f/sr;phases[(size_t)h]-=std::floor(phases[(size_t)h]);modalAmp[(size_t)h]*=modalRate[(size_t)h];x+=std::sin(2*davoe::pi*phases[(size_t)h])*modalAmp[(size_t)h]*harmonicWeight[(size_t)h];}x*=.7f;}
 else if(model==4){float pw=.12f+p[Body]*.65f;x=davoe::oscillator(phases[0],dt,1,pw)*.65f+davoe::oscillator(phases[1],dt,0,.5f)*.25f+noise()*p[Air]*.15f;}
 else{
  // A membrane body with independently damped modes, a short hand attack,
  // and filtered noise textures. Velocity changes attack colour as well as level.
  sweepAmp*=sweepRate;drumAmp*=drumRate;noiseAmp*=noiseRate;float rawNoise=noise();float filtered=drumNoise.tick(rawNoise);float tonal=0;const float ratios[]={1.f,1.593f,2.136f,2.296f,2.653f};const float weights[]={1.f,.48f,.28f,.17f,.1f};
  for(int h=0;h<5;h++){float ratio=1+(ratios[h]-1)*(.75f+p[Material]*.45f);float f=hz*ratio*(1+(.08f+p[Material]*.22f)*sweepAmp);if(f>sr*.42f)continue;size_t k=(size_t)(h+2);phases[k]+=f/sr;phases[k]-=std::floor(phases[k]);modalAmp[k]*=modalRate[(size_t)h];tonal+=std::sin(2*davoe::pi*phases[k])*modalAmp[k]*weights[h];}
  float attack=std::exp(-seconds/(.002f+p[Strike]*.007f));float snap=filtered*attack*(.12f+p[Strike]*.5f)*(.4f+.6f*vel);float texture=filtered*noiseAmp;
  if(p[Motion]>.5f){float rub=.15f+.85f*std::pow(.5f+.5f*std::sin(2*davoe::pi*seconds*(28+p[Motion]*75)),3.f);x=texture*rub*1.5f+tonal*(1-p[Air])*.3f;}
  else if(p[Air]>.92f){float burst=std::exp(-seconds/.009f);for(int j=1;j<4;j++)if(seconds>j*.009f)burst+=.7f*std::exp(-(seconds-j*.009f)/.009f);x=texture*(burst+noiseAmp*.25f)*1.2f;}
  else{x=tonal*(1-p[Air]*.8f)*(.7f+p[Body]*.45f)+snap+texture*p[Air]*.35f;}
 }

 if(model!=3&&model!=5&&model!=6){phases[0]+=dt;phases[0]-=std::floor(phases[0]);phases[1]+=std::min(.45f,dt*detuneRatio);phases[1]-=std::floor(phases[1]);}
 if(model!=6)xr=x;float bodyHz=400+p[Body]*3300;float body=bodyFilter.tick(x),rightBody=bodyFilterR.tick(xr);if(model!=6){x=x*(1-p[Material]*.2f)+body*p[Body]*.4f;xr=xr*(1-p[Material]*.2f)+rightBody*p[Body]*.4f;}float cutoff=p[FilterCutoff];x=toneFilter.tick(x);xr=toneFilterR.tick(xr);float level=amp.tick(p[Attack],p[Decay],p[Sustain],sr);float gain=level*vel*(model==6?.8f:(model==5?.58f:.32f))*(1-p[Motion]*.12f*(.5f+.5f*std::sin(seconds*13)));x*=gain;lastRight=xr*gain;if(level<.00001f&&amp.stage==davoe::Envelope::Sustain)amp.reset();return x;
 }
};
class Engine{
 float sr=48000;Values p=defaults();std::array<Voice,8>voices{};std::array<float,16>bend{},mod{},pressure{};std::array<bool,16>pedal{};std::array<std::array<bool,128>,16>down{};Effects fx;uint64_t serial=0;int quiet=0;bool awake=false;std::shared_ptr<const SampleData> loadedSample;
public:
 void prepare(double rate){sr=(float)rate;for(auto&v:voices)v.prepare(sr);fx.prepare(sr);reset();}
 void parameters(const Values&values){for(size_t i=0;i<p.size();i++)p[i]=std::isfinite(values[i])?davoe::clamp(values[i],specs[i].lo,specs[i].hi):specs[i].initial;fx.update(p);for(auto&v:voices)if(v.active())v.configure(p);}
 void reset(){for(auto&v:voices)v.reset();for(auto&c:down)c.fill(false);pedal.fill(false);bend.fill(0);mod.fill(0);pressure.fill(0);fx.reset();awake=false;quiet=0;}
 int activeVoices()const{int n=0;for(auto&v:voices)if(v.active())n++;return n;}bool active()const{return awake;}
 void setSample(std::shared_ptr<const SampleData> data){loadedSample=std::move(data);}
 void noteOn(int note,float vel,int ch=0){if(note<0||note>127||ch<0||ch>15)return;if(vel<=0){noteOff(note,ch);return;}down[(size_t)ch][(size_t)note]=true;Voice* voice=nullptr;for(auto&v:voices)if(!v.active()){voice=&v;break;}if(!voice)voice=&*std::min_element(voices.begin(),voices.end(),[](const Voice&a,const Voice&b){return a.ageId<b.ageId;});voice->on(note,ch,vel,++serial,p,loadedSample);awake=true;quiet=0;}
 void noteOff(int note,int ch=0){if(note<0||note>127||ch<0||ch>15)return;down[(size_t)ch][(size_t)note]=false;if(pedal[(size_t)ch])return;for(auto&v:voices)if(v.note==note&&v.channel==ch)v.off(p);}
 void controller(int n,int value,int ch=0){if(ch<0||ch>15)return;if(n==1)mod[(size_t)ch]=value/127.f;if(n==64){bool old=pedal[(size_t)ch];pedal[(size_t)ch]=value>=64;if(old&&value<64)for(auto&v:voices)if(v.channel==ch&&v.note>=0&&!down[(size_t)ch][(size_t)v.note])v.off(p);}if(n==120||n==123)reset();}
 void pitchWheel(int value,int ch=0){if(ch>=0&&ch<16)bend[(size_t)ch]=(value-8192)/8192.f*p[BendRange];}
 void aftertouch(float value,int ch=0){if(ch>=0&&ch<16)pressure[(size_t)ch]=value;}
 void sample(float&l,float&r){l=r=0;if(!awake)return;bool any=false;for(auto&v:voices){if(!v.active())continue;any=true;float x=v.sample(p,bend[(size_t)v.channel],std::max(mod[(size_t)v.channel],pressure[(size_t)v.channel]));l+=x*std::sqrt(.5f-v.pan*.8f);r+=v.lastRight*std::sqrt(.5f+v.pan*.8f);}fx.sample(l,r);if(!std::isfinite(l)||!std::isfinite(r)){reset();l=r=0;return;}if(!any&&std::max(std::abs(l),std::abs(r))<.00001f){quiet++;if(quiet>(int)(sr*(p[EchoTime]*.001f+.15f))){reset();}}else quiet=0;}
};
}
