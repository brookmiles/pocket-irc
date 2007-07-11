; The name of the installer
Name "Code North Pocket IRC"

InstallColors 525C99 FFFFFF

; The file to write
OutFile "PocketIRC.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Code North PocketIRC"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "SOFTWARE\Code North\PocketIRC" "InstallDir"

;ComponentText "Select the components of Pocket IRC that you would like to install."

; The text to prompt the user to enter a directory
;DirText "Choose a directory to install in:"
;DirShow hide
ShowInstDetails show
ShowUninstDetails show

LicenseText "Pocket IRC Installation Notes" "Install"
LicenseData "ReadMe.txt"

!define CEAPPMGR $0

; The stuff to install
Section "Program Files (required)"
  ReadRegStr ${CEAPPMGR} HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\CEAPPMGR.EXE" ""
  StrCmp ${CEAPPMGR} "" 0 NoAbort1
    MessageBox MB_OK "Microsoft ActiveSync Application Manager was not found."
    Abort "Install failed: Microsoft ActiveSync Application Manager was not found."
  NoAbort1:

  ReadRegStr $1 HKLM "SOFTWARE\Microsoft\Windows CE Services" InstalledDir
  StrCmp $1 "" 0 NoAbort2
    MessageBox MB_OK "Microsoft ActiveSync installation directory was not found."
    Abort "Install failed: Microsoft ActiveSync installation directory was not found."
  NoAbort2:

  StrCpy $INSTDIR "$1\Code North PocketIRC"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File "PocketIRC.SA.CAB"
  ;File "PocketIRC.MIPS.CAB"
  ;File "PocketIRC.SH3.CAB"
  File "PocketIRC.ini"

  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Code North\PocketIRC" "InstallDir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketIRC" "DisplayName" "Code North Pocket IRC (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketIRC" "DisplayIcon" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketIRC" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"

  Exec '"${CEAPPMGR}" "$INSTDIR\PocketIRC.ini"'
SectionEnd

; uninstall stuff

UninstallText "This will uninstall the Pocket IRC setup files from your PC.  Click Uninstall to continue."

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PocketIRC"
  DeleteRegKey HKLM "SOFTWARE\Code North\PocketIRC"

  ; remove files
  Delete "$INSTDIR\PocketIRC.SA.CAB"
  ;Delete "$INSTDIR\PocketIRC.MIPS.CAB"
  ;Delete "$INSTDIR\PocketIRC.SH3.CAB"
  Delete "$INSTDIR\PocketIRC.ini"

  ; MUST REMOVE UNINSTALLER, too
  Delete "$INSTDIR\uninstall.exe"

  ; remove directories used.
  RMDir "$INSTDIR"
SectionEnd

; eof
