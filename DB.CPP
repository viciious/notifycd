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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>

#include "resource.h"

#include "ntfy_cd.h"
#include "db.h"
#include "cddb.h"
#include "misc.h"
#include "mci.h"

#define MAX_INFOSTRING     10*1024

char* pzTmpInfoStr;
FILE* fpDB;
BOOL bFoundInRemoteCDDB;
extern BOOL bDBInEditor;

/////////////////////////////////////////////////////////////////////
//
// DB STUFF! (CDPLAYER.INI and CDDB is included)
//
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//
// EXTERNAL FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

BOOL DBOpen()
{
    char zFileName[512];

    if (nOptions & OPTIONS_USECDDB) {
        DebugPrintf("-> DBOpen using CDDB");

        return CDDBOpen();
    }
    else {
        DebugPrintf("-> DBOpen using INI");

        GetWindowsDirectory(zFileName, 256);
        strcat(zFileName, "\\CDPLAYER.INI");

        fpDB = fopen(zFileName, "r");
        if (!fpDB)
            return FALSE;

		bDBInEditor = TRUE;
    }

    return TRUE;
}


void DBClose()
{
    DebugPrintf("<- DBClose");

    if (nOptions & OPTIONS_USECDDB)
        CDDBClose();

	bDBInEditor = FALSE;
}


BOOL DBSave()
{
    bFoundInRemoteCDDB = FALSE;

    if (nOptions & OPTIONS_USECDDB)
        return CDDBSave();
    else {
        DebugPrintf("-> DBSave using CDPLAYER.INI");

        return TRUE;
    }
}


BOOL DBInternetGet(HWND hWnd)
{
    if (CDDBInternetGet(hWnd))
        bFoundInRemoteCDDB = TRUE;
    else
        bFoundInRemoteCDDB = FALSE;

    return bFoundInRemoteCDDB;
}


void DBInternetSend(HWND hWnd)
{
    CDDBInternetSend(hWnd);
}


BOOL DBGetDBID(char* pzID, char** ppzArtist, char** ppzTitle, char** ppzCategory)
{
    if (nOptions & OPTIONS_USECDDB)
        return CDDBGetID(pzID, ppzArtist, ppzTitle, ppzCategory);
    else {
        char zStr[256];
        char* pzTmp;

        pzTmp = NULL;

        while (fgets(zStr, 256, fpDB)) {
            if (zStr[0] == '[') {
                strcpy(pzID, &zStr[1]);
                pzID[strlen(pzID) - 2] = 0;

                // Check for artist name, otherwise this CD is deleted
                DBGetInfo(pzID, pzID, "ARTIST", &pzTmp);
                if (pzTmp[0]) {
                    free(pzTmp);

	                DBGetInfo(pzID, pzID, "ARTIST", ppzArtist);
	                DBGetInfo(pzID, pzID, "TITLE", ppzTitle);
	                DBGetInfo(pzID, pzID, "CATEGORY", ppzCategory);

                    return TRUE;
                }

                free(pzTmp);
                pzTmp = NULL;
            }
        }

        return FALSE;
    }
}


BOOL DBIsEnd()
{
    if (nOptions & OPTIONS_USECDDB)
        return CDDBIsEnd();
    else {
        if (!feof(fpDB))
            return TRUE;
        else {
            DebugPrintf("Closing CDPLAYER.INI");

            fclose(fpDB);

            return FALSE;
        }
    }
}


void DBGetDiscID(char* pzCDDBID, char* pzMCIID, BOOL bQuery, BOOL* pbFoundLocal, BOOL* pbFoundRemote)
{
    char zID[32];

    DebugPrintf("-> DBGetDiscID");

    CDGetDiscID(pzMCIID);

    DBGetAlias(pzMCIID, zID);
    
    CDDBGetDiscID(pzCDDBID);

    if (!bQuery)
        return;

    bFoundInRemoteCDDB = FALSE;

    // If we are not using the CDDB (local) try to find the disc in CDPLAYER.INI
    if (!(nOptions & OPTIONS_USECDDB)) {
        char zStr[10];

        DebugPrintf("Checking for disc in INI file");

        *pbFoundLocal = *pbFoundRemote = FALSE;

        if (GetPrivateProfileSection(pzMCIID, zStr, 10, "CDPLAYER.INI")) {
            DebugPrintf("Found disc in INI file!");
            *pbFoundLocal = TRUE;
        }
        else
            DebugPrintf("Did *NOT* find disc in INI file!");
    }

    // If we are using the CDDB local database or have enabled remote query and we didn't find
    // the disc in CDPLAYER.INI, try to query the CDDB database
    if ((nOptions & OPTIONS_USECDDB) || ((nOptions & OPTIONS_QUERYREMOTE) && !*pbFoundLocal)) {
        DebugPrintf("Checking for disc using CDDB");

        if ((nOptions & OPTIONS_QUERYREMOTE) && !*pbFoundLocal && !(nOptions & OPTIONS_QUERYLOCAL))
            DebugPrintf("Doing a remote CDDB query only! (We are not using the local CDDB)");
        
        CDDBDoQuery(FALSE, pbFoundLocal, pbFoundRemote);

        if (*pbFoundRemote) {
            bFoundInRemoteCDDB = TRUE;

            DebugPrintf("Found the disc on the remote CDDB server");
        }
    }

    DebugPrintf("<- DBGetDiscID");
}


void DBDelete(const char* pzCDDBID, const char* pzMCIID)
{
    if (nOptions & OPTIONS_USECDDB)
        CDDBDelete(pzCDDBID);
    else
        WritePrivateProfileSection(pzMCIID, "\0\0", "CDPLAYER.INI");
}


void DBGetAlias(const char* pzCheckID, char* pzDestID)
{
    GetPrivateProfileString("ALIASES", pzCheckID, pzCheckID, pzDestID, 31, "CDPLAYER.INI");
}


int DBGetInfoInt(const char* pzCDDBID, const char* pzMCIID, char* pzKey)
{
    int nRet;
    char* pzStr;

    pzStr = NULL;

    DBGetInfo(pzCDDBID, pzMCIID, pzKey, &pzStr);
    
    nRet = atoi(pzStr);

    free(pzStr);

    return nRet;
}


void DBGetInfo(const char* pzCDDBID, const char* pzMCIID, char* pzKey, char** ppzRet)
{
    char zID[32];
    DWORD nLen = 0;

    if (*ppzRet) {
        free(*ppzRet);

        *ppzRet = NULL;
    }

    *pzTmpInfoStr = 0;

    if ((nOptions & OPTIONS_QUERYLOCAL) || bFoundInRemoteCDDB || bDBInEditor) {
        DBGetAlias(pzMCIID, zID);
    
        if ((nOptions & OPTIONS_USECDDB) || (bFoundInRemoteCDDB && !bDBInEditor)) {
            DebugPrintf("-> Getting %s from CDDB (ID %s). Remote = %d", pzKey, pzCDDBID, bFoundInRemoteCDDB);

            CDDBGetInfo(pzCDDBID, pzKey, ppzRet);

			if (ppzRet && *ppzRet)
				DebugPrintf("<- Getting %s from CDDB (ID %s). Remote = %d, Value = '%s'", pzKey, pzCDDBID, bFoundInRemoteCDDB, *ppzRet);
			else
				DebugPrintf("<- Getting %s from CDDB (ID %s). Remote = %d, Value = NULL", pzKey, pzCDDBID, bFoundInRemoteCDDB);
        }
        else {
            DebugPrintf("-> Getting %s from INI (ID %s)", pzKey, zID);

		    if (strcmp(pzKey, "EXTD") && strncmp(pzKey, "EXTT", 4)) {
			    nLen = GetPrivateProfileString(zID, pzKey, "", pzTmpInfoStr, MAX_INFOSTRING, "CDPLAYER.INI");
	            if (nLen)
				    *ppzRet = strdup(pzTmpInfoStr);
                else
                    *ppzRet = strdup("");
		    }
		    else {		// extd, exttN
			    char zKey[32];
			    int nLoop;

			    nLoop = 0;
			    while(nLoop >= 0) {
				    sprintf(zKey, "%s_%d", pzKey, nLoop);

				    nLen = GetPrivateProfileString(zID, zKey, "", pzTmpInfoStr, MAX_INFOSTRING, "CDPLAYER.INI");
				    if (nLen) {
					    AppendString(ppzRet, pzTmpInfoStr, -1);

					    nLoop ++;
				    }
                    else {
                        if (!*ppzRet)
                            *ppzRet = strdup("");

					    nLoop = -1;
                    }
			    }
		    }

            if (*ppzRet)
                DebugPrintf("-> Getting %s from INI (ID %s), Value = '%s'", pzKey, zID, *ppzRet);
            else
                DebugPrintf("-> Getting %s from INI (ID %s), Value = NULL", pzKey, zID);
        }     
    }
    else
        *ppzRet = strdup("");
}


char** DBGetTrackTitles(const char* pzCDDBID, const char* pzMCIID, int nNum)
{
    int nLoop;
    char** pppzTracks;
    char szTmp[32];
    char* pzStr = NULL;
    char zID[32];
    BOOL bGetFromINI = FALSE;

    DBGetAlias(pzMCIID, zID);
    
    if (!(nOptions & OPTIONS_QUERYLOCAL) && !bFoundInRemoteCDDB && !bDBInEditor) {
        DebugPrintf("<- Setting default track titles");

        pppzTracks = (char**) malloc(nNum * sizeof(char*));
        for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
            pppzTracks[nLoop] = (char*) malloc(64*sizeof(char));
		    pppzTracks[nLoop][0] = 0;
	    }

        for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
            sprintf(szTmp, "%d", nLoop);
            DBGetInfo(zID, zID, szTmp, &pzStr);

            if (pzStr[0])
                strcpy(pppzTracks[nLoop], pzStr);
            else
                sprintf(pppzTracks[nLoop], "Track %d", nLoop + 1);

            free(pzStr);
            pzStr = NULL;
        }

        return pppzTracks;
    }

    if ((nOptions & OPTIONS_USECDDB) || (bFoundInRemoteCDDB && !bDBInEditor)) {
        char** ppzTmp;
    
        DebugPrintf("<- Getting track titles from CDDB. Remote = %d", bFoundInRemoteCDDB);

        ppzTmp = CDDBGetTrackTitles(pzCDDBID, nNum);

        return ppzTmp;
    }
    else {
        DebugPrintf("<- Getting track titles from INI");

        DBGetInfo(zID, zID, "0", &pzStr);
        if (pzStr && *pzStr)
            bGetFromINI = TRUE;

        free(pzStr);

        pzStr = NULL;

        pppzTracks = (char**) malloc(nNum * sizeof(char*));
        for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
            sprintf(szTmp, "%d", nLoop);
            DBGetInfo(zID, zID, szTmp, &pzStr);

            if (pzStr[0])
				pppzTracks[nLoop] = strdup(pzStr);
            else {
				sprintf(szTmp, "Track %d", nLoop + 1);
                pppzTracks[nLoop] = strdup(szTmp);
			}

            free(pzStr);
            pzStr = NULL;
        }

        return pppzTracks;
    }
}


void DBSetInfoInt(const char* pzCDDBID, const char* pzMCIID, char* pzKey, int nNum)
{
    char zStr[80];

    sprintf(zStr, "%d", nNum);

    DBSetInfo(pzCDDBID, pzMCIID, pzKey, zStr);
}


void DBSetInfo(const char* pzCDDBID, const char* pzMCIID, char* pzKey, char* pzInfo)
{
    char zID[32];

    if (strlen(pzInfo) < 200)
        DebugPrintf("-> DBSetInfo() Key %s = '%s'", pzKey, pzInfo);
    else
        DebugPrintf("-> DBSetInfo() Key %s = (long data)", pzKey);

    if (!(nOptions & OPTIONS_STORELOCAL))
        return;

    DBGetAlias(pzMCIID, zID);
    
    // Always update the CDDB information kept in memory if we issue an InternetSend later on.
    // This does NOT save the information into a file, that is done by DBSave
    CDDBSetInfo(pzCDDBID, pzKey, pzInfo);

    if (!(nOptions & OPTIONS_USECDDB) || (nCDDBOptions & OPTIONS_CDDB_STORECOPYININI)) {
        if (strcmp(pzKey, "EXTD") && strncmp(pzKey, "EXTT", 4)) {
			if (!WritePrivateProfileString(zID, pzKey, pzInfo, "CDPLAYER.INI"))
                DebugPrintf("WritePrivateProfileString failed (error code %d)", GetLastError());
        }
		else {
			char zKey[32];
			char zStr[81];
			int nLoop;
			int nLen = strlen(pzInfo);
			int nPos = 0;

			nLoop = 0;
			while(nPos < nLen) {
				sprintf(zKey, "%s_%d", pzKey, nLoop);

				if (nLen - nPos > 80) {
					strncpy(zStr, &pzInfo[nPos], 80);
					zStr[80] = 0;

					nPos += 80;
				}
				else {
					strcpy(zStr, &pzInfo[nPos]);

					nPos = nLen;
				}
				
				WritePrivateProfileString(zID, zKey, zStr, "CDPLAYER.INI");
				
				nLoop ++;
			}
            
            sprintf(zKey, "%s_%d", pzKey, nLoop);
            WritePrivateProfileString(zID, zKey, "", "CDPLAYER.INI");
		}
	}
}


void DBSetTrackTitles(const char* pzCDDBID, const char* pzMCIID, char** pppzTracks, int nNum)
{
    char zID[32];

    DebugPrintf("-> DBSetTrackTitles()");

    // Always update the CDDB information kept in memory if we issue an InternetSend later on.
    // This does NOT save the information into a file, that is done by DBSave
    CDDBSetTrackTitles(pzCDDBID, pppzTracks, nNum);

    if (!(nOptions & OPTIONS_STORELOCAL))
        return;

    DBGetAlias(pzMCIID, zID);
    
    if (!(nOptions & OPTIONS_USECDDB) || (nCDDBOptions & OPTIONS_CDDB_STORECOPYININI)) {
        int nLoop;
        char szTmp[32];

        DBSetInfoInt(zID, zID, "NUMTRACKS", nNum);
        
        for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
            sprintf(szTmp, "%d", nLoop);
            DBSetInfo(zID, zID, szTmp, pppzTracks[nLoop]);
        }
    }
}


void DBFreeTrackTitles(char** pppzTracks, int nTracks)
{
    if ((nOptions & OPTIONS_USECDDB))
        CDDBFreeTrackTitles(pppzTracks, nTracks);
    else {
        int nLoop;

        for (nLoop = 0 ; nLoop < nTracks; nLoop ++)
            free(pppzTracks[nLoop]);
        free(pppzTracks);
    }
}


void DBInit()
{
    pzTmpInfoStr = (char*) malloc(MAX_INFOSTRING*sizeof(char));

    bFoundInRemoteCDDB = FALSE;

    CDDBInit();
}


void DBFree()
{
    free(pzTmpInfoStr);

    CDDBFree();
}

