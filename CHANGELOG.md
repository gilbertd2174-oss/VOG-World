# 1.0.0
Initial VOG World: 160 synthesized world-instrument patches, sixteen independent synth/sample pads, per-pad ADSR/rotary filter/FX, autoplay rhythms, keyboard, 30-second recording, WAV import/export and embedded-audio pad/kit saving.

## 1.1.0
- Sculpted knobs with machined collars, grip details and cast shadows; raised rubber pads and beveled buttons; inset metal control panels and screws.
- Added any-channel pad-controller routing, per-pad MIDI Learn, mapping reset and MIDI note/channel monitor.
- Physical pad strikes now preserve MIDI velocity rather than using a fixed level.
- Hardware MPD218, MIDI Learn and Windows compile checks remain untested locally.

- Rebuilt the drum model with five damped membrane modes, velocity-sensitive hand attack, filtered noise, clap bursts and shaker/scrape textures. Dry percussion defaults remove chorus/delay smear; all 40 percussion variations updated.
- Sample loading now assigns the selected pad note as its root and a duration-based pad hold, preventing low-pitched/time-stretched playback and premature release. Loads audition immediately.
- Recording defaults to full mix; elapsed seconds and stopped-recording instructions are visible. USE / PLAY REC assigns and auditions the capture.
- Larger pad numbers and sequencer button text, including 10–16.

- Added audio-file drag and drop onto the selected pad as an alternative to the sample file picker.

- Region/family buttons now load the first matching patch onto the selected pad, with a status confirmation, rather than silently filtering the list. Empty intersections display a clear message.
