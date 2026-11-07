# VOG World

A self-contained Windows x64 VST3 instrument and standalone application: 16 independent instrument/sampler pads, a playable keyboard, per-pad effects, autoplay rhythms, recording and portable kit saving.

## Factory instruments
160 synthesized interpretations: 40 instrument inspirations × Natural, Soft, Bright and Space variations. Each of the five regional collections has 32 patches.

- Asia: koto, guzheng, pipa, shamisen, sitar, bansuri, shakuhachi and erhu inspired sounds.
- Middle East: oud, qanun, saz, ney, duduk, zurna, santur and darbuka inspired sounds.
- Africa: kora, ngoni, mbira, kalimba, balafon, djembe, talking-drum and udu inspired sounds.
- Spain: classical/flamenco guitar, bandurria, laud/lute, castanet, palmas and harmonic-guitar inspired sounds.
- Caribbean: tenor/cello steelpan, marimbula, Cuban tres, bongos, congas, guiro and maracas inspired sounds.

These are original synthesized timbres, not recordings, acoustic replicas, complete performance articulations or culturally specific tuning systems. No sample pack is needed for the factory sounds. Default pitch mapping is twelve-tone equal temperament; fine tune, transpose and pitch bend are adjustable per pad. Region labels are browsing categories, not claims of exclusive origin.

## Select and edit a pad
Click a pad to select and trigger it. All 33 sound controls immediately bind to that pad. Each pad has its own patch, source, ADSR, rotary cutoff/resonance, expression, pitch, stereo spread, chorus, echo, reverb and output level. Eight voices per pad; up to 128 total voices before CPU limits.

Click a region or family to filter the list and immediately load its first matching patch onto the selected pad. Use the patch dropdown, or the < and > buttons, to load a built-in instrument onto the selected pad. Changing its factory patch does not change the other pads or their rhythms. Each pad can select any of the six synthesis models. Factory patch selection switches that pad back to Synthesized source; any previously loaded sample remains available until replaced.

The keyboard plays the selected pad chromatically. Hardware MIDI channels other than channel 10 play that pad too. On MIDI channel 10, notes 36–51 trigger pads 1–16 using each pad's Pad MIDI note and Pad hold time. Keyboard note-offs still reach the original pad if you change selection while holding notes. Sustain pedal, ± configurable pitch bend and modulation wheel are supported. Incoming MIDI is not forwarded out.

## Autoplay
PLAY / ARM PADS starts the shared 16-step clock. Click the 16 rhythm buttons beneath the pad grid to enable steps for the selected pad. All 16 pad rhythms can play together, with each pad's own pitch, hold time, sound and effects.

Tempo, division, shuffle, DAW sync and Master output are shared transport/mixer controls. Sound controls on the right are per-pad. With DAW sync enabled, arm playback and start the host transport. Internal clock uses the Tempo control. The initial kit contains a starter rhythm; pads with empty rhythm rows play only when triggered manually. PANIC stops autoplay and clears voices and effect tails.

## Recording and samples
1. Choose Record selected pad, Record full mix, or Record audio input.
2. Select the pad to record when using the selected-pad source. The recording target/source is latched when REC starts, so switching the edited pad later does not change the capture bus.
3. Press REC, play or enable autoplay, then STOP REC. Recording stops automatically at 30 seconds. Audio device/host processing must be running.
4. SAVE REC WAV writes the stopped capture as stereo 24-bit WAV.
5. USE / PLAY REC assigns the stopped capture to the currently selected pad and switches that pad to Loaded sample source.
6. LOAD SAMPLE imports WAV, AIFF, FLAC or OGG onto the selected pad. The first 30 seconds and first two channels are used; supported file rates are 8–192 kHz. Mono files are duplicated to stereo.
7. Sample root is the MIDI note that plays the original sample pitch (set to the selected pad note on import). Other notes repitch it; this is resampling, not time-stretching. Sample loop repeats the entire file. Sample ADSR, filter, pitch and effects are per-pad. Physical excitation controls describe synthesized models.
8. SAVE PAD WAV writes the selected pad's loaded/assigned sample. To render its synthesized sound or effects, record the selected pad first and use SAVE REC WAV. Effects recorded from a pad or mix are baked into the WAV; subsequent pad effects remain adjustable.

Audio-input recording requires the host to enable and route the plugin's optional Sample input bus. It does not open a microphone by itself, and input audio is not monitored to the output. In standalone use, pad/mix recording works without an input bus; availability of external-input routing depends on the wrapper/device configuration.

## Save and load
- SAVE PAD / LOAD PAD use .wpad files for one pad's complete settings, rhythm and embedded sample audio. Loading a pad file affects only the selected pad.
- SAVE KIT / LOAD KIT use .vworld files containing all 16 pads, every setting/rhythm and embedded loaded or assigned recordings. Kits do not need the original sample files afterward.
- DAW session state stores the same complete kit. Autoplay resumes stopped after kit/session loading.
- The temporary recording buffer is not a pad until USE / PLAY REC is pressed. Save the capture as WAV or assign it to a pad before closing.
- Embedded audio makes kits/pad files larger. File read/write and sample restoration run outside processBlock; loading large kits can take time.

## Build on GitHub
Use a NEW repository for VOG World.
1. Upload VOG-World-v1.1.0-GitHub-Ready.zip intact to the root and commit.
2. Create .github/workflows/main.yml and paste ALL text from VOG-World-v1.1.0-main.yml; commit.
3. Actions → VOG World Windows Build. The workflow extracts the ZIP, builds VST3 and standalone and runs engine tests.
4. After a green build, download VOG-World-Windows-x64 under Artifacts. Extract it and any inner ZIP. Follow INSTALL.md.

Alternatively extract this source ZIP and upload ALL files/folders, including .github, Source, Tests and scripts.

Local build: Visual Studio 2022 Desktop development with C++, Windows SDK, CMake 3.24+ and Git for Windows. From the extracted project:

    ./scripts/build-windows.ps1 -Configuration Release -Parallel 2

First configure fetches JUCE 8.0.6. Output: dist/VOG-World-Windows-x64.zip.
Engine-only tests: configure CMake with -DDAVOE_ENGINE_ONLY=ON -DBUILD_TESTING=ON, build and run ctest.

See VALIDATION.md for what has actually been checked. This download is source code, not a compiled VST3.

## MPD218 and other pad controllers
Choose **Pad controller / MPD218** in MIDI routing. This accepts mapped pads on any MIDI channel and uses strike velocity. Default mapping is notes 36–51; unassigned notes are ignored rather than playing the selected instrument at unexpected pitches. For a different hardware preset, select a VOG pad, click **LEARN PAD**, then strike its matching hardware pad. Repeat for all 16 pads. Mapping is stored in the kit/DAW state. RESET MAP restores notes 36–51. Hardware pad hits use the pad hold time, so very short note-off messages do not cut sounds off immediately. The live MIDI note/channel display helps diagnose host routing.

For a regular keyboard, keep Keyboard + ch10 pads. MIDI Learn consumes and auditions its learning strike. Learning a note already used by another VOG pad unassigns the previous mapping; each note maps to one pad. The on-screen keyboard follows the same routing setting; use keyboard mode to play it chromatically.

## Recording and sampling in Reason 14
Use **Pad controller / MPD218**: Reason forwards VST instruments on channel 1, so the old channel-10-only pad path played the selected sound at MIDI note pitches instead. Source: https://docs.reasonstudios.com/reason14/working-with-vst-plugins

To capture VOG itself, leave **Record full mix** selected, click REC, play your pads, click STOP REC, then **USE / PLAY REC** to audition the capture on the selected pad. **SAVE REC WAV** exports directly. This records VOG audio, not the whole Reason song or Reason’s MIDI track. For external audio, choose Record audio input and route an audio source into VOG’s exposed input jacks on Reason’s rear rack panel. If no input bus is available, use Reason to record/bounce WAV, then LOAD SAMPLE. Host input compatibility has not been tested here.

LOAD SAMPLE should open a Windows file picker; choose WAV, AIFF, FLAC or OGG. A successful load displays its name, switches the pad source and auditions the audio. Samples now load at their original pitch on the chosen pad and hold for their duration (up to 30 seconds). Use the pad’s Sample root, Pad hold and ADSR controls to change this. If no file picker opens, report that separately along with the build version.

For testing these changes, insert a fresh VOG World instance or reload a factory drum patch: existing Reason songs preserve old patch/effect settings. The percussion remains synthesized; it is not an acoustic sample library.

You can also drag a WAV/AIFF/FLAC/OGG file from Windows Explorer onto the open VOG World editor to load and audition it on the selected pad. This provides an alternative to the file picker.
