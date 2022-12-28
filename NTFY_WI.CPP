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

//
// This file contains the initialization of the function pointers we 
// get from WININET.DLL. This is done because we don't want to 
// require WININET.DLL from users that don't want to use the remote
// database capability.
//

#include "ntfy_cd.h"
#include "ntfy_wininet.h"

#include "misc.h"

#define GETPROC(varname, funcname) 	pf##varname = (NTFY_##varname)			\
									GetProcAddress(hWININETDLL, funcname);	\
						if (!pf##varname)									\
							return FALSE;									\



BOOL InitWININET()
{
	// Try to load WININET.DLL
	hWININETDLL = LoadLibrary("WININET.DLL");
	if (hWININETDLL == NULL)
		return FALSE;

	// Try to get function pointers
	GETPROC(InternetOpen, "InternetOpenA")
	GETPROC(InternetCloseHandle, "InternetCloseHandle")
	GETPROC(InternetConnect, "InternetConnectA")

	// Initialize WININET

	hInternetSession = pfInternetOpen(APPNAME, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternetSession == NULL) {
		DebugPrintf("InternetOpen failed: %08X", GetLastError());

		return FALSE;
	}
	


	return TRUE;
}


void FreeWININET()
{
	pfInternetCloseHandle(hInternetSession);

	FreeLibrary(hWININETDLL);
}
