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

#ifndef __CLIST_H__
#define __CLIST_H__

template<class TYPE> 
class cList
{
protected:
	struct sEntry {
		TYPE* psItem;
		sEntry* psNext;
		sEntry* psPrev;
	}*psFirst, *psLast, *psCurr;

	unsigned int nItems;
		
public:

	cList() {
		psFirst = psLast = NULL;

		nItems = 0;
	}

	cList(cList& roList);

	~cList() {
		while(First())
			Delete();
	}

	unsigned int NumItems() { return nItems; }

	void Add(TYPE* psItem) {
		if (!psFirst) {
			psFirst = psLast = new sEntry;

			psFirst->psNext = psFirst->psPrev = NULL;
		}
		else {
			psLast->psNext = new sEntry;
			psLast->psNext->psPrev = psLast;

			psLast = psLast->psNext;
			psLast->psNext = NULL;		
		}

		psLast->psItem = psItem;
		psCurr = psLast;

		nItems ++;
	}

	void Delete(char xDelData = TRUE) {
		sEntry* psTmp;

		if (psCurr == psFirst) {
			psFirst = psCurr->psNext;

			if (psFirst)
				psFirst->psPrev = NULL;

			psTmp = psFirst;
		}
		else if (psCurr == psLast) {
			psLast = psCurr->psPrev;

			psLast->psNext = NULL;

			psTmp = psLast;
		}
		else {
			psCurr->psPrev->psNext = psCurr->psNext;
			psCurr->psNext->psPrev = psCurr->psPrev;

			psTmp = psCurr->psNext;
		}

		if (xDelData)
			delete psCurr->psItem;
		delete psCurr;

		psCurr = psTmp;

		nItems --;
	}

	TYPE* First() {
		psCurr = psFirst;

		if (psCurr)
			return psCurr->psItem;
		else
			return NULL;
	}

	TYPE* Last() {
		psCurr = psLast;

		if (psCurr)
			return psCurr->psItem;
		else
			return NULL;
	}

	TYPE* Next() {
		psCurr = psCurr->psNext;

		if (psCurr)
			return psCurr->psItem;
		else
			return NULL;
	}

	TYPE* Prev() {
		psCurr = psCurr->psPrev;

		if (psCurr)
			return psCurr->psItem;
		else
			return NULL;
	}

	TYPE* operator++() {
		return Next();
	}

	TYPE* operator--() {
		return Prev();
	}

	TYPE* operator () () {
		return psCurr->psItem;
	}

    void Sort(int(*pfSortCB)(void*, void*)) {
        BOOL bSorted = TRUE;
        void* pvPtr;

        if (!psFirst)
            return;

        while(bSorted) {
            bSorted = FALSE;

            psCurr = psFirst;
            while(psCurr->psNext) {
                if (pfSortCB(psCurr->psItem, psCurr->psNext->psItem) > 0) {
                    pvPtr = psCurr->psNext->psItem;

                    psCurr->psNext->psItem = psCurr->psItem;
                    psCurr->psItem = (TYPE*)pvPtr;

                    bSorted = TRUE;
                }
                else
                    psCurr = psCurr->psNext;
            }
        }
    }
};

#endif

