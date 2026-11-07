cmake_minimum_required(VERSION 3.24)
project(VOGWorld VERSION 1.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
if(MSVC)
 add_compile_options(/bigobj /utf-8)
endif()
if(BUILD_TESTING)
 add_executable(DavoeEngineTests Tests/EngineTests.cpp)
 target_include_directories(DavoeEngineTests PRIVATE Source)
 add_test(NAME SynthEngine COMMAND DavoeEngineTests)
endif()
option(DAVOE_ENGINE_ONLY "Build engine tests without JUCE" OFF)
if(DAVOE_ENGINE_ONLY)
 return()
endif()
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(VOGWorld
 COMPANY_NAME "Davoe Audio"
 BUNDLE_ID "audio.davoe.world"
 PLUGIN_MANUFACTURER_CODE Dvoe
 PLUGIN_CODE WVo1
 FORMATS VST3 Standalone
 PRODUCT_NAME "VOG World"
 DESCRIPTION "World-inspired instrument synth, sixteen-pad sampler and recorder"
 IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS TRUE
 COPY_PLUGIN_AFTER_BUILD FALSE
 VST3_CATEGORIES Instrument Synth)
target_sources(VOGWorld PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(VOGWorld PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(VOGWorld PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
