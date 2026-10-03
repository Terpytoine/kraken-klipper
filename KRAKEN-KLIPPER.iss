#define PluginName "DOUBLE CUP CLIPPER"
#define PluginVersion "1.0.3"
#define PluginPublisher "TODB"
#define PluginBundle "build\KrakenKlipper_artefacts\Release\VST3\DOUBLE CUP CLIPPER.vst3"

[Setup]
AppId={{9D2E6555-2122-4B66-86B5-41B2E4D2CB5E}
AppName={#PluginName}
AppVersion={#PluginVersion}
AppPublisher={#PluginPublisher}
DefaultDirName={autopf}\TODB\Double Cup Clipper
UsePreviousAppDir=no
DefaultGroupName={#PluginName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
OutputDir=dist
OutputBaseFilename=DOUBLE-CUP-CLIPPER-Setup-v1.0.3
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#PluginName} VST3
UninstallDisplayIcon={commoncf}\VST3\TODB\DOUBLE CUP CLIPPER.vst3\Contents\x86_64-win\DOUBLE CUP CLIPPER.vst3
CloseApplications=yes
RestartApplications=no
DisableWelcomePage=no

[Files]
Source: "{#PluginBundle}\*"; DestDir: "{commoncf}\VST3\TODB\DOUBLE CUP CLIPPER.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "THIRD_PARTY_NOTICES.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "dist\Double-Cup-Clipper-Source-v1.0.3.zip"; DestDir: "{app}\Source"; Flags: ignoreversion

[InstallDelete]
; Remove the previous VST3 bundle so a manual replacement install cannot leave two plug-ins in the host scan path.
Type: filesandordirs; Name: "{commoncf}\VST3\KRAKEN KLIPPER.vst3"
Type: filesandordirs; Name: "{commoncf}\VST3\DOUBLE CUP CLIPPER.vst3"
Type: filesandordirs; Name: "{commoncf}\VST3\TODB\DOUBLE CUP CLIPPER.vst3"

[Icons]
Name: "{group}\Uninstall {#PluginName}"; Filename: "{uninstallexe}"

[Messages]
WelcomeLabel2=This will install {#PluginName} into the shared Windows VST3 folder under TODB. Close your DAW before continuing, then rescan plugins after setup finishes.
