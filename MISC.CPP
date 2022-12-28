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
#include <shellapi.h>
#include <process.h>
#include <winsock.h>

#include "resource.h"

#include "ntfy_cd.h"
#define SOCK
#include "misc.h"

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
		strncpy(tnd.szTip, pszTip, sizeof(tnd.szTip));
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

void DebugPrintf(const char* pzFormat, ...)
{
    char szBuffer[1024];
    va_list ap;

    // This will make the vsprint code only to run uf bLogfile is set if we are a release build
#if !defined _DEBUG && !defined SPECIALDEBUG
    if (bLogfile) {
#endif
        va_start(ap, pzFormat);
        _vsnprintf(szBuffer, 1000, pzFormat, ap);
#if !defined _DEBUG && !defined SPECIALDEBUG
    }
#endif

	if (bLogfile) {
        FILE* fp;
		time_t t;
		struct tm* psTM;

		time(&t);
		psTM = localtime(&t);

        fp = fopen("C:\\NTFY_CD.LOG", "a");
        if (fp) {
            fprintf(fp, "%04X-%02d%02d%02d: %s\n", GetCurrentProcessId(), psTM->tm_hour, psTM->tm_min, psTM->tm_sec, szBuffer);

            fclose(fp);
        }
    }

#if defined _DEBUG || defined SPECIALDEBUG
    strcat(szBuffer, "\n\r");
    OutputDebugString("NTFY_CD: ");
    OutputDebugString(szBuffer);
#endif
    // This will make the va_end code only to run uf bLogfile is set if we are a release build
#if !defined _DEBUG && !defined SPECIALDEBUG
    if (bLogfile)
#endif
        va_end(ap);
}


void FixNewLine(BOOL bToNewLine, char** ppzStr)
{
	char* pzTmp = NULL;
	char* pzPtr = *ppzStr;
	char* pzLastStart = *ppzStr;

	while(*pzPtr) {
		if (!bToNewLine && *pzPtr == '\\' && *(pzPtr+1) == 'n') {
			AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			AppendString(&pzTmp, "\r\n", -1);
			pzPtr += 2;
			pzLastStart = pzPtr;
		}
		else if (bToNewLine && *pzPtr == '\r' && *(pzPtr + 1) == '\n') {
			AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
			AppendString(&pzTmp, "\\", -1);
			AppendString(&pzTmp, "n", -1);
			pzPtr += 2;
			pzLastStart = pzPtr;
		}
		else
			pzPtr ++;
	}

	if (pzLastStart != pzPtr)
		AppendString(&pzTmp, pzLastStart, pzPtr - pzLastStart);
	
	if (pzTmp) {
		free(*ppzStr);

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


void CheckAmpersand(char* pzStr)
{
	unsigned int nLoop;
	unsigned int nLoop2;

	for (nLoop = 0 ; nLoop < strlen(pzStr) ; nLoop ++) {
		if (pzStr[nLoop] == '&') {
			for (nLoop2 = strlen(pzStr) + 1 ; nLoop2 > nLoop ; nLoop2 --)
				pzStr[nLoop2] = pzStr[nLoop2-1];
			nLoop ++;
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


void AppendString(
    char** ppzPtr,
    const char* pzStr,
	int nLen)
{
	int nStartLen = 0;

	if (nLen == -1)
		nLen = strlen(pzStr);

    if (!*ppzPtr) {
		*ppzPtr = (char*) malloc(nLen + 1);
        memcpy(*ppzPtr, pzStr, nLen);
	}
    else {
		nStartLen = strlen(*ppzPtr);
        *ppzPtr = (char*)realloc(*ppzPtr, nStartLen + nLen + 1);
        memcpy(*ppzPtr+nStartLen, pzStr, nLen);
    }

	*(*ppzPtr + nStartLen + nLen) = 0;
}

void DoHelp(HWND hWnd, LPHELPINFO lpHelp)
{
    lpHelp = lpHelp;
    hWnd = hWnd;
    //    WinHelp(hWnd, "NTFY_CD.HLP", HELP_CONTEXT, lpHelp->dwContextId);
}


void UpdateTrackCombo(HWND hWnd, int nID)
{
    char szTmp[300];

    SendDlgItemMessage(hWnd, nID, CB_RESETCONTENT, 0, 0);

    for (int nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++) {
        sprintf(szTmp, "%d. %s", pnProgrammedTracks[nLoop]+1, ppzTracks[pnProgrammedTracks[nLoop]]);

        SendDlgItemMessage(hWnd, nID, CB_ADDSTRING, 0, (LPARAM) szTmp);
    }

}


void EncodeBase64(char* pzDest, ULONG nLen, const char* pxData)
{
    const char azTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int nDataLength = 0;
    unsigned int nLoop;
    unsigned int nInLoop;  
    unsigned int nOutLoop;
    unsigned int nLineLength;
    unsigned int nValue;
    unsigned int nTmp;
    unsigned int nCharCount = 0;
    unsigned int nEqualCount = 0;
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
	int nTimeout = nRemoteTimeout;

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
        MessageBox(GetForegroundWindow(), "Remote connection timed out", APPNAME, MB_OK);

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
		send(s, "\n", 1, 0) == SOCKET_ERROR) {
        DebugPrintf("send() failed %d", WSAGetLastError());

        closesocket(s);
    
        shutdown(s, 0);

        return SOCKET_ERROR;
    }

    return 1;
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
                    GetWindowText(GetDlgItem(hWnd, IDC_PASSWORD), zProxyPassword, 256);

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

    if ((nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD) && zProxyPassword[0] == 0) {
        if (DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_ASKFORPASSWORD), GetForegroundWindow(), (DLGPROC)AskForPasswordDlgProc) != IDOK)
            return SOCKET_ERROR;
    }

    sprintf(zStr, "%s:%s", zProxyUser, zProxyPassword);

    ZeroMemory(zEncoded, sizeof(zEncoded));
    EncodeBase64(zEncoded, strlen(zStr), zStr);

    sprintf(zStr, "Proxy-Authorization: Basic %s", zEncoded);

    return SendString(s, zStr);
}


BOOL CheckForNewVersion()
{
	WSADATA sData;
    struct sockaddr_in sAddrServer;
    struct hostent* psHostEntry;
    char zStr[8192];
    char zTmp[256];
	char zURL[1024];
	char zVersionNumber[80];
	char zVersionString[80];
    SOCKET s;

	if (MessageBox(NULL, "This check requires HTTP access or a proxy configured in the CDDB Remote tab. Do you want to continue?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDNO)
		return TRUE;

	DebugPrintf("CheckForNewVersion...");

	if (WSAStartup(0x0101, &sData)) {
		MessageBox(NULL, "Failed to initialize winsock!", APPNAME, MB_OK);

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

        return FALSE;
    }

	// Use the CDDB proxy
	if (nCDDBOptions & OPTIONS_CDDB_USEPROXY) {
		sAddrServer.sin_port = htons((short)nRemoteProxyPort);

		psHostEntry = gethostbyname(zRemoteProxyServer);
        if (!psHostEntry) {
    		MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK);

		    DebugPrintf("gethostbyname() failed to get address for %s. Error code: %d", zRemoteProxyServer, WSAGetLastError());

		    shutdown(s, 0);

		    return FALSE;
        }

		DebugPrintf("Check using HTTP proxy on port %d", nRemoteProxyPort);
	}
	else {
		sAddrServer.sin_port = htons((short)80);

		psHostEntry = gethostbyname(VERSION_SERVER);
        if (!psHostEntry) {
    		MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK);

		    DebugPrintf("gethostbyname() failed to get address for %s. Error code: %d", VERSION_SERVER, WSAGetLastError());

		    shutdown(s, 0);

		    return FALSE;
        }

		DebugPrintf("Check using HTTP");
	}

	// Set up remote address

	sAddrServer.sin_family = AF_INET;

    memcpy(&sAddrServer.sin_addr.s_addr, psHostEntry->h_addr_list[0], sizeof(sAddrServer.sin_addr.s_addr));

	if (connect(s, (struct sockaddr*)&sAddrServer, sizeof(sAddrServer))) {
		MessageBox(NULL, "Failed to connect to remote server!", APPNAME, MB_OK);

		DebugPrintf("connect() failed %d", WSAGetLastError());

		shutdown(s, 0);

		return FALSE;
	}
    
	if (nCDDBOptions & OPTIONS_CDDB_USEPROXY)
		DebugPrintf("Connected to proxy %s", zRemoteProxyServer);
	else
		DebugPrintf("Connected to %s", VERSION_SERVER);

    // Build query
	if (nCDDBOptions & OPTIONS_CDDB_USEPROXY) 
        sprintf(zStr, "GET http://%s%s", VERSION_SERVER, VERSION_PATH);
    else
        sprintf(zStr, "GET %s", VERSION_PATH);
    strcat(zStr, " HTTP/1.0");

	// Send query
	if (SendString(s, zStr) == SOCKET_ERROR)
    	return FALSE;
    if (nCDDBOptions & OPTIONS_CDDB_USEAUTHENTICATION) {
        if (SendAuthentication(s) == SOCKET_ERROR)
            return FALSE;
    }
	if (SendString(s, "\r") == SOCKET_ERROR)
    	return FALSE;
	if (SendString(s, "\r") == SOCKET_ERROR)
    	return FALSE;
	//
	// Get query result
	//

	// Get result
	if (!GetString(s, zStr, 1024))
		return FALSE;
	if (!strncmp(zStr, "HTTP/", 5) && !strncmp(&zStr[9], "200", 3)) {
		do {
			if (!GetString(s, zStr, 1024))
				return FALSE;
		} while(zStr[0] != 0 && zStr[0] != '\r' && zStr[0] != '\n');

		if (!GetString(s, zVersionNumber, 80))
			return FALSE;
		if (!GetString(s, zVersionString, 80))
			return FALSE;
		if (!GetString(s, zURL, 1024))
			return FALSE;
    }
    else {
		if (strlen(zStr) < 100)
			sprintf(zTmp, "Remote server reported: %s", zStr);
		else
			strcpy(zTmp, "Remote server reported an error");

		MessageBox(NULL, zTmp, APPNAME, MB_OK);

		shutdown(s, 0);

		return FALSE;
    }

	WSACleanup();

	if (atoi(zVersionNumber) > VERSION_VERSION) {
		DebugPrintf("Newer version exists on server");

		sprintf(zTmp, "A newer version (%s) exists. Do you want to go to the download page?", zVersionString);

		if (MessageBox(NULL, zTmp, APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
			DebugPrintf("Downloading newer version");

			if (ShellExecute(NULL, "open", zURL, NULL, ".", SW_SHOWNORMAL) <= (HINSTANCE) 32) {
				sprintf(zTmp, "Failed to open URL %s", zURL);

				MessageBox(NULL, zTmp, APPNAME, MB_OK);

				return FALSE;
			}
		}
	}
	else {
		DebugPrintf("You seem to run the latest version!");

		MessageBox(NULL, "You seem to run the latest version!", APPNAME, MB_OK);
	}

	return TRUE;
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

    hProgressWnd = CreateDialog(hMainInstance, MAKEINTRESOURCE(IDD_PROGRESS), sProgressInfo.hParent, (DLGPROC)ProgressDlgProc);

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
