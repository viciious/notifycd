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

#pragma warning(disable:4514)

BOOL DBOpen();
void DBClose();
BOOL DBSave();
BOOL DBInternetGet(HWND hWnd);
void DBInternetSend(HWND hWnd);
BOOL DBGetDBID(char* pzID, char** ppzArtist, char** ppzTitle, char** ppzCategory);
BOOL DBIsEnd();
void DBGetDiscID(char* pzCDDBID, char* pzMCIID, BOOL bQuery, BOOL* pbFoundLocal, BOOL* pbFoundRemote);
void DBDelete(const char* pzCDDBID, const char* pzMCIID);
void DBGetAlias(const char* pzCheckID, char* pzDestID);
int DBGetInfoInt(const char* pzCDDBID, const char* pzMCIID, char* pzKey);
void DBGetInfo(const char* pzCDDBID, const char* pzMCIID, char* pzKey, char** ppzRet);
char** DBGetTrackTitles(const char* pzCDDBID, const char* pzMCIID, int nNum);
void DBSetInfoInt(const char* pzCDDBID, const char* pzMCIID, char* pzKey, int nNum);
void DBSetInfo(const char* pzCDDBID, const char* pzMCIID, char* pzKey, char* pzInfo);
void DBSetTrackTitles(const char* pzCDDBID, const char* pzMCIID, char** pppzTracks, int nNum);
void DBFreeTrackTitles(char** pppzTracks, int nTracks);
void DBInit();
void DBFree();
