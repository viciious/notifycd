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
// History:
// 960312 Mats      Initial coding
// 980221 Mats		Moved history to changes.txt
//
///////////////////////////////////////////////////////////////////////////////

#include "common.h"

#include <commctrl.h>
#include <shellapi.h>
#include <dbt.h>

#include "res/resource.h"

#define ALLOCATE

#include "ntfy_cd.h"
#include "misc.h"
#include "db.h"
#include "mci.h"
#include "options.h"
#include "dbdlg.h"
#include "infodlg.h"

char szCredits[] =  APPNAME " for Windows NT and Windows 95\r\n"
                    "Copyright (c) 1996-1998, Mats Ljungqvist <mlt@cyberdude.com>\r\n"
                    "\r\n"
                    "This program is free software; you can redistribute it and/or modify\r\n"
                    "it under the terms of the GNU General Public License as published by\r\n"
                    "the Free Software Foundation; either version 2 of the License, or\r\n"
                    "(at your option) any later version.\r\n"
                    "\r\n"
                    "This program is distributed in the hope that it will be useful,\r\n"
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of\r\n"
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\r\n"
                    "GNU General Public License for more details.\r\n"
                    "\r\n"
                    "You should have received a copy of the GNU General Public License\r\n"
                    "along with this program; if not, write to the Free Software\r\n"
                    "Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\r\n"
                    "\r\n"
                    "Initial idea and main programming: Mats Ljungqvist <mlt@cyberdude.com>\r\n"
                    "\r\n"
                    "Controls tab initially by: Acy James Stapp <astapp@io.com>\r\n"
                    "Hotkey idea: Shaun Ivory <shaun@ivory.org>\r\n"
                    "Popupmenu bitmap by: <vginkel@idirect.com>\r\n"
                    "\r\n"
                    "Mailing list services provided by: MIDRANGE dot COM <info@midrange.com>\r\n"
                    "\r\n"
                    "";

GLOBALSTRUCT gs;

BOOL CheckCategory(char* pzCategory, unsigned int nCategory)
{
    unsigned int nLoop;

    if (nCategory == -1)
        return TRUE;

    for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++) {
        if (!stricmp(gs.ppzCategories[nLoop], pzCategory) && nCategory == nLoop)
            return TRUE;
    }


    return FALSE;
}


int GetRandTrack()
{	
	unsigned int nTrack, nLoop;
    BOOL bEqual = FALSE;
	unsigned int nTmpLast = 0xFFFFFFFF;
    
	DebugPrintf("CDPlayRandTrack()");

    if (gs.nLastRandomTrack == gs.di[0].nProgrammedTracks) {
		if (gs.nLastRandomTrack > 0)
			nTmpLast = gs.pnLastRandomTracks[gs.nLastRandomTrack-1];

        gs.nLastRandomTrack = 0;
		DebugPrintf("All tracks in the playlist played on random play");

		if (gs.state.bRepeat) 
			DebugPrintf("Resetting random played tracks and continue to play random since we have the repeat flag set");
		else {
			DebugPrintf("Stopping random play since we have played all tracks and we don't have repeat flag set");

			return -1;
		}
	}

	nTrack = gs.di[0].nCurrTrack;
	while(!bEqual) {
        bEqual = TRUE;

		nTrack = rand() % gs.di[0].nProgrammedTracks;

		// Check temporary track if it's not -1 so we don't play the same track twice
		// in a row when the random of the playlist is restarted due to repeat

		if (nTmpLast != 0xFFFFFFFF && nTrack == nTmpLast) {
			bEqual = FALSE;

			continue;
		}

        // Check last random tracks

        for (nLoop = 0 ; nLoop < gs.nLastRandomTrack ; nLoop ++) {
            if (gs.pnLastRandomTracks[nLoop] == nTrack)
                bEqual = FALSE;
        }
    }

    // Update last random tracks

    gs.pnLastRandomTracks[gs.nLastRandomTrack] = nTrack;
    if (gs.nLastRandomTrack <= gs.di[0].nProgrammedTracks)
        gs.nLastRandomTrack ++;

	DebugPrintf("Number of random tracks played are %d", gs.nLastRandomTrack);

	DebugPrintf("Random track to play is %d", nTrack);

	return nTrack;
}


void PlayRandTrack() 
{
	int nTrack;

	nTrack = GetRandTrack();
	if (nTrack != -1)
		CDPlay(gs.wDeviceID, nTrack);
}


void UpdateTooltipOrCaption(DISCINFO* psDI,
							BOOL bTooltipOrCaption, 
							char* pzStr)
{
    char szStr[512];

    if (!gs.state.bInit)
        return;

    if (!psDI->zMCIID[0] || !gs.state.bAudio) {
        strcpy(szStr, "No audio CD");

        if (strcmp(szStr, gs.szToolTip)) {
            strcpy(gs.szToolTip, szStr);
            NotifyModify(gs.hMainWnd, 100, gs.hTrayIcon, gs.szToolTip);
        }
        return;
    }

	if (bTooltipOrCaption)
		ParseDiscInformationFormat(psDI, gs.szTooltipFormat, szStr);
	else
		ParseDiscInformationFormat(psDI, gs.szCaptionFormat, szStr);

    if (gs.state.bPaused)
        strcat(szStr, " [pause]");
    else if (!gs.state.bPlaying)
        strcat(szStr, " [stop]");

    strcpy(pzStr, szStr);
}


void UpdateMenu(DISCINFO* psDI, HMENU hMenu)
{
    EnterCriticalSection(&gs.sDiscInfoLock);

    if (gs.state.bAudio && gs.state.bInit && gs.state.bMediaPresent) {
	    if (gs.nOptions & OPTIONS_ARTISTINMENU) {
		    char szTmp[256];

		    strcpy(szTmp, psDI->pzArtist);
		    if (psDI->pzTitle && *psDI->pzTitle) {
			    strcat(szTmp, " - ");
			    strcat(szTmp, psDI->pzTitle);
		    }

			CheckAmpersand(szTmp, FALSE);

		    ModifyMenu(hMenu, 0, MF_STRING | MF_BYPOSITION, 9999, szTmp);
	    }

        EnableMenuItem(hMenu, IDM_PLAY, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_PAUSE, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_STOP, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_NEXT, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_PREV, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, 7, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_ENABLED);

        EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (gs.state.bProgrammed) {
            if (gs.state.bPlayWhole) {
                EnableMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_ENABLED);
                CheckMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_CHECKED);
            }
            else {
                EnableMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_ENABLED);
                CheckMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
        else
            EnableMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (gs.state.bRandomize)
            EnableMenuItem(hMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        else
            EnableMenuItem(hMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_ENABLED);

        // Enable stuff in Other menu

        EnableMenuItem(hMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_SKIP, MF_BYCOMMAND | MF_ENABLED);

        if (!gs.state.bPlayWhole && !gs.bInInfoDlg)
            EnableMenuItem(hMenu, IDM_INFO, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_INFO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!gs.bInDBDlg)
            EnableMenuItem(hMenu, IDM_DB, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_DB, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!gs.bInSkipDlg)
            EnableMenuItem(hMenu, IDM_SKIP, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_SKIP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!gs.bInSetAbsTrackPosDlg)
            EnableMenuItem(hMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!gs.bInOptionsDlg)
            EnableMenuItem(hMenu, IDM_OPTIONS, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_OPTIONS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!gs.bInAboutDlg)
            EnableMenuItem(hMenu, IDM_ABOUT, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_ABOUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        if (!gs.state.bPlaying && !gs.state.bPaused) {
            EnableMenuItem(hMenu, IDM_STOP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PAUSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hMenu, IDM_NEXT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PREV, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

            // Disable stuff in Other menu

            EnableMenuItem(hMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

            EnableMenuItem(hMenu, IDM_SKIP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

		if (psDI->ppzTracks && psDI->ppzTrackLen) {
			HMENU hSubMenu;
			char szTmp[300];

            // Update curr track
            UpdateDiscInformation(psDI, FALSE, TRUE, NULL);

            if (!(gs.nOptions & OPTIONS_TRACKSMENUCOLUMN))
                hSubMenu = GetSubMenu(GetSubMenu(gs.hTrackMenu, 0), gs.nMenuIndexTracks);
            else
                hSubMenu = GetSubMenu(gs.hTrackMenu, 0);
			for (unsigned int nLoop = 0 ; nLoop < psDI->nProgrammedTracks ; nLoop ++) {
                int nFlags;

                if (!(gs.nOptions & OPTIONS_TRACKSMENUCOLUMN) || gs.bBrokenTracksMenu)
                    sprintf(szTmp, "%d. %s\t[%s]", psDI->pnProgrammedTracks[nLoop]+1, psDI->ppzTracks[psDI->pnProgrammedTracks[nLoop]], psDI->ppzTrackLen[psDI->pnProgrammedTracks[nLoop]]);
                else
                    sprintf(szTmp, "%d. %s", psDI->pnProgrammedTracks[nLoop]+1, psDI->ppzTracks[psDI->pnProgrammedTracks[nLoop]]);

				CheckAmpersand(szTmp, FALSE);

                nFlags = MF_BYCOMMAND | MF_STRING;
                if (nLoop == psDI->nCurrTrack)
                    nFlags |= MF_CHECKED;
                if (!nLoop && (gs.nOptions & OPTIONS_TRACKSMENUCOLUMN) && !gs.bBrokenTracksMenu)
                    nFlags |= MF_MENUBARBREAK;

                ModifyMenu(hSubMenu, IDM_TRACKS + nLoop, nFlags, IDM_TRACKS + nLoop, szTmp);
			}
		}
    }
    else {
	    if (gs.nOptions & OPTIONS_ARTISTINMENU)
		    ModifyMenu(hMenu, 0, MF_STRING | MF_BYPOSITION, 9999, "No Disc");

        EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | MF_ENABLED);

        EnableMenuItem(hMenu, IDM_PLAY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PAUSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_STOP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_NEXT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PREV, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_INFO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!(gs.nOptions & OPTIONS_TRACKSMENUCOLUMN))
            EnableMenuItem(hMenu, gs.nMenuIndexTracks, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        else {
            int nLoop;

            for (nLoop = 0 ; nLoop < 99 ; nLoop ++) {
                RemoveMenu(hMenu, IDM_TRACKS + nLoop, MF_BYCOMMAND);
                RemoveMenu(hMenu, IDM_TRACKS + nLoop, MF_BYCOMMAND);
            }
        }

        EnableMenuItem(hMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        // Disable stuff in Other menu

        if (gs.state.bProgrammed) {
            if (gs.state.bPlayWhole) {
                EnableMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_ENABLED | MF_CHECKED);
                CheckMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_CHECKED);
            }
            else {
                EnableMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_ENABLED);
                CheckMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
        else
            EnableMenuItem(hMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        EnableMenuItem(hMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        EnableMenuItem(hMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_SKIP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    }

    // Enable/Disable stuff depending on number of items in queue
    if (!gs.nNumberOfItemsInQueue) {
        EnableMenuItem(hMenu, IDM_GETQUEUEDITEMS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_REMOVEQUEUEDITEMS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    }
    else {
        EnableMenuItem(hMenu, IDM_GETQUEUEDITEMS, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_REMOVEQUEUEDITEMS, MF_BYCOMMAND | MF_ENABLED);
    }

    LeaveCriticalSection(&gs.sDiscInfoLock);
}


void StatusCheck(DISCINFO* psDI)
{
    BOOL bTmpMediaPresent;
    unsigned int nTmp;

    EnterCriticalSection(&gs.sDiscInfoLock);

    DebugPrintf("Status check!");
    DebugPrintf("Current device = %c", gs.nCurrentDevice + 'A');

    // If CDOpen has failed, try again
    if (!gs.state.bCDOpened) {
        CDClose(&gs.wDeviceID);

        if (!CDOpen(&gs.wDeviceID)) {
            gs.state.bMediaPresent = FALSE;
	        gs.state.bAudio = FALSE;
	        psDI->zCDDBID[0] = 0;
	        gs.state.bPaused = FALSE;
	        gs.state.bPlaying = FALSE;
	        psDI->nCurrTrack = 0;
	        gs.state.bInit = FALSE;

            LeaveCriticalSection(&gs.sDiscInfoLock);

            return;
        }
    }

    nTmp = CDGetStatus(gs.wDeviceID);
	if (gs.nStatus != nTmp) {
		gs.nStatus = nTmp;
	}

    bTmpMediaPresent = CDGetMediaPresent(gs.wDeviceID);
    if (!gs.state.bMediaPresent && bTmpMediaPresent) {
        DebugPrintf("Found media!");

        gs.state.bMediaPresent = TRUE;
	    gs.state.bPaused = FALSE;
	    gs.state.bPlaying = FALSE;
	    psDI->nCurrTrack = 0;

        // If we don't remeber status, revert to normal playmode
        if (!(gs.nOptions & OPTIONS_REMEMBERSTATUS)) {
            gs.state.bRandomize = FALSE;
            gs.state.bPlayWhole = FALSE;
            gs.state.bRepeat = FALSE;
            gs.state.bRepeatTrack = FALSE;
        }

        if (!gs.state.bInit) {
			DWORD dwId;

DebugPrintf("Killing TIMER");
			KillTimer(gs.hMainWnd, 1);

            DiscInit(&gs.di[0]);

			// Create version check thread
			if (gs.nOptions & OPTIONS_AUTOCHECKFORNEWVERSION)
				CloseHandle(CreateThread(NULL, 0, CheckVersionThread, NULL, 0, &dwId));

            SetTimer(gs.hMainWnd, 1, gs.nPollTime*1000, NULL);

DebugPrintf("Restarting TIMER");            
        }

	    if (gs.nStatus == 2)
            CDResume(gs.wDeviceID);
    }
    else if (gs.state.bMediaPresent && !bTmpMediaPresent) {
        DebugPrintf("Media lost!");

        gs.state.bMediaPresent = FALSE;
	    gs.state.bAudio = FALSE;
	    psDI->zCDDBID[0] = 0;
	    psDI->zMCIID[0] = 0;
	    gs.state.bPaused = FALSE;
	    gs.state.bPlaying = FALSE;
	    psDI->nCurrTrack = 0;
	    gs.state.bInit = FALSE;
		gs.state.bRepeatTrack = FALSE;

        if (gs.nOptions & OPTIONS_EXITONCDREMOVE)
			DestroyWindow(gs.hMainWnd);
    }

    LeaveCriticalSection(&gs.sDiscInfoLock);
}


void UpdateDiscInformation(DISCINFO* psDI, BOOL bNotify, BOOL bTooltip, char* pzStr)
{
    EnterCriticalSection(&gs.sDiscInfoLock);

    // WARNING: This is also used by UpdatePopupMenu
    
    if (pzStr)
        pzStr[0] = 0;

    // If CD is playing
    if (gs.state.bPlaying) {
        unsigned int nOldTrack;
        unsigned int nTrack, nMin, nSec, nFrame;

        if (gs.state.bProgrammed)
            nOldTrack = psDI->nCurrTrack;
        else
            nOldTrack = 0;

	    psDI->nCurrTrack = CDGetCurrTrack(gs.wDeviceID) - 1;

        if (bNotify) {
            CDGetTime(gs.wDeviceID, 0, NULL, TRUE, &nTrack, &nMin, &nSec, &nFrame);
            if (nMin == 0 && nSec < 5)
                psDI->nCurrTrack = nTrack - 2;
        }

	    if (gs.state.bProgrammed) {
            // Scan upwards for active track...

            for (unsigned int nLoop = nOldTrack ; nLoop < psDI->nProgrammedTracks ; nLoop ++) {
                if (psDI->pnProgrammedTracks[nLoop] == psDI->nCurrTrack) {
                    psDI->nCurrTrack = nLoop;
                    break;
                }
            }
        }
    }

    if (pzStr) {
        UpdateTooltipOrCaption(psDI, bTooltip, pzStr);

		CheckAmpersand (pzStr, bTooltip);

        if (bTooltip && strcmp(pzStr, gs.szToolTip)) {
            StringCpyZ(gs.szToolTip, pzStr, sizeof(gs.szToolTip));
            NotifyModify(gs.hMainWnd, 100, gs.hTrayIcon, gs.szToolTip);
        }
    }

    LeaveCriticalSection(&gs.sDiscInfoLock);
}


/////////////////////////////////////////////////////////////////////
//
// CREDITS!
//
/////////////////////////////////////////////////////////////////////


BOOL CALLBACK CreditsDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM/*  lParam*/)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            CenterWindow(hWnd);
            
            SetWindowText(GetDlgItem(hWnd, IDC_CREDITS), szCredits);
        }
		break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                case IDOK:
                    EndDialog(hWnd, TRUE);
                break;
            }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// ABOUT!
//
/////////////////////////////////////////////////////////////////////

HFONT hLinkFont;

BOOL CALLBACK AboutDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            char szStr[512];

            gs.bInAboutDlg = TRUE;

            CenterWindow(hWnd);

            sprintf(szStr, "About %s", APPNAME);
            SetWindowText(hWnd, szStr);

            sprintf(szStr, "Version %s", VERSION);
            SetWindowText(GetDlgItem(hWnd, IDC_VERSION), szStr);

            sprintf(szStr, "(%s)", MAIL_ADDRESS);
            SetWindowText(GetDlgItem(hWnd, IDC_EMAIL), szStr);

            sprintf(szStr, "%s", HOMEPAGE_URL);
            SetWindowText(GetDlgItem(hWnd, IDC_HOMEPAGE), szStr);
        }
		break;

        case WM_CTLCOLORSTATIC: {
            if ((HWND)lParam == GetDlgItem(hWnd, IDC_EMAIL) || (HWND)lParam == GetDlgItem(hWnd, IDC_HOMEPAGE)) {
                if (!hLinkFont) {
                    LOGFONT lf;

                    hLinkFont = (HFONT)SendDlgItemMessage(hWnd, IDC_EMAIL, WM_GETFONT, 0, 0);

                    GetObject(hLinkFont, sizeof(lf), &lf);

                    lf.lfUnderline = TRUE;

                    hLinkFont = CreateFontIndirect(&lf);
                }

                SelectObject((HDC)wParam, hLinkFont);
                SetTextColor((HDC)wParam, RGB(0, 0, 255));
                SetBkMode((HDC)wParam, TRANSPARENT);

                return (int)GetStockObject(HOLLOW_BRUSH);
            }

            return NULL;
        }
        break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDC_EMAIL:
					char zTemp[256];

					sprintf( zTemp, "mailto:%s", MAIL_ADDRESS );
                    ShellExecute(NULL, "open", zTemp, NULL, ".", SW_SHOWNORMAL);
                break;

                case IDC_HOMEPAGE: {
                    ShellExecute(NULL, "open", HOMEPAGE_URL, NULL, ".", SW_SHOWNORMAL);
                }
                break;

				case IDC_CHECKVERSION: {
					if (!CheckForNewVersion(FALSE)) 
						MessageBox(hWnd, "Check failed. Perhaps HTTP access wasn't acquired", APPNAME, MB_OK | MB_ICONERROR);
				}
				break;

                case IDC_CREDITS: {
                    DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_CREDITS), NULL, (DLGPROC)CreditsDlgProc);
                }
                break;

                case IDCANCEL:
                case IDOK:
                    gs.bInAboutDlg = FALSE;

                    EndDialog(hWnd, TRUE);
                break;
            }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// SET ABSOLUTE TRACK POSITION
//
/////////////////////////////////////////////////////////////////////

BOOL CALLBACK SetAbsTrackPosDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            gs.bInSetAbsTrackPosDlg = TRUE;

			CenterWindow(hWnd);

            UpdateTrackCombo(hWnd, IDC_TRACK);

            SendDlgItemMessage(hWnd, IDC_TRACK, CB_SETCURSEL, gs.di[0].nCurrTrack, 0);

            SetDlgItemInt(hWnd, IDC_MIN, 0, FALSE);

            SetDlgItemInt(hWnd, IDC_SEC, 0, FALSE);
        }
		break;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK: {
					unsigned int nTrack = SendDlgItemMessage(hWnd, IDC_TRACK, CB_GETCURSEL, 0, 0);
					unsigned int nMin = GetDlgItemInt(hWnd, IDC_MIN, NULL, FALSE);
					unsigned int nSec = GetDlgItemInt(hWnd, IDC_SEC, NULL, FALSE);

                    if (nMin * 60 + nSec <= gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[nTrack]])
                        CDPlayPos(gs.wDeviceID, nTrack, nMin, nSec);
                    else
                        MessageBox(hWnd, "Invalid time", APPNAME, MB_OK | MB_ICONERROR);

                    gs.bInSetAbsTrackPosDlg = FALSE;
                }
                break;

                case IDCANCEL: {
                    gs.bInSetAbsTrackPosDlg = FALSE;

                    EndDialog(hWnd, FALSE);
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


/////////////////////////////////////////////////////////////////////
//
// SKIP
//
/////////////////////////////////////////////////////////////////////

BOOL bInDrag = FALSE;
BOOL bInDrop = FALSE;
char zSkipDlgID[32];

void UpdateSlider(HWND hWnd)
{
	unsigned int nTrack;
    unsigned int nMin;
    unsigned int nSec;
    unsigned int nFrame;
    char zTmp[32];

	if (gs.state.bPlaying) {
		if (!gs.state.bProgrammed && !gs.state.bRandomize)
			gs.di[0].nCurrTrack = CDGetCurrTrack(gs.wDeviceID) - 1;

		if (gs.di[0].nCurrTrack != -1) {
			SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]]));
			SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETPAGESIZE, 0, 10);
			SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETTICFREQ, 30, 0);

			SendDlgItemMessage(hWnd, IDC_SPIN, UDM_SETRANGE, 0, MAKELONG(0, gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]]));

			if (!bInDrop)
				SendDlgItemMessage(hWnd, IDC_TRACK, CB_SETCURSEL, gs.di[0].nCurrTrack, 0);

			sprintf(zTmp, "%02d:%02d", gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] / 60, gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] % 60);

			SendDlgItemMessage(hWnd, IDC_LEN, WM_SETTEXT, 0, (LPARAM) zTmp);

			if (!bInDrag) {
				CDGetTime(gs.wDeviceID, 0, NULL, TRUE, &nTrack, &nMin, &nSec, &nFrame);

				sprintf(zTmp, "%02d:%02d", nMin, nSec);

				SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);

				nSec += nMin * 60;
  
				SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETPOS, TRUE, nSec);
				SendDlgItemMessage(hWnd, IDC_SPIN, UDM_SETPOS, 0, MAKELONG(gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] - nSec, 0));
			}
		}
		else {
			SendDlgItemMessage(hWnd, IDC_TRACK, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) "No CD");
			SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) "No CD");
		}

		if (strcmp(gs.di[0].zCDDBID, zSkipDlgID)) {
			UpdateTrackCombo(hWnd, IDC_TRACK);

			strcpy(zSkipDlgID, gs.di[0].zCDDBID);
		}
	}
}


BOOL CALLBACK SkipDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            CenterWindow(hWnd);

            strcpy(zSkipDlgID, gs.di[0].zCDDBID);

            UpdateTrackCombo(hWnd, IDC_TRACK);

            UpdateSlider(hWnd);

            gs.bInSkipDlg = TRUE;

            SetTimer(hWnd, 1, 1000, NULL);
        }
		break;

        case WM_TIMER: {
            switch(wParam) {
                case 1:
                    UpdateSlider(hWnd);
                break;
            }
        }
        break;

        case WM_HSCROLL: {
            int nPos = HIWORD(wParam);

            if ((HWND) lParam == GetDlgItem(hWnd, IDC_SLIDER)) {
                switch(LOWORD(wParam)) {
                    case SB_THUMBTRACK: {
                        char zTmp[32];

                        bInDrag = TRUE;

                        sprintf(zTmp, "%02d:%02d", nPos / 60, nPos % 60);

                        SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);
                    }
                    break;
                    
                    case SB_THUMBPOSITION: {
                        CDPlayPos(gs.wDeviceID, gs.di[0].nCurrTrack, nPos / 60, nPos % 60);

                        bInDrag = FALSE;
                    }
                    break;

                    case SB_TOP:
                    case SB_BOTTOM:
                    case SB_LINEUP:
                    case SB_LINEDOWN:
                    case SB_PAGEDOWN: 
                    case SB_PAGEUP: {
                        char zTmp[32];

                        bInDrag = TRUE;

                        nPos = SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_GETPOS, 0, 0);

                        sprintf(zTmp, "%02d:%02d", nPos / 60, nPos % 60);

                        SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);
                    }
                    break;

                    case SB_ENDSCROLL:
                        nPos = SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_GETPOS, 0, 0);
                        CDPlayPos(gs.wDeviceID, gs.di[0].nCurrTrack, nPos / 60, nPos % 60);
                        bInDrag = FALSE;
                    break;
                }
            }
            else if ((HWND) lParam == GetDlgItem(hWnd, IDC_SPIN)) {
                switch(LOWORD(wParam)) {
                    case SB_ENDSCROLL: {
                        CDPlayPos(gs.wDeviceID, gs.di[0].nCurrTrack, (gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] - nPos) / 60, (gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] - nPos) % 60);
                        bInDrag = FALSE;
                    }
                    break;

                    case SB_THUMBPOSITION: {
                        char zTmp[32];

                        bInDrag = TRUE;

                        SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETPOS, TRUE, gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] - nPos + 1);
                        sprintf(zTmp, "%02d:%02d", (gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] - nPos + 1) / 60, (gs.di[0].pnTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]] - nPos + 1) % 60);
        
                        SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);
                    }
                    break;
                }
            }
        }
        break;

        case WM_COMMAND: {
            if ((HWND) lParam == GetDlgItem(hWnd, IDC_TRACK)) {
                if (HIWORD(wParam) == CBN_SELCHANGE && gs.bInSkipDlg) {
                    int nSel;

                    nSel = SendDlgItemMessage(hWnd, IDC_TRACK, CB_GETCURSEL, 0, 0);
                    if (nSel != CB_ERR)
                        CDPlay(gs.wDeviceID, nSel);
                }
                else if (HIWORD(wParam) == CBN_DROPDOWN)
                    bInDrop = TRUE;
                else if (HIWORD(wParam) == CBN_CLOSEUP)
                    bInDrop = FALSE;
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                switch(LOWORD(wParam)) {
                    case IDOK: {
                        gs.bInSkipDlg = FALSE;

					    EndDialog(hWnd, TRUE);
                    }
                    break;

                    case IDCANCEL: {
                        gs.bInSkipDlg = FALSE;

                        EndDialog(hWnd, FALSE);
                    }
                    break;
                }
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}



/////////////////////////////////////////////////////////////////////
//
// MAIN WINDOW!
//
/////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MainWindowProc(                       
    HWND hWnd,
    UINT nMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch(nMsg) {
		case WM_CREATE: {
			DWORD dwId;

			gs.hTrayIcon = gs.hIconStop;

			NotifyAdd(hWnd, 100, gs.hTrayIcon, gs.szToolTip);

			CDOpen(&gs.wDeviceID);

			SetTimer(hWnd, 1, gs.nPollTime*1000, NULL);
			SetTimer(hWnd, 3, 1000, NULL);

			if (gs.nOptions & OPTIONS_AUTOCHECKFORNEWVERSION)
				CloseHandle(CreateThread(NULL, 0, CheckVersionThread, NULL, 0, &dwId));
		}
		break;

		case WM_ENDSESSION: {
			if (wParam) {
				SaveConfig();

				DestroyWindow(hWnd);

				CDClose(&gs.wDeviceID);
			}
        }
        break;

        case WM_CLOSE:
            NotifyDelete(gs.hMainWnd, 100);

            PostQuitMessage(0);
        break;

        case MM_MCINOTIFY: {
            if (wParam == MCI_NOTIFY_SUCCESSFUL) {
                DebugPrintf("MCI success");

                if (gs.state.bPlayWhole && !gs.state.bRepeatTrack) {
                    gs.di[0].nCurrTrack = gs.di[0].nProgrammedTracks - 1;
                    ResetPlaylist(&gs.di[0], TRUE);
                }

                gs.state.bPlaying = FALSE;

				UpdateDiscInformation(&gs.di[0], TRUE, TRUE, NULL);

                if (gs.state.bRepeat && !gs.state.bRepeatTrack && !gs.state.bRandomize && 
					(!gs.state.bProgrammed || (gs.state.bProgrammed && gs.nNextProgrammedTrack == gs.di[0].nProgrammedTracks))) {
					DebugPrintf("Playing from first track on the playlist");

					CDPlay(gs.wDeviceID, 0);
				}
                else if (!gs.state.bRandomize && !gs.state.bRepeatTrack && gs.state.bProgrammed && gs.nNextProgrammedTrack < gs.di[0].nProgrammedTracks) {
					DebugPrintf("Playing next track on the playlist");

                    CDPlay(gs.wDeviceID, gs.nNextProgrammedTrack);
				}
                else if (gs.state.bRepeatTrack) {
                    DebugPrintf("Repeating track");

					CDPlay(gs.wDeviceID, gs.nRepeatTrack);
				}
				else if (gs.state.bRandomize) {
					DebugPrintf("Playing random track");

					PlayRandTrack();
				}
                else
                    gs.state.bPlaying = FALSE;
			}
            else if (wParam == MCI_NOTIFY_ABORTED) {
                DebugPrintf("MCI aborted");

                StatusCheck(&gs.di[0]);
            }
            else if (wParam == MCI_NOTIFY_FAILURE) {
                DebugPrintf("MCI failure");

                StatusCheck(&gs.di[0]);
            }
            else if (wParam == MCI_NOTIFY_SUPERSEDED) {
                DebugPrintf("MCI superseeded");

                StatusCheck(&gs.di[0]);
            }

            if (!gs.state.bPlaying && !gs.state.bPaused)
                gs.di[0].nCurrTrack = 0;
		}
		break;

        case WM_DISPLAYCHANGE: {
            DebugPrintf("WM_DISPLAYCHANGE");

            NotifyDelete(gs.hMainWnd, 100);
            NotifyAdd(gs.hMainWnd, 100, gs.hTrayIcon, gs.szToolTip);
        }
        break;

        case WM_DEVICECHANGE: {
			if ((gs.nOptions & OPTIONS_NOINSERTNOTIFICATION) && gs.bHasNotification)
				DebugPrintf("No insert notification enabled, skipping WM_DEVICECHANGE");
			else {
				switch(wParam) {
					case DBT_DEVICEARRIVAL: {
						DEV_BROADCAST_HDR* p = (DEV_BROADCAST_HDR*) lParam;
						DebugPrintf("Device Arrival: Type = %d", p->dbch_devicetype);

						if (p->dbch_devicetype == DBT_DEVTYP_VOLUME) {
							if (!gs.bHasNotification) {
								gs.bHasNotification = TRUE;

								DebugPrintf("Use of notification found!");
							}

							gs.di[0].nCurrTrack = 0;

                            StatusCheck(&gs.di[0]);
						}
						else
							DebugPrintf("Not a volume!");
					}
					break;

					case DBT_DEVICEREMOVECOMPLETE: {
						DEV_BROADCAST_HDR* p = (DEV_BROADCAST_HDR*) lParam;
						DebugPrintf("Device Remove Complete: Type = %d", p->dbch_devicetype);

						if (p->dbch_devicetype == DBT_DEVTYP_VOLUME) {
							if (!gs.bHasNotification) {
								gs.bHasNotification = TRUE;

								DebugPrintf("Use of notification found!");

								gs.nOptions &= ~OPTIONS_NOINSERTNOTIFICATION;
							}

							StatusCheck(&gs.di[0]);
						}
						else
							DebugPrintf("Not a volume!");
					}
					break;

//					default:
//				        DebugPrintf("WM_DEVICECHANGE: %x", wParam);
				}
            }
        }
        break;

        case WM_TIMER: {
            if (gs.bQueryThreadHasUpdatedDI) {
                EnterCriticalSection(&gs.sDiscInfoLock);

                gs.bQueryThreadHasUpdatedDI = FALSE;

                DebugPrintf("QueryThread has updated DiscInfo");

                GetDiscInfo(gs.wDeviceID, &gs.sQueryThreadDI);
                ResetPlaylist(&gs.sQueryThreadDI, TRUE);

                CheckProgrammed(gs.wDeviceID, &gs.sQueryThreadDI);

                FreeDiscInfo(&gs.di[0]);

                gs.di[0] = gs.sQueryThreadDI;

				RunExternalCommand(&gs.di[0]);

                LeaveCriticalSection(&gs.sDiscInfoLock);
            }

            if (wParam == 1) {              
                if (gs.bCreateWindowFinished && !gs.bFirstStatusCheckDone) {
                    StatusCheck(&gs.di[0]);

                    gs.bFirstStatusCheckDone = TRUE;
                }
/*                if (!gs.state.bMediaPresent || !gs.state.bAudio || (!gs.state.bPlaying))
                    StatusCheck();
                else*/ if (gs.nOptions & OPTIONS_NOINSERTNOTIFICATION)
                    StatusCheck(&gs.di[0]);

                // Check for correct icon
                if (gs.state.bMediaPresent && gs.state.bAudio && gs.state.bPlaying && !gs.state.bPaused && gs.hTrayIcon != gs.hIconPlay) {
                    gs.hTrayIcon = gs.hIconPlay;
                    
                    NotifyModify(gs.hMainWnd, 100, gs.hTrayIcon, gs.szToolTip);
                }
                else if (gs.state.bMediaPresent && gs.state.bAudio && !gs.state.bPlaying && !gs.state.bPaused && gs.hTrayIcon != gs.hIconStop) {
                    gs.hTrayIcon = gs.hIconStop;
                    
                    NotifyModify(gs.hMainWnd, 100, gs.hTrayIcon, gs.szToolTip);
                }
                else if (gs.state.bMediaPresent && gs.state.bAudio && !gs.state.bPlaying && gs.state.bPaused && gs.hTrayIcon != gs.hIconPause) {
                    gs.hTrayIcon = gs.hIconPause;
                    
                    NotifyModify(gs.hMainWnd, 100, gs.hTrayIcon, gs.szToolTip);
                }
                else if ((!gs.state.bMediaPresent || !gs.state.bAudio) && gs.hTrayIcon != gs.hIconNoAudio) {
                    gs.hTrayIcon = gs.hIconNoAudio;
                    
                    NotifyModify(gs.hMainWnd, 100, gs.hTrayIcon, gs.szToolTip);
                }
			}
            else if (wParam == 2) {
                KillTimer(hWnd, 2);

                if (gs.state.bMediaPresent) {
                    if (gs.nClicks > 0 && gs.nClicks <= 5 && gs.anBindings[gs.nClicks-1]) {
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(gs.anBindings[gs.nClicks-1], 0), 0L);
                    }
				}

                gs.nClicks = 0;
            }
            else if (wParam == 3) {
                if (gs.nOptions & OPTIONS_SHOWONCAPTION)
					DrawOnCaption();
            }
        }
        break;

        case WM_DESTROY:
            if (gs.nOptions & OPTIONS_STOPONEXIT)
	            CDStop(gs.wDeviceID);

            NotifyDelete(gs.hMainWnd, 100);

            PostQuitMessage(1);
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) >= IDM_TRACKS && LOWORD(wParam) < IDM_TRACKS + 100) {
                CDPlay(gs.wDeviceID, LOWORD(wParam) - IDM_TRACKS);
            }
            else if (LOWORD(wParam) >= IDM_DEVICES && LOWORD(wParam) <= IDM_DEVICES + 'Z') {
                CDStop(gs.wDeviceID);

                CDClose(&gs.wDeviceID);

				gs.state.bInit = FALSE;	// Vic: bugfix
                gs.nCurrentDevice = LOWORD(wParam) - IDM_DEVICES - 'A';

                DebugPrintf("Changing device to %c", (char)gs.nCurrentDevice + 'A');

                // Modify devices menu

	            HMENU hDevicesMenu;
                hDevicesMenu = GetSubMenu(GetSubMenu(gs.hTrackMenu, 0), gs.nMenuIndexDevices);

                if (hDevicesMenu) {
                    char szTmp[80];

                    for (unsigned int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                        if (gs.abDevices[nLoop - 'A']) {
                            sprintf(szTmp, "Drive %c:", nLoop);
                            if (nLoop - 'A' == gs.nCurrentDevice)
                                ModifyMenu(hDevicesMenu, IDM_DEVICES+nLoop, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_DEVICES+nLoop, szTmp);
                            else
                                ModifyMenu(hDevicesMenu, IDM_DEVICES+nLoop, MF_BYCOMMAND | MF_STRING, IDM_DEVICES+nLoop, szTmp);
                        }
                    }
                }

                StatusCheck(&gs.di[0]);

				DiscInit(&gs.di[0]);
            }
            else {
                switch(LOWORD(wParam)) {
                    case IDM_PLAY:
                        // Call status check to determine CD change if another instance
                        // sent us this command and we haven't detected the CD change!

                        StatusCheck(&gs.di[0]);

                        if (gs.state.bPlayWhole)
                            ResetPlaylist(&gs.di[0], TRUE);

                        if (gs.state.bMediaPresent && gs.state.bAudio) {
							if (!gs.state.bPaused) {
								CDPlay(gs.wDeviceID, gs.di[0].nCurrTrack);
							}
							else
								CDResume(gs.wDeviceID);
						}

                        StatusCheck(&gs.di[0]);
                    break;

                    case IDM_PLAYWHOLE: {
                        HMENU hSubMenu;

                        hSubMenu = GetSubMenu(gs.hTrackMenu, 0);

                        if (!gs.state.bPlayWhole) {
							CheckMenuItem(hSubMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_CHECKED);

                            ResetPlaylist(&gs.di[0], FALSE);
                        }
                        else {
							CheckMenuItem(hSubMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_UNCHECKED);

                            ResetPlaylist(&gs.di[0], TRUE);
                        }

                        CDPlay(gs.wDeviceID, 0);
                    }
                    break;

                    case IDM_STOP:
                        if (gs.state.bMediaPresent && gs.state.bAudio)
							CDStop(gs.wDeviceID);
                    break;

                    case IDM_PAUSE:
						if (gs.state.bMediaPresent && gs.state.bAudio) {
							if (!gs.state.bPaused && gs.state.bPlaying)
								CDPause(gs.wDeviceID);
							else if (gs.state.bPaused)
								CDResume(gs.wDeviceID);
							else if (!gs.state.bPaused && !gs.state.bPlaying)
								CDPlay(gs.wDeviceID, gs.di[0].nCurrTrack);
						}
                    break;

                    case IDM_NEXT:
						if (gs.state.bMediaPresent && gs.state.bAudio) {
							if (gs.state.bRandomize)
								PlayRandTrack();
							else {
								if (gs.di[0].nCurrTrack < gs.di[0].nProgrammedTracks - 1)
									gs.di[0].nCurrTrack ++;
								else
									gs.di[0].nCurrTrack = 0;
                
								if (!gs.state.bPaused && gs.state.bPlaying)
									CDPlay(gs.wDeviceID, gs.di[0].nCurrTrack);
								else if (!gs.state.bPaused && !gs.state.bPlaying)
									CDPlay(gs.wDeviceID, 0);
                                else if (gs.state.bPaused)
                                    CDResume(gs.wDeviceID);
							}
						}
                    break;

                    case IDM_PREV: {
						if (gs.state.bMediaPresent && gs.state.bAudio) {
							if (gs.state.bRandomize)
								PlayRandTrack();
							else {
                                unsigned int nTrack, nMin, nSec, nFrame;

                                if (!(gs.nOptions & OPTIONS_PREVALWAYSPREV))
                                    CDGetTime(gs.wDeviceID, 0, NULL, TRUE, &nTrack, &nMin, &nSec, &nFrame);

                                if ((gs.nOptions & OPTIONS_PREVALWAYSPREV) || (nMin < 1 && nSec <= 5)) {
                                    if (gs.di[0].nCurrTrack > 0)
									    gs.di[0].nCurrTrack --;
								    else
									    gs.di[0].nCurrTrack = gs.di[0].nProgrammedTracks - 1;
                                }
                                
								if (!gs.state.bPaused)
									CDPlay(gs.wDeviceID, gs.di[0].nCurrTrack);
							}
						}
				    }
                    break;

                    case IDM_OPEN: {
                        if (!gs.state.bCDOpened)
                            CDOpen(&gs.wDeviceID);

                        CDEject(gs.wDeviceID);

                        SendMessage(hWnd, WM_TIMER, 1, 0);
					}
                    break;

                    case IDM_CLOSE: {
        				CDLoad(gs.wDeviceID);

                        SendMessage(hWnd, WM_TIMER, 1, 0);
					}
                    break;

					case IDM_REPEAT: {
						HMENU hSubMenu;

						hSubMenu = GetSubMenu(gs.hTrackMenu, 0);

						if (!gs.state.bRepeat) {
							ModifyMenu(hSubMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_REPEAT, "Repeat");
							gs.state.bRepeat = TRUE;
						}
						else {
							ModifyMenu(hSubMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING, IDM_REPEAT, "Repeat");
							gs.state.bRepeat = FALSE;
						}
					}
					break;

					case IDM_REPEATTRACK: {
						HMENU hSubMenu;

                        hSubMenu = GetSubMenu(gs.hTrackMenu, 0);
						hSubMenu = GetSubMenu(hSubMenu, gs.nMenuIndexOther);

						if (!gs.state.bRepeatTrack) {
							ModifyMenu(gs.hTrackMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_REPEATTRACK, "Repeat track");
							gs.state.bRepeatTrack = TRUE;

                            UpdateDiscInformation(&gs.di[0], FALSE, TRUE, NULL);

                            gs.nRepeatTrack = gs.di[0].nCurrTrack;
						}
						else {
							ModifyMenu(gs.hTrackMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_STRING, IDM_REPEATTRACK, "Repeat track");
							gs.state.bRepeatTrack = FALSE;
						}

                        CDResume(gs.wDeviceID);
					}
					break;

					case IDM_RANDOMIZE: {
						if (gs.state.bMediaPresent && gs.state.bAudio) {
						    HMENU hSubMenu;

						    hSubMenu = GetSubMenu(gs.hTrackMenu, 0);

						    if (!gs.state.bRandomize) {
							    ModifyMenu(hSubMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_RANDOMIZE, "Random Play");
							    gs.state.bRandomize = TRUE;

                                gs.nLastRandomTrack = 0;

                                PlayRandTrack();
						    }
						    else {
							    ModifyMenu(hSubMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING, IDM_RANDOMIZE, "Random Play");
							    gs.state.bRandomize = FALSE;
						    }
                        }
					}
					break;

                    case IDM_SETABSTRACKPOS:
                        DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_SETABSTRACKPOS), NULL, (DLGPROC)SetAbsTrackPosDlgProc);
                    break;

                    case IDM_SKIP:
                        DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_SKIP), NULL, (DLGPROC)SkipDlgProc);
                    break;

                    case IDM_CDDB:
                        DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_CDDB), NULL, (DLGPROC)DatabaseDlgProc);
                    break;

                    case IDM_OPTIONS:
                        DoOptions();
                    break;

                    case IDM_ABOUT:
                        DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_ABOUT), NULL, (DLGPROC)AboutDlgProc);
                    break;

                    case IDM_INFO:
                        INFODLGPARAM sParam;
                        DISCINFO sEditInfo;

                        InitDiscInfo(&sEditInfo);
                        CopyDiscInfo(&sEditInfo, &gs.di[0]);

                        sParam.psDI = &sEditInfo;
                        sParam.bSave = TRUE;

                        if (DialogBoxParam(gs.hMainInstance, MAKEINTRESOURCE(IDD_INFO), NULL, (DLGPROC)InfoDlgProc, (LPARAM)&sParam) == IDOK) {
                            if (!stricmp(gs.di[0].zCDDBID, sEditInfo.zCDDBID)) {
                                CopyDiscInfo(&gs.di[0], &sEditInfo);

                                CheckProgrammed(gs.wDeviceID, &gs.di[0]);

					            InitMenu(&gs.di[0]);
                            }
						}
						FreeDiscInfo( &sEditInfo ); // Vic: fix memory leak...
                    break;

                    case IDM_INTERNETGET: {
                        if (gs.state.bInit) {
                            DISCINFO sQueryDI;

                            CopyQueryInfo(&gs.di[0], &sQueryDI);

                            if (DBInternetGet(&sQueryDI, hWnd)) {
                                GetDiscInfo(gs.wDeviceID, &sQueryDI);
                                SetDiscInfo(&sQueryDI);

                                EnterCriticalSection(&gs.sDiscInfoLock);

                                FreeDiscInfo(&gs.di[0]);

                                gs.di[0] = sQueryDI;

                                ResetPlaylist(&gs.di[0], TRUE);
  
                                CheckProgrammed(gs.wDeviceID, &gs.di[0]);

                                LeaveCriticalSection(&gs.sDiscInfoLock);
                            }
                            else
                                MessageBox(hWnd, "No disc info found on remote server", APPNAME, MB_OK | MB_ICONINFORMATION);
                        }
                        else
                            MessageBox(hWnd, "There's no disc in the drive!", APPNAME, MB_OK | MB_ICONINFORMATION);
                    }
                    break;

                    case IDM_INTERNETSEND: {
                        DBInternetSend(&gs.di[0], hWnd);
                    }
                    break;

                    case IDM_ADDTOQUEUE: {
                        if (MessageBox(hWnd, "Do you want to add this disc to the queue for later retrieval?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
						    AddToQueue(&gs.di[0]);
                    }
                    break;

                    case IDM_GETQUEUEDITEMS: {
                        if (MessageBox(hWnd, "Do you want to try to retrieve the queued items?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            GetQueuedItems();

							if (gs.nNumberOfItemsInQueue)
		                        MessageBox(hWnd, "One or more queued items still remain due to server error or since they where not found! You might wan't to remove the items or it will be seached for the next time the queue is retrieved.", APPNAME, MB_ICONEXCLAMATION);
                        }
                    }
                    break;

                    case IDM_REMOVEQUEUEDITEMS: {
                        if (MessageBox(hWnd, "Do you want to remove all queued items?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            while (gs.nNumberOfItemsInQueue) 
                                RemoveQueueItem(gs.nNumberOfItemsInQueue);
                        }
                    }
                    break;

                    case IDM_QUIT: {
						if (gs.state.bProgrammed && gs.state.bMediaPresent && gs.state.bPlaying && !(gs.nOptions & OPTIONS_STOPONEXIT)) {
							if (MessageBox(NULL, "The CD is programmed. It will stop after the last sequential track if You continue this action. Are You sure You want to quit?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
								DestroyWindow(hWnd);
						}
						else
							DestroyWindow(hWnd);
					}
                    break;
                }
            }
        break;

		case MYWM_WAKEUP: {
            DebugPrintf("Wakeup call!");

            switch(wParam) {
				case IDM_PLAY: {
                    StatusCheck(&gs.di[0]);

                    if (lParam == -1 && gs.state.bPlaying)
                        DebugPrintf("Ignoring wakeup call to start playing since we are already playing and no track was specified");
                    else {
                        if (gs.state.bRandomize) {
							DebugPrintf("Skipping requested track since we are in repeat mode!");

							gs.di[0].nCurrTrack = GetRandTrack();
						}
						else
							gs.di[0].nCurrTrack = LOWORD(lParam);

                        DebugPrintf("Wakeup call to play track %d", gs.di[0].nCurrTrack);

                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PLAY, 0), 0);

                        SendMessage(hWnd, WM_TIMER, 1, 0);
                    }
				}
				break;

				case IDM_DEVICES: {
                    DebugPrintf("Version is %d", HIWORD(lParam));
                    DebugPrintf("Wakeup call to change device to %d(%c)", LOWORD(lParam), (char)LOWORD(lParam));
                    if ((unsigned)(LOWORD(lParam) - 'A') != gs.nCurrentDevice) {
                        DebugPrintf("Changing...");
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_DEVICES+LOWORD(lParam), 0), 0);
                    }
                    else
                        DebugPrintf("No change!");
				}
				break;

				case IDM_PAUSE:
                    DebugPrintf("Wakeup call to PAUSE");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PAUSE, 0), 0);
				break;

                case IDM_RANDOMIZE:
                    DebugPrintf("Wakeup call to RANDOMIZE");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_RANDOMIZE, 0), 0);
                break;

                case IDM_REPEAT:
                    DebugPrintf("Wakeup call to REPEAT");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_REPEAT, 0), 0);
                break;

				case IDM_STOP:
                    DebugPrintf("Wakeup call to STOP");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_STOP, 0), 0);
				break;

                case IDM_NEXT:
                    DebugPrintf("Wakeup call to NEXT");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_NEXT, 0), 0);
				break;

                case IDM_PREV:
                    DebugPrintf("Wakeup call to PREV");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PREV, 0), 0);
				break;

                case IDM_OPEN:
                    DebugPrintf("Wakeup call to OPEN");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_OPEN, 0), 0);
				break;

                case IDM_CLOSE:
                    DebugPrintf("Wakeup call to CLOSE");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_CLOSE, 0), 0);
				break;
            }
		}
		break;

        case WM_MEASUREITEM: {
            MEASUREITEMSTRUCT* psI = (MEASUREITEMSTRUCT*) lParam;
            if (psI->itemID == 999) {
                psI->itemWidth = 15;

                return TRUE;
            }
        }
        break;

        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* psI = (DRAWITEMSTRUCT*) lParam;
            if (psI->itemID == 999 && psI->itemAction == ODA_DRAWENTIRE) {
                HWND hMenuWnd = WindowFromDC(psI->hDC);
                RECT sRect;

                GetClientRect(hMenuWnd, &sRect);

                // TEMP

                HDC hDC = CreateCompatibleDC(psI->hDC);
                gs.hMenuBitmap = (HBITMAP)SelectObject(hDC, gs.hMenuBitmap);

                psI->rcItem.bottom = (sRect.bottom - sRect.top);
                BitBlt(psI->hDC, psI->rcItem.left, psI->rcItem.bottom - 330, 26, 330, hDC, 0, 0, SRCCOPY);
                RealizePalette(psI->hDC);

                if (psI->rcItem.bottom - 330 > 0) {
                    psI->rcItem.top = 0;
                    psI->rcItem.bottom = 330 - psI->rcItem.bottom;

                    FillRect(psI->hDC, &psI->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
                }

                gs.hMenuBitmap = (HBITMAP)SelectObject(hDC, gs.hMenuBitmap);
                DeleteDC(hDC);

                return TRUE;
            }
        }
        break;

        case WM_HOTKEY: {
            PostMessage(hWnd, WM_COMMAND, gs.anBindings[wParam], 0);
        }
        break;

        case MYWM_NOTIFYICON: {
		    switch (lParam) {
                case WM_RBUTTONUP: {
                    POINT sPoint;
                    HMENU hSubMenu = GetSubMenu(gs.hTrackMenu, 0);

                    GetCursorPos(&sPoint);

                    SetForegroundWindow(hWnd);

                    UpdateMenu(&gs.di[0], hSubMenu);
                    TrackPopupMenu(hSubMenu, TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, sPoint.x, sPoint.y, 0, hWnd, NULL);
                    
                    PostMessage(hWnd, WM_USER, 0, 0);
                }
                break;

                case WM_LBUTTONDOWN: {
                    if (gs.nClicks == 0) {
                        gs.bCaptured = TRUE;
                        gs.bDragged = FALSE;
                        SetCapture(hWnd);
                        POINT p;
                        GetCursorPos(&p);
                        gs.nCaptureX=p.x;
                        gs.nCaptureY=p.y;
                    }
                } break;

                case WM_LBUTTONUP: {
                    if (gs.bCaptured) {
                        ReleaseCapture();
                        gs.bCaptured = FALSE;
                    }

                    if (!gs.bDragged) { // Don't perform click action if user dragged
                        gs.nClicks ++;
                
                        SetTimer(hWnd, 2, GetDoubleClickTime(), NULL);
                    }
                }
                break;

                case WM_CANCELMODE: {
                   if (gs.bCaptured) {
                       ReleaseCapture();
                       gs.bCaptured = FALSE;
                   }
                } break;

                case WM_MOUSEMOVE: {
                    char szStr[512];
                    if (gs.bCaptured) {
                        POINT p;
                        GetCursorPos(&p);
                        int x=p.x;
                        int y=p.y;
                        int dx=x-gs.nCaptureX;
                        int dy=y-gs.nCaptureY;
                        /* Pick an axis */
                        if (abs(dx)<GetSystemMetrics(SM_CXDRAG)) dx=0;
                        if (abs(dy)<GetSystemMetrics(SM_CYDRAG)) dy=0;
                        if (abs(dx)>abs(dy)) dy=0;
                        else dx=0;
                        if (dx || dy) {
                           ReleaseCapture();
                           gs.bCaptured = FALSE;
                           gs.bDragged=TRUE;
                           int binding=0;
                           if (dx>0) {
                               binding=5;
                           } else if (dx<0) {
                               binding=6;
                           } else if (dy<0) {
                               binding=7;
                           } else if (dy>0) {
                               binding=8;
                           }
                           if (gs.anBindings[binding]) {
                              SendMessage(hWnd, WM_COMMAND, MAKELONG(gs.anBindings[binding], 0), 0L);
                           }
                        }
                    }

                    UpdateDiscInformation(&gs.di[0], FALSE, TRUE, szStr);
                }
                break;

                default:
		        break;
            }
        }
        break;

        default: {
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }
    }

    return 0L;
}

char * szBindingNames[NUM_BINDINGS] = {
   "Click 1", "Click 2", "Click 3", "Click 4", "Click 5", "Drag Right", "Drag Left", "Drag Up", "Drag Down"
};


struct CDDB_SERVER asDefaultServers[] = {
	{"freedb.freedb.org", "cddbp", 888, "-", "N000.00", "W000.00", "Random freedb server"},
	{"at.freedb.org", "cddbp", 888, "-", "N048.13", "E016.22", "Vienna, Austria"},
	{"au.freedb.org", "cddbp", 888, "-", "S033.52", "E151.13", "Sydney, Australia"},
	{"ca.freedb.org", "cddbp", 888, "-", "N049.48", "W097.08", "Winnipeg, MB Canada"},
	{"de.freedb.org", "cddbp", 888, "-", "N052.53", "E013.31", "Berlin, Germany"},
	{"es.freedb.org", "cddbp", 888, "-", "N040.30", "W003.48", "Madrid, Spain"},
	{"fi.freedb.org", "cddbp", 888, "-", "N061.30", "E023.42", "Tampere, Finland"},
	{"lu.freedb.org", "cddbp", 888, "-", "N049.47", "E006.13", "Betzdorf, Luxemburg"},
	{"ru.freedb.org", "cddbp", 888, "-", "N059.55", "E030.15", "Saint-Petersburg, Russia"},
	{"uk.freedb.org", "cddbp", 888, "-", "N051.49", "W000.01", "London, UK"},
	{"us.freedb.org", "cddbp", 888, "-", "N037.21", "W121.55", "San Jose, CA USA"},
    {"cddb.ton.tut.fi", "http",  80, "/~cddb/cddb.cgi", "N060.10", "E024.58", "Tampere, Finland"},
};

#define NUM_DEFAULT_SERVERS (sizeof(asDefaultServers) / sizeof(asDefaultServers[0]))

void LoadConfig()
{
    struct {
        char nLen;
        char zPassword[256];
        char xKey;
    } sPassword;
    unsigned int nLoop;
    char zKey[32];
	char zCategory[80];
    int nConfigVersion;
    int bc=0;
    int nLeftButtonNext;
    int nLeftButtonPause;
    int nLeftButtonPrev;
    int nLeftButtonStop;

    gs.bLogfile = ProfileGetInt("NTFY_CD", "Logfile", 0);
    gs.bDebug = ProfileGetInt("NTFY_CD", "Debug", 0);

#ifdef BETA
    gs.bLogfile = TRUE;
#endif

    if (gs.bLogfile) {
        time_t t;

        time(&t);

        DebugPrintf("--------------------------------------------------------------------------------------------------------");
        DebugPrintf("Starting version %s(built %s-%s) at %s", VERSION, __DATE__, __TIME__, ctime(&t));
    }

    gs.nOptions = ProfileGetInt("NTFY_CD", "Settings", -1);
DebugPrintf("gs.nOptions = %d", gs.nOptions);
    gs.nPollTime = ProfileGetInt("NTFY_CD", "PollTime", 1);
DebugPrintf("gs.nPollTime = %d", gs.nPollTime);
    gs.state.bRepeat = ProfileGetInt("NTFY_CD", "Repeat", 0);
    nLeftButtonPause = ProfileGetInt("NTFY_CD", "LeftButtonPause", 1);
    nLeftButtonNext = ProfileGetInt("NTFY_CD", "LeftButtonNext", 2);
    nLeftButtonPrev = ProfileGetInt("NTFY_CD", "LeftButtonPrev", 3);
    nLeftButtonStop = ProfileGetInt("NTFY_CD", "LeftButtonStop", 4);

	// Tooltip/Caption format
    ProfileGetString("NTFY_CD", "TooltipFormat", "%0 (%1) - %4. %2", gs.szTooltipFormat, sizeof(gs.szTooltipFormat));
    ProfileGetString("NTFY_CD", "CaptionFormat", "%0 - %4. %2 (%b)", gs.szCaptionFormat, sizeof(gs.szCaptionFormat));

	// Number of items in queue
	gs.nNumberOfItemsInQueue = ProfileGetInt("NTFY_CD", "QueueItems", 0);

    // Get bindings
    for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++) {
        gs.anBindings[nLoop] = ProfileGetInt("NTFY_CD", szBindingNames[nLoop], 0);
        bc += gs.anBindings[nLoop];
    }
    // Convert the old ones
    if (bc == 0) {
        gs.anBindings[nLeftButtonNext-1]=IDM_NEXT;
        gs.anBindings[nLeftButtonPause-1]=IDM_PAUSE;
        gs.anBindings[nLeftButtonPrev-1]=IDM_PREV;
        gs.anBindings[nLeftButtonStop-1]=IDM_STOP;
        gs.anBindings[4]=0;
        gs.anBindings[5]=0;
        gs.anBindings[6]=0;
        gs.anBindings[7]=0;
        gs.anBindings[8]=0;
    }

    // Get hotkeys
    for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++) {
        sprintf(zKey, "Hotkey%d", nLoop);

        gs.anHotkeys[nLoop] = ProfileGetInt("NTFY_CD", zKey, 0);
    }

    gs.nMenuBreak = ProfileGetInt("NTFY_CD", "MenuBreak", 20);

    gs.nDefaultDevice = ProfileGetInt("NTFY_CD", "DefaultDevice", gs.nCurrentDevice);
    
	ProfileGetString("NTFY_CD", "ExternalCommand", "", gs.zExternalCommand, sizeof(gs.zExternalCommand));

    gs.cddb.nCDDBType = ProfileGetInt("CDDB", "Type", 1);
    gs.cddb.nCDDBOptions = ProfileGetInt("CDDB", "Options_CDDB", 0);

    if (!ProfileGetStruct("NTFY_CD", "CaptionFont", &gs.sCaptionFont, sizeof(gs.sCaptionFont)) || 
        !(gs.nOptions & OPTIONS_USEFONT)) {
        NONCLIENTMETRICS sMetrics;
        sMetrics.cbSize = sizeof(sMetrics);

        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &sMetrics, FALSE);

        sMetrics.lfCaptionFont.lfItalic = !sMetrics.lfCaptionFont.lfItalic;

        CopyMemory( &gs.sCaptionFont, &sMetrics.lfCaptionFont, sizeof(gs.sCaptionFont) );
        gs.nCaptionFontColor = GetSysColor(COLOR_CAPTIONTEXT);
    }
    if (gs.nOptions & OPTIONS_USEFONT) {
        gs.nCaptionFontColor = ProfileGetInt("NTFY_CD", "CaptionFontColor", GetSysColor(COLOR_CAPTIONTEXT));
    }

    nConfigVersion = ProfileGetInt("NTFY_CD", "ConfigVersion", 121);
    if (nConfigVersion != CURR_CONFIG_VERSION && gs.nOptions != -1) {
        int nConvertOptions = gs.nOptions;
        int nConvertCDDBOptions = gs.cddb.nCDDBOptions;
        int nConvertCDDBType = gs.cddb.nCDDBType;

        // Convert from version 1.21
        if (nConfigVersion == 121) {
            gs.nOptions = 0;
            gs.cddb.nCDDBOptions = 0;
            gs.cddb.nCDDBType = 0;

            nConvertCDDBOptions = ProfileGetInt("CDDB", "Options", 0);

            gs.cddb.nCDDBType = nConvertCDDBType;

            if (nConvertOptions & V121_SETTINGS_STOPONEXIT)
                gs.nOptions |= OPTIONS_STOPONEXIT;
            if (nConvertOptions & V121_SETTINGS_STOPONSTART)
                gs.nOptions |= OPTIONS_STOPONSTART;
            if (nConvertOptions & V121_SETTINGS_EXITONCDREMOVE)
                gs.nOptions |= OPTIONS_EXITONCDREMOVE;
            if (nConvertOptions & V121_SETTINGS_PREVALWAYSPREV)
                gs.nOptions |= OPTIONS_PREVALWAYSPREV;
            if (nConvertOptions & V121_SETTINGS_REMEMBERSTATUS)
                gs.nOptions |= OPTIONS_REMEMBERSTATUS;
            if (nConvertOptions & V121_SETTINGS_NOINSERTNOTIFICATION)
                gs.nOptions |= OPTIONS_NOINSERTNOTIFICATION;
            if (nConvertOptions & V121_SETTINGS_TRACKSMENUCOLUMN)
                gs.nOptions |= OPTIONS_TRACKSMENUCOLUMN;
            if (nConvertOptions & V121_SETTINGS_NOMENUBITMAP)
                gs.nOptions |= OPTIONS_NOMENUBITMAP;
            if (nConvertOptions & V121_SETTINGS_NOMENUBREAK)
                gs.nOptions |= OPTIONS_NOMENUBREAK;

            if (nConvertCDDBOptions & V121_OPTIONS_QUERYLOCAL)
                gs.nOptions |= OPTIONS_QUERYLOCAL;
            if (nConvertCDDBOptions & V121_OPTIONS_QUERYREMOTE)
                gs.nOptions |= OPTIONS_QUERYREMOTE;
            if (nConvertCDDBOptions & V121_OPTIONS_STORELOCAL)
                gs.nOptions |= OPTIONS_STORELOCAL;
            if (nConvertCDDBOptions & V121_OPTIONS_STORERESULT)
                gs.nOptions |= OPTIONS_STORERESULT;

            if (nConvertCDDBOptions & V121_OPTIONS_USEHTTP)
                gs.cddb.nCDDBOptions |= OPTIONS_CDDB_USEHTTP;
            if (nConvertCDDBOptions & V121_OPTIONS_USEPROXY)
                gs.cddb.nCDDBOptions |= OPTIONS_CDDB_USEPROXY;

            if (ProfileGetInt("NTFY_CD", "UseDBDLL", 121) && gs.cddb.nCDDBOptions)
                gs.nOptions |= OPTIONS_USECDDB;
        }
        else if (nConfigVersion != 150)
            gs.nOptions = OPTIONS_STORELOCAL | OPTIONS_QUERYLOCAL;
    }
	// Convert the old time/info options
	if (nConfigVersion < CURR_CONFIG_VERSION) {
		char* pzStr;
		unsigned int nLoop;
		int nTimeOptions;
		int nCDInfoOptions;

		for (nLoop = 0 ; nLoop < 2 ; nLoop ++) {
			if (nLoop == 0) {
				nTimeOptions = ProfileGetInt("NTFY_CD", "TimeSettings", 0);
				nCDInfoOptions = ProfileGetInt("NTFY_CD", "CDInfoSettings", 0);

				pzStr = gs.szTooltipFormat;
			}
			else {
				nTimeOptions = ProfileGetInt("NTFY_CD", "CaptionTimeSettings", 0);
				nCDInfoOptions = ProfileGetInt("NTFY_CD", "CaptionCDInfoSettings", 0);

				pzStr = gs.szCaptionFormat;
			}

			if (nTimeOptions || nCDInfoOptions) {
				pzStr[0] = 0;

				if (nCDInfoOptions & V150_OPTIONS_CDINFO_ARTIST)
					strcat(pzStr, "%0");
				if (nCDInfoOptions & V150_OPTIONS_CDINFO_TITLE) {
					if (pzStr[0])
						strcat(pzStr, " ");
					strcat(pzStr, "(%1) ");
				}
				if (nCDInfoOptions & V150_OPTIONS_CDINFO_TRACKNO) {
					if (pzStr[0])
						strcat(pzStr, " - ");

					strcat(pzStr, "%4");
					if (nCDInfoOptions & V150_OPTIONS_CDINFO_TRACKTITLE) 
						strcat(pzStr, ". ");
				}
				if (nCDInfoOptions & V150_OPTIONS_CDINFO_TRACKTITLE) 
					strcat(pzStr, "%2");
				if (nCDInfoOptions & V150_OPTIONS_CDINFO_TRACKLENGTH)
					strcat(pzStr, " (%5) ");

				if (nTimeOptions & V150_OPTIONS_TIME_TRACK)
					strcat(pzStr, " [%a]");
				else if (nTimeOptions & V150_OPTIONS_TIME_TRACKREM)
					strcat(pzStr, " [%b]");
				else if (nTimeOptions & V150_OPTIONS_TIME_CD)
					strcat(pzStr, " [%c]");
				else if (nTimeOptions & V150_OPTIONS_TIME_CDREM)
					strcat(pzStr, " [%d]");
			}
		}
	}

    // Default options if not used before
    if (gs.nOptions == -1)
		gs.nOptions = OPTIONS_DEFAULT;

    gs.bHasNotification = ProfileGetInt("NTFY_CD", "HasNotification", 0);

    if (gs.nOptions & OPTIONS_REMEMBERSTATUS) {
        int nStatus = ProfileGetInt("NTFY_CD", "Status", 0);
        if (nStatus & STATUS_RANDOM)
            gs.state.bRandomize = TRUE;
        if (nStatus & STATUS_REPEAT)
            gs.state.bRepeat = TRUE;
    }

    // Load categories

    gs.nNumCategories = ProfileGetInt("NTFY_CD", "NumCategories", 0);
    // If no categories, add them!
    if (!gs.nNumCategories) {
        gs.nNumCategories = 10;

        ProfileWriteString("NTFY_CD", "NumCategories", "10");

        ProfileWriteString("NTFY_CD", "Category0", "Blues");
        ProfileWriteString("NTFY_CD", "Category1", "Classical");
        ProfileWriteString("NTFY_CD", "Category2", "Country");
        ProfileWriteString("NTFY_CD", "Category3", "Folk");
        ProfileWriteString("NTFY_CD", "Category4", "Jazz");
        ProfileWriteString("NTFY_CD", "Category5", "Misc");
        ProfileWriteString("NTFY_CD", "Category6", "Newage");
        ProfileWriteString("NTFY_CD", "Category7", "Reggae");
        ProfileWriteString("NTFY_CD", "Category8", "Rock");
        ProfileWriteString("NTFY_CD", "Category9", "Soundtrack");
    }

    gs.ppzCategories = new char *[gs.nNumCategories];
    for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++) {
        sprintf( zKey, "Category%d", nLoop );
		ProfileGetString( "NTFY_CD", zKey, "Unknown", zCategory, sizeof(zCategory) );
        gs.ppzCategories[nLoop] = StringCopy( zCategory );
    }

    ProfileGetString("CDDB", "Path", "C:\\CDDB\\", gs.cddb.zCDDBPath, sizeof(gs.cddb.zCDDBPath));
    ProfileGetString("CDDB", "RemoteServer", "freedb.freedb.org", gs.cddb.zRemoteServer, sizeof(gs.cddb.zRemoteServer));
    ProfileGetString("CDDB", "RemoteHTTPPath", "/~cddb/cddb.cgi", gs.cddb.zRemoteHTTPPath, sizeof(gs.cddb.zRemoteHTTPPath));
    ProfileGetString("CDDB", "RemoteProxyServer", "", gs.cddb.zRemoteProxyServer, sizeof(gs.cddb.zRemoteProxyServer));
    ProfileGetString("CDDB", "EmailServer", "", gs.cddb.zRemoteEmailServer, sizeof(gs.cddb.zRemoteEmailServer));
    ProfileGetString("CDDB", "EmailAddress", "", gs.cddb.zEmailAddress, sizeof(gs.cddb.zEmailAddress));
    ProfileGetString("CDDB", "Domain", "", gs.cddb.zDomain, sizeof(gs.cddb.zDomain));
    ProfileGetString("CDDB", "ProxyUser", "", gs.cddb.zProxyUser, sizeof(gs.cddb.zProxyUser));
    ProfileGetString("CDDB", "UserAgent", APPNAME "/1.00", gs.cddb.zUserAgent, sizeof(gs.cddb.zUserAgent));

    gs.cddb.nRemotePort = ProfileGetInt("CDDB", "RemotePort", 888);
    gs.cddb.nRemoteProxyPort = ProfileGetInt("CDDB", "RemoteProxyPort", 0);
    gs.cddb.nRemoteTimeout = ProfileGetInt("CDDB", "RemoteTimeout", 30);	

    // Password
    if (!(gs.cddb.nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)) {
        ZeroMemory(gs.cddb.zProxyPassword, sizeof(gs.cddb.zProxyPassword));

        if (ProfileGetStruct("CDDB", "ProxyPassword", &sPassword, sizeof(sPassword))) {
            for (nLoop = 0 ; nLoop < (unsigned)sPassword.nLen ; nLoop ++)
                gs.cddb.zProxyPassword[nLoop] = (char)(sPassword.zPassword[nLoop] + sPassword.xKey);
        }
    }
    else
        gs.cddb.zProxyPassword[0] = 0;

    if (gs.cddb.zRemoteHTTPPath[0] != 0 && gs.cddb.zRemoteHTTPPath[0] != '/') {
        char zTmp[256];

        StringCpyZ(zTmp, gs.cddb.zRemoteHTTPPath, sizeof(zTmp));
		StringPrintf(gs.cddb.zRemoteHTTPPath, sizeof(gs.cddb.zRemoteHTTPPath), "/%s", zTmp);
    }

    if (gs.cddb.zRemoteHTTPPath[0] != 0 && gs.cddb.zRemoteHTTPPath[strlen(gs.cddb.zRemoteHTTPPath)-1] == '/')
        gs.cddb.zRemoteHTTPPath[strlen(gs.cddb.zRemoteHTTPPath)-1] = 0;

    // Fix path
    if (gs.cddb.zCDDBPath[0] && gs.cddb.zCDDBPath[strlen(gs.cddb.zCDDBPath) - 1] != '\\')
        StringCatZ(gs.cddb.zCDDBPath, "\\", sizeof(gs.cddb.zCDDBPath));
    
    // Load servers
    gs.cddb.nNumCDDBServers = ProfileGetInt("CDDB", "NumServers", 0);
    if (gs.cddb.nNumCDDBServers) {
        char zServerInfo[1024];

        gs.cddb.psCDDBServers = new CDDB_SERVER[gs.cddb.nNumCDDBServers];

        for (nLoop = 0 ; nLoop < gs.cddb.nNumCDDBServers ; nLoop ++) {
            sprintf(zKey, "Server%d", nLoop);

            ProfileGetString("CDDB", zKey, "", zServerInfo, sizeof(zServerInfo));
            if (zServerInfo[0])
                ParseServerInfo(&gs.cddb.psCDDBServers[nLoop], zServerInfo);
        }
    }
    else {
        gs.cddb.nNumCDDBServers = NUM_DEFAULT_SERVERS;
        gs.cddb.psCDDBServers = new CDDB_SERVER[gs.cddb.nNumCDDBServers];
        CopyMemory(gs.cddb.psCDDBServers, asDefaultServers, sizeof(asDefaultServers));
    }
}

void SaveConfig()
{
    struct {
        char nLen;
        char zPassword[256];
        char xKey;
    } sPassword;
    char zKey[32];
    char zServerInfo[1024];
    unsigned int nLoop;

    if( !ProfileWriteInt( "NTFY_CD", "ConfigVersion", CURR_CONFIG_VERSION ) )
		return;
    if( !ProfileWriteInt( "NTFY_CD", "Settings", gs.nOptions ) )
		return;
    if( !ProfileWriteInt( "NTFY_CD", "PollTime", gs.nPollTime ) )
		return;
	if( !ProfileWriteInt( "NTFY_CD", "Repeat", gs.state.bRepeat ) )
		return;

	// Tooltip/Caption format
    if( !ProfileWriteString( "NTFY_CD", "TooltipFormat", gs.szTooltipFormat ) )
		return;
    if( !ProfileWriteString( "NTFY_CD", "CaptionFormat", gs.szCaptionFormat ) )
		return;

    // Save bindings
    for (nLoop = 0; nLoop < NUM_BINDINGS ; nLoop ++)
	    if( !ProfileWriteInt( "NTFY_CD", szBindingNames[nLoop], gs.anBindings[nLoop] ) )
			return;

    // Save hotkeys
    for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++) {
        sprintf(zKey, "Hotkey%d", nLoop);
	    if( !ProfileWriteInt( "NTFY_CD", zKey, gs.anHotkeys[nLoop] ) )
			return;
    }

    if( !ProfileWriteInt( "NTFY_CD", "DefaultDevice", gs.nDefaultDevice ) )
		return;
    if( !ProfileWriteInt( "NTFY_CD", "HasNotification", gs.bHasNotification ) )
		return;
    if( !ProfileWriteString( "NTFY_CD", "ExternalCommand", gs.zExternalCommand ) )
		return;
    if( !ProfileWriteString( "CDDB", "PATH", gs.cddb.zCDDBPath ) )
		return;
    if( !ProfileWriteInt( "CDDB", "TYPE", gs.cddb.nCDDBType ) )
		return;
    if( !ProfileWriteInt( "CDDB", "OPTIONS_CDDB", gs.cddb.nCDDBOptions ) )
		return;
    if( !ProfileWriteString( "CDDB", "RemoteServer", gs.cddb.zRemoteServer ) )
		return;
    if( !ProfileWriteString( "CDDB", "RemoteHTTPPath", gs.cddb.zRemoteHTTPPath ) )
		return;
    if( !ProfileWriteString( "CDDB", "RemoteProxyServer", gs.cddb.zRemoteProxyServer ) )
		return;
    if( !ProfileWriteString( "CDDB", "EmailServer", gs.cddb.zRemoteEmailServer ) )
		return;
    if( !ProfileWriteString( "CDDB", "EmailAddress", gs.cddb.zEmailAddress ) )
		return;
    if( !ProfileWriteString( "CDDB", "ProxyUser", gs.cddb.zProxyUser ) )
		return;

    // Password
    if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)
        gs.cddb.zProxyPassword[0] = 0;

    ZeroMemory(&sPassword, sizeof(sPassword));
    sPassword.xKey = (char)(rand() % 250);
    sPassword.nLen = (char)strlen(gs.cddb.zProxyPassword);
    for (nLoop = 0 ; gs.cddb.zProxyPassword[nLoop] ; nLoop ++)
        sPassword.zPassword[nLoop] = (char)(gs.cddb.zProxyPassword[nLoop] - sPassword.xKey);
    for ( ; nLoop < 256 ; nLoop ++)
        sPassword.zPassword[nLoop] = (char)(rand() % 250);

    if( !ProfileWriteStruct( "CDDB", "ProxyPassword", &sPassword, sizeof(sPassword) ) )
		return;

    if( !ProfileWriteInt( "CDDB", "RemotePort", gs.cddb.nRemotePort ) )
		return;
    if( !ProfileWriteInt( "CDDB", "RemoteProxyPort", gs.cddb.nRemoteProxyPort ) )
		return;
    if( !ProfileWriteInt( "CDDB", "RemoteTimeout", gs.cddb.nRemoteTimeout ) )
		return;

    if (gs.nOptions & OPTIONS_USEFONT) {
	    if( !ProfileWriteStruct( "NTFY_CD", "CaptionFont", &gs.sCaptionFont, sizeof(gs.sCaptionFont) ) )
			return;
        if( !ProfileWriteInt( "NTFY_CD", "CaptionFontColor", gs.nCaptionFontColor ) )
			return;
    }

    if (gs.nOptions & OPTIONS_REMEMBERSTATUS) {
        int nStatus = 0;
        if (gs.state.bRandomize)
            nStatus += STATUS_RANDOM;
        if (gs.state.bRepeat)
            nStatus += STATUS_REPEAT;

        if( !ProfileWriteInt( "NTFY_CD", "Status", nStatus ) )
			return;
    }

    // Save servers
    if( !ProfileWriteInt( "CDDB", "NumServers", gs.cddb.nNumCDDBServers ) )
		return;

    for (nLoop = 0 ; nLoop < gs.cddb.nNumCDDBServers ; nLoop ++) {
        sprintf(zKey, "Server%d", nLoop);

        sprintf(zServerInfo, "%s %s %d %s %s %s %s", 
            gs.cddb.psCDDBServers[nLoop].zSite,
            gs.cddb.psCDDBServers[nLoop].zProtocol,
            gs.cddb.psCDDBServers[nLoop].nPort, 
            gs.cddb.psCDDBServers[nLoop].zAddress,
            gs.cddb.psCDDBServers[nLoop].zLatitude,
            gs.cddb.psCDDBServers[nLoop].zLongitude,
            gs.cddb.psCDDBServers[nLoop].zDescription);
        if( !ProfileWriteString( "CDDB", zKey, zServerInfo ) )
			return;
    }
}                                          


void DoSetup()
{
    if (ProfileGetInt("NTFY_CD", "ConfigVersion", 121) != CURR_CONFIG_VERSION) {
        MessageBox(NULL, "Now Notify will ask you some questions about the initial configuration. Yes is normally the best answer to the questions.", APPNAME, MB_OK | MB_ICONINFORMATION);

	    if (MessageBox(NULL, "Do you want Notify to automatically query the Internet, if you're connected, for CD information when you insert a new CD that isn't found in the local database?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
		    gs.nOptions |= OPTIONS_QUERYREMOTE;
		    gs.nOptions |= OPTIONS_STORERESULT;
			    
//		    gs.cddb.nRemotePort = 80;	
//	        gs.cddb.nCDDBOptions |= OPTIONS_CDDB_USEHTTP;

		    MessageBox(NULL, "A default remote server has been configured, please go into the CDDB Remote tab of the Options dialog later and configure the server that is best suitable for your geographical locations as well as proxy information if needed!", APPNAME, MB_OK | MB_ICONINFORMATION);
	    }

	    if (gs.nOptions & OPTIONS_QUERYREMOTE) {
		    if (MessageBox(NULL, "Do you want Notify to queue the remote queries for later retrieval if the connection to the server fails?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
			    gs.nOptions |= OPTIONS_AUTOADDQUEUE;
			    gs.nOptions |= OPTIONS_AUTORETRIEVEQUEUE;
		    }
	    }

	    if (MessageBox(NULL, "Do you want Notify to automatically look for upgrades if you are connected to the Internet?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
		    gs.nOptions |= OPTIONS_AUTOCHECKFORNEWVERSION;

        MessageBox(NULL, "Notify will now show the Options dialog, it is recommended that you go through each tab and do additional configuration.", APPNAME, MB_OK | MB_ICONINFORMATION);

        DoOptions();
    }

    MessageBox(NULL, "Notify installation is done!", APPNAME, MB_OK);
}


void HandleCommandLine(LPSTR lpCmdLine,
                       HWND hWnd, 
                       unsigned int nFoundDevice, 
                       unsigned int nFoundTrack)
{
    if (strstr(lpCmdLine, "-OPEN"))
		PostMessage(hWnd, MYWM_WAKEUP, IDM_OPEN, 0);

    if (strstr(lpCmdLine, "-CLOSE"))
		PostMessage(hWnd, MYWM_WAKEUP, IDM_CLOSE, 0);

    if (strstr(lpCmdLine, "-PLAY") || strstr(lpCmdLine, "/PLAY")) {
        if (nFoundDevice != 0xFFFFFFFF) {
            DebugPrintf("...Device = %c", nFoundDevice);
            PostMessage(hWnd, MYWM_WAKEUP, IDM_DEVICES, MAKELONG(nFoundDevice, 1));
        }
		PostMessage(hWnd, MYWM_WAKEUP, IDM_PLAY, nFoundTrack - 1);
    }

	if (strstr(lpCmdLine, "-RANDOM"))
		PostMessage(hWnd, MYWM_WAKEUP, IDM_RANDOMIZE, 0);

	if (strstr(lpCmdLine, "-REPEAT"))
		PostMessage(hWnd, MYWM_WAKEUP, IDM_REPEAT, 0);

    // Stuff below only valid if second instance. 
    if (hWnd != gs.hMainWnd) {
        if (strstr(lpCmdLine, "-PREV"))
		    PostMessage(hWnd, MYWM_WAKEUP, IDM_PREV, 0);

        if (strstr(lpCmdLine, "-NEXT"))
		    PostMessage(hWnd, MYWM_WAKEUP, IDM_NEXT, 0);

        if (strstr(lpCmdLine, "-STOP"))
		    PostMessage(hWnd, MYWM_WAKEUP, IDM_STOP, 0);

		if (strstr(lpCmdLine, "-PAUSE"))
			PostMessage(hWnd, MYWM_WAKEUP, IDM_PAUSE, nFoundTrack - 1);
    }
    // Stuff below only valid on first instance
    else {
        if (strstr(lpCmdLine, "-PAUSE")) {
		    gs.di[0].nCurrTrack = nFoundTrack - 1;

		    PostMessage(gs.hMainWnd, MYWM_WAKEUP, IDM_PLAY, nFoundTrack - 1);
		    PostMessage(gs.hMainWnd, MYWM_WAKEUP, IDM_PAUSE, 0);
	    }

        if (strstr(lpCmdLine, "-EXITIFNODISC") && !gs.state.bMediaPresent) {
            NotifyDelete(gs.hMainWnd, 100); // Vic: commented out
            PostQuitMessage(0);
        }
    }
}


LONG WINAPI TopLevelExHandler(LPEXCEPTION_POINTERS lpex)
{
    TCHAR szErrorMsg[256];
    
	__try {
        DebugPrintf("**** EXCEPTION CAUGHT ***");
        DebugPrintf("ExceptionCode: %08X", lpex->ExceptionRecord->ExceptionCode);
        DebugPrintf("ExceptionAddress: %08X", lpex->ExceptionRecord->ExceptionAddress);

        sprintf(szErrorMsg, APPNAME " caused an exception 0x%08X at 0x%08X. " APPNAME " will terminate!", 
            lpex->ExceptionRecord->ExceptionCode, lpex->ExceptionRecord->ExceptionAddress);
    } 
    __except (TRUE) {
	} // __try

    MessageBox(NULL, szErrorMsg, APPNAME, MB_OK | MB_ICONSTOP);

    DebugPrintf("Ending");

    ExitProcess(1);

    return NO_ERROR;
}


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR lpCmdLine,
    int /*nShowCmd*/)
{
    WNDCLASS sWndClass;
    MSG sMsg;
    HWND hPrevWindow;
	char* pzTrackPtr;
	unsigned int nFoundTrack = 1;
    unsigned int nFoundDevice = 0xFFFFFFFF;
    unsigned int nLoop;   

//    SetUnhandledExceptionFilter(TopLevelExHandler);

    gs.sVersionInfo.dwOSVersionInfoSize = sizeof(gs.sVersionInfo);
    GetVersionEx(&gs.sVersionInfo);

	GetWindowsDirectory( gs.szProfilePath, MAX_PATH );

	if( gs.szProfilePath[strlen(gs.szProfilePath) - 1] == '\\' )
        StringCatZ( gs.szProfilePath, PROFILENAME, sizeof(gs.szProfilePath) );
	else
        StringCatZ( gs.szProfilePath, "\\" PROFILENAME, sizeof(gs.szProfilePath) );

	srand((unsigned)time(NULL));

    //
	// Variable Init
	// 

    InitializeCriticalSection(&gs.sDiscInfoLock);

	gs.hTrackMenu = NULL;
	gs.hMainWnd = NULL;

	gs.nClicks = 0;
	gs.nPollTime = 1;
	gs.nStatus = 0;

	gs.nMenuIndexTracks = 7;
	gs.nMenuIndexOther = 14;
	gs.nMenuIndexDevices = 15;

	gs.bMultiselectCategories = FALSE;
	gs.bBrokenTracksMenu = FALSE;
	gs.bCreateWindowFinished = FALSE;
	gs.bFirstStatusCheckDone = FALSE;

	gs.bInInfoDlg = FALSE;
	gs.bInDBDlg = FALSE;
	gs.bInSkipDlg = FALSE;
	gs.bInSetAbsTrackPosDlg = FALSE;
	gs.bInOptionsDlg = FALSE;
	gs.bHasNotification = FALSE;

    gs.bCaptured = FALSE;
    gs.bDragged = FALSE;

	gs.bDebug = FALSE;

	ZeroMemory(&gs.di[0], sizeof(gs.di[0]));
	ZeroMemory(&gs.state, sizeof(gs.state));

	gs.szToolTip[0] = 0;
	gs.ppzCategories = NULL;
	gs.nNumCategories = 0;
	ZeroMemory(&gs.abDevices, sizeof(gs.abDevices));
	gs.nCurrentDevice = 0xFFFFFFFF;
	gs.nDefaultDevice = 0xFFFFFFFF;
	gs.nNumberOfDevices = 0;
	gs.nMenuBreak = 20;
	gs.nRepeatTrack = 0;
	gs.nNextProgrammedTrack = 0;
	gs.hMenuFont = NULL;
	gs.hMenuBitmap = NULL;
	gs.pnLastRandomTracks = NULL;
	gs.nLastRandomTrack = 0;
    gs.zExternalCommand[0] = 0;

    InitDiscInfo(&gs.di[0]);

    // Check drives for CD-ROM devices

    for (nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
        char zRoot[4];

        sprintf(zRoot, "%c:\\", nLoop);

        if (GetDriveType(zRoot) == DRIVE_CDROM) {
            gs.abDevices[nLoop - 'A'] = TRUE;

            if (gs.nCurrentDevice == -1)
                gs.nCurrentDevice = nLoop - 'A';

            gs.nNumberOfDevices ++;
        }
        else
            gs.abDevices[nLoop - 'A'] = FALSE;
    }

    if (gs.nCurrentDevice == -1) {
        MessageBox(NULL, "No CD audio device found!", APPNAME, MB_OK | MB_ICONERROR);

        return 1;
    }

	// NOTE: No logging appear until after this line
    LoadConfig();

    // Command line stuff
    
    strupr(lpCmdLine);

    DebugPrintf("Command Line: %s", lpCmdLine);

    pzTrackPtr = strstr(lpCmdLine, "-PLAY");
    if (!pzTrackPtr)
        pzTrackPtr = strstr(lpCmdLine, "/PLAY");
	if (pzTrackPtr) {
		if (strchr(lpCmdLine, '\\')) {
            char szStr[3];

		    pzTrackPtr += 6;

		    nFoundDevice = (int)*pzTrackPtr;
            DebugPrintf("Found device %c on command line", nFoundDevice);

            pzTrackPtr = strstr(pzTrackPtr, "TRACK");
		    if (pzTrackPtr) {
			    StringCpyZ(szStr, pzTrackPtr+5, sizeof(szStr));
			    nFoundTrack = atoi(szStr);

				DebugPrintf("Found track %d on command-line", nFoundTrack);
		    }
        }
	}

	hPrevWindow = FindWindow("NOTIFY_CD_CLASS", APPNAME);
    if (hPrevWindow) {
        DebugPrintf("Sending to other instance...");

        HandleCommandLine(lpCmdLine, hPrevWindow, nFoundDevice, nFoundTrack);

        DebugPrintf("Instance ends....");
        DebugPrintf("--------------------------------------------------------------------------------------------------------");
    
        return 0;
    }

/*   
    pzTrackPtr = NULL;
    *pzTrackPtr = 0;
*/
	
#ifdef _DEBUG
    if (gs.bDebug) {
        // Fake more devices
        gs.nNumberOfDevices ++;
        gs.abDevices['F' - 'A'] = TRUE;
    }
#endif

    DebugPrintf("Current device = %c", gs.nCurrentDevice + 'A');

    DebugPrintf("Number of devices = %d", gs.nNumberOfDevices);

    for (nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
        if (gs.abDevices[nLoop - 'A'])
            DebugPrintf("%c is a CD-ROM device", nLoop);
    }

    // Set current to default. Default is current if default is missing from the INI file!
    // current might change below if a command line with a device in it is used
    gs.nCurrentDevice = gs.nDefaultDevice;

    DebugPrintf("Default device = %c", gs.nDefaultDevice + 'A');

    if (gs.sVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        DebugPrintf("Running Windows %d.%02d", gs.sVersionInfo.dwMajorVersion, gs.sVersionInfo.dwMinorVersion);
    else
        DebugPrintf("Running Windows NT %d.%02d.%d (%s)", gs.sVersionInfo.dwMajorVersion, gs.sVersionInfo.dwMinorVersion, 
                    gs.sVersionInfo.dwBuildNumber, gs.sVersionInfo.szCSDVersion);

    InitCommonControls();

    gs.hMainInstance = hInstance;

    DBInit();

	if (strstr(lpCmdLine, "-SETUP")) {
        DoSetup ();
		return 0;
	}

    ZeroMemory(&sWndClass, sizeof(sWndClass));
    sWndClass.lpfnWndProc = MainWindowProc;
    sWndClass.hInstance = hInstance;
    sWndClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
    sWndClass.lpszClassName = "NOTIFY_CD_CLASS";

    if (!RegisterClass(&sWndClass)) {
        MessageBox(NULL, "Error when registering class", APPNAME, MB_OK | MB_ICONERROR);

        return 0;
    }
    
    gs.hTrayIcon = NULL;

    gs.hIconNoAudio = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOAUDIO));
    gs.hIconPause = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAUSE));
    gs.hIconStop = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STOP));
    gs.hIconPlay = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLAY));

    strcpy(gs.szToolTip, APPNAME);

    gs.hMenuFont = CreateFont(20, 0, 900, 0, FW_EXTRABOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_DONTCARE, "Arial");
//    gs.hMenuBitmap = LoadBitmap(gs.hMainInstance, MAKEINTRESOURCE(IDB_NOTIFY));

    gs.hMainWnd = CreateWindow("NOTIFY_CD_CLASS", 
                            APPNAME, 
                            WS_POPUP/* | WS_VISIBLE | WS_CAPTION*/,
                            , 
                            0, 0,
                            200, 200,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);
    
    gs.bCreateWindowFinished = TRUE;

    SetHotkeys();

    InitMenu(&gs.di[0]);

    SendMessage(gs.hMainWnd, WM_TIMER, 1, 0);

    // Command line stuff
    HandleCommandLine(lpCmdLine, gs.hMainWnd, nFoundDevice, nFoundTrack);

    // Run the app
    while(GetMessage(&sMsg, NULL, 0, 0)) {
        TranslateMessage(&sMsg);
        DispatchMessage(&sMsg);
    }

    // Saving the config might be usefull

    SaveConfig();

    // Free stuff
	if( gs.ppzCategories ) {
		for( nLoop = 0; nLoop < gs.nNumCategories; nLoop ++ )
			delete[] gs.ppzCategories[nLoop];
		delete[] gs.ppzCategories;
	}

    if( gs.pnLastRandomTracks )
        delete[] gs.pnLastRandomTracks;

    FreeDiscInfo(&gs.di[0]);

    DBFree();

    CDClose(&gs.wDeviceID);

    if (gs.cddb.psCDDBServers)
        delete[] gs.cddb.psCDDBServers;

    DeleteObject(gs.hMenuFont);

	if (gs.hMenuBitmap)
		DeleteObject(gs.hMenuBitmap);

    DestroyMenu(gs.hTrackMenu);

    if (gs.nOptions & OPTIONS_SHOWONCAPTION) {
        HWND hForegroundWnd = GetForegroundWindow();

        if (hForegroundWnd) {
            char* pzText;
            int nLen;

            nLen = GetWindowTextLength(hForegroundWnd);
            pzText = new char[nLen + 1];

            GetWindowText(hForegroundWnd, pzText, nLen + 1);
            SetWindowText(hForegroundWnd, pzText);

            delete[] pzText;
        }
    }

    DeleteCriticalSection(&gs.sDiscInfoLock);

    DebugPrintf("Program ends....");
    DebugPrintf("--------------------------------------------------------------------------------------------------------");
    
    // Whoha!

    return 0;
}
