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

/////////////////////////////////////////////////////////////////////
//
// DEFINES
//
/////////////////////////////////////////////////////////////////////

#define BETA							// Remove when retail
#define VERSION  "1.60beta5"

#define CURR_CONFIG_VERSION 160

#ifndef BETA
#define VERSION_VERSION		16000		// Used to determine if a new version exists
										// in format NNnnxx where NN is major, nn minor and
										// xx build number or something 
#else
#define VERSION_VERSION		00004		// Special case for BETA versions 
#endif

#define MAIL_URL			"mailto:mlt@cyberdude.com"
#define HOMEPAGE_URL		"http://www.artech.se/~mlt/software"

#define VERSION_SERVER		"www.artech.se"
#ifndef BETA
	#define VERSION_PATH		"/~mlt/software/version.txt"
#else
	#define VERSION_PATH		"/~mlt/software/version_beta.txt"
#endif

#define APPNAME "Notify CD Player"

#define MYWM_NOTIFYICON		(WM_APP+100)
#define MYWM_WAKEUP			(WM_APP+101)

#define IDC_NOTIFY          1001
#define IDM_TRACKS          1000
#define IDM_DEVICES         2000


/////////////////////////////////////////////////////////////////////
//
// CURRENT VERSION OPTIONS
//
/////////////////////////////////////////////////////////////////////

#define OPTIONS_STOPONEXIT				0x00000001
#define OPTIONS_STOPONSTART				0x00000002
#define OPTIONS_EXITONCDREMOVE			0x00000004
#define OPTIONS_PREVALWAYSPREV			0x00000008
#define OPTIONS_REMEMBERSTATUS			0x00000010   
#define OPTIONS_NOINSERTNOTIFICATION	0x00000020
#define OPTIONS_TRACKSMENUCOLUMN		0x00000040
#define OPTIONS_NOMENUBITMAP			0x00000080
#define OPTIONS_NOMENUBREAK				0x00000100
#define OPTIONS_USECDDB					0x00000200
#define OPTIONS_QUERYLOCAL				0x00000400
#define OPTIONS_QUERYREMOTE				0x00000800
#define OPTIONS_STORELOCAL				0x00001000
#define OPTIONS_STORERESULT				0x00002000
#define OPTIONS_SHOWONCAPTION			0x00004000
#define OPTIONS_USEFONT					0x00008000
#define OPTIONS_ARTISTINMENU			0x00010000
#define OPTIONS_AUTOADDQUEUE			0x00020000
#define OPTIONS_AUTOCHECKFORNEWVERSION  0x00040000
#define OPTIONS_AUTORETRIEVEQUEUE       0x00080000

#define OPTIONS_CDDB_STORECOPYININI	    0x00000001
#define OPTIONS_CDDB_USEHTTP    	    0x00000002
#define OPTIONS_CDDB_USEPROXY   	    0x00000004
#define OPTIONS_CDDB_USEAUTHENTICATION  0x00000008
#define OPTIONS_CDDB_ASKFORPASSWORD     0x00000010

#define OPTIONS_RAS_USERAS				0x00000001
#define OPTIONS_RAS_AUTOCONNECT			0x00000002
#define OPTIONS_RAS_QUERYIFCONNECTED    0x00000004
#define OPTIONS_RAS_USEANYCONNECTION	0x00000008
#define OPTIONS_RAS_CLOSECONNECTION     0x00000010

/////////////////////////////////////////////////////////////////////
//
// VERSION 1.21 OPTIONS
//
/////////////////////////////////////////////////////////////////////

#define V121_SETTINGS_STOPONEXIT               1
#define V121_SETTINGS_STOPONSTART              2
#define V121_SETTINGS_EXITONCDREMOVE           4
#define V121_SETTINGS_PREVALWAYSPREV           8
#define V121_SETTINGS_REMEMBERSTATUS          16
#define V121_SETTINGS_NOINSERTNOTIFICATION    32
#define V121_SETTINGS_TRACKSMENUCOLUMN        64
#define V121_SETTINGS_NOMENUBITMAP           128
#define V121_SETTINGS_NOMENUBREAK            256

#define V121_OPTIONS_QUERYLOCAL          1
#define V121_OPTIONS_QUERYREMOTE         2
#define V121_OPTIONS_STORELOCAL          4
#define V121_OPTIONS_NOTRAYICON          8
#define V121_OPTIONS_STORERESULT        16
#define V121_OPTIONS_USEHTTP			32
#define V121_OPTIONS_USEPROXY		    64

/////////////////////////////////////////////////////////////////////
//
// VERSION 1.50 OPTIONS
//
/////////////////////////////////////////////////////////////////////

#define V150_OPTIONS_TIME_TRACK         1
#define V150_OPTIONS_TIME_TRACKREM      2
#define V150_OPTIONS_TIME_CD            4
#define V150_OPTIONS_TIME_CDREM         8

#define V150_OPTIONS_CDINFO_ARTIST		    1
#define V150_OPTIONS_CDINFO_TITLE		    2
#define V150_OPTIONS_CDINFO_TRACKTITLE       4
#define V150_OPTIONS_CDINFO_TRACKLENGTH      8
#define V150_OPTIONS_CDINFO_TRACKNO			16

/////////////////////////////////////////////////////////////////////
//
// TIME CONTROL CODES
//
/////////////////////////////////////////////////////////////////////

#define TIME_TRACK         1
#define TIME_TRACKREM      2
#define TIME_CD            4
#define TIME_CDREM         8

/////////////////////////////////////////////////////////////////////
//
// STATUS CODES
//
/////////////////////////////////////////////////////////////////////

#define STATUS_RANDOM               1
#define STATUS_REPEAT               2

struct CDDB_SERVER {
	char zSite[128];
	char zProtocol[10];
	int nPort;
	char zAddress[128];         // URL for HTTP
	char zLatitude[10];
	char zLongitude[10];
	char zDescription[128];
};


// Disc info
typedef struct {
    char zMCIID[32];					// MCI ID
	char zCDDBID[32];					// CDDB ID
    BOOL bDiscFound;					// Used by the CDDB queries
//	BOOL bFoundLocal;					// Entry found in local query
//    BOOL bFoundRemote;					// Entry found in remote query
    char* pzArtist;						// Artist name
	char* pzTitle;						// Disc title
	char* pzCategory;					// Disc category
	char* pzDiscExt;					// Extended disc information
	char** ppzTracks;					// Track names
	char** ppzTracksExt;				// Extended track information
	unsigned int nProgrammedTracks;		// Number of programmed tracks
	unsigned int *pnProgrammedTracks;	// Array of programmed tracks
    char* pzOrder;						// Playorder
	char** ppzTrackLen;					// Track length as text
	unsigned int* pnTrackLen;			// Track length in seconds
	unsigned int nFrameOffset;			// Offset in frames to first track
	unsigned int nCurrTrack;			// Current track
	unsigned int nMCITracks;			// Number of tracks on disc according to MCI
	unsigned int* pnFrames;				// Track offsets in frames
	unsigned long nDiscLenMS;			// Disc length in milliseconds
	unsigned int nDiscLength;			// Disc length in seconds
    char* pzDiscid;						// Discid from the CDDB entry
    unsigned int nTracks;				// Number of tracks in CDDB entry
    unsigned int nRevision;				// Revision from the CDDB entry
    char* pzSubmitted;					// Submission info from the CDDB entry
} DISCINFO;

/////////////////////////////////////////////////////////////////////
//
// GLOBALSTRUCT
//
/////////////////////////////////////////////////////////////////////

typedef struct {
	BOOL bLogfile;

	HINSTANCE hMainInstance;
	HWND hMainWnd;

	OSVERSIONINFO sVersionInfo;

    MCIDEVICEID wDeviceID;

	// 
	// Options
	//

	int nOptions;

	char szTooltipFormat[256];
	char szCaptionFormat[256];

	HICON hTrayIcon;                    // Current Icon
	HICON hIconNoAudio;
	HICON hIconStop;
	HICON hIconPause;
	HICON hIconPlay;

	HMENU hTrackMenu;

	unsigned int nClicks;
	unsigned int nPollTime;
	unsigned int nStatus;

	int nCaptureX;
	int nCaptureY;
	BOOL bCaptured;
	BOOL bDragged;

	LOGFONT sCaptionFont;
	unsigned int nLastCaptionLen;
    unsigned int nLastCaptionXPos;
	DWORD nCaptionFontColor;

	// Menu indexes

	unsigned int nMenuIndexTracks;
	unsigned int nMenuIndexOther;
	unsigned int nMenuIndexDevices;

	#define NUM_BINDINGS 9

	unsigned int anBindings[NUM_BINDINGS];
	unsigned int anHotkeys[NUM_BINDINGS];

	// Disable some dialogs when they are open!
	BOOL bInInfoDlg;
	BOOL bInDBDlg;
	BOOL bInSkipDlg;
	BOOL bInSetAbsTrackPosDlg;
	BOOL bInOptionsDlg;
	BOOL bInAboutDlg;

	// Misc stuff
	char szToolTip[512];
	char** ppzCategories;
	unsigned int nNumCategories;
	BOOL abDevices[40];                  // TRUE if device is a CD-ROM, FALSE otherwise
	unsigned int nCurrentDevice;
	unsigned int nDefaultDevice;
	unsigned int nNumberOfDevices;
	unsigned int nMenuBreak;
	unsigned int nRepeatTrack;
	unsigned int nNextProgrammedTrack;
	HFONT hMenuFont;
	HBITMAP hMenuBitmap;
	unsigned int* pnLastRandomTracks;
	unsigned int nLastRandomTrack;
	char zExternalCommand[MAX_PATH+1];
	BOOL bHasNotification;
	BOOL bDebug;
	BOOL bCreateWindowFinished;
	BOOL bFirstStatusCheckDone;
	BOOL bMultiselectCategories;
	BOOL bBrokenTracksMenu;
	unsigned int nNumberOfItemsInQueue;

    CRITICAL_SECTION sDiscInfoLock;

    DISCINFO sQueryThreadDI;
    BOOL bQueryThreadHasUpdatedDI;

    // State
	struct {
		BOOL bCDOpened;
		BOOL bPaused;
		BOOL bPlaying;
		BOOL bInit;
		BOOL bAudio;
		BOOL bRepeat;
		BOOL bRandomize;
		BOOL bMediaPresent;
		BOOL bProgrammed;
		BOOL bRepeatTrack;
		BOOL bPlayWhole;
	} state;

	// Disc info
	DISCINFO di[1];

	// CDDB stuff
	struct {
		unsigned int nCDDBOptions;
		unsigned int nCDDBType;

		char zCDDBPath[256];
		char zRemoteServer[256];
		char zRemoteHTTPPath[256];
		char zRemoteProxyServer[256];
		char zRemoteEmailServer[256];
		char zEmailAddress[256];
		char zDomain[256];
		unsigned int nRemotePort;
		unsigned int nRemoteProxyPort;
		unsigned int nRemoteTimeout;
		char zProxyUser[256];
		char zProxyPassword[256];
        char zUserAgent[80];

		struct CDDB_SERVER* psCDDBServers;
		unsigned int nNumCDDBServers;
	} cddb;

	// RAS stuff
	struct {
		int nOptions;
		char szConnectionName[RAS_MaxEntryName + 1];
		HRASCONN hCurrentConnection;
		
		// Dynamic stuff
		HINSTANCE hLib;
		DWORD (APIENTRY *RasEnumEntries)(LPSTR, LPSTR, LPRASENTRYNAME, LPDWORD, LPDWORD);
		DWORD (APIENTRY *RasEnumConnections)( LPRASCONNA, LPDWORD, LPDWORD );
		DWORD (APIENTRY *RasDial)( LPRASDIALEXTENSIONS, LPSTR, LPRASDIALPARAMSA, DWORD,
                   LPVOID, LPHRASCONN );
		DWORD (APIENTRY *RasHangUp)( HRASCONN );
		DWORD (APIENTRY *RasGetConnectStatus)( HRASCONN, LPRASCONNSTATUSA );
		DWORD (APIENTRY *RasGetErrorString)( UINT, LPSTR, DWORD );
		DWORD (APIENTRY *RasEditPhonebookEntry)( HWND, LPSTR, LPSTR );
		DWORD (APIENTRY *RasCreatePhonebookEntry)( HWND, LPSTR );
		DWORD (APIENTRY *RasGetEntryDialParams)( LPSTR, LPRASDIALPARAMSA, LPBOOL );
		DWORD (APIENTRY *RasSetEntryDialParams)( LPSTR, LPRASDIALPARAMSA, BOOL );
	} ras;
} GLOBALSTRUCT;
