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

BOOL DBSave(DISCINFO* psDI);
BOOL DBInternetGet(DISCINFO* psDI, HWND hWnd);
void DBInternetSend(DISCINFO* psDI, HWND hWnd);
void DBGetDiscID(MCIDEVICEID wDeviceID, 
                 DISCINFO* psDI);
void DBGetDiscInfoLocal(MCIDEVICEID wDeviceID, 
					    DISCINFO* psDI);
void DBGetDiscInfoRemote(MCIDEVICEID wDeviceID, 
					     DISCINFO* psDI,
						 BOOL* pbServerError);
void DBDelete(DISCINFO* psDI);
void DBGetAlias(const char* pzCheckID, char* pzDestID);
void DBInit();
void DBFree();
void DBFixCDDBStrings(BOOL bToCDDB, 
                      DISCINFO* psDI);

// DBDLG stuff
BOOL DBOpen();
void DBClose();
BOOL DBIsEnd();
BOOL DBGetDBID(char* pzID, DISCINFO* psDI);
