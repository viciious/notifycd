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
#include "res/resource.h"

#include "ntfy_cd.h"
#include "db.h"
#include "cddb.h"
#include "misc.h"
#include "mci.h"

extern GLOBALSTRUCT gs;

#define MAX_INFOSTRING     10*1024

char* pzTmpInfoStr;
FILE* fpDB;
BOOL bFoundInRemoteCDDB;
extern BOOL bDBInEditor;

void DBGetINIInfo(char* pzMCIID, char* pzKey, char** ppzRet);
int DBGetINIInfoInt(char* pzMCIID, char* pzKey);
void DBSetINIInfoInt(char* pzMCIID, char* pzKey, int nNum);
void DBSetINIInfo(char* pzMCIID, char* pzKey, char* pzInfo);

/////////////////////////////////////////////////////////////////////
//
// DB STUFF! (PROFILE and CDDB is included)
//
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//
// EXTERNAL FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

BOOL DBOpen()
{
    if (gs.nOptions & OPTIONS_USECDDB) {
        DebugPrintf("-> DBOpen using CDDB");

        return CDDBOpen();
    }
    else {
        DebugPrintf("-> DBOpen using PROFILE");

        fpDB = fopen(gs.szProfilePath, "r");
        if (!fpDB)
            return FALSE;
    }

    return TRUE;
}


void DBClose()
{
    DebugPrintf("<- DBClose");

    if (gs.nOptions & OPTIONS_USECDDB)
        CDDBClose();
}


void DBFixCDDBStrings(BOOL bToCDDB, 
                      DISCINFO* psDI)
{
    unsigned int nLoop;

    FixCDDBString(bToCDDB, &psDI->pzDiscExt);

    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
        if (psDI->ppzTracksExt[nLoop])
            FixCDDBString(bToCDDB, &psDI->ppzTracksExt[nLoop]);
    }
}


BOOL DBSave(DISCINFO* psDI)
{
    unsigned int nLoop;
    char szStr[32];

	if( !psDI->pzDiscid || !psDI->pzDiscid[0] ) {
		if( psDI->pzDiscid )
			delete[] psDI->pzDiscid;
		psDI->pzDiscid = StringCopy( psDI->zCDDBID );
	}

    DBFixCDDBStrings(TRUE, psDI);

    if (gs.nOptions & OPTIONS_USECDDB)
        CDDBScanFiles(psDI, MODE_CDDB_SAVE);

    if (!(gs.nOptions & OPTIONS_USECDDB) || (gs.cddb.nCDDBOptions & OPTIONS_CDDB_STORECOPYININI)) {
        DebugPrintf( "-> DBSave using %s", gs.szProfilePath );

        DBSetINIInfo(psDI->zMCIID, "entrytype", "1");
        DBSetINIInfo(psDI->zMCIID, "artist", psDI->pzArtist);
        DBSetINIInfo(psDI->zMCIID, "title", psDI->pzTitle);
        DBSetINIInfo(psDI->zMCIID, "category", psDI->pzCategory);
	    DBSetINIInfoInt(psDI->zMCIID, "numtracks", psDI->nMCITracks);

        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
            sprintf(szStr, "%d", nLoop);
            DBSetINIInfo(psDI->zMCIID, szStr, psDI->ppzTracks[nLoop]);
        }

        DBSetINIInfo(psDI->zMCIID, "extd", psDI->pzDiscExt);
        DBSetINIInfo(psDI->zMCIID, "order", psDI->pzOrder);
	    DBSetINIInfoInt(psDI->zMCIID, "numplay", psDI->nProgrammedTracks);

        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
		    sprintf(szStr, "extt%d", nLoop);
	        if (psDI->ppzTracksExt[nLoop])
                DBSetINIInfo(psDI->zMCIID, szStr, psDI->ppzTracksExt[nLoop]);
	    }          
    }

    // Ok, convert back!
    DBFixCDDBStrings(FALSE, psDI);

    return TRUE;
}


BOOL DBInternetGet(DISCINFO* psDI, HWND hWnd)
{
    if (CDDBInternetGet(psDI, hWnd))
        bFoundInRemoteCDDB = TRUE;
    else
        bFoundInRemoteCDDB = FALSE;

    return bFoundInRemoteCDDB;
}


void DBInternetSend(DISCINFO* psDI, HWND hWnd)
{
	unsigned int nLoop;

	// Convert the strings
    FixCDDBString(TRUE, &psDI->pzDiscExt);

    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
        if (psDI->ppzTracksExt[nLoop])
            FixCDDBString(TRUE, &psDI->ppzTracksExt[nLoop]);
    }

    CDDBInternetSend(gs.wDeviceID, psDI, hWnd);

    // Ok, convert back!
    FixCDDBString(FALSE, &psDI->pzDiscExt);

    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
        if (psDI->ppzTracksExt[nLoop])
            FixCDDBString(FALSE, &psDI->ppzTracksExt[nLoop]);
    }
}


BOOL DBIsEnd()
{
    if (gs.nOptions & OPTIONS_USECDDB)
        return CDDBIsEnd();
    else {
        if (!feof(fpDB))
            return TRUE;
        else {
            DebugPrintf( "Closing %s", gs.szProfilePath );

            fclose(fpDB);

            return FALSE;
        }
    }
}


BOOL DBGetDBID(char* pzID, DISCINFO* psDI)
{
    if (gs.nOptions & OPTIONS_USECDDB) {        
		if (CDDBGetID(psDI, pzID))
			return TRUE;
		return FALSE;
	}
    else {
        char zStr[256];
        char* pzTmp;

        pzTmp = NULL;

        while (fgets(zStr, 256, fpDB)) {
            if (zStr[0] == '[') {
                strcpy(pzID, &zStr[1]);
                pzID[sizeof(pzID) - 1] = 0;
                pzID[strlen(pzID) - 2] = 0;

                // Check for artist name, otherwise this CD is deleted
                DBGetINIInfo(pzID, "ARTIST", &pzTmp);
                if (pzTmp[0]) {
                    delete[] pzTmp;

					strcpy(psDI->zMCIID, pzID);
					DBGetDiscInfoLocal(gs.wDeviceID, psDI);
					
                    return TRUE;
                }

                delete[] pzTmp;
                pzTmp = NULL;
            }
        }

        return FALSE;
    }
}


void DBGetDiscID(MCIDEVICEID wDeviceID, 
                 DISCINFO* psDI)
{
    DebugPrintf("-> DBGetDiscID");

    EnterCriticalSection(&gs.sDiscInfoLock);

    CDGetDiscID(wDeviceID, psDI->zMCIID); 
    CDDBGetDiscID(wDeviceID, psDI);

	psDI->nMCITracks = CDGetTracks(wDeviceID);

    LeaveCriticalSection(&gs.sDiscInfoLock);

    DebugPrintf("<- DBGetDiscID");
}


void DBInternalQueryINI(DISCINFO* psDI)
{
    unsigned int nLoop;
    char szStr[32];
    char szTmp[32];

    if (psDI->bDiscFound == FALSE && (gs.nOptions & OPTIONS_USECDDB))
        DebugPrintf("Checking for disc in INI file cause we didn't find it in the CDDB database");
    else
        DebugPrintf("Checking for disc in INI file");

    if( ProfileGetSection( psDI->zMCIID, szStr, 10 ) ) {
        DebugPrintf("Found disc in INI file!");

    	psDI->bDiscFound = TRUE;

		DBGetINIInfo(psDI->zMCIID, "ARTIST", &psDI->pzArtist);
        DBGetINIInfo(psDI->zMCIID, "TITLE", &psDI->pzTitle);
        DBGetINIInfo(psDI->zMCIID, "CATEGORY", &psDI->pzCategory);
        DBGetINIInfo(psDI->zMCIID, "EXTD", &psDI->pzDiscExt);
        DBGetINIInfo(psDI->zMCIID, "ORDER", &psDI->pzOrder);
	    psDI->nProgrammedTracks = DBGetINIInfoInt(psDI->zMCIID, "NUMPLAY");
        psDI->nMCITracks = DBGetINIInfoInt(psDI->zMCIID, "NUMTRACKS");
        // If no tracks, try to figure out how many they are by reading the tracks
        if (!psDI->nMCITracks) {
            for (nLoop = 0 ; nLoop < 100 ; nLoop ++) {
                sprintf(szTmp, "%d", nLoop);

                ProfileGetString(psDI->zMCIID, szTmp, "NOTTHERE", szStr, sizeof(szStr) );
                if (strcmp(szStr, "NOTTHERE"))
                    psDI->nMCITracks ++;
                else
                    break;
            }
        }

        psDI->ppzTracks = new char *[psDI->nMCITracks];
        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
            sprintf(szTmp, "%d", nLoop);
            psDI->ppzTracks[nLoop] = NULL;
            DBGetINIInfo(psDI->zMCIID, szTmp, &psDI->ppzTracks[nLoop]);

            if (!psDI->ppzTracks[nLoop]) {
                char szTmp[32];

                sprintf(szTmp, "Track %d", nLoop + 1);
				psDI->ppzTracks[nLoop] = StringCopy( szTmp );
            }
        }

        psDI->ppzTracksExt = new char *[psDI->nMCITracks];
        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
		    sprintf(szStr, "EXTT%d", nLoop);
	        psDI->ppzTracksExt[nLoop] = NULL;
            DBGetINIInfo(psDI->zMCIID, szStr, &psDI->ppzTracksExt[nLoop]);
	    }          
    }
    else
        DebugPrintf("Did *NOT* find disc in INI file!");
}


void DBGetDiscInfoLocal(MCIDEVICEID wDeviceID, 
					    DISCINFO* psDI)
{
	psDI->bDiscFound = FALSE;

	DebugPrintf("-> DBGetDiscInfoLocal");

    bFoundInRemoteCDDB = FALSE;

    // Are we even supposed to do a local query?
    if (gs.nOptions & OPTIONS_QUERYLOCAL) {
        // If we are using the CDDB local database, query it
        if (gs.nOptions & OPTIONS_USECDDB) {
            DebugPrintf("Checking for disc using local CDDB");

            CDDBQueryLocal(psDI);
        }

        // If we use CDDB and didn't find anything, or if we don't use CDDB, try to query the INI file!
        if (((gs.nOptions & OPTIONS_USECDDB) && psDI->bDiscFound == FALSE) ||
            (!(gs.nOptions & OPTIONS_USECDDB)))
            DBInternalQueryINI(psDI);
    }

    ValidateDiscInfo(wDeviceID, psDI);

    DebugPrintf("<- DBGetDiscInfoLocal");
}


void DBGetDiscInfoRemote(MCIDEVICEID wDeviceID, 
					     DISCINFO* psDI,
						 BOOL* pbServerError)
{
	psDI->bDiscFound = FALSE;

	DebugPrintf("-> DBGetDiscInfo");

    bFoundInRemoteCDDB = FALSE;

    CDDBQueryRemote(psDI, FALSE, pbServerError);
    ValidateDiscInfo(wDeviceID, psDI);

    DebugPrintf("<- DBGetDiscInfoRemote");
}


void DBDelete(DISCINFO* psDI)
{
    if (gs.nOptions & OPTIONS_USECDDB)
        CDDBScanFiles(psDI, MODE_CDDB_DELETE);
    else
        ProfileWriteSection(psDI->zMCIID, "\0\0");
}

void DBInit()
{
	pzTmpInfoStr = NULL;

    bFoundInRemoteCDDB = FALSE;

    CDDBInit();
}


void DBFree()
{
	if( pzTmpInfoStr ) {
		delete[] pzTmpInfoStr;
		pzTmpInfoStr = NULL;
	}

    CDDBFree();
}


///////////////////////////////////////////////////////////////////
//
// INI FILE STUFF
//
///////////////////////////////////////////////////////////////////

void DBGetINIInfo(char* pzMCIID, char* pzKey, char** ppzRet)
{
    char zID[32];
    DWORD nLen = 0;

    if( *ppzRet ) {
        delete[] *ppzRet;
        *ppzRet = NULL;
    }

	if( !pzTmpInfoStr )
		pzTmpInfoStr = new char[MAX_INFOSTRING];
    *pzTmpInfoStr = 0;

    if ((gs.nOptions & OPTIONS_QUERYLOCAL) || bFoundInRemoteCDDB || bDBInEditor) {
	    ProfileGetString( "ALIASES", pzMCIID, pzMCIID, zID, sizeof(zID) );

        DebugPrintf("<- Getting %s from INI (ID %s)", pzKey, zID);

		if (strcmp(pzKey, "EXTD") && strncmp(pzKey, "EXTT", 4)) {
			nLen = ProfileGetString(zID, pzKey, "", pzTmpInfoStr, MAX_INFOSTRING);
			*ppzRet = new char[nLen + 1];
	        if (nLen)
				strcpy ( *ppzRet, pzTmpInfoStr );
			else
				**ppzRet = '\0';
		}
		else {		// extd, exttN
			char zKey[32];
			int nLoop;

			nLoop = 0;
			while(nLoop >= 0) {
				sprintf(zKey, "%s_%d", pzKey, nLoop);

				nLen = ProfileGetString(zID, zKey, "", pzTmpInfoStr, MAX_INFOSTRING);
				if (nLen) {
					AppendString(ppzRet, pzTmpInfoStr, -1);
					nLoop++;
				}
                else {
                    if( !*ppzRet )
						*ppzRet = StringCopy( "" );
					nLoop = -1;
                }
			}
		}

		return;
    }

	*ppzRet = StringCopy( "" );
}


int DBGetINIInfoInt(char* pzMCIID, char* pzKey)
{
    int nRet;
    char* pzStr;

    pzStr = NULL;

    DBGetINIInfo(pzMCIID, pzKey, &pzStr);
    
    nRet = atoi(pzStr);

    delete[] pzStr;

    return nRet;
}


void DBSetINIInfoInt(char* pzMCIID, char* pzKey, int nNum)
{
    char zStr[80];

    sprintf(zStr, "%d", nNum);

    DBSetINIInfo(pzMCIID, pzKey, zStr);
}


void DBSetINIInfo(char* pzMCIID, char* pzKey, char* pzInfo)
{
    char zID[32];

    if (strlen(pzInfo) < 200)
        DebugPrintf("-> DBSetInfo() Key %s = %s", pzKey, pzInfo);
    else
        DebugPrintf("-> DBSetInfo() Key %s = (long data)", pzKey);

    if (!(gs.nOptions & OPTIONS_STORELOCAL))
        return;

    ProfileGetString( "ALIASES", pzMCIID, pzMCIID, zID, sizeof(zID) );
    
    if (!(gs.nOptions & OPTIONS_USECDDB) || (gs.cddb.nCDDBOptions & OPTIONS_CDDB_STORECOPYININI)) {
		strlwr(pzKey);

        if (strcmp(pzKey, "extd") && strncmp(pzKey, "extt", 4)) {
            if (!ProfileWriteString(zID, pzKey, pzInfo)) {
                DebugPrintf("WritePrivateProfileString failed (error code %d)", GetLastError());
            }
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
					StringCpyZ(zStr, &pzInfo[nPos], sizeof(zStr));
					nPos += 80;
				}
				else {
					strcpy(zStr, &pzInfo[nPos]);
					nPos = nLen;
				}
				
				ProfileWriteString(zID, zKey, zStr);
				nLoop ++;
			}
            
            sprintf(zKey, "%s_%d", pzKey, nLoop);
            ProfileWriteString(zID, zKey, "");
		}
	}
}

