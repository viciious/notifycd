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

#include "common.h"

#include <commctrl.h>
#include <shellapi.h>
#include <process.h>

#include "res/resource.h"

#include "ntfy_cd.h"
#include "mci.h"
#include "db.h"
#include "cddb.h"
#define SOCK
#include "misc.h"

extern GLOBALSTRUCT gs;

/////////////////////////////////////////////////////////////////////
//
// Tray Notification stuff!
//
/////////////////////////////////////////////////////////////////////

BOOL TrayMessage(HWND hWnd, DWORD dwMessage, UINT uID, HICON hIcon, PSTR pszTip)
{
    NOTIFYICONDATA tnd;

	tnd.cbSize		        = sizeof(NOTIFYICONDATA);
	tnd.hWnd		        = hWnd;
	tnd.uID			        = uID;
	tnd.uFlags		        = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage    = MYWM_NOTIFYICON;
	tnd.hIcon		        = hIcon;

    if (pszTip)
		StringCpyZ(tnd.szTip, pszTip, sizeof(tnd.szTip));
	else
		tnd.szTip[0] = '\0';

	return Shell_NotifyIcon(dwMessage, &tnd);
}


void NotifyAdd(HWND hWnd, UINT nID, HICON hIcon, char* pzStr)
{
	TrayMessage(hWnd, NIM_ADD, nID, hIcon, pzStr);
}


void NotifyModify(HWND hWnd, UINT nID, HICON hIcon, char* pzStr)
{
	TrayMessage(hWnd, NIM_MODIFY, nID, hIcon, pzStr);
}


void NotifyDelete(HWND hWnd, UINT nID)
{
	TrayMessage(hWnd, NIM_DELETE, nID, NULL, NULL);
}


/////////////////////////////////////////////////////////////////////
//
// MISC STUFF!
//
/////////////////////////////////////////////////////////////////////


DWORD ProfileGetInt( const char *pzAppName, const char *pzKeyName, int nDefault )
{
	return GetPrivateProfileInt( pzAppName, pzKeyName, nDefault, gs.szProfilePath );
}

BOOL ProfileWriteInt( const char *pzAppName, const char *pzKeyName, int nNum )
{
	char str[32];

	sprintf( str, "%d", nNum );

	if( WritePrivateProfileString( pzAppName, pzKeyName, str, gs.szProfilePath ) == FALSE ) {
		MessageBox( NULL, "Write to INI file failed, some of the configuration might not be saved.", APPNAME, MB_OK | MB_ICONERROR );
		return FALSE;
	}
	return TRUE;
}

DWORD ProfileGetString( const char *pzAppName, const char *pzKeyName, const char *pzDefault, char *pzReturnedString, unsigned int nSize )
{
	DWORD res = GetPrivateProfileString( pzAppName, pzKeyName, pzDefault, pzReturnedString, nSize, gs.szProfilePath );
	pzReturnedString[nSize-1] = 0;
	return res;
}

BOOL ProfileWriteString( const char *pzAppName, const char *pzKeyName, char *pzString )
{
	if( WritePrivateProfileString( pzAppName, pzKeyName, pzString, gs.szProfilePath ) == FALSE ) {
		MessageBox( NULL, "Write to INI file failed, some of the configuration might not be saved.", APPNAME, MB_OK | MB_ICONERROR );
		return FALSE;
	}
	return TRUE;
}

DWORD ProfileGetStruct( const char *pzAppName, const char *pzKeyName, void *pzStruct, unsigned int nSize )
{
	return GetPrivateProfileStruct( pzAppName, pzKeyName, pzStruct, nSize, gs.szProfilePath );
}

BOOL ProfileWriteStruct( const char *pzAppName, const char *pzKeyName, void *pzStruct, unsigned int nSize )
{
	if( WritePrivateProfileStruct( pzAppName, pzKeyName, pzStruct, nSize, gs.szProfilePath ) == FALSE ) {
		MessageBox( NULL, "Write to INI file failed, some of the configuration might not be saved.", APPNAME, MB_OK | MB_ICONERROR );
		return FALSE;
	}
	return TRUE;
}

DWORD ProfileGetSection( const char *pzAppName, char *pzString, unsigned int nSize )
{
	return GetPrivateProfileSection( pzAppName, pzString, nSize, gs.szProfilePath );
}

BOOL ProfileWriteSection( const char *pzAppName, char *pzString )
{
	if( WritePrivateProfileSection( pzAppName, pzString, gs.szProfilePath ) == FALSE ) {
		MessageBox( NULL, "Write to INI file failed, some of the configuration might not be saved.", APPNAME, MB_OK | MB_ICONERROR );
		return FALSE;
	}
	return TRUE;
}

void GetTmpFile(char* pzFile)
{
#if 0
	FILE* fp;

	tmpnam(pzFile);

	fp = fopen(pzFile, "w");
	if (!fp)
		strcpy(pzFile, "C:\\" TEMPNAME);
    else {
		fclose(fp);
        DeleteFile(pzFile);
    }
#else
	char *zTempNname;

	if (!(zTempNname = _tempnam (NULL, "ncd")))
		strcpy(pzFile, TEMPNAME);
	else
		strcpy(pzFile, zTempNname);
#endif

	DebugPrintf("Using Temp file: %s", pzFile);
}

void DebugPrintf(const char* pzFormat, ...)
{
    char szBuffer[2048];
    va_list ap;

    // This will make the vsprint code only to run uf gs.bLogfile is set if we are a release build
#if !defined _DEBUG && !defined SPECIALDEBUG
    if (gs.bLogfile) {
#endif
        va_start(ap, pzFormat);
        _vsnprintf(szBuffer, sizeof(szBuffer), pzFormat, ap);
		szBuffer[sizeof(szBuffer) - 1] = 0;
#if !defined _DEBUG && !defined SPECIALDEBUG
    }
#endif

	if (gs.bLogfile) {
        FILE* fp;
        SYSTEMTIME sTime;

        GetLocalTime(&sTime);
        
        fp = fopen(LOGNAME, "a");
        if (fp) {
            fprintf(fp, "%04X.%04X:%02d%02d%02d.%03d: %s\n", GetCurrentProcessId(), GetCurrentThreadId(), 
                sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds, szBuffer);

            fclose(fp);
        }
    }

#if defined _DEBUG || defined SPECIALDEBUG
    strcat(szBuffer, "\n\r");
    OutputDebugString("NTFY_CD: ");
    OutputDebugString(szBuffer);
#endif
    // This will make the va_end code only to run uf gs.bLogfile is set if we are a release build
#if !defined _DEBUG && !defined SPECIALDEBUG
    if (gs.bLogfile)
#endif
        va_end(ap);
}


void FixCDDBString(BOOL bToCDDB, 
                   char** ppzStr)
{
	char* pzTmp = NULL;
	char* pzPtr = *ppzStr;
	char* pzLastStart = *ppzStr;

	while(*pzPtr) {
        if (!bToCDDB) {
            if (*pzPtr == '\\' && *(pzPtr+1) == 'n') {
			    AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			    AppendString(&pzTmp, "\r\n", -1);
			    pzPtr += 2;
			    pzLastStart = pzPtr;
            }
            else if (*pzPtr == '\\' && *(pzPtr+1) == 't') {
			    AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			    AppendString(&pzTmp, "\t", -1);
			    pzPtr += 2;
			    pzLastStart = pzPtr;
            }
            else if (*pzPtr == '\\' && *(pzPtr+1) == '\\') {
			    AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			    AppendString(&pzTmp, "\\", -1);
			    pzPtr += 2;
			    pzLastStart = pzPtr;
            }
		    else
			    pzPtr ++;
		}
        else if (bToCDDB) {
            if (*pzPtr == '\r' && *(pzPtr + 1) == '\n') {
			    AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			    AppendString(&pzTmp, "\\n", -1);
			    pzPtr += 2;
			    pzLastStart = pzPtr;
            }
            else if (*pzPtr == '\t') {
			    AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			    AppendString(&pzTmp, "\\t", -1);
			    pzPtr ++;
			    pzLastStart = pzPtr;
            }
            else if (*pzPtr == '\\') {
			    AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			    AppendString(&pzTmp, "\\\\", -1);
			    pzPtr ++;
			    pzLastStart = pzPtr;
            }
		    else
			    pzPtr ++;
		}
	}

	if (pzLastStart != pzPtr)
		AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
	
	if (pzTmp) {
		delete[] *ppzStr;
		*ppzStr = pzTmp;
	}
}


void ChangeDefButton(HWND hDlg, int nIDSet, int nIDRemove)
{
   HWND hRemove = GetDlgItem(hDlg, nIDRemove);
   HWND hSet = GetDlgItem(hDlg, nIDSet);
   
   SendMessage(hDlg, DM_SETDEFID, nIDSet, 0);

   SendMessage(hSet, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
   SendMessage(hRemove, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
}


void CheckAmpersand(char* pzStr, BOOL bTooltip)
{
	unsigned int nLoop;
	unsigned int nLoop2;

	for (nLoop = 0 ; pzStr[nLoop] ; nLoop ++) {
		if (pzStr[nLoop] == '&') {
			if( bTooltip ) {		// tooltips want &&& instead of &&
				for (nLoop2 = strlen(pzStr) + 2 ; nLoop2 > nLoop + 1 ; nLoop2 --)
					pzStr[nLoop2] = pzStr[nLoop2-2];
				pzStr[nLoop2] = '&';
				nLoop += 2;
			} else {
				for (nLoop2 = strlen(pzStr) + 1 ; nLoop2 > nLoop ; nLoop2 --)
					pzStr[nLoop2] = pzStr[nLoop2-1];
			}
			nLoop++;
		}
	}
}

void CenterWindow(HWND hWnd, BOOL bVertical)
{
    RECT sRect;
    int nCX;
    int nCY;
    int nWndCX;
    int nWndCY;

    GetWindowRect(hWnd, &sRect);
    nWndCX = sRect.right - sRect.left;
    nWndCY = sRect.bottom - sRect.top;
    
    nCX = GetSystemMetrics(SM_CXSCREEN);
    nCY = GetSystemMetrics(SM_CYSCREEN);

    nCX /= 2;
    nCY /= 2;

    sRect.left = nCX - nWndCX / 2;
    if (bVertical)
        sRect.top = nCY - nWndCY / 2;

    SetWindowPos(hWnd, 0, sRect.left, sRect.top, nWndCX, nWndCY, 0);
}

void StringCpyZ( char *pzDest, const char *pzSrc, size_t size )
{
	if( size ) {
		while( --size && (*pzDest++ = *pzSrc++) );
		*pzDest = '\0';
	}
}

void StringCatZ( char *pzDest, const char *pzSrc, size_t size )
{
	if( size ) {
		while( --size && *pzDest++ );
		if( size ) {
			pzDest--;
			while( --size && (*pzDest++ = *pzSrc++) );
		}
		*pzDest = '\0';
	}
}

void StringPrintf( char *pzDest, size_t size, const char *pzFmt, ... )
{
	va_list	argptr;

	if( size ) {
		va_start( argptr, pzFmt );
		_vsnprintf( pzDest, size, pzFmt, argptr );
		va_end( argptr );
		pzDest[size-1] = 0;
	}
}

char *StringCopy(const char* pzStr)
{
	char* pzNewStr;

	pzNewStr = new char[strlen( pzStr ) + 1];
	strcpy( pzNewStr, pzStr );

	return pzNewStr;
}

void AppendString(
    char** ppzPtr,
    const char* pzStr,
	int nLen)
{
	int nStartLen = 0;

	if( nLen == -1 )
		nLen = strlen( pzStr );

    if( !*ppzPtr ) {
		*ppzPtr = new char[nLen + 1];
        CopyMemory( *ppzPtr, pzStr, nLen );
	} else {
		char *pzOldPtr = *ppzPtr;
		nStartLen = strlen( pzOldPtr );
        *ppzPtr = new char[nStartLen + nLen + 1];
		CopyMemory( *ppzPtr, pzOldPtr, nStartLen );
		CopyMemory( *ppzPtr + nStartLen, pzStr, nLen );
		delete[] pzOldPtr;
    }

	*(*ppzPtr + nStartLen + nLen) = 0;
}

void InitMenu(DISCINFO* psDI)
{
    HMENU hTracksMenu;

DebugPrintf("-> InitMenu");

    SetMenu(gs.hMainWnd, NULL);

    if (gs.hTrackMenu)
        DestroyMenu(gs.hTrackMenu);
    gs.hTrackMenu = LoadMenu(gs.hMainInstance, MAKEINTRESOURCE(IDR_MENU));

    if (!(gs.nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
		if (!(gs.nOptions & OPTIONS_ARTISTINMENU)) {
			gs.nMenuIndexTracks = 8;
			gs.nMenuIndexOther = 13;
			gs.nMenuIndexDevices = 14;
		}
		else {
			gs.nMenuIndexTracks = 10;
			gs.nMenuIndexOther = 15;
			gs.nMenuIndexDevices = 16;
		}
    }
    else {
		if (!(gs.nOptions & OPTIONS_ARTISTINMENU)) {
			gs.nMenuIndexOther = 11;
			gs.nMenuIndexDevices = 12;
		}
		else {
			gs.nMenuIndexOther = 13;
			gs.nMenuIndexDevices = 14;
		}

		HMENU hSubMenu = GetSubMenu(gs.hTrackMenu, 0);
        RemoveMenu(hSubMenu, 8, MF_BYPOSITION);
        RemoveMenu(hSubMenu, 8, MF_BYPOSITION);
    }

	if (gs.nOptions & OPTIONS_NOMENUBITMAP)
	{ 
		if (gs.hMenuBitmap)
		{
			DeleteObject (gs.hMenuBitmap);
			gs.hMenuBitmap = NULL;
		}
	}
	else
	{
		if (!gs.hMenuBitmap)
			gs.hMenuBitmap = LoadBitmap(gs.hMainInstance, MAKEINTRESOURCE(IDB_NOTIFY));

        HMENU hSubMenu = GetSubMenu(gs.hTrackMenu, 0);

        SetMenu(gs.hMainWnd, hSubMenu);

        InsertMenu(hSubMenu, 20, MF_STRING | MF_OWNERDRAW | MF_BYPOSITION | MF_MENUBARBREAK, 999, (char*)1);  
	}

    hTracksMenu = GetSubMenu(gs.hTrackMenu, 0);
    
    EnterCriticalSection(&gs.sDiscInfoLock);

	if (gs.nOptions & OPTIONS_ARTISTINMENU) {
		char szTmp[256];

		if (psDI->pzArtist)
            strcpy(szTmp, psDI->pzArtist);
        else
            strcpy(szTmp, "No Disc");
		if (psDI->pzTitle && *psDI->pzTitle) {
			strcat(szTmp, " - ");
			strcat(szTmp, psDI->pzTitle);
		}

		InsertMenu(hTracksMenu, 0, MF_STRING | MF_BYPOSITION, 9999, szTmp);
		InsertMenu(hTracksMenu, 1, MF_SEPARATOR | MF_BYPOSITION, 9999, "");
		SetMenuDefaultItem(hTracksMenu, 0, TRUE);
	}

	if (gs.state.bRepeat)
		ModifyMenu(hTracksMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_REPEAT, "Repeat");
	else
		ModifyMenu(hTracksMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING, IDM_REPEAT, "Repeat");

	if (gs.state.bRandomize)
		ModifyMenu(hTracksMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_RANDOMIZE, "Random Play");
	else
		ModifyMenu(hTracksMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING, IDM_RANDOMIZE, "Random Play");

    // Add devices to the devices sub menu or remove submenu if only one is present
    
    if (gs.nNumberOfDevices == 1)
        RemoveMenu(GetSubMenu(gs.hTrackMenu, 0), gs.nMenuIndexDevices, MF_BYPOSITION);
    else {
        HMENU hDevicesMenu;
        hDevicesMenu = GetSubMenu(GetSubMenu(gs.hTrackMenu, 0), gs.nMenuIndexDevices);

        if (hDevicesMenu) {
            char szTmp[80];

            RemoveMenu(hDevicesMenu, 0, MF_BYPOSITION);

            for (unsigned int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                if (gs.abDevices[nLoop - 'A']) {
                    sprintf(szTmp, "Drive %c:", nLoop);

                    if (nLoop - 'A' == gs.nCurrentDevice)
                        InsertMenu(hDevicesMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING | MF_CHECKED, IDM_DEVICES+nLoop, szTmp);
                    else
                        InsertMenu(hDevicesMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_DEVICES+nLoop, szTmp);
                }
            }
        }
    }

    // Fix tracks menu
    
    InitTracksMenu(psDI);

    if (!(gs.nOptions & OPTIONS_NOMENUBITMAP) && (gs.nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
        RemoveMenu(GetSubMenu(gs.hTrackMenu, 0), 999, MF_BYCOMMAND);
        InsertMenu(GetSubMenu(gs.hTrackMenu, 0), 21+(psDI->nProgrammedTracks*2), MF_STRING | MF_OWNERDRAW | MF_BYPOSITION | MF_MENUBARBREAK, 999, (char*)1);
    }

    LeaveCriticalSection(&gs.sDiscInfoLock);
    
    DrawMenuBar(gs.hMainWnd);
DebugPrintf("<- InitMenu");
}


void InitTracksMenu(DISCINFO* psDI)
{
    HMENU hTracksMenu;

    EnterCriticalSection(&gs.sDiscInfoLock);

DebugPrintf("-> InitTracksMenu");
    if ((gs.nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
        RemoveMenu(GetSubMenu(gs.hTrackMenu, 0), IDM_TRACKS, MF_BYCOMMAND);
        hTracksMenu = GetSubMenu(gs.hTrackMenu, 0);
    }
    else    
        hTracksMenu = GetSubMenu(GetSubMenu(gs.hTrackMenu, 0), gs.nMenuIndexTracks);

    if (hTracksMenu) {
        char szTmp[300];

        if (!(gs.nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
            while (RemoveMenu(hTracksMenu, 0, MF_BYPOSITION))
                ;
        }

        if (psDI->nProgrammedTracks < gs.nMenuBreak || (gs.nOptions & OPTIONS_NOMENUBREAK)) {
            gs.bBrokenTracksMenu = FALSE;

            for (unsigned int nLoop = 0 ; nLoop < psDI->nProgrammedTracks ; nLoop ++) {
                sprintf(szTmp, "%d. %s", psDI->pnProgrammedTracks[nLoop]+1, psDI->ppzTracks[psDI->pnProgrammedTracks[nLoop]]);

			    CheckAmpersand(szTmp, FALSE);
                if (!nLoop && (gs.nOptions & OPTIONS_TRACKSMENUCOLUMN))
                    AppendMenu(GetSubMenu(gs.hTrackMenu, 0), MF_MENUBARBREAK | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                else
                    InsertMenu(hTracksMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TRACKS+nLoop, szTmp);
            }

            if ((gs.nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
                for (nLoop = 0 ; nLoop < psDI->nProgrammedTracks ; nLoop ++) {
                    sprintf(szTmp, "[%s]", psDI->ppzTrackLen[psDI->pnProgrammedTracks[nLoop]]);
			        CheckAmpersand(szTmp, FALSE);
                    if (!nLoop && (gs.nOptions & OPTIONS_TRACKSMENUCOLUMN))
                        AppendMenu(GetSubMenu(gs.hTrackMenu, 0), MF_MENUBREAK | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                    else
                        InsertMenu(hTracksMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                }
            }
        }
        else {
            unsigned int nNumSubMenys = psDI->nProgrammedTracks / 10;

            if (psDI->nProgrammedTracks % 10)
                nNumSubMenys ++;

            gs.bBrokenTracksMenu = TRUE;

            for (unsigned int nLoop2 = 0 ; nLoop2 < nNumSubMenys ; nLoop2 ++) {
                unsigned int nLoop;
                HMENU hMenu = CreateMenu();
                char zTmp[300];
            
                if (nLoop2*10+10 < psDI->nProgrammedTracks)
                    sprintf(zTmp, "Track %d to %d", nLoop2*10 + 1, nLoop2*10+10);
                else
                    sprintf(zTmp, "Track %d to %d", nLoop2*10 + 1, psDI->nProgrammedTracks);

                if (!nLoop2 && (gs.nOptions & OPTIONS_TRACKSMENUCOLUMN))
                    AppendMenu(hTracksMenu, MF_MENUBARBREAK | MF_BYPOSITION | MF_POPUP | MF_STRING, (int)hMenu, zTmp);
                else
                    InsertMenu(hTracksMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_POPUP | MF_STRING, (int)hMenu, zTmp);
    
                for (nLoop = nLoop2*10 ; (nLoop - nLoop2*10) < 10 && nLoop < psDI->nProgrammedTracks ; nLoop ++) {
                    sprintf(szTmp, "%d. %s", psDI->pnProgrammedTracks[nLoop]+1, psDI->ppzTracks[psDI->pnProgrammedTracks[nLoop]]);

			        CheckAmpersand(szTmp, FALSE);
                    InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                }
            }
        }
    }

    LeaveCriticalSection(&gs.sDiscInfoLock);
DebugPrintf("<- InitTracksMenu");
}


void CheckProgrammed(MCIDEVICEID wDeviceID, 
                     DISCINFO* psDI)
{
	unsigned int nLoop;

    EnterCriticalSection(&gs.sDiscInfoLock);

    psDI->nCurrTrack = CDGetCurrTrack(wDeviceID) - 1;

	gs.state.bProgrammed = FALSE;
	// Check if programmed list is the same as the track order
	if (psDI->nProgrammedTracks == psDI->nMCITracks) {
        if (psDI->pnProgrammedTracks) {
            for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
			    if (psDI->pnProgrammedTracks[nLoop] != nLoop) {
				    gs.state.bProgrammed = TRUE;
				    nLoop = psDI->nMCITracks;
			    }
		    }
        }
	}
	else {
		gs.state.bProgrammed = TRUE;
	}

    // Find first match!
	for (nLoop = 0 ; nLoop < psDI->nProgrammedTracks; nLoop ++) {
		if (psDI->pnProgrammedTracks[nLoop] == psDI->nCurrTrack) {
			psDI->nCurrTrack = nLoop;
			nLoop = psDI->nProgrammedTracks + 1;
		}
	}
	if (nLoop != psDI->nProgrammedTracks + 2)
		CDStop(gs.wDeviceID);

    if (gs.state.bProgrammed && gs.state.bPlaying)
        CDPlay(gs.wDeviceID, 0);

    if (gs.pnLastRandomTracks)
        delete[] gs.pnLastRandomTracks;

    gs.pnLastRandomTracks = new unsigned int[psDI->nMCITracks];
    gs.nLastRandomTrack = 0;

    LeaveCriticalSection(&gs.sDiscInfoLock);
}


void UpdateTrackCombo(HWND hWnd, int nID)
{
    char szTmp[300];

    SendDlgItemMessage(hWnd, nID, CB_RESETCONTENT, 0, 0);

    for (unsigned int nLoop = 0 ; nLoop < gs.di[0].nProgrammedTracks ; nLoop ++) {
        sprintf(szTmp, "%d. %s", gs.di[0].pnProgrammedTracks[nLoop]+1, gs.di[0].ppzTracks[gs.di[0].pnProgrammedTracks[nLoop]]);

        SendDlgItemMessage(hWnd, nID, CB_ADDSTRING, 0, (LPARAM) szTmp);
    }
}


void SetHotkeys()
{
    int nLoop;
    int nModifier;
    int nHotkeyModifier;

    for (nLoop = 0 ; nLoop < NUM_BINDINGS ; nLoop ++) {
        if (gs.anHotkeys[nLoop]) {
            nHotkeyModifier = HIBYTE(gs.anHotkeys[nLoop]);
            nModifier = 0;
            if (nHotkeyModifier & HOTKEYF_ALT)
                nModifier |= MOD_ALT;
            if (nHotkeyModifier & HOTKEYF_CONTROL)
                nModifier |= MOD_CONTROL;
            if (nHotkeyModifier & HOTKEYF_SHIFT)
                nModifier |= MOD_SHIFT;

            RegisterHotKey(gs.hMainWnd, nLoop, nModifier, LOBYTE(gs.anHotkeys[nLoop]));
        }
    }
}


void EncodeBase64(char* pzDest, ULONG nLen, const char* pxData)
{
    const char azTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int nLoop;
    unsigned int nInLoop;  
    unsigned int nOutLoop;
    unsigned int nLineLength;
    unsigned int nValue;
    unsigned int nTmp;
    unsigned int nCharCount = 0;
    const char* pzStr;

    pzStr = pxData;

    nValue = 0;

    nLineLength = 0;

    nInLoop = nOutLoop = 0;
    while(nInLoop < nLen) {
        nValue = 0;
        nCharCount = 0;

        //
        // Add characters to nValue
        //    

        for (nLoop = 0 ; nLoop < 3 ; nLoop ++) {
            if (nInLoop < nLen)
                nValue += (unsigned char) (pzStr [nInLoop]);
            if (nCharCount != 2)
                nValue <<= 8;

            nCharCount ++;
            nInLoop ++;
        }

        if (nInLoop > nLen) { 
            if (nInLoop == nLen + 2) {
				
                //
                // Get first six bits
                //

                nTmp = (DWORD) ((nValue & 0xFC0000) >> 18);
                pzDest[nOutLoop] = azTable[nTmp];

                //
                // Get next six bits
                //

                nTmp = (DWORD) ((nValue & 0x3F000) >> 12);
                pzDest[nOutLoop+1] = azTable[nTmp];

                pzDest[nOutLoop+2] = '=';
                pzDest[nOutLoop+3] = '=';

                nOutLoop += 4;
            }        
            else if (nInLoop == nLen + 1) {
                //
                // Get first six bits
                //

                nTmp = (DWORD) ((nValue & 0xFC0000) >> 18);
                pzDest[nOutLoop] = azTable[nTmp];

                //
                // Get next six bits
                //

                nTmp = (DWORD) ((nValue & 0x3F000) >> 12);
                pzDest[nOutLoop+1] = azTable[nTmp];

                //
                // Get next six bits
                //

                nTmp = (DWORD) ((nValue & 0xFC0) >> 6);
                pzDest[nOutLoop+2] = azTable[nTmp];

                pzDest[nOutLoop+3] = '=';

                nOutLoop += 4;
            }
        }
        else {
            //
            // Get first six bits
            //

            nTmp = (DWORD) ((nValue & 0xFC0000) >> 18);
            pzDest[nOutLoop] = azTable[nTmp];

            //
            // Get next six bits
            //

            nTmp = (DWORD) ((nValue & 0x3F000) >> 12);
            pzDest[nOutLoop+1] = azTable[nTmp];

            //
            // Get next six bits
            //

            nTmp = (DWORD) ((nValue & 0xFC0) >> 6);
            pzDest[nOutLoop+2] = azTable[nTmp];

            //
            // Get last six bits
            //

            nTmp = (DWORD) (nValue & 0x3F);
            pzDest[nOutLoop+3] = azTable[nTmp];

            nOutLoop += 4;
        }

        if (nLineLength >= 72) {
            pzDest[nOutLoop] = '\r';
            pzDest[nOutLoop+1] = '\n';

            nOutLoop += 2;
            nLineLength = 0;
        }
    }
}


BOOL GetString(
    SOCKET s, 
    char* pzStr, 
    int/* nLen*/,
    BOOL bNoClose)
{
	unsigned long nRead;
	int nTime;
	char xCh;
	int nPos;
	int nTimeout = gs.cddb.nRemoteTimeout;

    nTimeout *= 10;

    nTime = 0;
	nPos = 0;
	pzStr[0] = 0;

	while(nTime < nTimeout) {
		ioctlsocket(s, FIONREAD, &nRead);
		if (nRead) {
			while(nRead --) {
				recv(s, &xCh, 1, 0);

#if SOCKET_DEBUG
DebugPrintf("SocketDebug: %c (%d)", xCh, xCh);
#endif
                pzStr[nPos ++] = xCh;
				pzStr[nPos] = 0;

                if (xCh == '\n') {
                    pzStr[nPos-1] = 0;
DebugPrintf("Received: %s", pzStr);
				    return 1;
                }
			}
		}
		else {
			Sleep(100);
			nTime ++;
		}
	}

    DebugPrintf("GetString() failed %d, nPos = %d", WSAGetLastError(), nPos);

    if (!bNoClose) {
        MessageBox(GetForegroundWindow(), "Remote connection timed out", APPNAME, MB_OK | MB_ICONERROR);

        closesocket(s);

        shutdown(s, 0);
    }

    return 0;
}


int SendString(SOCKET s, char* pzStr) 
{
    if (strlen(pzStr) < 800)
        DebugPrintf("Sending: %s", pzStr);
    else
        DebugPrintf("Sending: Overflow");

    if (send(s, pzStr, strlen(pzStr), 0) == SOCKET_ERROR ||
		send(s, "\r\n", 2, 0) == SOCKET_ERROR) {
        DebugPrintf("send() failed %d", WSAGetLastError());

        closesocket(s);
    
        shutdown(s, 0);

        return SOCKET_ERROR;
    }

    return 1;
}


DWORD GetAddress(char* pzMachine)
{
    struct hostent* psHostEntry;
    DWORD dwAddress;

    DebugPrintf("-> GetAddress %s", pzMachine);

    dwAddress = inet_addr(pzMachine);
    if (dwAddress == INADDR_NONE) {
        psHostEntry = gethostbyname(pzMachine);
	    if (!psHostEntry) {
		    DebugPrintf("gethostbyname() failed to get address for %s. Error code: %d", pzMachine, WSAGetLastError());

		    return INADDR_NONE;
	    }
        CopyMemory( &dwAddress, psHostEntry->h_addr_list[0], sizeof(DWORD) );
    }

    DebugPrintf("<- GetAddress %s", pzMachine);

    return dwAddress;
}


/////////////////////////////////////////////////////////////////////
//
// ASK FOR PASSWORD DIALOG
//
/////////////////////////////////////////////////////////////////////

BOOL CALLBACK AskForPasswordDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM/*  lParam*/)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            CenterWindow(hWnd, TRUE);                              
        }
		break;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK: {
                    GetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), gs.cddb.zProxyPassword, 256);

                    EndDialog(hWnd, IDOK);
                }
                break;

                case IDCANCEL: {
                    EndDialog(hWnd, IDCANCEL);
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


int SendAuthentication(SOCKET s)
{
    char zStr[256];
    char zEncoded[256];

    if ((gs.cddb.nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD) && gs.cddb.zProxyPassword[0] == 0) {
        if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_ASKFORPASSWORD), GetForegroundWindow(), (DLGPROC)AskForPasswordDlgProc) != IDOK)
            return SOCKET_ERROR;
    }

    StringPrintf(zStr, sizeof(zStr), "%s:%s", gs.cddb.zProxyUser, gs.cddb.zProxyPassword);

    ZeroMemory(zEncoded, sizeof(zEncoded));
    EncodeBase64(zEncoded, strlen(zStr), zStr);

    StringPrintf(zStr, sizeof(zStr), "Proxy-Authorization: Basic %s", zEncoded);

    return SendString(s, zStr);
}


DWORD __stdcall CheckVersionThread(LPVOID) 
{
	DebugPrintf("-> CheckVersionThread");
	
	CheckForNewVersion(TRUE);

	DebugPrintf("<- CheckVersionThread");

	return 0;
}


BOOL CheckForNewVersion(BOOL bUnattended)
{
	WSADATA sData;
    struct sockaddr_in sAddrServer;
    char zStr[8192];
    char zTmp[256];
	char zURL[1024];
	char zVersionNumber[80];
	char zVersionString[80];
    SOCKET s;

	DebugPrintf("CheckForNewVersion... %d", bUnattended);

	if (bUnattended == FALSE && MessageBox(NULL, "This check requires HTTP access or a proxy configured in the CDDB Remote tab. Do you want to continue?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDNO)
		return TRUE;

	if (WSAStartup(0x0101, &sData)) {
		if (bUnattended == FALSE)
			MessageBox(NULL, "Failed to initialize winsock!", APPNAME, MB_OK | MB_ICONERROR);

		DebugPrintf("WSAStartup() failed!");

		return FALSE;
	}

    /////////////////////////////////////////////////////////////
    //
    // Check for new version!
    //
    /////////////////////////////////////////////////////////////

    // Create socket   
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        DebugPrintf("socket() failed %d", WSAGetLastError());
        goto err;
    }

	// Use the CDDB proxy
	if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY) {
		sAddrServer.sin_port = htons((short)gs.cddb.nRemoteProxyPort);

        sAddrServer.sin_addr.s_addr = GetAddress(gs.cddb.zRemoteProxyServer);
        if (sAddrServer.sin_addr.s_addr == INADDR_NONE) {
			if (bUnattended == FALSE)
				MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK | MB_ICONERROR);
			goto err;
        }

		DebugPrintf("Check using HTTP proxy on port %d", gs.cddb.nRemoteProxyPort);
	}
	else {
		sAddrServer.sin_port = htons((short)80);

        sAddrServer.sin_addr.s_addr = GetAddress(VERSION_SERVER);
        if (sAddrServer.sin_addr.s_addr == INADDR_NONE) {
    		if (bUnattended == FALSE)
				MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK | MB_ICONERROR);
			goto err;
        }

		DebugPrintf("Check using HTTP");
	}

	// Set up remote address

	sAddrServer.sin_family = AF_INET;

	if (connect(s, (struct sockaddr*)&sAddrServer, sizeof(sAddrServer))) {
		if (bUnattended == FALSE)
			MessageBox(NULL, "Failed to connect to remote server!", APPNAME, MB_OK | MB_ICONERROR);

		DebugPrintf("connect() failed %d", WSAGetLastError());
		goto err;
	}
    
	if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY)
		DebugPrintf("Connected to proxy %s", gs.cddb.zRemoteProxyServer);
	else
		DebugPrintf("Connected to %s", VERSION_SERVER);

    // Build query
	if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY) 
        StringPrintf(zStr, sizeof(zStr), "GET http://%s%s HTTP/1.0", VERSION_SERVER, VERSION_PATH);
    else
        StringPrintf(zStr, sizeof(zStr), "GET %s HTTP/1.0", VERSION_PATH);

	// Send query
	if (SendString(s, zStr) == SOCKET_ERROR)
		goto err;
    if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEAUTHENTICATION) {
        if (SendAuthentication(s) == SOCKET_ERROR)
			goto err;
    }
    StringPrintf(zStr, sizeof(zStr), "User-Agent: %s", gs.cddb.zUserAgent);
	if (SendString(s, zStr) == SOCKET_ERROR)
		goto err;
	if (SendString(s, "") == SOCKET_ERROR)
		goto err;
	if (SendString(s, "") == SOCKET_ERROR)
		goto err;

	//
	// Get query result
	//

	// Get result
	if (!GetString(s, zStr, 1024))
		goto err;
	if (!strncmp(zStr, "HTTP/", 5) && !strncmp(&zStr[9], "200", 3)) {
		do {
			if (!GetString(s, zStr, 1024))
				goto err;
		} while(zStr[0] != 0 && zStr[0] != '\r' && zStr[0] != '\n');

		if (!GetString(s, zVersionNumber, 80))
			goto err;
		if (!GetString(s, zVersionString, 80))
			goto err;
		if (!GetString(s, zURL, 1024))
			goto err;
    }
	else if (!strncmp(zStr, "HTTP/", 5) && !strncmp(&zStr[9], "404", 3)) {
		// File not found on remote host
	}
    else {
		if (strlen(zStr) < 100)
			StringPrintf(zTmp, sizeof(zTmp), "Remote server reported: %s", zStr);
		else
			StringPrintf(zTmp, sizeof(zTmp), "Remote server reported an error");

		if (bUnattended == FALSE)
			MessageBox(NULL, zTmp, APPNAME, MB_OK | MB_ICONERROR);
		goto err;
    }

	if (atoi(zVersionNumber) > VERSION_VERSION) {
		DebugPrintf("Newer version exists on server");

		StringPrintf(zTmp, sizeof(zTmp), "A newer version (%s) exists. Do you want to go to the download page?", zVersionString);

		if (MessageBox(NULL, zTmp, APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
			DebugPrintf("Downloading newer version");

			if (ShellExecute(NULL, "open", zURL, NULL, ".", SW_SHOWNORMAL) <= (HINSTANCE) 32) {
				sprintf(zTmp, "Failed to open URL %s", zURL);

				MessageBox(NULL, zTmp, APPNAME, MB_OK | MB_ICONERROR);

				goto err;
			}
		}
	}
	else {
		DebugPrintf("You seem to run the latest version!");

		if (bUnattended == FALSE)
			MessageBox(NULL, "You seem to run the latest version!", APPNAME, MB_OK | MB_ICONINFORMATION);
	}

	WSACleanup ();
    closesocket( s );
    shutdown( s, 0 );

	return TRUE;

err:

	WSACleanup ();

	if( s != INVALID_SOCKET ) {
		closesocket( s );
		shutdown( s, 0 );
	}

	return FALSE;
}


void ParseServerInfo(CDDB_SERVER* psServer, char* pzStr)
{
    char* pzPtr;
    char* pzPos = pzStr;

    __try {
        pzPtr = strchr(pzPos, ' ');
        *pzPtr = 0;
        strcpy(psServer->zSite, pzPos);
        pzPos = pzPtr + 1;

        pzPtr = strchr(pzPos, ' ');
        *pzPtr = 0;
        strcpy(psServer->zProtocol, pzPos);
        pzPos = pzPtr + 1;

        pzPtr = strchr(pzPos, ' ');
        *pzPtr = 0;
        psServer->nPort = atoi(pzPos);
        pzPos = pzPtr + 1;

        pzPtr = strchr(pzPos, ' ');
        *pzPtr = 0;
        strcpy(psServer->zAddress, pzPos);
        pzPos = pzPtr + 1;

        pzPtr = strchr(pzPos, ' ');
        *pzPtr = 0;
        strcpy(psServer->zLatitude, pzPos);
        pzPos = pzPtr + 1;

        pzPtr = strchr(pzPos, ' ');
        *pzPtr = 0;
        strcpy(psServer->zLongitude, pzPos);
        pzPos = pzPtr + 1;

        strcpy(psServer->zDescription, pzPos);
    }
    __except(TRUE) {
    }
}

unsigned int CDGetLastTrackInARow(unsigned int nTrack)
{
	unsigned int nLoop;
    
    for (nLoop = nTrack + 1 ; nLoop < gs.di[0].nProgrammedTracks ; nLoop ++) {
        if (gs.di[0].pnProgrammedTracks[nLoop] != gs.di[0].pnProgrammedTracks[nLoop-1] + 1) {
            gs.nNextProgrammedTrack = nLoop;
            return gs.di[0].pnProgrammedTracks[nLoop-1]+1;
        }
    }

    gs.nNextProgrammedTrack = gs.di[0].nProgrammedTracks;

    return gs.di[0].pnProgrammedTracks[nLoop-1]+1;
}


void RunExternalCommand(DISCINFO* psDI)
{
    EnterCriticalSection(&gs.sDiscInfoLock);

	// Run external command if any
	if (gs.zExternalCommand[0] && psDI->pzArtist && psDI->pzTitle) {
		char zCmd[512];
		STARTUPINFO sStartup;
		PROCESS_INFORMATION sProcessInfo;

		sprintf(zCmd, "%s \"%s\" \"%s\" %s %s", gs.zExternalCommand, psDI->pzArtist, psDI->pzTitle, psDI->zMCIID, psDI->zCDDBID);

		DebugPrintf("Running external command '%s'", zCmd);

		ZeroMemory(&sStartup, sizeof(sStartup));
		sStartup.cb = sizeof(sStartup);
		sStartup.dwFlags = STARTF_USESHOWWINDOW;
		sStartup.wShowWindow = SW_HIDE;

		if (CreateProcess(NULL, zCmd, NULL, NULL, FALSE, 0, NULL, NULL, &sStartup, &sProcessInfo)) {
			CloseHandle(sProcessInfo.hProcess);
			CloseHandle(sProcessInfo.hThread);
		}
	}

    LeaveCriticalSection(&gs.sDiscInfoLock);
}


void ParseDiscInformationFormat(DISCINFO* psDI,
								const char* pzFormat, 
								char* pzDest)
{
	unsigned int nFormatLoop;
	unsigned int nDestLoop = 0;

	pzDest[0] = 0;

	if (!gs.state.bInit)
		return;

    EnterCriticalSection(&gs.sDiscInfoLock);

	for (nFormatLoop = 0 ; pzFormat[nFormatLoop] ; nFormatLoop ++) {
		if (pzFormat[nFormatLoop] == '%') {
			switch(pzFormat[nFormatLoop+1]) {
				case '0':			// Artist
					strcat(pzDest, psDI->pzArtist);
					nDestLoop = strlen(pzDest);
					break;

				case '1':			// Title
					strcat(pzDest, psDI->pzTitle);
					nDestLoop = strlen(pzDest);
					break;

				case '2':			// Track Title
					if (psDI->nCurrTrack < psDI->nProgrammedTracks && psDI->nCurrTrack != -1) {
						strcat(pzDest, psDI->ppzTracks[psDI->pnProgrammedTracks[psDI->nCurrTrack]]);
						nDestLoop = strlen(pzDest);
					}
					break;

				case '3':			// Category
					strcat(pzDest, psDI->pzCategory);
					nDestLoop = strlen(pzDest);
					break;

				case '4': {			// Track no
					char szTmp[32];

					if (psDI->nCurrTrack < psDI->nProgrammedTracks && psDI->nCurrTrack != -1) {
						itoa(psDI->pnProgrammedTracks[psDI->nCurrTrack]+1, szTmp, 10);
						strcat(pzDest, szTmp);
						nDestLoop = strlen(pzDest);
					}
				}
					break;

				case '5':			// Track length
					if (psDI->nCurrTrack < psDI->nProgrammedTracks && psDI->nCurrTrack != -1) {
						strcat(pzDest, psDI->ppzTrackLen[psDI->pnProgrammedTracks[psDI->nCurrTrack]]);
						nDestLoop = strlen(pzDest);
					}
					break;

				case 'a': {			// Track pos
					char szTmp[32];

					CDGetTime(gs.wDeviceID, TIME_TRACK, szTmp);
					strcat(pzDest, szTmp);
					nDestLoop = strlen(pzDest);
				}
					break;

				case 'b': {			// Track remaining
					char szTmp[32];

					CDGetTime(gs.wDeviceID, TIME_TRACKREM, szTmp);
					strcat(pzDest, szTmp);
					nDestLoop = strlen(pzDest);
				}
					break;

				case 'c': {			// CD pos
					char szTmp[32];

					CDGetTime(gs.wDeviceID, TIME_CD, szTmp);
					strcat(pzDest, szTmp);
					nDestLoop = strlen(pzDest);
				}
					break;

				case 'd': {			// CD remaining
					char szTmp[32];

					CDGetTime(gs.wDeviceID, TIME_CDREM, szTmp);
					strcat(pzDest, szTmp);
					nDestLoop = strlen(pzDest);
				}
					break;

				case '%':
					pzDest[nDestLoop++] = '%';
					pzDest[nDestLoop] = 0;
					break;
			}
			nFormatLoop ++;
		}
		else {
			pzDest[nDestLoop++] = pzFormat[nFormatLoop];
			pzDest[nDestLoop] = 0;
		}
	}

    LeaveCriticalSection(&gs.sDiscInfoLock);
}


HWND hLastForegroundWindow;

void UpdateCaption(HWND hWnd)
{
    RECT sWindowRect;
	HRGN hRgn;
    unsigned int nStyle;
	unsigned int nSizeY = 0;
	unsigned int nSizeX = 0;
    char szWindowTitle[256];

    GetWindowRect(hWnd, &sWindowRect);

    nStyle = GetWindowLong(hWnd, GWL_STYLE);

    // Y size
    if ((nStyle & WS_BORDER) == WS_BORDER && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
        nSizeY += GetSystemMetrics(SM_CYBORDER);
    }
    if ((nStyle & WS_THICKFRAME) == WS_THICKFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
        nSizeY += GetSystemMetrics(SM_CYFRAME);
    }
    if ((nStyle & WS_DLGFRAME) == WS_DLGFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
        nSizeY += GetSystemMetrics(SM_CYDLGFRAME);
    }
	if ((nStyle & WS_CAPTION) == WS_CAPTION) {
		nSizeY += GetSystemMetrics(SM_CYCAPTION);
	}
	
    // X size
    nSizeX = 5;
    if ((nStyle & WS_SYSMENU) == WS_SYSMENU) {
        nSizeX += GetSystemMetrics(SM_CXSIZE);
    }
    if ((nStyle & WS_MINIMIZEBOX) == WS_MINIMIZEBOX || (nStyle & WS_MAXIMIZEBOX) == WS_MAXIMIZEBOX) {
        nSizeX += 2*GetSystemMetrics(SM_CXSIZE);
    }
    if ((nStyle & WS_BORDER) == WS_BORDER && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
        nSizeX += GetSystemMetrics(SM_CXBORDER);
    }
    if ((nStyle & WS_THICKFRAME) == WS_THICKFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
        nSizeX += GetSystemMetrics(SM_CXFRAME);
    }
    if ((nStyle & WS_DLGFRAME) == WS_DLGFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
        nSizeX += GetSystemMetrics(SM_CXDLGFRAME);
    }
    
    sWindowRect.bottom = sWindowRect.top + nSizeY + 3;
    sWindowRect.right -= nSizeX;
    sWindowRect.left = gs.nLastCaptionXPos - 20;

	GetWindowText(hWnd, szWindowTitle, 256);
/*
    DebugPrintf("UpdateCaption (%s): nSizeY = %d, nSizeX = %d, Region: %d, %d, %d, %d", 
                          szWindowTitle,
                          nSizeY, 
                          nSizeX,
                          sWindowRect.left, 
						  sWindowRect.top,
						  sWindowRect.right,
						  sWindowRect.bottom);
*/
	hRgn = CreateRectRgn(sWindowRect.left, 
						 sWindowRect.top,
						 sWindowRect.right,
						 sWindowRect.bottom);
//	SendMessage(hWnd, WM_NCPAINT, (WPARAM)hRgn, 0);
    DefWindowProc(hWnd, WM_NCPAINT, (WPARAM) hRgn, 0);

//    UpdateWindow(hWnd);

	DeleteObject(hRgn);
}


void DrawOnCaption()
{
    HWND hForegroundWnd = GetForegroundWindow();
    char szName[80];
    unsigned int nStyle;
    unsigned int nExStyle;

    if (!hForegroundWnd)
        return;

    // Find out the actual parent of this foreground window 
    // in case it is a popup window with a caption
    if (GetParent(hForegroundWnd)) {
        HWND hWnd;

        hWnd = GetParent(hForegroundWnd);
        while(hWnd) {
            hForegroundWnd = hWnd;
            hWnd = GetParent(hForegroundWnd);
        }
    }

    // If we have changed the foreground window, 
    // repaint the last foreground window caption
    if (hForegroundWnd != hLastForegroundWindow && hLastForegroundWindow && IsWindow(hLastForegroundWindow)) {
//	    DebugPrintf("Caption of old foreground window updated");

		UpdateCaption(hLastForegroundWindow);
    }

    // Check if we are supposed to draw this info

    GetClassName(hForegroundWnd, szName, 80);
    nStyle = GetWindowLong(hForegroundWnd, GWL_STYLE);
    nExStyle = GetWindowLong(hForegroundWnd, GWL_EXSTYLE);

//                    DebugPrintf("ClassName %s", szName);

    if ((nStyle & WS_CAPTION) == WS_CAPTION && 
        !(nExStyle & WS_EX_TOOLWINDOW) &&
        strcmp(szName, "#32770") && 
        strcmp(szName, "Shell_TrayWnd")) {
        char szTmp[512];
        char szText[512];
        HDC hDC;
        RECT sRect;
        SIZE sSize;
        SIZE sSize2;
        NONCLIENTMETRICS sMetrics;
        HFONT hDrawFont;
        signed int nLeft;
        unsigned int nLeftSpace = 0;
        unsigned int nRightSpace = 0;
        unsigned int nDrawCaptionPos = 0;
        RECT sDrawRect;
        
		hLastForegroundWindow = hForegroundWnd;

        UpdateDiscInformation(&gs.di[0], FALSE, FALSE, szText);

        hDC = GetWindowDC(hForegroundWnd);

        GetWindowText(hForegroundWnd, szTmp, 511);

        sMetrics.cbSize = sizeof(sMetrics);

        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &sMetrics, FALSE);

        // First we need to calculate the actual length of the active window text

        hDrawFont = CreateFontIndirect(&sMetrics.lfCaptionFont);

        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

        GetTextExtentPoint(hDC, szTmp, strlen(szTmp), &sSize2);

        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

        DeleteObject(hDrawFont);

        // Now use the user defined font

        hDrawFont = CreateFontIndirect(&gs.sCaptionFont);

        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

        GetWindowRect(hForegroundWnd, &sRect);

        GetTextExtentPoint(hDC, szText, strlen(szText), &sSize);

        nRightSpace = 10;

//#define DEBUG_SHOW

#ifdef DEBUG_SHOW
        DebugPrintf("-----");
#endif
        // Close button
        if ((nStyle & WS_SYSMENU) == WS_SYSMENU) {
#ifdef DEBUG_SHOW
            DebugPrintf("WS_SYSMENU");
#endif
            nRightSpace += GetSystemMetrics(SM_CXSIZE);
            nLeftSpace += GetSystemMetrics(SM_CXSIZE);
        }
        if ((nStyle & WS_MINIMIZEBOX) == WS_MINIMIZEBOX || (nStyle & WS_MAXIMIZEBOX) == WS_MAXIMIZEBOX) {
#ifdef DEBUG_SHOW
            DebugPrintf("WS_MINIMIZEBOX or WS_MAXIMIZEBOX");
#endif
            nRightSpace += 2*GetSystemMetrics(SM_CXSIZE);
        }
        if ((nStyle & WS_BORDER) == WS_BORDER && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
#ifdef DEBUG_SHOW
            DebugPrintf("WS_BORDER");
#endif
            nRightSpace += GetSystemMetrics(SM_CXBORDER);
            nLeftSpace += GetSystemMetrics(SM_CXBORDER);

            nDrawCaptionPos += GetSystemMetrics(SM_CXBORDER);
        }
        if ((nStyle & WS_THICKFRAME) == WS_THICKFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
#ifdef DEBUG_SHOW
            DebugPrintf("WS_THICKFRAME");
#endif
            nRightSpace += GetSystemMetrics(SM_CXFRAME);
            nLeftSpace += GetSystemMetrics(SM_CXFRAME);

            nDrawCaptionPos += GetSystemMetrics(SM_CXFRAME);
        }
        if ((nStyle & WS_DLGFRAME) == WS_DLGFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
#ifdef DEBUG_SHOW
            DebugPrintf("WS_DLGFRAME");
#endif
            nRightSpace += GetSystemMetrics(SM_CXDLGFRAME);
            nLeftSpace += GetSystemMetrics(SM_CXDLGFRAME);

            nDrawCaptionPos += GetSystemMetrics(SM_CXDLGFRAME);
        }

        nLeft = sRect.right - sRect.left - nRightSpace - sSize.cx;

        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, gs.nCaptionFontColor);

        sDrawRect.top = GetSystemMetrics(SM_CYFRAME) + (GetSystemMetrics(SM_CYCAPTION) - sSize.cy) / 2 - 1;
        sDrawRect.bottom = sDrawRect.top + sSize.cy;

        UpdateCaption(hForegroundWnd);

        gs.nLastCaptionLen = sSize.cx;

        if (nLeft > (signed) (sSize2.cx + nLeftSpace + 30)) {
            sDrawRect.left = nLeft;
            sDrawRect.right = sDrawRect.left + sSize.cx;
#ifdef DEBUG_SHOW
            DebugPrintf("Text fits");
#endif
        }
        else {
            sDrawRect.left = sSize2.cx + 30 + nLeftSpace;
            sDrawRect.right = nLeft + sSize.cx;
#ifdef DEBUG_SHOW
            DebugPrintf("Text too large");
#endif
        }
       
        gs.nLastCaptionXPos = sDrawRect.left;

        DrawText(hDC, szText, strlen(szText), &sDrawRect, DT_LEFT | DT_VCENTER | DT_NOPREFIX);

        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

        DeleteObject(hDrawFont);

        ReleaseDC(hForegroundWnd, hDC);
        
        hLastForegroundWindow = hForegroundWnd;
    }
    else {
//                        DebugPrintf("ShowOnCaption: No paint");

        hLastForegroundWindow = hForegroundWnd;
    }
}


/////////////////////////////////////////////////////////////////////
//
// PROGRESS STUFF!
//
/////////////////////////////////////////////////////////////////////

HWND hProgressWnd;

BOOL CALLBACK ProgressDlgProc(
    HWND  /*hWnd*/,
    UINT  nMsg,
    WPARAM  /*wParam*/,
    LPARAM  /*lParam*/)
{
    switch(nMsg) {
        case WM_INITDIALOG:
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


struct PROGRESSINFO {
    BOOL bKill;
	char zStr[80];
    HWND hParent;
	DWORD dwMax;
	int nStyle;
} sProgressInfo;

unsigned int __stdcall ProgressThread(void* /*pvPtr*/)
{
    MSG sMsg;

    hProgressWnd = CreateDialog(gs.hMainInstance, MAKEINTRESOURCE(IDD_PROGRESS), sProgressInfo.hParent, (DLGPROC)ProgressDlgProc);
	SetWindowText(hProgressWnd, APPNAME);
    CenterWindow(hProgressWnd);

	if (sProgressInfo.nStyle == 1)
		ShowWindow(GetDlgItem(hProgressWnd, IDC_PROGRESS), SW_HIDE);
	else if (sProgressInfo.nStyle == 0) {
		SendDlgItemMessage(hProgressWnd, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, sProgressInfo.dwMax));

		ShowWindow(GetDlgItem(hProgressWnd, IDC_COUNT), SW_HIDE);
	}
    else if (sProgressInfo.nStyle == 2) {
		ShowWindow(GetDlgItem(hProgressWnd, IDC_PROGRESS), SW_HIDE);
		ShowWindow(GetDlgItem(hProgressWnd, IDC_COUNT), SW_HIDE);
    }

    ShowWindow(hProgressWnd, SW_SHOW);
    SetDlgItemText(hProgressWnd, IDC_TITLE, sProgressInfo.zStr);

	SetForegroundWindow(hProgressWnd);

    while(GetMessage(&sMsg, NULL, 0, 0) && !sProgressInfo.bKill) {
        TranslateMessage(&sMsg);
        DispatchMessage(&sMsg);
    }

    DestroyWindow(hProgressWnd);

	return 0;
}


void ProgressOpen(HWND hParent, char* pzStr, int nStyle, DWORD dwMax)
{
    unsigned int nID;

	sProgressInfo.bKill = FALSE;
    sProgressInfo.hParent = hParent;
    sProgressInfo.dwMax = dwMax;
    sProgressInfo.nStyle = nStyle;
    strcpy(sProgressInfo.zStr, pzStr);

    _beginthreadex(NULL, 0, ProgressThread, 0, 0, &nID);
}


void ProgressSetStr(char* pzStr)
{
    SetDlgItemText(hProgressWnd, IDC_TITLE, pzStr);
}


void ProgressClose()
{
	sProgressInfo.bKill = TRUE;
	while(IsWindow(hProgressWnd))
		PostMessage(hProgressWnd, WM_LBUTTONDOWN, 0, 0);
}


void ProgressSet(int nCount)
{
    if (sProgressInfo.nStyle == 1) {
		char zStr[32];

		sprintf(zStr, "%d", nCount);

		if (hProgressWnd && IsWindow(hProgressWnd))
			SetDlgItemText(hProgressWnd, IDC_COUNT, zStr);
	}
	else if (sProgressInfo.nStyle == 0)
		SendDlgItemMessage(hProgressWnd, IDC_PROGRESS, PBM_SETPOS, nCount, 0);
}

/////////////////////////////////////////////////////////////////////
//
// QUEUE STUFF!
//
/////////////////////////////////////////////////////////////////////

BOOL AddToQueue(DISCINFO* psDI)
{
	char zSection[32];
	char zKey[32];
	unsigned int nLoop;

    DebugPrintf("-> AddToQueue");

    for (nLoop = 0 ; nLoop < gs.nNumberOfItemsInQueue ; nLoop ++) {
    	sprintf(zSection, "Queue%d", gs.nNumberOfItemsInQueue);
        
        ProfileGetString(zSection, "CDDBID", "", zKey, sizeof(zKey));
        if (!stricmp(zKey, psDI->zCDDBID)) {
            DebugPrintf("<- Entry already in queue");

            return FALSE;
        }
    }

    gs.nNumberOfItemsInQueue ++;

	ProfileWriteInt("NTFY_CD", "QueueItems", gs.nNumberOfItemsInQueue);

	sprintf(zSection, "Queue%d", gs.nNumberOfItemsInQueue);

	ProfileWriteString(zSection, "MCIID", psDI->zMCIID);
	ProfileWriteString(zSection, "CDDBID", psDI->zCDDBID);

	ProfileWriteInt(zSection, "Tracks", psDI->nMCITracks);
	for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
		sprintf(zKey, "Track%d", nLoop);
		ProfileWriteInt(zSection, zKey, psDI->pnFrames[nLoop]);
	}

	ProfileWriteInt(zSection, "Length", psDI->nDiscLength);

    DebugPrintf("<- AddToQueue");

    return TRUE;
}


void RemoveQueueItem(int nItem) 
{
    unsigned int nLoop;
	char zSection[32];
    char* pzData;

    DebugPrintf("-> RemoveQueueItem %d", nItem);

    pzData = new char[32000];

    for (nLoop = nItem ; nLoop < gs.nNumberOfItemsInQueue - 1 ; nLoop ++) {
	    sprintf(zSection, "Queue%d", nLoop+1);

        ProfileGetSection(zSection, pzData, 32000);

        sprintf(zSection, "Queue%d", nLoop);

        ProfileWriteSection(zSection, pzData);
    }
    
    delete[] pzData;

    sprintf(zSection, "Queue%d", nLoop);
    ProfileWriteSection(zSection, "\0\0");

    gs.nNumberOfItemsInQueue --;

	ProfileWriteInt("NTFY_CD", "QueueItems", gs.nNumberOfItemsInQueue);

    DebugPrintf("<- RemoveQueueItem");
}


BOOL RetrieveQueueItem(int nItem)
{
    DISCINFO sDI;
	char zSection[32];
	char zKey[32];
	unsigned int nLoop;
	BOOL bServerError;
    BOOL bFound = FALSE;

    DebugPrintf("-> RetrieveQueueItem %d", nItem);

    ZeroMemory(&sDI, sizeof(sDI));

    sprintf(zSection, "Queue%d", nItem);

	ProfileGetString(zSection, "MCIID", "", sDI.zMCIID, sizeof(sDI.zMCIID));
	ProfileGetString(zSection, "CDDBID", "", sDI.zCDDBID, sizeof(sDI.zCDDBID));

	sDI.nMCITracks = ProfileGetInt(zSection, "Tracks", 0);
    sDI.pnFrames = new unsigned int[sDI.nMCITracks];

    for (nLoop = 0 ; nLoop < sDI.nMCITracks ; nLoop ++) {
		sprintf(zKey, "Track%d", nLoop);

		sDI.pnFrames[nLoop] = ProfileGetInt(zSection, zKey, 0);
	}

	sDI.nDiscLength = ProfileGetInt(zSection, "Length", 0);

    CDDBQueryRemote(&sDI, TRUE, &bServerError);
    if (sDI.bDiscFound) {
        SetDiscInfo(&sDI);

		if (!stricmp(sDI.zCDDBID, gs.di[0].zCDDBID)) {
			DebugPrintf("RetrieveQueueItem found the current disc. Update current information");
			DiscInit(&gs.di[0]);
		}

        FreeDiscInfo(&sDI);

        bFound = TRUE;
    }

    DebugPrintf("<- RetrieveQueueItem");

    return bFound;
}


void GetQueuedItems()
{
    unsigned int nLoop;

    DebugPrintf("-> GetQueuedItems");

    for (nLoop = gs.nNumberOfItemsInQueue ; nLoop > 0 ; nLoop --) {
        if (RetrieveQueueItem(nLoop) == TRUE)
            RemoveQueueItem(nLoop);
    }

    DebugPrintf("<- GetQueuedItems");
}

/////////////////////////////////////////////////////////////////////////
//
// DISC INFO
// 
/////////////////////////////////////////////////////////////////////////

void InitDiscInfo(DISCINFO* psDI)
{
    ZeroMemory(psDI, sizeof(DISCINFO));
}


void FreeDiscInfo(DISCINFO* psDI)
{
    unsigned int nLoop;

    DebugPrintf("-> FreeDiscInfo() %s:%s:%s:%s", psDI->zMCIID, psDI->zCDDBID, psDI->pzArtist ? psDI->pzArtist : "", psDI->pzTitle ? psDI->pzTitle : "");

    if (psDI->ppzTrackLen) {
        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++)
            delete[] psDI->ppzTrackLen[nLoop];
        delete[] psDI->ppzTrackLen;
    }

    if (psDI->pnTrackLen)
        delete[] psDI->pnTrackLen;

    if (psDI->ppzTracks) {
        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++)
            delete[] psDI->ppzTracks[nLoop];
        delete[] psDI->ppzTracks;
    }

    if (psDI->ppzTracksExt) {
        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++)
            delete[] psDI->ppzTracksExt[nLoop];
        delete[] psDI->ppzTracksExt;
    }

    if (psDI->pnProgrammedTracks)
        delete[] psDI->pnProgrammedTracks;

    if (psDI->pzArtist)
        delete[] psDI->pzArtist;

    if (psDI->pzTitle)
        delete[] psDI->pzTitle;

    if (psDI->pzDiscExt)
        delete[] psDI->pzDiscExt;

    if (psDI->pzCategory)
        delete[] psDI->pzCategory;

    if (psDI->pnFrames)
        delete[] psDI->pnFrames;

    if (psDI->pzDiscid)
        delete[] psDI->pzDiscid;

    if (psDI->pzSubmitted)
        delete[] psDI->pzSubmitted;

    if (psDI->pzOrder)
        delete[] psDI->pzOrder;

    ZeroMemory(psDI, sizeof(DISCINFO));

    DebugPrintf("<- FreeDiscInfo()");
}

#define COPYSTR(dest, src, field)                   if( !src->field ) dest->field = NULL; else dest->field = StringCopy( src->field );

#define COPYSTRLOOP(dest, src, field, loop)         if (!loop) dest->field = NULL; \
													else { dest->field = new char *[loop]; \
														for (nLoop = 0 ; nLoop < loop ; nLoop ++) \
															dest->field[nLoop] = StringCopy( src->field[nLoop] ); \
													}

#define COPYNUMLOOP(dest, src, field, loop)         if (!loop) dest->field = NULL; \
													else { dest->field = new unsigned int[loop]; \
                                                    for (nLoop = 0 ; nLoop < loop ; nLoop ++) \
                                                        dest->field[nLoop] = src->field[nLoop]; \
													}

void CopyDiscInfo(DISCINFO* psDIDest, 
                  DISCINFO* psDISrc)
{
    unsigned int nLoop;

    DebugPrintf("-> CopyDiscInfo() %s:%s:%s:%s", psDISrc->zMCIID, psDISrc->zCDDBID, psDISrc->pzArtist ? psDISrc->pzArtist : "", psDISrc->pzTitle ? psDISrc->pzTitle : "");

    FreeDiscInfo(psDIDest);    

    *psDIDest = *psDISrc;

    // Now copy pointers

    COPYSTR(psDIDest, psDISrc, pzArtist);
    COPYSTR(psDIDest, psDISrc, pzTitle);
    COPYSTR(psDIDest, psDISrc, pzCategory);
    COPYSTR(psDIDest, psDISrc, pzDiscExt);
    COPYSTR(psDIDest, psDISrc, pzOrder);
    COPYSTR(psDIDest, psDISrc, pzDiscid);
    COPYSTR(psDIDest, psDISrc, pzSubmitted);

    COPYSTRLOOP(psDIDest, psDISrc, ppzTracks, psDISrc->nTracks);
    COPYSTRLOOP(psDIDest, psDISrc, ppzTracksExt, psDISrc->nTracks);
    COPYSTRLOOP(psDIDest, psDISrc, ppzTrackLen, psDISrc->nTracks);

    COPYNUMLOOP(psDIDest, psDISrc, pnProgrammedTracks, psDISrc->nProgrammedTracks);
    COPYNUMLOOP(psDIDest, psDISrc, pnTrackLen, psDISrc->nMCITracks);
    COPYNUMLOOP(psDIDest, psDISrc, pnFrames, psDISrc->nTracks);

    DebugPrintf("<- CopyDiscInfo()");
}


void CopyQueryInfo(DISCINFO* psSrc, 
                   DISCINFO* psDst)
{
    InitDiscInfo(psDst);

    strcpy(psDst->zCDDBID, psSrc->zCDDBID);
    strcpy(psDst->zMCIID, psSrc->zMCIID);
    psDst->nMCITracks = psSrc->nMCITracks;
    psDst->pnFrames = new unsigned int[psSrc->nMCITracks];
    CopyMemory(psDst->pnFrames, psSrc->pnFrames, psSrc->nMCITracks*sizeof(unsigned int));
    psDst->nDiscLength = psSrc->nDiscLength;
}


void GetDiscInfo(MCIDEVICEID wDeviceID, 
                 DISCINFO* psDI)
{
	MCI_SET_PARMS sMCISet;
    unsigned int nLoop;
    char szStr[80];

DebugPrintf("-> GetDiscInfo()");

    EnterCriticalSection(&gs.sDiscInfoLock);

    psDI->nCurrTrack = CDGetCurrTrack(wDeviceID) - 1;

    psDI->ppzTrackLen = new char *[psDI->nMCITracks];
    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
        psDI->ppzTrackLen[nLoop] = new char[10];
		psDI->ppzTrackLen[nLoop][0] = 0;
	}

    psDI->pnTrackLen = new unsigned int[psDI->nMCITracks];

	sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	mciSendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
	
    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
		psDI->pnTrackLen[nLoop] = CDGetTrackLength(wDeviceID, nLoop + 1, szStr);
		strcpy(psDI->ppzTrackLen[nLoop], szStr);
    }

	sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	mciSendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);

    LeaveCriticalSection(&gs.sDiscInfoLock);
DebugPrintf("<- GetDiscInfo()");
}


BOOL SetDiscInfo(DISCINFO* psDI)
{
    BOOL bRet;
    unsigned int nLoop;
    char szNum[32];
    char szTmp[256];

    // Save the info!

    DebugPrintf("-> SetDiscInfo()");

    EnterCriticalSection(&gs.sDiscInfoLock);

    szTmp[0] = 0;

	for (nLoop = 0 ; nLoop < psDI->nProgrammedTracks ; nLoop ++) {
		if (szTmp[0])
			strcat(szTmp, " ");
		
		itoa(psDI->pnProgrammedTracks[nLoop], szNum, 10);
		strcat(szTmp, szNum);
	}

    if (psDI->pzOrder) {
        delete[] psDI->pzOrder;
        psDI->pzOrder = NULL;
    }

    AppendString(&psDI->pzOrder, szTmp, -1);

    bRet = DBSave(psDI);

    LeaveCriticalSection(&gs.sDiscInfoLock);

    DebugPrintf("<- SetDiscInfo()");

    return bRet;
}


void ValidateDiscInfo(MCIDEVICEID wDeviceID,
					  DISCINFO* psDI)
{
	char zTemp[64];
	unsigned int nLoop;

    if (!psDI->nMCITracks && wDeviceID)
        psDI->nMCITracks = CDGetTracks(wDeviceID);

    if (!psDI->nTracks)
        psDI->nTracks = psDI->nMCITracks;

    // Make sure the information is ok
    if (!psDI->pzArtist || !psDI->pzArtist[0]) {
        if (psDI->pzArtist)
            delete[] psDI->pzArtist;
        if( !psDI->bDiscFound )
            psDI->pzArtist = StringCopy( "New Artist" );
    }
    if (!psDI->pzTitle || !psDI->pzTitle[0]) {
        if (psDI->pzTitle)
            delete[] psDI->pzTitle;
        if( !psDI->bDiscFound )
            psDI->pzTitle = StringCopy( "New Title" );
    }
    if (!psDI->pzCategory || !psDI->pzCategory[0]) {
        if (psDI->pzCategory)
            delete[] psDI->pzCategory;
        psDI->pzCategory = StringCopy( "" );
    }
    if (!psDI->ppzTracksExt) {
        psDI->ppzTracksExt = new char *[psDI->nMCITracks];
        ZeroMemory( psDI->ppzTracksExt, psDI->nMCITracks * sizeof( char * ) );
    }

    if (!psDI->pzOrder)
        psDI->pzOrder = StringCopy( "" );

    if (!psDI->ppzTracks) {
        psDI->ppzTracks = new char *[psDI->nMCITracks];
        ZeroMemory( psDI->ppzTracks, psDI->nMCITracks * sizeof( char * ) );
    }

    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
        if( !psDI->ppzTracks[nLoop] ) {
			sprintf( zTemp, "Track %d", nLoop + 1 );
            psDI->ppzTracks[nLoop] = StringCopy( zTemp );
        }
	}

    if (!psDI->pzDiscExt)
        psDI->pzDiscExt = StringCopy( "" );

    FixCDDBString(FALSE, &psDI->pzDiscExt);

    if (!psDI->ppzTracksExt) {
        psDI->ppzTracksExt = new char *[psDI->nMCITracks];
        ZeroMemory(psDI->ppzTracksExt, psDI->nMCITracks * sizeof(char*));
    }

    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
        if( !psDI->ppzTracksExt[nLoop] )
	        psDI->ppzTracksExt[nLoop] = StringCopy( "" );
        FixCDDBString(FALSE, &psDI->ppzTracksExt[nLoop]);
    }

    if( !psDI->pzSubmitted )
		psDI->pzSubmitted = StringCopy( "" );

    // Count the number of programmed tracks

	psDI->nProgrammedTracks = 1;
    if (!psDI->pzOrder[0]) {
        psDI->nProgrammedTracks = psDI->nMCITracks;

        // Create fake list
        psDI->pnProgrammedTracks = new unsigned int[psDI->nMCITracks];
        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++)
            psDI->pnProgrammedTracks[nLoop] = nLoop;
    }
	else {
		for (nLoop = 0 ; psDI->pzOrder[nLoop] ; nLoop ++) {
			if (psDI->pzOrder[nLoop] == ' ')
				psDI->nProgrammedTracks++;
		}
	}
}


void ParsePlaylist( DISCINFO* psDI, BOOL bProgram )
{
	unsigned int nLoop;
	int nPos;
	char zTemp, *pzPtr;

	// get the programmed list!
	if( psDI->pnProgrammedTracks ) {
		delete[] psDI->pnProgrammedTracks;
		psDI->pnProgrammedTracks = NULL;
	}

	if( !bProgram || !psDI->nProgrammedTracks || !psDI->pzOrder[0] )
		goto create_default;

	DebugPrintf( "Got playlist, parse" );

	nPos = 0;
	nLoop = 0;
	while( psDI->pzOrder[nPos] ) {
		pzPtr = strchr( &psDI->pzOrder[nPos], ' ' );
		if( pzPtr )
			nPos += (pzPtr - &psDI->pzOrder[nPos]) + 1;
		else
			nPos += strlen( &psDI->pzOrder[nPos] );
		nLoop++;
	}

	if( !nLoop )
		goto create_default;

	psDI->nProgrammedTracks = nLoop;
	psDI->pnProgrammedTracks = new unsigned int[nLoop];

	nPos = 0;
	nLoop = 0;
	while( psDI->pzOrder[nPos] ) {
		pzPtr = strchr( &psDI->pzOrder[nPos], ' ' );

		if( pzPtr ) {
			zTemp = *pzPtr;
			*pzPtr = 0;
			psDI->pnProgrammedTracks[nLoop] = atoi( &psDI->pzOrder[nPos] );
			*pzPtr = zTemp;

			nPos += (pzPtr - &psDI->pzOrder[nPos]) + 1;
		} else {
			psDI->pnProgrammedTracks[nLoop] = atoi( &psDI->pzOrder[nPos] );
			nPos += strlen( &psDI->pzOrder[nPos] );
		}
		nLoop++;
	}
	return;

create_default:
	DebugPrintf( "No playlist, using default" );

	psDI->nProgrammedTracks = psDI->nMCITracks;
	psDI->pnProgrammedTracks = new unsigned int[psDI->nProgrammedTracks];
	for( nLoop = 0; nLoop < psDI->nProgrammedTracks; nLoop ++ )
		psDI->pnProgrammedTracks[nLoop] = nLoop;
}


void ResetPlaylist(DISCINFO* psDI, BOOL bProgram)
{
DebugPrintf("-> ResetPlaylist()");

    EnterCriticalSection(&gs.sDiscInfoLock);

    if (!bProgram)
        gs.state.bPlayWhole = TRUE;
    else
        gs.state.bPlayWhole = FALSE;

	ParsePlaylist(psDI, bProgram);

    // Update the "Tracks" menu!

    InitMenu(psDI);

    LeaveCriticalSection(&gs.sDiscInfoLock);

DebugPrintf("<- ResetPlaylist()");
}


DWORD APIPRIVATE QueryThread(LPVOID lpParam)
{
    DISCINFO* psDI = (DISCINFO*) lpParam;
	BOOL bServerError;

DebugPrintf("-> QueryThread()");

    CopyQueryInfo(psDI, &gs.sQueryThreadDI);

    DBGetDiscInfoRemote(gs.wDeviceID, &gs.sQueryThreadDI, &bServerError);

    if ((gs.sQueryThreadDI.bDiscFound && (gs.cddb.nCDDBOptions & OPTIONS_CDDB_STORECOPYININI) && (gs.nOptions & OPTIONS_USECDDB)) || 
        (gs.sQueryThreadDI.bDiscFound && (gs.nOptions & OPTIONS_STORERESULT)))
        SetDiscInfo(&gs.sQueryThreadDI);

    if (gs.sQueryThreadDI.bDiscFound) {
        DebugPrintf("QueryThread found discinfo");

        gs.bQueryThreadHasUpdatedDI = TRUE;
    }
    else if (bServerError) {
        DebugPrintf("QueryThread didn't find discinfo");

		if (gs.nOptions & OPTIONS_AUTOADDQUEUE) {
			DebugPrintf("Auto Queue");

			AddToQueue(psDI);
		}
	}
    else if (!bServerError && (gs.nOptions & OPTIONS_AUTOADDQUEUE)) {
        DebugPrintf("No server error so disc not added to queue");

        MessageBox(NULL, "Disc wasn't added to the queue since the server reported disc not found", APPNAME, MB_OK | MB_ICONINFORMATION);
    }

    // Try to retrieve queued items too
    if (bServerError == FALSE && (gs.nOptions & OPTIONS_AUTORETRIEVEQUEUE)) {
        DebugPrintf("QueryThread is retrieving queued items too");

        GetQueuedItems();
    }
        
DebugPrintf("<- QueryThread()");

    return 0;
}


void DiscInit(DISCINFO* psDI)
{
    DWORD dwTid;
    DISCINFO sQueryDI;

    if (gs.state.bInit)
        return;

DebugPrintf("DiscInit()");

    EnterCriticalSection(&gs.sDiscInfoLock);

    if (!gs.state.bCDOpened && !CDOpen(&gs.wDeviceID)) {
        DebugPrintf("Open device failed");

        gs.state.bMediaPresent = FALSE;
	    gs.state.bAudio = FALSE;
	    psDI->zCDDBID[0] = 0;
	    psDI->zMCIID[0] = 0;
	    gs.state.bPaused = FALSE;
	    gs.state.bPlaying = FALSE;
	    psDI->nCurrTrack = 0;
	    gs.state.bInit = FALSE;

        LeaveCriticalSection(&gs.sDiscInfoLock);

        return;
    }

    gs.state.bAudio = CDGetAudio(gs.wDeviceID);
	if (!gs.state.bAudio) {
        DebugPrintf("Media not audio!");

        if (!(gs.nOptions & OPTIONS_NOINSERTNOTIFICATION))
            CDClose(&gs.wDeviceID);

        LeaveCriticalSection(&gs.sDiscInfoLock);

        return;
    }
               
    if (gs.nOptions & OPTIONS_STOPONSTART)
		CDStop(gs.wDeviceID);

    FreeDiscInfo(psDI);
    InitDiscInfo(psDI);
	DBGetDiscID(gs.wDeviceID, psDI);

    InitDiscInfo(&sQueryDI);
    strcpy(sQueryDI.zCDDBID, psDI->zCDDBID);
    strcpy(sQueryDI.zMCIID, psDI->zMCIID);
    sQueryDI.nMCITracks = psDI->nMCITracks;
    sQueryDI.pnFrames = new unsigned int[psDI->nMCITracks];
    CopyMemory(sQueryDI.pnFrames, psDI->pnFrames, psDI->nMCITracks*sizeof(unsigned int));
    sQueryDI.nDiscLength = psDI->nDiscLength;
   
    DBGetDiscInfoLocal(gs.wDeviceID, &sQueryDI);
    if (sQueryDI.bDiscFound) {
        FreeDiscInfo(psDI);
        *psDI = sQueryDI;
    }

    ValidateDiscInfo(gs.wDeviceID, psDI);
    GetDiscInfo(gs.wDeviceID, psDI);
    ResetPlaylist(psDI, TRUE);

    CheckProgrammed(gs.wDeviceID, psDI);

    gs.state.bInit = TRUE;

	if (!psDI->bDiscFound && (gs.nOptions & OPTIONS_QUERYREMOTE))
		CloseHandle(CreateThread(NULL, 0, QueryThread, psDI, 0, &dwTid));
	else if (!psDI->bDiscFound && (gs.nOptions & OPTIONS_AUTOADDQUEUE)) {
        DebugPrintf("Auto Queue");

        AddToQueue(psDI);
	}

    if (psDI->bDiscFound)
        RunExternalCommand(psDI);

    LeaveCriticalSection(&gs.sDiscInfoLock);

DebugPrintf("DiscInit() finished");
}


