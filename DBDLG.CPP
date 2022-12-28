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

#define STRICT
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable:4201)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>
#include <commdlg.h>

#include "resource.h"
#include "clist.h"

#include "ntfy_cd.h"
#include "misc.h"
#include "db.h"
#include "dbdlg.h"
#include "cddb.h"

extern CDDB_DISC_ENTRY sEntry;

/////////////////////////////////////////////////////////////////////
//
// DATABASE EDITOR
//
/////////////////////////////////////////////////////////////////////

BOOL CALLBACK CategoryChooseDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

struct sTrack {
    sTrack() {
        pzName = NULL;
    }
    ~sTrack() {
        if (pzName) 
            free(pzName);
        pzName = NULL;
    }

    char xType;
    char* pzName;
    BOOL bChanged;
};

struct sArtist;

struct sCD {
    sCD() {
        pzName = pzCategory = NULL;
        bChanged = bCategoryChanged = FALSE;
    }
    ~sCD() {
        if (pzName) 
            free(pzName);
        if (pzCategory)
            free(pzCategory);
        if (pnFrames)
            free(pnFrames);
        pzName = pzCategory = NULL;
        pnFrames = NULL;
    }
    
    char xType;
    char zID[32];
    char* pzName;
    char* pzCategory;
    BOOL bRead;
    BOOL bChanged;
    BOOL bCategoryChanged;
    int nDiscLength;
    int* pnFrames;
    int nNumFrames;
    sArtist* psArtist;
    cList<sTrack> oTrackList;
};

struct sArtist {
    sArtist() {
        pzName = NULL;
    }
    ~sArtist() {
        if (pzName) 
            free(pzName);
        pzName = NULL;
    }

    char xType;
    char* pzName;
    BOOL bChanged;
    cList<sCD> oCDList;
};

cList<sArtist>* poArtistList;
int nNumArtists;
int nNumCDs;

int SortArtistCB(void* pvPtr1, void* pvPtr2)
{
    sArtist* psArtist1 = (sArtist*) pvPtr1;
    sArtist* psArtist2 = (sArtist*) pvPtr2;

    return stricmp(psArtist1->pzName, psArtist2->pzName);
}


int SortCDCB(void* pvPtr1, void* pvPtr2)
{
    sCD* psCD1 = (sCD*) pvPtr1;
    sCD* psCD2 = (sCD*) pvPtr2;

    return stricmp(psCD1->pzName, psCD2->pzName);
}


BOOL BuildList(HWND hWnd)
{
    TV_INSERTSTRUCT sTVInsertArtist;
	HWND hTree = GetDlgItem(hWnd, IDC_TREE);
    char* pzDBArtist = NULL;
    char* pzDBTitle = NULL;
    char* pzDBCategory = NULL;
    char zID[32];
    int nCount = 0;
    sArtist* psArtist;
    sCD* psCD;

    if (!DBOpen())
        return FALSE;

    bInDBDLGBuildList = TRUE;

    nNumArtists = nNumCDs = 0;

    poArtistList = new cList<sArtist>;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    ProgressOpen(NULL, "Reading discs", TRUE, 0);
    ProgressSet(0);

    TreeView_DeleteAllItems(GetDlgItem(hWnd, IDC_TREE));

    sTVInsertArtist.hParent = TVI_ROOT;
    sTVInsertArtist.hInsertAfter = TVI_LAST;
    sTVInsertArtist.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
    sTVInsertArtist.item.pszText = LPSTR_TEXTCALLBACK;
    sTVInsertArtist.item.cChildren = 1;


    do {
        if (pzDBArtist)
            free(pzDBArtist);
        if (pzDBTitle)
            free(pzDBTitle);
        if (pzDBCategory)
            free(pzDBCategory);

        pzDBArtist = NULL;
		pzDBTitle = NULL;
		pzDBCategory = NULL;

		if (DBGetDBID(zID, &pzDBArtist, &pzDBTitle, &pzDBCategory)) {
			if (pzDBArtist && *pzDBArtist) {
				// Find existing artist!

				psArtist = poArtistList->First();
				while(psArtist) {
					if (!strcmp(psArtist->pzName, pzDBArtist))
						break;
    
					psArtist = poArtistList->Next();
				}

				// If not found, add new
				if (!psArtist) {
					psArtist = new sArtist;

					psArtist->pzName = strdup(pzDBArtist);
					psArtist->xType = 0;
					psArtist->bChanged = FALSE;
					poArtistList->Add(psArtist);

					nNumArtists ++;

                    sTVInsertArtist.item.lParam = (LONG) psArtist;
        
                    TreeView_InsertItem(hTree, &sTVInsertArtist);
				}

				// Add CD to artist CD list
				psCD = new sCD;
				psCD->pzName = strdup(pzDBTitle);
				psCD->pzCategory = strdup(pzDBCategory);
				strcpy(psCD->zID, zID);
				psCD->xType = 1;
				psCD->bRead = FALSE;
				psCD->bChanged = FALSE;
                psCD->nNumFrames = 0;
                psCD->pnFrames = NULL;
                psCD->nDiscLength = 0;
                psCD->psArtist = psArtist;

				nNumCDs ++;

                psArtist->oCDList.Add(psCD);
			}

            nCount ++;
            if (!(nCount % 10))
                ProgressSet(nCount);
        }
    } while(DBIsEnd());

    bInDBDLGBuildList = FALSE;

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    ProgressClose();

    return TRUE;
}


void ReadTracks(sCD* psCD)
{
    int nTracks;
    int nLoop;
    sTrack* psTrack;
    char** pppzTracks;

    if (psCD->bRead)
        return;

    psCD->bRead = TRUE;

    nTracks = DBGetInfoInt(psCD->zID, psCD->zID, "NUMTRACKS");
    pppzTracks = DBGetTrackTitles(psCD->zID, psCD->zID, nTracks);
    
    for (nLoop = 0 ; nLoop < nTracks; nLoop ++) {
        psTrack = new sTrack;
        psTrack->xType = 2;
        psTrack->bChanged = FALSE;

        psTrack->pzName = strdup(pppzTracks[nLoop]);

        psCD->oTrackList.Add(psTrack);
    }

    DBFreeTrackTitles(pppzTracks, nTracks);
}


BOOL DBCheckCategory(char* pzDiscCat, char* pzChooseCat)
{
    if (!pzChooseCat[0])
        return TRUE;
    else if (!stricmp(pzDiscCat, pzChooseCat))
        return TRUE;
    else
        return FALSE;
}


void InitTree(HWND hWnd)
{
	HWND hTree = GetDlgItem(hWnd, IDC_TREE);

    TreeView_SortChildren(hTree, TVI_ROOT, FALSE);
}


void ListInitCD(HWND hWnd, sArtist* psArtist, int nItems)
{
    HWND hList = GetDlgItem(hWnd, IDC_LIST);
    LV_COLUMN sLVColumn;
    sCD* psCD;

    ListView_DeleteAllItems(hList);
    
    ListView_DeleteColumn(hList, 1);
    ListView_DeleteColumn(hList, 0);

    sLVColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    sLVColumn.cx = 200;
    sLVColumn.iSubItem = 0;
    sLVColumn.fmt = LVCFMT_LEFT;
    sLVColumn.pszText = "Title";
    ListView_InsertColumn(hList, 0, &sLVColumn);

    sLVColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    sLVColumn.cx = 80;
    sLVColumn.iSubItem = 1;
    sLVColumn.fmt = LVCFMT_LEFT;
    sLVColumn.pszText = "Category";
    ListView_InsertColumn(hList, 1, &sLVColumn);

    int nLoop = 0;

    if (psArtist) {
        psArtist->oCDList.Sort(SortCDCB);

        psCD = psArtist->oCDList.First();
    }
    else
        psCD = NULL;

    while(nItems --) {
        LV_ITEM sLVItem;

        sLVItem.mask = LVIF_TEXT | LVIF_PARAM;
        sLVItem.iItem = nLoop;
        sLVItem.iSubItem = 0;
        sLVItem.pszText = LPSTR_TEXTCALLBACK;
  		sLVItem.cchTextMax = 80;
        sLVItem.lParam = (LONG) psCD;

        ListView_InsertItem(hList, &sLVItem);

        nLoop ++;
        psCD = psArtist->oCDList.Next();
    }
}


void ListInitTracks(HWND hWnd, sCD* psCD, int nItems)
{
    HWND hList = GetDlgItem(hWnd, IDC_LIST);
    LV_COLUMN sLVColumn;
    sTrack* psTrack;

    ListView_DeleteAllItems(hList);
    
    ListView_DeleteColumn(hList, 2);
    ListView_DeleteColumn(hList, 1);
    ListView_DeleteColumn(hList, 0);

    sLVColumn.mask = LVCF_TEXT | LVCF_WIDTH;
    sLVColumn.cx = 230;
    sLVColumn.iSubItem = 0;
    sLVColumn.fmt = LVCFMT_LEFT;
    sLVColumn.pszText = "Title";
    ListView_InsertColumn(hList, 0, &sLVColumn);

    sLVColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    sLVColumn.cx = 50;
    sLVColumn.iSubItem = 1;
    sLVColumn.fmt = LVCFMT_LEFT;
    sLVColumn.pszText = "Track";
    ListView_InsertColumn(hList, 1, &sLVColumn);

    int nLoop = 0;

    if (psCD)
        psTrack = psCD->oTrackList.First();
    else
        psTrack = NULL;

    while(nItems --) {
        LV_ITEM sLVItem;

        sLVItem.mask = LVIF_TEXT | LVIF_PARAM;
        sLVItem.iItem = nLoop;
        sLVItem.iSubItem = 0;
        sLVItem.pszText = LPSTR_TEXTCALLBACK;
  		sLVItem.cchTextMax = 80;
        sLVItem.lParam = (LONG) psTrack;

        ListView_InsertItem(hList, &sLVItem);

        psTrack = psCD->oTrackList.Next();
        nLoop ++;
    }
}


BOOL SaveEntry(sCD* psCD)
{
    int nNum = psCD->oTrackList.NumItems();
    BOOL bChanged = FALSE;
    char** pppzTracks;
    sTrack* psTrack;
    int nLoop;

	// No delete needed if we use CDPLAYER.INI. In fact a delete here will screw up the INI file based database!
    if (psCD && psCD->bCategoryChanged && (nOptions & OPTIONS_USECDDB))
        DBDelete(psCD->zID, psCD->zID);

    if (psCD && psCD->bChanged) {
        DBSetInfo(psCD->zID, psCD->zID, "TITLE", psCD->pzName);
        DBSetInfo(psCD->zID, psCD->zID, "CATEGORY", psCD->pzCategory);
    }

    if (psCD && psCD->psArtist && psCD->psArtist->bChanged) 
        DBSetInfo(psCD->zID, psCD->zID, "ARTIST", psCD->psArtist->pzName);

    if (nNum && psCD) {
        psTrack = psCD->oTrackList.First();

        pppzTracks = (char**) malloc(nNum * sizeof(char*));
        for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
            pppzTracks[nLoop] = strdup(psTrack->pzName);

            if (psTrack->bChanged)
                bChanged = TRUE;

            psTrack = psCD->oTrackList.Next();
        }

        if (bChanged)
            DBSetTrackTitles(psCD->zID, psCD->zID, pppzTracks, nNum);

        for (nLoop = 0 ; nLoop < nNum ; nLoop ++)
            free(pppzTracks[nLoop]);

        free(pppzTracks);
    }

    if (bChanged || (psCD && psCD->bChanged) || (psCD && psCD->psArtist->bChanged)) {
        if (psCD->nNumFrames) {        
            if (sEntry.pnFrames) 
                free(sEntry.pnFrames);

            sEntry.pnFrames = (int*) malloc(psCD->nNumFrames*sizeof(int));

            for (nLoop = 0 ; nLoop < psCD->nNumFrames ; nLoop ++)
                sEntry.pnFrames[nLoop] = psCD->pnFrames[nLoop];

            sEntry.nDiscLength = psCD->nDiscLength;
            sEntry.nTracks = psCD->nNumFrames;

            psCD->nNumFrames = nNum;
        }

        DBSave();

        return TRUE;
    }

    return FALSE;
}


void SaveList()
{   
    sArtist* psArtist;
    sCD* psCD;
    int nCount = 0;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    ProgressOpen(NULL, "Saving changes", TRUE, 0);
    ProgressSet(0);

    if (poArtistList) {
        psArtist = poArtistList->First();
        while(psArtist) {
            psCD = psArtist->oCDList.First();
            while(psCD) {
                if (SaveEntry(psCD)) {
                    nCount ++;
                    if (!(nCount % 10))
                        ProgressSet(nCount);
                }

                psCD = psArtist->oCDList.Next();
            }

            psArtist = poArtistList->Next();
        }
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    ProgressClose();
}


BOOL bInDBEdit;

BOOL DBEditorNotifyHandler(HWND hWnd, UINT /*nMsg*/, WPARAM wParam, LPARAM lParam)
{
    switch(wParam) {
        case IDC_TREE: {
            NM_TREEVIEW *psNm = (NM_TREEVIEW*)lParam;
            TV_DISPINFO *psTVDispInfo = (TV_DISPINFO *)lParam;
            sArtist* psArtist = ((sArtist*)psTVDispInfo->item.lParam);
            sCD* psCD = ((sCD*)psTVDispInfo->item.lParam);
            
            switch(psNm->hdr.code) {
				case TVN_ITEMEXPANDING: {
                    char xType = *((char*)psNm->itemNew.lParam);

					if (xType == 0) {
			            sArtist* psArtist = ((sArtist*)psNm->itemNew.lParam);
	
						if ((psNm->action & TVE_EXPAND) && 
							!TreeView_GetChild(GetDlgItem(hWnd, IDC_TREE), psNm->itemNew.hItem)) {
						    TV_INSERTSTRUCT sTVInsertCD;

							sTVInsertCD.hParent = psNm->itemNew.hItem;
							sTVInsertCD.hInsertAfter = TVI_SORT;
							sTVInsertCD.item.mask = TVIF_TEXT | TVIF_PARAM;
							sTVInsertCD.item.pszText = LPSTR_TEXTCALLBACK;

							psCD = psArtist->oCDList.First();
							while(psCD) {
								sTVInsertCD.item.lParam = (LONG) psCD;
    
								TreeView_InsertItem(GetDlgItem(hWnd, IDC_TREE), &sTVInsertCD);

								psCD = psArtist->oCDList.Next();
							}

						}
					}
				}
				break;

				case TVN_SELCHANGED: {
                    // Check type and init list columns, add items etc
                    char xType = *((char*)psNm->itemNew.lParam);

                    if (xType == 0) {
                        ListInitCD(hWnd, ((sArtist*)psNm->itemNew.lParam), ((sArtist*)psNm->itemNew.lParam)->oCDList.NumItems());
                    }
                    else if (xType == 1) {
                        ReadTracks(((sCD*)psNm->itemNew.lParam));

                        ListInitTracks(hWnd, ((sCD*)psNm->itemNew.lParam), ((sCD*)psNm->itemNew.lParam)->oTrackList.NumItems());
                    }
                    
                    ListView_RedrawItems(GetDlgItem(hWnd, IDC_LIST), 0, 128);
                }
                break;

                case TVN_BEGINLABELEDIT: {
                    char xType = *((char*)psTVDispInfo->item.lParam);

                    bInDBEdit = TRUE;

                    HWND hEdit = TreeView_GetEditControl(GetDlgItem(hWnd, IDC_LIST));
//                    if (hEdit)
//                        SendMessage(hEdit, EM_SETLIMITTEXT, (WPARAM)60, 0);
                }
                break;

                case TVN_ENDLABELEDIT: {
                    char xType = *((char*)psTVDispInfo->item.lParam);

                    bInDBEdit = FALSE;

                    if (psTVDispInfo->item.pszText == NULL)
                        return FALSE;

                    if (xType == 1) {
                        free(psCD->pzName);

                        psCD->pzName = strdup(psTVDispInfo->item.pszText);
                        psCD->bChanged = TRUE;
                    }
                    else if (xType == 0) {
                        free(psArtist->pzName);

                        psArtist->pzName = strdup(psTVDispInfo->item.pszText);
                        psArtist->bChanged = TRUE;
                    }
                }
                break;

                case TVN_GETDISPINFO: {
                    char xType = *((char*)psTVDispInfo->item.lParam);

                    if (psTVDispInfo->item.mask & TVIF_TEXT) {
						if (xType == 0) {
							strncpy(psTVDispInfo->item.pszText, psArtist->pzName, psTVDispInfo->item.cchTextMax);
						}
						else if (xType == 1) {
							strncpy(psTVDispInfo->item.pszText, psCD->pzName, psTVDispInfo->item.cchTextMax);
						}
					}
                }
                break;
            }
        }
        break;

        case IDC_LIST: {
            LV_DISPINFO *psLVDispInfo = (LV_DISPINFO *)lParam;
            NM_LISTVIEW *psNm = (NM_LISTVIEW *)lParam;
            sCD* psCD = ((sCD*)psLVDispInfo->item.lParam);
            sTrack* psTrack = ((sTrack*)psLVDispInfo->item.lParam);
            
            switch(psLVDispInfo->hdr.code) {
                case LVN_BEGINLABELEDIT: {
                    HWND hEdit = ListView_GetEditControl(GetDlgItem(hWnd, IDC_LIST));

//                    SendMessage(hEdit, EM_SETLIMITTEXT, (WPARAM)60, 0);
                }
                break;

                case LVN_ENDLABELEDIT: {
                    char xType = *((char*)psLVDispInfo->item.lParam);

                    if (psLVDispInfo->item.iItem == -1 || psLVDispInfo->item.pszText == NULL)
                        return FALSE;

                    switch (psLVDispInfo->item.iSubItem) {
                        case 0: {
                            if (xType == 1) {
                                free(psCD->pzName);

                                psCD->pzName = strdup(psLVDispInfo->item.pszText);
                                psCD->bChanged = TRUE;
                            }
                            else if (xType == 2) {
                                free(psTrack->pzName);

                                psTrack->pzName = strdup(psLVDispInfo->item.pszText);
                                psTrack->bChanged = TRUE;
                            }
                        }
                        break;
                    }

                    ListView_RedrawItems(GetDlgItem(hWnd, IDC_LIST), 0, 128);
                }
                break;

                case LVN_GETDISPINFO: {
                    char xType = *((char*)psLVDispInfo->item.lParam);

                    switch (psLVDispInfo->item.iSubItem) {
                        case 0:
                            if (xType == 1)
                                strncpy(psLVDispInfo->item.pszText, psCD->pzName, psLVDispInfo->item.cchTextMax);
                            else if (xType == 2)
                                strncpy(psLVDispInfo->item.pszText, psTrack->pzName, psLVDispInfo->item.cchTextMax);
                        break;

                        case 1:
                            if (xType == 1)
                                strncpy(psLVDispInfo->item.pszText, psCD->pzCategory, psLVDispInfo->item.cchTextMax);
                            else if (xType == 2) {
                                char zTmp[32];

                                sprintf(zTmp, "%d", psLVDispInfo->item.iItem+1);

                                strncpy(psLVDispInfo->item.pszText, zTmp, psLVDispInfo->item.cchTextMax);
                            }
                        break;
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}


void GetRelativeWindowRect(HWND hOwner, HWND hItem, RECT* psRect)
{
    RECT sRect;
    int nCX;
    int nCY;

    GetWindowRect(hOwner, &sRect);
    GetWindowRect(hItem, psRect);

    nCY = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
    nCX = GetSystemMetrics(SM_CXFRAME);

    psRect->left -= sRect.left + nCX;
    psRect->right -= sRect.left + nCX;
    psRect->top -= sRect.top + nCY;
    psRect->bottom -= sRect.top + nCY;
}


void Export(HWND hWnd, char* pzFile, char* pzCategory)
{
    FILE* fp;
    sArtist* psArtist;
    sCD* psCD;
    sTrack* psTrack;
    int nCount = 0;

    fp = fopen(pzFile, "w");
    if (!fp) {
        MessageBox(hWnd, "Error opening file", APPNAME, MB_OK | MB_ICONSTOP);
        
        return;
    }

    fputs("V1.1\n", fp);

    if (nOptions & OPTIONS_USECDDB)
        fputs("CDDB\n", fp);
    else
        fputs("INI\n", fp);

    ProgressOpen(NULL, "Exporting records", FALSE, poArtistList->NumItems());
    ProgressSet(0);

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    psArtist = poArtistList->First();
    while(psArtist) {
        psCD = psArtist->oCDList.First();
        while(psCD) {
            if (DBCheckCategory(psCD->pzCategory, pzCategory)) {
                BOOL bFreeTracks = FALSE;

                // Write Artist, Title, num tracks and tracks

                // If track list is not read, read it!
                if (!psCD->bRead)
                    bFreeTracks = TRUE;
                else {
                    while(psCD->oTrackList.First())
                        psCD->oTrackList.Delete();

                    psCD->bRead = FALSE;
                }

                ReadTracks(psCD);

                // Make sure we have read frames if we use the CDDB database
                if (((nOptions & OPTIONS_USECDDB) && sEntry.pnFrames) || !(nOptions & OPTIONS_USECDDB)) {
                    fputs(psCD->zID, fp);
                    fputs("\n", fp);
                    fputs(psArtist->pzName, fp);
                    fputs("\n", fp);
                    fputs(psCD->pzName, fp);
                    fputs("\n", fp);
                    fputs(psCD->pzCategory, fp);
                    fputs("\n", fp);
                    fprintf(fp, "%d\n", psCD->oTrackList.NumItems());

                    psTrack = psCD->oTrackList.First();
                    while(psTrack) {
                        fputs(psTrack->pzName, fp);
                        fputs("\n", fp);

                        psTrack = psCD->oTrackList.Next();
                    }

                    // Write out frames and disc length if we use CDDB
                    if (nOptions & OPTIONS_USECDDB) {
                        int nLoop;

                        fprintf(fp, "%d\n", sEntry.nDiscLength);
                        fprintf(fp, "%d\n", sEntry.nTracks);

                        for (nLoop = 0 ; nLoop < sEntry.nTracks ; nLoop ++) 
                            fprintf(fp, "%d\n", sEntry.pnFrames[nLoop]);
                    }

                    fprintf(fp, "--\n");
                }

                if (bFreeTracks) {
                    while(psCD->oTrackList.First())
                        psCD->oTrackList.Delete();

                    psCD->bRead = FALSE;
                }
            }

            psCD = psArtist->oCDList.Next();
        }
        
        nCount ++;
        if (!(nCount % 10))
            ProgressSet(nCount);

        psArtist = poArtistList->Next();
    }

    fclose(fp);

    ProgressClose();

    SetCursor(LoadCursor(NULL, IDC_ARROW));
}


void Import(HWND hWnd, char* pzFile, BOOL bOverwrite)
{
    TV_INSERTSTRUCT sTVInsertArtist;
	HWND hTree = GetDlgItem(hWnd, IDC_TREE);
    FILE* fp;
    sArtist* psArtist;
    sCD* psCD;
    sTrack* psTrack;
//        sTrack* psTrack;
    char zID[256];
    char zArtist[256];
    char zTitle[256];
    char zNum[256];
    char zCategory[256];
    char zVersion[32];
    int nNum;
    int nCount = 0;
    BOOL bINI = TRUE;

    DebugPrintf("Importing");

    fp = fopen(pzFile, "r");
    if (!fp) {
        MessageBox(hWnd, "Error opening file", APPNAME, MB_OK | MB_ICONSTOP);
        
        return;
    }

    fgets(zVersion, 32, fp);
    zVersion[strlen(zVersion) - 1] = 0;

    if (strcmp(zVersion, "V1.0") && strcmp(zVersion, "V1.1")) {
        fclose(fp);

        MessageBox(hWnd, "Unknown format or wrong format version", APPNAME, MB_OK | MB_ICONSTOP);

        fclose(fp);

        return;
    }

    if (!strcmp(zVersion, "V1.1")) {
        char zStr[81];

        fgets(zStr, 80, fp);

        zStr[strlen(zStr) - 1] = 0;

        if (!strcmp(zStr, "CDDB"))
            bINI = FALSE;
        else if (!strcmp(zStr, "INI"))
            bINI = TRUE;
        else {
            MessageBox(hWnd, "Unknown database format", APPNAME, MB_OK | MB_ICONSTOP);

            fclose(fp);

            return;
        }
    }

    if (bINI && (nOptions & OPTIONS_USECDDB)) {
        MessageBox(hWnd, "Database exported from INI file can't be imported to the CDDB format", APPNAME, MB_OK | MB_ICONSTOP);

        fclose(fp);

        return;
    }
    else if (!bINI && !(nOptions & OPTIONS_USECDDB)) {
        MessageBox(hWnd, "Database exported from the CDDB format can't be imported to the INI file", APPNAME, MB_OK | MB_ICONSTOP);

        fclose(fp);

        return;
    }

    ProgressOpen(NULL, "Importing records", TRUE, 0);
    ProgressSet(0);

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    sTVInsertArtist.hParent = TVI_ROOT;
    sTVInsertArtist.hInsertAfter = TVI_LAST;
    sTVInsertArtist.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
    sTVInsertArtist.item.pszText = LPSTR_TEXTCALLBACK;
    sTVInsertArtist.item.cChildren = 1;

    do {
        if (fgets(zID, 255, fp) && fgets(zArtist, 255, fp) && fgets(zTitle, 255, fp) && fgets(zCategory, 255, fp) && fgets(zNum, 255, fp)) {
            zID[strlen(zID) - 1] = 0;
            zArtist[strlen(zArtist) - 1] = 0;
            zTitle[strlen(zTitle) - 1] = 0;
            zCategory[strlen(zCategory) - 1] = 0;
            zNum[strlen(zNum) - 1] = 0;

            DebugPrintf("Found entry %s - %s with id %s", zArtist, zTitle, zID);

            nNum = atoi(zNum);

            // Find artist in list or create new

            psArtist = poArtistList->First();
            while(psArtist) {
                if (!strcmp(psArtist->pzName, zArtist))
                    break;
        
                psArtist = poArtistList->Next();
            }

            // If not found, add new
            if (!psArtist) {
                psArtist = new sArtist;

                psArtist->pzName = strdup(zArtist);
                psArtist->xType = 0;
                poArtistList->Add(psArtist);

                nNumArtists ++;

                sTVInsertArtist.item.lParam = (LONG) psArtist;
    
                TreeView_InsertItem(hTree, &sTVInsertArtist);
            }

            // Find CD or create new
            psCD = psArtist->oCDList.First();
            while(psCD) {
                if (!strcmp(psCD->pzName, zTitle))
                    break;
        
                psCD = psArtist->oCDList.Next();
            }
        
            psArtist->bChanged = TRUE;

            if (!psCD) {
                psCD = new sCD;
                psCD->xType = 1;
                psCD->bChanged = FALSE;

                nNumCDs ++;

                psArtist->oCDList.Add(psCD);
            }
            else if (!bOverwrite)
                psCD = NULL;

            if (psCD) {
                int nLoop;

                psCD->psArtist = psArtist;

                psCD->pzName = strdup(zTitle);
                psCD->pzCategory = strdup(zCategory);
                strcpy(psCD->zID, zID);
                psCD->bChanged = TRUE;

                psCD->bRead = FALSE;
                while(psCD->oTrackList.First())
                    psCD->oTrackList.Delete();

                psCD->bRead = TRUE;

                for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
                    fgets(zTitle, 255, fp);
                    zTitle[strlen(zTitle)-1] = 0;

                    psTrack = new sTrack;
                    psTrack->xType = 2;
                    psTrack->bChanged = TRUE;
                    psTrack->pzName = strdup(zTitle);

                    psCD->oTrackList.Add(psTrack);
                }

                // Read CDDB extra info if exported in CDDB format
                if (!bINI) {
                    int nLoop;
                    int nNum;

                    fgets(zNum, 32, fp);
                    zNum[strlen(zNum) - 1] = 0;
                    
                    psCD->nDiscLength = atoi(zNum);
                    fgets(zNum, 32, fp);
                    zNum[strlen(zNum) - 1] = 0;

                    nNum = atoi(zNum);

                    psCD->pnFrames = (int*) malloc(nNum*sizeof(int));

                    for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
                        fgets(zNum, 32, fp);
                        zNum[strlen(zNum) - 1] = 0;   

                        psCD->pnFrames[nLoop] = atoi(zNum);
                    }

                    psCD->nNumFrames = nNum;
                }
                else {
                    psCD->pnFrames = NULL;
                    psCD->nDiscLength = 0;
                    psCD->nNumFrames = 0;
                }

                SaveEntry(psCD);

                while(psCD->oTrackList.First())
                    psCD->oTrackList.Delete();

                psCD->bRead = FALSE;
            }
            else {
                while(nNum --)
                    fgets(zTitle, 255, fp);
            }

            // Read "--"
            fgets(zTitle, 255, fp);
        }
        nCount ++;
        if (!(nCount % 10))
            ProgressSet(nCount);
    } while(!feof(fp));

    fclose(fp);

    ProgressClose();

#ifdef NTFYCD
    InitTree(hWnd);
#endif

    SetCursor(LoadCursor(NULL, IDC_ARROW));
}


void Report(HWND hWnd, char* pzFile, char* pzCategory)
{
    FILE* fp;
    sArtist* psArtist;
    sCD* psCD;
    BOOL bArtistWritten;
    int nCount = 0;

    ProgressOpen(NULL, "Sorting", 2, 0);
    poArtistList->Sort(SortArtistCB);
    ProgressClose();

    fp = fopen(pzFile, "w");
    if (!fp) {
        MessageBox(hWnd, "Error opening file", APPNAME, MB_OK | MB_ICONSTOP);
        
        return;
    }

    ProgressOpen(NULL, "Generating report", FALSE, poArtistList->NumItems());
    ProgressSet(0);

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    psArtist = poArtistList->First();
    while(psArtist) {
        psArtist->oCDList.Sort(SortCDCB);

        bArtistWritten = FALSE;

        psCD = psArtist->oCDList.First();
        while(psCD) {
            if (DBCheckCategory(psCD->pzCategory, pzCategory)) {
                if (!bArtistWritten) {
                    fputs(psArtist->pzName, fp);
                    fputs("\n", fp);
                    fputs("--------------------------------------------------------------------\n", fp);

                    bArtistWritten = TRUE;
                }

                fputs("         ", fp);
                fputs(psCD->pzName, fp);
                fputs("\n", fp);

                nCount ++;
                if (!(nCount % 10))
                    ProgressSet(nCount);
            }

            psCD = psArtist->oCDList.Next();
        }

        if (bArtistWritten)
            fputs("\n", fp);
        
        psArtist = poArtistList->Next();
    }

    fclose(fp);

    ProgressClose();

    SetCursor(LoadCursor(NULL, IDC_ARROW));
}


BOOL CALLBACK DatabaseDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    static RECT sWindowRect;
    static RECT sListRect;
    static RECT sButtonRect;

    switch(nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            CenterWindow(hWnd);
            
            bInDBEdit = FALSE;

            bInDBDlg = TRUE;

            // Read INI file

            if (!BuildList(hWnd)) {
                MessageBox(NULL, "This CD database DLL doesn't support the database editor!", APPNAME, MB_OK);

                EndDialog(hWnd, TRUE);

                return TRUE;
            }

            // TreeView stuff

            InitTree(hWnd);

            // Setup columns

            ListInitCD(hWnd, NULL, 0);

            GetWindowRect(hWnd, &sWindowRect);
            // Get size of listview...
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_LIST), &sListRect);
            // Get size of statistics static
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_STATISTICS), &sButtonRect);

            int nX, nY, nCX, nCY;

            nX = GetPrivateProfileInt("NTFY_CD", "DB_X", sWindowRect.left, "CDPLAYER.INI");
            nY = GetPrivateProfileInt("NTFY_CD", "DB_Y", sWindowRect.top, "CDPLAYER.INI");
            nCX = GetPrivateProfileInt("NTFY_CD", "DB_CX", sWindowRect.right - sWindowRect.left, "CDPLAYER.INI");
            nCY = GetPrivateProfileInt("NTFY_CD", "DB_CY", sWindowRect.bottom - sWindowRect.top, "CDPLAYER.INI");

            MoveWindow(hWnd, nX, nY, nCX, nCY, TRUE);

            SetForegroundWindow(hWnd);
        }
		break;

        case WM_GETMINMAXINFO: {
            MINMAXINFO* psM = (LPMINMAXINFO) lParam;

            psM->ptMinTrackSize.x = sWindowRect.right - sWindowRect.left;
            psM->ptMinTrackSize.y = sWindowRect.bottom - sWindowRect.top;
        }
        break;

        case WM_SIZE: {
            RECT sRect;
            RECT sItemRect;
            int nSpacingY;
            int nSpacingX;
            int nPosX;

            GetClientRect(hWnd, &sRect);

            // Size Artists static

            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_ARTISTS), &sItemRect);
            nSpacingY = sItemRect.top;
            nSpacingX = sItemRect.left;
            MoveWindow(GetDlgItem(hWnd, IDC_ARTISTS), sItemRect.left, sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);

            // Size Artists treeview

            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_TREE), &sItemRect);
            nPosX = sRect.right - (sListRect.right - sListRect.left) - (sButtonRect.right - sButtonRect.left) - nSpacingX * 3;
            MoveWindow(GetDlgItem(hWnd, IDC_TREE), sItemRect.left, sItemRect.top, 
                nPosX - sItemRect.left - nSpacingX, 
                sRect.bottom - sItemRect.top - nSpacingY, TRUE);

            // Size Tracks static

            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_TRACKS), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDC_TRACKS), nPosX, sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);

            // Size Tracks listview

            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_LIST), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDC_LIST), nPosX, sItemRect.top, sListRect.right - sListRect.left, sRect.bottom - sItemRect.top - nSpacingY, TRUE);

            // Nail buttons to the right

            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDOK), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDOK), sRect.right - (sItemRect.right - sItemRect.left) - nSpacingX, 
                       sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDCANCEL), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDCANCEL), sRect.right - (sItemRect.right - sItemRect.left) - nSpacingX, 
                       sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_EXPORT), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDC_EXPORT), sRect.right - (sItemRect.right - sItemRect.left) - nSpacingX, 
                       sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_IMPORT), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDC_IMPORT), sRect.right - (sItemRect.right - sItemRect.left) - nSpacingX, 
                       sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_REPORT), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDC_REPORT), sRect.right - (sItemRect.right - sItemRect.left) - nSpacingX, 
                       sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_STATISTICS), &sItemRect);
            MoveWindow(GetDlgItem(hWnd, IDC_STATISTICS), sRect.right - (sItemRect.right - sItemRect.left) - nSpacingX, 
                       sItemRect.top, sItemRect.right-sItemRect.left, sItemRect.bottom-sItemRect.top, TRUE);

            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

        case WM_NOTIFY: {
			if (poArtistList)
				return DBEditorNotifyHandler(hWnd, nMsg, wParam, lParam);
        }
        break;

        case WM_CONTEXTMENU: {
            if ((HWND)wParam == GetDlgItem(hWnd, IDC_TREE)) {
                TV_ITEM sItem;
                HTREEITEM hTreeItem;

                hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                sItem.hItem = hTreeItem;

                if (TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem)) {
                    if (*((char*)sItem.lParam) == 1) {
                        POINT sPoint;
                        HMENU hMenu = LoadMenu(hMainInstance, MAKEINTRESOURCE(IDR_CONTEXT_DISC));
                        HMENU hPopup = GetSubMenu(hMenu, 0);

                        GetCursorPos(&sPoint);

                        TrackPopupMenu(hPopup, TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, sPoint.x, sPoint.y, 0, hWnd, NULL);

                        DestroyMenu(hMenu);
                    }
                }
            }
        }
        break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == BN_CLICKED) {
                switch(LOWORD(wParam)) {
                    case IDM_MAKEALIAS: {
                        if (MessageBox(hWnd, "Are You sure You want to make the current CD an alias to this artist's CD?", APPNAME, MB_YESNO) == IDYES) {
                            TV_ITEM sItem;
                            HTREEITEM hTreeItem;
                            sCD* psCD;

                            hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                            sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                            sItem.hItem = hTreeItem;

                            TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem);

                            psCD = (sCD*) sItem.lParam;

                            WritePrivateProfileString("ALIASES", zCurrCDDBID, psCD->zID, "CDPLAYER.INI");
                        }
                    }
                    break;

                    case IDM_DELETE: {
                        if (MessageBox(hWnd, "Are You sure You want to delete the current CD?", APPNAME, MB_YESNO) == IDYES) {
                            TV_ITEM sItem;
                            HTREEITEM hTreeItem;
                            sArtist* psArtist;
                            sCD* psCurrCD;
                            sCD* psCD;

                            hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));
                            if (hTreeItem) {
                                sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                                sItem.hItem = hTreeItem;

                                if (TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem)) {
                                    psCD = (sCD*) sItem.lParam;

                                    DBDelete(psCD->zID, psCD->zID);

                                    // Find CD in artist list 

                                    psArtist = psCD->psArtist;
                                    psCurrCD = psCD;

                                    psCD = psArtist->oCDList.First();
                                    while(psCD) {
                                        if (psCD == psCurrCD)
                                            break;

                                        psCD = psArtist->oCDList.Next();
                                    }

                                    if (psCD) {
                                        psArtist->oCDList.Delete();

                                        if (!psArtist->oCDList.NumItems())
                                            hTreeItem = TreeView_GetParent(GetDlgItem(hWnd, IDC_TREE), hTreeItem);
                                    }

                                    TreeView_DeleteItem(GetDlgItem(hWnd, IDC_TREE), hTreeItem);
                                }
                            }
                        }
                    }
                    break;

                    case IDM_SEND: {
                        TV_ITEM sItem;
                        HTREEITEM hTreeItem;
                        sCD* psCD;

                        if (!(nOptions & OPTIONS_USECDDB))
                            MessageBox(NULL, "You cannot use the DB Editor to send to the Internet repository when You use CDPLAYER.INI as local database. Please use the CD Info dialog instead!", APPNAME, MB_OK);
                        else {                          
                            hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                            sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                            sItem.hItem = hTreeItem;

                            TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem);

                            psCD = (sCD*) sItem.lParam;

                            if (!psCD->bChanged)
                                psCD->bChanged = TRUE;

                            SaveEntry(psCD);
                        
                            DBInternetSend(hWnd);

                            psCD->bChanged = FALSE;
                        }
                    }
                    break;

                    case IDM_SETCATEGORY: {
                        bChooseAllPresent = FALSE;
                        if (DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_CATEGORYCHOOSE), hWnd, (DLGPROC)CategoryChooseDlgProc) == IDOK) {
                            TV_ITEM sItem;
                            HTREEITEM hTreeItem;
                            sCD* psCD;

                            hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                            sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                            sItem.hItem = hTreeItem;

                            TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem);

                            psCD = (sCD*) sItem.lParam;

                            free(psCD->pzCategory);

                            psCD->pzCategory = strdup(ppzCategories[nCategoryChoosen]);
                            psCD->bChanged = TRUE;
                            psCD->bCategoryChanged = TRUE;
                        }
                    }
                    break;

                    case IDC_EXPORT: {
                        OPENFILENAME sOF;
                        char zFile[513];

                        // Get category
                        bChooseAllPresent = TRUE;
                        if (DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_CATEGORYCHOOSE), hWnd, (DLGPROC)CategoryChooseDlgProc) != IDOK)
                            break;

                        strcpy(zFile, "EXPORT.NCD");
                        sOF.lStructSize = sizeof(sOF);
                        sOF.hwndOwner = hWnd;
                        sOF.lpstrFilter = "NCD Files (*.NCD)\0*.NCD\0All Files (*.*)\0*.*\0\0";
                        sOF.lpstrCustomFilter = NULL;
                        sOF.nFilterIndex = 1;
                        sOF.lpstrFile = zFile;
                        sOF.nMaxFile = 512;
                        sOF.lpstrFileTitle = NULL;
                        sOF.lpstrInitialDir = NULL;
                        sOF.lpstrTitle = "Export As";
                        sOF.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN | OFN_HIDEREADONLY;
                        sOF.nFileOffset = 0;
                        sOF.nFileExtension = 7;
                        sOF.lpstrDefExt = ".NCD";

                        if (GetSaveFileName(&sOF)) {
                            if (nCategoryChoosen == -1)
                                Export(hWnd, zFile, "");
                            else
                                Export(hWnd, zFile, ppzCategories[nCategoryChoosen]);
                        }
                    }            
                    break;

                    case IDC_IMPORT: {
                        BOOL bOverwrite = FALSE;
                        OPENFILENAME sOF;
                        char zFile[513];

                        if (MessageBox(hWnd, "Do You want to overwrite existing entires if duplicates are found?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
                            bOverwrite = TRUE;
   
                        strcpy(zFile, "EXPORT.NCD");
                        sOF.lStructSize = sizeof(sOF);
                        sOF.hwndOwner = hWnd;
                        sOF.lpstrFilter = "NCD Files (*.NCD)\0*.NCD\0All Files (*.*)\0*.*\0\0";
                        sOF.lpstrCustomFilter = NULL;
                        sOF.nFilterIndex = 1;
                        sOF.lpstrFile = zFile;
                        sOF.nMaxFile = 512;
                        sOF.lpstrFileTitle = NULL;
                        sOF.lpstrInitialDir = NULL;
                        sOF.lpstrTitle = "Import";
                        sOF.Flags = OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN;
                        sOF.nFileOffset = 0;
                        sOF.nFileExtension = 7;
                        sOF.lpstrDefExt = ".NCD";

                        if (GetOpenFileName(&sOF))
                            Import(hWnd, zFile, bOverwrite);
                    }            
                    break;

                    case IDC_REPORT: {
                        OPENFILENAME sOF;
                        char zFile[513];
    
                        // Get category
                        bChooseAllPresent = TRUE;
                        if (DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_CATEGORYCHOOSE), hWnd, (DLGPROC)CategoryChooseDlgProc) != IDOK)
                            break;

                        strcpy(zFile, "REPORT.TXT");
                        sOF.lStructSize = sizeof(sOF);
                        sOF.hwndOwner = hWnd;
                        sOF.lpstrFilter = "Text files (*.TXT)\0*.TXT\0All Files (*.*)\0*.*\0\0";
                        sOF.lpstrCustomFilter = NULL;
                        sOF.nFilterIndex = 1;
                        sOF.lpstrFile = zFile;
                        sOF.nMaxFile = 512;
                        sOF.lpstrFileTitle = NULL;
                        sOF.lpstrInitialDir = NULL;
                        sOF.lpstrTitle = "Save Report As";
                        sOF.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN;
                        sOF.nFileOffset = 0;
                        sOF.nFileExtension = 7;
                        sOF.lpstrDefExt = ".TXT";

                        if (GetSaveFileName(&sOF)) {
                            if (nCategoryChoosen == -1)
                                Report(hWnd, zFile, "");
                            else
                                Report(hWnd, zFile, ppzCategories[nCategoryChoosen]);
                        }
                    }            
                    break;

                    case IDC_STATISTICS: {
                        char zStr[80];

                        sprintf(zStr, "%d Artists\n%d CDs", nNumArtists, nNumCDs);

                        MessageBox(hWnd, zStr, "Statistics", MB_OK);
                    }            
                    break;

                    case IDOK: {
                        if (bInDBEdit) {
                            TreeView_EndEditLabelNow(GetDlgItem(hWnd, IDC_TREE), FALSE);

                            break;
                        }

                        GetWindowRect(hWnd, &sWindowRect);

                        WritePrivateProfileInt("NTFY_CD", "DB_X", sWindowRect.left, "CDPLAYER.INI");
                        WritePrivateProfileInt("NTFY_CD", "DB_Y", sWindowRect.top, "CDPLAYER.INI");
                        WritePrivateProfileInt("NTFY_CD", "DB_CX", sWindowRect.right - sWindowRect.left, "CDPLAYER.INI");
                        WritePrivateProfileInt("NTFY_CD", "DB_CY", sWindowRect.bottom - sWindowRect.top, "CDPLAYER.INI");

                        SaveList();

                        bInDBDlg = FALSE;

                        DBClose();

                        EndDialog(hWnd, TRUE);

						delete poArtistList;

						poArtistList = NULL;
                    }
                    break;

                    case IDCANCEL: {
                        if (bInDBEdit) {
                            TreeView_EndEditLabelNow(GetDlgItem(hWnd, IDC_TREE), TRUE);

                            break;
                        }

                        bInDBDlg = FALSE;

                        DBClose();

                        EndDialog(hWnd, FALSE);

                        delete poArtistList;

						poArtistList = NULL;
                    }
                    break;
                }
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


BOOL CALLBACK CategoryChooseDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM /* lParam */)
{
    switch(nMsg) {
        case WM_INITDIALOG: {
            int nLoop;

            // Fill category list
            for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++)
                SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_ADDSTRING, 0, (LPARAM)ppzCategories[nLoop]);

            if (!bChooseAllPresent)
                ShowWindow(GetDlgItem(hWnd, IDC_CHOOSEALL), SW_HIDE);
            else
                SendMessage(GetDlgItem(hWnd, IDC_CHOOSEALL), BM_SETCHECK, 1, 0);
        }
        break;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK: {
                    int nLoop;
                    char szTmp[80];

                    // Get category
                    SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_GETLBTEXT, SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_GETCURSEL, 0, 0), (LPARAM)szTmp);

                    for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++) {
                        if (!stricmp(ppzCategories[nLoop], szTmp)) {
                            nCategoryChoosen = nLoop;

                            nLoop = nNumCategories;
                        }
                    }

                    if (SendDlgItemMessage(hWnd, IDC_CHOOSEALL, BM_GETCHECK, 0, 0))
                        nCategoryChoosen = -1;

                    EndDialog(hWnd, TRUE);
                }
                break;

                case IDCANCEL: {
                    EndDialog(hWnd, FALSE);
                }
                break;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


