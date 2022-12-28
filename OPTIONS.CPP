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

#define STRICT
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable:4201)
#pragma warning(disable:4514)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>

#include "resource.h"

#include "ntfy_cd.h"
#include "misc.h"
#include "options.h"
#include "mci.h"

#pragma warning(disable:4100)

/////////////////////////////////////////////////////////////////////
//
// PROTOTYPES
//
/////////////////////////////////////////////////////////////////////

void InitMenu();

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

/////////////////////////////////////////////////////////////////////
//
// OPTIONS
//
/////////////////////////////////////////////////////////////////////

#define NUM_TABS 7

struct {
	char* pzTabName;
	int nDialogTemplate;
    DLGPROC pfnDlgProc;
} asOptionsTabs[NUM_TABS] = {
	{"General", IDD_OPTIONS_TAB_GENERAL, OptionsTab_General},
	{"Tooltip/Caption", IDD_OPTIONS_TAB_TOOLTIP, OptionsTab_Tooltip},
	{"Database", IDD_OPTIONS_TAB_DATABASE, OptionsTab_Database},
	{"CDDB Local", IDD_OPTIONS_TAB_LOCAL, OptionsTab_Local},
	{"CDDB Remote", IDD_OPTIONS_TAB_REMOTE, OptionsTab_Remote},
	{"e-mail", IDD_OPTIONS_TAB_EMAIL, OptionsTab_Email},
	{"Categories", IDD_OPTIONS_TAB_CATEGORIES, OptionsTab_Categories},
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

            if (nOptions & OPTIONS_STOPONEXIT)
                SendDlgItemMessage(hWnd, IDC_STOPONEXIT, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_STOPONSTART)
                SendDlgItemMessage(hWnd, IDC_STOPONSTART, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_EXITONCDREMOVE)
                SendDlgItemMessage(hWnd, IDC_EXITONCDREMOVE, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_PREVALWAYSPREV)
                SendDlgItemMessage(hWnd, IDC_PREVALWAYSPREV, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_REMEMBERSTATUS)
                SendDlgItemMessage(hWnd, IDC_REMEMBERSTATUS, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_NOINSERTNOTIFICATION)
                SendDlgItemMessage(hWnd, IDC_NOINSERTNOTIFICATION, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_TRACKSMENUCOLUMN)
                SendDlgItemMessage(hWnd, IDC_TRACKSMENUCOLUMN, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_NOMENUBITMAP)
                SendDlgItemMessage(hWnd, IDC_NOMENUBITMAP, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_NOMENUBREAK)
                SendDlgItemMessage(hWnd, IDC_NOMENUBREAK, BM_SETCHECK, 1, 0);                
            if (nOptions & OPTIONS_ARTISTINMENU)
                SendDlgItemMessage(hWnd, IDC_ARTISTINMENU, BM_SETCHECK, 1, 0);                

            // Left button clicks

			SetDlgItemInt(hWnd, IDC_LEFT_NEXT, nLeftButtonNext, FALSE);
			
			SetDlgItemInt(hWnd, IDC_LEFT_PAUSE, nLeftButtonPause, FALSE);
			
			SetDlgItemInt(hWnd, IDC_LEFT_PREV, nLeftButtonPrev, FALSE);

			SetDlgItemInt(hWnd, IDC_LEFT_STOP, nLeftButtonStop, FALSE);

			// Poll time

			SetDlgItemInt(hWnd, IDC_POLLTIME, nPollTime, FALSE);

            // Default CD device

            char szTmp[80];
            int nCount = 0;

            for (int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                if (abDevices[nLoop - 'A']) {
                    sprintf(szTmp, "Drive %c:", nLoop);
                    SendDlgItemMessage(hWnd, IDC_DEFAULTDEVICE, CB_ADDSTRING, 0, (LPARAM)szTmp);
                    if (nLoop - 'A' == nDefaultDevice)
                        SendDlgItemMessage(hWnd, IDC_DEFAULTDEVICE, CB_SETCURSEL, nCount, 0);

                    nCount ++;
                }
            }

            // External command

            SetWindowText(GetDlgItem(hWnd, IDC_EXTERNALCOMMAND), zExternalCommand);
        }
		break;

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_BROWSE) {
                OPENFILENAME sOF;
                char zFile[MAX_PATH+1];

                strcpy(zFile, zExternalCommand);
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
        }
        break;

        case WM_NOTIFY: {
            switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    int nCount;
                    int nSel;

                    // Misc settings
                    
                    if (SendDlgItemMessage(hWnd, IDC_STOPONEXIT, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_STOPONEXIT;
                    else
                        nOptions &= ~OPTIONS_STOPONEXIT;
                    if (SendDlgItemMessage(hWnd, IDC_STOPONSTART, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_STOPONSTART;
                    else
                        nOptions &= ~OPTIONS_STOPONSTART;
                    if (SendDlgItemMessage(hWnd, IDC_EXITONCDREMOVE, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_EXITONCDREMOVE;
                    else
                        nOptions &= ~OPTIONS_EXITONCDREMOVE;
                    if (SendDlgItemMessage(hWnd, IDC_PREVALWAYSPREV, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_PREVALWAYSPREV;
                    else
                        nOptions &= ~OPTIONS_PREVALWAYSPREV;
                    if (SendDlgItemMessage(hWnd, IDC_REMEMBERSTATUS, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_REMEMBERSTATUS;
                    else
                        nOptions &= ~OPTIONS_REMEMBERSTATUS;
                    if (SendDlgItemMessage(hWnd, IDC_NOINSERTNOTIFICATION, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_NOINSERTNOTIFICATION;
                    else
                        nOptions &= ~OPTIONS_NOINSERTNOTIFICATION;
                    if (SendDlgItemMessage(hWnd, IDC_TRACKSMENUCOLUMN, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_TRACKSMENUCOLUMN;
                    else
                        nOptions &= ~OPTIONS_TRACKSMENUCOLUMN;
                    if (SendDlgItemMessage(hWnd, IDC_NOMENUBITMAP, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_NOMENUBITMAP;
                    else
                        nOptions &= ~OPTIONS_NOMENUBITMAP;
                    if (SendDlgItemMessage(hWnd, IDC_NOMENUBREAK, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_NOMENUBREAK;
                    else
                        nOptions &= ~OPTIONS_NOMENUBREAK;
                    if (SendDlgItemMessage(hWnd, IDC_ARTISTINMENU, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_ARTISTINMENU;
                    else
                        nOptions &= ~OPTIONS_ARTISTINMENU;

                    // Left button

					nLeftButtonNext = GetDlgItemInt(hWnd, IDC_LEFT_NEXT, NULL, FALSE);
					nLeftButtonPause = GetDlgItemInt(hWnd, IDC_LEFT_PAUSE, NULL, FALSE);
					nLeftButtonPrev = GetDlgItemInt(hWnd, IDC_LEFT_PREV, NULL, FALSE);
					nLeftButtonStop = GetDlgItemInt(hWnd, IDC_LEFT_STOP, NULL, FALSE);

					// Poll time

					nPollTime = GetDlgItemInt(hWnd, IDC_POLLTIME, NULL, FALSE);

                    // Default CD device

                    nCount = 0;
                    nSel = SendDlgItemMessage(hWnd, IDC_DEFAULTDEVICE, CB_GETCURSEL, 0, 0);

                    for (int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                        if (abDevices[nLoop - 'A']) {
                            if (nCount == nSel)
                                nDefaultDevice = nLoop - 'A';

                            nCount ++;
                        }
                    }                  

                    // External command

                    GetWindowText(GetDlgItem(hWnd, IDC_EXTERNALCOMMAND), zExternalCommand, MAX_PATH);
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


void SetTooltipCaptionBoxes(HWND hWnd, BOOL bTooltip)
{
    int nTimeOptions;
    int nCDInfoOptions;

    if (bTooltip) {
        nTimeOptions = nTooltipTimeOptions;
        nCDInfoOptions = nTooltipCDInfoOptions;
    }
    else {
        nTimeOptions = nCaptionTimeOptions;
        nCDInfoOptions = nCaptionCDInfoOptions;
    }

    // Time settings
    if (!nTimeOptions)
        SendDlgItemMessage(hWnd, IDC_TIME_NONE, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_TIME_NONE, BM_SETCHECK, 0, 0);
    if (nTimeOptions & OPTIONS_TIME_TRACK)
        SendDlgItemMessage(hWnd, IDC_TIME_TRACK, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_TIME_TRACK, BM_SETCHECK, 0, 0);
    if (nTimeOptions & OPTIONS_TIME_TRACKREM)
        SendDlgItemMessage(hWnd, IDC_TIME_TRACKREM, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_TIME_TRACKREM, BM_SETCHECK, 0, 0);
    if (nTimeOptions & OPTIONS_TIME_CD)
        SendDlgItemMessage(hWnd, IDC_TIME_CD, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_TIME_CD, BM_SETCHECK, 0, 0);
    if (nTimeOptions & OPTIONS_TIME_CDREM)
        SendDlgItemMessage(hWnd, IDC_TIME_CDREM, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_TIME_CDREM, BM_SETCHECK, 0, 0);

    // CD Info settings

    if (nCDInfoOptions & OPTIONS_CDINFO_ARTIST)
        SendDlgItemMessage(hWnd, IDC_CDINFO_ARTIST, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_CDINFO_ARTIST, BM_SETCHECK, 0, 0);
	if (nCDInfoOptions & OPTIONS_CDINFO_TITLE)
        SendDlgItemMessage(hWnd, IDC_CDINFO_TITLE, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_CDINFO_TITLE, BM_SETCHECK, 0, 0);
	if (nCDInfoOptions & OPTIONS_CDINFO_TRACKTITLE)
        SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKTITLE, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKTITLE, BM_SETCHECK, 0, 0);
	if (nCDInfoOptions & OPTIONS_CDINFO_TRACKNO)
        SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKNO, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKNO, BM_SETCHECK, 0, 0);
	if (nCDInfoOptions & OPTIONS_CDINFO_TRACKLENGTH)
        SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKLENGTH, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKLENGTH, BM_SETCHECK, 0, 0);
}


void GetTooltipCaptionBoxes(HWND hWnd, BOOL bTooltip)
{
    int nTimeOptions;
    int nCDInfoOptions;

    // Time settings

    nTimeOptions = 0;

    if (SendDlgItemMessage(hWnd, IDC_TIME_NONE, BM_GETCHECK, 0, 0))
        nTimeOptions = 0;
    else if (SendDlgItemMessage(hWnd, IDC_TIME_TRACK, BM_GETCHECK, 0, 0))
        nTimeOptions = OPTIONS_TIME_TRACK;
    else if (SendDlgItemMessage(hWnd, IDC_TIME_TRACKREM, BM_GETCHECK, 0, 0))
        nTimeOptions = OPTIONS_TIME_TRACKREM;
    else if (SendDlgItemMessage(hWnd, IDC_TIME_CD, BM_GETCHECK, 0, 0))
        nTimeOptions = OPTIONS_TIME_CD;
    else if (SendDlgItemMessage(hWnd, IDC_TIME_CDREM, BM_GETCHECK, 0, 0))
        nTimeOptions = OPTIONS_TIME_CDREM;

	// CD Info settings

    nCDInfoOptions = 0;

    if (SendDlgItemMessage(hWnd, IDC_CDINFO_ARTIST, BM_GETCHECK, 0, 0))
        nCDInfoOptions |= OPTIONS_CDINFO_ARTIST;
    if (SendDlgItemMessage(hWnd, IDC_CDINFO_TITLE, BM_GETCHECK, 0, 0))
        nCDInfoOptions |= OPTIONS_CDINFO_TITLE;
    if (SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKTITLE, BM_GETCHECK, 0, 0))
        nCDInfoOptions |= OPTIONS_CDINFO_TRACKTITLE;
    if (SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKNO, BM_GETCHECK, 0, 0))
        nCDInfoOptions |= OPTIONS_CDINFO_TRACKNO;
    if (SendDlgItemMessage(hWnd, IDC_CDINFO_TRACKLENGTH, BM_GETCHECK, 0, 0))
        nCDInfoOptions |= OPTIONS_CDINFO_TRACKLENGTH;

    if (bTooltip) {
        nTooltipTimeOptions = nTimeOptions;
        nTooltipCDInfoOptions = nCDInfoOptions;
    }
    else {
        nCaptionTimeOptions = nTimeOptions;
        nCaptionCDInfoOptions = nCDInfoOptions;
    }
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
            SetTooltipCaptionBoxes(hWnd, TRUE);

            SendDlgItemMessage(hWnd, IDC_TOOLTIP, BM_SETCHECK, 1, 0);

            if (nOptions & OPTIONS_SHOWONCAPTION)
                SendDlgItemMessage(hWnd, IDC_SHOWONCAPTION, BM_SETCHECK, 1, 0);
            if (nOptions & OPTIONS_USEFONT)
                SendDlgItemMessage(hWnd, IDC_USEFONT, BM_SETCHECK, 1, 0);
        }
		break;

        case WM_COMMAND: {
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
                sFont.lpLogFont = &sCaptionFont;
                sFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
                sFont.rgbColors = nCaptionFontColor;
            
                if (ChooseFont(&sFont))
                    nCaptionFontColor = sFont.rgbColors;
            }
            else if (LOWORD(wParam) == IDC_TOOLTIP) {
                GetTooltipCaptionBoxes(hWnd, FALSE);
                SetTooltipCaptionBoxes(hWnd, TRUE);

//                SendDlgItemMessage(hWnd, IDC_CAPTION, BM_SETCHECK, 0, 0);
            }
            else if (LOWORD(wParam) == IDC_CAPTION) {
                GetTooltipCaptionBoxes(hWnd, TRUE);
                SetTooltipCaptionBoxes(hWnd, FALSE);

//                SendDlgItemMessage(hWnd, IDC_TOOLTIP, BM_SETCHECK, 0, 0);
            }
        }
        break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {
                    GetTooltipCaptionBoxes(hWnd, SendDlgItemMessage(hWnd, IDC_TOOLTIP, BM_GETCHECK, 0, 0));
                        
                    if (SendDlgItemMessage(hWnd, IDC_SHOWONCAPTION, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_SHOWONCAPTION;
                    else
                        nOptions &= ~OPTIONS_SHOWONCAPTION;
                    if (SendDlgItemMessage(hWnd, IDC_USEFONT, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_USEFONT;
                    else {
                        NONCLIENTMETRICS sMetrics;                       
                        sMetrics.cbSize = sizeof(sMetrics);

                        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &sMetrics, FALSE);

                        sMetrics.lfCaptionFont.lfItalic = !sMetrics.lfCaptionFont.lfItalic;

                        memcpy(&sCaptionFont, &sMetrics.lfCaptionFont, sizeof(sCaptionFont));

                        nOptions &= ~OPTIONS_USEFONT;
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
            if (nOptions & OPTIONS_USECDDB) {
                SendDlgItemMessage(hWnd, IDC_USEINI, BM_SETCHECK, 0, 0);
                SendDlgItemMessage(hWnd, IDC_USECDDB, BM_SETCHECK, 1, 0);
            }
            else {
                SendDlgItemMessage(hWnd, IDC_USEINI, BM_SETCHECK, 1, 0);
                SendDlgItemMessage(hWnd, IDC_USECDDB, BM_SETCHECK, 0, 0);
            }

            if (nOptions & OPTIONS_QUERYLOCAL)
                SendDlgItemMessage(hWnd, IDC_QUERYLOCAL, BM_SETCHECK, 1, 0);
            if (nOptions & OPTIONS_QUERYREMOTE)
                SendDlgItemMessage(hWnd, IDC_QUERYREMOTE, BM_SETCHECK, 1, 0);
            if (nOptions & OPTIONS_STORELOCAL)
                SendDlgItemMessage(hWnd, IDC_STORELOCAL, BM_SETCHECK, 1, 0);
            if (nOptions & OPTIONS_STORERESULT)
                SendDlgItemMessage(hWnd, IDC_STORERESULT, BM_SETCHECK, 1, 0);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    if (SendDlgItemMessage(hWnd, IDC_USECDDB, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_USECDDB;
                    else
                        nOptions &= ~OPTIONS_USECDDB;
                    if (SendDlgItemMessage(hWnd, IDC_QUERYLOCAL, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_QUERYLOCAL;
                    else
                        nOptions &= ~OPTIONS_QUERYLOCAL;
                    if (SendDlgItemMessage(hWnd, IDC_QUERYREMOTE, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_QUERYREMOTE;
                    else
                        nOptions &= ~OPTIONS_QUERYREMOTE;
                    if (SendDlgItemMessage(hWnd, IDC_STORELOCAL, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_STORELOCAL;
                    else
                        nOptions &= ~OPTIONS_STORELOCAL;
                    if (SendDlgItemMessage(hWnd, IDC_STORERESULT, BM_GETCHECK, 0, 0))
                        nOptions |= OPTIONS_STORERESULT;
                    else
                        nOptions &= ~OPTIONS_STORERESULT;
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
        
			SetWindowText(GetDlgItem(hWnd, IDC_LOCALPATH), zCDDBPath);
			SendMessage(GetDlgItem(hWnd, IDC_LOCALTYPE), CB_SETCURSEL, nCDDBType-1, 0);

            if (nCDDBOptions & OPTIONS_CDDB_STORECOPYININI)
                SendDlgItemMessage(hWnd, IDC_STOREININI, BM_SETCHECK, 1, 0);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    GetWindowText(GetDlgItem(hWnd, IDC_LOCALPATH), zCDDBPath, 256);
                    nCDDBType = SendMessage(GetDlgItem(hWnd, IDC_LOCALTYPE), CB_GETCURSEL, 0, 0) + 1;
            
                    if (SendDlgItemMessage(hWnd, IDC_STOREININI, BM_GETCHECK, 0, 0))
                        nCDDBOptions |= OPTIONS_CDDB_STORECOPYININI;
                    else
                        nCDDBOptions &= ~OPTIONS_CDDB_STORECOPYININI;

                    // Fix path

                    if (strlen(zCDDBPath) && zCDDBPath[strlen(zCDDBPath) - 1] != '\\')
                        strcat(zCDDBPath, "\\");

                    // Check path!

                    if (strlen(zCDDBPath)) {
                        char zPath[256];
                        WIN32_FIND_DATA sFind;
                        HANDLE hFind;

                        sprintf(zPath, "%s*.*", zCDDBPath);
                        hFind = FindFirstFile(zPath, &sFind);
                        if (hFind == INVALID_HANDLE_VALUE) {
                            MessageBox(hWnd, "The path specified for the local database is invalid!", APPNAME, MB_OK);
            
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
    }
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
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.celestial.com");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.sonic.net");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "smurfy.tcimet.net");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.schoneal.com");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.dartmouth.edu");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "sunsite.unc.edu");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.shu.ac.uk");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.ton.tut.fi");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.westel.hu");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.sai.msu.su");
			SendDlgItemMessage(hWnd, IDC_REMOTESERVER, CB_ADDSTRING, 0, (LPARAM) "cddb.ieo.nctu.edu.tw");
        
			SetWindowText(GetDlgItem(hWnd, IDC_REMOTESERVER), zRemoteServer);
			SetDlgItemInt(hWnd, IDC_REMOTEPORT, nRemotePort, FALSE);

			SetDlgItemInt(hWnd, IDC_REMOTETIMEOUT, nRemoteTimeout, FALSE);

            SetWindowText(GetDlgItem(hWnd, IDC_HTTPPATH), zRemoteHTTPPath);
			SetWindowText(GetDlgItem(hWnd, IDC_PROXYSERVER), zRemoteProxyServer);
			SetDlgItemInt(hWnd, IDC_PROXYPORT, nRemoteProxyPort, FALSE);

            SetWindowText(GetDlgItem(hWnd, IDC_USER), zProxyUser);
			SetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), zProxyPassword);

            if (nCDDBOptions & OPTIONS_CDDB_USEHTTP)  {
                SendDlgItemMessage(hWnd, IDC_PROTHTTP, BM_SETCHECK, 1, 0);
                SendDlgItemMessage(hWnd, IDC_PROTCDDB, BM_SETCHECK, 0, 0);
            }
            else {
                SendDlgItemMessage(hWnd, IDC_PROTHTTP, BM_SETCHECK, 0, 0);
                SendDlgItemMessage(hWnd, IDC_PROTCDDB, BM_SETCHECK, 1, 0);
            }

            if (nCDDBOptions & OPTIONS_CDDB_USEPROXY) 
                SendDlgItemMessage(hWnd, IDC_USEPROXY, BM_SETCHECK, 1, 0);                  

            if (nCDDBOptions & OPTIONS_CDDB_USEAUTHENTICATION) 
                SendDlgItemMessage(hWnd, IDC_USEAUTHENTICATION, BM_SETCHECK, 1, 0);                  

            if (nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD) 
                SendDlgItemMessage(hWnd, IDC_ASKFORPASSWORD, BM_SETCHECK, 1, 0);                  

            Update_Remote(hWnd);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    if (SendDlgItemMessage(hWnd, IDC_USEHTTP, BM_GETCHECK, 0, 0))
                        nCDDBOptions |= OPTIONS_CDDB_USEHTTP;
                    else
                        nCDDBOptions &= ~OPTIONS_CDDB_USEHTTP;
                    
                    if (SendDlgItemMessage(hWnd, IDC_USEPROXY, BM_GETCHECK, 0, 0))
                        nCDDBOptions |= OPTIONS_CDDB_USEPROXY;
                    else
                        nCDDBOptions &= ~OPTIONS_CDDB_USEPROXY;

                    if (SendDlgItemMessage(hWnd, IDC_USEAUTHENTICATION, BM_GETCHECK, 0, 0) &&
                        nCDDBOptions & OPTIONS_CDDB_USEPROXY)
                        nCDDBOptions |= OPTIONS_CDDB_USEAUTHENTICATION;
                    else
                        nCDDBOptions &= ~OPTIONS_CDDB_USEAUTHENTICATION;

                    if (SendDlgItemMessage(hWnd, IDC_ASKFORPASSWORD, BM_GETCHECK, 0, 0) &&
                        nCDDBOptions & OPTIONS_CDDB_USEPROXY)
                        nCDDBOptions |= OPTIONS_CDDB_ASKFORPASSWORD;
                    else
                        nCDDBOptions &= ~OPTIONS_CDDB_ASKFORPASSWORD;

                    nRemoteTimeout = GetDlgItemInt(hWnd, IDC_REMOTETIMEOUT, NULL, FALSE);

                    GetWindowText(GetDlgItem(hWnd, IDC_REMOTESERVER), zRemoteServer, 256);
                    nRemotePort = GetDlgItemInt(hWnd, IDC_REMOTEPORT, NULL, FALSE);

                    GetWindowText(GetDlgItem(hWnd, IDC_HTTPPATH), zRemoteHTTPPath, 256);
                    GetWindowText(GetDlgItem(hWnd, IDC_PROXYSERVER), zRemoteProxyServer, 256);
                    GetWindowText(GetDlgItem(hWnd, IDC_USER), zProxyUser, 256);
                    GetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), zProxyPassword, 256);

                    nRemoteProxyPort = GetDlgItemInt(hWnd, IDC_PROXYPORT, NULL, FALSE);

                    // Fix HTTP path

                    if (zRemoteHTTPPath[0] != 0 && zRemoteHTTPPath[0] != '/') {
                        char zTmp[100];

                        strcpy(zTmp, zRemoteHTTPPath);
                        strcpy(zRemoteHTTPPath, "/");
                        strcat(zRemoteHTTPPath, zTmp);
                    }

                    if (zRemoteHTTPPath[0] != 0 && zRemoteHTTPPath[strlen(zRemoteHTTPPath)-1] == '/')
                        zRemoteHTTPPath[strlen(zRemoteHTTPPath)-1] = 0;
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
			SetWindowText(GetDlgItem(hWnd, IDC_EMAILSERVER), zRemoteEmailServer);
			SetWindowText(GetDlgItem(hWnd, IDC_EMAILADDRESS), zEmailAddress);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    GetWindowText(GetDlgItem(hWnd, IDC_EMAILSERVER), zRemoteEmailServer, 256);
                    GetWindowText(GetDlgItem(hWnd, IDC_EMAILADDRESS), zEmailAddress, 256);
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
            int nLoop;

            for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++) {
                SendMessage(GetDlgItem(hWnd, IDC_CATEGORIES), LB_ADDSTRING, 0, (LPARAM) ppzCategories[nLoop]);
            }

            EnableWindow(GetDlgItem(hWnd, IDC_SETNAME), FALSE);
            EnableWindow(GetDlgItem(hWnd, IDC_ADD), FALSE);
        }
		break;

        case WM_NOTIFY: {
    		switch (((NMHDR FAR *) lParam)->code) {
                case PSN_APPLY: {               
                    int nCount;
                    int nLoop;
                    
                    // Categories

                    char zKey[80];
                    nCount = SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETCOUNT, 0, 0);

                    for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++)
                        free(ppzCategories[nLoop]);
                    free(ppzCategories);

                    ppzCategories = (char**)malloc(nCount * sizeof(char*));
                    for (nLoop = 0 ; nLoop < nCount ; nLoop ++)
                        ppzCategories[nLoop] = (char*)malloc(80*sizeof(char));

                    for (nLoop = 0 ; nLoop < nCount ; nLoop ++)
                        SendDlgItemMessage(hWnd, IDC_CATEGORIES, LB_GETTEXT, nLoop, (LPARAM) ppzCategories[nLoop]);

                    nNumCategories = nCount;

                    for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++) {
                        sprintf(zKey, "Category%d", nLoop);
                        WritePrivateProfileString("NTFY_CD", zKey, ppzCategories[nLoop], "CDPLAYER.INI");
                    }

                    WritePrivateProfileInt("NTFY_CD", "NumCategories", nNumCategories, "CDPLAYER.INI");
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


void DoOptions()
{
    PROPSHEETHEADER sPropSheet;
    PROPSHEETPAGE asPropPages[NUM_TABS];
    int nLoop;

    bInOptionsDlg = TRUE;

    // Setup pages

    for (nLoop = 0 ; nLoop < NUM_TABS ; nLoop ++) {
        memset(&asPropPages[nLoop], 0, sizeof(PROPSHEETPAGE));
        asPropPages[nLoop].dwSize = sizeof(PROPSHEETPAGE);
        asPropPages[nLoop].dwFlags = PSP_USETITLE | PSP_USECALLBACK;
        asPropPages[nLoop].hInstance = hMainInstance;
        asPropPages[nLoop].pszTemplate = MAKEINTRESOURCE(asOptionsTabs[nLoop].nDialogTemplate);
        asPropPages[nLoop].pszTitle = asOptionsTabs[nLoop].pzTabName;
        asPropPages[nLoop].pfnDlgProc = asOptionsTabs[nLoop].pfnDlgProc;
    }

    // Setup sheet
    
    memset(&sPropSheet, 0, sizeof(PROPSHEETHEADER));
    sPropSheet.dwSize = sizeof(PROPSHEETHEADER);
    sPropSheet.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_USEICONID;
    sPropSheet.hwndParent = NULL;
    sPropSheet.hInstance = hMainInstance;
    sPropSheet.pszCaption = (LPSTR)"Options";
    sPropSheet.pszIcon = MAKEINTRESOURCE(IDI_MAIN);
    sPropSheet.nPages = NUM_TABS;
    sPropSheet.ppsp = (LPCPROPSHEETPAGE)&asPropPages;

    bInitApplyDone = FALSE;

    PropertySheet(&sPropSheet);

    // Fix popup menu

    InitMenu();

	KillTimer(hMainWnd, 1);
	SetTimer(hMainWnd, 1, nPollTime*1000, NULL);

	SaveConfig();
    
    bInOptionsDlg = FALSE;

    if ((nOptions & OPTIONS_USECDDB) && (!strlen(zEmailAddress) || !strchr(zEmailAddress, '@'))) {
        MessageBox(NULL, "You should enter a valid e-mail address", APPNAME, MB_OK);

        return;
    }
    if ((nOptions & OPTIONS_QUERYLOCAL) && !strlen(zCDDBPath) && (nOptions & OPTIONS_USECDDB)) {
        MessageBox(NULL, "Query local database requires a local database path", APPNAME, MB_OK);

        return;
    }
    if ((nOptions & OPTIONS_QUERYREMOTE) && !strlen(zRemoteServer)) {
        MessageBox(NULL, "Query remote database server requires a server name", APPNAME, MB_OK);

        return;
    }
    if ((nOptions & OPTIONS_QUERYREMOTE) && !nRemotePort) {
        MessageBox(NULL, "Query remote database server requires a server port", APPNAME, MB_OK);

        return;
    }
    if ((nOptions & OPTIONS_STORELOCAL) && !(nOptions & OPTIONS_QUERYLOCAL)) {
        MessageBox(NULL, "Store local requires query local! (Wouldn't do much good otherwise would it?)", APPNAME, MB_OK);

        return;
    }
    if ((nOptions & OPTIONS_STORERESULT) && !(nOptions & OPTIONS_QUERYLOCAL)) {
        MessageBox(NULL, "Store result requires query local! (Wouldn't do much good otherwise would it?)", APPNAME, MB_OK);

        return;
    }
}


