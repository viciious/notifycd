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

#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <winsock.h>
#include <commctrl.h>

#include "resource.h"

#include "clist.h"
#include "ntfy_cd.h"
#include "mci.h"
#define SOCK
#include "misc.h"
#include "cddb.h"

#define MAX_READ    204800

#define SOCKET_DEBUG        0
#define PROXY_DEBUG         0

BOOL OpenEntry(const char* pzPath);

char azCategories[][32] = {"blues",
                           "classical",
                           "country",
                           "data",
                           "folk",
                           "jazz",
                           "misc",
                           "newage",
                           "reggae",
                           "rock",
                           "soundtrack"};


CDDB_DISC_ENTRY sEntry;

// DB Editor stuff
BOOL bDBInEditor = FALSE;
WIN32_FIND_DATA sDBFindData;
int nDBCategory;
HANDLE hDBFind;
CDDB_DISC_ENTRY* psDBSavedEntry;
char zDBSavedID[10];

char zLastPathRead[MAX_PATH];
HANDLE hReadFile;
DWORD dwSizeOfFile;
char* pzReadData;
char* pzCurrData;
char* pzGetNextIDCurrData;
char zDBLastPath[MAX_PATH];
int* pnQueryFrames = NULL;
int nQueryDiscLength;

int nCDDBNumCategories = 11;
int nNewTrackNum;
BOOL bDiscFound = FALSE;
BOOL bInRemoteQuery = FALSE;
WORD wCurrDeviceID;

HICON hCDDBTrayIcon;
HICON hIconLocal;
HICON hIconRemote;

char zID[10];
char zSendID[10];

char* pzMOTD;

struct sRemoteEntry {
    char zCategory[32];
    char zDiscID[10];
    char zTitle[256];
};

cList<sRemoteEntry>* poRemoteList;
sRemoteEntry* psRemoteChoise = NULL;

void GetTmpFile(char* pzFile)
{
	FILE* fp;

	tmpnam(pzFile);

	fp = fopen(pzFile, "w");
	if (!fp)
		strcpy(pzFile, "C:\\NOTIFY.TMP");
    else {
		fclose(fp);

        DeleteFile(pzFile);
    }
}


/////////////////////////////////////////////////////////////////////
//
// MESSAGE OF THE DAY
//
/////////////////////////////////////////////////////////////////////

BOOL CALLBACK MOTDDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM/*  lParam*/)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            SetWindowText(GetDlgItem(hWnd, IDC_EDIT), pzMOTD);
        }
		break;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK: {
                    if (SendDlgItemMessage(hWnd, IDC_NOMOTD, BM_GETCHECK, 0, 0))
                        WritePrivateProfileInt("CDDB", "No_MOTD", 1, "CDPLAYER.INI");

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


/////////////////////////////////////////////////////////////////////
//
// CHOOSE DISC DIALOG
//
/////////////////////////////////////////////////////////////////////

BOOL CALLBACK ChooseDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM/*  lParam*/)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            char zTmp[300];
            int anTabs[2] = {40, 50};
            int nWidth = 0;
            HDC hDC;
            SIZE sSize;

            CenterWindow(hWnd, TRUE);

            hDC = GetDC(GetDlgItem(hWnd, IDC_LIST));

            SendDlgItemMessage(hWnd, IDC_LIST, LB_SETTABSTOPS, 1, (LPARAM) anTabs);

            psRemoteChoise = poRemoteList->First();
            while(psRemoteChoise) {
                sprintf(zTmp, "%s\t%s", psRemoteChoise->zCategory, psRemoteChoise->zTitle);

                SendDlgItemMessage(hWnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM) zTmp);

                GetTextExtentPoint32(hDC, zTmp, strlen(zTmp), &sSize);

                nWidth = max(nWidth, sSize.cx);

                psRemoteChoise = poRemoteList->Next();
            }

            ReleaseDC(GetDlgItem(hWnd, IDC_LIST), hDC);

            SendDlgItemMessage(hWnd, IDC_LIST, LB_SETCURSEL, 0, 0);
            SendDlgItemMessage(hWnd, IDC_LIST, LB_SETHORIZONTALEXTENT, nWidth, 0);
        }
		break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == LBN_DBLCLK) {
                switch(LOWORD(wParam)) {
				    case IDC_LIST: {
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0L);
                    }
                    break;
                }
            }
            else {
                switch(LOWORD(wParam)) {
                    case IDOK: {
                        int nIndex = SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0);

                        psRemoteChoise = poRemoteList->First();
                        while(nIndex-- && psRemoteChoise)
                            psRemoteChoise = poRemoteList->Next();

                        EndDialog(hWnd, IDOK);
                    }
                    break;

                    case IDCANCEL: {
                        EndDialog(hWnd, IDCANCEL);
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
// CDDB STUFF!
//
/////////////////////////////////////////////////////////////////////

BOOL bParseFound;

void FreeEntry()
{
    int nLoop;

    DebugPrintf("-> FreeEntry");

    // Free old stuff

    if (sEntry.pzArtist)
        free(sEntry.pzArtist);
    if (sEntry.pzTitle)
        free(sEntry.pzTitle);
    if (sEntry.pzPlayorder) 
        free(sEntry.pzPlayorder);
    if (sEntry.pzDiscid) 
        free(sEntry.pzDiscid);
    if (sEntry.pzExtD) 
        free(sEntry.pzExtD);
    if (sEntry.pnFrames) 
        free(sEntry.pnFrames);
    if (sEntry.ppzTracks) {
        for(nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
            if (sEntry.ppzTracks[nLoop])
                free(sEntry.ppzTracks[nLoop]);
        }

        free(sEntry.ppzTracks);
        sEntry.ppzTracks = NULL;
    }
    if (sEntry.ppzTracksExt) {
        for(nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
            if (sEntry.ppzTracksExt[nLoop])
                free(sEntry.ppzTracksExt[nLoop]);
        }

        free(sEntry.ppzTracksExt);
        sEntry.ppzTracksExt = NULL;
    }

    memset(&sEntry, 0, sizeof(sEntry));

    DebugPrintf("<- FreeEntry");
}


void FixEntry()
{
    char* pzPtr;

    pzPtr = strstr(sEntry.pzArtist, "/");
    if (pzPtr) {
        pzPtr[0] = 0;
        while(sEntry.pzArtist[strlen(sEntry.pzArtist)-1] == ' ')
            sEntry.pzArtist[strlen(sEntry.pzArtist)-1] = 0;
        pzPtr ++;
        while(*pzPtr == ' ')
            pzPtr ++;
        AppendString(&sEntry.pzTitle, pzPtr, -1);
        while(sEntry.pzTitle[strlen(sEntry.pzTitle)-1] == ' ')
            sEntry.pzTitle[strlen(sEntry.pzTitle)-1] = 0;
    }
	else
		AppendString(&sEntry.pzTitle, "", -1);

    if (sEntry.pzPlayorder) {
        unsigned int nLoop;

        for(nLoop = 0 ; nLoop < strlen(sEntry.pzPlayorder) ; nLoop ++) {
            if (sEntry.pzPlayorder[nLoop] == ',')
                sEntry.pzPlayorder[nLoop] = ' ';
        }
    }
}

void CheckNumTracks(int nNum)
{
	if (nNum == sEntry.nTracks) {
		sEntry.nTracks ++;

		if (!sEntry.ppzTracks) 
			sEntry.ppzTracks = (char**)malloc(sEntry.nTracks*sizeof(char*));
		else
			sEntry.ppzTracks = (char**)realloc(sEntry.ppzTracks, sEntry.nTracks*sizeof(char*));

		if (!sEntry.ppzTracksExt)
			sEntry.ppzTracksExt = (char**)malloc(sEntry.nTracks*sizeof(char*));
		else
			sEntry.ppzTracksExt = (char**)realloc(sEntry.ppzTracksExt, sEntry.nTracks*sizeof(char*));

		sEntry.ppzTracks[nNum] = NULL;
		sEntry.ppzTracksExt[nNum] = NULL;
	}
}


char* GetDataString(char* pzData, char** ppzStr, DWORD dwSize) 
{
    *ppzStr = NULL;

    char* pzPtr = strchr(pzData, '\n');
    if (!pzPtr) {
		if (dwSize)
			pzPtr = pzData + dwSize;
		else
			return NULL;
	}

    *ppzStr = (char*) malloc(((pzPtr - pzData) + 1) * sizeof(char));
    if (!*ppzStr) {
        DebugPrintf("GetDataString: Out of memory");

        return NULL;
    }

    strncpy(*ppzStr, pzData, pzPtr - pzData);
    if ((*ppzStr)[pzPtr - pzData - 1] == '\r')
        (*ppzStr)[pzPtr - pzData - 1] = 0;

    (*ppzStr)[pzPtr - pzData] = 0;

    return pzPtr + 1;
}


void ParseEntry(char* pzData, DWORD dwSize) 
{
    char* pzPtr;
    char* pzCurrPtr;
    char* pzLastPtr;
    char* pzStr;
    BOOL bDiscIdFound = FALSE;
    BOOL bDTITLEFound = FALSE;
    BOOL bInCollectTrackFrames = FALSE;
    int nCollectedTrackFrames = 0;

    pzLastPtr = pzCurrPtr = pzData;

    while ((unsigned)(pzCurrPtr - pzData) < dwSize) {
        pzLastPtr = pzCurrPtr;
        pzStr = NULL;
        pzCurrPtr = GetDataString(pzCurrPtr, &pzStr, pzData + dwSize - pzCurrPtr);

        if (!pzCurrPtr)
			return;

        if (!strncmp(pzStr, "DISCID=", 7)) {
            DebugPrintf("Found new discid: %s", &pzStr[7]);

            if (sEntry.pzDiscid) 
                AppendString(&sEntry.pzDiscid, ",", -1);

            AppendString(&sEntry.pzDiscid, &pzStr[7], -1);

            strlwr(sEntry.pzDiscid);
            strlwr(zID);

            // If we are doing a remote query we accept everything as discid since the server might have
            // decided that this is a "close" match
            if (strstr(sEntry.pzDiscid, zID) || bInRemoteQuery)
                bParseFound = TRUE;

            bDiscIdFound = TRUE;
        }
        else if (!bInDBDLGBuildList && !strncmp(pzStr, "# Revision: ", 12))
            sEntry.nRevision = atoi(&pzStr[12]);
        else if (!bInDBDLGBuildList && !strncmp(pzStr, "# Submitted via: ", 12))
            strcpy(sEntry.zSubmitted, &pzStr[17]);
        else if (!bInDBDLGBuildList && !strncmp(pzStr, "# Track frame offsets:", 22))
            bInCollectTrackFrames = TRUE;
        else if (!bInDBDLGBuildList && !strncmp(pzStr, "# Disc length: ", 15)) {
            bInCollectTrackFrames = FALSE;

            sEntry.nDiscLength = atoi(&pzStr[15]);
        }
        else if (bDiscIdFound && !bParseFound)
            return;
        else if (bInCollectTrackFrames) {
            int nStrPos = 1;

            while(pzStr[nStrPos] == ' ' || pzStr[nStrPos] == '\t')
                nStrPos ++;

            if (isdigit(pzStr[nStrPos])) {
                nCollectedTrackFrames ++;
            
                if (sEntry.pnFrames)
                    sEntry.pnFrames = (int*) realloc(sEntry.pnFrames, nCollectedTrackFrames*sizeof(int));
                else
                    sEntry.pnFrames = (int*) malloc(nCollectedTrackFrames*sizeof(int));

                sEntry.pnFrames[nCollectedTrackFrames-1] = atoi(&pzStr[nStrPos]);
            }
            else
                bInCollectTrackFrames = FALSE;
        }

        if (bParseFound) {
            if (!strncmp(pzStr, "DTITLE=", 7)) {
                AppendString(&sEntry.pzArtist, &pzStr[7], -1);
                bDTITLEFound = TRUE;
            }
            else {
                if (bDTITLEFound && bInDBDLGBuildList)
                    return;

                if (!strncmp(pzStr, "PLAYORDER=", 10))
                    AppendString(&sEntry.pzPlayorder, &pzStr[10], -1);
                else if (!strncmp(pzStr, "TTITLE", 6)) {
                    int nNum;

                    pzPtr = strchr(&pzStr[6], '=');
                    pzPtr[0] = 0;
                    nNum = atoi(&pzStr[6]);

					CheckNumTracks(nNum);

			        AppendString(&sEntry.ppzTracks[nNum], &pzPtr[1], -1);
                }
                else if (!strncmp(pzStr, "EXTD=", 5))
                    AppendString(&sEntry.pzExtD, &pzStr[5], -1);
                else if (!strncmp(pzStr, "EXTT", 4)) {
                    int nNum;

                    pzPtr = strchr(&pzStr[4], '=');
                    pzPtr[0] = 0;
                    nNum = atoi(&pzStr[4]);

			        CheckNumTracks(nNum);

                    AppendString(&sEntry.ppzTracksExt[nNum], &pzPtr[1], -1);
                }
            }
        }

        if (pzStr)
            free(pzStr);
    }

    return;
}


char* ParseData(char* pzData, DWORD dwSize)
{
    char* pzPtrStart;
    char* pzPtrEnd;
    char* pzCurrPtr;

    // First we need to find the entry in the file
    // This is done by searching for the id

    pzCurrPtr = pzData;
    do {
        pzPtrStart = strstr(pzCurrPtr, "# xmcd");
        if (pzPtrStart) {
            DebugPrintf("Found new entry in file");

			pzPtrEnd = strstr(pzPtrStart + 6, "# xmcd");

            if (!pzPtrEnd || pzPtrEnd > pzData + dwSize) {
				DebugPrintf("Last in file!");
                pzPtrEnd = pzData + dwSize;
			}

			ParseEntry(pzPtrStart, pzPtrEnd - pzPtrStart);

            pzCurrPtr = pzPtrEnd;
        }
    } while(!bParseFound && (unsigned)(pzCurrPtr - pzData) < dwSize && pzCurrPtr);

    return pzCurrPtr;
}


void CloseEntry()
{
    // Free the old data
	if (pzReadData) {
		free(pzReadData);

		pzReadData = NULL;

		DebugPrintf("Deleted cache of %s", zLastPathRead);

		zLastPathRead[0] = 0;
	}

    if (hReadFile != INVALID_HANDLE_VALUE && hReadFile != NULL) {
        if (!CloseHandle(hReadFile)) {
            DebugPrintf("CloseHandle(hReadFile) failed for %s (error code %d)", zLastPathRead, GetLastError());

            return;
        }
    }

    hReadFile = INVALID_HANDLE_VALUE;
}


BOOL OpenEntry(const char* pzPath)
{
	if (stricmp(pzPath, zLastPathRead)) {
		HANDLE hTmpFile;
		DWORD nRead;

        DebugPrintf("Trying to open %s for caching", pzPath);
        
        // First try to open the file. It might not exist.
		// If it doesn't exist, keep the old map in memory
    
		hTmpFile = CreateFile(pzPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hTmpFile == INVALID_HANDLE_VALUE) {
            DebugPrintf("... not found");
			return FALSE;
        }

        CloseEntry();

		hReadFile = hTmpFile;

		// Make new cache!
		dwSizeOfFile  = GetFileSize(hReadFile, NULL);
        if (!dwSizeOfFile ) {
            DebugPrintf("Filesize is 0, closing entry!");

            CloseEntry();

            return FALSE;
        }

		pzReadData = (char*) malloc(dwSizeOfFile  + 10);
		if (!pzReadData) {
			DebugPrintf("malloc failed (error code %d)", GetLastError());

			return FALSE;
		}

		nRead = 0;
		if (!ReadFile(hReadFile, pzReadData, dwSizeOfFile, &nRead, NULL) || nRead != dwSizeOfFile) {
			DebugPrintf("ReadFile failed (error code %d)", GetLastError());

			return FALSE;
		}
		
		pzReadData[dwSizeOfFile] = 0;
		pzReadData[dwSizeOfFile+1] = 0;

		strcpy(zLastPathRead, pzPath);

		pzCurrData = pzReadData;

		DebugPrintf("Created cache for %s (%d)", pzPath, dwSizeOfFile);
	}

	return TRUE;
}


BOOL ReadEntry(
    const char* pzPath,
    const char* pzCategory)
{
    FreeEntry();

    bParseFound = FALSE;
                              
	if (!OpenEntry(pzPath))
		return FALSE;

    strcpy(sEntry.zCategory, pzCategory);

	// Frist try from last position if this isn't the start position
	if (pzCurrData != pzReadData)
		pzCurrData = ParseData(pzCurrData, dwSizeOfFile  - (unsigned)(pzCurrData - pzReadData));

	if (!bParseFound)
		pzCurrData = ParseData(pzReadData, dwSizeOfFile );

    if (!bParseFound) {
        if (sEntry.pzDiscid) {
            free(sEntry.pzDiscid);

            sEntry.pzDiscid = NULL;
        }

        return FALSE;
	}
    else {
        FixEntry();

        return TRUE;
    }
}


BOOL QueryLocal()
{
    char zPath[256];
    BOOL bFound = FALSE;

DebugPrintf("Query local");

    if (nCDDBType == 2) {   // Unix
        int nLoop;

        for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++) {
            sprintf(zPath, "%s%s\\%s", zCDDBPath, ppzCategories[nLoop], zID);
            if (ReadEntry(zPath, ppzCategories[nLoop]))
                return TRUE;
        }
    }   
    else if (nCDDBType == 1) {     // Windows
        int nLoop;
        char zIDStart[3];
        int nIDStart;
        
        strncpy(zIDStart, zID, 2);
        zIDStart[2] = 0;

        sscanf(zIDStart, "%X", &nIDStart);

        for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++) {
            WIN32_FIND_DATA sFF;
            HANDLE hFind;
            char zCategory[20];
            char zFileIDStart[3];
            char zFileIDEnd[3];
            int nFileIDStart;
            int nFileIDEnd;

            // Loop through all files in all dirs to find the correct file to search!

            strcpy(zCategory, ppzCategories[nLoop]);
            zCategory[8] = 0;

            sprintf(zPath, "%s%s\\*.*", zCDDBPath, zCategory);

            DebugPrintf("Scanning %s", zPath);

            hFind = FindFirstFile(zPath, &sFF);
            while(hFind && 
                  hFind != INVALID_HANDLE_VALUE && 
                  !bFound) {
                if (!(sFF.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strlen(sFF.cFileName) == 6) {
                    strncpy(zFileIDStart, &sFF.cFileName[strlen(sFF.cFileName) - 6], 2);
                    strncpy(zFileIDEnd, &sFF.cFileName[strlen(sFF.cFileName) - 2], 2);
                    zFileIDStart[2] = zFileIDEnd[2] = 0;

                    sscanf(zFileIDStart, "%X", &nFileIDStart);
                    sscanf(zFileIDEnd, "%X", &nFileIDEnd);

                    if (nIDStart >= nFileIDStart && nIDStart <= nFileIDEnd) {
                        sprintf(zPath, "%s%s\\%s", zCDDBPath, zCategory, sFF.cFileName);

                        DebugPrintf("Found file %s", zPath);

                        bFound = ReadEntry(zPath, ppzCategories[nLoop]);

                        FindClose(hFind);

                        hFind = NULL;                       
                    }
                }
                
                if (hFind != NULL && !FindNextFile(hFind, &sFF)) {
                    FindClose(hFind);

                    hFind = NULL;
                }
            }
            
            if (bFound)
                return TRUE;
        }
    }

    return FALSE;
}


void DoQP(const char* pzSrc, char** ppzDest) 
{
    char* pzDest = *ppzDest = (char*) malloc(80*sizeof(char));
    int nLastSoftbreak = 0;
    int nSize = 80;
    int nPos = 0;
    char cHigh, cLow;

    while(*pzSrc) {
        if ((*pzSrc >= 33 && *pzSrc <= 60) || (*pzSrc >= 62 && *pzSrc <= 126) || *pzSrc == 32 || *pzSrc == 9)
            pzDest[nPos++] = *pzSrc;
        else {
            cHigh = (char) ((*pzSrc & 0xF0) >> 4);
            cLow = (char) (*pzSrc & 0x0F);

            pzDest[nPos++] = '=';
            if (cHigh > 9)
                pzDest[nPos++] = (char) ('A' + cHigh - 10);
            else
                pzDest[nPos++] = (char) ('0' + cHigh);

            if (cLow > 9)
                pzDest[nPos++] = (char) ('A' + cLow - 10);
            else
                pzDest[nPos++] = (char) ('0' + cLow);
        }

        if (nPos - nLastSoftbreak > 70) {
            nLastSoftbreak = nPos;

            pzDest[nPos++] = '=';
            pzDest[nPos++] = '\r';
            pzDest[nPos++] = '\n';
        }

        if (nPos > nSize - 10) {
            nSize += 80;
            pzDest = *ppzDest = (char*) realloc(*ppzDest, nSize*sizeof(char));
        }

        pzSrc ++;
    }

    pzDest[nPos] = 0;
}


int SendStringQuotedPrintable(SOCKET s, char* pzStr) 
{
    char* pzQPStr = NULL;

    if (strlen(pzStr) < 800)
        DebugPrintf("Sending(QP): %s", pzStr);
    else
        DebugPrintf("Sending(QP): Overflow");

    DoQP(pzStr, &pzQPStr);

//    DebugPrintf("...sending QP text %s", pzQPStr);

    if (send(s, pzQPStr, strlen(pzQPStr), 0) == SOCKET_ERROR ||
		send(s, "\n", 1, 0) == SOCKET_ERROR) {
        DebugPrintf("send() failed %d", WSAGetLastError());

        closesocket(s);
    
        shutdown(s, 0);

        return SOCKET_ERROR;
    }

    free(pzQPStr);

    return 1;
}


void ConvertSpace(char* pzStr)
{
    while(*pzStr) {
        if (*pzStr == ' ')
            *pzStr = '+';

        pzStr ++;
    }
}

BOOL Reconnect(SOCKET* ps, struct sockaddr_in* psAddr)
{
    closesocket(*ps);
	shutdown(*ps, 0);

    *ps = socket(PF_INET, SOCK_STREAM, 0);
    if (*ps == INVALID_SOCKET) {
        DebugPrintf("socket() failed %d", WSAGetLastError());

        return FALSE;
    }

	if (connect(*ps, (struct sockaddr*)psAddr, sizeof(*psAddr))) {
		MessageBox(NULL, "Failed to connect to remote server!", APPNAME, MB_OK);

		DebugPrintf("connect() failed %d", WSAGetLastError());

		shutdown(*ps, 0);

		return FALSE;
	}

    return TRUE;
}


BOOL QueryRemote()
{
    SOCKET s;
    struct sockaddr_in sAddrServer;
    struct hostent* psHostEntry;
    char zStr[8192];
    char zQuery[8192];
    char zHello[100];
    char zTmp[256];
    char zTmpID[10];
    int nLoop;
    char zCategory[32];
    char* pzPtr;
    char* pzEntry = NULL;
    BOOL bEnd;
    BOOL bNoMOTD = FALSE;
    
    bParseFound = FALSE;

	DebugPrintf("Query remote");

    bNoMOTD = GetPrivateProfileInt("CDDB", "No_MOTD", 0, "CDPLAYER.INI");

#if PROXY_DEBUG    
    nCDDBOptions |= OPTIONS_CDDB_USEPROXY;
#endif

    //
    // Build different query strings!
    //

    // Build hello
    pzPtr = NULL;
	if (zEmailAddress) {
		strcpy(zTmp, zEmailAddress);
		pzPtr = strchr(zTmp, '@');
		if (pzPtr) {
			*pzPtr = ' ';

            sprintf(zHello, "%s NotifyCDPlayer(CDDB) %s", zTmp, VERSION);
		}
	}
	if (!pzPtr)
		sprintf(zHello, "username hostname NotifyCDPlayer(CDDB) %s", VERSION);

	// Build query
	strcpy(zTmp, zID);
	sprintf(zQuery, "cddb query %s %d ", zTmp, nNewTrackNum);
	for (nLoop = 0 ; nLoop < nNewTrackNum ; nLoop ++) {
		sprintf(zTmp, "%d ", pnQueryFrames[nLoop]);
		strcat(zQuery, zTmp);
	}
	sprintf(zTmp, "%d", nQueryDiscLength);
	strcat(zQuery, zTmp);

	if (nCDDBOptions & OPTIONS_CDDB_USEHTTP) {
        ConvertSpace(zQuery);
	    ConvertSpace(zHello);
    }

    // Create socket   
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        DebugPrintf("socket() failed %d", WSAGetLastError());

        return FALSE;
    }

    NotifyModify(hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Connecting...");

    /////////////////////////////////////////////////////////////
    //
    // Check message of the day!
    //
    /////////////////////////////////////////////////////////////

	if (!(nCDDBOptions & OPTIONS_CDDB_USEHTTP)) {
    	DebugPrintf("Query normal");

        // Set up remote address

	    sAddrServer.sin_family = AF_INET;
	    sAddrServer.sin_port = htons((short)nRemotePort);

	    psHostEntry = gethostbyname(zRemoteServer);
	    if (!psHostEntry) {
  		    MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK);

            DebugPrintf("gethostbyname() failed to get address for %s. Error code: %d", zRemoteServer, WSAGetLastError());

		    shutdown(s, 0);

		    return FALSE;
	    }

	    memcpy(&sAddrServer.sin_addr.s_addr, psHostEntry->h_addr_list[0], sizeof(sAddrServer.sin_addr.s_addr));

	    if (connect(s, (struct sockaddr*)&sAddrServer, sizeof(sAddrServer))) {
		    MessageBox(NULL, "Failed to connect to remote server!", APPNAME, MB_OK);

		    DebugPrintf("connect() failed %d", WSAGetLastError());

		    shutdown(s, 0);

		    return FALSE;
	    }
    
	    DebugPrintf("Connected to %s", zRemoteServer);

	    //
	    // Get server sign-on and send handshake!
	    //

	    Sleep(200);
	    
	    if (!GetString(s, zStr, 1024))
		    return FALSE;
	    // Check for valid sign-on status
	    if (strncmp(zStr, "200", 3) && strncmp(zStr, "201", 3)) {
		    DebugPrintf("Remote server denies clients for the moment");
		    MessageBox(NULL, "Remote server denies clients for the moment", APPNAME, MB_OK);

		    closesocket(s);

		    shutdown(s, 0);

		    return FALSE;
	    }

	    DebugPrintf("Connection ok!");

        sprintf(zStr, "cddb hello %s", zHello);
        // Send handshake
	    if (SendString(s, zStr) == SOCKET_ERROR)
		    return FALSE;

        // Get handshake result    
	    if (!GetString(s, zStr, 1024))
		    return FALSE;
	    // Check for valid handshake status
	    if (strncmp(zStr, "200", 3)) {
		    DebugPrintf("Remote server denies clients for the moment");
		    MessageBox(NULL, "Remote server denies clients for the moment", APPNAME, MB_OK);

		    closesocket(s);

		    shutdown(s, 0);

		    return FALSE;
	    }

        if (!bNoMOTD) {
            strcpy(zStr, "motd");
            // Send motd
	        if (SendString(s, zStr) == SOCKET_ERROR)
    	        return FALSE;
        }
    }
    else {
		if (nCDDBOptions & OPTIONS_CDDB_USEPROXY) {
			sAddrServer.sin_port = htons((short)nRemoteProxyPort);

			psHostEntry = gethostbyname(zRemoteProxyServer);
            if (!psHostEntry) {
    		    MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK);

		        DebugPrintf("gethostbyname() failed to get address for %s. Error code: %d", zRemoteProxyServer, WSAGetLastError());

		        shutdown(s, 0);

		        return FALSE;
            }

			DebugPrintf("Query using HTTP proxy on port %d", nRemoteProxyPort);
		}
		else {
			sAddrServer.sin_port = htons((short)80);

			psHostEntry = gethostbyname(zRemoteServer);
            if (!psHostEntry) {
    		    MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK);

		        DebugPrintf("gethostbyname() failed to get address for %s. Error code: %d", zRemoteServer, WSAGetLastError());

		        shutdown(s, 0);

		        return FALSE;
            }

			DebugPrintf("Query using HTTP");
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
			DebugPrintf("Connected to %s", zRemoteServer);

        if (!bNoMOTD) {
            // Build query
            strcpy(zTmp, zID);
		    if (nCDDBOptions & OPTIONS_CDDB_USEPROXY) 
                sprintf(zStr, "GET http://%s%s?cmd=motd&hello=%s&proto=1", zRemoteServer, zRemoteHTTPPath, zHello);
            else
                sprintf(zStr, "GET %s?cmd=motd&hello=%s&proto=1", zRemoteHTTPPath, zHello);
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
            }
            else {
				// Reset password if this went wrong. This forces us to ask for it again the next time in case
				// we entered the wrong password. If motd works, we have a valid password
				if (nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)
					zProxyPassword[0] = 0;

				if (strlen(zStr) < 100)
					sprintf(zTmp, "Remote server reported: %s", zStr);
				else
					strcpy(zTmp, "Remote server reported an error");

				MessageBox(NULL, zTmp, APPNAME, MB_OK);

			    shutdown(s, 0);

			    return FALSE;
            }
        }
    }

    if (!bNoMOTD) {
        //
	    // Get query result
	    //

	    // Get result
	    if (!GetString(s, zStr, 1024))
		    return FALSE;

        // Did we get a MOTD?
        if (!strncmp(zStr, "210", 3)) {
            char zLastMOTD[256];
            BOOL bDisplay = FALSE;

            pzMOTD = NULL;

		    while (zStr[strlen(zStr)-1] == '\n' || zStr[strlen(zStr)-1] == '\r')
			    zStr[strlen(zStr)-1] = 0;

            GetPrivateProfileString("CDDB", "LastMOTD", "", zLastMOTD, 255, "CDPLAYER.INI");
            if (strcmp(zStr, zLastMOTD)) {
                WritePrivateProfileString("CDDB", "LastMOTD", zStr, "CDPLAYER.INI");
                bDisplay = TRUE;
            }

            bEnd = FALSE;
		    while(!bEnd) {
			    if (!GetString(s, zStr, 8192))
				    return FALSE;

			    while (zStr[strlen(zStr)-1] == '\n' || zStr[strlen(zStr)-1] == '\r')
				    zStr[strlen(zStr)-1] = 0;

			    if (zStr[0] == '.')
				    bEnd = TRUE;
                else {
                    AppendString(&pzMOTD, zStr, -1);
                    AppendString(&pzMOTD, "\r\n", -1);
                }
		    }

            if (bDisplay)
                DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_MOTD), GetForegroundWindow(), (DLGPROC)MOTDDlgProc);

            free(pzMOTD);
        }
    }

    if (!(nCDDBOptions & OPTIONS_CDDB_USEHTTP)) {
	    //
	    // Send disc query and check results
	    //

	    NotifyModify(hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Querying...");

	    // Send query
	    if (SendString(s, zQuery) == SOCKET_ERROR)
		    return FALSE;
    }
    else {
        if (!bNoMOTD && !Reconnect(&s, &sAddrServer))
            return FALSE;

        NotifyModify(hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Querying...");

		// Build query
        strcpy(zTmp, zID);
		if (nCDDBOptions & OPTIONS_CDDB_USEPROXY) 
            sprintf(zStr, "GET http://%s%s?cmd=%s&hello=%s&proto=1", zRemoteServer, zRemoteHTTPPath, zQuery, zHello);
        else
            sprintf(zStr, "GET %s?cmd=%s&hello=%s&proto=1", zRemoteHTTPPath, zQuery, zHello);
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
    }

	//
	// Get query result
	//

	// Get result
	if (!GetString(s, zStr, 1024))
		return FALSE;
	// Check result 200 and 211 is supported
	if (strncmp(zStr, "200", 3) && strncmp(zStr, "202", 3) && strncmp(zStr, "211", 3)) {
		DebugPrintf("Server reported some unsupported error!", WSAGetLastError());

		SendString(s, "quit");

		closesocket(s);

		shutdown(s, 0);

		MessageBox(NULL, "Server error!", APPNAME, MB_OK);

		return FALSE;
	}
	// No match!
	if (!strncmp(zStr, "202", 3)) {
		DebugPrintf("No match on remote server!", WSAGetLastError());

		SendString(s, "quit");

		closesocket(s);

		shutdown(s, 0);

		return FALSE;
	}
	if (!strncmp(zStr, "211", 3)) {
		DebugPrintf("No exact match! Building list!", WSAGetLastError());

		poRemoteList = new cList<sRemoteEntry>;

		bEnd = FALSE;
		while(!bEnd) {
			if (!GetString(s, zStr, 8192))
				return FALSE;

			while(zStr[strlen(zStr)-1] == '\n' || zStr[strlen(zStr)-1] == '\r')
				zStr[strlen(zStr)-1] = 0;

			if (zStr[0] != '.') {
				psRemoteChoise = new sRemoteEntry;

				pzPtr = strchr(zStr, ' ');
				strncpy(psRemoteChoise->zCategory, zStr, (int)(pzPtr - &zStr[0]));
				psRemoteChoise->zCategory[(int)(pzPtr - &zStr[0])] = 0;

				pzPtr ++;

				strncpy(psRemoteChoise->zDiscID, pzPtr, 8);
				psRemoteChoise->zDiscID[8] = 0;
				pzPtr += 9;

				strcpy(psRemoteChoise->zTitle, pzPtr);

				poRemoteList->Add(psRemoteChoise);
			}
			else 
				bEnd = TRUE;
		}
    
		psRemoteChoise = NULL;

		if (DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_CHOOSEDISC), GetForegroundWindow(), (DLGPROC)ChooseDlgProc) == IDCANCEL) {
			SendString(s, "quit");

			closesocket(s);

			shutdown(s, 0);

			delete poRemoteList;

			return FALSE;
		}

		strcpy(zCategory, psRemoteChoise->zCategory);
		strcpy(zTmp, psRemoteChoise->zDiscID);

		delete poRemoteList;
	}
	else if (!strncmp(zStr, "200", 3)) {
		//
		// Get entry and loop it through the magic parse function above
		// 

		strncpy(zCategory, &zStr[4], 30);
        zCategory[30] = 0;
		pzPtr = strchr(zCategory, ' ');
		*pzPtr = 0;

		strcpy(zTmp, zID);
	}

	strcpy(zTmpID, zID);
	strcpy(zID, zTmp);

	NotifyModify(hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Reading...");

    if (!(nCDDBOptions & OPTIONS_CDDB_USEHTTP)) {
	    sprintf(zStr, "cddb read %s %s", zCategory, zTmp);
	    // Send read
	    if (SendString(s, zStr) == SOCKET_ERROR)
		    return FALSE;
    }
    else {
        if (!Reconnect(&s, &sAddrServer))
            return FALSE;

		//
		// Send read request
		//

		if (nCDDBOptions & OPTIONS_CDDB_USEPROXY) 
            sprintf(zStr, "GET http://%s%s?cmd=cddb+read+%s+%s&hello=%s&proto=1", zRemoteServer, zRemoteHTTPPath, zCategory, zTmp, zHello);
        else
            sprintf(zStr, "GET %s?cmd=cddb+read+%s+%s&hello=%s&proto=1", zRemoteHTTPPath, zCategory, zTmp, zHello);
		strcat(zStr, " HTTP/1.0");

		// Send read
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

		if (!GetString(s, zStr, 1024))
			return FALSE;
        if (!strncmp(zStr, "HTTP/", 5)) {
			do {
				if (!GetString(s, zStr, 1024))
					return FALSE;
			} while(zStr[0] != 0 && zStr[0] != '\r' && zStr[0] != '\n');
		}
    }

	bEnd = FALSE;
	while(!bEnd) {
		if (!GetString(s, zStr, 8192))
			return FALSE;

		while(zStr[strlen(zStr)-1] == '\n' || zStr[strlen(zStr)-1] == '\r')
			zStr[strlen(zStr)-1] = 0;

        if (zStr[0] == '.')
            bEnd = TRUE;
        else {
            AppendString(&pzEntry, zStr, -1);
            AppendString(&pzEntry, "\n", -1);
        }
	}

    if (pzEntry) {
        bInRemoteQuery = TRUE;
        
        ParseEntry(pzEntry, strlen(pzEntry));

        bInRemoteQuery = FALSE;

        free(pzEntry);
    }
    
    strcpy(sEntry.zCategory, zCategory);
	
	strcpy(zID, zTmpID);
	if (bParseFound) {
        strlwr(sEntry.pzDiscid);
        strlwr(zID);

		if (!strstr(sEntry.pzDiscid, zID)) {
			// Add this disc id to the list since it was a faked id

			AppendString(&sEntry.pzDiscid, ",", -1);
			AppendString(&sEntry.pzDiscid, zID, -1);
		}
	}

    if (!(nCDDBOptions & OPTIONS_CDDB_USEHTTP))
    	SendString(s, "quit");

	closesocket(s);

    shutdown(s, 0);

    return TRUE;
}


void CDDBDoQuery(BOOL bManual, BOOL* pbFoundLocal, BOOL* pbFoundRemote)
{
    BOOL bRemoteFound = FALSE;

    if (pbFoundLocal)
        *pbFoundLocal = FALSE;
    if (pbFoundRemote)
        *pbFoundRemote = FALSE;

    if (!bDBInEditor) {
        hCDDBTrayIcon = hIconLocal;
        NotifyAdd(hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query");
    }
    
    FreeEntry();
    
    bDiscFound = FALSE;

    if (!bManual)
        DebugPrintf("Auto query");
    else
        DebugPrintf("Manual query");

    if (((nOptions & OPTIONS_QUERYLOCAL) || bDBInEditor) && (nOptions & OPTIONS_USECDDB) && !bManual) {
        if (!bDBInEditor) {
            hCDDBTrayIcon = hIconLocal;
            NotifyModify(hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query local database");
        }

        if (QueryLocal())
            bDiscFound = TRUE;

        if (bDiscFound) {
            DebugPrintf("Found disc %s in category %s!", sEntry.pzArtist, sEntry.zCategory);

			DebugPrintf("After disc found, sEntry.nTracks = %d", sEntry.nTracks);

            if (pbFoundLocal)
                *pbFoundLocal = TRUE;
        }
        else
            DebugPrintf("*NO* disc found in DB!");
    }

    if (!bDiscFound && ((nOptions & OPTIONS_QUERYREMOTE) || bManual) && !bDBInEditor) {
		WSADATA sData;

        hCDDBTrayIcon = hIconRemote;
        NotifyModify(hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query remote server");

		if (WSAStartup(0x0101, &sData)) {
			MessageBox(NULL, "Failed to initialize winsock!", APPNAME, MB_OK);

			DebugPrintf("WSAStartup() failed!");
		}
		else {
			if (QueryRemote()) {
				bDiscFound = TRUE;
				bRemoteFound = TRUE;
    
                if (pbFoundRemote)
                    *pbFoundRemote = TRUE;
			}

	        WSACleanup();
		}
    }

    if (!bDBInEditor)
        NotifyDelete(hMainWnd, 200);

	if (!bDiscFound) {
		free(sEntry.pzDiscid);
		sEntry.pzDiscid = NULL;

		AppendString(&sEntry.pzDiscid, zID, -1);

        DebugPrintf("*NO* disc found in DB!");
	}
    else {
        FixEntry();

        DebugPrintf("Found disc %s - %s in category %s!", sEntry.pzArtist, sEntry.pzTitle, sEntry.zCategory);
    }
}


void WriteLine(FILE* fp, const char* pzName, const char* pzValue)
{
    char zStr[80];
    unsigned int nPos = 0;

    if (strlen(pzValue) > (75 - strlen(pzName))) {
        while(nPos < strlen(pzValue)) {
            strncpy(zStr, pzValue+nPos, 75-strlen(pzName));
            zStr[75-strlen(pzName)] = 0;

            fprintf(fp, "%s=%s\n", pzName, zStr);

            nPos += strlen(zStr);
        }
    }
    else
        fprintf(fp, "%s=%s\n", pzName, pzValue);
}


void WriteEntry(FILE* fp, BOOL bLocal)
{
    int nLoop;
    char* pzStr;
    char zName[80];

    DebugPrintf("Write entry");

    // Write header
    fputs("# xmcd CD database file\n", fp);
    fputs("# Copyright (C) 1995 Ti Kan\n", fp);
    fputs("#\n", fp);

    // Write track offsets
    fputs("# Track frame offsets:\n", fp);

    for (nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
        if (sEntry.pnFrames)
            fprintf(fp, "#\t%d\n", sEntry.pnFrames[nLoop]);
        else
            fprintf(fp, "#\t%d\n", pnQueryFrames[nLoop]);
    }

    // Write disc length
    if (!sEntry.nDiscLength)
        sEntry.nDiscLength = nQueryDiscLength;

    fputs("#\n", fp);
    fprintf(fp, "# Disc length: %d seconds\n", sEntry.nDiscLength);

    // Write revision and submitted via
    fputs("#\n", fp);
    fprintf(fp, "# Revision: %d\n", sEntry.nRevision);
    if (bLocal && sEntry.zSubmitted[0])
        fprintf(fp, "# Submitted via: %s\n", sEntry.zSubmitted);
    else
        fprintf(fp, "# Submitted via: NotifyCDPlayer(CDDB) %s\n", VERSION);

    fputs("#\n", fp);

    // DISCID
    if (sEntry.pzDiscid && bLocal)
        WriteLine(fp, "DISCID", sEntry.pzDiscid);
    else
        WriteLine(fp, "DISCID", zSendID);

    // DTITLE
    pzStr = (char*)malloc(strlen(sEntry.pzArtist) + strlen(sEntry.pzTitle) + 10);
    if (sEntry.pzArtist && sEntry.pzTitle) {
        strcpy(pzStr, sEntry.pzArtist);
        strcat(pzStr, " / ");
        strcat(pzStr, sEntry.pzTitle);
    }
    else if (sEntry.pzArtist)
        strcpy(pzStr, sEntry.pzArtist);
    else if (sEntry.pzTitle)
        strcpy(pzStr, sEntry.pzTitle);

    WriteLine(fp, "DTITLE", pzStr);

    free(pzStr);

    // TRACKS
    for (nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
        sprintf(zName, "TTITLE%d", nLoop);
        if (sEntry.ppzTracks && sEntry.ppzTracks[nLoop])
            WriteLine(fp, zName, sEntry.ppzTracks[nLoop]);
        else
            WriteLine(fp, zName, "");
    }
    // EXTD
    if (sEntry.pzExtD)
        WriteLine(fp, "EXTD", sEntry.pzExtD);
    else
        WriteLine(fp, "EXTD", "");
    // EXTT
    for (nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
        sprintf(zName, "EXTT%d", nLoop);
        if (sEntry.ppzTracksExt && sEntry.ppzTracksExt[nLoop])
            WriteLine(fp, zName, sEntry.ppzTracksExt[nLoop]);
        else
            WriteLine(fp, zName, "");
    }

    if (bLocal) {
        if (sEntry.pzPlayorder) {
            for (nLoop = 0 ; nLoop < (signed)strlen(sEntry.pzPlayorder) ; nLoop ++) {
                if (sEntry.pzPlayorder[nLoop] == ' ')
                    sEntry.pzPlayorder[nLoop] = ',';
            }

            WriteLine(fp, "PLAYORDER", sEntry.pzPlayorder);

            for (nLoop = 0 ; nLoop < (signed)strlen(sEntry.pzPlayorder) ; nLoop ++) {
                if (sEntry.pzPlayorder[nLoop] == ',')
                    sEntry.pzPlayorder[nLoop] = ' ';
            }
        }
        else
            fputs("PLAYORDER=\n", fp);
    }
    else
        fputs("PLAYORDER=\n", fp);

    DebugPrintf("Entry written");
}


/*
 * cddb_sum
 *	Convert an integer to its text string representation, and
 *	compute its checksum.  Used by cddb_discid to derive the
 *	disc ID.
 *
 * Args:
 *	n - The integer value.
 *
 * Return:
 *	The integer checksum.
 */
int
cddb_sum(int n)
{
	char	buf[12],
		*p;
	int	ret = 0;

	/* For backward compatibility this algorithm must not change */
	sprintf(buf, "%lu", n);
	for (p = buf; *p != '\0'; p++)
		ret += (*p - '0');

	return (ret);
}

/*
 * cddb_discid
 *	Compute a magic disc ID based on the number of tracks,
 *	the length of each track, and a checksum of the string
 *	that represents the offset of each track.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	The integer disc ID.
 */

unsigned long
cddb_discid(unsigned char nTracks, int* pnMin, int* pnSec)
{
	int	i,
		t = 0,
		n = 0;

	/* For backward compatibility this algorithm must not change */
	for (i = 0; i < (int) nTracks; i++) {
		n += cddb_sum((pnMin[i] * 60) + pnSec[i]);

		t += ((pnMin[i+1] * 60) + pnSec[i+1]) - ((pnMin[i] * 60) + pnSec[i]);
	}

	return ((n % 0xff) << 24 | t << 8 | nTracks);
}

void CDGetEndFrame(int nOffset, int* pnMin, int* pnSec)
{
    int nFrame;

    sMCIStatus.dwItem = MCI_STATUS_LENGTH;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
				    (DWORD) (LPVOID) &sMCIStatus);
    nFrame = (int)((float)sMCIStatus.dwReturn/1000*75) + nOffset;
    nFrame ++; // Due to bug in MCI according to CDDB docs!

    DebugPrintf("Frame End = %d", nFrame);

    nQueryDiscLength = nFrame / 75;

    DebugPrintf("Disc length = %d", nQueryDiscLength);

	*pnMin = nFrame / 75 / 60;
	*pnSec = (nFrame / 75) % 60;
}


void CDGetAbsoluteTrackPos(int nTrack, int* pnFrame, int* pnMin, int* pnSec)
{
    sMCIStatus.dwItem = MCI_STATUS_POSITION;
	sMCIStatus.dwTrack = nTrack;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);
    *pnFrame = (int)((float)sMCIStatus.dwReturn/1000*75);

    DebugPrintf("Frame %d = %d", nTrack, *pnFrame);

    *pnMin = *pnFrame / 75 / 60;
	*pnSec = (*pnFrame / 75) % 60;
}


void CDDBGetDiscID(char* pzID)
{
    int nLoop;
    int nMaxTrack = CDGetTracks();
    int* pnMin = NULL;
    int* pnSec = NULL;

    pnMin = (int*)malloc((nMaxTrack+1)*sizeof(int));
    pnSec = (int*)malloc((nMaxTrack+1)*sizeof(int));
    if (pnQueryFrames)
        free(pnQueryFrames);

    pnQueryFrames = (int*)malloc((nMaxTrack+1)*sizeof(int));

    nNewTrackNum = nMaxTrack;

    DebugPrintf("-- New disc! -----------------------------------------------------");

    sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	mciSendCommand(sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);

    for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) 
        CDGetAbsoluteTrackPos(nLoop+1, &pnQueryFrames[nLoop], &pnMin[nLoop], &pnSec[nLoop]);

    CDGetEndFrame(pnQueryFrames[0], &pnMin[nLoop], &pnSec[nLoop]);

    sprintf(zID, "%08x", cddb_discid((unsigned char)nMaxTrack, pnMin, pnSec));

    DebugPrintf("Disc ID = %s", zID);

    sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);

    if (pzID)
		strcpy(pzID, zID);

    if (pnMin) {
        free(pnMin);
        free(pnSec);
    }
}


/////////////////////////////////////////////////////////////////////
//
// CDDB STUFF!
//
/////////////////////////////////////////////////////////////////////

BOOL bIsEnd = FALSE;

BOOL CDDBSave()
{
    DebugPrintf("DBSave()");

    if (!(nOptions & OPTIONS_STORELOCAL)) {
        DebugPrintf("Store local disabled");

        MessageBox(NULL, "Store local disabled. Changes will not be saved", APPNAME, MB_OK);

        return FALSE;
    }

    if (!sEntry.zCategory[0]) {
        DebugPrintf("Missing category");

        MessageBox(NULL, "You must specifiy a category in order to save in the CDDB format", APPNAME, MB_OK);

        return FALSE;
    }

    if (nCDDBType == 1) {            // Windows
        FILE* fpIn;
        FILE* fpOut;
        char zPath[256];
        char zIDStart[3];
        int nIDStart;
        char zTmpFileName[256];
        WIN32_FIND_DATA sFF;
        HANDLE hFind;
        char zCategory[20];
        char zFileIDStart[3];
        char zFileIDEnd[3];
        int nFileIDStart;
        int nFileIDEnd;
        BOOL bWritten = FALSE;
        BOOL bFileFound = FALSE;

        DebugPrintf("Windows format");

        GetTmpFile(zTmpFileName);

        // First we have to check for an appropriate file

        strncpy(zIDStart, zID, 2);
        zIDStart[2] = 0;

        sscanf(zIDStart, "%X", &nIDStart);

        // Loop through all files in all dirs to find the correct file to search!

        strcpy(zCategory, sEntry.zCategory);
        zCategory[8] = 0;

        sprintf(zPath, "%s%s", zCDDBPath, zCategory);
        CreateDirectory(zPath, NULL);
        sprintf(zPath, "%s%s\\*.*", zCDDBPath, zCategory);

        DebugPrintf("Searching %s", zPath);

        hFind = FindFirstFile(zPath, &sFF);
        while(hFind && hFind != INVALID_HANDLE_VALUE) {
            if (!(sFF.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strlen(sFF.cFileName) == 6) {
                strncpy(zFileIDStart, &sFF.cFileName[strlen(sFF.cFileName) - 6], 2);
                strncpy(zFileIDEnd, &sFF.cFileName[strlen(sFF.cFileName) - 2], 2);
                zFileIDStart[2] = zFileIDEnd[2] = 0;

                sscanf(zFileIDStart, "%X", &nFileIDStart);
                sscanf(zFileIDEnd, "%X", &nFileIDEnd);

                if (nIDStart >= nFileIDStart && nIDStart <= nFileIDEnd) {
                    sprintf(zPath, "%s%s\\%s", zCDDBPath, zCategory, sFF.cFileName);

                    DebugPrintf("Found file %s", zPath);

                    bFileFound = TRUE;

                    fpIn = fopen(zPath, "r");
                    if (fpIn) {
                        char zStr[1025];
                        char zCompare[80];
                        BOOL bSkip = FALSE;

                        sprintf(zCompare, "#FILENAME=%s", zID);

                        fpOut = fopen(zTmpFileName, "w");
                        if (!fpOut) {
                            DebugPrintf("Failed to open temporary file! (%s)", zTmpFileName);

                            return FALSE;
                        }
                        
                        do {
                            if (fgets(zStr, 1024, fpIn)) {
                                if (strstr(zStr, zCompare)) {
                                    DebugPrintf("Found entry in file!");

                                    fprintf(fpOut, "#FILENAME=%s\n", zID);

                                    WriteEntry(fpOut, TRUE);

                                    bWritten = TRUE;
                                    bSkip = TRUE;
                                }
                                else {
                                    if (bSkip && strstr(zStr, "#FILENAME="))
                                        bSkip = FALSE;

                                    if (!bSkip)
                                        fputs(zStr, fpOut);
                                }
                            }
                        } while(!feof(fpIn));
                        
                        fclose(fpOut);
                        fclose(fpIn);

                        CloseEntry();

                        if (!CopyFile(zTmpFileName, zPath, FALSE))
                            DebugPrintf("CopyFile failed: %d", GetLastError());

                        if (!DeleteFile(zTmpFileName))
                            DebugPrintf("DeleteFile failed: %d", GetLastError());
                    }
                    else
                        DebugPrintf("Failed to open %s for reading", zPath);

                    FindClose(hFind);

                    hFind = NULL;                       
                }
            }
            
            if (hFind != NULL && !FindNextFile(hFind, &sFF)) {
                FindClose(hFind);

                hFind = NULL;
            }
        }
        
        // or if we should append it
        if (!bWritten && bFileFound) {
            DebugPrintf("Appending entry");

            CloseEntry();

            fpOut = fopen(zPath, "a");
            if (!fpOut) {
                DebugPrintf("Failed to open %s", zPath);

                return FALSE;
            }

            fprintf(fpOut, "#FILENAME=%s\n", zID);
            WriteEntry(fpOut, TRUE);

            fclose(fpOut);
        }
        else if (!bWritten) {          // Create new
            DebugPrintf("Create new file!");

            sprintf(zPath, "%s%s", zCDDBPath, zCategory);
            CreateDirectory(zPath, NULL);

            sprintf(zPath, "%s%s\\%sTO%s", zCDDBPath, zCategory, zIDStart, zIDStart);
            DebugPrintf("Creating file %s", zPath);

            CloseEntry();
    
            fpOut = fopen(zPath, "w");
            if (!fpOut) {
                DebugPrintf("Failed to open %s", zPath);

                return FALSE;
            }
            else {
                fprintf(fpOut, "#FILENAME=%s\n", zID);
                WriteEntry(fpOut, TRUE);

                fclose(fpOut);
            }
        }
    }
    else if (nCDDBType == 2) {       // Unix
        char zPath[256];
        FILE* fp;

        DebugPrintf("Unix format");

        sprintf(zPath, "%s%s", zCDDBPath, sEntry.zCategory);
        CreateDirectory(zPath, NULL);

        sprintf(zPath, "%s%s\\%s", zCDDBPath, sEntry.zCategory, zID);

        DebugPrintf("Writing file %s", zPath);

        CloseEntry();

        fp = fopen(zPath, "w");
        if (fp) {
            WriteEntry(fp, TRUE);

            fclose(fp);
        }
    }

    return TRUE;
}


BOOL CDDBInternetGet(HWND hWnd)
{
    if (!strlen(zRemoteServer) || !nRemotePort) {
        MessageBox(hWnd, "You must configure a remote server to use this function!", APPNAME, MB_OK);

        return FALSE;
    }

    // Do a remote query!

    CDDBDoQuery(TRUE, NULL, NULL);

    return bDiscFound;
}


void CDDBInternetSend(HWND hWnd)
{
    char zTmpFileName[255];
    char zTmp[80];
    int nLoop;
    FILE* fp;
    BOOL bNoMIME = FALSE;
    char zToAddress[80];

    if (MessageBox(hWnd, "Are You sure You want to send this entry to the CDDB repository?", APPNAME, MB_YESNO) == IDNO)
        return;

    DebugPrintf("DBInternetSend()");	
    
    for (nLoop = 0 ; nLoop < nCDDBNumCategories ; nLoop ++) {
        if (!stricmp(sEntry.zCategory, azCategories[nLoop]))
            break;
    }
    if (nLoop == nCDDBNumCategories) {
    	DebugPrintf("Illegal category '%s'", sEntry.zCategory);

		MessageBox(NULL, "Invalid category for submitting to the CDDB repository!", APPNAME, MB_OK);
        return;
    }

    if (strchr(sEntry.pzArtist, '/') || strchr(sEntry.pzArtist, ',')) {
    	DebugPrintf("Illegal character in artist name");

		MessageBox(NULL, "An illegal (/ or ,) character was found in the artist field!", APPNAME, MB_OK);
        return;
    }
    if (strchr(sEntry.pzTitle, '/') || strchr(sEntry.pzTitle, ',')) {
    	DebugPrintf("Illegal character in title name");

		MessageBox(NULL, "An illegal (/ or ,) character was found in the title field!", APPNAME, MB_OK);
        return;
    }
    if ((!sEntry.pzArtist || !strlen(sEntry.pzArtist)) && (!sEntry.pzTitle || !strlen(sEntry.pzTitle))) {
    	DebugPrintf("Both artist and title is empty");

		MessageBox(NULL, "You cannot leave both artist and title empty!", APPNAME, MB_OK);
        return;
    }

    if (!sEntry.pzDiscid || !strlen(sEntry.pzDiscid) || !strlen(zID)) {
        DebugPrintf("Error! Discid is empty!");
        return;
    }

    if (sEntry.pzDiscid)
        strlwr(sEntry.pzDiscid);
    strlwr(zID);

    if (sEntry.pzDiscid && !strstr(sEntry.pzDiscid, zID)) {
        free(sEntry.pzDiscid);
        sEntry.pzDiscid = NULL;

        AppendString(&sEntry.pzDiscid, zID, -1); 
    }
    
    strcpy(zTmp, zID);
	CDDBGetDiscID(NULL);
	if (strcmp(zTmp, zID)) {
        int* pnMin = NULL;
        int* pnSec = NULL;

		DebugPrintf("Discid in entry isn't the same as the current disc!");

        pnMin = (int*)malloc((sEntry.nTracks+1)*sizeof(int));
        pnSec = (int*)malloc((sEntry.nTracks+1)*sizeof(int));

        for (nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
            pnMin[nLoop] = sEntry.pnFrames[nLoop] / 75 / 60;
	        pnSec[nLoop] = (sEntry.pnFrames[nLoop] / 75) % 60;
        }
        pnMin[nLoop] = (sEntry.nDiscLength * 75 + 1) / 75 / 60;
	    pnSec[nLoop] = ((sEntry.nDiscLength * 75 + 1) / 75)% 60;

        sprintf(zSendID, "%08x", cddb_discid((unsigned char)sEntry.nTracks, pnMin, pnSec));
    }
    else {
        strcpy(zSendID, zID);

        free(sEntry.pnFrames);
        sEntry.pnFrames = (int*) malloc(sEntry.nTracks*sizeof(int));
        memcpy(sEntry.pnFrames, pnQueryFrames, sEntry.nTracks*sizeof(int));
        sEntry.nDiscLength = nQueryDiscLength;
    }

    DebugPrintf("Send DiscID = %s", zSendID);

    if (!stricmp(sEntry.pzArtist, "New Artist") || !stricmp(sEntry.pzTitle, "New Title")) {
		DebugPrintf("Bad Artist or Title entries");

		MessageBox(NULL, "Illegal artist or title value", APPNAME, MB_OK);

		return;
	}

	for (nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
        if (sEntry.ppzTracks[nLoop] && !strnicmp(sEntry.ppzTracks[nLoop], "Track ", 6)) {
			DebugPrintf("Bad track name");

			MessageBox(NULL, "Illegal track name", APPNAME, MB_OK);

			return;
		}
	}

	GetTmpFile(zTmpFileName);

    fp = fopen(zTmpFileName, "w");
    if (!fp) {
        DebugPrintf("Couldn't open temporary file! (%s)", zTmpFileName);

        MessageBox(hWnd, "Couldn't create temp file", APPNAME, MB_OK);

        return;
    }

    sEntry.nRevision ++;

    WriteEntry(fp, FALSE);

    fclose(fp);

    //////////////////////////////////////////////////////////////////////
    //
    // Do SMTP comms!
    //
    //////////////////////////////////////////////////////////////////////

    SOCKET s;
    struct sockaddr_in sAddrServer;
    struct hostent* psHostEntry;
    char zStr[8192];
    char zHostName[256];
	WSADATA sData;

	if (WSAStartup(0x0101, &sData)) {
		MessageBox(NULL, "Failed to initialize winsock!", APPNAME, MB_OK);

		DebugPrintf("WSAStartup() failed!");
	}

DebugPrintf("Send SMTP");

    if (gethostname(zHostName, 255) == SOCKET_ERROR) {
        DebugPrintf("gethostname() failed!");

        MessageBox(hWnd, "Couldn't get local hostname!", APPNAME, MB_OK);

        goto send_end;
    }

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        DebugPrintf("socket() failed %d", WSAGetLastError());

        goto send_end;
    }

    // Set up remote address

    sAddrServer.sin_family = AF_INET;
    sAddrServer.sin_port = htons((short)25);

    psHostEntry = gethostbyname(zRemoteEmailServer);
    if (!psHostEntry) {
        DebugPrintf("gethostbyname() failed to get address for %s. Error code: %d", zRemoteEmailServer, WSAGetLastError());

        shutdown(s, 0);

        goto send_end;
    }
    
    memcpy(&sAddrServer.sin_addr.s_addr, psHostEntry->h_addr_list[0], sizeof(sAddrServer.sin_addr.s_addr));

    if (connect(s, (struct sockaddr*)&sAddrServer, sizeof(sAddrServer))) {
        MessageBox(hWnd, "Failed to connect to remote server!", APPNAME, MB_OK);

        DebugPrintf("connect() failed %d", WSAGetLastError());

        shutdown(s, 0);

        goto send_end;
    }
        
DebugPrintf("Connected to %s", zRemoteEmailServer);

    //
    // Get server sign-on and send handshake!
    //

    Sleep(200);
	
    if (!GetString(s, zStr, 1024))
        goto send_end;
    // Check for valid sign-on status      
    if (strncmp(zStr, "220", 3)) {
        DebugPrintf("Server not ready");

        MessageBox(hWnd, "Server not ready", APPNAME, MB_OK);

        closesocket(s);
    
        shutdown(s, 0);

        goto send_end;
    }
	int nTmp;
	nTmp = nRemoteTimeout;
	nRemoteTimeout = 2;
    GetString(s, zStr, 1024, TRUE);        // FOR ESMTP, do not return if failed!
	nRemoteTimeout = nTmp;

    // Send HELO
    if (zDomain[0])
        sprintf(zStr, "HELO %s.%s", zHostName, zDomain);
    else
        sprintf(zStr, "HELO %s", zHostName);
    if (SendString(s, zStr) == SOCKET_ERROR)
        return;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "250", 3)) {
        DebugPrintf("HELO answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied access after HELO", APPNAME, MB_OK);

        goto send_end;
    }
    // Send MAIL FROM
    sprintf(zStr, "MAIL FROM:<%s>", zEmailAddress);
    if (SendString(s, zStr) == SOCKET_ERROR)
        return;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "250", 3)) {
        DebugPrintf("MAIL FROM answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied MAIL FROM address", APPNAME, MB_OK);

        goto send_end;
    }
    // Send RCPT TO
#ifdef _DEBUG
    if (MessageBox(NULL, "Do You want to make a test submission?", APPNAME, MB_YESNO) == IDYES)
        strcpy(zToAddress, "cddb-test@amb.org");
    else
        strcpy(zToAddress, "xmcd-cddb@amb.org");
#else
    strcpy(zToAddress, "xmcd-cddb@amb.org");
#endif
    sprintf(zStr, "RCPT TO:<%s>", zToAddress);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "250", 3)) {
        DebugPrintf("RCPT TO answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied RCPT TO address", APPNAME, MB_OK);

        goto send_end;
    }
    // Send DATA
    strcpy(zStr, "DATA");
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "354", 3)) {
        DebugPrintf("DATA answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied DATA", APPNAME, MB_OK);

        goto send_end;
    }

    // Send message
    sprintf(zStr, "From: %s", zEmailAddress);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    sprintf(zStr, "To: %s", zToAddress);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    strcpy(zTmp, sEntry.zCategory);
    sprintf(zStr, "Subject: cddb %s %s", strlwr(zTmp), zSendID);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    
    bNoMIME = GetPrivateProfileInt("CDDB", "No_MIME", 0, "CDPLAYER.INI");

    if (!bNoMIME) {
        SendString(s, "MIME-Version: 1.0");
        SendString(s, "Content-Type: text/plain; charset=iso-8859-1");
        SendString(s, "Content-Transfer-Encoding: quoted-printable");
    }

    // Send last newline after headers
    if (SendString(s, "\n") == SOCKET_ERROR)
        goto send_end;

    fp = fopen(zTmpFileName, "r");
    do {
        if (fgets(zStr, 1024, fp)) {
            if (strlen(zStr) && zStr[strlen(zStr) - 1] == '\n')
                zStr[strlen(zStr) - 1] = 0;

            if (!bNoMIME) {
                if (SendStringQuotedPrintable(s, zStr) == SOCKET_ERROR)
                    goto send_end;
            }
            else {
                if (SendString(s, zStr) == SOCKET_ERROR)
                    goto send_end;
            }
        }
    } while(!feof(fp));
    fclose(fp);

    if (SendString(s, ".") == SOCKET_ERROR)
        goto send_end;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "250", 3)) {
        DebugPrintf("DATA end answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied DATA end", APPNAME, MB_OK);

        goto send_end;
    }

    // Send QUIT
    if (SendString(s, "QUIT") == SOCKET_ERROR) 
        goto send_end;

    MessageBox(hWnd, "Entry submitted!", APPNAME, MB_OK);

send_end:
    closesocket(s);

    shutdown(s, 0);

    WSACleanup();

    fclose(fp);

    DeleteFile(zTmpFileName);
}


void CDDBDelete(const char* pzID)
{   
    DebugPrintf("CDDBDelete()");

    if (!(nOptions & OPTIONS_STORELOCAL)) {
        DebugPrintf("Store local disabled");

        return;
    }

    if (!sEntry.zCategory[0]) {
        DebugPrintf("Missing category");

        return;
    }

    if (nCDDBType == 1) {            // Windows
        FILE* fpIn;
        FILE* fpOut;
        char zPath[256];
        char zIDStart[3];
        int nIDStart;
        char zTmpFileName[256];
        WIN32_FIND_DATA sFF;
        HANDLE hFind;
        char zCategory[20];
        char zFileIDStart[3];
        char zFileIDEnd[3];
        int nFileIDStart;
        int nFileIDEnd;
        BOOL bWritten = FALSE;
        BOOL bFileFound = FALSE;

        DebugPrintf("Windows format");

        GetTmpFile(zTmpFileName);

        // First we have to check for an appropriate file

        strncpy(zIDStart, pzID, 2);
        zIDStart[2] = 0;

        sscanf(zIDStart, "%X", &nIDStart);

        // Loop through all files in all dirs to find the correct file to search!

        strcpy(zCategory, sEntry.zCategory);
        zCategory[8] = 0;

        sprintf(zPath, "%s%s", zCDDBPath, zCategory);
        CreateDirectory(zPath, NULL);
        sprintf(zPath, "%s%s\\*.*", zCDDBPath, zCategory);

        DebugPrintf("Searching %s", zPath);

        hFind = FindFirstFile(zPath, &sFF);
        while(hFind && hFind != INVALID_HANDLE_VALUE) {
            if (!(sFF.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strlen(sFF.cFileName) == 6) {
                strncpy(zFileIDStart, &sFF.cFileName[strlen(sFF.cFileName) - 6], 2);
                strncpy(zFileIDEnd, &sFF.cFileName[strlen(sFF.cFileName) - 2], 2);
                zFileIDStart[2] = zFileIDEnd[2] = 0;

                sscanf(zFileIDStart, "%X", &nFileIDStart);
                sscanf(zFileIDEnd, "%X", &nFileIDEnd);

                if (nIDStart >= nFileIDStart && nIDStart <= nFileIDEnd) {
                    sprintf(zPath, "%s%s\\%s", zCDDBPath, zCategory, sFF.cFileName);

                    DebugPrintf("Found file %s", zPath);

                    bFileFound = TRUE;

                    fpIn = fopen(zPath, "r");
                    if (fpIn) {
                        char zStr[1025];
                        char zCompare[80];
                        BOOL bSkip = FALSE;
                        HANDLE hFile;

                        sprintf(zCompare, "#FILENAME=%s", pzID);

                        fpOut = fopen(zTmpFileName, "w");
                        if (!fpOut) {
                            DebugPrintf("Failed to open temporary file! (%s)", zTmpFileName);

                            return;
                        }
                        
                        do {
                            if (fgets(zStr, 1024, fpIn)) {
                                if (strstr(zStr, zCompare)) {
                                    DebugPrintf("Found entry in file!");
                                    DebugPrintf("Skipping entry in order to delete it!");

                                    bWritten = TRUE;
                                    bSkip = TRUE;
                                }
                                else {
                                    if (bSkip && strstr(zStr, "#FILENAME="))
                                        bSkip = FALSE;

                                    if (!bSkip)
                                        fputs(zStr, fpOut);
                                }
                            }
                        } while(!feof(fpIn));
                        
                        fclose(fpOut);
                        fclose(fpIn);

                        CloseEntry();

                        if (!CopyFile(zTmpFileName, zPath, FALSE))
                            DebugPrintf("CopyFile failed: %d", GetLastError());

                        if (!DeleteFile(zTmpFileName))
                            DebugPrintf("DeleteFile failed: %d", GetLastError());

		                hFile = CreateFile(zPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                        if (hFile != INVALID_HANDLE_VALUE) {
                            if (GetFileSize(hFile, NULL) == 0) {
                                DebugPrintf("The resulting file is empty! Deleting...");

                                CloseHandle(hFile);

                                if (!DeleteFile(zPath))
                                    DebugPrintf("Delete of %s failed with error code %d", zPath, GetLastError());
                            }
                            else
                                CloseHandle(hFile);
                        }
                    }
                    else
                        DebugPrintf("Failed to open %s for reading", zPath);

                    FindClose(hFind);

                    hFind = NULL;                       
                }
            }
            
            if (hFind != NULL && !FindNextFile(hFind, &sFF)) {
                FindClose(hFind);

                hFind = NULL;
            }
        }
        
        if (!bWritten && bFileFound)
            DebugPrintf("Entry not found in file, delete not nessecary");
        else if (!bWritten)
            DebugPrintf("File for entry not found, delete not nessecary");
    }
    else if (nCDDBType == 2) {       // Unix
        char zPath[256];

        DebugPrintf("Unix format");

        sprintf(zPath, "%s%s", zCDDBPath, sEntry.zCategory);
        CreateDirectory(zPath, NULL);

        sprintf(zPath, "%s%s\\%s", zCDDBPath, sEntry.zCategory, zID);

        CloseEntry();

        DebugPrintf("Deleting file %s", zPath);

        if (!DeleteFile(zPath))
            DebugPrintf("DeleteFile failed: %d", GetLastError());
    }

    return;
}


BOOL CDDBOpen()
{
	bDBInEditor = TRUE;

	hDBFind = INVALID_HANDLE_VALUE;
	nDBCategory = -1;

	psDBSavedEntry = (CDDB_DISC_ENTRY*)malloc(sizeof(CDDB_DISC_ENTRY));
	memcpy(psDBSavedEntry, &sEntry, sizeof(CDDB_DISC_ENTRY));
	memset(&sEntry, 0, sizeof(CDDB_DISC_ENTRY));

	strcpy(zDBSavedID, zID);
    zID[0] = 0;

	return TRUE;
}


void CDDBClose()
{
	bDBInEditor = FALSE;

    CloseEntry();

	pzGetNextIDCurrData = NULL;

    FreeEntry();

	memcpy(&sEntry, psDBSavedEntry, sizeof(CDDB_DISC_ENTRY));
	free(psDBSavedEntry);

	strcpy(zID, zDBSavedID);
}


BOOL FindNextID(char* pzID)
{
    char* pzPtr;
	char* pzEndPtr;

    if (!pzGetNextIDCurrData)
        return FALSE;

    pzPtr = strstr(pzGetNextIDCurrData, "#FILENAME=");
    if (pzPtr) {
        pzGetNextIDCurrData = pzPtr += 10;

        pzEndPtr = strchr(pzPtr, '\n');
		if (!pzEndPtr)
			pzEndPtr = pzReadData + dwSizeOfFile;

		strncpy(pzID, pzPtr, pzEndPtr - pzPtr);
		pzID[pzEndPtr - pzPtr] = 0;
		if (pzID[pzEndPtr - pzPtr - 1] == '\r')
			pzID[pzEndPtr - pzPtr - 1] = 0;

        return TRUE;
    }

	pzGetNextIDCurrData = NULL;

    return FALSE;
}


BOOL CDDBGetID(char* pzID, char** ppzArtist, char** ppzTitle, char** ppzCategory)
{
    BOOL bFound = FALSE;
    char zCategory[12];
    char zPath[MAX_PATH];
    char zName[MAX_PATH];
    
	if (nCDDBType == 2) {   // Unix
start_again_unix:
		while (hDBFind == INVALID_HANDLE_VALUE) {
			nDBCategory ++;
			
			if (nDBCategory == nNumCategories) {
				bIsEnd = TRUE;

				return FALSE;
			}

            sprintf(zPath, "%s%s\\*", zCDDBPath, ppzCategories[nDBCategory]);

			hDBFind = FindFirstFile(zPath, &sDBFindData);
		}

		if (hDBFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(sDBFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					bFound = TRUE;

					bIsEnd = FALSE;

					strcpy(pzID, sDBFindData.cFileName);

		            sprintf(zPath, "%s%s\\%s", zCDDBPath, ppzCategories[nDBCategory], pzID);
                    
                    strcpy(zID, pzID);

					if (ReadEntry(zPath, ppzCategories[nDBCategory])) {
						CDDBGetInfo(pzID, "ARTIST", ppzArtist);
						CDDBGetInfo(pzID, "TITLE", ppzTitle);
						CDDBGetInfo(pzID, "CATEGORY", ppzCategory);
					}
				}

				if (!FindNextFile(hDBFind, &sDBFindData)) {
					FindClose(hDBFind);

					hDBFind = INVALID_HANDLE_VALUE;

                    if (!bFound)
    					goto start_again_unix;
				}
			} while (!bFound);
		}

		return bFound;
    }   
    else if (nCDDBType == 1) {     // Windows
start_again_windows:
        if (!pzGetNextIDCurrData) {
            while (hDBFind == INVALID_HANDLE_VALUE) {
			    nDBCategory ++;
			    
			    if (nDBCategory == nNumCategories) {
				    bIsEnd = TRUE;

				    return FALSE;
			    }

                strcpy(zCategory, ppzCategories[nDBCategory]);
                zCategory[8] = 0;

                sprintf(zPath, "%s%s\\*", zCDDBPath, zCategory);

			    hDBFind = FindFirstFile(zPath, &sDBFindData);
		    }

		    if (hDBFind != INVALID_HANDLE_VALUE) {
			    do {
				    if (!(sDBFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					    bFound = TRUE;

					    bIsEnd = FALSE;

                        strcpy(zName, sDBFindData.cFileName);
				    }

                    if (!FindNextFile(hDBFind, &sDBFindData)) {
					    FindClose(hDBFind);

					    hDBFind = INVALID_HANDLE_VALUE;

                        if (!bFound)
                            goto start_again_windows;
				    }
			    } while (!bFound);
		    }

            if (bFound) {
                strcpy(zCategory, ppzCategories[nDBCategory]);
                zCategory[8] = 0;

                sprintf(zDBLastPath, "%s%s\\%s", zCDDBPath, zCategory, zName);

				OpenEntry(zDBLastPath);

				pzGetNextIDCurrData = pzReadData;
            }
            else
                return FALSE;
        }

        if (!FindNextID(pzID))
            goto start_again_windows;

		strcpy(zID, pzID);

		if (ReadEntry(zDBLastPath, ppzCategories[nDBCategory])) {
			CDDBGetInfo(zID, "ARTIST", ppzArtist);
			CDDBGetInfo(zID, "TITLE", ppzTitle);
			CDDBGetInfo(zID, "CATEGORY", ppzCategory);
		}

        return TRUE;
    }

    return FALSE;
}


BOOL CDDBIsEnd()
{
    if (bIsEnd)
        zID[0] = 0;

    return !bIsEnd;
}


char** CDDBGetTrackTitles(const char* pzID, int nNumTracks)
{
    char** ppzTracks;
    int nLoop;

    if (bDBInEditor && strcmp(zID, pzID)) {
		strcpy(zID, pzID);

		CDDBDoQuery(FALSE, NULL, NULL);
	}
    
    ppzTracks = (char**) malloc(nNumTracks * sizeof(char*));
    if (bDiscFound) {
        for (nLoop = 0 ; nLoop < nNumTracks && nLoop < sEntry.nTracks ; nLoop ++) {
            ppzTracks[nLoop] = (char*) malloc((strlen(sEntry.ppzTracks[nLoop])+1)*sizeof(char));
		    ppzTracks[nLoop][0] = 0;
	    }

        // Copy data to ppzTracks

        for (nLoop = 0 ; nLoop < nNumTracks && nLoop < sEntry.nTracks ; nLoop ++)
            strcpy(ppzTracks[nLoop], sEntry.ppzTracks[nLoop]);
    }
	else
		nLoop = 0;

    if (nLoop < nNumTracks) {
		for ( ; nLoop < nNumTracks ; nLoop ++) {
			ppzTracks[nLoop] = (char*) malloc(64*sizeof(char));
			ppzTracks[nLoop][0] = 0;
		}

		for (nLoop = 0 ; nLoop < nNumTracks ; nLoop ++)
			sprintf(ppzTracks[nLoop], "Track %d", nLoop + 1);
	}

    return ppzTracks;
}


void CDDBSetTrackTitles(const char* /*pzID*/, char** ppzTracks, int nNumTracks)
{
    int nLoop;

    if (sEntry.ppzTracks) {
        for(nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) {
            if (sEntry.ppzTracks[nLoop])
                free(sEntry.ppzTracks[nLoop]);
        }

        free(sEntry.ppzTracks);
    }

    if (sEntry.nTracks != nNumTracks)
        sEntry.nTracks = nNumTracks;

    sEntry.ppzTracks = (char**) malloc(nNumTracks*sizeof(char*));
    for(nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++)
        sEntry.ppzTracks[nLoop] = strdup(ppzTracks[nLoop]);
}


void CDDBFreeTrackTitles(char** ppzTracks, int nTracks)
{
    int nLoop;

    for (nLoop = 0 ; nLoop < nTracks; nLoop ++)
        free(ppzTracks[nLoop]);
    free(ppzTracks);
}


void CDDBGetInfo(const char* pzID, char* pzKey, char** ppzRet)
{
    if (bDBInEditor && strcmp(zID, pzID)) {
		strcpy(zID, pzID);

        DebugPrintf("CDDBGetInfo: New disc, do query");

        CDDBDoQuery(FALSE, NULL, NULL);
	}
    
    if (!strcmp(pzKey, "ARTIST") && sEntry.pzArtist)
        AppendString(ppzRet, sEntry.pzArtist, -1);
    else if (!strcmp(pzKey, "TITLE") && sEntry.pzTitle)
        AppendString(ppzRet, sEntry.pzTitle, -1);
    else if (!strcmp(pzKey, "CATEGORY") && sEntry.zCategory[0])
        AppendString(ppzRet, sEntry.zCategory, -1);
    else if (!strcmp(pzKey, "ORDER") && sEntry.pzPlayorder)
        AppendString(ppzRet, sEntry.pzPlayorder, -1);
    else if (!strcmp(pzKey, "EXTD") && sEntry.pzExtD)
        AppendString(ppzRet, sEntry.pzExtD, -1);
    else if (!strcmp(pzKey, "NUMTRACKS")) {
        char szTmp[32];

        sprintf(szTmp, "%d", sEntry.nTracks);

        AppendString(ppzRet, szTmp, -1);
    }
    else if (!strncmp(pzKey, "EXTT", 4)) {
        int nNum;

        nNum = atoi(&pzKey[4]);

        if (sEntry.ppzTracksExt && nNum < sEntry.nTracks)
            AppendString(ppzRet, sEntry.ppzTracksExt[nNum], -1);
        else
            AppendString(ppzRet, "", -1);
    }
    else if (!strcmp(pzKey, "NUMPLAY") && sEntry.pzPlayorder) {
        char szTmp[32];

        // Count´the number of programmed tracks

        int nNum = 1;
        unsigned int nLoop;
        for (nLoop = 0 ; nLoop < strlen(sEntry.pzPlayorder) ; nLoop ++) {
            if (sEntry.pzPlayorder[nLoop] == ' ')
                nNum ++;
        }

        if (!sEntry.pzPlayorder[0])
            nNum = 0;

        sprintf(szTmp, "%d", nNum);

        AppendString(ppzRet, szTmp, -1);
    }
    else {
        DebugPrintf("CDDBGetInfo: Setting default empty string for %s", pzKey);

        AppendString(ppzRet, "", -1);
    }

	DebugPrintf("<- CDDBGetInfo: %s = '%s'", pzKey, *ppzRet);
}

void CDDBSetInfo(const char* pzID, char* pzKey, char* pzInfo)
{
	if (bDBInEditor && strcmp(zID, pzID)) {
		strcpy(zID, pzID);

		CDDBDoQuery(FALSE, NULL, NULL);
	}
    
    if (sEntry.pzDiscid)
        strlwr(sEntry.pzDiscid);
    strlwr(zID);

    if (!sEntry.pzDiscid || !strstr(sEntry.pzDiscid, pzID)) {
        if (sEntry.pzDiscid) {
            free(sEntry.pzDiscid);
            sEntry.pzDiscid = NULL;
        }

        AppendString(&sEntry.pzDiscid, pzID, -1);
    }

    if (!strcmp(pzKey, "ARTIST")) {
        if (sEntry.pzArtist)
            free(sEntry.pzArtist);

        sEntry.pzArtist = strdup(pzInfo);
    }
    else if (!strcmp(pzKey, "TITLE")) {
        if (sEntry.pzTitle)
            free(sEntry.pzTitle);

        sEntry.pzTitle = strdup(pzInfo);
    }
    else if (!strcmp(pzKey, "EXTD")) {
        if (sEntry.pzExtD)
            free(sEntry.pzExtD);

        sEntry.pzExtD = strdup(pzInfo);
    }
    else if (!strncmp(pzKey, "EXTT", 4)) {
        int nNum;

        nNum = atoi(&pzKey[4]);

        if (!sEntry.ppzTracksExt) {
            int nLoop;

            sEntry.ppzTracksExt = (char**) malloc(sEntry.nTracks*sizeof(char*));
            for(nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++)
                sEntry.ppzTracksExt[nLoop] = NULL;
        }
        if (sEntry.ppzTracksExt[nNum])
            free(sEntry.ppzTracksExt[nNum]);

        sEntry.ppzTracksExt[nNum] = strdup(pzInfo);
    }
    else if (!strcmp(pzKey, "CATEGORY"))
        strcpy(sEntry.zCategory, pzInfo);
    else if (!strcmp(pzKey, "ORDER")) {
        if (sEntry.pzPlayorder)
            free(sEntry.pzPlayorder);

        sEntry.pzPlayorder = strdup(pzInfo);
    }
}


void CDDBInit()
{
    memset(&sEntry, 0, sizeof(sEntry));

	pzGetNextIDCurrData = NULL;

    hIconLocal = LoadIcon(hMainInstance, MAKEINTRESOURCE(IDI_LOCAL));
    hIconRemote = LoadIcon(hMainInstance, MAKEINTRESOURCE(IDI_REMOTE));
}


void CDDBFree()
{
    // Free old stuff

    FreeEntry();

    if (pzReadData) {
        free(pzReadData);

		pzReadData = NULL;

        CloseHandle(hReadFile);
    }

    if (pnQueryFrames)
        free(pnQueryFrames);

    DestroyIcon(hIconLocal);
    DestroyIcon(hIconRemote);
}
