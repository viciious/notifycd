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

typedef struct {
    char* pzArtist;
    char* pzTitle;
    char** ppzTracks;
    char* pzPlayorder;
    // Saved info used to recreate the disc entry!
    char* pzDiscid;
    int nRevision;
    char** ppzTracksExt;
    char* pzExtD;
    int nTracks;
    char zCategory[32];
    char zSubmitted[80];
    int nDiscLength;
    int* pnFrames;
} CDDB_DISC_ENTRY;

void CDDBInit();
void CDDBFree();

void CDDBGetDiscID(char* pzID);
void CDDBDoQuery(BOOL bManual, BOOL* pbFoundLocal, BOOL* pbFoundRemote);

BOOL CDDBOpen();
void CDDBClose();
BOOL CDDBGetID(char* pzID, char** ppzArtist, char** ppzTitle, char** ppzCategory);
BOOL CDDBIsEnd();
void CDDBDelete(const char* pzID);
BOOL CDDBSave();

void CDDBInternetSend(HWND hWnd);
BOOL CDDBInternetGet(HWND hWnd);

void CDDBGetInfo(const char* pzID, char* pzKey, char** ppzRet);
void CDDBSetInfo(const char* pzID, char* pzKey, char* pzInfo);

void CDDBSetTrackTitles(const char* pzID, char** ppzTracks, int nNumTracks);
char** CDDBGetTrackTitles(const char* pzID, int nNumTracks);
void CDDBFreeTrackTitles(char** ppzTracks, int nTracks);
