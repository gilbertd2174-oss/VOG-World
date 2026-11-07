#pragma once
#include "Parameters.h"
namespace world {
enum Param{Model,Brightness,Body,Material,Air,Strike,Attack,Decay,Sustain,Release,VibratoRate,VibratoDepth,Motion,Detune,
 Spread,Chorus,EchoMix,EchoTime,Feedback,Reverb,Room,PreDelay,Volume,Tune,Transpose,BendRange,PadNote,PadGate,FilterCutoff,FilterResonance,Source,SampleRoot,SampleLoop,Count};
struct Spec{const char*id;const char*name;float lo,hi,initial,step,skew;const char*choices;};
inline constexpr std::array<Spec,Count> specs{{
 {"model","Sound model",0,5,0,1,1,"Plucked string|Wind / flute|Bowed string|Bell / mallet|Reed|Drum / texture"},
 {"bright","Brightness",0,1,.55f,.001f,1,""},{"body","Body",0,1,.45f,.001f,1,""},
 {"material","Material",0,1,.3f,.001f,1,""},{"air","Air / noise",0,1,.12f,.001f,1,""},
 {"strike","Strike",0,1,.6f,.001f,1,""},{"attack","Attack ms",1,2000,3,1,.35f,""},
 {"decay","Decay ms",40,10000,1800,1,.35f,""},{"sustain","Sustain",0,1,0,.001f,1,""},
 {"release","Release ms",10,6000,600,1,.4f,""},{"vrate","Vibrato Hz",.1f,12,5,.01f,.5f,""},
 {"vdepth","Vibrato cents",0,100,3,.1f,1,""},{"motion","Motion",0,1,.1f,.001f,1,""},
 {"detune","Detune cents",0,30,1,.1f,1,""},{"spread","Stereo width",0,1,.6f,.001f,1,""},
 {"chorus","Chorus",0,1,.08f,.001f,1,""},{"echo","Echo mix",0,.65f,.08f,.001f,1,""},
 {"etime","Echo ms",40,1000,320,1,.6f,""},{"feedback","Feedback",0,.8f,.28f,.001f,1,""},
 {"reverb","Reverb mix",0,.65f,.18f,.001f,1,""},{"room","Room size",0,1,.5f,.001f,1,""},
 {"predelay","Pre-delay ms",0,150,15,1,1,""},{"volume","Output dB",-36,0,-7,.1f,1,""},
 {"tune","Fine tune",-100,100,0,.1f,1,""},{"transpose","Transpose",-24,24,0,1,1,""},
 {"bend","Bend range",1,24,2,1,1,""},
 {"padnote","Pad MIDI note",24,96,60,1,1,""},{"padgate","Pad hold ms",40,30000,350,1,.5f,""},
 {"cutoff","Filter cutoff Hz",30,18000,9000,1,.3f,""},{"resonance","Resonance",0,.95f,.1f,.001f,1,""},
 {"source","Pad source",0,1,0,1,1,"Synthesized|Loaded sample"},{"sampleroot","Sample root",0,127,60,1,1,""},{"sampleloop","Sample loop",0,1,0,1,1,"Off|On"}
}};
using Values=std::array<float,Count>;
inline Values defaults(){Values v{};for(size_t i=0;i<v.size();i++)v[i]=specs[i].initial;return v;}
struct Patch{const char* name;const char* instrument;int region;int family;Values values;};
inline const char*const regions[]={"Asia","Middle East","Africa","Spain","Caribbean"};
inline const char*const families[]={"Plucked strings","Winds / flutes","Bowed strings","Bells / mallets","Reeds","Drums / textures"};
}
