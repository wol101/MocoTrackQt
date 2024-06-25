#define MyAppName "MocoTrackQt"
#define MyAppVersion "v1"
#define MyAppPublisher "WIS"
#define MyAppURL "http://www.animalsimulation.org"
#define MyAppExeName "MocoTrackQt.exe"

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
Source: "..\build\Desktop_Qt_6_7_1_MSVC2019_64bit-Release\MocoTrackQt.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\Desktop_Qt_6_7_1_MSVC2019_64bit-Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\Desktop_Qt_6_7_1_MSVC2019_64bit-Release\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\command_line\build\Desktop_Qt_6_7_1_MSVC2019_64bit-Release\mocotrack.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\command_line\build\Desktop_Qt_6_7_1_MSVC2019_64bit-Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

