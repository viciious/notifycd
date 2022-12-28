///////////////////////////////////////////////////////////////////////////////
//
//   Notify CD Player for Windows NT and Windows 95
//
//   Copyright (c) 1996-1998, Mats Ljungqvist (mlt@cyberdude.com)
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
///////////////////////////////////////////////////////////////////////////////

#include "pre.h"

#include "resource.h"

#include "ntfy_cd.h"
#include "misc.h"
#include "options.h"
#include "mci.h"
#include "cddb.h"

extern GLOBALSTRUCT gs;

#pragma warning(disable:4100)

/////////////////////////////////////////////////////////////////////
//
// PROTOTYPES
//
/////////////////////////////////////////////////////////////////////

BOOL APIENTRY OptionsTab_General(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_Tooltip(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_Database(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_Local(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_Remote(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_Email(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_Categories(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_Controls(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

BOOL APIENTRY OptionsTab_DialUp(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

/////////////////////////////////////////////////////////////////////
//
// OPTIONS
//
/////////////////////////////////////////////////////////////////////

#define NUM_TABS 9

struct {
	char* pzTabName;
	int nDialogTemplate;
    DLGPROC pfnDlgProc;
} asOptionsTabs[NUM_TABS] = {
	{"General", IDD_OPTIONS_TAB_GENERAL, OptionsTab_General},
    {"Controls", IDD_OPTIONS_TAB_CONTROLS, OptionsTab_Controls},
	{"Tooltip/Caption", IDD_OPTIONS_TAB_TOOLTIP, OptionsTab_Tooltip},
	{"Database", IDD_OPTIONS_TAB_DATABASE, OptionsTab_Database},
	{"CDDB Local", IDD_OPTIONS_TAB_LOCAL, OptionsTab_Local},
	{"CDDB Remote", IDD_OPTIONS_TAB_REMOTE, OptionsTab_Remote},
	{"e-mail", IDD_OPTIONS_TAB_EMAIL, OptionsTab_Email},
	{"Categories", IDD_OPTIONS_TAB_CATEGORIES, OptionsTab_Categories},
	{"Dial-Up", IDD_OPTIONS_TAB_DIALUP, OptionsTab_DialUp},
};

BOOL bInitApplyDone;

BOOL APIENTRY OptionsTab_General(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

        case WM_INITDIALOG: {
    		// Misc settings

            if (gs.nOptions & OPTIONS_STOPONEXIT)
                SendDlgItemMessage(hWnd, IDC_STOPONEXIT, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_STOPONSTART)
                SendDlgItemMessage(hWnd, IDC_STOPONSTART, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_EXITONCDREMOVE)
                SendDlgItemMessage(hWnd, IDC_EXITONCDREMOVE, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_PREVALWAYSPREV)
                SendDlgItemMessage(hWnd, IDC_PREVALWAYSPREV, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_REMEMBERSTATUS)
                SendDlgItemMessage(hWnd, IDC_REMEMBERSTATUS, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_NOINSERTNOTIFICATION)
                SendDlgItemMessage(hWnd, IDC_NOINSERTNOTIFICATION, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_TRACKSMENUCOLUMN)
                SendDlgItemMessage(hWnd, IDC_TRACKSMENUCOLUMN, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_NOMENUBITMAP)
                SendDlgItemMessage(hWnd, IDC_NOMENUBITMAP, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_NOMENUBREAK)
                SendDlgItemMessage(hWnd, IDC_NOMENUBREAK, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_ARTISTINMENU)
                SendDlgItemMessage(hWnd, IDC_ARTISTINMENU, BM_SETCHECK, 1, 0);                
            if (gs.nOptions & OPTIONS_AUTOCHECKFORNEWVERSION)
                SendDlgItemMessage(hWnd, IDC_AUTOCHECKFORNEWVERSION, BM_SETCHECK, 1, 0);                

			// Poll time

			SetDlgItemInt(hWnd, IDC_POLLTIME, gs.nPollTime, FALSE);

            // Default CD device

            char szTmp[80];
            int nCount = 0;

            for (unsigned int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                if (gs.abDevices[nLoop - 'A']) {
                    sprintf(szTmp, "Drive %c:", nLoop);
                    SendDlgItemMessage(hWnd, IDC_DEFAULTDEVICE, CB_ADDSTRING, 0, (LPARAM)szTmp);
                    if (nLoop - 'A' == gs.nDefaultDevice)
                        SendDlgItemMessage(hWnd, IDC_DEFAULTDEVICE, CB_SETCURSEL, nCount, 0);

                    nCount ++;
                }
            }

            // External command

            SetWindowText(GetDlgItem(hWnd, IDC_EXTERNALCOMMAND), gs.zExternalCommand);
        }
		break;

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_BROWSE) {
                OPENFILENAME sOF;
                char zFile[MAX_PATH+1];

                strcpy(zFile, gs.zExternalCommand);
                sOF.lStructSize = sizeof(sOF);
                sOF.hwndOwner = hWnd;
                sOF.lpstrFilter = "Executable files (*.EXE;*.BAT;*.CMD)\0*.EXE;*.BAT;*.CMD\0All Files (*.*)\0*.*\0\0";
                sOF.lpstrCustomFilter = NULL;
                sOF.nFilterIndex = 1;
                sOF.lpstrFile = zFile;
                sOF.nMaxFile = MAX_PATH;
                sOF.lpstrFileTitle = NULL;
                sOF.lpstrInitialDir = NULL;
                sOF.lpstrTitle = "Choose executable";
                sOF.Flags = OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN;
                sOF.nFileOffset = 0;
                sOF.nFileExtension = 7;
                sOF.lpstrDefExt = ".NCD";

                if (GetOpenFileName(&sOF))
                    SetWindowText(GetDlgItem(hWnd, IDC_EXTERNALCOMMAND), zFile);
            }
            else if (LOWORD(wParam) == IDC_REGISTERDEFAULT) {
                HKEY hKey;
                char szModulePath[256];
                char szCommandLine[256];
                DWORD dwResult;
                BOOL bFailed = FALSE;

                GetModuleFileName(gs.hMainInstance, szModulePath, 256);

                // Set the AudioCD\\shell\\Play\\command
                if (!bFailed) {
                    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "AudioCD\\shell\\Play\\command", 0, "", 0, KEY_SET_VALUE, NULL, &hKey, &dwResult) == ERROR_SUCCESS) {
                        sprintf(szCommandLine, "\"%s\" /play %%1", szModulePath);
                        RegSetValueEx(hKey, "", 0, REG_SZ, (BYTE*)szCommandLine, strlen(szCommandLine)+1);
                        RegCloseKey(hKey);
                    }
                    else
                        bFailed = TRUE;
                }

                // Set the cdafile\\shell\\Play\\command
                if (!bFailed) {
                    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "cdafile\\shell\\Play\\command", 0, "", 0, KEY_SET_VALUE, NULL, &hKey, &dwResult) == ERROR_SUCCESS) {
                        sprintf(szCommandLine, "\"%s\" -play %%1", szModulePath);
                        RegSetValueEx(hKey, "", 0, REG_SZ, (BYTE*)szCommandLine, strlen(szCommandLine)+1);
                        RegCloseKey(hKey);
                    }
                    else
                        bFailed = TRUE;
                }
                // Set the .cda type
                if (!bFailed) {
                    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, ".cda", 0, "", 0, KEY_SET_VALUE, NULL, &hKey, &dwResult) == ERROR_SUCCESS) {
                        strcpy(szCommandLine, "cdafile");
                        RegSetValueEx(hKey, "", 0, REG_SZ, (BYTE*)szCommandLine, strlen(szCommandLine)+1);
                        RegCloseKey(hKey);
                    }
                    else
                        bFailed = TRUE;
                }

                // Set the default action for AudioCD
                if (!bFailed) {
                    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "AudioCD\\shell", 0, "", 0, KEY_SET_VALUE, NULL, &hKey, &dwResult) == ERROR_SUCCESS) {
                        strcpy(szCommandLine, "Play");
                        RegSetValueEx(hKey, "", 0, REG_SZ, (BYTE*)szCommandLine, strlen(szCommandLine)+1);
                        RegCloseKey(hKey);
                    }
                    else
                        bFailed = TRUE;
                }

                // Set the default action for cdafile
                if (!bFailed) {
                    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "cdafile\\shell", 0, "", 0, KEY_SET_VALUE, NULL, &hKey, &dwResult) == ERROR_SUCCESS) {
                        strcpy(szCommandLine, "Play");
                        RegSetValueEx(hKey, "", 0, REG_SZ, (BYTE*)szCommandLine, strlen(szCommandLine)+1);
                        RegCloseKey(hKey);
                    }
                    else
                        bFailed = TRUE;
                }

                // Setup the AudioCD icon
                if (!bFailed) {
                    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "AudioCD\\DefaultIcon", 0, "", 0, KEY_SET_VALUE, NULL, &hKey, &dwResult) == ERROR_SUCCESS) {
                        strcpy(szCommandLine, "%SystemRoot%\\system32\\cdplayer.exe,0");
                        RegSetValueEx(hKey, "", 0, REG_SZ, (BYTE*)szCommandLine, strlen(szCommandLine)+1);
                        RegCloseKey(hKey);
                    }
                    else
                        bFailed = TRUE;
                }

                // Setup the cdafile icon
                if (!bFailed) {
                    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "cdafile\\DefaultIcon", 0, "", 0, KEY_SET_VALUE, NULL, &hKey, &dwResult) == ERROR_SUCCESS) {
                        strcpy(szCommandLine, "%SystemRoot%\\system32\\cdplayer.exe,1");
                        RegSetValueEx(hKey, "", 0, REG_SZ, (BYTE*)szCommandLine, strlen(szCommandLine)+1);
                        RegCloseKey(hKey);
                    }
                    else
                        bFailed = TRUE;
                }

                if (!bFailed) {    
                    // Let the shell know about the change...
	    			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH | SHCNF_IDLIST, 0, 0);
		    		SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_FLUSH | SHCNF_IDLIST, 0, 0);

                    MessageBox(hWnd, "Notify CD Player is now your default CD player", APPNAME, MB_OK | MB_ICONINFORMATION);
                }
                else
                    MessageBox(hWnd, "Failed to register Notify CD Player as your default CD player", APPNAME, MB_OK | MB_ICONERROR);
            }
        }
        break;

        case WM_NOTIFY: {
            switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    int nCount;
                    int nSel;

                    // Misc settings
                    
                    if (SendDlgItemMessage(hWnd, IDC_STOPONEXIT, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_STOPONEXIT;
                    else
                        gs.nOptions &= ~OPTIONS_STOPONEXIT;
                    if (SendDlgItemMessage(hWnd, IDC_STOPONSTART, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_STOPONSTART;
                    else
                        gs.nOptions &= ~OPTIONS_STOPONSTART;
                    if (SendDlgItemMessage(hWnd, IDC_EXITONCDREMOVE, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_EXITONCDREMOVE;
                    else
                        gs.nOptions &= ~OPTIONS_EXITONCDREMOVE;
                    if (SendDlgItemMessage(hWnd, IDC_PREVALWAYSPREV, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_PREVALWAYSPREV;
                    else
                        gs.nOptions &= ~OPTIONS_PREVALWAYSPREV;
                    if (SendDlgItemMessage(hWnd, IDC_REMEMBERSTATUS, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_REMEMBERSTATUS;
                    else
                        gs.nOptions &= ~OPTIONS_REMEMBERSTATUS;
                    if (SendDlgItemMessage(hWnd, IDC_NOINSERTNOTIFICATION, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_NOINSERTNOTIFICATION;
                    else
                        gs.nOptions &= ~OPTIONS_NOINSERTNOTIFICATION;
                    if (SendDlgItemMessage(hWnd, IDC_TRACKSMENUCOLUMN, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_TRACKSMENUCOLUMN;
                    else
                        gs.nOptions &= ~OPTIONS_TRACKSMENUCOLUMN;
                    if (SendDlgItemMessage(hWnd, IDC_NOMENUBITMAP, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_NOMENUBITMAP;
                    else
                        gs.nOptions &= ~OPTIONS_NOMENUBITMAP;
                    if (SendDlgItemMessage(hWnd, IDC_NOMENUBREAK, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_NOMENUBREAK;
                    else
                        gs.nOptions &= ~OPTIONS_NOMENUBREAK;
                    if (SendDlgItemMessage(hWnd, IDC_ARTISTINMENU, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_ARTISTINMENU;
                    else
                        gs.nOptions &= ~OPTIONS_ARTISTINMENU;
                    if (SendDlgItemMessage(hWnd, IDC_AUTOCHECKFORNEWVERSION, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_AUTOCHECKFORNEWVERSION;
                    else
                        gs.nOptions &= ~OPTIONS_AUTOCHECKFORNEWVERSION;

                    // Poll time

					gs.nPollTime = GetDlgItemInt(hWnd, IDC_POLLTIME, NULL, FALSE);

                    // Default CD device

                    nCount = 0;
                    nSel = SendDlgItemMessage(hWnd, IDC_DEFAULTDEVICE, CB_GETCURSEL, 0, 0);

                    for (int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                        if (gs.abDevices[nLoop - 'A']) {
                            if (nCount == nSel)
                                gs.nDefaultDevice = nLoop - 'A';

                            nCount ++;
                        }
                    }                  

                    // External command

                    GetWindowText(GetDlgItem(hWnd, IDC_EXTERNALCOMMAND), gs.zExternalCommand, MAX_PATH);
                }
                break;

                default:
					return FALSE;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


void SetTooltipTextBoxes(HWND hWnd, 
						 BOOL bTooltip)
{
	char szExample[512];

	if (bTooltip) {
		SetWindowText(GetDlgItem(hWnd, IDC_FORMAT), gs.szTooltipFormat);

		ParseDiscInformationFormat(&gs.di[0], gs.szTooltipFormat, szExample);

		SetWindowText(GetDlgItem(hWnd, IDC_EXAMPLE), szExample);
	}
	else {
		SetWindowText(GetDlgItem(hWnd, IDC_FORMAT), gs.szCaptionFormat);

		ParseDiscInformationFormat(&gs.di[0], gs.szCaptionFormat, szExample);

		SetWindowText(GetDlgItem(hWnd, IDC_EXAMPLE), szExample);
	}
}


void GetTooltipTextBoxes(HWND hWnd, 
						 BOOL bTooltip)
{
	if (bTooltip)
		GetWindowText(GetDlgItem(hWnd, IDC_FORMAT), gs.szTooltipFormat, 256);
	else
		GetWindowText(GetDlgItem(hWnd, IDC_FORMAT), gs.szCaptionFormat, 256);
}


BOOL APIENTRY OptionsTab_Tooltip(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            SendDlgItemMessage(hWnd, IDC_TOOLTIP, BM_SETCHECK, 1, 0);

            if (gs.nOptions & OPTIONS_SHOWONCAPTION)
                SendDlgItemMessage(hWnd, IDC_SHOWONCAPTION, BM_SETCHECK, 1, 0);
            if (gs.nOptions & OPTIONS_USEFONT)
                SendDlgItemMessage(hWnd, IDC_USEFONT, BM_SETCHECK, 1, 0);

			SetTooltipTextBoxes(hWnd, TRUE);
        }
		break;

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_FORMAT && HIWORD(wParam) == EN_CHANGE) {
				char szExample[512];
				char szFormat[256];

				GetWindowText(GetDlgItem(hWnd, IDC_FORMAT), szFormat, 256);

				ParseDiscInformationFormat(&gs.di[0], szFormat, szExample);

				SetWindowText(GetDlgItem(hWnd, IDC_EXAMPLE), szExample);
			}

			if (LOWORD(wParam) == IDC_SHOWONCAPTION) {
                if (SendDlgItemMessage(hWnd, IDC_SHOWONCAPTION, BM_GETCHECK, 0, 0)) {
                    EnableWindow(GetDlgItem(hWnd, IDC_FONT), TRUE);
                    EnableWindow(GetDlgItem(hWnd, IDC_USEFONT), TRUE);
                    EnableWindow(GetDlgItem(hWnd, IDC_TOOLTIP), TRUE);
                    EnableWindow(GetDlgItem(hWnd, IDC_CAPTION), TRUE);
                }
                else {
                    EnableWindow(GetDlgItem(hWnd, IDC_FONT), FALSE);
                    EnableWindow(GetDlgItem(hWnd, IDC_USEFONT), FALSE);
                    EnableWindow(GetDlgItem(hWnd, IDC_TOOLTIP), FALSE);
                    EnableWindow(GetDlgItem(hWnd, IDC_CAPTION), FALSE);
                }
            }
            else if (LOWORD(wParam) == IDC_FONT) {
                CHOOSEFONT sFont;

                memset(&sFont, 0, sizeof(sFont));
                sFont.lStructSize = sizeof(sFont);
                sFont.hwndOwner = hWnd;
                sFont.lpLogFont = &gs.sCaptionFont;
                sFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
                sFont.rgbColors = gs.nCaptionFontColor;
            
                if (ChooseFont(&sFont))
                    gs.nCaptionFontColor = sFont.rgbColors;
            }
            else if (LOWORD(wParam) == IDC_TOOLTIP) {
				GetTooltipTextBoxes(hWnd, FALSE);
				SetTooltipTextBoxes(hWnd, TRUE);

//                SendDlgItemMessage(hWnd, IDC_CAPTION, BM_SETCHECK, 0, 0);
            }
            else if (LOWORD(wParam) == IDC_CAPTION) {
				GetTooltipTextBoxes(hWnd, TRUE);
				SetTooltipTextBoxes(hWnd, FALSE);

//                SendDlgItemMessage(hWnd, IDC_TOOLTIP, BM_SETCHECK, 0, 0);
            }
        }
        break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {
                    GetTooltipTextBoxes(hWnd, SendDlgItemMessage(hWnd, IDC_TOOLTIP, BM_GETCHECK, 0, 0));
                        
                    if (SendDlgItemMessage(hWnd, IDC_SHOWONCAPTION, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_SHOWONCAPTION;
                    else
                        gs.nOptions &= ~OPTIONS_SHOWONCAPTION;
                    if (SendDlgItemMessage(hWnd, IDC_USEFONT, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_USEFONT;
                    else {
                        NONCLIENTMETRICS sMetrics;                       
                        sMetrics.cbSize = sizeof(sMetrics);

                        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &sMetrics, FALSE);

                        sMetrics.lfCaptionFont.lfItalic = !sMetrics.lfCaptionFont.lfItalic;

                        memcpy(&gs.sCaptionFont, &sMetrics.lfCaptionFont, sizeof(gs.sCaptionFont));

                        gs.nOptions &= ~OPTIONS_USEFONT;
                    }
                }
                break;

                default:
                    return FALSE;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


BOOL APIENTRY OptionsTab_Database(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            if (gs.nOptions & OPTIONS_USECDDB) {
                SendDlgItemMessage(hWnd, IDC_USEINI, BM_SETCHECK, 0, 0);
                SendDlgItemMessage(hWnd, IDC_USECDDB, BM_SETCHECK, 1, 0);
            }
            else {
                SendDlgItemMessage(hWnd, IDC_USEINI, BM_SETCHECK, 1, 0);
                SendDlgItemMessage(hWnd, IDC_USECDDB, BM_SETCHECK, 0, 0);
            }

            if (gs.nOptions & OPTIONS_QUERYLOCAL)
                SendDlgItemMessage(hWnd, IDC_QUERYLOCAL, BM_SETCHECK, 1, 0);
            if (gs.nOptions & OPTIONS_QUERYREMOTE)
                SendDlgItemMessage(hWnd, IDC_QUERYREMOTE, BM_SETCHECK, 1, 0);
            if (gs.nOptions & OPTIONS_STORELOCAL)
                SendDlgItemMessage(hWnd, IDC_STORELOCAL, BM_SETCHECK, 1, 0);
            if (gs.nOptions & OPTIONS_STORERESULT)
                SendDlgItemMessage(hWnd, IDC_STORERESULT, BM_SETCHECK, 1, 0);
            if (gs.nOptions & OPTIONS_AUTOADDQUEUE)
                SendDlgItemMessage(hWnd, IDC_AUTOADDQUEUE, BM_SETCHECK, 1, 0);
            if (gs.nOptions & OPTIONS_AUTORETRIEVEQUEUE)
                SendDlgItemMessage(hWnd, IDC_AUTORETRIEVEQUEUE, BM_SETCHECK, 1, 0);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    if (SendDlgItemMessage(hWnd, IDC_USECDDB, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_USECDDB;
                    else
                        gs.nOptions &= ~OPTIONS_USECDDB;
                    if (SendDlgItemMessage(hWnd, IDC_QUERYLOCAL, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_QUERYLOCAL;
                    else
                        gs.nOptions &= ~OPTIONS_QUERYLOCAL;
                    if (SendDlgItemMessage(hWnd, IDC_QUERYREMOTE, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_QUERYREMOTE;
                    else
                        gs.nOptions &= ~OPTIONS_QUERYREMOTE;
                    if (SendDlgItemMessage(hWnd, IDC_STORELOCAL, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_STORELOCAL;
                    else
                        gs.nOptions &= ~OPTIONS_STORELOCAL;
                    if (SendDlgItemMessage(hWnd, IDC_STORERESULT, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_STORERESULT;
                    else
                        gs.nOptions &= ~OPTIONS_STORERESULT;
                    if (SendDlgItemMessage(hWnd, IDC_AUTOADDQUEUE, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_AUTOADDQUEUE;
                    else
                        gs.nOptions &= ~OPTIONS_AUTOADDQUEUE;
                    if (SendDlgItemMessage(hWnd, IDC_AUTORETRIEVEQUEUE, BM_GETCHECK, 0, 0))
                        gs.nOptions |= OPTIONS_AUTORETRIEVEQUEUE;
                    else
                        gs.nOptions &= ~OPTIONS_AUTORETRIEVEQUEUE;
                }
                break;

                default:
                    return FALSE;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


BOOL APIENTRY OptionsTab_Local(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
			SendDlgItemMessage(hWnd, IDC_LOCALTYPE, CB_ADDSTRING, 0, (LPARAM) "Windows CDDB format");
			SendDlgItemMessage(hWnd, IDC_LOCALTYPE, CB_ADDSTRING, 0, (LPARAM) "Unix CDDB format");
        
			SetWindowText(GetDlgItem(hWnd, IDC_LOCALPATH), gs.cddb.zCDDBPath);
			SendMessage(GetDlgItem(hWnd, IDC_LOCALTYPE), CB_SETCURSEL, gs.cddb.nCDDBType-1, 0);

            if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_STORECOPYININI)
                SendDlgItemMessage(hWnd, IDC_STOREININI, BM_SETCHECK, 1, 0);
        }
		break;

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_BROWSE) {
				BROWSEINFO sBI;
				char szFolder[MAX_PATH];
				LPITEMIDLIST lpItemIDList;

				strcpy(szFolder, gs.cddb.zCDDBPath);

				ZeroMemory(&sBI, sizeof(sBI));

				sBI.hwndOwner = hWnd;
				sBI.lpszTitle = "Select folder for local CDDB";
				sBI.ulFlags = BIF_RETURNONLYFSDIRS;			
				
				lpItemIDList = SHBrowseForFolder(&sBI);
				if (lpItemIDList) {
					IMalloc* pMalloc;

					if (SHGetMalloc(&pMalloc) == NOERROR) {
						SHGetPathFromIDList(lpItemIDList, szFolder);

						pMalloc->Free(lpItemIDList);
						pMalloc->Release();
				
						SetWindowText(GetDlgItem(hWnd, IDC_LOCALPATH), szFolder);
					}
				}
			}
		}
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    GetWindowText(GetDlgItem(hWnd, IDC_LOCALPATH), gs.cddb.zCDDBPath, 256);
                    gs.cddb.nCDDBType = SendMessage(GetDlgItem(hWnd, IDC_LOCALTYPE), CB_GETCURSEL, 0, 0) + 1;
            
                    if (SendDlgItemMessage(hWnd, IDC_STOREININI, BM_GETCHECK, 0, 0))
                        gs.cddb.nCDDBOptions |= OPTIONS_CDDB_STORECOPYININI;
                    else
                        gs.cddb.nCDDBOptions &= ~OPTIONS_CDDB_STORECOPYININI;

                    // Fix path

                    if (strlen(gs.cddb.zCDDBPath) && gs.cddb.zCDDBPath[strlen(gs.cddb.zCDDBPath) - 1] != '\\')
                        strcat(gs.cddb.zCDDBPath, "\\");

                    // Check path!

                    if (strlen(gs.cddb.zCDDBPath)) {
                        char zPath[256];
                        WIN32_FIND_DATA sFind;
                        HANDLE hFind;

                        sprintf(zPath, "%s*.*", gs.cddb.zCDDBPath);
                        hFind = FindFirstFile(zPath, &sFind);
                        if (hFind == INVALID_HANDLE_VALUE) {
                            MessageBox(hWnd, "The path specified for the local database is invalid!", APPNAME, MB_OK | MB_ICONERROR);
            
                            SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        }
                        else
                            FindClose(hFind);
                    }
                }
                break;

                default:
                    return FALSE;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


void Update_Remote(HWND hWnd) 
{
    if (SendDlgItemMessage(hWnd, IDC_PROTHTTP, BM_GETCHECK, 0, 0)) {
        // Disable CDDB stuff
        EnableWindow(GetDlgItem(hWnd, IDC_REMOTEPORT), FALSE);
        
        // Enable HTTP stuff
        EnableWindow(GetDlgItem(hWnd, IDC_HTTPPATH), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_USEPROXY), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_PROXYSERVER), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_PROXYPORT), TRUE);

        if (SendDlgItemMessage(hWnd, IDC_USEAUTHENTICATION, BM_GETCHECK, 0, 0)) {
            EnableWindow(GetDlgItem(hWnd, IDC_USER), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_ASKFORPASSWORD), TRUE);
        }
        else {
            EnableWindow(GetDlgItem(hWnd, IDC_USER), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_ASKFORPASSWORD), FALSE);
        }

        if (SendDlgItemMessage(hWnd, IDC_USEPROXY, BM_GETCHECK, 0, 0)) {
            EnableWindow(GetDlgItem(hWnd, IDC_PROXYSERVER), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_PROXYPORT), TRUE);
            EnableWindow(GetDlgItem(hWnd, IDC_USEAUTHENTICATION), TRUE);
        }
        else {
            EnableWindow(GetDlgItem(hWnd, IDC_PROXYSERVER), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_PROXYPORT), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_USEAUTHENTICATION), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_USER), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_ASKFORPASSWORD), FALSE);
        }

        if (SendDlgItemMessage(hWnd, IDC_ASKFORPASSWORD, BM_GETCHECK, 0, 0))
            EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), FALSE);
        else
            EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), TRUE);
    }
    else {
        // Enable CDDB stuff
        EnableWindow(GetDlgItem(hWnd, IDC_REMOTEPORT), TRUE);
        
        // Disable HTTP stuff
        EnableWindow(GetDlgItem(hWnd, IDC_HTTPPATH), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_USEPROXY), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_PROXYSERVER), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_PROXYPORT), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_USEAUTHENTICATION), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_USER), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_ASKFORPASSWORD), FALSE);
    }
}


void AddServerNames(HWND hWnd)
{
    unsigned int nLoop;
    int nIndex;
    char zServerInfo[512];
    BOOL bHTTP = SendDlgItemMessage(hWnd, IDC_USEHTTP, BM_GETCHECK, 0, 0);

    SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_RESETCONTENT, 0, 0);
    
    for (nLoop = 0 ; nLoop < gs.cddb.nNumCDDBServers ; nLoop ++) {
        if ((!stricmp(gs.cddb.psCDDBServers[nLoop].zProtocol, "cddbp") && !bHTTP) ||
            (!stricmp(gs.cddb.psCDDBServers[nLoop].zProtocol, "http") && bHTTP)) {
            sprintf(zServerInfo, "%s, %s, %s, %s", 
                gs.cddb.psCDDBServers[nLoop].zSite,
                gs.cddb.psCDDBServers[nLoop].zDescription,
                gs.cddb.psCDDBServers[nLoop].zLatitude,
                gs.cddb.psCDDBServers[nLoop].zLongitude);
            nIndex = SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) zServerInfo);
            SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_SETITEMDATA, nIndex, (DWORD)&gs.cddb.psCDDBServers[nLoop]);
        }
    }
}


void GetRemoteSettings(HWND hWnd)
{
	gs.cddb.nCDDBOptions = 0;

    if (SendDlgItemMessage(hWnd, IDC_USEHTTP, BM_GETCHECK, 0, 0)) {
        gs.cddb.nCDDBOptions |= OPTIONS_CDDB_USEHTTP;
		if (SendDlgItemMessage(hWnd, IDC_USEPROXY, BM_GETCHECK, 0, 0))
			gs.cddb.nCDDBOptions |= OPTIONS_CDDB_USEPROXY;

		if (SendDlgItemMessage(hWnd, IDC_USEAUTHENTICATION, BM_GETCHECK, 0, 0) &&
			gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY)
			gs.cddb.nCDDBOptions |= OPTIONS_CDDB_USEAUTHENTICATION;

		if (SendDlgItemMessage(hWnd, IDC_ASKFORPASSWORD, BM_GETCHECK, 0, 0) &&
			gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY)
			gs.cddb.nCDDBOptions |= OPTIONS_CDDB_ASKFORPASSWORD;

		GetWindowText(GetDlgItem(hWnd, IDC_HTTPPATH), gs.cddb.zRemoteHTTPPath, 256);
		GetWindowText(GetDlgItem(hWnd, IDC_PROXYSERVER), gs.cddb.zRemoteProxyServer, 256);
		GetWindowText(GetDlgItem(hWnd, IDC_USER), gs.cddb.zProxyUser, 256);
		GetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), gs.cddb.zProxyPassword, 256);

		gs.cddb.nRemoteProxyPort = GetDlgItemInt(hWnd, IDC_PROXYPORT, NULL, FALSE);

		// Fix HTTP path

		if (gs.cddb.zRemoteHTTPPath[0] != 0 && gs.cddb.zRemoteHTTPPath[0] != '/') {
			char zTmp[100];

			strcpy(zTmp, gs.cddb.zRemoteHTTPPath);
			strcpy(gs.cddb.zRemoteHTTPPath, "/");
			strcat(gs.cddb.zRemoteHTTPPath, zTmp);
		}

		if (gs.cddb.zRemoteHTTPPath[0] != 0 && gs.cddb.zRemoteHTTPPath[strlen(gs.cddb.zRemoteHTTPPath)-1] == '/')
			gs.cddb.zRemoteHTTPPath[strlen(gs.cddb.zRemoteHTTPPath)-1] = 0;
	}

    gs.cddb.nRemoteTimeout = GetDlgItemInt(hWnd, IDC_REMOTETIMEOUT, NULL, FALSE);

    GetWindowText(GetDlgItem(hWnd, IDC_REMOTESERVER), gs.cddb.zRemoteServer, 256);
    gs.cddb.nRemotePort = GetDlgItemInt(hWnd, IDC_REMOTEPORT, NULL, FALSE);
}


BOOL APIENTRY OptionsTab_Remote(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            // Add server names!

			SetDlgItemInt(hWnd, IDC_REMOTEPORT, gs.cddb.nRemotePort, FALSE);

			SetDlgItemInt(hWnd, IDC_REMOTETIMEOUT, gs.cddb.nRemoteTimeout, FALSE);

            SetWindowText(GetDlgItem(hWnd, IDC_HTTPPATH), gs.cddb.zRemoteHTTPPath);
			SetWindowText(GetDlgItem(hWnd, IDC_PROXYSERVER), gs.cddb.zRemoteProxyServer);
			SetDlgItemInt(hWnd, IDC_PROXYPORT, gs.cddb.nRemoteProxyPort, FALSE);

            SetWindowText(GetDlgItem(hWnd, IDC_USER), gs.cddb.zProxyUser);
			SetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), gs.cddb.zProxyPassword);

            if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP)  {
                SendDlgItemMessage(hWnd, IDC_PROTHTTP, BM_SETCHECK, 1, 0);
                SendDlgItemMessage(hWnd, IDC_PROTCDDB, BM_SETCHECK, 0, 0);
            }
            else {
                SendDlgItemMessage(hWnd, IDC_PROTHTTP, BM_SETCHECK, 0, 0);
                SendDlgItemMessage(hWnd, IDC_PROTCDDB, BM_SETCHECK, 1, 0);
            }

            if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY) 
                SendDlgItemMessage(hWnd, IDC_USEPROXY, BM_SETCHECK, 1, 0);                  

            if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEAUTHENTICATION) 
                SendDlgItemMessage(hWnd, IDC_USEAUTHENTICATION, BM_SETCHECK, 1, 0);                  

            if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD) 
                SendDlgItemMessage(hWnd, IDC_ASKFORPASSWORD, BM_SETCHECK, 1, 0);                  

            AddServerNames(hWnd);
        
			SetWindowText(GetDlgItem(hWnd, IDC_REMOTESERVER), gs.cddb.zRemoteServer);

            Update_Remote(hWnd);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
					GetRemoteSettings(hWnd);
                }
                break;

                default:
                    return FALSE;
            }
        }
        break;

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_PROTHTTP || LOWORD(wParam) == IDC_PROTCDDB || 
                LOWORD(wParam) == IDC_USEPROXY || LOWORD(wParam) == IDC_USEAUTHENTICATION || 
                LOWORD(wParam) == IDC_ASKFORPASSWORD)
                Update_Remote(hWnd);

            if (LOWORD(wParam) == IDC_PROTHTTP || LOWORD(wParam) == IDC_PROTCDDB)
                AddServerNames(hWnd);

            if (LOWORD(wParam) == IDC_REMOTESERVER) {
                if (HIWORD(wParam) == CBN_SELENDOK) {
                    int nSel;

                    nSel = SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_GETCURSEL, 0, 0);
                    if (nSel != CB_ERR) {
                        CDDB_SERVER* psServer;

                        psServer = (CDDB_SERVER*) SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_GETITEMDATA, nSel, 0);
                        
                        if (SendDlgItemMessage(hWnd, IDC_USEHTTP, BM_GETCHECK, 0, 0))
                            SetWindowText(GetDlgItem(hWnd, IDC_HTTPPATH), psServer->zAddress);
                        else
            			    SetDlgItemInt(hWnd, IDC_REMOTEPORT, psServer->nPort, FALSE);
                    }
                }
            }

			if (LOWORD(wParam) == IDC_QUERYSITES) {
                if (MessageBox(hWnd, "Do you wan't to try to get a site list from this server using the selected options?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
					int nOldCDDBOptions;
					int nOldRemoteTimeout;
					int nOldRemotePort;
					char zOldRemoteServer[256];
					char zOldRemoteHTTPPath[256];
					char zOldRemoteProxyServer[256];
					char zOldProxyUser[256];
					char zOldProxyPassword[256];
					int nOldRemoteProxyPort;

					nOldCDDBOptions = gs.cddb.nCDDBOptions;
					nOldRemoteTimeout = gs.cddb.nRemoteTimeout;
					nOldRemotePort = gs.cddb.nRemotePort;
					nOldRemoteProxyPort = gs.cddb.nRemoteProxyPort;
					strcpy(zOldRemoteServer, gs.cddb.zRemoteServer);
					strcpy(zOldRemoteHTTPPath, gs.cddb.zRemoteHTTPPath);
					strcpy(zOldRemoteProxyServer, gs.cddb.zRemoteProxyServer);
					strcpy(zOldProxyUser, gs.cddb.zProxyUser);
					strcpy(zOldProxyPassword, gs.cddb.zProxyPassword);

					GetRemoteSettings(hWnd);
				
					if (CDDBQuerySites())
                        AddServerNames(hWnd);

					gs.cddb.nCDDBOptions = nOldCDDBOptions;
					gs.cddb.nRemoteTimeout = nOldRemoteTimeout;
					gs.cddb.nRemotePort = nOldRemotePort;
					gs.cddb.nRemoteProxyPort = nOldRemoteProxyPort;
					strcpy(gs.cddb.zRemoteServer, zOldRemoteServer);
					strcpy(gs.cddb.zRemoteHTTPPath, zOldRemoteHTTPPath);
					strcpy(gs.cddb.zRemoteProxyServer, zOldRemoteProxyServer);
					strcpy(gs.cddb.zProxyUser, zOldProxyUser);
					strcpy(gs.cddb.zProxyPassword, zOldProxyPassword);
                }
			}
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


BOOL APIENTRY OptionsTab_Email(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
			SetWindowText(GetDlgItem(hWnd, IDC_EMAILSERVER), gs.cddb.zRemoteEmailServer);
			SetWindowText(GetDlgItem(hWnd, IDC_EMAILADDRESS), gs.cddb.zEmailAddress);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    GetWindowText(GetDlgItem(hWnd, IDC_EMAILSERVER), gs.cddb.zRemoteEmailServer, 256);
                    GetWindowText(GetDlgItem(hWnd, IDC_EMAILADDRESS), gs.cddb.zEmailAddress, 256);
                }
                break;

                default:
                    return FALSE;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


BOOL APIENTRY OptionsTab_Categories(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            unsigned int nLoop;

            for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++)
                SendMessage(GetDlgItem(hWnd, IDC_CATEGORIES), LB_ADDSTRING, 0, (LPARAM) gs.ppzCategories[nLoop]);

            EnableWindow(GetDlgItem(hWnd, IDC_SETNAME), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_ADD), FALSE);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    unsigned int nCount;
                    unsigned int nLoop;
                    
                    // Categories

                    char zKey[80];
                    nCount = SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETCOUNT, 0, 0);

                    for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++)
                        free(gs.ppzCategories[nLoop]);
                    free(gs.ppzCategories);

                    gs.ppzCategories = (char**)malloc(nCount * sizeof(char*));
                    for (nLoop = 0 ; nLoop < nCount ; nLoop ++)
                        gs.ppzCategories[nLoop] = (char*)malloc(80*sizeof(char));

                    for (nLoop = 0 ; nLoop < nCount ; nLoop ++)
                        SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETTEXT, nLoop, (LPARAM) gs.ppzCategories[nLoop]);

                    gs.nNumCategories = nCount;

                    for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++) {
                        sprintf(zKey, "Category%d", nLoop);
                        WritePrivateProfileString("NTFY_CD", zKey, gs.ppzCategories[nLoop], "CDPLAYER.INI");
                    }

                    WritePrivateProfileInt("NTFY_CD", "NumCategories", gs.nNumCategories, "CDPLAYER.INI");
                }
                break;

                default:
                    return FALSE;
            }
        }
        break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_SETFOCUS) {
                if (LOWORD(wParam) == IDC_NAME) {
					if (HIWORD(wParam) == EN_SETFOCUS)
						ChangeDefButton(hWnd, IDC_SETNAME, IDOK);
					else if (HIWORD(wParam) == EN_KILLFOCUS)
						ChangeDefButton(hWnd, IDOK, IDC_SETNAME);
				}
            }
            else if (HIWORD(wParam) == LBN_SELCHANGE) {
                switch(LOWORD(wParam)) {
				    case IDC_CATEGORIES: {
                        char zStr[256];
                        int nIndex = SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETCURSEL, 0, 0);

                        SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETTEXT, nIndex, (LPARAM) zStr);
                        SetWindowText(GetDlgItem(hWnd, IDC_NAME), zStr);

                        EnableWindow(GetDlgItem(hWnd, IDC_SETNAME), TRUE);
                    }
				    break;
                }
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                switch(LOWORD(wParam)) {
                    case IDC_ADD: {
                        char zStr[256];

                        GetWindowText(GetDlgItem(hWnd, IDC_NAME), zStr, 255);

                        SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_ADDSTRING, 0, (LPARAM) zStr);

                        SetWindowText(GetDlgItem(hWnd, IDC_NAME), "");
                    }
                    break;
           
                    case IDC_REMOVE: {
                        int nIndex = SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETCURSEL, 0, 0);

                        SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_DELETESTRING, nIndex, 0);

                        EnableWindow(GetDlgItem(hWnd, IDC_SETNAME), FALSE);
                    }
                    break;


                    case IDC_SETNAME: {
                        char szName[80];
                        int nIndex = SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETCURSEL, 0, 0);
                        if (nIndex != LB_ERR) {
                            GetWindowText(GetDlgItem(hWnd, IDC_NAME), szName, 80);

                            SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_DELETESTRING, nIndex, 0);
                            SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_ADDSTRING, nIndex, (LPARAM) szName);

                            SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_SETCURSEL, nIndex, 0);
                        }
                    }
                }
            }
            else if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_NAME) {
                if (GetWindowTextLength(GetDlgItem(hWnd, IDC_NAME)))
                    EnableWindow(GetDlgItem(hWnd, IDC_ADD), TRUE);
                else
                    EnableWindow(GetDlgItem(hWnd, IDC_ADD), FALSE);
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


struct _BindingDesc {
   const char * pszDesc;
   unsigned int nValue;
} aBindingDescs[] = {
   {"Nothing", 0},
   {"Play", IDM_PLAY},
   {"Play Whole", IDM_PLAYWHOLE},
   {"First Track", IDM_TRACKS},
   {"Open Tray", IDM_OPEN},
   {"Close Tray", IDM_CLOSE},
   {"Pause", IDM_PAUSE},
   {"Stop", IDM_STOP},
   {"Next", IDM_NEXT},
   {"Previous", IDM_PREV},
   {"Info", IDM_INFO},
   {"Repeat", IDM_REPEAT},
   {"Repeat Track", IDM_REPEATTRACK},
   {"Random Play", IDM_RANDOMIZE},
   {"Database", IDM_CDDB},
   {"Skip", IDM_SKIP},
   {"Set Position", IDM_SETABSTRACKPOS},
   {"Set Category", IDM_SETCATEGORY},
   {"Get From Internet", IDM_INTERNETGET},
   {"Quit", IDM_QUIT},
   {NULL, 0}
};


int anActionControls[NUM_BINDINGS] = {
    IDC_CLICK1,
    IDC_CLICK2,
    IDC_CLICK3,
    IDC_CLICK4,
    IDC_CLICK5,
    IDC_DRAGR,
    IDC_DRAGL,
    IDC_DRAGU,
    IDC_DRAGD,
};

int anHotkeyControls[NUM_BINDINGS] = {
    IDC_HOTKEY1,
    IDC_HOTKEY2,
    IDC_HOTKEY3,
    IDC_HOTKEY4,
    IDC_HOTKEY5,
    IDC_HOTKEY6,
    IDC_HOTKEY7,
    IDC_HOTKEY8,
    IDC_HOTKEY9,
};

BOOL CALLBACK OptionsTab_Controls(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

        case WM_INITDIALOG: {
            int nLoop;

            // Left button clicks
            for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++) {
                _BindingDesc * b=aBindingDescs;
                int index=0;
                while (b->pszDesc) {
                    SendDlgItemMessage(hWnd, anActionControls[nLoop], CB_ADDSTRING, 0, (LPARAM)b->pszDesc);
                    if (gs.anBindings[nLoop]==b->nValue) {
                        SendDlgItemMessage(hWnd, anActionControls[nLoop], CB_SETCURSEL, index, 0);
                    }
    
                    b++;
                    index++;
                }
            }           

            // hotkeys
            for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++)
                SendDlgItemMessage(hWnd, anHotkeyControls[nLoop], HKM_SETHOTKEY, gs.anHotkeys[nLoop], 0);

            // Unregister hotkeys
            for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++)
                UnregisterHotKey(gs.hMainWnd, nLoop);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_RESET: {                  
                    SetHotkeys();
                }
                break;

                case PSN_APPLY: {
                    int nLoop;

                    // Left button
                    for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop++) {
                        int sel=SendDlgItemMessage(hWnd, anActionControls[nLoop], CB_GETCURSEL, 0, 0);
                        gs.anBindings[nLoop] = aBindingDescs[sel].nValue;
                    }

                    // Unregister hotkeys
                    for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++)
                        UnregisterHotKey(gs.hMainWnd, nLoop);

                    // Get Hotkeys and register them
                    for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++)
                        gs.anHotkeys[nLoop] = SendDlgItemMessage(hWnd, anHotkeyControls[nLoop], HKM_GETHOTKEY, 0, 0);
                   
                    SetHotkeys();
                }
                break;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}

void Update_Dialup(HWND hWnd) 
{
    if (SendDlgItemMessage(hWnd, IDC_USERAS, BM_GETCHECK, 0, 0)) {
		EnableWindow(GetDlgItem(hWnd, IDC_QUERYIFCONNECTED), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_AUTOCONNECT), TRUE);

		if (SendDlgItemMessage(hWnd, IDC_AUTOCONNECT, BM_GETCHECK, 0, 0)) {
			EnableWindow(GetDlgItem(hWnd, IDC_USEANYCONNECTION), TRUE);		
			EnableWindow(GetDlgItem(hWnd, IDC_CONNECTION), TRUE);		
			EnableWindow(GetDlgItem(hWnd, IDC_PROPERTIES), TRUE);		
			EnableWindow(GetDlgItem(hWnd, IDC_NEWCONNECTION), TRUE);		
			EnableWindow(GetDlgItem(hWnd, IDC_USERNAME), TRUE);		
			EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), TRUE);		
			EnableWindow(GetDlgItem(hWnd, IDC_CLOSECONNECTION), TRUE);		
		}
		else {
			EnableWindow(GetDlgItem(hWnd, IDC_USEANYCONNECTION), FALSE);		
			EnableWindow(GetDlgItem(hWnd, IDC_CONNECTION), FALSE);		
			EnableWindow(GetDlgItem(hWnd, IDC_PROPERTIES), FALSE);		
			EnableWindow(GetDlgItem(hWnd, IDC_NEWCONNECTION), FALSE);		
			EnableWindow(GetDlgItem(hWnd, IDC_USERNAME), FALSE);		
			EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), FALSE);		
			EnableWindow(GetDlgItem(hWnd, IDC_CLOSECONNECTION), FALSE);		
		}
    }
    else {
		EnableWindow(GetDlgItem(hWnd, IDC_QUERYIFCONNECTED), FALSE);

		EnableWindow(GetDlgItem(hWnd, IDC_AUTOCONNECT), FALSE);

		EnableWindow(GetDlgItem(hWnd, IDC_USEANYCONNECTION), FALSE);		
		EnableWindow(GetDlgItem(hWnd, IDC_CONNECTION), FALSE);		
		EnableWindow(GetDlgItem(hWnd, IDC_PROPERTIES), FALSE);		
		EnableWindow(GetDlgItem(hWnd, IDC_NEWCONNECTION), FALSE);		
		EnableWindow(GetDlgItem(hWnd, IDC_USERNAME), FALSE);		
		EnableWindow(GetDlgItem(hWnd, IDC_PASSWORD), FALSE);		
		EnableWindow(GetDlgItem(hWnd, IDC_CLOSECONNECTION), FALSE);		
    }
}


LPRASENTRYNAME lpRasEntries;
DWORD dwNumRasEntries;

BOOL APIENTRY OptionsTab_DialUp(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

        case WM_INITDIALOG: {
			unsigned int nLoop;

            if (gs.ras.nOptions & OPTIONS_RAS_USERAS)
                SendDlgItemMessage(hWnd, IDC_USERAS, BM_SETCHECK, 1, 0);                
            if (gs.ras.nOptions & OPTIONS_RAS_AUTOCONNECT) {
                SendDlgItemMessage(hWnd, IDC_AUTOCONNECT, BM_SETCHECK, 1, 0);
                SendDlgItemMessage(hWnd, IDC_QUERYIFCONNECTED, BM_SETCHECK, 0, 0);
            }
            if (gs.ras.nOptions & OPTIONS_RAS_USEANYCONNECTION)
                SendDlgItemMessage(hWnd, IDC_USEANYCONNECTION, BM_SETCHECK, 1, 0);
            if (gs.ras.nOptions & OPTIONS_RAS_CLOSECONNECTION)
                SendDlgItemMessage(hWnd, IDC_CLOSECONNECTION, BM_SETCHECK, 1, 0);
            if (gs.ras.nOptions & OPTIONS_RAS_QUERYIFCONNECTED) {
                SendDlgItemMessage(hWnd, IDC_AUTOCONNECT, BM_SETCHECK, 0, 0);
                SendDlgItemMessage(hWnd, IDC_QUERYIFCONNECTED, BM_SETCHECK, 1, 0);
            }

            for (nLoop = 0 ; nLoop < dwNumRasEntries ; nLoop ++) 
				SendDlgItemMessage(hWnd, IDC_CONNECTION, CB_ADDSTRING, 0, (LPARAM)lpRasEntries[nLoop].szEntryName);

			SendDlgItemMessage(hWnd, IDC_CONNECTION, CB_SELECTSTRING, 0, (LPARAM)gs.ras.szConnectionName);

			RASDIALPARAMS sParams;
			BOOL bPassword;

			sParams.dwSize = sizeof(RASDIALPARAMS);
			strcpy(sParams.szEntryName, gs.ras.szConnectionName);

            if (gs.ras.RasGetEntryDialParams(NULL, &sParams, &bPassword) == NO_ERROR) {
			    SetWindowText(GetDlgItem(hWnd, IDC_USERNAME), sParams.szUserName);
			    SetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), sParams.szPassword);
            }

            Update_Dialup(hWnd);
        }
		break;

        case WM_NOTIFY: {
            switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    gs.ras.nOptions = 0;
                    
                    if (SendDlgItemMessage(hWnd, IDC_USERAS, BM_GETCHECK, 0, 0)) {
                        gs.ras.nOptions |= OPTIONS_RAS_USERAS; 

                        if (SendDlgItemMessage(hWnd, IDC_AUTOCONNECT, BM_GETCHECK, 0, 0))
                            gs.ras.nOptions |= OPTIONS_RAS_AUTOCONNECT;
                        if (SendDlgItemMessage(hWnd, IDC_QUERYIFCONNECTED, BM_GETCHECK, 0, 0))
                            gs.ras.nOptions |= OPTIONS_RAS_QUERYIFCONNECTED;
                        if (SendDlgItemMessage(hWnd, IDC_USEANYCONNECTION, BM_GETCHECK, 0, 0))
                            gs.ras.nOptions |= OPTIONS_RAS_USEANYCONNECTION;
                        if (SendDlgItemMessage(hWnd, IDC_CLOSECONNECTION, BM_GETCHECK, 0, 0))
                            gs.ras.nOptions |= OPTIONS_RAS_CLOSECONNECTION;

					    GetWindowText(GetDlgItem(hWnd, IDC_CONNECTION), gs.ras.szConnectionName, RAS_MaxEntryName + 1);

                        if (strlen(gs.ras.szConnectionName)) {
                            RASDIALPARAMS sParams;

					        sParams.dwSize = sizeof(RASDIALPARAMS);
					        strcpy(sParams.szEntryName, gs.ras.szConnectionName);

					        GetWindowText(GetDlgItem(hWnd, IDC_USERNAME), sParams.szUserName, UNLEN + 1);
					        GetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), sParams.szPassword, PWLEN + 1);
					        gs.ras.RasSetEntryDialParams(NULL, &sParams, FALSE);
                        }
                    }
                }
                break;

                default:
					return FALSE;
            }
        }
        break;

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_USERAS || LOWORD(wParam) == IDC_AUTOCONNECT || 
                LOWORD(wParam) == IDC_QUERYIFCONNECTED)
				Update_Dialup(hWnd);

            if (LOWORD(wParam) == OPTIONS_RAS_QUERYIFCONNECTED || LOWORD(wParam) == IDC_AUTOCONNECT) {
                if (SendDlgItemMessage(hWnd, IDC_AUTOCONNECT, BM_GETCHECK, 0, 0)) {
                    SendDlgItemMessage(hWnd, IDC_AUTOCONNECT, BM_SETCHECK, 1, 0);
                    SendDlgItemMessage(hWnd, IDC_QUERYIFCONNECTED, BM_SETCHECK, 0, 0);
                }
                else if (SendDlgItemMessage(hWnd, IDC_QUERYIFCONNECTED, BM_GETCHECK, 0, 0)) {
                    SendDlgItemMessage(hWnd, IDC_AUTOCONNECT, BM_SETCHECK, 0, 0);
                    SendDlgItemMessage(hWnd, IDC_QUERYIFCONNECTED, BM_SETCHECK, 1, 0);
                }
            }
            
            if (LOWORD(wParam) == IDC_CONNECTION && HIWORD(wParam) == CBN_SELCHANGE) {
				RASDIALPARAMS sParams;
				BOOL bPassword;
				char szEntry[RAS_MaxEntryName + 1];
					
				GetWindowText(GetDlgItem(hWnd, IDC_CONNECTION), szEntry, RAS_MaxEntryName + 1);

				sParams.dwSize = sizeof(RASDIALPARAMS);
				strcpy(sParams.szEntryName, szEntry);

                if (gs.ras.RasGetEntryDialParams(NULL, &sParams, &bPassword) == NO_ERROR) {
    				SetWindowText(GetDlgItem(hWnd, IDC_USERNAME), sParams.szUserName);
	    			SetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), sParams.szPassword);
                }
			}

			if (HIWORD(wParam) == BN_CLICKED) {
				if (LOWORD(wParam) == IDC_PROPERTIES) {
					char szEntry[RAS_MaxEntryName + 1];
					
					GetWindowText(GetDlgItem(hWnd, IDC_CONNECTION), szEntry, RAS_MaxEntryName + 1);

					gs.ras.RasEditPhonebookEntry(hWnd, NULL, szEntry);
				}
				else if (LOWORD(wParam) == IDC_NEWCONNECTION) {
					char szEntry[RAS_MaxEntryName + 1];
					unsigned int nLoop;

					GetWindowText(GetDlgItem(hWnd, IDC_CONNECTION), szEntry, RAS_MaxEntryName + 1);

					gs.ras.RasCreatePhonebookEntry(hWnd, NULL);

					free(lpRasEntries);
					lpRasEntries = GetRASEntries(&dwNumRasEntries);

					SendDlgItemMessage(hWnd, IDC_CONNECTION, CB_RESETCONTENT, 0, 0);

					for (nLoop = 0 ; nLoop < dwNumRasEntries ; nLoop ++) 
						SendDlgItemMessage(hWnd, IDC_CONNECTION, CB_ADDSTRING, 0, (LPARAM)lpRasEntries[nLoop].szEntryName);
	
					SendDlgItemMessage(hWnd, IDC_CONNECTION, CB_SELECTSTRING, 0, (LPARAM)szEntry);
				}
			}
		}
		break;

        default:
            return FALSE;
    }

    return TRUE;
}


void DoOptions()
{
    PROPSHEETHEADER sPropSheet;
    PROPSHEETPAGE asPropPages[NUM_TABS];
	int nNum = NUM_TABS;
    int nLoop;

	if (!gs.ras.hLib)
		nNum --;
	else
		lpRasEntries = GetRASEntries(&dwNumRasEntries);

    gs.bInOptionsDlg = TRUE;

    // Setup pages

    for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
        memset(&asPropPages[nLoop], 0, sizeof(PROPSHEETPAGE));
        asPropPages[nLoop].dwSize = sizeof(PROPSHEETPAGE);
        asPropPages[nLoop].dwFlags = PSP_USETITLE | PSP_USECALLBACK;
        asPropPages[nLoop].hInstance = gs.hMainInstance;
        asPropPages[nLoop].pszTemplate = MAKEINTRESOURCE(asOptionsTabs[nLoop].nDialogTemplate);
        asPropPages[nLoop].pszTitle = asOptionsTabs[nLoop].pzTabName;
        asPropPages[nLoop].pfnDlgProc = asOptionsTabs[nLoop].pfnDlgProc;
    }

    // Setup sheet
    
    memset(&sPropSheet, 0, sizeof(PROPSHEETHEADER));
    sPropSheet.dwSize = sizeof(PROPSHEETHEADER);
    sPropSheet.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_USEICONID;
    sPropSheet.hwndParent = NULL;
    sPropSheet.hInstance = gs.hMainInstance;
    sPropSheet.pszCaption = (LPSTR)"Options";
    sPropSheet.pszIcon = MAKEINTRESOURCE(IDI_MAIN);
    sPropSheet.nPages = nNum;
    sPropSheet.ppsp = (LPCPROPSHEETPAGE)&asPropPages;

    bInitApplyDone = FALSE;

    PropertySheet(&sPropSheet);

	// Free stuff
	if (lpRasEntries)
		free(lpRasEntries);

    // Fix popup menu

    InitMenu(&gs.di[0]);

	KillTimer(gs.hMainWnd, 1);
	SetTimer(gs.hMainWnd, 1, gs.nPollTime*1000, NULL);

	SaveConfig();
    
    gs.bInOptionsDlg = FALSE;

    if ((gs.nOptions & OPTIONS_USECDDB) && (!strlen(gs.cddb.zEmailAddress) || !strchr(gs.cddb.zEmailAddress, '@'))) {
        MessageBox(NULL, "You should enter a valid e-mail address", APPNAME, MB_OK | MB_ICONINFORMATION);

        return;
    }
    if ((gs.nOptions & OPTIONS_QUERYLOCAL) && !strlen(gs.cddb.zCDDBPath) && (gs.nOptions & OPTIONS_USECDDB)) {
        MessageBox(NULL, "Query local database requires a local database path", APPNAME, MB_OK | MB_ICONINFORMATION);

        return;
    }
    if ((gs.nOptions & OPTIONS_QUERYREMOTE) && !strlen(gs.cddb.zRemoteServer)) {
        MessageBox(NULL, "Query remote database server requires a server name", APPNAME, MB_OK | MB_ICONINFORMATION);

        return;
    }
    if ((gs.nOptions & OPTIONS_QUERYREMOTE) && !gs.cddb.nRemotePort) {
        MessageBox(NULL, "Query remote database server requires a server port", APPNAME, MB_OK | MB_ICONINFORMATION);

        return;
    }
    if ((gs.nOptions & OPTIONS_STORELOCAL) && !(gs.nOptions & OPTIONS_QUERYLOCAL)) {
        MessageBox(NULL, "Store local requires query local! (Wouldn't do much good otherwise would it?)", APPNAME, MB_OK | MB_ICONINFORMATION);

        return;
    }
    if ((gs.nOptions & OPTIONS_STORERESULT) && !(gs.nOptions & OPTIONS_QUERYLOCAL)) {
        MessageBox(NULL, "Store result requires query local! (Wouldn't do much good otherwise would it?)", APPNAME, MB_OK | MB_ICONINFORMATION);

        return;
    }
}


