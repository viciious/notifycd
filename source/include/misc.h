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

#ifndef __MISC_H__
#define __MISC_H__

/////////////////////////////////////////////////////////////////////
//
// PROTOTYPES
//
/////////////////////////////////////////////////////////////////////

void StringCpyZ( char *pzDest, const char *pzSrc, size_t size );
void StringCatZ( char *pzDest, const char *pzSrc, size_t size );
void StringPrintf( char *pzDest, size_t size, const char *pzFmt, ... );
char *StringCopy( const char *pzStr );

void Notify_CreatePath( const char *path );

void UpdateTooltipOrCaption(DISCINFO* psDI,
							BOOL bTooltipOrCaption, 
							char* pzStr);
void UpdateDiscInformation(DISCINFO* psDI, BOOL bNotify, BOOL bTooltip, char* pzStr);
void SaveConfig();
void CenterWindow(HWND hWnd, BOOL bVertical = TRUE);
void MarkExport();
void InitTree(HWND hWnd);
void DebugPrintf(const char* pzFormat, ...);
void FixCDDBString(BOOL bToCDDB, char** ppzStr);
void CheckAmpersand(char* pzStr, BOOL bTooltip);
void ChangeDefButton(HWND hDlg, int nIDSet, int nIDRemove);
void AppendString(char** ppzPtr, const char* pzStr, int nLen);
void UpdateTrackCombo(HWND hWnd, int nID);
void EncodeBase64(char* pzDest, ULONG nLen, const char* pxData);
#ifdef SOCK
BOOL GetString(SOCKET s, char* pzStr, int nLen, BOOL bNoClose = FALSE);
int SendString(SOCKET s, char* pzStr);
int SendAuthentication(SOCKET s);
DWORD GetAddress(char* pzMachine);
#endif 
BOOL CheckForNewVersion(BOOL bUnattended);
DWORD __stdcall CheckVersionThread(LPVOID);
void SetHotkeys();
void ParseServerInfo(CDDB_SERVER* psServer, char* pzStr);
void CheckProgrammed(MCIDEVICEID wDeviceID, 
                     DISCINFO* psDI);
void InitMenu(DISCINFO* psDI);
void InitTracksMenu(DISCINFO* psDI);
unsigned int CDGetLastTrackInARow(unsigned int nTrack);
void RunExternalCommand(DISCINFO* psDI);
void ParseDiscInformationFormat(DISCINFO* psDI,
								const char* pzFormat, 
								char* pzDest);
void DrawOnCaption();

DWORD ProfileGetInt( const char *pzAppName, const char *pzKeyName, int nDefault );
BOOL ProfileWriteInt( const char *pzAppName, const char *pzKeyName, int nNum );
DWORD ProfileGetString( const char *pzAppName, const char *pzKeyName, const char *pzDefault, char *pzReturnedString, unsigned int nSize );
BOOL ProfileWriteString( const char *pzAppName, const char *pzKeyName, char *pzString );
DWORD ProfileGetStruct( const char *pzAppName, const char *pzKeyName, void *pzStruct, unsigned int nSize );
BOOL ProfileWriteStruct( const char *pzAppName, const char *pzKeyName, void *pzStruct, unsigned int nSize );
DWORD ProfileGetSection( const char *pzAppName, char *pzString, unsigned int nSize );
BOOL ProfileWriteSection( const char *pzAppName, char *pzString );
void GetTmpFile(char* pzFile);

// Tray

void NotifyAdd(HWND hWnd, UINT nID, HICON hIcon, char* pzStr);
void NotifyModify(HWND hWnd, UINT nID, HICON hIcon, char* pzStr);
void NotifyDelete(HWND hWnd, UINT nID);

// Progress

void ProgressOpen(HWND hParent, char* pzStr, int nStyle, DWORD dwMax);
void ProgressClose();
void ProgressSet(int nCount);
void ProgressSetStr(char* pzStr);
		  
// Queue

BOOL AddToQueue(DISCINFO* psDI);
void RemoveQueueItem(int nItem);
int RetrieveQueueItem(int nItem);
void GetQueuedItems();

// Discinfo

void FreeDiscInfo(DISCINFO* psDI);
void CopyDiscInfo(DISCINFO* psDIDest, 
                  DISCINFO* psDISrc);
void InitDiscInfo(DISCINFO* psDI);
void CopyQueryInfo(DISCINFO* psSrc, 
                   DISCINFO* psDst);
void GetDiscInfo(MCIDEVICEID wDeviceID, 
                 DISCINFO* psDI);
BOOL SetDiscInfo(DISCINFO* psDI);
void ValidateDiscInfo(MCIDEVICEID wDeviceID,
                      DISCINFO* psDI);
void ParsePlaylist(DISCINFO* psDI, BOOL bProgram);
void ResetPlaylist(DISCINFO* psDI, BOOL bProgram);
void DiscInit(DISCINFO* psDI);

#endif //__MISC_H__

