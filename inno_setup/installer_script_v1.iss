#define MyAppName "MocoTrackQt"
#define MyAppVersion "v1"
#define MyAppPublisher "WIS"
#define MyAppURL "http://www.animalsimulation.org"
#define MyAppExeName "MocoTrackQt.exe"

#define QtBuildFolder "Desktop_Qt_6_8_2_MSVC2022_64bit-Release"

; this script assumes that both the command line and the GUI version are built using QtCreator with the default build paths

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
AppId={{4a8228e6-4dc9-40ab-a54c-ee0c24ed7668}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=..\LICENSE
OutputDir=inno_setup
OutputBaseFilename=MocoTrackQt_v1_Setup
Compression=lzma
SolidCompression=yes
PrivilegesRequiredOverridesAllowed=commandline dialog
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
ChangesAssociations = yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\build\{#QtBuildFolder}\MocoTrackQt.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\{#QtBuildFolder}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\{#QtBuildFolder}\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\{#QtBuildFolder}\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\{#QtBuildFolder}\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\{#QtBuildFolder}\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\{#QtBuildFolder}\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\{#QtBuildFolder}\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\{#QtBuildFolder}\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\{#QtBuildFolder}\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs

Source: "..\command_line\build\{#QtBuildFolder}\mocotrack.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\command_line\build\{#QtBuildFolder}\*.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\build\{#QtBuildFolder}\vc_redist.x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: {tmp}\vc_redist.x64.exe; Parameters: "/q /passive /Q:a /c:""msiexec /q /i vcredist.msi"""; StatusMsg: "Installing VC++ 20xx Redistributables..."
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

