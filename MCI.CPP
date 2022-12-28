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

#include "resource.h"

#include "ntfy_cd.h"
#include "misc.h"
#include "mci.h"

/////////////////////////////////////////////////////////////////////
//
// MCI STUFF!
//
/////////////////////////////////////////////////////////////////////

BOOL CDOpen()
{
	char zDevice[4];
    DWORD nErr;

    CDClose();

    sprintf(zDevice, "%c:", (char)nCurrentDevice + 'A');

    DebugPrintf("Opening %s (%d)", zDevice, nCurrentDevice);

    sMCIOpen.lpstrDeviceType = (LPCSTR) MCI_DEVTYPE_CD_AUDIO;
    sMCIOpen.lpstrElementName = zDevice;
    nErr = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, 
					      (DWORD) (LPVOID) &sMCIOpen);
    if (nErr) 
    {
		DebugPrintf("Open non-shared");

        nErr = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, 
					          (DWORD) (LPVOID) &sMCIOpen);
	    if (nErr) {
		    char zError[256];

            bCDOpened = FALSE;

            mciGetErrorString(nErr, zError, 255);

            DebugPrintf("Open failed: %s", zError);

		    return FALSE;
        }
    }
    
    DebugPrintf("Open ok!");

    bCDOpened = TRUE;

	sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
	
	return TRUE;
}


void CDClose()
{
    DebugPrintf("Closing CD");

    sMCIGeneric.dwCallback = (DWORD) hMainWnd;
    mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD) (LPVOID) &sMCIGeneric);

    bCDOpened = FALSE;
}


int CDGetTracks()
{
    sMCIStatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	DebugPrintf("CDGetTracks() = %d", min ((int) sMCIStatus.dwReturn, 99));

    return (min ((int) sMCIStatus.dwReturn, 99));
}


BOOL CDGetAudio()
{
    BOOL bRet = FALSE;

    int nTracks = CDGetTracks();

    sMCIStatus.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
    sMCIStatus.dwTrack = 1;
    mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	if (sMCIStatus.dwReturn == MCI_CDA_TRACK_AUDIO) {
        DebugPrintf("Media is Audio");

        return TRUE;
    }
	else {
        for (int nLoop = 1 ; nLoop <= nTracks ; nLoop ++) {
            sMCIStatus.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
            sMCIStatus.dwTrack = nLoop;
            mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
					        (DWORD) (LPVOID) &sMCIStatus);
	        if (sMCIStatus.dwReturn == MCI_CDA_TRACK_AUDIO)
                bRet = TRUE;
        }

        if (bRet)
            DebugPrintf("Media is Audio");
        else
            DebugPrintf("Media is *NOT* Audio");

        return bRet;
    }
}


int CDGetCurrTrack()
{
	sMCIStatus.dwItem = MCI_STATUS_POSITION;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	return ((int) MCI_TMSF_TRACK (sMCIStatus.dwReturn));
}


void CDPlay(int nTrack, BOOL bResume, int nMin, int nSec)
{
    int nFlags;
	int nActualTrack = pnProgrammedTracks[nTrack]+1;
	int nPlayTo;       

	DebugPrintf("CDPlay() (Track = %d, Min = %d, Sec = %d, Resume = %d)", nTrack, nMin, nSec, bResume);

    sMCIPlay.dwCallback = (DWORD) hMainWnd;
    sMCIPlay.dwFrom = MCI_MAKE_TMSF (nActualTrack, nMin, nSec, 0);
	if ((bProgrammed || bRepeatTrack || bRandomize)) {// && nActualTrack < nMaxTrack) {
        if (!bProgrammed || bRepeatTrack)
            nPlayTo = nActualTrack+1;
        else
            nPlayTo = CDGetLastTrackInARow(nTrack)+1;

        if (nPlayTo <= nMaxTrack) {
            sMCIPlay.dwTo = MCI_MAKE_TMSF (nPlayTo, 0, 0, 0);
            if (!bResume)
                nFlags = MCI_FROM | MCI_TO | MCI_NOTIFY;
            else
                nFlags = MCI_TO | MCI_NOTIFY;
            mciSendCommand (sMCIOpen.wDeviceID, MCI_PLAY, nFlags,
			    (DWORD) (LPVOID) &sMCIPlay);
        }
        else {
            if (!bResume) {
                nFlags = MCI_FROM | MCI_NOTIFY;

				mciSendCommand (sMCIOpen.wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay);
			}
            else {
				sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
				mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
				
				sMCIPlay.dwTo = nDiscLenMS + nFrameOffset/75*1000;
            
				nFlags = MCI_TO | MCI_NOTIFY;

				if (mciSendCommand (sMCIOpen.wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay)) {
					sMCIPlay.dwTo = nDiscLenMS;

					mciSendCommand (sMCIOpen.wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay);
				}

				sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
				mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
			}
        }
	}
	else {
        if (!bResume) {
            nFlags = MCI_FROM | MCI_NOTIFY;

			mciSendCommand (sMCIOpen.wDeviceID, MCI_PLAY, nFlags,
				(DWORD) (LPVOID) &sMCIPlay);
		}
        else {
			sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
			mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
			
			nFlags = MCI_NOTIFY;

			if (mciSendCommand (sMCIOpen.wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay)) {
	            sMCIPlay.dwTo = nDiscLenMS;

				mciSendCommand (sMCIOpen.wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay);
			}

			sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
			mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
		}
    }

    nCurrTrack = nTrack;

    bPaused = FALSE;
    bPlaying = TRUE;
}


void CDPlayPos(int nTrack, int nMin, int nSec)
{
    DebugPrintf("CDPlayPos()");

	CDPlay(nTrack, FALSE, nMin, nSec);
}


void CDResume()
{
	DebugPrintf("CDResume()");

    CDPlay(nCurrTrack, TRUE);
}


void CDStop()
{
	DebugPrintf("CDStop()");

	mciSendCommand (sMCIOpen.wDeviceID, MCI_STOP, NULL, NULL);

    nCurrTrack = 0;

    bPaused = FALSE;
    bPlaying = FALSE;
}


void CDPause()
{
	DebugPrintf("CDPause()");

	mciSendCommand (sMCIOpen.wDeviceID, MCI_PAUSE, NULL, NULL);

	bPaused = TRUE;
    bPlaying = FALSE;
}


void CDEject()
{
	DebugPrintf("CDEject()");

    mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, (DWORD) (LPVOID) &sMCISet);

    bPaused = FALSE;
    bPlaying = FALSE;
}


void CDLoad()
{
	DebugPrintf("CDLoad()");

    mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, (DWORD) (LPVOID) &sMCISet);

    bPaused = FALSE;
    bPlaying = FALSE;
}


long CDGetAbsoluteTrackPos(int nTrack)
{
    DWORD nFrame;

    sMCIStatus.dwItem = MCI_STATUS_POSITION;
	sMCIStatus.dwTrack = nTrack;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);
    
	nFrame = (int)((float)sMCIStatus.dwReturn/1000*75);

    if (nTrack == 1)
        nFrameOffset = nFrame;

    return (nFrame / 75);
}


long CDGetEndFrame()
{
    int nFrame;

    sMCIStatus.dwItem = MCI_STATUS_LENGTH;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
				    (DWORD) (LPVOID) &sMCIStatus);
    nDiscLenMS = sMCIStatus.dwReturn;
	nFrame = (int)((float)sMCIStatus.dwReturn/1000*75) + nFrameOffset;
    nFrame ++; // Due to bug in MCI according to DB docs!

//	nEndFrame = nFrame;

    return (nFrame / 75);
}


int CDGetTrackLength(int nTrack, char* pzStr)
{
	int nSec1, nSec2;

	nSec1 = CDGetAbsoluteTrackPos(nTrack);
	if (nTrack != nMaxTrack)
        nSec2 = CDGetAbsoluteTrackPos(nTrack+1);
    else
        nSec2 = CDGetEndFrame();

    nSec2 -= nSec1;

	sprintf(pzStr, "%02d:%02d", (int) nSec2/60, nSec2%60);

    return nSec2;
}


int CDGetStatus()
{
    sMCIStatus.dwItem = MCI_STATUS_MODE;
    mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);
    if (sMCIStatus.dwReturn == MCI_MODE_NOT_READY ||
        sMCIStatus.dwReturn == MCI_MODE_OPEN)
        return 0;
    else if (sMCIStatus.dwReturn == MCI_MODE_PLAY)
        return 2;
    else if ((sMCIStatus.dwReturn == MCI_MODE_STOP) && bPaused)
        return 3;
    else
        return 1;
}


BOOL CDGetMediaPresent()
{
    sMCIStatus.dwItem = MCI_STATUS_MEDIA_PRESENT;
    mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	return sMCIStatus.dwReturn;
}


void CDGetTime(int nTimeOptions, BOOL bTMSF, int* pnTrack, int* pnMin, int* pnSec, int *pnFrame)
{
    if (nCurrTrack >= nProgrammedTracks)
        return;

    if ((nTimeOptions & OPTIONS_TIME_TRACK) || bTMSF) {
		sMCIStatus.dwItem = MCI_STATUS_POSITION;
		mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
						(DWORD) (LPVOID) &sMCIStatus);

        if (!bTMSF)
            sprintf(szTime, "%02d:%02d", (int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn), 
		  	  	(int) MCI_TMSF_SECOND (sMCIStatus.dwReturn));
        else {
            *pnTrack = MCI_TMSF_TRACK(sMCIStatus.dwReturn);
            *pnMin = MCI_TMSF_MINUTE(sMCIStatus.dwReturn);
            *pnSec = MCI_TMSF_SECOND(sMCIStatus.dwReturn);
            *pnFrame = MCI_TMSF_FRAME(sMCIStatus.dwReturn);
        }
    }
    else if (nTimeOptions & OPTIONS_TIME_TRACKREM) {
        int nLenMin, nLenSec;
        int nPosMin, nPosSec;

		sMCIStatus.dwItem = MCI_STATUS_POSITION;
		mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
						(DWORD) (LPVOID) &sMCIStatus);

		nPosMin = (int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn);
		nPosSec = (int) MCI_TMSF_SECOND (sMCIStatus.dwReturn);

        sscanf(ppzTrackLen[pnProgrammedTracks[nCurrTrack]], "%02d:%02d", &nLenMin, &nLenSec);
								
        nLenMin -= nPosMin;
        nLenSec -= nPosSec;
        if (nLenSec < 0) {
            nLenSec += 60;
            nLenMin --;
        }

        sprintf(szTime, "%02d:%02d", nLenMin, nLenSec);
    }
    else if (nTimeOptions & OPTIONS_TIME_CD && !bRandomize) {
		int nLoop;
		int nSec = 0;
        int nPosMin, nPosSec;

		if (!bProgrammed) {
			// Loop and add all tracks before this one
			for (nLoop = 0 ; nLoop < nCurrTrack ; nLoop ++)
				nSec += pnTrackLen[nLoop];

			// Fix nPosMin and nPosSec

			nPosMin = nSec / 60;
			nSec -= (nPosMin * 60);
			nPosSec = nSec;

			// Get curr pos on track
			sMCIStatus.dwItem = MCI_STATUS_POSITION;
			mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
							(DWORD) (LPVOID) &sMCIStatus);

			nPosMin += (int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn);
			nPosSec += (int) MCI_TMSF_SECOND (sMCIStatus.dwReturn);

			sprintf(szTime, "%02d:%02d", nPosMin, nPosSec);
		}
		else
			strcpy(szTime, "N/A");
    }
    else if (nTimeOptions & OPTIONS_TIME_CDREM && !bRandomize) {
		int nLoop;
		int nPos = 0;
		int nLen = 0;
        int nPosMin, nPosSec;
        int nLenMin, nLenSec;

		if (!bProgrammed) {
			// Loop and add all tracks 
			for (nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++)
				nLen += pnTrackLen[nLoop];

			// Fix nPosMin and nPosSec

			nLenMin = nLen / 60;
			nLen -= (nLenMin * 60);
			nLenSec = nLen;

			// Loop and add all tracks before this one
			for (nLoop = 0 ; nLoop < nCurrTrack ; nLoop ++)
				nPos += pnTrackLen[nLoop];

			// Fix nPosMin and nPosSec

			nPosMin = nPos / 60;
			nPos -= (nPosMin * 60);
			nPosSec = nPos;

			// Get curr pos on track
			sMCIStatus.dwItem = MCI_STATUS_POSITION;
			mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
							(DWORD) (LPVOID) &sMCIStatus);

			nPosMin += (int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn);
			nPosSec += (int) MCI_TMSF_SECOND (sMCIStatus.dwReturn);

			nLenMin -= nPosMin;
			nLenSec -= nPosSec;
			if (nLenSec < 0) {
				nLenSec += 60;
				nLenMin --;
			}

			sprintf(szTime, "%02d:%02d", nLenMin, nLenSec);
		}
		else
			strcpy(szTime, "N/A");
    }
	else 
		sprintf(szTime, "N/A");
}


void CDGetDiscID(char* pzID)
{
	DebugPrintf("CDGetDiscID()");

    sMCIInfo.lpstrReturn = szMCIReturnString;
	sMCIInfo.dwRetSize = 79;
    mciSendCommand (sMCIOpen.wDeviceID, MCI_INFO, MCI_INFO_MEDIA_IDENTITY | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIInfo);

    sprintf(pzID, "%X", atoi(sMCIInfo.lpstrReturn));
}


int CDGetLastTrackInARow(int nTrack)
{
	int nLoop;
    
    for (nLoop = nTrack + 1 ; nLoop < nProgrammedTracks ; nLoop ++) {
        if (pnProgrammedTracks[nLoop] != pnProgrammedTracks[nLoop-1] + 1) {
            nNextProgrammedTrack = nLoop;
            return pnProgrammedTracks[nLoop-1]+1;
        }
    }

    nNextProgrammedTrack = nProgrammedTracks;

    return pnProgrammedTracks[nLoop-1]+1;
}


