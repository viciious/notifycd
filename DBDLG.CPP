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

#include "pre.h"

#include "resource.h"

#include "ntfy_cd.h"
#include "misc.h"
#include "db.h"
#include "dbdlg.h"
#include "cddb.h"
#include "infodlg.h"

extern GLOBALSTRUCT gs;

unsigned int nNumOfChoosenCategories = 0;
char** ppzChoosenCategories = NULL;
unsigned int nListViewMode = 0;
unsigned int nListViewModeEdit;
unsigned int nTreeViewMode = 0;

void Report(HWND hWnd, char* pzFile);
void Import(HWND hWnd, char* pzFile, BOOL bOverwrite);
void Export(HWND hWnd, char* pzFile, int nType);

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

struct sArtist;

struct sCD {
    sCD() {
        bChanged = bCategoryChanged = FALSE;
        pzOldCategory = NULL;
		ZeroMemory(&sDI, sizeof(sDI));
    }
    ~sCD() {
		FreeDiscInfo(&sDI);
    }
    
    // Do not add anything before this one!
	char xType;

	HTREEITEM hItem;
	BOOL bChanged;
	BOOL bCategoryChanged;
    char* pzOldCategory;
	sArtist* psArtist;
	DISCINFO sDI;
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

    // Do not add anything before this one!
    char xType;

	HTREEITEM hItem;
    char* pzName;
    BOOL bChanged;
    cList<sCD> oCDList;
};

cList<sArtist>* poArtistList;
int nNumArtists;
int nNumCDs;

struct sCategory {
	sCategory() {
		pzName = NULL;
	}
	~sCategory() {
		if (pzName)
			free(pzName);
		pzName = NULL;
	}

	char* pzName;
	HTREEITEM hItem;
};

cList<sCategory>* poCategoryList;

/////////////////////////////////////////////////////////////////////
//
// SEARCH DIALOG
//
/////////////////////////////////////////////////////////////////////

char szSearchString[256];
unsigned int nLastSearchType;
void* pvLastSearchPtr;
int nLastSearchTrack;

BOOL CALLBACK SearchDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM/*  lParam*/)
{
    switch(nMsg) {
    	case WM_INITDIALOG: {
            CenterWindow(hWnd, TRUE);                              
        }
		break;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK: {
                    GetWindowText(GetDlgItem(hWnd, IDC_SEARCH), szSearchString, 256);

                    EndDialog(hWnd, IDOK);
                }
                break;

                case IDCANCEL: {
                    EndDialog(hWnd, IDCANCEL);
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

    return stricmp(psCD1->sDI.pzTitle, psCD2->sDI.pzTitle);
}


BOOL ReadDiscs()
{
    char zID[32];
    int nCount = 0;
    sArtist* psArtist;
    sCD* psCD;

    if (!DBOpen())
        return FALSE;

    nNumArtists = nNumCDs = 0;

    poArtistList = new cList<sArtist>;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    ProgressOpen(NULL, "Reading discs", TRUE, 0);
    ProgressSet(0);

    do {
		psCD = new sCD;

		if (DBGetDBID(zID, &psCD->sDI)) {
			strcpy(psCD->sDI.zMCIID, zID);
			strcpy(psCD->sDI.zCDDBID, zID);

			psCD->sDI.nMCITracks = psCD->sDI.nTracks;
			ValidateDiscInfo(0, &psCD->sDI);
			ParsePlaylist(&psCD->sDI, TRUE);

			// Find existing artist!

			psArtist = poArtistList->First();
			while(psArtist) {
				if (!strcmp(psArtist->pzName, psCD->sDI.pzArtist))
					break;

				psArtist = poArtistList->Next();
			}

			// If not found, add new
			if (!psArtist) {
				psArtist = new sArtist;

				psArtist->pzName = strdup(psCD->sDI.pzArtist);
				psArtist->xType = 0;
				psArtist->bChanged = FALSE;
				poArtistList->Add(psArtist);

				nNumArtists ++;
			}

			// Add CD to artist CD list
			nNumCDs ++;

			psCD->xType = 1;
			psCD->psArtist = psArtist;
			psArtist->oCDList.Add(psCD);
       
			nCount ++;
			if (!(nCount % 10))
				ProgressSet(nCount);
		}
        else
            delete psCD;
    } while(DBIsEnd());

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    poArtistList->Sort(SortArtistCB);

    ProgressClose();

    return TRUE;
}


void BuildList(HWND hWnd,
			   unsigned int nType)
{
    TV_INSERTSTRUCT sTVInsertArtist;
	HWND hTree = GetDlgItem(hWnd, IDC_TREE);
    sArtist* psArtist;
	sCD* psCD;

    TreeView_DeleteAllItems(GetDlgItem(hWnd, IDC_TREE));

	nTreeViewMode = nType;

	if (nType == 0) {
        EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_ENABLED);

		sTVInsertArtist.hParent = TVI_ROOT;
		sTVInsertArtist.hInsertAfter = TVI_LAST;
		sTVInsertArtist.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
		sTVInsertArtist.item.pszText = LPSTR_TEXTCALLBACK;
		sTVInsertArtist.item.cChildren = 1;

		psArtist = poArtistList->First();
		while (psArtist) {
			sTVInsertArtist.item.lParam = (LONG) psArtist;

			psArtist->hItem = TreeView_InsertItem(hTree, &sTVInsertArtist);

			psArtist = poArtistList->Next();
		}
	}
	else if (nType == 1) {
		unsigned int nCount;
		sCategory* psCategory;
	    TV_INSERTSTRUCT sTVInsertCategory;
		TV_ITEM sItem;

        EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);

		// Build Category List
		if (!poCategoryList) {
			poCategoryList = new cList<sCategory>;

			psArtist = poArtistList->First();
			while (psArtist) {
				psCD = psArtist->oCDList.First();
				while (psCD) {
					psCategory = poCategoryList->First();
					while (psCategory) {
						if (!stricmp(psCategory->pzName, psCD->sDI.pzCategory))
							break;

						psCategory = poCategoryList->Next();
					}

					if (!psCategory) {
						psCategory = new sCategory;
						psCategory->pzName = strdup(psCD->sDI.pzCategory);

						poCategoryList->Add(psCategory);
					}

					psCD = psArtist->oCDList.Next();
				}

				psArtist = poArtistList->Next();
			}
		}

		sTVInsertCategory.hParent = TVI_ROOT;
		sTVInsertCategory.hInsertAfter = TVI_SORT;

		psCategory = poCategoryList->First();
		while (psCategory) {
			sTVInsertCategory.item.mask = TVIF_TEXT | TVIF_CHILDREN;
			sTVInsertCategory.item.pszText = psCategory->pzName;
			sTVInsertCategory.item.cChildren = 0;

			psCategory->hItem = TreeView_InsertItem(hTree, &sTVInsertCategory);

			// Scan list of discs and try to find the ones that fits
			sTVInsertArtist.hParent = psCategory->hItem;
			sTVInsertArtist.hInsertAfter = TVI_LAST;
			sTVInsertArtist.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
			sTVInsertArtist.item.pszText = LPSTR_TEXTCALLBACK;
			sTVInsertArtist.item.cChildren = 0;

			nCount = 0;

			psArtist = poArtistList->First();
			while (psArtist) {
				psCD = psArtist->oCDList.First();
				while (psCD) {
					if (!stricmp(psCD->sDI.pzCategory, psCategory->pzName)) {
						sTVInsertArtist.item.lParam = (LONG) psCD;

						psCD->hItem = TreeView_InsertItem(hTree, &sTVInsertArtist);

						nCount ++;
					}

					psCD = psArtist->oCDList.Next();
				}

				psArtist = poArtistList->Next();
			}

			// Update count
			sItem.mask = TVIF_CHILDREN | TVIF_HANDLE;
			sItem.hItem = psCategory->hItem;
			sItem.cChildren = nCount;

			TreeView_SetItem(hTree, &sItem);

			psCategory = poCategoryList->Next();
		}
	}

    // Check the view menu
	if (nType == 0) {
		CheckMenuItem(GetMenu(hWnd), ID_VIEW_BYARTIST, MF_BYCOMMAND | MF_CHECKED);
		CheckMenuItem(GetMenu(hWnd), ID_VIEW_BYCATEGORY, MF_BYCOMMAND | MF_UNCHECKED);
	}
	else if (nType == 1) {
		CheckMenuItem(GetMenu(hWnd), ID_VIEW_BYARTIST, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem(GetMenu(hWnd), ID_VIEW_BYCATEGORY, MF_BYCOMMAND | MF_CHECKED);
	}

    DrawMenuBar(hWnd);
}


void DBFreeChoosenCategories()
{
	unsigned int nLoop;
	
	for (nLoop = 0 ; nLoop < nNumOfChoosenCategories ; nLoop ++)
		free(ppzChoosenCategories[nLoop]);

	free(ppzChoosenCategories);
}


BOOL DBCheckCategory(char* pzDiscCat)
{
	unsigned int nLoop;
	
	for (nLoop = 0 ; nLoop < nNumOfChoosenCategories ; nLoop ++) {
		if (!stricmp(pzDiscCat, ppzChoosenCategories[nLoop]))
			return TRUE;
	}

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
    sLVColumn.cx = 300;
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

    ListView_DeleteAllItems(hList);
    
    ListView_DeleteColumn(hList, 2);
    ListView_DeleteColumn(hList, 1);
    ListView_DeleteColumn(hList, 0);

    sLVColumn.mask = LVCF_TEXT | LVCF_WIDTH;
    sLVColumn.cx = 330;
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

    while(nItems --) {
        LV_ITEM sLVItem;

        sLVItem.mask = LVIF_TEXT | LVIF_PARAM;
        sLVItem.iItem = nLoop;
        sLVItem.iSubItem = 0;
        sLVItem.pszText = LPSTR_TEXTCALLBACK;
  		sLVItem.cchTextMax = 256;
        sLVItem.lParam = (LONG) psCD;

        ListView_InsertItem(hList, &sLVItem);

        nLoop ++;
    }
}


BOOL SaveEntry(sCD* psCD)
{
	// No delete needed if we use CDPLAYER.INI. In fact a delete here will screw up the INI file based database!
    if (psCD && psCD->bCategoryChanged && (gs.nOptions & OPTIONS_USECDDB)) {
        char* pzTemp = NULL;

        // Use the old category here to delete the correct entry. Otherwise we end up trying to delete the new
        // entry which isn't saved yet...
        AppendString(&pzTemp, psCD->sDI.pzCategory, -1);
        free(psCD->sDI.pzCategory);
        psCD->sDI.pzCategory = NULL;
        AppendString(&psCD->sDI.pzCategory, psCD->pzOldCategory, -1);

        DBDelete(&psCD->sDI);

        free(psCD->sDI.pzCategory);
        psCD->sDI.pzCategory = NULL;
        AppendString(&psCD->sDI.pzCategory, pzTemp, -1);
        free(pzTemp);

        free(psCD->pzOldCategory);
    }

    if ((psCD && psCD->bChanged) || (psCD && psCD->psArtist->bChanged)) {
		free(psCD->sDI.pzArtist);
		psCD->sDI.pzArtist = strdup(psCD->psArtist->pzName);

        DBSave(&psCD->sDI);

        // Has the current disc been updated?
        if (!stricmp(psCD->sDI.zCDDBID, gs.di[0].zCDDBID)) {
            EnterCriticalSection(&gs.sDiscInfoLock);

            psCD->sDI.ppzTrackLen = gs.di[0].ppzTrackLen;
            psCD->sDI.pnProgrammedTracks = gs.di[0].pnProgrammedTracks;
            psCD->sDI.pnTrackLen = gs.di[0].pnTrackLen;
            psCD->sDI.pnFrames = gs.di[0].pnFrames;
            
            gs.di[0].ppzTrackLen = NULL;
            gs.di[0].pnProgrammedTracks = NULL;
            gs.di[0].pnTrackLen = NULL;
            gs.di[0].pnFrames = NULL;
            
            FreeDiscInfo(&gs.di[0]);
            CopyDiscInfo(&gs.di[0], &psCD->sDI);

            psCD->sDI.ppzTrackLen = NULL;
            psCD->sDI.pnProgrammedTracks = NULL;
            psCD->sDI.pnTrackLen = NULL;
            psCD->sDI.pnFrames = NULL;
            
            LeaveCriticalSection(&gs.sDiscInfoLock);
        }

        return TRUE;
    }

    return FALSE;
}


BOOL GetChanged()
{   
    sArtist* psArtist;
    sCD* psCD;
    int nCount = 0;

    psArtist = poArtistList->First();
    while(psArtist) {
        if (psArtist->bChanged)
            return TRUE;

        psCD = psArtist->oCDList.First();
        while(psCD) {
            if (psCD->bChanged)
                return TRUE;

            psCD = psArtist->oCDList.Next();
        }

        psArtist = poArtistList->Next();
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

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    ProgressClose();
}


BOOL Search(HWND hWnd,
			unsigned int nType,
			BOOL bFirst)
{
	if (nType == 0) {
		char* pzTmpString;
		sArtist* psArtist;

		nLastSearchType = nType;
		if (bFirst)
			pvLastSearchPtr = NULL;

		strupr(szSearchString);

		if (pvLastSearchPtr) {
			psArtist = poArtistList->First();
			while (psArtist != pvLastSearchPtr)
				psArtist = poArtistList->Next();
			
			psArtist = poArtistList->Next();
		}
		else
			psArtist = poArtistList->First();
		while (psArtist) {
			pzTmpString = strdup(psArtist->pzName);
			strupr(pzTmpString);

			if (strstr(pzTmpString, szSearchString)) {
				TreeView_SelectItem(GetDlgItem(hWnd, IDC_TREE), psArtist->hItem);

				pvLastSearchPtr = psArtist;

				free(pzTmpString);

				return TRUE;
			}

			free(pzTmpString);

			psArtist = poArtistList->Next();
		}
	}
	else if (nType == 1) {
		char* pzTmpString;
		sArtist* psArtist;
		sCD* psCD;

		nLastSearchType = nType;
		if (bFirst)
			pvLastSearchPtr = NULL;

		strupr(szSearchString);

		if (pvLastSearchPtr) {
			psCD = NULL;

			psArtist = poArtistList->First();
			while (psArtist && !psCD) {
				psCD = psArtist->oCDList.First();
				while (psCD) {
					if (psCD == pvLastSearchPtr)
						break;

					psCD = psArtist->oCDList.Next();
				}
				if (!psCD)
					psArtist = poArtistList->Next();
			}
			
			if (psArtist)
				psCD = psArtist->oCDList.Next();
		}
		else
			psArtist = poArtistList->First();
		while (psArtist) {
			if (pvLastSearchPtr == NULL)
				psCD = psArtist->oCDList.First();
			else
				pvLastSearchPtr = NULL;
			while (psCD) {
				pzTmpString = strdup(psCD->sDI.pzTitle);
				strupr(pzTmpString);

				if (strstr(pzTmpString, szSearchString)) {
					TreeView_Expand(GetDlgItem(hWnd, IDC_TREE), psArtist->hItem, TVE_EXPAND);

					TreeView_SelectItem(GetDlgItem(hWnd, IDC_TREE), psCD->hItem);

					pvLastSearchPtr = psCD;

					free(pzTmpString);

					return TRUE;
				}

				free(pzTmpString);

				psCD = psArtist->oCDList.Next();
			}

			psArtist = poArtistList->Next();
		}
	}
	else if (nType == 2) {
		char* pzTmpString;
		sArtist* psArtist;
		sCD* psCD;
		unsigned int nLoop;

		nLastSearchType = nType;
		if (bFirst) {
			pvLastSearchPtr = NULL;
			nLastSearchTrack = -1;
		}

		strupr(szSearchString);

		if (pvLastSearchPtr) {
			psCD = NULL;

			psArtist = poArtistList->First();
			while (psArtist && !psCD) {
				psCD = psArtist->oCDList.First();
				while (psCD) {
					if (psCD == pvLastSearchPtr)
						break;

					psCD = psArtist->oCDList.Next();
				}
				if (!psCD)
					psArtist = poArtistList->Next();
			}
		}
		else
			psArtist = poArtistList->First();
		while (psArtist) {
			if (pvLastSearchPtr == NULL)
				psCD = psArtist->oCDList.First();
			else
				pvLastSearchPtr = NULL;
			while (psCD) {
				for (nLoop = nLastSearchTrack + 1 ; nLoop < psCD->sDI.nTracks ; nLoop ++) {
					pzTmpString = strdup(psCD->sDI.ppzTracks[nLoop]);
					strupr(pzTmpString);

					if (strstr(pzTmpString, szSearchString)) {
						int nItemLoop;

						TreeView_Expand(GetDlgItem(hWnd, IDC_TREE), psArtist->hItem, TVE_EXPAND);

						TreeView_SelectItem(GetDlgItem(hWnd, IDC_TREE), psCD->hItem);

						for (nItemLoop = 0 ; nItemLoop < ListView_GetItemCount(GetDlgItem(hWnd, IDC_LIST)) ; nItemLoop ++)
							ListView_SetItemState(GetDlgItem(hWnd, IDC_LIST), nItemLoop, 0, 0x00FF);

						SetFocus(GetDlgItem(hWnd, IDC_LIST));
						ListView_SetItemState(GetDlgItem(hWnd, IDC_LIST), nLoop, MAKELONG(LVIS_SELECTED | LVIS_FOCUSED, 0), 0x00FF);

						pvLastSearchPtr = psCD;
						nLastSearchTrack = nLoop;

						free(pzTmpString);

						return TRUE;
					}

					free(pzTmpString);
				}

				nLastSearchTrack = 0;
                
                psCD = psArtist->oCDList.Next();
			}

			psArtist = poArtistList->Next();
		}
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

    nCY = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYMENU);
    nCX = GetSystemMetrics(SM_CXFRAME);

    psRect->left -= sRect.left + nCX;
    psRect->right -= sRect.left + nCX;
    psRect->top -= sRect.top + nCY;
    psRect->bottom -= sRect.top + nCY;
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
                case TVN_KEYDOWN: {
                    NMTVKEYDOWN* psKeyDown = (NMTVKEYDOWN*) lParam;

                    if (psKeyDown->wVKey == VK_DELETE) {
                        TV_ITEM sItem;
                        HTREEITEM hTreeItem;

                        hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                        sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                        sItem.hItem = hTreeItem;

                        if (TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem)) {
                            if (*((char*)sItem.lParam) == 1)
                                PostMessage(hWnd, WM_COMMAND, MAKELONG(IDM_DELETE, 0), 0);
                        }
                    }
                    else if (psKeyDown->wVKey == VK_F2 && nTreeViewMode == 0) {
                        HTREEITEM hTreeItem;

                        hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                        TreeView_EditLabel(GetDlgItem(hWnd, IDC_TREE), hTreeItem);
                    }
                    else if (psKeyDown->wVKey == VK_F3 && nTreeViewMode == 0) {
						if (!Search(hWnd, nLastSearchType, FALSE))
							MessageBox(hWnd, "Not found", APPNAME, MB_OK | MB_ICONINFORMATION);
					}
                    else if (psKeyDown->wVKey == VK_DELETE) {
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_DELETE, 0), 0);
                    }

                    return FALSE;
                }
                break;

                case TVN_ITEMEXPANDING: {
                    if (nTreeViewMode == 0) {
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
    
									psCD->hItem = TreeView_InsertItem(GetDlgItem(hWnd, IDC_TREE), &sTVInsertCD);

									psCD = psArtist->oCDList.Next();
								}
							}
						}
					}
				}
				break;

				case TVN_SELCHANGED: {
					if (nTreeViewMode == 0) {
						// Check type and init list columns, add items etc
						char xType = *((char*)psNm->itemNew.lParam);

						if (xType == 0) {
							nListViewMode = 0;
							ListInitCD(hWnd, ((sArtist*)psNm->itemNew.lParam), ((sArtist*)psNm->itemNew.lParam)->oCDList.NumItems());

							EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);

							DrawMenuBar(hWnd);
						}
						else if (xType == 1) {
							nListViewMode = 1;
							ListInitTracks(hWnd, ((sCD*)psNm->itemNew.lParam), ((sCD*)psNm->itemNew.lParam)->sDI.nTracks);

							EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_ENABLED);

							DrawMenuBar(hWnd);
						}
					}
					else if (nTreeViewMode == 1 && psNm->itemNew.lParam) {
						// Check type and init list columns, add items etc
						char xType = *((char*)psNm->itemNew.lParam);

						if (xType == 1) {
							nListViewMode = 1;
							ListInitTracks(hWnd, ((sCD*)psNm->itemNew.lParam), ((sCD*)psNm->itemNew.lParam)->sDI.nTracks);

							EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_ENABLED);

							DrawMenuBar(hWnd);
						}
					}
					else if (nTreeViewMode == 1) {
						EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);

						ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LIST));
					}
                    
					ListView_RedrawItems(GetDlgItem(hWnd, IDC_LIST), 0, 128);
                }
                break;

                case TVN_BEGINLABELEDIT: {
                    if (psTVDispInfo->item.lParam && nTreeViewMode == 0) {
                        char xType = *((char*)psTVDispInfo->item.lParam);

                        bInDBEdit = TRUE;

                        HWND hEdit = TreeView_GetEditControl(GetDlgItem(hWnd, IDC_LIST));
                    }
                    else
                        return TRUE;
                }
                break;

                case TVN_ENDLABELEDIT: {
                    if (nTreeViewMode == 0) {
                        char xType = *((char*)psTVDispInfo->item.lParam);

                        bInDBEdit = FALSE;

                        if (psTVDispInfo->item.pszText == NULL)
                            return FALSE;

                        if (xType == 1) {
                            free(psCD->sDI.pzTitle);

                            psCD->sDI.pzTitle = strdup(psTVDispInfo->item.pszText);
                            psCD->bChanged = TRUE;
                        }
                        else if (xType == 0) {
                            free(psArtist->pzName);

                            psArtist->pzName = strdup(psTVDispInfo->item.pszText);
                            psArtist->bChanged = TRUE;
                        }
                    }
                }
                break;

                case TVN_GETDISPINFO: {
					if (nTreeViewMode == 0) {
						char xType = *((char*)psTVDispInfo->item.lParam);

						if (psTVDispInfo->item.mask & TVIF_TEXT) {
							if (xType == 0) {
								strncpy(psTVDispInfo->item.pszText, psArtist->pzName, psTVDispInfo->item.cchTextMax);
							}
							else if (xType == 1) {
								if (psCD->sDI.pzTitle && strlen(psCD->sDI.pzTitle))
									strncpy(psTVDispInfo->item.pszText, psCD->sDI.pzTitle, psTVDispInfo->item.cchTextMax);
								else
									strncpy(psTVDispInfo->item.pszText, "[no title]", psTVDispInfo->item.cchTextMax);
							}
						}
					}
					else if (nTreeViewMode == 1) {
						char xType = *((char*)psTVDispInfo->item.lParam);

						if (psTVDispInfo->item.mask & TVIF_TEXT) {
							if (xType == 1) {
								char* pzTmp = NULL;

								AppendString(&pzTmp, psCD->sDI.pzArtist, -1);
								AppendString(&pzTmp, " - ", -1);
								AppendString(&pzTmp, psCD->sDI.pzTitle, -1);

								strncpy(psTVDispInfo->item.pszText, pzTmp, psTVDispInfo->item.cchTextMax);

								free(pzTmp);
							}
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
            
            switch(psLVDispInfo->hdr.code) {
                case LVN_KEYDOWN: {
                    NMLVKEYDOWN* psKeyDown = (NMLVKEYDOWN*) lParam;

                    if (psKeyDown->wVKey == VK_F2) {
                        if (ListView_GetSelectedCount(GetDlgItem(hWnd, IDC_LIST))) {
                            int nItem = ListView_GetNextItem(GetDlgItem(hWnd, IDC_LIST), -1, LVNI_ALL | LVNI_SELECTED);

                            ListView_EditLabel(GetDlgItem(hWnd, IDC_LIST), nItem);
                        }
                    }
                    else if (psKeyDown->wVKey == VK_F3) {
						if (!Search(hWnd, nLastSearchType, FALSE))
							MessageBox(hWnd, "Not found", APPNAME, MB_OK | MB_ICONINFORMATION);
					}

                    return FALSE;
                }
                break;

                case LVN_BEGINLABELEDIT: {
					nListViewModeEdit = nListViewMode;
                }
                break;

                case LVN_ENDLABELEDIT: {
                    char xType = *((char*)psLVDispInfo->item.lParam);

                    if (psLVDispInfo->item.iItem == -1 || psLVDispInfo->item.pszText == NULL)
                        return FALSE;

                    switch (psLVDispInfo->item.iSubItem) {
                        case 0: {
                            if (nListViewModeEdit == 0) {
                                free(psCD->sDI.pzTitle);

                                psCD->sDI.pzTitle = strdup(psLVDispInfo->item.pszText);
                                psCD->bChanged = TRUE;
                            }
                            else if (nListViewModeEdit == 1) {
								psCD = ((sCD*)psLVDispInfo->item.lParam);

                                free(psCD->sDI.ppzTracks[psLVDispInfo->item.iItem]);

                                psCD->sDI.ppzTracks[psLVDispInfo->item.iItem] = strdup(psLVDispInfo->item.pszText);
                                psCD->bChanged = TRUE;
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
                            if (nListViewMode == 0) {
								if (psCD->sDI.pzTitle && strlen(psCD->sDI.pzTitle))
									strncpy(psLVDispInfo->item.pszText, psCD->sDI.pzTitle, psLVDispInfo->item.cchTextMax);
								else
									strncpy(psLVDispInfo->item.pszText, "[no title]", psLVDispInfo->item.cchTextMax);
							}
                            else if (nListViewMode == 1) {
								psCD = ((sCD*)psLVDispInfo->item.lParam);

                                strncpy(psLVDispInfo->item.pszText, psCD->sDI.ppzTracks[psLVDispInfo->item.iItem], psLVDispInfo->item.cchTextMax);
							}
                        break;

                        case 1:
                            if (nListViewMode == 0)
                                strncpy(psLVDispInfo->item.pszText, psCD->sDI.pzCategory, psLVDispInfo->item.cchTextMax);
                            else if (nListViewMode == 1) {
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
            
			nLastSearchType = 0xFFFF;
			pvLastSearchPtr = NULL;
			nLastSearchTrack = 0;

            bInDBEdit = FALSE;

            gs.bInDBDlg = TRUE;

            // Read INI file

            if (!ReadDiscs()) {
                MessageBox(NULL, "Failed to read discs!", APPNAME, MB_OK | MB_ICONERROR);

                EndDialog(hWnd, TRUE);

                return TRUE;
            }

			// Build treeview
			BuildList(hWnd, 0);

            // Disable DISC menu
            EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_GRAYED | MF_DISABLED);

            // TreeView stuff
            InitTree(hWnd);

            // Setup columns
            ListInitCD(hWnd, NULL, 0);

            GetWindowRect(hWnd, &sWindowRect);
            // Get size of listview...
            GetRelativeWindowRect(hWnd, GetDlgItem(hWnd, IDC_LIST), &sListRect);

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

            psM->ptMinTrackSize.x = 500;
            psM->ptMinTrackSize.y = 200;
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

            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

        case WM_NOTIFY: {
        	if (poArtistList)
				return DBEditorNotifyHandler(hWnd, nMsg, wParam, lParam);
        }
        break;

		case WM_COMMAND: {
            if (HIWORD(wParam) == BN_CLICKED) {
                switch(LOWORD(wParam)) {
					case ID_SEARCH_FINDNEXT: {
						if (!Search(hWnd, nLastSearchType, FALSE))
							MessageBox(hWnd, "Not found", APPNAME, MB_OK | MB_ICONINFORMATION);
					}
					break;

					case ID_SEARCH_ARTIST: {
						if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc) == IDOK) {
							if (!Search(hWnd, 0, TRUE))
								MessageBox(hWnd, "Not found", APPNAME, MB_OK | MB_ICONINFORMATION);
						}
					}
					break;

					case ID_SEARCH_TITLE: {
						if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc) == IDOK) {
							if (!Search(hWnd, 1, TRUE))
								MessageBox(hWnd, "Not found", APPNAME, MB_OK | MB_ICONINFORMATION);
						}
					}
					break;

					case ID_SEARCH_TRACK: {
						if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_SEARCH), hWnd, SearchDlgProc) == IDOK) {
							if (!Search(hWnd, 2, TRUE))
								MessageBox(hWnd, "Not found", APPNAME, MB_OK | MB_ICONINFORMATION);
						}
					}
					break;

					case IDM_MAKEALIAS: {
                        if (MessageBox(hWnd, "Are You sure You want to make the current CD an alias to this artist's CD?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            TV_ITEM sItem;
                            HTREEITEM hTreeItem;
                            sCD* psCD;

                            hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                            sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                            sItem.hItem = hTreeItem;

                            TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem);

                            psCD = (sCD*) sItem.lParam;

                            WritePrivateProfileString("ALIASES", gs.di[0].zCDDBID, psCD->sDI.zCDDBID, "CDPLAYER.INI");
                        }
                    }
                    break;

                    case IDM_DELETE: {
                        if (MessageBox(hWnd, "Are You sure You want to delete the current CD?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
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

                                    DBDelete(&psCD->sDI);

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

					case IDM_CDINFO: {
                        TV_ITEM sItem;
                        HTREEITEM hTreeItem;
                        sCD* psCD;
                        INFODLGPARAM sParam;

                        hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                        sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                        sItem.hItem = hTreeItem;

                        TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem);

                        psCD = (sCD*) sItem.lParam;

                        sParam.psDI = &psCD->sDI;
                        sParam.bSave = FALSE;

                        if (DialogBoxParam(gs.hMainInstance, MAKEINTRESOURCE(IDD_INFO), NULL, (DLGPROC)InfoDlgProc, (LPARAM)&sParam) == IDOK) {
                            ListView_RedrawItems(GetDlgItem(hWnd, IDC_LIST), 0, 128);

                            psCD->bChanged = TRUE;

                            // If the artist changed. Loop through all discs under this artist and change them!
                            if (strcmp(psCD->sDI.pzArtist, psCD->psArtist->pzName)) {
                                sCD* psCDLoop;

				                psCDLoop = psCD->psArtist->oCDList.First();
				                while (psCDLoop) {
                                    psCD->bChanged = TRUE;

                                    if (psCDLoop != psCD) {
                                        if (psCDLoop->sDI.pzArtist)
                                            free(psCDLoop->sDI.pzArtist);
                                        psCDLoop->sDI.pzArtist = strdup(psCD->sDI.pzArtist);
                                    }

					                psCDLoop = psCD->psArtist->oCDList.Next();
                                }

                                if (psCD->psArtist->pzName)
                                    free(psCD->psArtist->pzName);
                                psCD->psArtist->pzName = strdup(psCD->sDI.pzArtist);
                            }
                        }
					}
					break;

                    case IDM_SEND: {
                        TV_ITEM sItem;
                        HTREEITEM hTreeItem;
                        sCD* psCD;

                        if (!(gs.nOptions & OPTIONS_USECDDB))
                            MessageBox(NULL, "You cannot use the DB Editor to send to the Internet repository when You use CDPLAYER.INI as local database. Please use the CD Info dialog instead!", APPNAME, MB_OK | MB_ICONINFORMATION);
                        else {                          
                            hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                            sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                            sItem.hItem = hTreeItem;

                            TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem);

                            psCD = (sCD*) sItem.lParam;

                            SaveEntry(psCD);

                            DBInternetSend(&psCD->sDI, hWnd);
                        }
                    }
                    break;

                    case IDM_SETCATEGORY: {
                        gs.bMultiselectCategories = FALSE;
                        if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_CATEGORYCHOOSESINGLE), hWnd, (DLGPROC)CategoryChooseDlgProc) == IDOK) {
                            TV_ITEM sItem;
                            HTREEITEM hTreeItem;
                            sCD* psCD;
                            BOOL bDeleteCategory = FALSE;
                            HTREEITEM hParent;

                            hTreeItem = TreeView_GetSelection(GetDlgItem(hWnd, IDC_TREE));

                            sItem.mask = TVIF_PARAM | TVIF_HANDLE;
                            sItem.hItem = hTreeItem;

                            TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &sItem);

                            psCD = (sCD*) sItem.lParam;

                            // Only copy the changed category first time, otherwise we cannot delete the old entry later on
                            if (!psCD->pzOldCategory)
                                AppendString(&psCD->pzOldCategory, psCD->sDI.pzCategory, -1);

                            free(psCD->sDI.pzCategory);

                            psCD->sDI.pzCategory = strdup(ppzChoosenCategories[0]);
                            psCD->bChanged = TRUE;
                            psCD->bCategoryChanged = TRUE;

							free(ppzChoosenCategories[0]);
							free(ppzChoosenCategories);
							
							if (nTreeViewMode == 1) {
                                sCategory* psCategory;
                                TV_INSERTSTRUCT sTVInsertArtist;

                                // First try to find the category and make a new one if not found
                                psCategory = poCategoryList->First();
					            while (psCategory) {
						            if (!stricmp(psCategory->pzName, psCD->sDI.pzCategory))
							            break;

						            psCategory = poCategoryList->Next();
					            }

    				            if (!psCategory) {
                                  	TV_INSERTSTRUCT sTVInsertCategory;
    
                                    psCategory = new sCategory;
						            psCategory->pzName = strdup(psCD->sDI.pzCategory);

						            poCategoryList->Add(psCategory);

		                            sTVInsertCategory.hParent = TVI_ROOT;
		                            sTVInsertCategory.hInsertAfter = TVI_SORT;
			                        sTVInsertCategory.item.mask = TVIF_TEXT | TVIF_CHILDREN;
			                        sTVInsertCategory.item.pszText = psCategory->pzName;
			                        sTVInsertCategory.item.cChildren = 1;
                        
                                    psCategory->hItem = TreeView_InsertItem(GetDlgItem(hWnd, IDC_TREE), &sTVInsertCategory);
					            }

                                // Expand the branch                              
                                TreeView_Expand(GetDlgItem(hWnd, IDC_TREE), psCategory->hItem, TVE_EXPAND);

                                // Find out the parent
                                hParent = TreeView_GetParent(GetDlgItem(hWnd, IDC_TREE), psCD->hItem);

                                // Now move the item to the new branch and select the item...
                                TreeView_DeleteItem(GetDlgItem(hWnd, IDC_TREE), psCD->hItem);

                                sTVInsertArtist.hParent = psCategory->hItem;
			                    sTVInsertArtist.hInsertAfter = TVI_LAST;
			                    sTVInsertArtist.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
			                    sTVInsertArtist.item.pszText = LPSTR_TEXTCALLBACK;
			                    sTVInsertArtist.item.cChildren = 0;
						        sTVInsertArtist.item.lParam = (LONG) psCD;

						        psCD->hItem = TreeView_InsertItem(GetDlgItem(hWnd, IDC_TREE), &sTVInsertArtist);
      
                                TreeView_SelectItem(GetDlgItem(hWnd, IDC_TREE), psCD->hItem);

                                // Check if this was the last item in the old category and delete that category if it was
                                if (TreeView_GetChild(GetDlgItem(hWnd, IDC_TREE), hParent) == NULL) {
                                    psCategory = poCategoryList->First();
					                while (psCategory) {
                                        if (psCategory->hItem == hParent) {
                                            TreeView_DeleteItem(GetDlgItem(hWnd, IDC_TREE), hParent);

                                            poCategoryList->Delete();

                                            psCategory = NULL;
                                        }
                                        else
						                    psCategory = poCategoryList->Next();
					                }
                                }
							}
                        }
                    }
                    break;

                    case ID_FILE_EXPORT: {
                        OPENFILENAME sOF;
                        char zFile[513];

                        // Get category
                        gs.bMultiselectCategories = TRUE;
                        if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_CATEGORYCHOOSEEXTENDED), hWnd, (DLGPROC)CategoryChooseDlgProc) != IDOK)
                            break;

                        strcpy(zFile, "EXPORT");
                        sOF.lStructSize = sizeof(sOF);
                        sOF.hwndOwner = hWnd;
                        sOF.lpstrFilter = "NCD Files (*.NCD)\0*.NCD\0Comma Separated Files (*.CSV)\0*.CSV\0All Files (*.*)\0*.*\0\0";
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

                        if (GetSaveFileName(&sOF))
                            Export(hWnd, zFile, sOF.nFilterIndex);

						DBFreeChoosenCategories();
					}            
                    break;

                    case ID_FILE_IMPORT: {
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

                    case ID_FILE_REPORT: {
                        OPENFILENAME sOF;
                        char zFile[513];
    
                        // Get category
                        gs.bMultiselectCategories = TRUE;
                        if (DialogBox(gs.hMainInstance, MAKEINTRESOURCE(IDD_CATEGORYCHOOSEEXTENDED), hWnd, (DLGPROC)CategoryChooseDlgProc) != IDOK)
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

                        if (GetSaveFileName(&sOF))
                            Report(hWnd, zFile);

						DBFreeChoosenCategories();
                    }            
                    break;

                    case ID_FILE_STATISTICS: {
                        char zStr[80];

                        sprintf(zStr, "%d Artists\n%d CDs", nNumArtists, nNumCDs);

                        MessageBox(hWnd, zStr, "Statistics", MB_OK | MB_ICONINFORMATION);
                    }            
                    break;

					case ID_VIEW_BYARTIST: {
						BuildList(hWnd, 0);
					}
					break;

					case ID_VIEW_BYCATEGORY: {
						BuildList(hWnd, 1);
					}
					break;

                    case IDCANCEL:
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

                        if (GetChanged() && MessageBox(hWnd, "Do you want to save changes?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
                            SaveList();

                        gs.bInDBDlg = FALSE;

                        DBClose();

                        EndDialog(hWnd, TRUE);

						delete poArtistList;
						poArtistList = NULL;

						delete poCategoryList;
						poCategoryList = NULL;
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
            unsigned int nLoop;
			unsigned int nIndex;

            // Fill category list
            for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++) {
                nIndex = SendDlgItemMessage(hWnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)gs.ppzCategories[nLoop]);

				if (gs.bMultiselectCategories)
					SendDlgItemMessage(hWnd, IDC_LIST, LB_SETSEL, TRUE, nIndex);
			}
        }
        break;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK: {
                    unsigned int nLoop;

                    if (gs.bMultiselectCategories) {
					    nNumOfChoosenCategories = SendDlgItemMessage(hWnd, IDC_LIST, LB_GETSELCOUNT, 0, 0);
					    if (nNumOfChoosenCategories) {
						    unsigned int* pnSelItems;
						    char szTmp[80];

						    ppzChoosenCategories = (char**) malloc(nNumOfChoosenCategories * sizeof(char*));
						    pnSelItems = (unsigned int*) malloc(nNumOfChoosenCategories * sizeof(unsigned int));

						    SendDlgItemMessage(hWnd, IDC_LIST, LB_GETSELITEMS, nNumOfChoosenCategories, (LPARAM)pnSelItems);

						    for (nLoop = 0 ; nLoop < nNumOfChoosenCategories ; nLoop ++) {
							    SendDlgItemMessage(hWnd, IDC_LIST, LB_GETTEXT, pnSelItems[nLoop], (LPARAM)szTmp);
							    ppzChoosenCategories[nLoop] = strdup(szTmp);
						    }

						    free(pnSelItems);
					    }
                    }
                    else {
					    int nSel = SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0);
                        if (nSel != LB_ERR) {
						    char szTmp[80];

                            nNumOfChoosenCategories = 1;
                            ppzChoosenCategories = (char**) malloc(nNumOfChoosenCategories * sizeof(char*));

                            SendDlgItemMessage(hWnd, IDC_LIST, LB_GETTEXT, nSel, (LPARAM)szTmp);
							ppzChoosenCategories[0] = strdup(szTmp);
                        }
                    }

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


////////////////////////////////////////////////////////////////////////////
//
// IMPORT/EXPORT/REPORT stuff
//
////////////////////////////////////////////////////////////////////////////

void Export(HWND hWnd, char* pzFile, int nType)
{
    FILE* fp;
    sArtist* psArtist;
    sCD* psCD;
    int nCount = 0;
	unsigned int nLoop;
	char xListSeparator = ',';

	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, &xListSeparator, sizeof(xListSeparator));

    fp = fopen(pzFile, "w");
    if (!fp) {
        MessageBox(hWnd, "Error opening file", APPNAME, MB_OK | MB_ICONERROR);
        
        return;
    }

	if (nType == 1) {
		fputs("V1.1\n", fp);

		if (gs.nOptions & OPTIONS_USECDDB)
			fputs("CDDB\n", fp);
		else
			fputs("INI\n", fp);
	}
	else if (nType == 2) {
//		fputs("ID,Artist,Title,Category,Track Number,Track Title\n", fp);
	}

    ProgressOpen(NULL, "Exporting records", FALSE, poArtistList->NumItems());
    ProgressSet(0);

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    psArtist = poArtistList->First();
    while(psArtist) {
        psCD = psArtist->oCDList.First();
        while(psCD) {
            if (DBCheckCategory(psCD->sDI.pzCategory)) {
                if (nType == 1) {
					// Write Artist, Title, num tracks and tracks
					// Make sure we have read frames if we use the CDDB database
					if (((gs.nOptions & OPTIONS_USECDDB) && psCD->sDI.pnFrames) || !(gs.nOptions & OPTIONS_USECDDB)) {
						fputs(psCD->sDI.zMCIID, fp);
						fputs("\n", fp);
						fputs(psCD->sDI.pzArtist, fp);
						fputs("\n", fp);
						fputs(psCD->sDI.pzTitle, fp);
						fputs("\n", fp);
						fputs(psCD->sDI.pzCategory, fp);
						fputs("\n", fp);
						fprintf(fp, "%d\n", psCD->sDI.nTracks);

						for (nLoop = 0 ; nLoop < psCD->sDI.nTracks ; nLoop ++) {
							fputs(psCD->sDI.ppzTracks[nLoop], fp);
							fputs("\n", fp);
						}

						// Write out frames and disc length if we use CDDB
						if (gs.nOptions & OPTIONS_USECDDB) {
							fprintf(fp, "%d\n", psCD->sDI.nDiscLength);
							fprintf(fp, "%d\n", psCD->sDI.nTracks);

							for (nLoop = 0 ; nLoop < psCD->sDI.nTracks ; nLoop ++) 
								fprintf(fp, "%d\n", psCD->sDI.pnFrames[nLoop]);
						}

						fprintf(fp, "--\n");
					}
				}
				else if (nType == 2) {
				    for (nLoop = 0 ; nLoop < psCD->sDI.nTracks ; nLoop ++) {
                        fprintf(fp, "%s%c\"%s\"%c\"%s\"%c\"%s\"%c%d%c\"%s\"\n", 
                            psCD->sDI.zMCIID,
							xListSeparator,
                            psCD->sDI.pzCategory,
							xListSeparator,
                            psCD->sDI.pzArtist,
							xListSeparator,
                            psCD->sDI.pzTitle,
							xListSeparator,
                            nLoop + 1,
							xListSeparator,
                            psCD->sDI.ppzTracks[nLoop]);
                    }
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
        MessageBox(hWnd, "Error opening file", APPNAME, MB_OK | MB_ICONERROR);
        
        return;
    }

    fgets(zVersion, 32, fp);
    zVersion[strlen(zVersion) - 1] = 0;

    if (strcmp(zVersion, "V1.0") && strcmp(zVersion, "V1.1")) {
        fclose(fp);

        MessageBox(hWnd, "Unknown format or wrong format version", APPNAME, MB_OK | MB_ICONERROR);

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
            MessageBox(hWnd, "Unknown database format", APPNAME, MB_OK | MB_ICONERROR);

            fclose(fp);

            return;
        }
    }

    if (bINI && (gs.nOptions & OPTIONS_USECDDB)) {
        MessageBox(hWnd, "Database exported from INI file can't be imported to the CDDB format", APPNAME, MB_OK | MB_ICONSTOP);

        fclose(fp);

        return;
    }
    else if (!bINI && !(gs.nOptions & OPTIONS_USECDDB)) {
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
                if (!strcmp(psCD->sDI.pzTitle, zTitle))
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

                psCD->sDI.pzTitle = strdup(zTitle);
                psCD->sDI.pzCategory = strdup(zCategory);
                strcpy(psCD->sDI.zMCIID, zID);
                strcpy(psCD->sDI.zCDDBID, zID);
                psCD->bChanged = TRUE;

				psCD->sDI.ppzTracks = (char**) malloc(nNum*sizeof(char*));

                for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
                    fgets(zTitle, 255, fp);
                    zTitle[strlen(zTitle)-1] = 0;

                    psCD->sDI.ppzTracks[nLoop] = strdup(zTitle);
                }

                // Read CDDB extra info if exported in CDDB format
                if (!bINI) {
                    int nLoop;
                    int nNum;

                    fgets(zNum, 32, fp);
                    zNum[strlen(zNum) - 1] = 0;
                    
                    psCD->sDI.nDiscLength = atoi(zNum);
                    fgets(zNum, 32, fp);
                    zNum[strlen(zNum) - 1] = 0;

                    nNum = atoi(zNum);

                    psCD->sDI.pnFrames = (unsigned int*) malloc(nNum*sizeof(unsigned int));

                    for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
                        fgets(zNum, 32, fp);
                        zNum[strlen(zNum) - 1] = 0;   

                        psCD->sDI.pnFrames[nLoop] = atoi(zNum);
                    }
                }
                else {
                    psCD->sDI.pnFrames = NULL;
                    psCD->sDI.nDiscLength = 0;
                }

                SaveEntry(psCD);
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


void Report(HWND hWnd, char* pzFile)
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
            if (DBCheckCategory(psCD->sDI.pzCategory)) {
                if (!bArtistWritten) {
                    fputs(psCD->sDI.pzArtist, fp);
                    fputs("\n", fp);
                    fputs("--------------------------------------------------------------------\n", fp);

                    bArtistWritten = TRUE;
                }

                fputs("         ", fp);
                fputs(psCD->sDI.pzTitle, fp);
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


