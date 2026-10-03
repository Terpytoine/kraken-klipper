#define PluginName "KRAKEN KLIPPER"
#define PluginVersion "1.0.0"
#define PluginPublisher "TODB Audio"
#define PluginBundle "build\KrakenKlipper_artefacts\Release\VST3\KRAKEN KLIPPER.vst3"

[Setup]
AppId={{9D2E6555-2122-4B66-86B5-41B2E4D2CB5E}
AppName={#PluginName}
AppVersion={#PluginVersion}
AppPublisher={#PluginPublisher}
DefaultDirName={autopf}\TODB\KRAKEN KLIPPER
DefaultGroupName={#PluginName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
OutputDir=dist
OutputBaseFilename=KRAKEN-KLIPPER-Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#PluginName} VST3
UninstallDisplayIcon={commoncf}\VST3\KRAKEN KLIPPER.vst3\Contents\x86_64-win\KRAKEN KLIPPER.vst3
CloseApplications=no
DisableWelcomePage=no

[Files]
Source: "{#PluginBundle}\*"; DestDir: "{commoncf}\VST3\KRAKEN KLIPPER.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "THIRD_PARTY_NOTICES.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "dist\Kraken-Klipper-Source.zip"; DestDir: "{app}\Source"; Flags: ignoreversion

[Icons]
Name: "{group}\Uninstall {#PluginName}"; Filename: "{uninstallexe}"

[Messages]
WelcomeLabel2=This will install {#PluginName} into the standard Windows VST3 folder. Close your DAW before continuing, then rescan plugins after setup finishes.
