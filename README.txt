DOUBLE CUP CLIPPER  •  TODB
Twon On Da Beat  |  The Yay Area  |  Hella Sauce
============================================================

A free Windows x64 VST3 clipper with a smooth adjustable knee, distinct Soft,
Medium, and Hard clipping, 8x oversampling, parallel mix, output trim, Delta
audition, live meters, a live transfer curve, quick-start presets, and save/load
for your own .dcpreset files.

The interface is a gold and purple double-cup syrup shell, with a detailed Bay Bridge,
graffiti title artwork, miniature cup knobs, and the supplied TODB logo. Each cup's syrup fill and drip length follows its knob position. Brief ripples and droplets respond to control changes on the UI thread; these visual effects do not change the audio. “Yadadamean?” is Bay Area slang
for “do you know what I mean?” The project is independent and is not affiliated
with another audio company, sports team, or brand.

QUICK START
-----------
1. Pick a starting point: 808 Weight for bass, Drum Punch for drums,
   Medium Punch for general use, or Bus Glue for a subtle blend.
2. Slowly raise DRIVE. More Drive means more clipping and a denser sound.
3. Set CEILING to choose the level where the clipper starts holding the signal down.
4. Turn KNEE up for a smoother transition; turn it down for a sharper one.
   Hard mode ignores Knee.
5. MIX blends the original and clipped sound. Use less than 100% to keep more
   of the original transients.
6. Set OUTPUT for final loudness. For a fair before/after check, match the
   output loudness to the bypassed sound and watch the OUTPUT meter.
7. Use SAVE PRESET to keep your settings in a .dcpreset file. LOAD PRESET opens
   a preset you saved or received from someone else. Preset files can be shared.

The curve is a picture of the signal: read input level along the bottom and
output level up the side. The dashed diagonal is the signal without clipping.
The bright line is the sound after processing. A flatter top means stronger
clipping. Bypass helps compare the original and processed sound. Delta lets
you hear only the difference. Hover over controls and displays for more help.

CONTROLS
--------
Drive      Raises the input before clipping: -24 to +24 dB.
Ceiling    Sets the clipper's level target: -24 to 0 dBFS.
Knee       Smoothness of the transition: 0 dB is sharp; 24 dB is wide.
Soft       The widest, smoothest transition. Knee has its strongest effect here.
Medium     A tighter transition for a firmer sound.
Hard       Flat-top clipping. The Knee control does not change Hard mode.
Mix        Blends the original input with the clipped signal: 0% is dry;
           100% is fully processed.
Output     Final level trim after the clipper: -24 to +12 dB.
Bypass     Fades between the processed sound and the latency-aligned input.
Delta      Lets you hear the difference between the processed and dry signal.
Presets    808 Weight, Drum Punch, Medium Punch, and Bus Glue are starting
           points. Choose one, then adjust any control to taste. SAVE PRESET
           and LOAD PRESET share complete settings in .dcpreset files.

User presets default to:
Documents\TODB\Double Cup Clipper\Presets

The clip shape runs at 8x the host sample rate to reduce aliasing. The Ceiling
is the clip threshold; Output trim can intentionally raise the final level above
that threshold. The meters show sample peaks in dBFS, including levels above
0 dBFS. This is not a true-peak limiter and cannot prevent inter-sample peaks
after later processing or lossy encoding.

INSTALL
-------
1. Close your DAW.
2. Run DOUBLE-CUP-CLIPPER-Setup-v1.0.3.exe and approve the standard administrator prompt.
3. Reopen your DAW, rescan VST3 plug-ins, and load DOUBLE CUP CLIPPER as an effect.

The installer places the VST3 here:
C:\Program Files\Common Files\VST3\TODB\DOUBLE CUP CLIPPER.vst3

Installer support files go here:
C:\Program Files\TODB\Double Cup Clipper

Setup removes the earlier direct-install bundle and the old Kraken bundle so
your DAW does not scan duplicate copies.

No self-updater is included. To share the plug-in, send friends the installer
from the project's GitHub Releases page:
https://github.com/Terpytoine/kraken-klipper/releases/latest

Windows may show a SmartScreen unknown-publisher warning because this free
release is not digitally signed. Download it only from the project Releases page.

BUILD ON WINDOWS
----------------
Requirements:
  * 64-bit Windows
  * Visual Studio 2022 Build Tools with the Desktop development with C++ workload
    (MSVC v143, Windows SDK, and C++ CMake tools for Windows)
  * CMake 3.22 or newer
  * Inno Setup 6 to create the one-file installer
  * Internet access on first build so CMake can fetch JUCE 9.0.3

Open PowerShell in this folder and run:

  .\build-windows.ps1

This builds the VST3, runs the transfer-curve and processor smoke checks, and
creates DOUBLE-CUP-CLIPPER-Setup-v1.0.3.exe plus a matching source archive in
.\dist\.

BUILD AND VERIFICATION
----------------------
The Windows CI build checks the clip curve for its ceiling, monotonicity,
symmetry, knee continuity, unity below the knee, and non-finite input safety.
Processor smoke checks exercise mono and stereo layouts, all three clip
characters, oversized host blocks, one-sample blocks, and non-finite inputs.
These checks do not replace loading the plug-in in every DAW or level-matched
listening tests on real sessions and material.

This plug-in is an original project with an adjustable knee, 8x oversampling,
Mix, Output, Delta, a live transfer curve, peak and reduction readouts, and
starting presets. Feature lists cannot prove sound quality; comparisons require
level-matched listening on the same material.

LICENSING
---------
This project uses JUCE 9.0.3 through CMake FetchContent and is shared under
AGPLv3-or-later. Read THIRD_PARTY_NOTICES.md before building or redistributing.

