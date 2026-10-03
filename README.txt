KRAKEN KLIPPER — TODB
=====================

An original, stereo/mono VST3 soft clipper with a continuously adjustable
soft-knee width, three clip characters, 8x oversampling, parallel blend,
Delta audition, meters, a live transfer curve, and starting-point presets.

The sound and interface are an original TODB/Bay Area design. They are not
affiliated with BNO Audio, the NBA, or any other brand.

CONTROLS
--------
Drive      Pushes the signal into the clip curve: -24 to +24 dB.
Ceiling    Sets the final sample peak limit: 0 to -24 dBFS while active.
Knee       Sets the soft transition width: 0 to 24 dB.
Soft       Wide, gradual transition into the ceiling.
Medium     A tighter knee for a firmer clip.
Hard       Hard clipping at the selected ceiling.
Mix        Blends the dry input and clipped signal.
Output     Output trim. The selected ceiling remains the active peak limit.
Delta      Auditions the difference between input and processed signal.
Bypass     Crossfades to the unprocessed signal.

The clipper runs its curve at 8x the host sample rate and then downsamples.
The final sample-peak guard catches overshoot from reconstruction. As with
other sampled audio processors, this is not a guarantee of an inter-sample
true-peak ceiling after lossy encoding or a later gain stage.

BUILD ON WINDOWS
----------------
Requirements:
  * 64-bit Windows
  * Visual Studio 2022 Build Tools with the Desktop development with C++ workload
    (MSVC v143, Windows SDK, and C++ CMake tools for Windows)
  * CMake 3.22 or newer
  * Inno Setup 6 to make the single-file Setup.exe
  * Internet access for CMake to fetch JUCE 9.0.3 on the first configure

Open PowerShell in this folder and run:

  .\build-windows.ps1

This builds and checks the plugin, then creates a single installer and its
matching source archive in .\dist\ . Double-click KRAKEN-KLIPPER-Setup.exe,
follow the installer, approve the Windows administrator prompt, and restart
or rescan plugins in your DAW. The installer places the plugin in:

  C:\Program Files\Common Files\VST3

To share it, send the Setup.exe. The installer also places the matching source
archive and license notices alongside its program files.

DOWNLOAD AND INSTALL
--------------------
Download KRAKEN-KLIPPER-Setup.exe from the latest release:

  https://github.com/Terpytoine/kraken-klipper/releases/latest

Close your DAW, run the installer, approve the standard Windows administrator
prompt, and let it install the VST3 bundle into the shared Windows VST3 folder.
Then restart or rescan plugins in your DAW and load KRAKEN KLIPPER as an effect.
The installer is not digitally signed, so Windows SmartScreen may show an
unknown-publisher warning. That is a reputation/signing warning, not proof of
malware; only download the installer from this project's Releases page.

BUILD AND VERIFICATION
----------------------
The Windows GitHub Actions workflow builds the x64 VST3, runs the transfer-curve
checks, and packages the installer. The first successful workflow run completed
all three stages. The curve checks cover ceiling, monotonicity, symmetry, knee
continuity, unity below the knee, and non-finite input safety.

The binary has been compiled and its curve checks passed, but it has not yet
been independently loaded in a DAW on multiple systems. Please report host or
installer issues in the repository's Issues page.

TESTS
-----
The CMake build runs transfer-curve checks for ceiling, monotonicity,
symmetry, knee continuity, unity below the knee, and non-finite input safety.
The current Windows VST3 build passed these curve checks in GitHub Actions.
The curve checks validate the shaping math, not every DAW/host combination or
subjective audio quality.

COMPARISON TARGET
-----------------
The control layout covers the central jobs of BNO CLIP-1 (drive, ceiling,
and soft/medium/hard character). Kraken Klipper adds an adjustable knee,
8x oversampling, Mix, Output, Delta, a live curve, and input/output/clip
readouts. Feature count cannot establish that one plugin sounds better;
that takes level-matched listening tests on real material.

LICENSING
---------
This project uses JUCE 9.0.3 through CMake FetchContent and is shared under
AGPLv3-or-later. Read THIRD_PARTY_NOTICES.md before building or sharing it.

Design: TODB / TWON ON DA BEAT
