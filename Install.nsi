;NSIS Modern User Interface version 1.63
;Basic Example Script
;Written by Joost Verburg

!define PRODUCT_CLASS "NCDP"
!define VER_MAJOR "1"
!define VER_MINOR "60 beta 5"
!define MUI_PRODUCT "Notify CD Player" ;Define your own software name here
!define MUI_VERSION "${VER_MAJOR}.${VER_MINOR}" ;Define your own software version here

!include "MUI.nsh"

;--------------------------------
;Configuration

  ;General
  OutFile "NCDSetup.exe"

  ;Folder selection page
  InstallDir "$PROGRAMFILES\${MUI_PRODUCT}"
  
  ;Remember install folder
  InstallDirRegKey HKCU "Software\${MUI_PRODUCT}" ""

  ;$9 is being used to store the Start Menu Folder.
  ;Do not use this variable in your script (or Push/Pop it)!

  ;To change this variable, use MUI_STARTMENUPAGE_VARIABLE.
  ;Have a look at the Readme for info about other options (default folder,
  ;registry).

  ;Remember the Start Menu Folder
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${MUI_PRODUCT}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !define TEMP $R0

;--------------------------------
;Modern UI Configuration

  !define MUI_LICENSEPAGE
;  !define MUI_COMPONENTSPAGE
  !define MUI_DIRECTORYPAGE
  !define MUI_STARTMENUPAGE
  
  !define MUI_ABORTWARNING
  
  !define MUI_UNINSTALLER
  !define MUI_UNCONFIRMPAGE
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  
;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecMain ${LANG_ENGLISH} "Install ${MUI_PRODUCT} on your computer."

;--------------------------------
;Data
  
  LicenseData ".\ntfycd\GNU"


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

Function .onInit

  ; Default shortcuts path to nothing
  StrCpy $9 ""

FunctionEnd

;--------------------------------
;Installer Sections

Section "!${MUI_PRODUCT}" SecMain

  ;Disable selection
  SectionIn RO

  SetOutPath "$INSTDIR"
  File ".\ntfycd\ntfy_cd.exe"
  File ".\ntfycd\ntfy_cd.txt"

  ;Store install folder
  WriteRegStr HKLM "Software\${MUI_PRODUCT}" "" $INSTDIR

  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayName" "${MUI_PRODUCT}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayIcon" "$INSTDIR\ntfy_cd.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayVersion" "${MUI_VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "VersionMajor" "${VER_MAJOR}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "VersionMinor" "${VER_MINOR}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "NoRepair" "1"

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
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "Provider" "${MUI_PRODUCT}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayCDAudioOnArrival" "${PRODUCT_CLASS}PlayCDAudioOnArrival" ""

  WriteRegStr HKCR "${PRODUCT_CLASS}.AudioCD\Shell\Play\Command" "" "$INSTDIR\ntfy_cd.exe /play %1"

EndRegistration:

  !insertmacro MUI_STARTMENU_WRITE_BEGIN
 
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\${MUI_STARTMENUPAGE_VARIABLE}"
    CreateShortCut "$SMPROGRAMS\${MUI_STARTMENUPAGE_VARIABLE}\${MUI_PRODUCT}.lnk" "$INSTDIR\ntfy_cd.exe"
    CreateShortCut "$SMPROGRAMS\${MUI_STARTMENUPAGE_VARIABLE}\${MUI_PRODUCT} Readme.lnk" "$INSTDIR\ntfy_cd.txt"
    CreateShortCut "$SMPROGRAMS\${MUI_STARTMENUPAGE_VARIABLE}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Run once installed
  ExecWait '"$INSTDIR\ntfy_cd.exe" -SETUP'

SectionEnd

;Display the Finish header
;Insert this macro after the sections if you are not using a finish page
!insertmacro MUI_SECTIONS_FINISHHEADER

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTIONS_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
!insertmacro MUI_FUNCTIONS_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  MessageBox MB_OK|MB_ICONINFORMATION "If you are using AutoRun feature to start ${MUI_PRODUCT} on new Audio CD insertion it is recommended to temporary disable it or set a new default player"

  Delete "$INSTDIR\ntfy_cd.exe"
  Delete "$INSTDIR\ntfy_cd.txt"
  Delete "$INSTDIR\Uninstall.exe"

  ;Remove shortcut
  ReadRegStr ${TEMP} "${MUI_STARTMENUPAGE_REGISTRY_ROOT}" "${MUI_STARTMENUPAGE_REGISTRY_KEY}" "${MUI_STARTMENUPAGE_REGISTRY_VALUENAME}"

  StrCmp ${TEMP} "" noshortcuts

    Delete "$SMPROGRAMS\${TEMP}\${MUI_PRODUCT}.lnk"
    Delete "$SMPROGRAMS\${TEMP}\${MUI_PRODUCT} Readme.lnk"
    Delete "$SMPROGRAMS\${TEMP}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${TEMP}" ;Only if empty, so it won't delete other shortcuts

  noshortcuts:

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKLM "Software\${MUI_PRODUCT}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}"

;WinXP removal
;Comented out for now
;  DeleteRegKey HKCR "${PRODUCT_CLASS}.AudioCD"
;  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayCDAudioOnArrival\${PRODUCT_CLASS}PlayCDAudioOnArrival"
;  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${PRODUCT_CLASS}PlayCDAudioOnArrival" "Provider" "${MUI_PRODUCT} [Removed]"

;Delete the default player if present

  ReadRegStr ${TEMP} HKCR "AudioCD\shell\play\command" ""
  StrCmp ${TEMP}  "$INSTDIR\ntfy_cd.exe /play %1" 0 CDAFileStr
  DeleteRegKey HKCR "AudioCD\shell\play\command"

;Delete the default player if present
CDAFileStr:
  ReadRegStr ${TEMP} HKCR "cdafile\shell\play\command" ""
  StrCmp ${TEMP}  "$INSTDIR\ntfy_cd.exe -play %1" 0 DisplayFinish
  DeleteRegKey HKCR "cdafile\shell\play\command"

DisplayFinish:
  ;Display the Finish header
  !insertmacro MUI_UNFINISHHEADER

SectionEnd
