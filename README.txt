DOUBLE CUP CLIPPER
Bay Area Plugins #001 | Twon On Da Beat
============================================================

A free Windows x64 VST3 clipper built around a smooth adjustable knee,
three clipping characters, 8x oversampling, parallel mix, output trim,
Delta audition, live meters, a live transfer curve, and quick-start presets.

The original interface art nods to the San Francisco Bay Area, the Bay Bridge,
lightning, and a double-cup mark. The project is an independent release and is
not affiliated with any other audio company, sports team, or brand.

QUICK START
-----------
1. Choose a preset, or start with MEDIUM.
2. Raise DRIVE until the sound gets the weight you want.
3. Use KNEE to make the transition into clipping smoother or sharper.
4. Lower MIX if you want some of the original sound back.
5. Use OUTPUT to set the final level, then watch the OUTPUT meter.

The curve is a picture of the signal: read input level along the bottom and
output level up the side. The dashed diagonal is the signal without clipping.
The purple line is the current clipped sound. A flatter top means stronger
clipping. Hover over the curve or controls for more help.

CONTROLS
--------
Drive      Input gain before clipping: -24 to +24 dB.
Ceiling    The level where the clipper reaches its ceiling: -24 to 0 dBFS.
Knee       Width of the smooth transition: 0 dB is sharp; 24 dB is wide.
Soft       The widest, smoothest transition. Knee has its strongest effect here.
Medium     A tighter transition for a firmer sound.
Hard       Flat-top clipping. The Knee control does not change Hard mode.
Mix        Blends the original input with the clipped signal: 0% to 100%.
Output     Final level trim after the clipper: -24 to +12 dB.
Bypass     Fades between the processed sound and the latency-aligned input.
Delta      Lets you hear the difference between the processed and dry signal.
Presets    Load a starting point; adjust any control afterward.

The clip shape runs at 8x the host sample rate to reduce aliasing. The Ceiling
is the clip threshold; Output trim can intentionally raise the final level above
that threshold. The meters show sample peaks in dBFS, including levels above
0 dBFS. This is not a true-peak limiter and cannot prevent inter-sample peaks
after later processing or lossy encoding.

INSTALL
-------
1. Close your DAW.
2. Run DOUBLE-CUP-CLIPPER-Setup.exe and approve the standard administrator prompt.
3. Reopen your DAW, rescan VST3 plug-ins, and load DOUBLE CUP CLIPPER as an effect.

The installer places the VST3 in the shared Windows folder:
C:\Program Files\Common Files\VST3

When replacing the earlier Kraken Klipper build, Setup removes its old VST3
folder so your DAW does not scan both names as separate plug-ins.

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
creates DOUBLE-CUP-CLIPPER-Setup.exe plus a matching source archive in .\dist\.

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
