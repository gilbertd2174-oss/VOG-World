#pragma once
#include "PluginProcessor.h"
class WorldTheme:public juce::LookAndFeel_V4{public:WorldTheme();juce::Font getTextButtonFont(juce::TextButton&,int)override{return juce::Font(14.f,juce::Font::bold); }void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool)override;void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;};
struct WorldControl:juce::Component{juce::Label label;juce::Slider knob;juce::ComboBox combo;world::Param parameter;std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>sa;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>ca;WorldControl(juce::AudioProcessorValueTreeState&,int,world::Param);void bind(juce::AudioProcessorValueTreeState&,int);void resized()override;};
struct WorldSurface:juce::Component{void paint(juce::Graphics&)override;private:juce::Image texture;};
struct WorldPad:juce::Component{WorldProcessor&processor;int index;std::function<void(int)>onSelect;WorldPad(WorldProcessor&p,int i):processor(p),index(i){}void paint(juce::Graphics&)override;void mouseDown(const juce::MouseEvent&)override;};
struct WorldWaveform:juce::Component{WorldProcessor&processor;explicit WorldWaveform(WorldProcessor&p):processor(p){}void paint(juce::Graphics&)override;};
class WorldEditor final:public juce::AudioProcessorEditor,public juce::FileDragAndDropTarget,private juce::Timer{
public:bool isInterestedInFileDrag(const juce::StringArray&)override;void filesDropped(const juce::StringArray&,int,int)override;explicit WorldEditor(WorldProcessor&);~WorldEditor()override;void paint(juce::Graphics&)override;void resized()override;
private:WorldProcessor&processor;WorldTheme theme;juce::Viewport viewport;WorldSurface body;WorldWaveform waveform;juce::MidiKeyboardComponent keyboard;
 std::array<std::unique_ptr<WorldPad>,16>pads;std::array<std::unique_ptr<WorldControl>,world::Count>controls;
 juce::ComboBox patches,family,division,recordSource,midiRouting;std::array<juce::TextButton,6>regions;std::array<juce::TextButton,16>steps;
 juce::TextButton learnMidi{"LEARN PAD"},resetMidi{"RESET MAP"};juce::TextButton previous{"<"},next{">"},savePad{"SAVE PAD"},loadPad{"LOAD PAD"},saveKit{"SAVE KIT"},loadKit{"LOAD KIT"},panic{"PANIC"},autoplay{"PLAY PADS"},rec{"REC"},stopRec{"STOP REC"},loadSample{"LOAD SAMPLE"},useRec{"USE / PLAY REC"},savePadWav{"SAVE PAD WAV"},saveRecWav{"SAVE REC WAV"};juce::ToggleButton host{"DAW sync"};juce::Slider tempo,shuffle,master;juce::Label editing,status,tempoLabel,shuffleLabel,masterLabel;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>tempoAttachment,shuffleAttachment,masterAttachment;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>divisionAttachment,midiAttachment;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>hostAttachment;
 std::unique_ptr<juce::FileChooser>chooser;juce::TooltipWindow tips{this,600};int selected=-1,program=-1,region=-1;std::vector<int>visiblePatches;juce::String notice;int noticeTicks=0;
 void timerCallback()override;void rebuildPatches(bool chooseFirst=false);void browse(int);void result(const juce::Result&);void openSaveWav(bool);void select(int);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorldEditor)
};
