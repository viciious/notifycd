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
#include "misc.h"
#include "mci.h"

extern GLOBALSTRUCT gs;

/////////////////////////////////////////////////////////////////////
//
// MCI STUFF!
//
/////////////////////////////////////////////////////////////////////

MCIERROR NotifyMCISendCommand(MCIDEVICEID IDDevice,    
                              UINT uMsg,             
                              DWORD fdwCommand,        
                              DWORD dwParam)
{
    MCIERROR nErr;

    nErr = mciSendCommand(IDDevice, uMsg, fdwCommand, dwParam);
    if (nErr) {
        char szError[256];

        mciGetErrorString(nErr, szError, 256);

        DebugPrintf("NotifyMCI error: %s", szError);
    }

    return nErr;
}



BOOL CDOpen(MCIDEVICEID* lpDeviceID)
{
    MCI_OPEN_PARMS sMCIOpen;
    MCI_SET_PARMS sMCISet;
	char zDevice[4];
    DWORD nErr;

    if (*lpDeviceID)
        CDClose(lpDeviceID);

    sprintf(zDevice, "%c:", (char)gs.nCurrentDevice + 'A');

    DebugPrintf("Opening %s (%d)", zDevice, gs.nCurrentDevice);

    sMCIOpen.lpstrDeviceType = (LPCSTR) MCI_DEVTYPE_CD_AUDIO;
    sMCIOpen.lpstrElementName = zDevice;
    nErr = NotifyMCISendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, 
					      (DWORD) (LPVOID) &sMCIOpen);
    if (nErr) {
		DebugPrintf("Open non-shared");

        nErr = NotifyMCISendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, 
					          (DWORD) (LPVOID) &sMCIOpen);
	    if (nErr) {
		    char zError[256];

            gs.state.bCDOpened = FALSE;

            mciGetErrorString(nErr, zError, 255);

            DebugPrintf("Open failed: %s", zError);

		    return FALSE;
        }
    }
    
    DebugPrintf("Open ok!");

    if (lpDeviceID == &gs.wDeviceID)
        gs.state.bCDOpened = TRUE;

	sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	NotifyMCISendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
	
    *lpDeviceID = sMCIOpen.wDeviceID;

    return TRUE;
}


void CDClose(MCIDEVICEID* lpDeviceID)
{
	MCI_GENERIC_PARMS sMCIGeneric;
        
    DebugPrintf("Closing CD");

    sMCIGeneric.dwCallback = (DWORD) gs.hMainWnd;
    NotifyMCISendCommand(*lpDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD) (LPVOID) &sMCIGeneric);

    *lpDeviceID = 0;
    if (lpDeviceID == &gs.wDeviceID)
        gs.state.bCDOpened = FALSE;
}


unsigned int CDGetTracks(MCIDEVICEID wDeviceID)
{
    MCI_STATUS_PARMS sMCIStatus;

    sMCIStatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	DebugPrintf("CDGetTracks() = %d", min ((int) sMCIStatus.dwReturn, 99));

    return (min ((int) sMCIStatus.dwReturn, 99));
}


BOOL CDGetAudio(MCIDEVICEID wDeviceID)
{
    MCI_STATUS_PARMS sMCIStatus;
    BOOL bRet = FALSE;

    unsigned int nTracks = CDGetTracks(wDeviceID);

    sMCIStatus.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
    sMCIStatus.dwTrack = 1;
    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	if (sMCIStatus.dwReturn == MCI_CDA_TRACK_AUDIO) {
        DebugPrintf("Media is Audio");

        return TRUE;
    }
	else {
		DebugPrintf("Checking each track just to be sure...");

        for (unsigned int nLoop = 1 ; nLoop <= nTracks ; nLoop ++) {
            sMCIStatus.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
            sMCIStatus.dwTrack = nLoop;
            NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
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


unsigned int CDGetCurrTrack(MCIDEVICEID wDeviceID)
{
    MCI_STATUS_PARMS sMCIStatus;

    sMCIStatus.dwItem = MCI_STATUS_POSITION;
	NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	return ((int) MCI_TMSF_TRACK (sMCIStatus.dwReturn));
}


void CDPlay(MCIDEVICEID wDeviceID, 
            unsigned int nTrack, 
            BOOL bResume, 
            unsigned int nMin, 
            unsigned int nSec)
{
	MCI_SET_PARMS sMCISet;
	MCI_PLAY_PARMS sMCIPlay;
    unsigned int nFlags;
	unsigned int nActualTrack = gs.di[0].pnProgrammedTracks[nTrack]+1;
	unsigned int nPlayTo;       

	DebugPrintf("CDPlay() (Track = %d, Min = %d, Sec = %d, Resume = %d)", nTrack, nMin, nSec, bResume);

    sMCIPlay.dwCallback = (DWORD) gs.hMainWnd;
    sMCIPlay.dwFrom = MCI_MAKE_TMSF (nActualTrack, nMin, nSec, 0);
	if ((gs.state.bProgrammed || gs.state.bRepeatTrack || gs.state.bRandomize)) {// && nActualTrack < gs.di[0].nMCITracks) {
        if (!gs.state.bProgrammed || gs.state.bRepeatTrack)
            nPlayTo = nActualTrack+1;
        else
            nPlayTo = CDGetLastTrackInARow(nTrack)+1;

        if (nPlayTo <= gs.di[0].nMCITracks) {
            sMCIPlay.dwTo = MCI_MAKE_TMSF (nPlayTo, 0, 0, 0);
            if (!bResume)
                nFlags = MCI_FROM | MCI_TO | MCI_NOTIFY;
            else
                nFlags = MCI_TO | MCI_NOTIFY;
            NotifyMCISendCommand (wDeviceID, MCI_PLAY, nFlags,
			    (DWORD) (LPVOID) &sMCIPlay);
        }
        else {
            if (!bResume) {
                nFlags = MCI_FROM | MCI_NOTIFY;

				NotifyMCISendCommand (wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay);
			}
            else {
				sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
				NotifyMCISendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
				
				sMCIPlay.dwTo = gs.di[0].nDiscLenMS + gs.di[0].nFrameOffset/75*1000;
            
				nFlags = MCI_TO | MCI_NOTIFY;

				if (NotifyMCISendCommand (wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay)) {
					sMCIPlay.dwTo = gs.di[0].nDiscLenMS;

					NotifyMCISendCommand (wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay);
				}

				sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
				NotifyMCISendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
			}
        }
	}
	else {
        if (!bResume) {
            nFlags = MCI_FROM | MCI_NOTIFY;

			NotifyMCISendCommand (wDeviceID, MCI_PLAY, nFlags,
				(DWORD) (LPVOID) &sMCIPlay);
		}
        else {
			sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
			NotifyMCISendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
			
			nFlags = MCI_NOTIFY;

			if (NotifyMCISendCommand (wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay)) {
	            sMCIPlay.dwTo = gs.di[0].nDiscLenMS;

				NotifyMCISendCommand (wDeviceID, MCI_PLAY, nFlags, (DWORD) (LPVOID) &sMCIPlay);
			}

			sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
			NotifyMCISendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
		}
    }

    gs.di[0].nCurrTrack = nTrack;

	if (gs.state.bRepeatTrack)
		gs.nRepeatTrack = gs.di[0].nCurrTrack;

    gs.state.bPaused = FALSE;
    gs.state.bPlaying = TRUE;
}


void CDPlayPos(MCIDEVICEID wDeviceID, 
               unsigned int nTrack, 
               unsigned int nMin, 
               unsigned int nSec)
{
    DebugPrintf("CDPlayPos()");

	CDPlay(wDeviceID, nTrack, FALSE, nMin, nSec);
}


void CDResume(MCIDEVICEID wDeviceID)
{
	DebugPrintf("CDResume()");

    CDPlay(wDeviceID, gs.di[0].nCurrTrack, TRUE);
}


void CDStop(MCIDEVICEID wDeviceID)
{
	DebugPrintf("CDStop()");

	NotifyMCISendCommand(wDeviceID, MCI_STOP, NULL, NULL);

    gs.di[0].nCurrTrack = 0;

    gs.state.bPaused = FALSE;
    gs.state.bPlaying = FALSE;
}


void CDPause(MCIDEVICEID wDeviceID)
{
	DebugPrintf("CDPause()");

	NotifyMCISendCommand (wDeviceID, MCI_PAUSE, NULL, NULL);

	gs.state.bPaused = TRUE;
    gs.state.bPlaying = FALSE;
}


void CDEject(MCIDEVICEID wDeviceID)
{
	MCI_SET_PARMS sMCISet;

    DebugPrintf("CDEject()");

    NotifyMCISendCommand (wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, (DWORD) (LPVOID) &sMCISet);

    gs.state.bPaused = FALSE;
    gs.state.bPlaying = FALSE;
}


void CDLoad(MCIDEVICEID wDeviceID)
{
	MCI_SET_PARMS sMCISet;

    DebugPrintf("CDLoad()");

    NotifyMCISendCommand (wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, (DWORD) (LPVOID) &sMCISet);

    gs.state.bPaused = FALSE;
    gs.state.bPlaying = FALSE;
}


unsigned int CDGetTrackLength(MCIDEVICEID wDeviceID,
                              unsigned int nTrack, 
                              char* pzStr)
{
    MCI_STATUS_PARMS sMCIStatus;
    
    sMCIStatus.dwItem = MCI_STATUS_LENGTH;
	sMCIStatus.dwTrack = nTrack;
	NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);
   
	sMCIStatus.dwReturn /= 1000;
    sprintf(pzStr, "%02d:%02d", (int) sMCIStatus.dwReturn/60, sMCIStatus.dwReturn%60);
	return sMCIStatus.dwReturn;
}


unsigned int CDGetStatus(MCIDEVICEID wDeviceID)
{
    MCI_STATUS_PARMS sMCIStatus;

    sMCIStatus.dwItem = MCI_STATUS_MODE;
    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);
    if (sMCIStatus.dwReturn == MCI_MODE_NOT_READY ||
        sMCIStatus.dwReturn == MCI_MODE_OPEN)
        return 0;
    else if (sMCIStatus.dwReturn == MCI_MODE_PLAY)
        return 2;
    else if ((sMCIStatus.dwReturn == MCI_MODE_STOP) && gs.state.bPaused)
        return 3;
    else
        return 1;
}


BOOL CDGetMediaPresent(MCIDEVICEID wDeviceID)
{
    MCI_STATUS_PARMS sMCIStatus;

    sMCIStatus.dwItem = MCI_STATUS_MEDIA_PRESENT;
    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);

	return sMCIStatus.dwReturn;
}


void CDGetTime(MCIDEVICEID wDeviceID,
               unsigned int nTimeOptions, 
			   char* pzTime,
			   BOOL bTMSF, 
			   unsigned int* pnTrack, 
			   unsigned int* pnMin, 
			   unsigned int* pnSec, 
			   unsigned int *pnFrame)
{
    MCI_STATUS_PARMS sMCIStatus;

    if (gs.di[0].nCurrTrack >= gs.di[0].nProgrammedTracks)
        return;

    if (gs.state.bPlaying || gs.state.bPaused) {
        if ((nTimeOptions & TIME_TRACK) || bTMSF) {
		    sMCIStatus.dwItem = MCI_STATUS_POSITION;
		    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
						    (DWORD) (LPVOID) &sMCIStatus);

            if (!bTMSF)
                sprintf(pzTime, "%02d:%02d", (unsigned int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn), 
		  	  	    (unsigned int) MCI_TMSF_SECOND (sMCIStatus.dwReturn));
            else {
                *pnTrack = MCI_TMSF_TRACK(sMCIStatus.dwReturn);
                *pnMin = MCI_TMSF_MINUTE(sMCIStatus.dwReturn);
                *pnSec = MCI_TMSF_SECOND(sMCIStatus.dwReturn);
                *pnFrame = MCI_TMSF_FRAME(sMCIStatus.dwReturn);
            }
        }
        else if (nTimeOptions & TIME_TRACKREM) {
            int nLenMin, nLenSec;
            int nPosMin, nPosSec;

            sMCIStatus.dwItem = MCI_STATUS_POSITION;
		    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
						    (DWORD) (LPVOID) &sMCIStatus);

		    nPosMin = (int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn);
		    nPosSec = (int) MCI_TMSF_SECOND (sMCIStatus.dwReturn);

            sscanf(gs.di[0].ppzTrackLen[gs.di[0].pnProgrammedTracks[gs.di[0].nCurrTrack]], "%02d:%02d", &nLenMin, &nLenSec);
								    
            nLenMin -= nPosMin;
            nLenSec -= nPosSec;
            if (nLenSec < 0) {
                nLenSec += 60;
                nLenMin --;
            }

            sprintf(pzTime, "%02d:%02d", nLenMin, nLenSec);
        }
        else if (nTimeOptions & TIME_CD && !gs.state.bRandomize) {
		    unsigned int nLoop;
		    unsigned int nSec = 0;
            unsigned int nPosMin, nPosSec;

		    if (!gs.state.bProgrammed) {
			    // Loop and add all tracks before this one
			    for (nLoop = 0 ; nLoop < gs.di[0].nCurrTrack ; nLoop ++)
				    nSec += gs.di[0].pnTrackLen[nLoop];

			    // Fix nPosMin and nPosSec

			    nPosMin = nSec / 60;
			    nSec -= (nPosMin * 60);
			    nPosSec = nSec;

			    // Get curr pos on track
			    sMCIStatus.dwItem = MCI_STATUS_POSITION;
			    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
							    (DWORD) (LPVOID) &sMCIStatus);

			    nPosMin += (unsigned int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn);
			    nPosSec += (unsigned int) MCI_TMSF_SECOND (sMCIStatus.dwReturn);

			    sprintf(pzTime, "%02d:%02d", nPosMin, nPosSec);
		    }
		    else
			    strcpy(pzTime, "00:00");
        }
        else if (nTimeOptions & TIME_CDREM && !gs.state.bRandomize) {
		    unsigned int nLoop;
		    unsigned int nPos = 0;
		    unsigned int nLen = 0;
            unsigned int nPosMin, nPosSec;
            int nLenMin, nLenSec;

		    if (!gs.state.bProgrammed) {
			    // Loop and add all tracks 
			    for (nLoop = 0 ; nLoop < gs.di[0].nProgrammedTracks ; nLoop ++)
				    nLen += gs.di[0].pnTrackLen[nLoop];

			    // Fix nPosMin and nPosSec

			    nLenMin = nLen / 60;
			    nLen -= (nLenMin * 60);
			    nLenSec = nLen;

			    // Loop and add all tracks before this one
			    for (nLoop = 0 ; nLoop < gs.di[0].nCurrTrack ; nLoop ++)
				    nPos += gs.di[0].pnTrackLen[nLoop];

			    // Fix nPosMin and nPosSec

			    nPosMin = nPos / 60;
			    nPos -= (nPosMin * 60);
			    nPosSec = nPos;

			    // Get curr pos on track
			    sMCIStatus.dwItem = MCI_STATUS_POSITION;
			    NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
							    (DWORD) (LPVOID) &sMCIStatus);

			    nPosMin += (unsigned int) MCI_TMSF_MINUTE (sMCIStatus.dwReturn);
			    nPosSec += (unsigned int) MCI_TMSF_SECOND (sMCIStatus.dwReturn);

			    nLenMin -= nPosMin;
			    nLenSec -= nPosSec;
			    if (nLenSec < 0) {
				    nLenSec += 60;
				    nLenMin --;
			    }

			    sprintf(pzTime, "%02d:%02d", nLenMin, nLenSec);
		    }
		    else
			    strcpy(pzTime, "00:00");
        }
        else
			strcpy(pzTime, "00:00");
    }
	else {
		if (pzTime)
			strcpy(pzTime, "00:00");
	}
}


void CDGetDiscID(MCIDEVICEID wDeviceID,
                 char* pzID)
{
	char szMCIReturnString[80];
    MCI_INFO_PARMS sMCIInfo;
        
    DebugPrintf("CDGetDiscID()");

    sMCIInfo.lpstrReturn = szMCIReturnString;
	sMCIInfo.dwRetSize = 79;
    NotifyMCISendCommand (wDeviceID, MCI_INFO, MCI_INFO_MEDIA_IDENTITY | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIInfo);

    sprintf(pzID, "%X", atoi(sMCIInfo.lpstrReturn));
}


// Functions used to generate the CDDB id

void CDGetEndFrame(MCIDEVICEID wDeviceID,
                   DISCINFO* psDI, 
                   unsigned int nOffset, 
				   unsigned int* pnMin, 
				   unsigned int* pnSec)
{
    MCI_STATUS_PARMS sMCIStatus;
    int nFrame;

    sMCIStatus.dwItem = MCI_STATUS_LENGTH;
	NotifyMCISendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, 
				    (DWORD) (LPVOID) &sMCIStatus);
    nFrame = (int)((float)sMCIStatus.dwReturn/1000*75) + nOffset;
    nFrame ++; // Due to bug in MCI according to CDDB docs!

    DebugPrintf("Frame End = %d", nFrame);

    psDI->nDiscLength = nFrame / 75;

    DebugPrintf("Disc length = %d", psDI->nDiscLength);

	*pnMin = nFrame / 75 / 60;
	*pnSec = (nFrame / 75) % 60;
}


void CDGetAbsoluteTrackPos(MCIDEVICEID wDeviceID,
                           unsigned int nTrack, 
						   unsigned int* pnFrame, 
						   unsigned int* pnMin, 
						   unsigned int* pnSec)
{
    MCI_STATUS_PARMS sMCIStatus;

    sMCIStatus.dwItem = MCI_STATUS_POSITION;
	sMCIStatus.dwTrack = nTrack;
	NotifyMCISendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, 
					(DWORD) (LPVOID) &sMCIStatus);
    *pnFrame = (int)((float)sMCIStatus.dwReturn/1000*75);

    DebugPrintf("Frame %d = %d", nTrack, *pnFrame);

    *pnMin = *pnFrame / 75 / 60;
	*pnSec = (*pnFrame / 75) % 60;
}


