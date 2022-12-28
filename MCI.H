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

#pragma once

MCIERROR NotifyMCISendCommand(MCIDEVICEID IDDevice,    
                              UINT uMsg,             
                              DWORD fdwCommand,        
                              DWORD dwParam);

BOOL CDOpen(MCIDEVICEID* lpDeviceID);
void CDClose(MCIDEVICEID* lpDeviceID);
unsigned int CDGetTracks(MCIDEVICEID wDeviceID);
BOOL CDGetAudio(MCIDEVICEID wDeviceID);
unsigned int CDGetCurrTrack(MCIDEVICEID wDeviceID);
void CDPlay(MCIDEVICEID wDeviceID,
            unsigned int nTrack, 
            BOOL bResume = FALSE, 
            unsigned int nMin = 0, 
            unsigned int nSec = 0);
void CDPlayPos(MCIDEVICEID wDeviceID,
               unsigned int nTrack, 
               unsigned int nMin, 
               unsigned int nSec);
void CDResume(MCIDEVICEID wDeviceID);
void CDStop(MCIDEVICEID wDeviceID);
void CDPause(MCIDEVICEID wDeviceID);
void CDEject(MCIDEVICEID wDeviceID);
void CDLoad(MCIDEVICEID wDeviceID);
unsigned int CDGetTrackLength(MCIDEVICEID wDeviceID,
                              unsigned int nTrack, 
                              char* pzStr);
unsigned int CDGetStatus(MCIDEVICEID wDeviceID);
BOOL CDGetMediaPresent(MCIDEVICEID wDeviceID);
void CDGetTime(MCIDEVICEID wDeviceID,
               unsigned int nTimeOptions, 
			   char* pzTime,
			   BOOL bTMSF = FALSE, 
			   unsigned int* pnTrack = NULL, 
			   unsigned int* pnMin = NULL, 
			   unsigned int* pnSec = NULL, 
			   unsigned int *pnFrame = NULL);
void CDGetDiscID(MCIDEVICEID wDeviceID,
                 char* pzID);

// Functions used to generate the CDDB id
void CDGetEndFrame(MCIDEVICEID wDeviceID,
                   DISCINFO* psDI, 
                   unsigned int nOffset, 
				   unsigned int* pnMin, 
				   unsigned int* pnSec);
void CDGetAbsoluteTrackPos(MCIDEVICEID wDeviceID,
                           unsigned int nTrack, 
						   unsigned int* pnFrame, 
						   unsigned int* pnMin, 
						   unsigned int* pnSec);
