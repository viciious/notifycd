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

#ifndef __APP_H__
#define __APP_H__

#define APPNAME				"Notify CD Player"
#define APPNAME_NOSPACES	"NotifyCDPlayer"

//#define BETA				// Remove when retail

#ifdef BETA
	#define VERSION			"1.60 beta 66"
#else
	#define VERSION			"1.60"
#endif

#define CURR_CONFIG_VERSION 160

#ifndef BETA
#define VERSION_VERSION		16000		// Used to determine if a new version exists
										// in format NNnnxx where NN is major, nn minor and
										// xx build number or something 
#else
#define VERSION_VERSION		00004		// Special case for BETA versions 
#endif

#define MAIL_ADDRESS		"mlt@cyberdude.com"
#define HOMEPAGE_URL		"http://www.artech.se/~mlt/software"

#define VERSION_SERVER		"www.artech.se"

#define COPYRIGHT_NOTICE	"Copyright (c) 1996-1998, Mats Ljungqvist"

#ifndef BETA
	#define VERSION_PATH	"/~mlt/software/version.txt"
#else
	#define VERSION_PATH	"/~mlt/software/version_beta.txt"
#endif

#define PROFILENAME			"cdplayer.ini"
#define LOGNAME				"notify.log"
#define TEMPNAME			"notify.tmp"

#endif //__APP_H__
