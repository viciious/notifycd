;NSIS Modern User Interface version 1.70
;Start Menu Folder Selection Example Script
;Written by Joost Verburg

!define PRODUCT_CLASS "NCDP"
!define VER_MAJOR "1"
!define VER_MINOR "60 beta 5"
!define PRODUCT "Notify CD Player" ;Define your own software name here
!define VERSION "${VER_MAJOR}.${VER_MINOR}" ;Define your own software version here

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;LZMA compression
  SetCompressor lzma

  ;Name and file
  Name "${PRODUCT} ${VERSION}"
  OutFile "NCDSetup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${PRODUCT}"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${PRODUCT}" ""

;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE ".\GNU"
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${PRODUCT}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT}"

  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
  
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Functions

 ; GetWindowsVersion
 ;
 ; Based on Yazno's function, http://yazno.tripod.com/powerpimpit/
 ; Returns on top of stack
 ;
 ; Windows Version (95, 98, ME, NT x.x, 2000, XP, .NET Server)
 ; or
 ; '' (Unknown Windows Version)
 ;
 ; Usage:
 ;   Call GetWindowsVersion
 ;   Pop $R0
 ;   ; at this point $R0 is "NT 4.0" or whatnot

 Function GetWindowsVersion
   Push $R0
   Push $R1
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" "CurrentVersion"
   StrCmp $R0 "" 0 lbl_winnt
   ; we are not NT.
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion" "VersionNumber"

   StrCpy $R1 $R0 1
   StrCmp $R1 '4' 0 lbl_error

   StrCpy $R1 $R0 3

   StrCmp $R1 '4.0' lbl_win32_95
   StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98

   lbl_win32_95:
     StrCpy $R0 '95'
   Goto lbl_done

   lbl_win32_98:
     StrCpy $R0 '98'
   Goto lbl_done

   lbl_win32_ME:
     StrCpy $R0 'ME'
   Goto lbl_done

   lbl_winnt:

     StrCpy $R1 $R0 1

     StrCmp $R1 '3' lbl_winnt_x
     StrCmp $R1 '4' lbl_winnt_x

     StrCpy $R1 $R0 3

     StrCmp $R1 '5.0' lbl_winnt_2000
     StrCmp $R1 '5.1' lbl_winnt_XP
     StrCmp $R1 '5.2' lbl_winnt_dotNET lbl_error

     lbl_winnt_x:
       StrCpy $R0 "NT $R0" 6
     Goto lbl_done

     lbl_winnt_2000:
       Strcpy $R0 '2000'
     Goto lbl_done

     lbl_winnt_XP:
       Strcpy $R0 'XP'
     Goto lbl_done

     lbl_winnt_dotNET:
       Strcpy $R0 '.NET Server'
     Goto lbl_done

   lbl_error:
     Strcpy $R0 ''
   lbl_done:
   Pop $R1
   Exch $R0
 FunctionEnd

;--------------------------------
;Installer Sections

Section "Dummy Section" SecDummy

  SetOutPath "$INSTDIR"
  File ".\ntfy_cd.exe"
  File ".\ntfy_cd.txt"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\${PRODUCT}" "" $INSTDIR
  
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayIcon" "$INSTDIR\ntfy_cd.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayVersion" "${VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "VersionMajor" "${VER_MAJOR}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "VersionMinor" "${VER_MINOR}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoRepair" "1"

  ;Register the player

  ;Win95\98 registration
  WriteRegStr HKCR "AudioCD" "" "AudioCD"
  WriteRegBin HKCR "AudioCD" "EditFlags" 02000000
  WriteRegStr HKCR "AudioCD\Shell" "" "Play"
  ReadRegStr $R0 HKCR "AudioCD\Shell\Play" ""
  StrCmp $R0 "" 0 mark0
  WriteRegStr HKCR "AudioCD\Shell\Play" "" "&Play"

mark0:
  WriteRegStr HKCR "AudioCD\Shell\Play\Command" "" "$INSTDIR\ntfy_cd.exe /play %1"

  WriteRegStr HKCR "cdafile" "" "CD Audio Track"
  WriteRegStr HKCR "cdafile\Shell" "" "Play"
  ReadRegStr $R0 HKCR "cdafile\Shell\Play" ""
  StrCmp $R0 "" 0 mark1
  WriteRegStr HKCR "cdafile\Shell\Play" "" "&Play"

mark1:
  WriteRegStr HKCR "cdafile\Shell\Play\Command" "" "$INSTDIR\ntfy_cd.exe -play %1"

  Call GetWindowsVersion
  Pop $R0
  StrCmp $R0 "XP" 0 EndRegistration

  ;WinXP registration
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "Action" "@wmproc.dll,-6502"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "DefaultIcon" "$INSTDIR\ntfy_cd.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "InvokeProgID" "${PRODUCT_CLASS}.AudioCD"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "InvokeVerb" "Play"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "Provider" "${PRODUCT}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayCDAudioOnArrival" "${PRODUCT_CLASS}PlayCDAudioOnArrival" ""

  WriteRegStr HKCR "${PRODUCT_CLASS}.AudioCD\Shell\Play\Command" "" "$INSTDIR\ntfy_cd.exe /play %1"

EndRegistration:

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
 
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT}.lnk" "$INSTDIR\ntfy_cd.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT} Readme.lnk" "$INSTDIR\ntfy_cd.txt"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  MessageBox MB_OK|MB_ICONINFORMATION "If you are using AutoRun feature to start ${PRODUCT} on new Audio CD insertion it is recommended to temporary disable it or set to a new player"

  Delete "$INSTDIR\ntfy_cd.exe"
  Delete "$INSTDIR\ntfy_cd.txt"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  StrCmp $MUI_TEMP "" noshortcuts

  Delete "$SMPROGRAMS\$MUI_TEMP\${PRODUCT}.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\${PRODUCT} Readme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  noshortcuts:

  DeleteRegKey /ifempty HKLM "Software\${PRODUCT}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"

;WinXP removal
;Comented out for now
;  DeleteRegKey HKCR "${PRODUCT_CLASS}.AudioCD"
;  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayCDAudioOnArrival\${PRODUCT_CLASS}PlayCDAudioOnArrival"
;  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "Provider" "${PRODUCT} [Removed]"

;Delete the default player if present

  ReadRegStr $MUI_TEMP HKCR "AudioCD\shell\play\command" ""
  StrCmp $MUI_TEMP "$INSTDIR\ntfy_cd.exe /play %1" 0 CDAFileStr
  DeleteRegKey HKCR "AudioCD\shell\play\command"

;Delete the default player if present
CDAFileStr:
  ReadRegStr $MUI_TEMP HKCR "cdafile\shell\play\command" ""
  StrCmp $MUI_TEMP "$INSTDIR\ntfy_cd.exe -play %1" 0 UninstFinish
  DeleteRegKey HKCR "cdafile\shell\play\command"

UninstFinish:

SectionEnd