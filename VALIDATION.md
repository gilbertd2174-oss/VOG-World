# Validation status

## Completed locally
Full project/workflow structure, parameter counts/IDs, preset values/counts, UI control coverage and source-delimiter checks. Entire GitHub Actions YAML parsed and its inline PowerShell syntax checked. Actual deliverable ZIP extracted and discovered by the workflow scripts. ZIP integrity and embedded/downloadable workflow equality checked. Interface preview rendered and visually inspected.

## Not completed here
This computer has no C++ compiler/CMake available. VOG World has NOT been compiled locally. The new C++ engine tests have NOT been executed here. No listening, latency/CPU measurements, DAW/plugin-validator testing, live recording, sample loading/export or state-roundtrip testing has been performed. The UI preview is illustrative, not a screenshot of a running plugin.

## Tests included for the Windows build
All 160 preset ranges/audio, six synthesis models, isolation between two engines, eight-voice allocation, sustain, reset, stereo sample playback, one-shot ending, looping/release, and 44.1/48/96 kHz engine operation.

## Required manual validation after building
- Audition all factory patches across useful note ranges. Synthesized interpretations are not acoustic replicas.
- Trigger all 16 pads, switch selected pads while holding MIDI keys, test sustain/bend and channel-10 mapping.
- Verify every selected-pad control changes only that pad, including ADSR and cutoff/resonance.
- Exercise autoplay tempo/divisions/shuffle, DAW start/stop/loop/seek and the per-pad rhythm rows.
- Record selected pad, mix and routed input; stop at 30 seconds; save WAV and reload it; compare stereo content.
- Save/reload complete kits and individual pads with embedded samples. Confirm restoration does not require original files, and other pads remain unchanged when loading one pad.
- Confirm sample loop/root/tuning, missing/invalid file handling and source switching.
- Run a VST3 validator and measure CPU at intended buffer sizes, especially many simultaneous bowed/bell voices and long effect tails. Check resize/scrolling at Windows display scaling used on your machine.

## 1.1.0 update
Source checks and packaging repeated after the hardware-style interface, MIDI controller routing, recording feedback, sample pitch/hold and percussion changes. The additional drum velocity/transient tests are included for Windows and remain unexecuted here. No physical MPD218 or Reason 14 session was available. The channel-1 host behavior is documented in Reason 14’s official VST manual; the new routing is intended to address that mismatch and still requires a live test.
