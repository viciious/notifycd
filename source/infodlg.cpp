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

#include "common.h"

#include <commctrl.h>

#include "res/resource.h"

#include "ntfy_cd.h"

#include "infodlg.h"
#include "misc.h"
#include "db.h"

extern GLOBALSTRUCT gs;

///////////////////////////////////////////////////////////////////////////
//
// InfoDlg
//
///////////////////////////////////////////////////////////////////////////

void UpdatePlaylistTime(HWND hWnd,
						DISCINFO* psDI);

struct {
	char* pzTabName;
	int nDialogTemplate;
	HWND hTabWnd;
} asInfoTabs[] = {
	{"Playlist", IDD_INFO_TAB_PLAYLIST, NULL},
	{"More CD info", IDD_INFO_TAB_DISCINFO, NULL},
	{"More track info", IDD_INFO_TAB_TRACKINFO, NULL},
};

#define NUM_INFO_TABS (sizeof(asInfoTabs) / sizeof(asInfoTabs[0]))

int nCurrInfoTab;
HWND hInfoTabCtrl;
HWND hInfoDlg;

BOOL CALLBACK InfoTabDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

void InfoChangeTab()
{
	ShowWindow(asInfoTabs[nCurrInfoTab].hTabWnd, SW_HIDE);

	nCurrInfoTab = TabCtrl_GetCurSel(hInfoTabCtrl);

	ShowWindow(asInfoTabs[nCurrInfoTab].hTabWnd, SW_SHOW);
}


BOOL InfoDlgNotifyHandler(HWND hWnd, UINT /*nMsg*/, WPARAM wParam, LPARAM lParam)
{
    switch(wParam) {
        case IDC_TAB: {
			NMHDR* psNM = (NMHDR*) lParam;
            
            switch(psNM->code) {
                case TCN_SELCHANGE: {
					InfoChangeTab();

                    if (nCurrInfoTab == 1) {
                        EnableWindow(GetDlgItem(hWnd, IDC_SETTRACK), FALSE);
                        EnableWindow(GetDlgItem(hWnd, IDC_TRACK), FALSE);
                    }
                    else {
                        unsigned int nCurSel;
                        unsigned int nID;
                        HWND hTab;
						INFODLGPARAM* psParam = (INFODLGPARAM*)GetWindowLong(hWnd, GWL_USERDATA);
						DISCINFO* psDI = NULL;

						if (psParam)
							psDI = psParam->psDI;

                        EnableWindow(GetDlgItem(hWnd, IDC_SETTRACK), TRUE);
                        EnableWindow(GetDlgItem(hWnd, IDC_TRACK), TRUE);

						if (psDI)
						{
							EnterCriticalSection(&gs.sDiscInfoLock);

							if (nCurrInfoTab == 0)
								nID = IDC_TRACKS;
							else if (nCurrInfoTab == 2)
								nID = IDC_TRACKS2;
							hTab = asInfoTabs[nCurrInfoTab].hTabWnd;

							nCurSel = SendMessage(GetDlgItem(hTab, nID), LB_GETCURSEL, 0, 0);
							if (nCurSel != LB_ERR) {
								SetWindowText(GetDlgItem(hInfoDlg, IDC_TRACK), psDI->ppzTracks[nCurSel]);
								SendMessage(GetDlgItem(hInfoDlg, IDC_TRACK), EM_SETSEL, 0, -1);
							}

							LeaveCriticalSection(&gs.sDiscInfoLock);
						}
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}


BOOL CALLBACK InfoTabDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	DISCINFO* psDI;

	psDI = (DISCINFO*)GetWindowLong(hWnd, GWL_USERDATA);

    switch(nMsg) {
    	case WM_INITDIALOG: {
			SetWindowLong(hWnd, GWL_USERDATA, lParam);
            gs.bInInfoDlg = TRUE;

            CenterWindow(hWnd);
        }
		break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == LBN_DBLCLK) {
                switch(LOWORD(wParam)) {
					case IDC_TRACKS:
					case IDC_TRACKS2: {
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDC_ADD, BN_CLICKED), 0L);
                    }
                    break;

                    case IDC_PLAYLIST: {
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDC_REMOVE, BN_CLICKED), 0L);
                    }
                    break;
                }
            }
            else if (HIWORD(wParam) == EN_KILLFOCUS) {
                switch(LOWORD(wParam)) {
					case IDC_TRACKINFO: {
						int nSel;

						nSel = SendDlgItemMessage(hWnd, IDC_TRACKS2, LB_GETCURSEL, 0, 0);
						if (nSel != LB_ERR) {
							int nLen;

							nLen = GetWindowTextLength(GetDlgItem(hWnd, IDC_TRACKINFO));

						    EnterCriticalSection(&gs.sDiscInfoLock);

							delete[] psDI->ppzTracksExt[nSel];
							psDI->ppzTracksExt[nSel] = new char[nLen + 1];
							GetWindowText(GetDlgItem(hWnd, IDC_TRACKINFO), psDI->ppzTracksExt[nSel], nLen + 1);

							LeaveCriticalSection(&gs.sDiscInfoLock);
						}
					}
					break;
				}
			}
            else if (HIWORD(wParam) == LBN_SELCHANGE) {
                switch(LOWORD(wParam)) {				    
                    case IDC_TRACKS: {
						if (nCurrInfoTab == 0) {
							int* pnTracks = NULL;
							int* pnSel = NULL;
							int nLoop;
							char szTmp[300];
							int nNum;
							int nSecs = 0;
							int nMin;

							EnterCriticalSection(&gs.sDiscInfoLock);
	
							if (psDI->pnTrackLen) {
								// Get playlist

								nNum = SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELCOUNT, 0, 0);
								if (nNum) {
									pnSel = new int[nNum];
    								SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELITEMS, nNum, (LPARAM)pnSel);

									pnTracks = new int[nNum];
									for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
        								SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETTEXT, pnSel[nLoop], (LPARAM)szTmp);
										sscanf(szTmp, "%d.\t", &pnTracks[nLoop]);
										pnTracks[nLoop] --;
									}

									for (nLoop = 0 ; nLoop < nNum ; nLoop ++) 
										nSecs += psDI->pnTrackLen[pnTracks[nLoop]];
								}

								// Calc time!

								nMin = nSecs / 60;
								nSecs %= 60;

								sprintf(szTmp, "(Selected time: %02d:%02d)", nMin, nSecs);
							}
							else
								strcpy(szTmp, "(Selected time: N/A)");

							SetDlgItemText(hWnd, IDC_SELTIME, szTmp);

							if (pnTracks)
								delete[] pnTracks;
							if (pnSel)
								delete[] pnSel;

							int nCurrSel;

							nCurrSel = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
							if (nCurrSel != LB_ERR) {
								SetWindowText(GetDlgItem(hInfoDlg, IDC_TRACK), psDI->ppzTracks[nCurrSel]);
								SendMessage(GetDlgItem(hInfoDlg, IDC_TRACK), EM_SETSEL, 0, -1);
							}

							LeaveCriticalSection(&gs.sDiscInfoLock);
						}
					}
					break;

                    case IDC_TRACKS2: {
					    EnterCriticalSection(&gs.sDiscInfoLock);

						if (nCurrInfoTab == 2) {
							int nCurrSel;

							nCurrSel = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
                            if (nCurrSel != LB_ERR) {
								SetWindowText(GetDlgItem(hWnd, IDC_TRACKINFO), psDI->ppzTracksExt[nCurrSel]);
								
                                SetWindowText(GetDlgItem(hInfoDlg, IDC_TRACK), psDI->ppzTracks[nCurrSel]);
								SendMessage(GetDlgItem(hInfoDlg, IDC_TRACK), EM_SETSEL, 0, -1);
                            }
						}
						
						LeaveCriticalSection(&gs.sDiscInfoLock);
                    }
				    break;
                }
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                // This is because the tab is it's own dialog and when I click the button it seems to
                // decide to set it as the default button for the tab dialog...

                ChangeDefButton(hWnd, IDOK, LOWORD(wParam));

                switch(LOWORD(wParam)) {
				    case IDC_ADD: {
                        int nSelCount;

					    EnterCriticalSection(&gs.sDiscInfoLock);

					    nSelCount = SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELCOUNT, 0, 0);
					    if (nSelCount) {
						    int* pnSel = new int[nSelCount];
						    int nLoop;
						    char szTmp[300];

						    SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELITEMS, nSelCount, (LPARAM) pnSel);
						    for (nLoop = 0 ; nLoop < nSelCount ; nLoop ++) {			
							    sprintf(szTmp, "%d.\t%s", pnSel[nLoop] + 1, psDI->ppzTracks[pnSel[nLoop]]);
							    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LONG)szTmp);
						    }

						    delete [] pnSel;
					    }

					    LeaveCriticalSection(&gs.sDiscInfoLock);

                        UpdatePlaylistTime(hWnd, psDI);
				    }
				    break;

				    case IDC_INSERT: {
                        int nCurSel;
                        int nSelCount;

					    EnterCriticalSection(&gs.sDiscInfoLock);

                        nCurSel = SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETCURSEL, 0, 0);

					    nSelCount = SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELCOUNT, 0, 0);
					    if (nSelCount) {
						    int* pnSel = new int[nSelCount];
						    int nLoop;
						    char szTmp[300];

						    SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELITEMS, nSelCount, (LPARAM) pnSel);
						    for (nLoop = 0 ; nLoop < nSelCount ; nLoop ++) {			
							    sprintf(szTmp, "%d.\t%s", pnSel[nLoop] + 1, psDI->ppzTracks[pnSel[nLoop]]);
							    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_INSERTSTRING, nCurSel+nLoop, (LONG)szTmp);
						    }

						    delete [] pnSel;
					    }

					    LeaveCriticalSection(&gs.sDiscInfoLock);

                        UpdatePlaylistTime(hWnd, psDI);
				    }
				    break;

				    case IDC_REMOVE: {
                        int nSelCount;

					    nSelCount = SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETSELCOUNT, 0, 0);
					    while (nSelCount) {
						    int anSel[1];

						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETSELITEMS, 1, (LPARAM) anSel);
						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_DELETESTRING, anSel[0], 0);

						    nSelCount --;
					    }

                        UpdatePlaylistTime(hWnd, psDI);
				    }
				    break;

				    case IDC_CLEAR: {
					    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_RESETCONTENT, 0, 0);

                        UpdatePlaylistTime(hWnd, psDI);
				    }
				    break;

				    case IDC_RESET: {
					    unsigned int nLoop;
					    char szTmp[300];

					    EnterCriticalSection(&gs.sDiscInfoLock);

					    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_RESETCONTENT, 0, 0);

					    for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
						    sprintf(szTmp, "%d.\t%s", nLoop + 1, psDI->ppzTracks[nLoop]);
						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LPARAM) szTmp);
					    }

						LeaveCriticalSection(&gs.sDiscInfoLock);
		
                        UpdatePlaylistTime(hWnd, psDI);
				    }
				    break;

                    case IDC_RANDOM: {
					    EnterCriticalSection(&gs.sDiscInfoLock);

                        unsigned int* pnRandomTracks = new unsigned int[psDI->nMCITracks];
                        unsigned int nLoop, nLoop2;
                        unsigned int nCount = 0;
                        BOOL bUnique;
                        char zTmp[300];
                        
                        for (nLoop = 0 ; nLoop < psDI->nMCITracks ; nLoop ++) {
                            bUnique = FALSE;

                            while (!bUnique) {
                                pnRandomTracks[nCount] = rand() % psDI->nMCITracks;

                                bUnique = TRUE;

                                for (nLoop2 = 0 ; nLoop2 < nCount ; nLoop2 ++){
                                    if (pnRandomTracks[nCount] == pnRandomTracks[nLoop2])
                                        bUnique = FALSE;
                                }
                            }

						    sprintf(zTmp, "%d.\t%s", pnRandomTracks[nCount]+1, psDI->ppzTracks[pnRandomTracks[nCount]]);
						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LPARAM) zTmp);

                            nCount ++;
                        }

						LeaveCriticalSection(&gs.sDiscInfoLock);
	
                        UpdatePlaylistTime(hWnd, psDI);
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


void InitInfoDlg(HWND hWnd, 
				 DISCINFO* psDI,
				 BOOL bAddTabs)
{
	TC_ITEM sTabItem;
	unsigned int nLoop;
	RECT sRect;
	int nInfoX;
	int nInfoY;
	char szTmp[300];
    unsigned int nWidth;
    SIZE sSize;
    HDC hDC;
	char* pzTmp;
	
    EnterCriticalSection(&gs.sDiscInfoLock);

    if (bAddTabs) {
        // Init tabs

	    hInfoTabCtrl = GetDlgItem(hWnd, IDC_TAB);

	    for (nLoop = 0 ; nLoop < NUM_INFO_TABS ; nLoop ++) {
		    sTabItem.mask = TCIF_TEXT; 
		    sTabItem.pszText = asInfoTabs[nLoop].pzTabName; 
		    TabCtrl_InsertItem(hInfoTabCtrl, nLoop, &sTabItem); 
	    }
	    
	    GetClientRect(hInfoTabCtrl, &sRect);

	    TabCtrl_AdjustRect(hInfoTabCtrl, FALSE, &sRect); 

	    nInfoX = sRect.left;
	    nInfoY = sRect.top;

	    for (nLoop = 0 ; nLoop < NUM_INFO_TABS ; nLoop ++) {
		    nCurrInfoTab = nLoop;

		    asInfoTabs[nLoop].hTabWnd = CreateDialogParam(gs.hMainInstance, MAKEINTRESOURCE(asInfoTabs[nLoop].nDialogTemplate), hInfoTabCtrl, InfoTabDlgProc, (LPARAM)psDI);
		    GetClientRect(asInfoTabs[nLoop].hTabWnd, &sRect);

		    MoveWindow(asInfoTabs[nLoop].hTabWnd, nInfoX, nInfoY, sRect.right, sRect.bottom, FALSE);	
	    }

	    nCurrInfoTab = 0;

	    InfoChangeTab();
    }

	char zID[32];
	char zTmp[80];
    int nTabs[2] = {15, 20};

    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_TRACKS, LB_SETTABSTOPS, 1, (LPARAM) nTabs);
    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_TRACKS, LB_RESETCONTENT, 0, 0);

    SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_SETTABSTOPS, 1, (LPARAM) nTabs);
    SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_RESETCONTENT, 0, 0);

    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_PLAYLIST, LB_SETTABSTOPS, 1, (LPARAM) nTabs);
    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_PLAYLIST, LB_RESETCONTENT, 0, 0);

    ProfileGetString( "ALIASES", psDI->zMCIID, psDI->zMCIID, zID, sizeof(zID) );

	StringPrintf(zTmp, sizeof(zTmp), "ID: %s", psDI->zCDDBID);
	SetWindowText(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_CURRID), zTmp);
	if (stricmp(psDI->zCDDBID, zID)) {
		StringPrintf(zTmp, sizeof(zTmp), "Alias: %s", zID);
		SetWindowText(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_ALIASID), zTmp);
	}
	else
		SetWindowText(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_ALIASID), "");   

    if (nCurrInfoTab == 0) {
        nWidth = 0;
        
        hDC = GetDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST));

        for (nLoop = 0 ; nLoop < psDI->nProgrammedTracks; nLoop ++) {
			sprintf(szTmp, "%d.\t%s", psDI->pnProgrammedTracks[nLoop]+1, psDI->ppzTracks[psDI->pnProgrammedTracks[nLoop]]);
			SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LPARAM) szTmp);

            GetTextExtentPoint32(hDC, szTmp, strlen(szTmp), &sSize);

            nWidth = max(nWidth, (unsigned)sSize.cx);
		}

        SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_SETHORIZONTALEXTENT, nWidth, 0);

        ReleaseDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), hDC);
    }

    nWidth = 0;
    
    hDC = GetDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS));

	for (nLoop = 0 ; nLoop < psDI->nTracks ; nLoop ++) {
		sprintf(szTmp, "%d.\t%s", nLoop + 1, psDI->ppzTracks[nLoop]);
		SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_ADDSTRING, 0, (LPARAM) szTmp);
		SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_ADDSTRING, 0, (LPARAM) szTmp);

        GetTextExtentPoint32(hDC, szTmp, strlen(szTmp), &sSize);

        nWidth = max(nWidth, (unsigned)sSize.cx);
	}

    ReleaseDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), hDC);

    SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_SETHORIZONTALEXTENT, nWidth, 0);
    SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_SETHORIZONTALEXTENT, nWidth, 0);

	SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_SETCURSEL, 0, 0);
	SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_SETCURSEL, 0, 0);
    
    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_TRACKS, LB_SETCURSEL, 0, -1);
    SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_SETCURSEL, 0, -1);

    UpdatePlaylistTime(asInfoTabs[0].hTabWnd, psDI);

    SetWindowText(GetDlgItem(asInfoTabs[1].hTabWnd, IDC_DISCINFO), psDI->pzDiscExt);

    SetWindowText(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKINFO), psDI->ppzTracksExt[0]);

    pzTmp = NULL;

	SetWindowText(GetDlgItem(hWnd, IDC_ARTIST), psDI->pzArtist);
	SetWindowText(GetDlgItem(hWnd, IDC_TITLE), psDI->pzTitle);
	
	SetActiveWindow(GetDlgItem(hWnd, IDC_ARTIST));

	SetWindowText(GetDlgItem(hWnd, IDC_TRACK), psDI->ppzTracks[0]);

	// Set category
	SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_SELECTSTRING, (WPARAM) -1, (LPARAM)psDI->pzCategory);

    delete[] pzTmp;

    LeaveCriticalSection(&gs.sDiscInfoLock);
}


BOOL SetDiscInfoFromInfoDlg(HWND hWnd,
							DISCINFO* psDI,
                            BOOL bSave)
{
    unsigned int nLoop;
    char szTmp[1024];
	unsigned int nNum;
    unsigned int nLen;
	unsigned int nSel;
	BOOL bRet = TRUE;

    EnterCriticalSection(&gs.sDiscInfoLock);

    delete[] psDI->pzArtist;
    nLen = GetWindowTextLength(GetDlgItem(hWnd, IDC_ARTIST));
    psDI->pzArtist = new char[nLen + 1];
    GetWindowText(GetDlgItem(hWnd, IDC_ARTIST), psDI->pzArtist, nLen + 1);

    delete[] psDI->pzTitle;
    nLen = GetWindowTextLength(GetDlgItem(hWnd, IDC_TITLE));
    psDI->pzTitle = new char[nLen + 1];
    GetWindowText(GetDlgItem(hWnd, IDC_TITLE), psDI->pzTitle, nLen + 1);
    
    delete[] psDI->pzDiscExt;
    nLen = GetWindowTextLength(GetDlgItem(asInfoTabs[1].hTabWnd, IDC_DISCINFO));
    psDI->pzDiscExt = new char[nLen + 1];
    GetWindowText(GetDlgItem(asInfoTabs[1].hTabWnd, IDC_DISCINFO), psDI->pzDiscExt, nLen + 1);

	nSel = SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_GETCURSEL, 0, 0);
	if (nSel != LB_ERR) {
		int nLen;

		nLen = GetWindowTextLength(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKINFO));

		delete[] psDI->ppzTracksExt[nSel];
		psDI->ppzTracksExt[nSel] = new char[nLen + 1];
		GetWindowText(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKINFO), psDI->ppzTracksExt[nSel], nLen + 1);
	}

    // Get category
    if (SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_GETLBTEXT, SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_GETCURSEL, 0, 0), (LPARAM)szTmp) != CB_ERR) {
        // Category has changed. Delete old entry in case we use CDDB
        if (stricmp(szTmp, psDI->pzCategory) && (gs.nOptions & OPTIONS_USECDDB))
            DBDelete(psDI);

        delete[] psDI->pzCategory;
		psDI->pzCategory = StringCopy( szTmp );
	}

	// Get playlist

	delete[] psDI->pnProgrammedTracks;

    nNum = SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_GETCOUNT, 0, 0);
	if (!nNum) {
		psDI->nProgrammedTracks = psDI->nMCITracks;

		psDI->pnProgrammedTracks = new unsigned int[psDI->nProgrammedTracks];
		for (nLoop = 0 ; nLoop < psDI->nProgrammedTracks ; nLoop ++)
			psDI->pnProgrammedTracks[nLoop] = nLoop;
	}
	else {
		psDI->nProgrammedTracks = nNum;

		psDI->pnProgrammedTracks = new unsigned int[psDI->nProgrammedTracks];
		for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
			SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_GETTEXT, nLoop, (LONG)szTmp);
			sscanf(szTmp, "%d.\t", &psDI->pnProgrammedTracks[nLoop]);
			psDI->pnProgrammedTracks[nLoop] --;
		}
	}

    if (bSave)
        bRet = SetDiscInfo(psDI);

    LeaveCriticalSection(&gs.sDiscInfoLock);

	return bRet;
}


void ExitInfoDlg()
{
    int nLoop;

    for (nLoop = 0 ; nLoop < NUM_INFO_TABS ; nLoop ++)
		DestroyWindow(asInfoTabs[nLoop].hTabWnd);
}


BOOL CALLBACK InfoDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	INFODLGPARAM* psParam = (INFODLGPARAM*)GetWindowLong(hWnd, GWL_USERDATA);
    DISCINFO* psDI;

    if (psParam)
        psDI = psParam->psDI;

    switch(nMsg) {
    	case WM_INITDIALOG: {
            unsigned int nLoop;

			SetWindowLong(hWnd, GWL_USERDATA, lParam);

            psParam = (INFODLGPARAM*) lParam;
            psDI = psParam->psDI;

			hInfoDlg = hWnd;

            gs.bInInfoDlg = TRUE;

            CenterWindow(hWnd);

            // Fill category list
            SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_ADDSTRING, 0, (LPARAM)" ");
            for (nLoop = 0 ; nLoop < gs.nNumCategories ; nLoop ++)
                SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_ADDSTRING, 0, (LPARAM)gs.ppzCategories[nLoop]);

            // Fix ID's 
            InitInfoDlg(hWnd, psDI, TRUE);
        }
		break;

        case WM_NOTIFY: {
            return InfoDlgNotifyHandler(hWnd, nMsg, wParam, lParam);
        }
        break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_SETFOCUS || HIWORD(wParam) == EN_KILLFOCUS) {
                if (LOWORD(wParam) == IDC_TRACK) {
					if (HIWORD(wParam) == EN_SETFOCUS)
						ChangeDefButton(hWnd, IDC_SETTRACK, IDOK);
					else
						ChangeDefButton(hWnd, IDOK, IDC_SETTRACK);
				}
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                switch(LOWORD(wParam)) {
                    case IDC_SETTRACK: {
                        unsigned int nCurSel;
                        char szTmp[300];
                        unsigned int nID;
                        HWND hTab;

					    EnterCriticalSection(&gs.sDiscInfoLock);

                        if (nCurrInfoTab == 0) {
                            nID = IDC_TRACKS;
                            hTab = asInfoTabs[nCurrInfoTab].hTabWnd;
                        }
                        else if (nCurrInfoTab == 2) {
                            nID = IDC_TRACKS2;
                            hTab = asInfoTabs[nCurrInfoTab].hTabWnd;
                        }
                        else
                            break;

                        nCurSel = SendDlgItemMessage(hTab, nID, LB_GETCURSEL, 0, 0);
                        if (nCurSel != LB_ERR) {
						    unsigned int nLoop;
                            SIZE sSize;
                            HDC hDC;
                            unsigned int nWidth;

                            delete[] psDI->ppzTracks[nCurSel];
                            psDI->ppzTracks[nCurSel] = new char[GetWindowTextLength(GetDlgItem(hWnd, IDC_TRACK)) + 1];
                            GetWindowText(GetDlgItem(hWnd, IDC_TRACK), psDI->ppzTracks[nCurSel], GetWindowTextLength(GetDlgItem(hWnd, IDC_TRACK))+1);

                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_DELETESTRING, nCurSel, 0);
                            SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_DELETESTRING, nCurSel, 0);
                            StringPrintf(szTmp, sizeof(szTmp), "%d.\t%s", nCurSel+1, psDI->ppzTracks[nCurSel]);
                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_INSERTSTRING, nCurSel, (LPARAM) szTmp);
                            SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_INSERTSTRING, nCurSel, (LPARAM) szTmp);

                            hDC = GetDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS));
                            nWidth = SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_GETHORIZONTALEXTENT, 0, 0);
                            GetTextExtentPoint32(hDC, szTmp, strlen(szTmp), &sSize);
                            nWidth = max(nWidth, (unsigned )sSize.cx);
                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_SETHORIZONTALEXTENT, nWidth, 0);
                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_SETHORIZONTALEXTENT, nWidth, 0);
                            SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_SETHORIZONTALEXTENT, nWidth, 0);
                            ReleaseDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), hDC);

						    for (nLoop = 0 ; nLoop < psDI->nProgrammedTracks ; nLoop ++) {
							    if (psDI->pnProgrammedTracks[nLoop] == nCurSel) {
								    SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_DELETESTRING, nLoop, 0);
								    StringPrintf(szTmp, sizeof(szTmp), "%d.\t%s", psDI->pnProgrammedTracks[nLoop]+1, psDI->ppzTracks[nCurSel]);
								    SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_INSERTSTRING, nLoop, (LPARAM) szTmp);
							    }
						    }

                            if (nID == IDC_TRACKS) {
								nCurSel = (nCurSel + 1) % psDI->nMCITracks;
								SendDlgItemMessage(hTab, nID, LB_SETSEL, (WPARAM)TRUE, (LPARAM)nCurSel);
							}
							else {
	                            SendDlgItemMessage(hTab, nID, LB_SETCURSEL, (WPARAM)nCurSel, (LPARAM)0);
							}

							SetWindowText(GetDlgItem(hWnd, IDC_TRACK), psDI->ppzTracks[nCurSel]);
							SetActiveWindow(GetDlgItem(hWnd, IDC_TRACK)); 
							SendMessage(GetDlgItem(hWnd, IDC_TRACK), EM_SETSEL, 0, -1);
                        }

						LeaveCriticalSection(&gs.sDiscInfoLock);
                    }
                    break;

				    case IDC_INTERNET: {
                        POINT sPoint;
                        HMENU hMenu = LoadMenu(gs.hMainInstance, MAKEINTRESOURCE(IDR_INTERNET));
                        HMENU hPopup = GetSubMenu(hMenu, 0);

                        GetCursorPos(&sPoint);

                        TrackPopupMenu(hPopup, TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, sPoint.x, sPoint.y, 0, hWnd, NULL);

                        DestroyMenu(hMenu);                           
                    }
				    break;

                    case IDM_INTERNETGET: {
                        DISCINFO sQueryDI;

					    EnterCriticalSection(&gs.sDiscInfoLock);

                        InitDiscInfo(&sQueryDI);

                        strcpy(sQueryDI.zCDDBID, psDI->zCDDBID);
                        strcpy(sQueryDI.zMCIID, psDI->zMCIID);
                        sQueryDI.nMCITracks = psDI->nMCITracks;
                        sQueryDI.pnFrames = new unsigned int[psDI->nMCITracks];
                        CopyMemory(sQueryDI.pnFrames, psDI->pnFrames, psDI->nMCITracks*sizeof(unsigned int));
                        sQueryDI.nDiscLength = psDI->nDiscLength;

                        if (DBInternetGet(&sQueryDI, hWnd)) {
                            GetDiscInfo(gs.wDeviceID, &sQueryDI);
                            ValidateDiscInfo(gs.wDeviceID, &sQueryDI);

                            CopyDiscInfo(psDI, &sQueryDI);

                            GetDiscInfo(gs.wDeviceID, psDI);

                            InitInfoDlg(hWnd, psDI, FALSE);
                        }
                        else {
                            MessageBox(hWnd, "No disc info found on remote server", APPNAME, MB_OK | MB_ICONINFORMATION);
                        }

						LeaveCriticalSection(&gs.sDiscInfoLock);
                    }
                    break;

                    case IDM_INTERNETSEND: {
					    EnterCriticalSection(&gs.sDiscInfoLock);

                        if (SetDiscInfoFromInfoDlg(hWnd, psDI, TRUE)) 
                            DBInternetSend(psDI, hWnd);

						LeaveCriticalSection(&gs.sDiscInfoLock);
                    }
                    break;

                    case IDM_ADDTOQUEUE: {
					    EnterCriticalSection(&gs.sDiscInfoLock);

                        if (MessageBox(hWnd, "Do you want to add this disc to the queue for later retrieval?", APPNAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
						    AddToQueue(psDI);

						LeaveCriticalSection(&gs.sDiscInfoLock);
                    }
                    break;

                    case IDOK: {
                        DebugPrintf("-> InfoDlg SAVE");

					    EnterCriticalSection(&gs.sDiscInfoLock);

                        if (SetDiscInfoFromInfoDlg(hWnd, psDI, psParam->bSave)) {
                            gs.bInInfoDlg = FALSE;

					        ExitInfoDlg();

                            EndDialog(hWnd, TRUE);
                        }

					    LeaveCriticalSection(&gs.sDiscInfoLock);

                        DebugPrintf("<- InfoDlg SAVE");
                    }
                    break;

                    case IDCANCEL: {
                        gs.bInInfoDlg = FALSE;

                        ExitInfoDlg();

                        EndDialog(hWnd, FALSE);
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


void UpdatePlaylistTime(HWND hWnd,
						DISCINFO* psDI)
{
    int* pnTracks = NULL;
    int nLoop;
    char szTmp[300];
	int nNum;
    int nSecs = 0;
    int nMin;

	EnterCriticalSection(&gs.sDiscInfoLock);

	if (psDI->pnTrackLen) {
		// Get playlist

		nNum = SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETCOUNT, 0, 0);
		if (nNum) {
			pnTracks = new int[nNum];
			for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
				SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETTEXT, nLoop, (LONG)szTmp);
				sscanf(szTmp, "%d.\t", &pnTracks[nLoop]);
				pnTracks[nLoop] --;
			}

			for (nLoop = 0 ; nLoop < nNum ; nLoop ++) 
				nSecs += psDI->pnTrackLen[pnTracks[nLoop]];
		}

		// Calc time!

		nMin = nSecs / 60;
		nSecs %= 60;

		StringPrintf(szTmp, sizeof(szTmp), "(Play time: %02d:%02d)", nMin, nSecs);

		SetDlgItemText(hWnd, IDC_PLAYTIME, szTmp);

		if (pnTracks)
			delete[] pnTracks;
	}
	else
		SetDlgItemText(hWnd, IDC_PLAYTIME, "(Play time: N/A)");

	LeaveCriticalSection(&gs.sDiscInfoLock);
}


