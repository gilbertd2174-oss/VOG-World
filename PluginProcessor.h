#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "WorldEngine.h"
#include "WorldPresets.h"
class WorldProcessor final:public juce::AudioProcessor,private juce::AsyncUpdater{
public:
 WorldProcessor();~WorldProcessor()override{cancelPendingUpdate();}
 void prepareToPlay(double,int)override;void releaseResources()override{};bool isBusesLayoutSupported(const BusesLayout&)const override;void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 juce::AudioProcessorEditor*createEditor()override;bool hasEditor()const override{return true;}const juce::String getName()const override{return "VOG World";}bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}double getTailLengthSeconds()const override{return 30;}
 int getNumPrograms()override{return (int)world::patches.size();}int getCurrentProgram()override{return programs[(size_t)selectedPad.load()].load();}void setCurrentProgram(int)override;const juce::String getProgramName(int i)override{return i>=0&&i<getNumPrograms()?juce::String::fromUTF8(world::patches[(size_t)i].name):juce::String();}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 juce::AudioProcessorValueTreeState state;juce::MidiKeyboardState keyboard;std::array<std::atomic<int>,16>padMidiNotes{};std::atomic<int>learnPad{-1},lastMidiNote{-1},lastMidiChannel{0};
 std::atomic<int>selectedPad{0},playStep{-1},recordStatus{0},recordCommand{0},recordSource{0},recordedFrames{0},recordPad{0};std::atomic<bool>panic{false},loading{false};std::atomic<float>peak{0},bpm{120};std::array<std::atomic<float>,16>padPeaks{};std::array<std::atomic<uint32_t>,16>triggers{},rhythms{};
 static juce::String id(int pad,world::Param p){return "pad"+juce::String(pad+1)+"_"+world::specs[(size_t)p].id;}
 juce::String padTitle(int pad)const{return juce::String::fromUTF8(world::patches[(size_t)programs[(size_t)pad].load()].instrument);}
 void getPadStateInformation(int,juce::MemoryBlock&);juce::Result setPadStateInformation(int,const void*,int);
 void selectPad(int);void loadPatch(int,int);void setPadValue(int,world::Param,float);void setGlobal(const char*,float);void cleanupSamples();
 juce::String sampleName(int)const;juce::Result loadSample(int,const juce::File&);juce::Result useRecording(int);juce::Result saveWav(const juce::File&,bool recording,int pad);double recordingSeconds()const;std::shared_ptr<const world::SampleData>sampleData(int pad)const{return std::atomic_load(&samples[(size_t)pad]);}
private:
 std::array<std::array<std::atomic<float>*,world::Count>,16>raw{};std::array<std::atomic<int>,16>programs{};std::array<std::atomic<bool>,16>resetPad{};std::array<world::Engine,16>engines{};
 std::array<std::shared_ptr<const world::SampleData>,16>samples{};std::vector<std::shared_ptr<const world::SampleData>>retired;mutable juce::CriticalSection metaLock;std::array<juce::String,16>sampleNames{};
 juce::AudioFormatManager formats;juce::AudioBuffer<float>capture;double sr=48000;std::atomic<double>captureRate{48000};int capturePosition=0,captureSource=0,capturePad=0;
 std::array<int,16>gateTimers{},gateNotes{};std::array<std::array<int,128>,16>keyPad{};double internalPpq=0;int64_t lastStep=INT64_MIN;bool wasAuto=false;
 juce::CriticalSection restoreLock;juce::ValueTree pendingRestore;
 std::atomic<float>*autoOn=nullptr,*tempo=nullptr,*division=nullptr,*shuffle=nullptr,*hostSync=nullptr,*master=nullptr,*midiMode=nullptr;
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();world::Values values(int)const;void fire(int,const world::Values&,float velocity=.9f);std::shared_ptr<const world::SampleData>recordingCopy()const;void assignSample(int,std::shared_ptr<const world::SampleData>,juce::String);void handleAsyncUpdate()override;juce::ValueTree padNode(int)const;void restorePadNode(int,const juce::ValueTree&);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorldProcessor)
};
