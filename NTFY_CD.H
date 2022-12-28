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

#define VERSION  "1.51.3"

#define CURR_CONFIG_VERSION 150
#define VERSION_VERSION		15104		// Used to determine if a new version exists
										// in format NNnnxx where NN is major, nn minor and
										// xx build number or something 

#define VERSION_SERVER		"www.artech.se"
#define VERSION_PATH		"/~mlt/software/version.txt"

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

#define OPTIONS_TIME_TRACK         1
#define OPTIONS_TIME_TRACKREM      2
#define OPTIONS_TIME_CD            4
#define OPTIONS_TIME_CDREM         8

#define OPTIONS_CDINFO_ARTIST		    1
#define OPTIONS_CDINFO_TITLE		    2
#define OPTIONS_CDINFO_TRACKTITLE       4
#define OPTIONS_CDINFO_TRACKLENGTH      8
#define OPTIONS_CDINFO_TRACKNO			16

#define OPTIONS_STOPONEXIT                1
#define OPTIONS_STOPONSTART               2
#define OPTIONS_EXITONCDREMOVE            4
#define OPTIONS_PREVALWAYSPREV            8
#define OPTIONS_REMEMBERSTATUS           16
#define OPTIONS_NOINSERTNOTIFICATION     32
#define OPTIONS_TRACKSMENUCOLUMN         64
#define OPTIONS_NOMENUBITMAP            128
#define OPTIONS_NOMENUBREAK             256
#define OPTIONS_USECDDB			        512
#define OPTIONS_QUERYLOCAL             1024
#define OPTIONS_QUERYREMOTE            2048
#define OPTIONS_STORELOCAL             4096
#define OPTIONS_STORERESULT            8192
#define OPTIONS_SHOWONCAPTION         16384
#define OPTIONS_USEFONT               32768
#define OPTIONS_ARTISTINMENU	      65536

#define OPTIONS_CDDB_STORECOPYININI	      1
#define OPTIONS_CDDB_USEHTTP    	      2
#define OPTIONS_CDDB_USEPROXY   	      4
#define OPTIONS_CDDB_USEAUTHENTICATION    8
#define OPTIONS_CDDB_ASKFORPASSWORD      16

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
// STATUS CODES
//
/////////////////////////////////////////////////////////////////////

#define STATUS_RANDOM               1
#define STATUS_REPEAT               2

/////////////////////////////////////////////////////////////////////
//
// VARIABLES
//
/////////////////////////////////////////////////////////////////////

#ifdef ALLOCATE
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL BOOL bLogfile;

GLOBAL HINSTANCE hMainInstance;
GLOBAL HWND hMainWnd;

GLOBAL MCI_STATUS_PARMS sMCIStatus;
GLOBAL MCI_INFO_PARMS sMCIInfo;
GLOBAL MCI_OPEN_PARMS sMCIOpen;
GLOBAL MCI_PLAY_PARMS sMCIPlay;
GLOBAL MCI_SET_PARMS sMCISet;
GLOBAL MCI_GENERIC_PARMS sMCIGeneric;

GLOBAL char szMCIReturnString[80];

// 
// Options
//

GLOBAL int nOptions;
GLOBAL int nCDDBOptions;
GLOBAL int nCDDBType;
GLOBAL int nTooltipTimeOptions;
GLOBAL int nTooltipCDInfoOptions;
GLOBAL int nCaptionTimeOptions;
GLOBAL int nCaptionCDInfoOptions;

GLOBAL HICON hTrayIcon;                    // Current Icon
GLOBAL HICON hIconNoAudio;
GLOBAL HICON hIconStop;
GLOBAL HICON hIconPause;
GLOBAL HICON hIconPlay;

GLOBAL HMENU hTrackMenu;

GLOBAL int nCurrTrack;
GLOBAL int nMaxTrack;
GLOBAL int nClicks;
GLOBAL int nPollTime;
GLOBAL int nStatus;
GLOBAL int nProgrammedTracks;
GLOBAL int nLeftButtonNext;
GLOBAL int nLeftButtonPause;
GLOBAL int nLeftButtonPrev;
GLOBAL int nLeftButtonStop;
GLOBAL int nCategoryChoosen;

GLOBAL LOGFONT sCaptionFont;
GLOBAL int nLastCaptionLen;
GLOBAL DWORD nCaptionFontColor;

// Menu indexes

GLOBAL int nMenuIndexTracks;
GLOBAL int nMenuIndexOther;
GLOBAL int nMenuIndexDevices;

GLOBAL BOOL bCDOpened;
GLOBAL BOOL bPaused;
GLOBAL BOOL bPlaying;
GLOBAL BOOL bInit;
GLOBAL BOOL bAudio;
GLOBAL BOOL bRepeat;
GLOBAL BOOL bRandomize;
GLOBAL BOOL bMediaPresent;
GLOBAL BOOL bProgrammed;
GLOBAL BOOL bRepeatTrack;
GLOBAL BOOL bPlayWhole;
GLOBAL BOOL bChooseAllPresent;
GLOBAL BOOL bBrokenTracksMenu;
GLOBAL BOOL bCreateWindowFinished;
GLOBAL BOOL bFirstStatusCheckDone;

// Disable some dialogs when they are open!
GLOBAL BOOL bInInfoDlg;
GLOBAL BOOL bInDBDlg;
GLOBAL BOOL bInSkipDlg;
GLOBAL BOOL bInSetAbsTrackPosDlg;
GLOBAL BOOL bInOptionsDlg;
GLOBAL BOOL bInAboutDlg;

GLOBAL BOOL bHasNotification;

GLOBAL BOOL bDebug;

GLOBAL BOOL bInDBDLGBuildList;

// Disc info
GLOBAL char* pzArtist;
GLOBAL char* pzTitle;
GLOBAL char* pzDiscExt;
GLOBAL char** ppzTracks;
GLOBAL char** ppzTracksExt;
GLOBAL char** ppzTrackLen;
GLOBAL int* pnTrackLen;
GLOBAL int *pnProgrammedTracks;
GLOBAL char* pzCategory;

GLOBAL char zCurrMCIID[32];
GLOBAL char zCurrCDDBID[32];

GLOBAL int nFrameOffset;
GLOBAL char szToolTip[512];
GLOBAL char szTime[32];
GLOBAL char** ppzCategories;
GLOBAL int nNumCategories;
GLOBAL BOOL abDevices[40];                  // TRUE if device is a CD-ROM, FALSE otherwise
GLOBAL int nCurrentDevice;
GLOBAL int nDefaultDevice;
GLOBAL int nNumberOfDevices;
GLOBAL int nMenuBreak;
GLOBAL int nRepeatTrack;
GLOBAL int nNextProgrammedTrack;
GLOBAL HFONT hMenuFont;
GLOBAL HBITMAP hMenuBitmap;
GLOBAL int* pnLastRandomTracks;
GLOBAL int nLastRandomTrack;
GLOBAL long nDiscLenMS;
GLOBAL char zExternalCommand[MAX_PATH+1];

// CDDB stuff

GLOBAL char zCDDBPath[256];
GLOBAL char zRemoteServer[256];
GLOBAL char zRemoteHTTPPath[256];
GLOBAL char zRemoteProxyServer[256];
GLOBAL char zRemoteEmailServer[256];
GLOBAL char zEmailAddress[256];
GLOBAL char zDomain[256];
GLOBAL int nRemotePort;
GLOBAL int nRemoteProxyPort;
GLOBAL int nRemoteTimeout;
GLOBAL char zProxyUser[256];
GLOBAL char zProxyPassword[256];
