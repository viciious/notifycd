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
// Precompiled headers...
//

#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable:4201)
#pragma warning(disable:4514)

#include <windows.h>
#include <wininet.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <process.h>
#include <winsock.h>
#include <dbt.h>
#include <ras.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "clist.h"

