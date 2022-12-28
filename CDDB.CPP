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
#include "mci.h"
#define SOCK
#include "misc.h"
#include "cddb.h"

extern GLOBALSTRUCT gs;

#define MAX_READ    204800

#define SOCKET_DEBUG        0
#define PROXY_DEBUG         0

BOOL OpenEntry(const char* pzPath);

// Connection stuff
BOOL OpenConnection(SOCKET* pnSocket,
					BOOL* pbCDDBConnected);
BOOL CloseConnection(SOCKET* pnSocket,
					 BOOL* pbCDDBConnected);
BOOL SendCommand(SOCKET* pnSocket, 
				 BOOL* pbCDDBConnected,
				 BOOL bNoHTTP, 
				 char* pzCommand, 
				 char* pzStr, 
				 unsigned int nLen);
BOOL GetResult(SOCKET* pnSocket,
			   BOOL* pbCDDBConnected,
			   char* pzStr, 
			   unsigned int nLen);
BOOL VerifyResult(SOCKET* pnSocket,
				  BOOL* pbCDDBConnected,
				  char* pzStr, 
				  unsigned int nCodes, ...);

// Wrappers for the query functions
BOOL QueryLocal(DISCINFO* psDI);
BOOL QueryRemote(DISCINFO* psDI,
				 BOOL* pbServerError);

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


// DB Editor stuff
BOOL bDBInEditor = FALSE;
WIN32_FIND_DATA sDBFindData;
int nDBCategory;
HANDLE hDBFind;

char zLastPathRead[MAX_PATH];
HANDLE hReadFile;
DWORD dwSizeOfFile;
char* pzReadData;
char* pzCurrData;
char* pzGetNextIDCurrData;
char zDBLastPath[MAX_PATH];

unsigned int nCDDBNumCategories = 11;
BOOL bInRemoteQuery = FALSE;

HICON hCDDBTrayIcon;
HICON hIconLocal;
HICON hIconRemote;

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

void FixEntry(DISCINFO* psDI)
{
    char* pzPtr;

    pzPtr = strstr(psDI->pzArtist, "/");
    if (pzPtr) {
        pzPtr[0] = 0;
        while(psDI->pzArtist[strlen(psDI->pzArtist)-1] == ' ')
            psDI->pzArtist[strlen(psDI->pzArtist)-1] = 0;
        pzPtr ++;
        while(*pzPtr == ' ')
            pzPtr ++;
        AppendString(&psDI->pzTitle, pzPtr, -1);
        while(psDI->pzTitle[strlen(psDI->pzTitle)-1] == ' ')
            psDI->pzTitle[strlen(psDI->pzTitle)-1] = 0;
    }
	else
		AppendString(&psDI->pzTitle, "", -1);

    if (psDI->pzOrder) {
        unsigned int nLoop;

        for(nLoop = 0 ; nLoop < strlen(psDI->pzOrder) ; nLoop ++) {
            if (psDI->pzOrder[nLoop] == ',')
                psDI->pzOrder[nLoop] = ' ';
        }
    }
}

void CheckNumTracks(DISCINFO* psDI, unsigned int nNum)
{
	if (nNum == psDI->nTracks) {
		psDI->nTracks ++;

		if (!psDI->ppzTracks) 
			psDI->ppzTracks = (char**)malloc(psDI->nTracks*sizeof(char*));
		else
			psDI->ppzTracks = (char**)realloc(psDI->ppzTracks, psDI->nTracks*sizeof(char*));

		if (!psDI->ppzTracksExt)
			psDI->ppzTracksExt = (char**)malloc(psDI->nTracks*sizeof(char*));
		else
			psDI->ppzTracksExt = (char**)realloc(psDI->ppzTracksExt, psDI->nTracks*sizeof(char*));

		psDI->ppzTracks[nNum] = NULL;
		psDI->ppzTracksExt[nNum] = NULL;
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


void ParseEntry(DISCINFO* psDI, char* pzData, DWORD dwSize) 
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

            if (psDI->pzDiscid) 
                AppendString(&psDI->pzDiscid, ",", -1);

            AppendString(&psDI->pzDiscid, &pzStr[7], -1);

            strlwr(psDI->zCDDBID);
            strlwr(psDI->pzDiscid);

            // If we are doing a remote query we accept everything as discid since the server might have
            // decided that this is a "close" match
            if (strstr(psDI->pzDiscid, psDI->zCDDBID) || bInRemoteQuery)
                bParseFound = TRUE;

            bDiscIdFound = TRUE;
        }
        else if (!strncmp(pzStr, "# Revision: ", 12))
            psDI->nRevision = atoi(&pzStr[12]);
        else if (!strncmp(pzStr, "# Submitted via: ", 12)) {
            if (psDI->pzSubmitted) {
                free(psDI->pzSubmitted);
                psDI->pzSubmitted = NULL;
            }
            AppendString(&psDI->pzSubmitted, &pzStr[17], -1);
        }
        else if (!strncmp(pzStr, "# Track frame offsets:", 22))
            bInCollectTrackFrames = TRUE;
        else if (!strncmp(pzStr, "# Disc length: ", 15)) {
            bInCollectTrackFrames = FALSE;

            psDI->nDiscLength = atoi(&pzStr[15]);
        }
        else if (bDiscIdFound && !bParseFound)
            return;
        else if (bInCollectTrackFrames) {
            int nStrPos = 1;

            while(pzStr[nStrPos] == ' ' || pzStr[nStrPos] == '\t')
                nStrPos ++;

            if (isdigit(pzStr[nStrPos])) {
                nCollectedTrackFrames ++;
            
                if (psDI->pnFrames)
                    psDI->pnFrames = (unsigned int*) realloc(psDI->pnFrames, nCollectedTrackFrames*sizeof(unsigned int));
                else
                    psDI->pnFrames = (unsigned int*) malloc(nCollectedTrackFrames*sizeof(unsigned int));

                psDI->pnFrames[nCollectedTrackFrames-1] = atoi(&pzStr[nStrPos]);
            }
            else
                bInCollectTrackFrames = FALSE;
        }

        if (bParseFound) {
            if (!strncmp(pzStr, "DTITLE=", 7)) {
                AppendString(&psDI->pzArtist, &pzStr[7], -1);
                bDTITLEFound = TRUE;
            }
            else {
                if (!strncmp(pzStr, "PLAYORDER=", 10))
                    AppendString(&psDI->pzOrder, &pzStr[10], -1);
                else if (!strncmp(pzStr, "TTITLE", 6)) {
                    int nNum;

                    pzPtr = strchr(&pzStr[6], '=');
                    pzPtr[0] = 0;
                    nNum = atoi(&pzStr[6]);

					CheckNumTracks(psDI, nNum);

			        AppendString(&psDI->ppzTracks[nNum], &pzPtr[1], -1);
                }
                else if (!strncmp(pzStr, "EXTD=", 5))
                    AppendString(&psDI->pzDiscExt, &pzStr[5], -1);
                else if (!strncmp(pzStr, "EXTT", 4)) {
                    int nNum;

                    pzPtr = strchr(&pzStr[4], '=');
                    pzPtr[0] = 0;
                    nNum = atoi(&pzStr[4]);

			        CheckNumTracks(psDI, nNum);

                    AppendString(&psDI->ppzTracksExt[nNum], &pzPtr[1], -1);
                }
            }
        }

        if (pzStr)
            free(pzStr);
    }

    return;
}


char* ParseData(DISCINFO* psDI, char* pzData, DWORD dwSize)
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

			ParseEntry(psDI, pzPtrStart, pzPtrEnd - pzPtrStart);

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

	    if (gs.cddb.nCDDBType == 2) // Unix
			DebugPrintf("Trying to open %s for caching (Unix format)", pzPath);
		else
			DebugPrintf("Trying to open %s for caching (Windows format)", pzPath);
        
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

		DebugPrintf("... ok, created cache for %s (%d)", pzPath, dwSizeOfFile);
	}

	return TRUE;
}


BOOL ReadEntry(DISCINFO* psDI,    
               const char* pzPath,
               const char* pzCategory)
{
    bParseFound = FALSE;
                              
	if (!OpenEntry(pzPath))
		return FALSE;

    AppendString(&psDI->pzCategory, pzCategory, -1);

	// Frist try from last position if this isn't the start position
	if (pzCurrData != pzReadData)
		pzCurrData = ParseData(psDI, pzCurrData, dwSizeOfFile  - (unsigned)(pzCurrData - pzReadData));

	if (!bParseFound)
		pzCurrData = ParseData(psDI, pzReadData, dwSizeOfFile);

    if (!bParseFound) {
        if (psDI->pzDiscid) {
            free(psDI->pzDiscid);

            psDI->pzDiscid = NULL;
        }

		if (psDI->pzCategory) {
			free(psDI->pzCategory);

			psDI->pzCategory = NULL;
		}

		return FALSE;
	}
    else {
        FixEntry(psDI);

        return TRUE;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Query Local database
//
//////////////////////////////////////////////////////////////////////
BOOL CDDBQueryLocal(DISCINFO* psDI)
{
    if (!bDBInEditor) {
        hCDDBTrayIcon = hIconLocal;
        NotifyAdd(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query");
    }
    
    psDI->bDiscFound = FALSE;

    if (((gs.nOptions & OPTIONS_QUERYLOCAL) || bDBInEditor) && (gs.nOptions & OPTIONS_USECDDB)) {
        if (!bDBInEditor) {
            hCDDBTrayIcon = hIconLocal;
            NotifyModify(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query local database");
        }

        if (QueryLocal(psDI))
            psDI->bDiscFound = TRUE;

        if (psDI->bDiscFound) {
            DebugPrintf("Found disc %s in category %s!", psDI->pzArtist, psDI->pzCategory);

			DebugPrintf("After disc found, psDI->nTracks = %d", psDI->nTracks);
        }
        else
            DebugPrintf("*NO* disc found in DB!");
    }

    if (!bDBInEditor)
        NotifyDelete(gs.hMainWnd, 200);

	if (!psDI->bDiscFound) {
		if (psDI->pzDiscid)
            free(psDI->pzDiscid);
		psDI->pzDiscid = NULL;

		AppendString(&psDI->pzDiscid, psDI->zCDDBID, -1);
	}
    else
        FixEntry(psDI);

	return psDI->bDiscFound;
}


BOOL QueryLocal(DISCINFO* psDI)
{
    char zPath[256];
    BOOL bFound = FALSE;

DebugPrintf("Query local");

    if (gs.cddb.nCDDBType == 2) {   // Unix
        unsigned int nLoop;

        for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++) {
            sprintf(zPath, "%s%s\\%s", gs.cddb.zCDDBPath, gs.ppzCategories[nLoop], psDI->zCDDBID);
            if (ReadEntry(psDI, zPath, gs.ppzCategories[nLoop]))
                return TRUE;
        }
    }   
    else if (gs.cddb.nCDDBType == 1) {     // Windows
        unsigned int nLoop;
        char zIDStart[3];
        unsigned int nIDStart;
        
        strncpy(zIDStart, psDI->zCDDBID, 2);
        zIDStart[2] = 0;

        sscanf(zIDStart, "%X", &nIDStart);

        for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++) {
            WIN32_FIND_DATA sFF;
            HANDLE hFind;
            char zCategory[20];
            char zFileIDStart[3];
            char zFileIDEnd[3];
            unsigned int nFileIDStart;
            unsigned int nFileIDEnd;

            // Loop through all files in all dirs to find the correct file to search!

            strcpy(zCategory, gs.ppzCategories[nLoop]);
            zCategory[8] = 0;

            sprintf(zPath, "%s%s\\*.*", gs.cddb.zCDDBPath, zCategory);

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
                        sprintf(zPath, "%s%s\\%s", gs.cddb.zCDDBPath, zCategory, sFF.cFileName);

                        DebugPrintf("Found file %s", zPath);

                        bFound = ReadEntry(psDI, zPath, gs.ppzCategories[nLoop]);

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
		send(s, "\r\n", 2, 0) == SOCKET_ERROR) {
        DebugPrintf("send() failed %d", WSAGetLastError());

        closesocket(s);
    
        shutdown(s, 0);

        return SOCKET_ERROR;
    }

    free(pzQPStr);

    return 1;
}

/////////////////////////////////////////////////////////////
//
// Convert a string to HTTP CGI format
//
/////////////////////////////////////////////////////////////
void ConvertSpace(char* pzStr)
{
    while(*pzStr) {
        if (*pzStr == ' ')
            *pzStr = '+';

        pzStr ++;
    }
}


/////////////////////////////////////////////////////////////
//
// Remove location and stuff from server name
//
/////////////////////////////////////////////////////////////
void GetServerName(char* zServer, char* zRemoteServer)
{
	DWORD nLoop;

	strcpy(zServer, zRemoteServer);
	for (nLoop = 0 ; nLoop < strlen(zServer) ; nLoop ++) {
		if (zServer[nLoop] == ',' || zServer[nLoop] == ' ') {
			zServer[nLoop] = 0;

			break;
		}
	}
}


/////////////////////////////////////////////////////////////
//
// Open a CDDB or HTTP connection
//
/////////////////////////////////////////////////////////////
BOOL OpenConnection(SOCKET* pnSocket,
					BOOL* pbCDDBConnected)
{
    char zStr[1024];
    struct sockaddr_in sAddrServer;
	char zServer[256];

	if (!(gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP) && *pbCDDBConnected)
		return TRUE;

	// Make a nice disconnect of the socket if we used HTTP for the last query and 
	// we "think" we are still connected. Which we aren't since HTTP 1.0 only handles
	// one query per connection. 
	if ((gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP) && *pbCDDBConnected)
		CloseConnection(pnSocket, pbCDDBConnected);

	// Get the "real" servername. I.e. remove the location stuff etc
	GetServerName(zServer, gs.cddb.zRemoteServer);

	// Create socket   
	*pnSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (*pnSocket == INVALID_SOCKET) {
		DebugPrintf("socket() failed %d", WSAGetLastError());

		return FALSE;
	}

	if (!(gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP)) {
    	DebugPrintf("Opening CDDB connection");

        // Set up remote address

	    sAddrServer.sin_family = AF_INET;
	    sAddrServer.sin_port = htons((short)gs.cddb.nRemotePort);
	    sAddrServer.sin_addr.s_addr = GetAddress(zServer);
        if (sAddrServer.sin_addr.s_addr == INADDR_NONE) {
  		    MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK | MB_ICONERROR);

            CloseConnection(pnSocket, pbCDDBConnected);

			return FALSE;
        }

	    if (connect(*pnSocket, (struct sockaddr*)&sAddrServer, sizeof(sAddrServer))) {
		    MessageBox(NULL, "Failed to connect to remote server!", APPNAME, MB_OK | MB_ICONERROR);

		    DebugPrintf("connect() failed %d", WSAGetLastError());

			CloseConnection(pnSocket, pbCDDBConnected);

		    return FALSE;
	    }

		DebugPrintf("Connected to %s", zServer);

		//
	    // Get server sign-on and send handshake!
	    //

	    Sleep(200);
	    
	    if (!GetResult(pnSocket, pbCDDBConnected, zStr, 1024))
		    return FALSE;
	    // Check for valid sign-on status
	    if (strncmp(zStr, "200", 3) && strncmp(zStr, "201", 3)) {
		    DebugPrintf("Remote server denies clients for the moment");
		    MessageBox(NULL, "Remote server denies clients for the moment", APPNAME, MB_OK | MB_ICONERROR);

			CloseConnection(pnSocket, pbCDDBConnected);

		    return FALSE;
	    }

		DebugPrintf("Connection ok!");
	}
	else {
		DebugPrintf("Opening HTTP connection");

		if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY) {
			sAddrServer.sin_port = htons((short)gs.cddb.nRemoteProxyPort);

    	    sAddrServer.sin_addr.s_addr = GetAddress(gs.cddb.zRemoteProxyServer);
            if (sAddrServer.sin_addr.s_addr == INADDR_NONE) {
    		    MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK | MB_ICONERROR);

				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
            }

			DebugPrintf("Using HTTP proxy on port %d", gs.cddb.nRemoteProxyPort);
		}
		else {
			sAddrServer.sin_port = htons((short)80);
    	    sAddrServer.sin_addr.s_addr = GetAddress(zServer);
            if (sAddrServer.sin_addr.s_addr == INADDR_NONE) {
    		    MessageBox(NULL, "Remote server not found!", APPNAME, MB_OK | MB_ICONERROR);

				CloseConnection(pnSocket, pbCDDBConnected);

		        return FALSE;
            }
		}

		// Set up remote address

		sAddrServer.sin_family = AF_INET;

		if (connect(*pnSocket, (struct sockaddr*)&sAddrServer, sizeof(sAddrServer))) {
			MessageBox(NULL, "Failed to connect to remote server!", APPNAME, MB_OK | MB_ICONERROR);

			DebugPrintf("connect() failed %d", WSAGetLastError());

			CloseConnection(pnSocket, pbCDDBConnected);

			return FALSE;
		}

		if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY)
			DebugPrintf("Connected to proxy %s", gs.cddb.zRemoteProxyServer);
		else
			DebugPrintf("Connected to %s", zServer);
	}        
    
	*pbCDDBConnected = TRUE;

	return TRUE;
}


/////////////////////////////////////////////////////////////
//
// Send a command to the server and return the result.
// This function also checks the HTTP answer if HTTP is used
//
/////////////////////////////////////////////////////////////
BOOL SendCommand(SOCKET* pnSocket, 
				 BOOL* pbCDDBConnected,
				 BOOL bNoHTTP, 
				 char* pzCommand, 
				 char* pzStr, 
				 unsigned int nLen)
{
    char zHello[100];
    char* pzPtr;
    char zTmp[256];
    char zStr[8192];

	// This is good to make sure we have the correct lenght supplied. It might
	// cause bad bugs later on if we don't do this.
	ZeroMemory(pzStr, nLen);

	if (bNoHTTP && (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP)) {
		*pzStr = 1;		// Fake valid reply
		
		return TRUE;
	}

	if (!OpenConnection(pnSocket, pbCDDBConnected))
		return FALSE;

    // Build hello
    pzPtr = NULL;
	if (gs.cddb.zEmailAddress[0]) {
		strcpy(zTmp, gs.cddb.zEmailAddress);
		pzPtr = strchr(zTmp, '@');
		if (pzPtr) {
			*pzPtr = ' ';

            sprintf(zHello, "%s NotifyCDPlayer(CDDB) %s", zTmp, VERSION);
		}
	}
	if (!pzPtr) {
		char szUserName[256];
		char szHostName[256];
		struct sockaddr_in sAddr;
		int nLen = sizeof(sAddr);

		strcpy(szHostName, "hostname");

		if (getsockname(*pnSocket, (sockaddr*) &sAddr, &nLen) != SOCKET_ERROR)
			sprintf(szHostName, "%d.%d.%d.%d", 
				sAddr.sin_addr.S_un.S_un_b.s_b1,
				sAddr.sin_addr.S_un.S_un_b.s_b2,
				sAddr.sin_addr.S_un.S_un_b.s_b3,
				sAddr.sin_addr.S_un.S_un_b.s_b4);

		if (gethostname(szUserName, 256) == SOCKET_ERROR)
			strcpy(szUserName, "username");

		sprintf(zHello, "%s %s NotifyCDPlayer(CDDB) %s", 
				szUserName, szHostName, VERSION);
	}
	
	if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP) {
        if (pzCommand)
			ConvertSpace(pzCommand);
	    ConvertSpace(zHello);
    }

	if (!pzCommand) {
		if (!(gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP)) {
			sprintf(zStr, "cddb hello %s", zHello);
			// Send handshake
			if (SendString(*pnSocket, zStr) == SOCKET_ERROR) {
				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
			}
		}
		else
			return TRUE;
	}
	else {
		if (!(gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP)) {
			if (SendString(*pnSocket, pzCommand) == SOCKET_ERROR) {
				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
			}
		}
		else {
			if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEPROXY) {
				char zServer[256];

				// Get the "real" servername. I.e. remove the location stuff etc
				GetServerName(zServer, gs.cddb.zRemoteServer);

                sprintf(zStr, "GET http://%s%s?cmd=%s&hello=%s&proto=3", zServer, gs.cddb.zRemoteHTTPPath, pzCommand, zHello);
			}
            else
                sprintf(zStr, "GET %s?cmd=%s&hello=%s&proto=3", gs.cddb.zRemoteHTTPPath, pzCommand, zHello);
            strcat(zStr, " HTTP/1.0");

		    // Send query
			if (SendString(*pnSocket, zStr) == SOCKET_ERROR) {
				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
			}
            if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEAUTHENTICATION) {
                if (SendAuthentication(*pnSocket) == SOCKET_ERROR) {
					CloseConnection(pnSocket, pbCDDBConnected);

					return FALSE;
				}
            }
            
            sprintf(zStr, "User-Agent: %s", gs.cddb.zUserAgent);
			if (SendString(*pnSocket, zStr) == SOCKET_ERROR) {
				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
			}
			if (SendString(*pnSocket, "") == SOCKET_ERROR) {
				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
			}
			if (SendString(*pnSocket, "") == SOCKET_ERROR) {
				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
			}
		}
	}

	if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_USEHTTP) {
		char zStr[256];

		if (!GetString(*pnSocket, zStr, 1024)) {
			CloseConnection(pnSocket, pbCDDBConnected);

			return FALSE;
		}

		// Check for valid handshake status
		if (!strncmp(pzStr, "HTTP/", 5) && !strncmp(&pzStr[9], "200", 3)) {
			DebugPrintf("Server returned %s", pzStr);

			CloseConnection(pnSocket, pbCDDBConnected);

			return FALSE;
		}

		do {
			if (!GetString(*pnSocket, zStr, 1024)) {
				CloseConnection(pnSocket, pbCDDBConnected);

				return FALSE;
			}
		} while(zStr[0] != 0 && zStr[0] != '\r' && zStr[0] != '\n');
	}

	if (!GetResult(pnSocket, pbCDDBConnected, pzStr, nLen))
		return FALSE;

	return TRUE;
}


/////////////////////////////////////////////////////////////
//
// Get the result string from the server
//
/////////////////////////////////////////////////////////////
BOOL GetResult(SOCKET* pnSocket,
			   BOOL* pbCDDBConnected,
			   char* pzStr, 
			   unsigned int nLen)
{
    // Get handshake result    
	if (!GetString(*pnSocket, pzStr, nLen)) {
		CloseConnection(pnSocket, pbCDDBConnected);

		return FALSE;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////
//
// Verify the result code
//
/////////////////////////////////////////////////////////////
BOOL VerifyResult(SOCKET* pnSocket,
				  BOOL* pbCDDBConnected,
				  char* pzStr, 
				  unsigned int nCodes, ...)
{
	char* pzCode;
    BOOL bOk = FALSE;
    va_list ap;

    // In this case it was a command marked as not HTTP so we let the logic think it 
	// was sent and that the result was ok.
	if (pzStr[0] == 1)
		return TRUE;

    va_start(ap, nCodes);

    while (nCodes --) {
        pzCode = va_arg(ap, char*);
        // Check for valid handshake status
	    if (!strncmp(pzStr, pzCode, strlen(pzCode)))
            bOk = TRUE;
    }

    va_end(ap);

    if (!bOk) {
        DebugPrintf("Server returned %s", pzStr);

		CloseConnection(pnSocket, pbCDDBConnected);

		return FALSE;
    }
    else
        return TRUE;
}


/////////////////////////////////////////////////////////////
//
// Close the connection
//
/////////////////////////////////////////////////////////////
BOOL CloseConnection(SOCKET* pnSocket,
					 BOOL* pbCDDBConnected)
{
	DebugPrintf("Closing connection");

	*pbCDDBConnected = FALSE;

	closesocket(*pnSocket);

	shutdown(*pnSocket, 0);
	*pnSocket = 0;

	return TRUE;
}

/////////////////////////////////////////////////////////////
//
// Query the remote server for a disc
//
/////////////////////////////////////////////////////////////
BOOL CDDBQueryRemote(DISCINFO* psDI,
					 BOOL bManual,
					 BOOL* pbServerError)
{
    if (!bDBInEditor) {
        hCDDBTrayIcon = hIconLocal;
        NotifyAdd(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query");
    }
    
    psDI->bDiscFound = FALSE;

	*pbServerError = FALSE;

    if (!bManual)
        DebugPrintf("Auto query");
    else
        DebugPrintf("Manual query");

    if (((gs.nOptions & OPTIONS_QUERYREMOTE) || bManual) && !bDBInEditor) {
		WSADATA sData;

        hCDDBTrayIcon = hIconRemote;
        NotifyModify(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query remote server");

		*pbServerError = TRUE;

		if (!RASConnect())
			DebugPrintf("Remote query cancelled due to RASConnect returning FALSE");
		else {
			if (WSAStartup(0x0101, &sData)) {
				MessageBox(NULL, "Failed to initialize winsock!", APPNAME, MB_OK | MB_ICONERROR);

				DebugPrintf("WSAStartup() failed!");
			}
			else {
				if (QueryRemote(psDI, pbServerError))
					psDI->bDiscFound = TRUE;
	
				WSACleanup();
			}

			RASDisconnect();
		}

        if (psDI->bDiscFound)
            DebugPrintf("Found disc %s in category %s!", psDI->pzArtist, psDI->pzCategory);
        else
            DebugPrintf("*NO* disc found in DB!");
    }

    if (!bDBInEditor)
        NotifyDelete(gs.hMainWnd, 200);

	if (!psDI->bDiscFound) {
		if (psDI->pzDiscid)
            free(psDI->pzDiscid);
		psDI->pzDiscid = NULL;

		AppendString(&psDI->pzDiscid, psDI->zCDDBID, -1);
	}
    else
        FixEntry(psDI);

	return psDI->bDiscFound;
}


BOOL QueryRemote(DISCINFO* psDI,
				 BOOL* pbServerError)
{
    char zStr[8192];
    char zQuery[8192];
    char* pzPtr;
    char zTmpID[10];
	char zTmp[256];
	char zCmd[256];
    unsigned int nLoop;
    char zCategory[32];
    char* pzEntry = NULL;
    BOOL bEnd;
    BOOL bNoMOTD = FALSE;
	SOCKET nSocket;
    BOOL bConnected = FALSE;

	*pbServerError = TRUE;

    bParseFound = FALSE;

	DebugPrintf("Query remote");

    bNoMOTD = GetPrivateProfileInt("CDDB", "No_MOTD", 0, "CDPLAYER.INI");

#if PROXY_DEBUG    
    gs.cddb.nCDDBOptions |= OPTIONS_CDDB_USEPROXY;
#endif

    /////////////////////////////////////////////////////////////
    //
    // Build query string!
    //
    /////////////////////////////////////////////////////////////

	strcpy(zTmp, psDI->zCDDBID);
	sprintf(zQuery, "cddb query %s %d ", zTmp, psDI->nMCITracks);
	for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
		sprintf(zTmp, "%d ", psDI->pnFrames[nLoop]);
		strcat(zQuery, zTmp);
	}
	sprintf(zTmp, "%d", psDI->nDiscLength);
	strcat(zQuery, zTmp);

    /////////////////////////////////////////////////////////////
    //
    // Send hello
    //
    /////////////////////////////////////////////////////////////

    NotifyModify(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Connecting...");

	if (!SendCommand(&nSocket, &bConnected, TRUE, NULL, zStr, 8192))
		return FALSE;
	if (!VerifyResult(&nSocket, &bConnected, zStr, 2, "200", "201")) {
		DebugPrintf("Remote server denies clients for the moment");
		MessageBox(NULL, "Remote server denies clients for the moment", APPNAME, MB_OK | MB_ICONERROR);

		return FALSE;
	}

    /////////////////////////////////////////////////////////////
    //
    // Send proto
    //
    /////////////////////////////////////////////////////////////

	if (!SendCommand(&nSocket, &bConnected, TRUE, "proto 3", zStr, 8192))
		return FALSE;
	if (!VerifyResult(&nSocket, &bConnected, zStr, 1, "201"))
		return FALSE;

    /////////////////////////////////////////////////////////////
    //
    // Check message of the day!
    //
    /////////////////////////////////////////////////////////////

    if (!bNoMOTD) {
		char zLastMOTD[256];
		BOOL bDisplay = FALSE;

		if (!SendCommand(&nSocket, &bConnected, FALSE, "motd", zStr, 8192)) {
			// Reset password if this went wrong. This forces us to ask for it again the next time in case
			// we entered the wrong password. If motd works, we have a valid password
			if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)
				gs.cddb.zProxyPassword[0] = 0;

			return FALSE;
		}
		if (!VerifyResult(&nSocket, &bConnected, zStr, 1, "210"))
			return FALSE;

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
			if (!GetResult(&nSocket, &bConnected, zStr, 8192))
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
			DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_MOTD), GetForegroundWindow(), (DLGPROC)MOTDDlgProc);

		free(pzMOTD);
    }

    /////////////////////////////////////////////////////////////
	//
	// Send disc query and check results
	//
    /////////////////////////////////////////////////////////////

	NotifyModify(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Querying...");

	if (!SendCommand(&nSocket, &bConnected, FALSE, zQuery, zStr, 8192)) {
		// Reset password if this went wrong. This forces us to ask for it again the next time in case
		// we entered the wrong password. If motd works, we have a valid password
		if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)
			gs.cddb.zProxyPassword[0] = 0;

		return FALSE;
	}
	if (!VerifyResult(&nSocket, &bConnected, zStr, 3, "200", "202", "211"))
		return FALSE;

	//
	// Get query result
	//

	// No match!
	if (!strncmp(zStr, "202", 3)) {
		DebugPrintf("No match on remote server!", WSAGetLastError());

		*pbServerError = FALSE;

		SendCommand(&nSocket, &bConnected, TRUE, "quit", zStr, 1024);

		CloseConnection(&nSocket, &bConnected);

		return FALSE;
	}
	if (!strncmp(zStr, "211", 3)) {
		DebugPrintf("No exact match! Building list!", WSAGetLastError());

		poRemoteList = new cList<sRemoteEntry>;

		bEnd = FALSE;
		while(!bEnd) {
			if (!GetResult(&nSocket, &bConnected, zStr, 8192))
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

		if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_CHOOSEDISC), GetForegroundWindow(), (DLGPROC)ChooseDlgProc) == IDCANCEL) {
			SendCommand(&nSocket, &bConnected, TRUE, "quit", zStr, 1024);

			CloseConnection(&nSocket, &bConnected);

			*pbServerError = FALSE;

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

		strcpy(zTmp, psDI->zCDDBID);
	}

	strcpy(zTmpID, psDI->zCDDBID);
	strcpy(psDI->zCDDBID, zTmp);

    /////////////////////////////////////////////////////////////
	//
	// Read disc information
	//
    /////////////////////////////////////////////////////////////

	NotifyModify(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Reading...");

	sprintf(zCmd, "cddb read %s %s", zCategory, zTmp);

	if (!SendCommand(&nSocket, &bConnected, FALSE, zCmd, zStr, 8192))
		return FALSE;
	if (!VerifyResult(&nSocket, &bConnected, zStr, 1, "210"))
		return FALSE;

	bEnd = FALSE;
	while(!bEnd) {
		if (!GetResult(&nSocket, &bConnected, zStr, 8192))
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
        
        ParseEntry(psDI, pzEntry, strlen(pzEntry));

        bInRemoteQuery = FALSE;

        free(pzEntry);
    }
    
    AppendString(&psDI->pzCategory, zCategory, -1);
	
	strcpy(psDI->zCDDBID, zTmpID);
	if (bParseFound) {
        strlwr(psDI->zCDDBID);
        strlwr(psDI->pzDiscid);

		if (!strstr(psDI->pzDiscid, psDI->zCDDBID)) {
			// Add this disc id to the list since it was a faked id

			AppendString(&psDI->pzDiscid, ",", -1);
			AppendString(&psDI->pzDiscid, psDI->zCDDBID, -1);
		}
	}

   	SendCommand(&nSocket, &bConnected, TRUE, "quit", zStr, 1024);

	CloseConnection(&nSocket, &bConnected);
    
	*pbServerError = FALSE;

	return TRUE;
}


/////////////////////////////////////////////////////////////
//
// Query the remote server for site information
//
/////////////////////////////////////////////////////////////
BOOL QuerySites()
{
    char zStr[8192];
    unsigned int nCount = 0;
	SOCKET nSocket;
    BOOL bConnected = FALSE;

	DebugPrintf("Query sites");

    /////////////////////////////////////////////////////////////
    //
    // Send hello
    //
    /////////////////////////////////////////////////////////////

    NotifyModify(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Connecting...");

	if (!SendCommand(&nSocket, &bConnected, TRUE, NULL, zStr, 8192))
		return FALSE;
	if (!VerifyResult(&nSocket, &bConnected, zStr, 2, "200", "201")) {
		DebugPrintf("Remote server denies clients for the moment");
		MessageBox(NULL, "Remote server denies clients for the moment", APPNAME, MB_OK | MB_ICONERROR);

		return FALSE;
	}

    /////////////////////////////////////////////////////////////
    //
    // Send proto
    //
    /////////////////////////////////////////////////////////////

	if (!SendCommand(&nSocket, &bConnected, TRUE, "proto 3", zStr, 8192))
		return FALSE;
	if (!VerifyResult(&nSocket, &bConnected, zStr, 1, "201"))
		return FALSE;

    /////////////////////////////////////////////////////////////
    //
    // Get sites
    //
    /////////////////////////////////////////////////////////////

	if (!SendCommand(&nSocket, &bConnected, FALSE, "sites", zStr, 8192)) {
		// Reset password if this went wrong. This forces us to ask for it again the next time in case
		// we entered the wrong password. If motd works, we have a valid password
		if (gs.cddb.nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)
			gs.cddb.zProxyPassword[0] = 0;

		return FALSE;
	}

	if (!VerifyResult(&nSocket, &bConnected, zStr, 1, "210"))
		return FALSE;

    free(gs.cddb.psCDDBServers);
    gs.cddb.psCDDBServers = NULL;
    gs.cddb.nNumCDDBServers = 0;

    do {
        if (!GetResult(&nSocket, &bConnected, zStr, 8192))
			return FALSE;

        if (zStr[0] != '.') {
            if (++nCount > gs.cddb.nNumCDDBServers) {
                if (!gs.cddb.psCDDBServers)
                    gs.cddb.psCDDBServers = (CDDB_SERVER*) malloc(nCount * sizeof(CDDB_SERVER));
                else
                    gs.cddb.psCDDBServers = (CDDB_SERVER*) realloc(gs.cddb.psCDDBServers, nCount * sizeof(CDDB_SERVER));
            }

            if (zStr[strlen(zStr) - 1] == '\r')
                zStr[strlen(zStr) - 1] = 0;

            ParseServerInfo(&gs.cddb.psCDDBServers[nCount-1], zStr);

            gs.cddb.nNumCDDBServers ++;
        }
    } while(zStr[0] != '.');

   	SendCommand(&nSocket, &bConnected, TRUE, "quit", zStr, 1024);

	CloseConnection(&nSocket, &bConnected);
    
	return TRUE;
}


BOOL CDDBQuerySites()
{
	WSADATA sData;

    hCDDBTrayIcon = hIconRemote;
    NotifyAdd(gs.hMainWnd, 200, hCDDBTrayIcon, "Notify CD Player - Query remote server");

	if (!RASConnect()) {
		DebugPrintf("QuerySites cancelled due to RASConnect returning FALSE");

		return FALSE;
	}

	if (WSAStartup(0x0101, &sData)) {
		MessageBox(NULL, "Failed to initialize winsock!", APPNAME, MB_OK | MB_ICONERROR);

		DebugPrintf("WSAStartup() failed!");
	}

	QuerySites();

    NotifyDelete(gs.hMainWnd, 200);

	WSACleanup();

	RASDisconnect();

	return TRUE;
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


void WriteEntry(DISCINFO* psDI, FILE* fp, BOOL bLocal)
{
    unsigned int nLoop;
    char* pzStr;
    char zName[80];

    DebugPrintf("Write entry");

    // Write header
    fputs("# xmcd CD database file\n", fp);
    fputs("# Copyright (C) 1995 Ti Kan\n", fp);
    fputs("#\n", fp);

    // Write track offsets
    fputs("# Track frame offsets:\n", fp);

    for (nLoop = 0 ; nLoop < psDI->nTracks ; nLoop ++) {
        if (psDI->pnFrames)
            fprintf(fp, "#\t%d\n", psDI->pnFrames[nLoop]);
        else
            fprintf(fp, "#\t%d\n", psDI->pnFrames[nLoop]);
    }

    // Write disc length
    fputs("#\n", fp);
    fprintf(fp, "# Disc length: %d seconds\n", psDI->nDiscLength);

    // Write revision and submitted via
    fputs("#\n", fp);
    fprintf(fp, "# Revision: %d\n", psDI->nRevision);
    if (bLocal && psDI->pzSubmitted[0])
        fprintf(fp, "# Submitted via: %s\n", psDI->pzSubmitted);
    else
        fprintf(fp, "# Submitted via: NotifyCDPlayer(CDDB) %s\n", VERSION);

    fputs("#\n", fp);

    // DISCID
    WriteLine(fp, "DISCID", psDI->pzDiscid);

    // DTITLE
    pzStr = (char*)malloc(strlen(psDI->pzArtist) + strlen(psDI->pzTitle) + 10);
    if (psDI->pzArtist && strlen(psDI->pzArtist) && psDI->pzTitle && strlen(psDI->pzTitle)) {
        strcpy(pzStr, psDI->pzArtist);
        strcat(pzStr, " / ");
        strcat(pzStr, psDI->pzTitle);
    }
    else if (psDI->pzArtist)
        strcpy(pzStr, psDI->pzArtist);
    else if (psDI->pzTitle)
        strcpy(pzStr, psDI->pzTitle);

    WriteLine(fp, "DTITLE", pzStr);

    free(pzStr);

    // TRACKS
    for (nLoop = 0 ; nLoop < psDI->nTracks ; nLoop ++) {
        sprintf(zName, "TTITLE%d", nLoop);
        if (psDI->ppzTracks && psDI->ppzTracks[nLoop])
            WriteLine(fp, zName, psDI->ppzTracks[nLoop]);
        else
            WriteLine(fp, zName, "");
    }
    // EXTD
    if (psDI->pzDiscExt)
        WriteLine(fp, "EXTD", psDI->pzDiscExt);
    else
        WriteLine(fp, "EXTD", "");
    // EXTT
    for (nLoop = 0 ; nLoop < psDI->nTracks ; nLoop ++) {
        sprintf(zName, "EXTT%d", nLoop);
        if (psDI->ppzTracksExt && psDI->ppzTracksExt[nLoop])
            WriteLine(fp, zName, psDI->ppzTracksExt[nLoop]);
        else
            WriteLine(fp, zName, "");
    }

    if (bLocal) {
        if (psDI->pzOrder) {
            for (nLoop = 0 ; nLoop < (signed)strlen(psDI->pzOrder) ; nLoop ++) {
                if (psDI->pzOrder[nLoop] == ' ')
                    psDI->pzOrder[nLoop] = ',';
            }

            WriteLine(fp, "PLAYORDER", psDI->pzOrder);

            for (nLoop = 0 ; nLoop < (signed)strlen(psDI->pzOrder) ; nLoop ++) {
                if (psDI->pzOrder[nLoop] == ',')
                    psDI->pzOrder[nLoop] = ' ';
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
 * Return:
 *	The integer disc ID.
 */

unsigned long
cddb_discid(unsigned char nTracks, unsigned int* pnMin, unsigned int* pnSec)
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


void CDDBGetDiscID(MCIDEVICEID wDeviceID,
                   DISCINFO* psDI)
{
    MCI_SET_PARMS sMCISet;
    unsigned int nLoop;
    unsigned int nMCITracks = CDGetTracks(wDeviceID);
    unsigned int* pnMin = NULL;
    unsigned int* pnSec = NULL;

    pnMin = (unsigned int*)malloc((nMCITracks+1)*sizeof(unsigned int));
    pnSec = (unsigned int*)malloc((nMCITracks+1)*sizeof(unsigned int));
    
    if (psDI->pnFrames)
        free(psDI->pnFrames);

    psDI->pnFrames = (unsigned int*)malloc((nMCITracks+1)*sizeof(unsigned int));

    DebugPrintf("-- New disc! -----------------------------------------------------");

    sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	NotifyMCISendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);

    for (nLoop = 0 ; nLoop < nMCITracks ; nLoop ++) 
        CDGetAbsoluteTrackPos(wDeviceID, nLoop+1, &psDI->pnFrames[nLoop], &pnMin[nLoop], &pnSec[nLoop]);

    CDGetEndFrame(wDeviceID, psDI, psDI->pnFrames[0], &pnMin[nLoop], &pnSec[nLoop]);

    sprintf(psDI->zCDDBID, "%08x", cddb_discid((unsigned char)nMCITracks, pnMin, pnSec));

    DebugPrintf("Disc ID = %s", psDI->zCDDBID);

    sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	NotifyMCISendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);

	strcpy(psDI->zCDDBID, psDI->zCDDBID);

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

BOOL CDDBSave(DISCINFO* psDI)
{
    DebugPrintf("DBSave()");

    if (!(gs.nOptions & OPTIONS_STORELOCAL)) {
        DebugPrintf("Store local disabled");

        MessageBox(NULL, "Store local disabled. Changes will not be saved", APPNAME, MB_OK | MB_ICONINFORMATION);

        return FALSE;
    }

    if (!psDI->pzCategory[0]) {
        DebugPrintf("Missing category");

        MessageBox(NULL, "You must specifiy a category in order to save in the CDDB format", APPNAME, MB_OK | MB_ICONINFORMATION);

        return FALSE;
    }

    if (gs.cddb.nCDDBType == 1) {            // Windows
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

        strncpy(zIDStart, psDI->zCDDBID, 2);
        zIDStart[2] = 0;

        sscanf(zIDStart, "%X", &nIDStart);

        // Loop through all files in all dirs to find the correct file to search!

        strcpy(zCategory, psDI->pzCategory);
        zCategory[8] = 0;

        sprintf(zPath, "%s%s", gs.cddb.zCDDBPath, zCategory);
        CreateDirectory(zPath, NULL);
        sprintf(zPath, "%s%s\\*.*", gs.cddb.zCDDBPath, zCategory);

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
                    sprintf(zPath, "%s%s\\%s", gs.cddb.zCDDBPath, zCategory, sFF.cFileName);

                    DebugPrintf("Found file %s", zPath);

                    bFileFound = TRUE;

                    fpIn = fopen(zPath, "r");
                    if (fpIn) {
                        char zStr[1025];
                        char zCompare[80];
                        BOOL bSkip = FALSE;

                        sprintf(zCompare, "#FILENAME=%s", psDI->zCDDBID);

                        fpOut = fopen(zTmpFileName, "w");
                        if (!fpOut) {
                            DebugPrintf("Failed to open temporary file! (%s)", zTmpFileName);

                            return FALSE;
                        }
                        
                        do {
                            if (fgets(zStr, 1024, fpIn)) {
                                if (strstr(zStr, zCompare)) {
                                    DebugPrintf("Found entry in file!");

                                    fprintf(fpOut, "#FILENAME=%s\n", psDI->zCDDBID);

                                    WriteEntry(psDI, fpOut, TRUE);

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

            fprintf(fpOut, "#FILENAME=%s\n", psDI->zCDDBID);
            WriteEntry(psDI, fpOut, TRUE);

            fclose(fpOut);
        }
        else if (!bWritten) {          // Create new
            DebugPrintf("Create new file!");

            sprintf(zPath, "%s%s", gs.cddb.zCDDBPath, zCategory);
            CreateDirectory(zPath, NULL);

            sprintf(zPath, "%s%s\\%sTO%s", gs.cddb.zCDDBPath, zCategory, zIDStart, zIDStart);
            DebugPrintf("Creating file %s", zPath);

            CloseEntry();
    
            fpOut = fopen(zPath, "w");
            if (!fpOut) {
                DebugPrintf("Failed to open %s", zPath);

                return FALSE;
            }
            else {
                fprintf(fpOut, "#FILENAME=%s\n", psDI->zCDDBID);
                WriteEntry(psDI, fpOut, TRUE);

                fclose(fpOut);
            }
        }
    }
    else if (gs.cddb.nCDDBType == 2) {       // Unix
        char zPath[256];
        FILE* fp;

        DebugPrintf("Unix format");

        sprintf(zPath, "%s%s", gs.cddb.zCDDBPath, psDI->pzCategory);
        CreateDirectory(zPath, NULL);

        sprintf(zPath, "%s%s\\%s", gs.cddb.zCDDBPath, psDI->pzCategory, psDI->zCDDBID);

        DebugPrintf("Writing file %s", zPath);

        CloseEntry();

        fp = fopen(zPath, "w");
        if (fp) {
            WriteEntry(psDI, fp, TRUE);

            fclose(fp);
        }
    }

    return TRUE;
}


BOOL CDDBInternetGet(DISCINFO* psDI, HWND hWnd)
{
	BOOL bServerError;

    if (!strlen(gs.cddb.zRemoteServer) || !gs.cddb.nRemotePort) {
        MessageBox(hWnd, "You must configure a remote server to use this function!", APPNAME, MB_OK | MB_ICONINFORMATION);

        return FALSE;
    }

    // Do a remote query!

    CDDBQueryRemote(psDI, TRUE, &bServerError);

    return psDI->bDiscFound;
}


void InternetSend(MCIDEVICEID wDeviceID, DISCINFO* psDI, HWND hWnd);

void CDDBInternetSend(MCIDEVICEID wDeviceID, 
                      DISCINFO* psDI, 
                      HWND hWnd)
{
	WSADATA sData;

    if (MessageBox(hWnd, "Are You sure You want to send this entry to the CDDB repository?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDNO)
        return;

	if (!RASConnect()) {
		DebugPrintf("InternetSend cancelled due to RASConnect returning FALSE");

		return;
	}

	if (WSAStartup(0x0101, &sData)) {
		MessageBox(NULL, "Failed to initialize winsock!", APPNAME, MB_OK | MB_ICONERROR);

		DebugPrintf("WSAStartup() failed!");
	}

	InternetSend(wDeviceID, psDI, hWnd);

    WSACleanup();

	RASDisconnect();
}


void InternetSend(MCIDEVICEID wDeviceID, 
                  DISCINFO* psDI, 
                  HWND hWnd)
{
    char zTmpFileName[255];
    char zTmp[80];
    unsigned int nLoop;
    FILE* fp;
    BOOL bNoMIME = FALSE;
    char zToAddress[80];
    DISCINFO sDI;
	SYSTEMTIME sTime;
	char zDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char zMonth[][4] = {"Jan", "Feb", "Mar", "Apr", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
						
    DebugPrintf("DBInternetSend()");	
    
    for (nLoop = 0 ; nLoop < nCDDBNumCategories ; nLoop ++) {
        if (!stricmp(psDI->pzCategory, azCategories[nLoop]))
            break;
    }
    if (nLoop == nCDDBNumCategories) {
    	DebugPrintf("Illegal category '%s'", psDI->pzCategory);

		MessageBox(NULL, "Invalid category for submitting to the CDDB repository!", APPNAME, MB_OK | MB_ICONERROR);
        return;
    }

    if (strchr(psDI->pzArtist, '/') || strchr(psDI->pzArtist, ',')) {
    	DebugPrintf("Illegal character in artist name");

		MessageBox(NULL, "An illegal (/ or ,) character was found in the artist field!", APPNAME, MB_OK | MB_ICONERROR);
        return;
    }
    if (strchr(psDI->pzTitle, '/') || strchr(psDI->pzTitle, ',')) {
    	DebugPrintf("Illegal character in title name");

		MessageBox(NULL, "An illegal (/ or ,) character was found in the title field!", APPNAME, MB_OK | MB_ICONERROR);
        return;
    }
    if ((!psDI->pzArtist || !strlen(psDI->pzArtist)) && (!psDI->pzTitle || !strlen(psDI->pzTitle))) {
    	DebugPrintf("Both artist and title is empty");

		MessageBox(NULL, "You cannot leave both artist and title empty!", APPNAME, MB_OK | MB_ICONERROR);
        return;
    }

    if (!psDI->pzDiscid || !strlen(psDI->pzDiscid) || !strlen(psDI->zCDDBID)) {
        DebugPrintf("Error! Discid is empty!");
        return;
    }

    strlwr(psDI->zCDDBID);
    if (psDI->pzDiscid)
        strlwr(psDI->pzDiscid);

    if (psDI->pzDiscid && !strstr(psDI->pzDiscid, psDI->zCDDBID)) {
        free(psDI->pzDiscid);
        psDI->pzDiscid = NULL;

        AppendString(&psDI->pzDiscid, psDI->zCDDBID, -1); 
    }
    
    // Copy the information
    ZeroMemory(&sDI, sizeof(DISCINFO));
    CDDBGetDiscID(wDeviceID, &sDI);
	if (strcmp(sDI.zCDDBID, psDI->zCDDBID)) {
        unsigned int* pnMin = NULL;
        unsigned int* pnSec = NULL;

        sDI = *psDI;

        DebugPrintf("Discid in entry isn't the same as the current disc!");

        pnMin = (unsigned int*)malloc((psDI->nTracks+1)*sizeof(unsigned int));
        pnSec = (unsigned int*)malloc((psDI->nTracks+1)*sizeof(unsigned int));

        for (nLoop = 0 ; nLoop < psDI->nTracks ; nLoop ++) {
            pnMin[nLoop] = psDI->pnFrames[nLoop] / 75 / 60;
	        pnSec[nLoop] = (psDI->pnFrames[nLoop] / 75) % 60;
        }
        pnMin[nLoop] = (psDI->nDiscLength * 75 + 1) / 75 / 60;
	    pnSec[nLoop] = ((psDI->nDiscLength * 75 + 1) / 75)% 60;

        sprintf(sDI.zCDDBID, "%08x", cddb_discid((unsigned char)psDI->nTracks, pnMin, pnSec));
    }
    else {
        DISCINFO sTmpDI;

        sTmpDI = sDI;

        sDI = *psDI;

        sDI.nTracks = CDGetTracks(wDeviceID);
        sDI.nDiscLength = sTmpDI.nDiscLength;
        sDI.pnFrames = sTmpDI.pnFrames;
    }

    sDI.pzDiscid = strdup(sDI.zCDDBID);

    DebugPrintf("Send DiscID = %s", sDI.zCDDBID);

    if (!stricmp(psDI->pzArtist, "New Artist") || !stricmp(psDI->pzTitle, "New Title")) {
		DebugPrintf("Bad Artist or Title entries");

		MessageBox(NULL, "Illegal artist or title value", APPNAME, MB_OK | MB_ICONERROR);

		return;
	}

	for (nLoop = 0 ; nLoop < psDI->nTracks ; nLoop ++) {
        if (psDI->ppzTracks[nLoop] && !strnicmp(psDI->ppzTracks[nLoop], "Track ", 6)) {
			DebugPrintf("Bad track name");

			MessageBox(NULL, "Illegal track name", APPNAME, MB_OK | MB_ICONERROR);

			return;
		}
	}

	GetTmpFile(zTmpFileName);

    fp = fopen(zTmpFileName, "w");
    if (!fp) {
        DebugPrintf("Couldn't open temporary file! (%s)", zTmpFileName);

        MessageBox(hWnd, "Couldn't create temp file", APPNAME, MB_OK | MB_ICONERROR);

        return;
    }

    sDI.nRevision ++;
    psDI->nRevision ++;

    WriteEntry(&sDI, fp, FALSE);

    if (sDI.pnFrames != psDI->pnFrames)
        free(sDI.pnFrames);

    fclose(fp);

    //////////////////////////////////////////////////////////////////////
    //
    // Do SMTP comms!
    //
    //////////////////////////////////////////////////////////////////////

    SOCKET s;
    struct sockaddr_in sAddrServer;
    char zStr[8192];
    char zHostName[256];

	DebugPrintf("Send SMTP");

    if (gethostname(zHostName, 255) == SOCKET_ERROR) {
        DebugPrintf("gethostname() failed!");

        MessageBox(hWnd, "Couldn't get local hostname!", APPNAME, MB_OK | MB_ICONERROR);

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
    sAddrServer.sin_addr.s_addr = GetAddress(gs.cddb.zRemoteEmailServer);
    if (sAddrServer.sin_addr.s_addr == INADDR_NONE) {
        shutdown(s, 0);

        goto send_end;
    }
    
    if (connect(s, (struct sockaddr*)&sAddrServer, sizeof(sAddrServer))) {
        MessageBox(hWnd, "Failed to connect to remote server!", APPNAME, MB_OK | MB_ICONERROR);

        DebugPrintf("connect() failed %d", WSAGetLastError());

        shutdown(s, 0);

        goto send_end;
    }
        
DebugPrintf("Connected to %s", gs.cddb.zRemoteEmailServer);

    //
    // Get server sign-on and send handshake!
    //

    Sleep(200);
	
    if (!GetString(s, zStr, 1024))
        goto send_end;
    // Check for valid sign-on status      
    if (strncmp(zStr, "220", 3)) {
        DebugPrintf("Server not ready");

        MessageBox(hWnd, "Server not ready", APPNAME, MB_OK | MB_ICONERROR);

        closesocket(s);
    
        shutdown(s, 0);

        goto send_end;
    }
	int nTmp;
	nTmp = gs.cddb.nRemoteTimeout;
	gs.cddb.nRemoteTimeout = 2;
    GetString(s, zStr, 1024, TRUE);        // FOR ESMTP, do not return if failed!
	gs.cddb.nRemoteTimeout = nTmp;

    // Send HELO
    if (gs.cddb.zDomain[0])
        sprintf(zStr, "HELO %s.%s", zHostName, gs.cddb.zDomain);
    else
        sprintf(zStr, "HELO %s", zHostName);
    if (SendString(s, zStr) == SOCKET_ERROR)
        return;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "250", 3)) {
        DebugPrintf("HELO answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied access after HELO", APPNAME, MB_OK | MB_ICONERROR);

        goto send_end;
    }
    // Send MAIL FROM
    sprintf(zStr, "MAIL FROM:<%s>", gs.cddb.zEmailAddress);
    if (SendString(s, zStr) == SOCKET_ERROR)
        return;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "250", 3)) {
        DebugPrintf("MAIL FROM answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied MAIL FROM address", APPNAME, MB_OK | MB_ICONERROR);

        goto send_end;
    }
    // Send RCPT TO
#ifdef _DEBUG
    if (MessageBox(NULL, "Do You want to make a test submission?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
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

        MessageBox(hWnd, "Server denied RCPT TO address", APPNAME, MB_OK | MB_ICONERROR);

        goto send_end;
    }
    // Send DATA
    if (SendString(s, "DATA") == SOCKET_ERROR)
        goto send_end;
    if (!GetString(s, zStr, 1024))
        goto send_end;
    if (strncmp(zStr, "354", 3)) {
        DebugPrintf("DATA answer not ok %s", zStr);

        MessageBox(hWnd, "Server denied DATA", APPNAME, MB_OK | MB_ICONERROR);

        goto send_end;
    }

    // Send message
    sprintf(zStr, "From: %s", gs.cddb.zEmailAddress);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    sprintf(zStr, "To: %s", zToAddress);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
	// Build date string
	GetSystemTime(&sTime);
	sprintf(zStr, "Date: %s, %d %s %02d %02d:%02d:%02d GMT",
			zDay[sTime.wDayOfWeek], sTime.wDay, zMonth[sTime.wMonth-1], sTime.wYear - 1900, 
			sTime.wHour, sTime.wMinute, sTime.wSecond);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    strcpy(zTmp, psDI->pzCategory);
    sprintf(zStr, "Subject: cddb %s %s", strlwr(zTmp), psDI->zCDDBID);
    if (SendString(s, zStr) == SOCKET_ERROR)
        goto send_end;
    if (SendString(s, "X-Cddbd-Note: Sent by Notify CD Player - Questions: mlt@cyberdude.com") == SOCKET_ERROR)
        goto send_end;

    bNoMIME = GetPrivateProfileInt("CDDB", "No_MIME", 0, "CDPLAYER.INI");

    if (!bNoMIME) {
        SendString(s, "MIME-Version: 1.0");
        SendString(s, "Content-Type: text/plain; charset=iso-8859-1");
        SendString(s, "Content-Transfer-Encoding: quoted-printable");
    }

    // Send last newline after headers
    if (SendString(s, "") == SOCKET_ERROR)
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

        MessageBox(hWnd, "Server denied DATA end", APPNAME, MB_OK | MB_ICONERROR);

        goto send_end;
    }

    // Send QUIT
    if (SendString(s, "QUIT") == SOCKET_ERROR) 
        goto send_end;

    MessageBox(hWnd, "Entry submitted!", APPNAME, MB_OK | MB_ICONINFORMATION);

send_end:
    closesocket(s);

    shutdown(s, 0);

    fclose(fp);

    DeleteFile(zTmpFileName);
}


void CDDBDelete(DISCINFO* psDI)
{   
    DebugPrintf("CDDBDelete()");

    if (!(gs.nOptions & OPTIONS_STORELOCAL)) {
        DebugPrintf("Store local disabled");

        return;
    }

    if (!psDI->pzCategory[0]) {
        DebugPrintf("Missing category");

        return;
    }

    if (gs.cddb.nCDDBType == 1) {            // Windows
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

        strncpy(zIDStart, psDI->zCDDBID, 2);
        zIDStart[2] = 0;

        sscanf(zIDStart, "%X", &nIDStart);

        // Loop through all files in all dirs to find the correct file to search!

        strcpy(zCategory, psDI->pzCategory);
        zCategory[8] = 0;

        sprintf(zPath, "%s%s", gs.cddb.zCDDBPath, zCategory);
        CreateDirectory(zPath, NULL);
        sprintf(zPath, "%s%s\\*.*", gs.cddb.zCDDBPath, zCategory);

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
                    sprintf(zPath, "%s%s\\%s", gs.cddb.zCDDBPath, zCategory, sFF.cFileName);

                    DebugPrintf("Found file %s", zPath);

                    bFileFound = TRUE;

                    fpIn = fopen(zPath, "r");
                    if (fpIn) {
                        char zStr[1025];
                        char zCompare[80];
                        BOOL bSkip = FALSE;
                        HANDLE hFile;

                        sprintf(zCompare, "#FILENAME=%s", psDI->zCDDBID);

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
    else if (gs.cddb.nCDDBType == 2) {       // Unix
        char zPath[256];

        DebugPrintf("Unix format");

        sprintf(zPath, "%s%s", gs.cddb.zCDDBPath, psDI->pzCategory);
        CreateDirectory(zPath, NULL);

        sprintf(zPath, "%s%s\\%s", gs.cddb.zCDDBPath, psDI->pzCategory, psDI->zCDDBID);

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

    return TRUE;
}


void CDDBClose()
{
	bDBInEditor = FALSE;

    CloseEntry();

	pzGetNextIDCurrData = NULL;
}


BOOL FindNextID(char* zID)
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

		strncpy(zID, pzPtr, pzEndPtr - pzPtr);
		zID[pzEndPtr - pzPtr] = 0;
		if (zID[pzEndPtr - pzPtr - 1] == '\r')
			zID[pzEndPtr - pzPtr - 1] = 0;

        return TRUE;
    }

	pzGetNextIDCurrData = NULL;

    return FALSE;
}


BOOL CDDBGetID(DISCINFO* psDI, char* zID)
{
    BOOL bFound = FALSE;
    char zCategory[12];
    char zPath[MAX_PATH];
    char zName[MAX_PATH];
    
	if (gs.cddb.nCDDBType == 2) {   // Unix
start_again_unix:
		while (hDBFind == INVALID_HANDLE_VALUE) {
			nDBCategory ++;
			
			if (nDBCategory == (signed)gs.nNumCategories) {
				bIsEnd = TRUE;

				DebugPrintf("Last category searched");

				return FALSE;
			}

			DebugPrintf("Next category is %s", gs.ppzCategories[nDBCategory]);

            sprintf(zPath, "%s%s\\*", gs.cddb.zCDDBPath, gs.ppzCategories[nDBCategory]);

			hDBFind = FindFirstFile(zPath, &sDBFindData);
		}

		if (hDBFind != INVALID_HANDLE_VALUE) {
			do {
				strlwr(sDBFindData.cFileName);

                if (!(sDBFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !strstr(sDBFindData.cFileName, "to")) {
					bFound = TRUE;

					bIsEnd = FALSE;

					strcpy(zID, sDBFindData.cFileName);

					DebugPrintf("Found file %s with attributes %d", sDBFindData.cFileName, 
								sDBFindData.dwFileAttributes);

		            sprintf(zPath, "%s%s\\%s", gs.cddb.zCDDBPath, gs.ppzCategories[nDBCategory], zID);
                    
                    strcpy(psDI->zCDDBID, zID);

                    ReadEntry(psDI, zPath, gs.ppzCategories[nDBCategory]);
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
    else if (gs.cddb.nCDDBType == 1) {     // Windows
start_again_windows:
        if (!pzGetNextIDCurrData) {
            while (hDBFind == INVALID_HANDLE_VALUE) {
				nDBCategory ++;
			    
			    if (nDBCategory == (signed)gs.nNumCategories) {
				    bIsEnd = TRUE;

					DebugPrintf("Last category searched");

				    return FALSE;
			    }

				DebugPrintf("Next category is %s", gs.ppzCategories[nDBCategory]);

                strcpy(zCategory, gs.ppzCategories[nDBCategory]);
                zCategory[8] = 0;

                sprintf(zPath, "%s%s\\??TO??", gs.cddb.zCDDBPath, zCategory);

			    hDBFind = FindFirstFile(zPath, &sDBFindData);
		    }

		    if (hDBFind != INVALID_HANDLE_VALUE) {
			    do {
				    if (!(sDBFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					    bFound = TRUE;

					    bIsEnd = FALSE;

                        strcpy(zName, sDBFindData.cFileName);

						DebugPrintf("Found file %s with attributes %d", sDBFindData.cFileName, 
									sDBFindData.dwFileAttributes);
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
                strcpy(zCategory, gs.ppzCategories[nDBCategory]);
                zCategory[8] = 0;

                sprintf(zDBLastPath, "%s%s\\%s", gs.cddb.zCDDBPath, zCategory, zName);

				OpenEntry(zDBLastPath);

				pzGetNextIDCurrData = pzReadData;
            }
            else
                return FALSE;
        }

        if (!FindNextID(zID))
            goto start_again_windows;

		strcpy(psDI->zCDDBID, zID);

		ReadEntry(psDI, zDBLastPath, gs.ppzCategories[nDBCategory]);

        return TRUE;
    }

    return FALSE;
}


BOOL CDDBIsEnd()
{
    return !bIsEnd;
}


void CDDBInit()
{
	pzGetNextIDCurrData = NULL;

    hIconLocal = LoadIcon(gs.hMainInstance, MAKEINTRESOURCE(IDI_LOCAL));
    hIconRemote = LoadIcon(gs.hMainInstance, MAKEINTRESOURCE(IDI_REMOTE));
}


void CDDBFree()
{
    // Free old stuff

    if (pzReadData) {
        free(pzReadData);

		pzReadData = NULL;

        CloseHandle(hReadFile);
    }

    DestroyIcon(hIconLocal);
    DestroyIcon(hIconRemote);
}
