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

#ifndef __CDDB_H__
#define __CDDB_H__

#define MODE_QUERY_LOCAL		0
#define MODE_CDDB_SAVE			1
#define MODE_CDDB_DELETE		2

void CDDBInit();
void CDDBFree();

void CDDBGetDiscID(MCIDEVICEID wDeviceID,
                   DISCINFO* psDI);
BOOL CDDBQueryLocal(DISCINFO* psDI);
int CDDBQueryRemote(DISCINFO* psDI,
					 BOOL bManual,
					 BOOL* pbServerError);

BOOL CDDBOpen();
void CDDBClose();
BOOL CDDBGetID(DISCINFO* psDI, char* zID);
BOOL CDDBIsEnd();

void CDDBInternetSend(MCIDEVICEID wDeviceID, 
                      DISCINFO* psDI, 
                      HWND hWnd);
BOOL CDDBInternetGet(DISCINFO* psDI, HWND hWnd);

BOOL CDDBScanFiles(DISCINFO* psDI, int mode);

//void CDDBGetInfo(const char* pzID, char* pzKey, char** ppzRet);
//void CDDBSetInfo(const char* pzID, char* pzKey, char* pzInfo);

BOOL CDDBQuerySites();

#endif //__CDDB_H__
