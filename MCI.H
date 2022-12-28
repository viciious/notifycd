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

BOOL CDOpen();
void CDClose();
int CDGetTracks();
BOOL CDGetAudio();
int CDGetCurrTrack();
void CDPlay(int nTrack, BOOL bResume = FALSE, int nMin = 0, int nSec = 0);
void CDPlayPos(int nTrack, int nMin, int nSec);
void CDResume();
void CDStop();
void CDPause();
void CDEject();
void CDLoad();
long CDGetAbsoluteTrackPos(int nTrack);
long CDGetEndFrame();
int CDGetTrackLength(int nTrack, char* pzStr);
int CDGetStatus();
BOOL CDGetMediaPresent();
void CDGetTime(int nTimeOptions, BOOL bTMSF = FALSE, int* pnTrack = NULL, int* pnMin = NULL, int* pnSec = NULL, int *pnFrame = NULL);
void CDGetDiscID(char* pzID);
int CDGetLastTrackInARow(int nTrack);
