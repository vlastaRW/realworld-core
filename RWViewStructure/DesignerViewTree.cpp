// DesignerViewTree.cpp : Implementation of CDesignerViewTree

#include "stdafx.h"
#include "DesignerViewTree.h"

#include <XPGUI.h>
#include <StringParsing.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h> // overkill
#include <RWDesignerCore.h>

//void CDesignerViewTree::CheckTree()
//{
//	ATLASSERT(m_cBackMap.size() == m_wndTree.GetCount());
//	CBackMap::iterator i;
//	for (i = m_cBackMap.begin(); i != m_cBackMap.end(); i++)
//	{
//		i->first->AddRef();
//		i->first->Release();
//		CBackMap::iterator iTmp = CastItemData(m_wndTree.GetItemData(i->second));
//		ATLASSERT(iTmp->first == i->first);
//	}
//}

// CDesignerViewTree

void CDesignerViewTree::Init(ISharedStateManager* a_pStateMgr, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID, IMenuCommandsManager* a_pCmdMgr)
{
	static INITCOMMONCONTROLSEX tICCE = {sizeof(tICCE), ICC_TREEVIEW_CLASSES};
	static BOOL bDummy = InitCommonControlsEx(&tICCE);

	m_pStatusBar = a_pStatusBar;
	m_pConfig = a_pConfig;
	m_nStyle = a_nStyle;
	m_pCmdMgr = a_pCmdMgr;

	CConfigValue cSyncGroup;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_TREE_SELSYNCGROUP), &cSyncGroup);
	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_TREE_KEYDOWNACTION), &cVal);
	m_eKeyDownAction = cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_TREE_SHOWITEMONFOCUS), &cVal);
	m_bEnsureVisibleOnKillFocus = cVal;

	m_pOriginal = a_pDoc;
	a_pDoc->QueryFeatureInterface(__uuidof(IStructuredItemsRichGUI), reinterpret_cast<void**>(&m_pRichGUI));
	if (m_pRichGUI)
	{
		BYTE b = 0;
		if (SUCCEEDED(m_pRichGUI->ClipboardPriority(&b)))
			m_nClipboardPriority = b;
	}
	a_pDoc->QueryFeatureInterface(__uuidof(IStructuredDocument), reinterpret_cast<void**>(&m_pDocument));
	if (m_pDocument != NULL)
	{
		// structured document supported
		m_pDocument->StructuredRootsEnum(&m_pRoots);
		if (m_pRoots == NULL)
			m_pDocument = NULL;
	}
	CComPtr<IStructuredRoot> pRoot;
	a_pDoc->QueryFeatureInterface(__uuidof(IStructuredRoot), reinterpret_cast<void**>(&pRoot));
	if (pRoot == NULL)
	{
		// structured document not supported and failed to obtain the generic structured root
		throw E_FAIL;
	}
	CComBSTR bstrPrefix;
	pRoot->StatePrefix(&bstrPrefix);
	if (bstrPrefix.Length())
	{
		m_bstrSyncGroup.Attach(bstrPrefix.Detach());
		m_bstrSyncGroup += cSyncGroup;
	}
	else
	{
		m_bstrSyncGroup.Attach(cSyncGroup.Detach().bstrVal);
	}

	m_tLocaleID = a_tLocaleID;

	a_pStateMgr->ObserverIns(CObserverImpl<CDesignerViewTree, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	m_pStateMgr = a_pStateMgr;

	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("Structure_Tree"), WS_CHILDWINDOW|WS_CLIPSIBLINGS, 0, 0, 0) == NULL)
	{
		// creation failed
		throw E_FAIL; // TODO: error code
	}

	ShowWindow(SW_SHOW);
}

void CDesignerViewTree::OwnerNotify(TCookie a_tCookie, TSharedStateChange a_pChangeParams)
{
	typedef set<HTREEITEM> CTreeItems;
	typedef vector<HTREEITEM> CTreeItemsOut;
	if (!m_bSelectionSending && (m_bstrSyncGroup == a_pChangeParams.bstrName))
	{
		m_pLastSel = a_pChangeParams.pState;
		CComPtr<IEnumUnknowns> pSelectedItems;
		m_pRoot->StateUnpack(a_pChangeParams.pState, &pSelectedItems);
		if (pSelectedItems != NULL)
		{
			CTreeItems cNewSel;
			ULONG nSize = 0;
			pSelectedItems->Size(&nSize);
			for (ULONG i = 0; i < nSize; i++)
			{
				CComPtr<IComparable> pItem;
				pSelectedItems->Get(i, __uuidof(IComparable) , reinterpret_cast<void**>(&pItem));
				if (pItem != NULL)
				{
					CBackMap::const_iterator iItem = m_cBackMap.find(pItem);
					if (iItem != m_cBackMap.end())
					{
						cNewSel.insert(iItem->second);
					}
				}
			}
			CTreeItemList cList;
			GetSelectedList(cList);
			CTreeItemsOut cOldSel;
			copy(cList.GetData(), cList.GetData()+cList.GetSize(), back_inserter(cOldSel));
			sort(cOldSel.begin(), cOldSel.end());
			CTreeItemsOut cToSel;
			CTreeItemsOut cToUnsel;
			set_difference(cOldSel.begin(), cOldSel.end(), cNewSel.begin(), cNewSel.end(), back_inserter(cToUnsel));
			set_difference(cNewSel.begin(), cNewSel.end(), cOldSel.begin(), cOldSel.end(), back_inserter(cToSel));

			if (cToSel.size() != 0 || cToUnsel.size() != 0)
			{
				m_bSelectionSending = true;
				m_wndTree.SetRedraw(FALSE);
				CTreeItemsOut::const_iterator i;
				for (i = cToUnsel.begin(); i != cToUnsel.end(); i++)
				{
					m_wndTree.SetItemState(*i, 0, TVIS_SELECTED);
				}
				bool bStdSelect = true;
				HTREEITEM hFocused = m_wndTree.GetNextItem(TVI_ROOT, TVGN_CARET);
				if (hFocused)
				{
					if ((m_wndTree.GetItemState(hFocused, TVIS_SELECTED)&TVIS_SELECTED) == TVIS_SELECTED)
					{
						bStdSelect = false;
					}
				}
				for (i = cToSel.begin(); i != cToSel.end(); i++)
				{
					m_wndTree.SetItemState(*i, TVIS_SELECTED, TVIS_SELECTED);
					if (bStdSelect)
					{
						// move focus to selected item
						m_wndTree.SelectItem(*i);
						bStdSelect = false;
					}
					m_wndTree.EnsureVisible(*i);
				}
				m_wndTree.SetRedraw(TRUE);
				//RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
				m_bSelectionSending = false;
			}
			//m_bUpdateToolbar = true;
		}
	}
}

int CALLBACK CDesignerViewTree::CompareTreeItems(LPARAM a_lParam1, LPARAM a_lParam2, LPARAM a_lParamSort)
{
	IComparable* p1 = reinterpret_cast<IComparable*>(a_lParam1);
	IComparable* p2 = reinterpret_cast<IComparable*>(a_lParam2);
	CItems const& cItems = *reinterpret_cast<CItems const*>(a_lParamSort);
	CItems::const_iterator i;
	for (i = cItems.begin(); i != cItems.end(); i++)
	{
		if (i->second == 0)
			continue;
		HRESULT hRes1 = p1->Compare(i->first);
		HRESULT hRes2 = p2->Compare(i->first);
		if (hRes1 == S_OK)
		{
			return hRes2 == S_OK ? 0 : -1;
		}
		else
		{
			if (hRes2 == S_OK)
				return 1;
		}
	}
	ATLASSERT(0); // error: items were not in the array!
	return 0; 
}

template<typename TIterator>
void ReleaseComparables(TIterator a_1, TIterator a_2)
{
	TIterator i;
	for (i = a_1; i != a_2; i++)
	{
		i->first->Release();
	}
}

void CDesignerViewTree::OwnerNotify(TCookie a_tCookie, TStructuredChanges a_tChanges)
{
	m_bSelectionSending = true;
	ObjectLock cLock(this);
	ULONG i;
	for (i = 0; i != a_tChanges.nChanges; i++)
	{
		vector<HTREEITEM> cEquivalentItems;
		if (a_tChanges.aChanges[i].pItem == NULL)
		{
			cEquivalentItems.push_back(TVI_ROOT);
		}
		else
		{
			pair<CBackMap::const_iterator, CBackMap::const_iterator> tEquivalentRange = m_cBackMap.equal_range(a_tChanges.aChanges[i].pItem);
			for (CBackMap::const_iterator iEI = tEquivalentRange.first; iEI != tEquivalentRange.second; iEI++)
			{
				cEquivalentItems.push_back(iEI->second);
			}
		}

		if (a_tChanges.aChanges[i].nChangeFlags & ESCChildren)
		{
			// synchronize subitems, try to keep item state
			// !! operational complexity n*n*log n
			for (vector<HTREEITEM>::const_iterator j = cEquivalentItems.begin(); j != cEquivalentItems.end(); j++)
			{
				HTREEITEM const hRootItem = *j;
				CItems cNewItems;
				{
					CComPtr<IEnumUnknowns> pNewItems;
					m_pRoot->ItemsEnum(a_tChanges.aChanges[i].pItem, &pNewItems);
					if (pNewItems != NULL)
					{
						CComPtr<IComparable> pItem;
						for (ULONG i = 0; SUCCEEDED(pNewItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem))); i++)
						{
							cNewItems.push_back(make_pair(pItem.Detach(), HTREEITEM(0)));
						}
					}
				}
				CItems cOldItems;
				HTREEITEM hChild = m_wndTree.GetChildItem(hRootItem);
				while (hChild)
				{
					IComparable* p = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(hChild));
					cOldItems.push_back(make_pair(p, hChild));
					hChild = m_wndTree.GetNextSiblingItem(hChild);
				}
//CheckTree();
				// step 1: remove all items that are in the old but not in the new vector
				CItems::iterator iOld;
				size_t iLastNew = 0;
				bool bSorted = true;
				for (iOld = cOldItems.begin(); iOld != cOldItems.end(); iOld++)
				{
					CItems::iterator iNew;
					for (iNew = cNewItems.begin(); iNew != cNewItems.end(); iNew++)
					{
						if (iNew->second == 0 && iOld->first->Compare(iNew->first) == S_OK)
						{
							break;
						}
					}
					if (iNew == cNewItems.end())
					{
						DeleteItem(iOld->second);
					}
					else
					{
						if (bSorted)
						{
							if (iNew < (cNewItems.begin()+iLastNew))
							{
								bSorted = false;
							}
							else
							{
								iLastNew = iNew-cNewItems.begin();
							}
						}
						iNew->second = iOld->second;
					}
				}
				if (hRootItem != TVI_ROOT && cOldItems.empty() && !cNewItems.empty())
				{
					RECT rc = {0, 0, 0, 0};
					m_wndTree.GetItemRect(hRootItem, &rc, FALSE);
					m_wndTree.InvalidateRect(&rc, FALSE);
				}
				cOldItems.clear();
//CheckTree();

				// step 2: TODO: sort remaining items
				if (!bSorted)
				{
					TVSORTCB tSortInfo;
					tSortInfo.hParent = hRootItem;
					tSortInfo.lParam = reinterpret_cast<LPARAM>(&cNewItems);
					tSortInfo.lpfnCompare = CompareTreeItems;
					m_wndTree.SortChildrenCB(&tSortInfo);

					// TODO: this is actually not correct - this check should be done at EVERY update
					// the backmap is invalid, because the sort rules likely changed
					CBackMap cBM;
					for (CBackMap::const_iterator i = m_cBackMap.begin(); i != m_cBackMap.end(); ++i)
						cBM.insert(make_pair(i->first, i->second));
					std::swap(cBM, m_cBackMap);
				}

				// step 3: add new items
				hChild = m_wndTree.GetChildItem(hRootItem);
				while (hChild)
				{
					cOldItems.push_back(make_pair(reinterpret_cast<IComparable*>(m_wndTree.GetItemData(hChild)), hChild));
					hChild = m_wndTree.GetNextSiblingItem(hChild);
				}
				CItems::const_iterator iAddNew = cNewItems.begin();
				CItems::const_iterator iAddOld = cOldItems.begin();
				HTREEITEM hInsertPos = TVI_FIRST;
				while (iAddOld != cOldItems.end())
				{
					ATLASSERT(iAddNew != cNewItems.end());
					if (iAddNew->first->Compare(iAddOld->first) == S_OK)
					{
						hInsertPos = iAddOld->second;
						iAddOld++;
						iAddNew++;
					}
					else
					{
						// insert new item
//CheckTree();
						hInsertPos = InsertItem(hRootItem, iAddNew->first, false, hInsertPos);
//CheckTree();
						InsertItems(iAddNew->first, hInsertPos, false);
//CheckTree();
						iAddNew++;
					}
				}
				while (iAddNew != cNewItems.end())
				{
//CheckTree();
					HTREEITEM hRoot = InsertItem(hRootItem, iAddNew->first, false);
//CheckTree();
					InsertItems(iAddNew->first, hRoot, false);
//CheckTree();
					iAddNew++;
				}
				ReleaseComparables(cNewItems.begin(), cNewItems.end());
//CheckTree();
			}
		}
		if (a_tChanges.aChanges[i].nChangeFlags & ESCGUIRepresentation)
		{
			// update item name and icon if it is in the tree
			for (vector<HTREEITEM>::const_iterator j = cEquivalentItems.begin(); j != cEquivalentItems.end(); j++)
			{
				if (m_pThumbData && m_pThumbData->nRefCount == 2)
				{
					// using custom draw
					TVITEMEX tItem;
					ZeroMemory(&tItem, sizeof tItem);
					bool bRefresh = false;
					bool bExpanded = false;
					GetItemDisplayProps(a_tChanges.aChanges[i].pItem, bRefresh, bExpanded, tItem);
					tItem.hItem = *j;
					m_wndTree.SetItem(&tItem);
					if (bRefresh)
					{
						EnterCriticalSection(&m_pThumbData->tLock);
						bool bAlreadyInQueue = false;
						for (std::list<IComparable*>::iterator q = m_pThumbData->cQueue.begin(); q != m_pThumbData->cQueue.end(); ++q)
							if (*q && (*q)->Compare(a_tChanges.aChanges[i].pItem) == S_OK)
							{
								bAlreadyInQueue = true;
								break;
							}
						if (!bAlreadyInQueue)
						{
							m_pThumbData->cQueue.push_back(a_tChanges.aChanges[i].pItem);
							a_tChanges.aChanges[i].pItem->AddRef();
							ReleaseSemaphore(m_pThumbData->hSemaphore, 1, NULL);
						}
						LeaveCriticalSection(&m_pThumbData->tLock);
					}
				}
				else
				{
					int nImage = I_IMAGENONE;
					std::tstring strText;
					bool bExpanded = false;
					GetItemDisplayProps(a_tChanges.aChanges[i].pItem, strText, nImage, bExpanded);
					m_wndTree.SetItem(*j, TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE, strText.c_str(), nImage, nImage, 0, 0, 0);
				}
			}
		}
		//if (a_tChanges.aChanges[i].nChangeFlags & ESCContent)
		//{
		//}
	}
	m_bSelectionSending = false;
	//m_bUpdateToolbar = true;
}

unsigned __stdcall CDesignerViewTree::ThumbProc(void* a_pRawData)
{
	SThumbData* pData = reinterpret_cast<SThumbData*>(a_pRawData);
	try
	{
		while (true)
		{
			if (WaitForSingleObject(pData->hSemaphore, INFINITE) == WAIT_OBJECT_0)
			{
				CComPtr<IComparable> pItem;
				EnterCriticalSection(&pData->tLock);
				if (pData->nRefCount < 2 || pData->cQueue.empty())
				{
					LeaveCriticalSection(&pData->tLock);
					break;
				}
				pItem.Attach(pData->cQueue.front());
				pData->cQueue.pop_front();
				CComPtr<IStructuredItemsRichGUI> pRichGUI = pData->pThis->m_pRichGUI;
				ULONG nX = pData->pThis->m_nThumbSizeX;
				ULONG nY = pData->pThis->m_nThumbSizeY;
				HWND hThis = pData->pThis->m_hWnd;
				//CComPtr<IDocument> pMainDoc(pData->pThis->m_pOriginal);
				CComPtr<IStructuredRoot> pRoot(pData->pThis->m_pRoot);
				CComPtr<IThumbnailRendererDoc> pThumbnails(pData->pThis->m_pThumbnails);
				LeaveCriticalSection(&pData->tLock);
				CAutoVectorPtr<DWORD> cData(new DWORD[nX*nY]);
				RECT rcBounds;
				ULONG nTimeStamp;
				HRESULT hRes = E_FAIL;
				if (pRichGUI)
				{
					hRes = pRichGUI->Thumbnail(pItem, nX, nY, cData, &rcBounds, &nTimeStamp);
				}
				if (FAILED(hRes) && pThumbnails && pRoot)
				{
					//CReadLock<IDocument> cLock(pMainDoc);
					CComPtr<ISubDocumentID> pSDID;
					pRoot->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
					if (pSDID)
					{
						CComPtr<IDocument> pDoc;
						pSDID->SubDocumentGet(&pDoc);
						nTimeStamp = 0;
						if (pDoc)
							hRes = pThumbnails->GetThumbnail(pDoc, nX, nY, cData, &rcBounds, 0, NULL);
					}
				}

				if (SUCCEEDED(hRes))
				{
					ULONG nMaskLineSize = (((nX+7)>>3)+3)&~3;
					CAutoVectorPtr<BYTE> pIconRes(new BYTE[sizeof(BITMAPINFOHEADER)+nX*nY*4+nMaskLineSize*nY]);
					BITMAPINFOHEADER* pBIH = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
					pBIH->biSize = sizeof*pBIH;
					pBIH->biWidth = nX;
					pBIH->biHeight = nY<<1;
					pBIH->biPlanes = 1;
					pBIH->biBitCount = 32;
					pBIH->biCompression = BI_RGB;
					pBIH->biSizeImage = nX*nY*4+nMaskLineSize*nY;
					pBIH->biXPelsPerMeter = 0x8000;
					pBIH->biYPelsPerMeter = 0x8000;
					pBIH->biClrUsed = 0;
					pBIH->biClrImportant = 0;
					DWORD* pXOR = reinterpret_cast<DWORD*>(pBIH+1);
					for (ULONG y = 0; y < nY; ++y)
						CopyMemory(pXOR+nX*(nY-y-1), cData.m_p+nX*y, nX*4);
					BYTE* pAND = reinterpret_cast<BYTE*>(pXOR+nX*nY);
					// create mask
					for (ULONG y = 0; y < nY; ++y)
					{
						BYTE* pA = pAND+nMaskLineSize*y;
						DWORD* pC = pXOR+nX*y;
						for (ULONG x = 0; x < nX; ++x, ++pC)
						{
							BYTE* p = pA+(x>>3);
							if (*pC&0xff000000)
								*p &= ~(0x80 >> (x&7));
							else
								*p |= 0x80 >> (x&7);
						}
					}

					std::pair<HICON, ULONG> tInfo;
					tInfo.first = CreateIconFromResourceEx(pIconRes, sizeof(BITMAPINFOHEADER)+nX*nY*4+nMaskLineSize*nY, TRUE, 0x00030000, nX, nY, LR_DEFAULTCOLOR);
					tInfo.second = nTimeStamp;
					if (pData->nRefCount == 2)
						::SendMessage(hThis, WM_THUMBNAILGENERATED, reinterpret_cast<WPARAM>(&tInfo), reinterpret_cast<LPARAM>(pItem.p));
					DestroyIcon(tInfo.first);
				}
			}
			else
			{
				break;
			}
		}
	}
	catch (...)
	{
	}
	LONG nCount = InterlockedDecrement(&pData->nRefCount);
	if (nCount == 0)
		delete pData;
	return 0;
}

LRESULT CDesignerViewTree::OnThumbnailGenerated(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	std::pair<HICON, ULONG>* pInfo = reinterpret_cast<std::pair<HICON, ULONG>*>(a_wParam);
	IComparable* pItem = reinterpret_cast<IComparable*>(a_lParam);
	ObjectLock cLock(this);
	auto tRange = m_cBackMap.equal_range(pItem);
	if (tRange.first == tRange.second)
		return 1; // invalid item, probably deleted from tree while thumbnail was generated
	CThumbnailMap::iterator iTM = m_cThumbnailMap.find(pItem);
	int index;
	if (iTM != m_cThumbnailMap.end())
	{
		iTM->second.nTimestamp = pInfo->second;
		index = iTM->second.iImageIndex;
		m_cThumbnailList.ReplaceIcon(index, pInfo->first);
	}
	else
	{
		if (m_cFreeThumbnailIndices.empty())
		{
			index = m_cThumbnailList.AddIcon(pInfo->first);
		}
		else
		{
			index = m_cFreeThumbnailIndices[m_cFreeThumbnailIndices.size()-1];
			m_cFreeThumbnailIndices.resize(m_cFreeThumbnailIndices.size()-1);
			m_cThumbnailList.ReplaceIcon(index, pInfo->first);
		}
		SThumbnailInList t = { pInfo->second, index };
		pItem->AddRef();
		m_cThumbnailMap[pItem] = t;
	}
	for (CBackMap::const_iterator i = tRange.first; i != tRange.second; ++i)
	{
		InvalidateItem(i->second);
		//m_wndTree.SetItemImage(i->second, index, index);
	}
	return 0;
}

LRESULT CDesignerViewTree::OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	CREATESTRUCT const* pCreateStruct = reinterpret_cast<CREATESTRUCT const*>(a_lParam);

	CConfigValue cViewMode;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_TREE_VIEWMODE), &cViewMode);
	m_nIconSize = m_nThumbSizeX = m_nThumbSizeY = XPGUI::GetSmallIconSize();
	m_nGapSize = m_nIconSize>>3;
	m_nCheckSize = 0;
	HTHEME hTheme = GetTheme();
	if (hTheme)
	{
		SIZE sz = {0, 0};
		GetThemePartSize(hTheme, NULL, BP_CHECKBOX, CBS_CHECKEDNORMAL, NULL, TS_TRUE, &sz);
		m_nCheckSize = sz.cy + 2;
	}
	if (m_nCheckSize < 10 || m_nCheckSize > m_nIconSize+2)
		m_nCheckSize = (m_nIconSize*8+5)/10 + 2; // extra 1 pixel margin (subtracted during drawing)
	m_nButtonSize = m_nIconSize+(2*m_nIconSize+8)/16;
	m_nIndentSize = m_nIconSize>>1;

	switch (cViewMode.operator LONG())
	{
	case CFGVAL_TVM_MEDIUMICONS:
		m_nIconSize *= 2;
		break;
	case CFGVAL_TVM_LARGEICONS:
		m_nIconSize *= 3;
		break;
	case CFGVAL_TVM_THUMBNAILS:
		RWCoCreateInstance(m_pThumbnails, __uuidof(ThumbnailRenderer));
		if (m_pThumbnails)
		{
			m_pThumbData = new SThumbData(this);
			m_hThumbThread = (HANDLE)_beginthreadex(NULL, 0, ThumbProc, m_pThumbData, 0, &m_uThumbThreadID);
			if (m_hThumbThread)
			{
				m_nThumbSizeX *= 4;
				m_nThumbSizeY *= 3;
				m_cThumbnailList.Create(m_nThumbSizeX, m_nThumbSizeY, XPGUI::GetImageListColorFlags(), 16, 16);
			}
			else
			{
				delete m_pThumbData;
				m_pThumbData = NULL;
				m_pThumbnails = NULL;
			}
		}
		break;
	}
	m_cImageList.Create(m_nIconSize, m_nIconSize, XPGUI::GetImageListColorFlags(), 16, 16);
	m_cImageMenu.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 16, 16);

	CConfigValue cLines;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_TREE_VIEWLINES), &cLines);
	m_bHasLines = cLines;
	DWORD dwExStyle = m_pThumbData ? TVS_EX_DOUBLEBUFFER : TVS_EX_AUTOHSCROLL|TVS_EX_DOUBLEBUFFER;
	DWORD dwStyle = m_pThumbData ? TVS_NOHSCROLL : 0;
	m_hWndClient = m_wndTree.Create(m_hWnd, NULL, _T("MultiselectionTree"), WS_CHILDWINDOW|TVS_EDITLABELS|(m_bHasLines ? TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT : TVS_FULLROWSELECT)|TVS_INFOTIP|TVS_SHOWSELALWAYS|TVS_TRACKSELECT|dwStyle|WS_VISIBLE, (m_nStyle&EDVWSBorderMask) != EDVWSNoBorder ? WS_EX_CLIENTEDGE : 0, IDC_TREE_TREE);
	if (XPGUI::IsVista())
	{
		m_wndTree.SendMessage(TVM_SETEXTENDEDSTYLE, dwExStyle, dwExStyle);
		//if (m_pThumbData == NULL)
			::SetWindowTheme(m_wndTree, L"explorer", NULL);
	}
	//m_wndTree.SetBkColor(GetSysColor(m_pThumbData ? COLOR_3DFACE : COLOR_WINDOW));
	m_wndTree.SetImageList(m_cImageList, TVSIL_NORMAL);

	if (m_pDocument != NULL)
	{
		CConfigValue cRootIID;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_TREE_STRUCTUREROOT), &cRootIID);
		RootIDSet(cRootIID);
		if (m_pRoot == 0)
		{
			GUID tRootIID = GUID_NULL;
			m_pRoots->Get(0, &tRootIID);
			RootIDSet(tRootIID);
		}
	}
	if (m_pRoot == 0)
	{
		RootIDSet(__uuidof(IStructuredRoot));
	}

	return 0;
}

LRESULT CDesignerViewTree::OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_hTheme)
		CloseThemeData(m_hTheme);
	StopThumbThread();
	m_bSelectionSending = true;
	{
		ObjectLock cLock(this);
		DeleteItems(TVI_ROOT);
	}
	m_bSelectionSending = false;
	return 0;
}

LRESULT CDesignerViewTree::OnSelectionChanged(int UNREF(a_nCtrlID), LPNMHDR a_pNMHeader, BOOL& UNREF(a_bHandled))
{
	if (m_bSelectionSending)
		return 0;

	NM_TREEVIEW* pNMTreeView = reinterpret_cast<NM_TREEVIEW*>(a_pNMHeader);

	CTreeItemList cList;
	GetSelectedList(cList);
	CAutoVectorPtr<IComparable*> apItems(cList.GetSize() > 0 ? new IComparable*[cList.GetSize()] : NULL);
	HTREEITEM hFocus = m_wndTree.GetSelectedItem();
	int i;
	for (i = 0; i < cList.GetSize(); i++)
	{
		apItems[i] = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(cList[i]));
		if (cList[i] == hFocus && i != 0)
			std::rotate(apItems.m_p, apItems.m_p+i-1, apItems.m_p+i);// move focused item to the front
	}

	CComPtr<ISharedState> pState;
	m_pRoot->StatePack(i, apItems, &pState);
	if (pState != NULL)
	{
		m_bSelectionSending = true;
		m_pLastSel = pState;
		m_pStateMgr->StateSet(m_bstrSyncGroup, pState);
		m_bSelectionSending = false;
	}

	//m_bUpdateToolbar = true;

	return 0;
}

LRESULT CDesignerViewTree::OnBeginLabelEdit(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	LPNMTVDISPINFO pNMTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(a_pNMHeader);
	std::tstring str;
	bool bEditable = GetItemDataAsString(reinterpret_cast<IComparable*>(pNMTVDispInfo->item.lParam), true, str);
	if (bEditable)
	{
		CEdit wndEdit = m_wndTree.GetEditControl();
		if (wndEdit.IsWindow())
		{
			wndEdit.SetWindowText(str.c_str());
			PostMessage(WM_RW_REPOSITIONEDIT, 0, reinterpret_cast<LPARAM>(pNMTVDispInfo->item.hItem));
		}
	}
	return bEditable ? FALSE : TRUE;
}

LRESULT CDesignerViewTree::OnEditUpdated(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	LRESULT lRes = CallWindowProc(m_wndTree.m_pfnSuperWindowProc, m_wndTree, WM_COMMAND, MAKEWPARAM(a_wID, a_wNotifyCode), (LPARAM)a_hWndCtl);
	CEdit wndEdit = m_wndTree.GetEditControl();
	if (a_hWndCtl != wndEdit.m_hWnd)
		return lRes;
	HTREEITEM hItem = m_wndTree.GetSelectedItem();
	if (hItem == NULL)
		return lRes;
	SItemLayout tIL;
	ComputeItemLayout(hItem, tIL);
	DWORD dwMargins = wndEdit.GetMargins();
	wndEdit.SetWindowPos(NULL, tIL.rcValue.left-LOWORD(dwMargins)-GetSystemMetrics(SM_CXEDGE), tIL.rcValue.top-GetSystemMetrics(SM_CYEDGE)+1, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	return lRes;

	//CEdit wndEdit = m_wndTree.GetEditControl();
	//if (a_hWndCtl != wndEdit.m_hWnd)
	//{
	//	a_bHandled = FALSE;
	//	return 0;
	//}
	//HDC hDC = wndEdit.GetDC();
	//wchar_t szBuffer[512];
	//wndEdit.GetWindowText(szBuffer, 512);
	//szBuffer[511] = L'\0';

	//RECT rcClient = {0, 0, 0, 0};
	//GetClientRect(&rcClient);
	//RECT rcEdit = {0, 0, 0, 0};
	//wndEdit.GetWindowRect(&rcEdit);
	//ScreenToClient(&rcEdit);

	//HFONT hFont = wndEdit.GetFont();
	//HGDIOBJ hOldFont = SelectObject(hDC, hFont);

	//SIZE sz = {0, 0};
	//if (GetTextExtentPoint32(hDC, szBuffer, wcslen(szBuffer), &sz))
	//{
	//	DWORD dwMargins = wndEdit.GetMargins();
	//	// Add extra spacing for the next character
	//	TEXTMETRICW textMetric;
	//	GetTextMetrics(hDC, &textMetric);
	//	sz.cx += LOWORD(dwMargins)+HIWORD(dwMargins)+textMetric.tmAveCharWidth*2;//sz.cy;//(textMetric.tmMaxCharWidth * 2);

	//	sz.cx = max(sz.cx, textMetric.tmMaxCharWidth * 3);
	//	sz.cx = min(sz.cx, rcClient.right - rcEdit.left + 2);
 //
	//	wndEdit.SetWindowPos(HWND_TOP, 0, 0, sz.cx, rcEdit.bottom - rcEdit.top, SWP_NOMOVE | SWP_DRAWFRAME);
	//}

	//SelectObject(hDC, hOldFont);
	//wndEdit.ReleaseDC(hDC);

	//return 0;
}

LRESULT CDesignerViewTree::OnRepositionEdit(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	CEdit wndEdit = m_wndTree.GetEditControl();
	if (wndEdit.IsWindow())
	{
		HTREEITEM hItem = reinterpret_cast<HTREEITEM>(a_lParam);

		SItemLayout tIL;
		ComputeItemLayout(hItem, tIL);
		DWORD dwMargins = wndEdit.GetMargins();
		wndEdit.SetWindowPos(NULL, tIL.rcValue.left-LOWORD(dwMargins)-GetSystemMetrics(SM_CXEDGE), tIL.rcValue.top-GetSystemMetrics(SM_CYEDGE)+1, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

		//RECT rcItem = {0, 0, 0, 0};
		//m_wndTree.GetItemRect(hItem, &rcItem, FALSE);
		//RECT rcInner = {0, 0, 0, 0};
		//wndEdit.GetRect(&rcInner);
		//RECT rcMyEdit = {0, 0, 0, 0};
		////GetValueRect(rcItem, &rcMyEdit);
		//TVITEMEX item = { 0 };
		//item.hItem = hItem;
		//item.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_INTEGRAL;
		//m_wndTree.GetItem(&item);
		//LONG iIndent = 0;
		//for (HTREEITEM h = m_wndTree.GetParentItem(hItem); h; h = m_wndTree.GetParentItem(h))
		//	++iIndent;
		//if (item.iIntegral == 1)
		//{
		//	ULONG nCheckSize = item.iImage&2 ? m_nButtonSize : 0;
		//	ULONG nIcSize = item.iImage&1 ? m_nButtonSize : 0;
		//	rcMyEdit.left = m_nIndentSize*iIndent+m_nButtonSize+nIcSize+nCheckSize+5;
		//	rcMyEdit.right = rcMyEdit.left+(item.iSelectedImage&0xffff);
		//	rcMyEdit.top = 0;
		//	rcMyEdit.bottom = rcItem.bottom-rcItem.top;
		//}
		//else // item.iIntegral == 3
		//{
		//	rcMyEdit.left = rcItem.left+m_nIndentSize*iIndent+m_nButtonSize+m_nThumbSizeX+5;
		//	rcMyEdit.right = rcMyEdit.left+(item.iSelectedImage&0xffff);
		//	rcMyEdit.top = 0;
		//	rcMyEdit.bottom = (rcItem.bottom-rcItem.top)>>1;
		//}
		//rcMyEdit.top += rcItem.top+(((rcMyEdit.bottom-rcMyEdit.top)-(rcInner.bottom-rcInner.top))>>1);
		//rcMyEdit.bottom = rcMyEdit.top+(rcInner.bottom-rcInner.top);
		//wndEdit.ClientToScreen(&rcInner);
		//RECT rcWindow = {0, 0, 0, 0};
		//wndEdit.GetWindowRect(&rcWindow);
		//wndEdit.SetWindowPos(NULL, rcWindow.left+rcMyEdit.left-rcInner.left, rcWindow.top+rcMyEdit.top-rcInner.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	}
	return 0;
}

LRESULT CDesignerViewTree::OnEndLabelEdit(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	if (m_bSelectionSending)
		return FALSE;
	LPNMTVDISPINFO pNMTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(a_pNMHeader);
	if (pNMTVDispInfo->item.pszText)
		SetItemDataFromString(pNMTVDispInfo->item.pszText, reinterpret_cast<IComparable*>(pNMTVDispInfo->item.lParam));
	m_wndTree.SetItemState(pNMTVDispInfo->item.hItem, TVIS_SELECTED, TVIS_SELECTED);
	return FALSE;
}

LRESULT CDesignerViewTree::OnKillFocus(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	if (!m_bEnsureVisibleOnKillFocus)
		return 0;
	CWindow cWnd = GetFocus();
	if (cWnd == NULL || cWnd == m_hWnd || cWnd == m_wndTree) return 0;
	cWnd = cWnd.GetParent();
	if (cWnd == m_hWnd || cWnd == m_wndTree)
		return 0;
	//m_pConfig->ItemValueGet();
	m_wndTree.EnsureVisible(GetFirstSelectedItem());
	return 0;
}

void CDesignerViewTree::DrawItemLines(HDC hDC, HTREEITEM hItem, int iLevel, int nTop, int nBot, int nStep, int nWidth, bool bHasButtons, bool bHasLines, bool bRootLines, bool bButtonHot)
{
	if (iLevel == 0 && !bRootLines)
		return;
	if (!bHasButtons && !bHasLines)
		return;

	int centerx = nStep * iLevel + nWidth / 2;
	int centery = (nTop + nBot) / 2;

	if (bHasButtons)
		bHasButtons = GetNextItem(hItem, TVGN_CHILD);
	bool expanded = bHasButtons && m_wndTree.GetItemState(hItem, TVIS_EXPANDED);
	UINT iGlyphPart = TVP_GLYPH;
	RECT glyphRect = {INT_MAX, INT_MAX, INT_MIN, INT_MIN};

	HTHEME hTheme = bHasButtons ? GetTreeTheme() : NULL;

	if (hTheme)
	{
		if (bButtonHot && IsThemePartDefined(hTheme, TVP_HOTGLYPH, 0))
			iGlyphPart = TVP_HOTGLYPH;
		SIZE sz = {0, 0};
		GetThemePartSize(hTheme, hDC, iGlyphPart, expanded ? GLPS_OPENED : GLPS_CLOSED, NULL, TS_DRAW, &sz);
		glyphRect.left = centerx - (sz.cx>>1);
		glyphRect.top = centery - (sz.cy>>1);
		glyphRect.right = centerx - (sz.cx>>1) + sz.cx;
		glyphRect.bottom = centery - (sz.cy>>1) + sz.cy;
	}

	if (bHasLines)
	{
		// get a dotted grey pen
		LOGBRUSH lb;
		lb.lbStyle = BS_SOLID;
		lb.lbColor = GetSysColor(COLOR_GRAYTEXT);
		HPEN hNewPen = ExtCreatePen(PS_COSMETIC|PS_ALTERNATE, 1, &lb, 0, NULL);
		HGDIOBJ hOldPen = SelectObject(hDC, hNewPen);

		// make sure the center is on a dot (using +2 instead
		// of +1 gives us pixel-by-pixel compat with native)
		centery = (centery + 2) & ~1;

		LONG right = nStep * iLevel + nWidth;
		if (glyphRect.right <= right)
		{
			MoveToEx(hDC, right, centery, NULL);
			LineTo(hDC, max(centerx - 1, glyphRect.right), centery);
		}

		HTREEITEM prev = GetNextItem(hItem, TVGN_PREVIOUS);
		HTREEITEM next = GetNextItem(hItem, TVGN_NEXT);
		HTREEITEM parent = GetNextItem(hItem, TVGN_PARENT);
		HTREEITEM root = GetNextItem(hItem, TVGN_ROOT);

		if (prev || parent || hItem == root && glyphRect.top >= nTop)
		{
			MoveToEx(hDC, centerx, nTop, NULL);
			LineTo(hDC, centerx, min(glyphRect.top, centery));
		}

		if (next && glyphRect.bottom <= nBot+1)
		{
			MoveToEx(hDC, centerx, max(glyphRect.bottom, centery), NULL);
			LineTo(hDC, centerx, nBot + 1);
		}

		// draw the line from our parent to its next sibling
		HTREEITEM item = parent;
		int pcenterx = centerx - nStep;
		while (item)
		{
			next = GetNextItem(item, TVGN_NEXT);
			item = GetNextItem(item, TVGN_PARENT);
			if (next && (bRootLines || item != NULL)) // skip top-levels unless TVS_LINESATROOT
			{
				MoveToEx(hDC, pcenterx, nTop, NULL);
				LineTo(hDC, pcenterx, nBot + 1);
			}
			pcenterx -= nStep;
		}

		SelectObject(hDC, hOldPen);
		DeleteObject(hNewPen);
	}

	// display the (+/-) signs
	if (bHasButtons)
	{
		if (hTheme)
		{
			DrawThemeBackground(hTheme, hDC, iGlyphPart, expanded ? GLPS_OPENED : GLPS_CLOSED, &glyphRect, NULL);
		}
		else
		{
			LONG height = nBot - nTop;
			LONG width  = nWidth;
			LONG rectsize = min(height, width) / 4;
			LONG plussize = (rectsize + 1) * 3 / 4;

			HPEN new_pen  = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_GRAYTEXT));
			HGDIOBJ old_pen  = SelectObject(hDC, new_pen);

			Rectangle(hDC, centerx - rectsize - 1, centery - rectsize - 1,
					  centerx + rectsize + 2, centery + rectsize + 2);

			SelectObject(hDC, old_pen);
			DeleteObject(new_pen);

			// draw +/- signs with current text color
			new_pen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWTEXT));
			old_pen = SelectObject(hDC, new_pen);

			if (height < 18 || width < 18)
			{
				MoveToEx(hDC, centerx - plussize + 1, centery, NULL);
				LineTo(hDC, centerx + plussize, centery);

				if (!expanded)// || (item->state & TVIS_EXPANDPARTIAL))
				{
					MoveToEx(hDC, centerx, centery - plussize + 1, NULL);
					LineTo(hDC, centerx, centery + plussize);
				}
			}
			else
			{
				if (!expanded)// || (item->state & TVIS_EXPANDPARTIAL))
				{
					MoveToEx(hDC, centerx - 1, centery - plussize + 1, NULL);
					LineTo(hDC, centerx + 1, centery - plussize + 1);
					LineTo(hDC, centerx + 1, centery - 1);
					LineTo(hDC, centerx + plussize - 1, centery - 1);
					LineTo(hDC, centerx + plussize - 1, centery + 1);
					LineTo(hDC, centerx + 1, centery + 1);
					LineTo(hDC, centerx + 1, centery + plussize - 1);
					LineTo(hDC, centerx - 1, centery + plussize - 1);
					LineTo(hDC, centerx - 1, centery + 1);
					LineTo(hDC, centerx - plussize + 1, centery + 1);
					LineTo(hDC, centerx - plussize + 1, centery - 1);
					LineTo(hDC, centerx - 1, centery - 1);
					LineTo(hDC, centerx - 1, centery - plussize + 1);
					//Rectangle(hDC, centerx - 1, centery - plussize + 1,
					//		  centerx + 2, centery + plussize);
					//SetPixel(hdc, centerx - 1, centery, clrBk);
					//SetPixel(hdc, centerx + 1, centery, clrBk);
				}
				else
				{
					Rectangle(hDC, centerx - plussize + 1, centery - 1,
							  centerx + plussize, centery + 2);

				}
			}

			SelectObject(hDC, old_pen);
			DeleteObject(new_pen);
		}
	}
}

void CDesignerViewTree::DrawCheck(HTHEME hTheme, HDC hDC, int left, int top, int size, bool checked, bool pressed, bool hot)
{
	if (hTheme)
	{
		RECT rc = {left, top, left+size, top+size};
		DrawThemeBackground(hTheme, hDC, BP_CHECKBOX, checked ? (pressed ? CBS_CHECKEDPRESSED : (hot ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL)) : (pressed ? CBS_UNCHECKEDPRESSED : (hot ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL)), &rc, NULL);
	}
	else
	{
		HPEN hPen = CreatePen(PS_SOLID, 1, GetTextColor(hDC));
		HGDIOBJ hOldPen = SelectObject(hDC, hPen);
		HBRUSH hBrush = CreateSolidBrush(GetBkColor(hDC));
		HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);
		Rectangle(hDC, left, top, left+size, top+size);
		if (checked)
		{
			//HPEN hPen2 = CreatePen(PS_SOLID, 2, GetTextColor(hDC));
			//SelectObject(hDC, hPen2);
			//DeleteObject(hPen);
			//hPen = hPen2;
			int midx = size*0.4f+0.5f;
			int midy = size*0.7f+0.5f;
			MoveToEx(hDC, left+2, top+midy-midx+2, NULL);
			LineTo(hDC, left+midx, top+midy);
			LineTo(hDC, left+size-2, top+midy+midx-size+2);
			MoveToEx(hDC, left+3, top+midy-midx+2, NULL);
			LineTo(hDC, left+midx, top+midy-1);
			LineTo(hDC, left+size-3, top+midy+midx-size+2);
		}
		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}
}

LRESULT CDesignerViewTree::OnThemeChanged(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_hTheme)
	{
		CloseThemeData(m_hTheme);
		m_hTheme = OpenThemeData(m_hWnd, VSCLASS_BUTTON);
	}
	a_bHandled = FALSE;
	return 0;
}

HTHEME CDesignerViewTree::GetTheme()
{
	if (!m_bTheme)
	{
		m_bTheme = true;
		m_hTheme = CTheme::IsThemingSupported() ? OpenThemeData(m_hWnd, VSCLASS_BUTTON) : NULL;
	}
	return m_hTheme;
}


LRESULT CDesignerViewTree::OnCustomDraw(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	LPNMTVCUSTOMDRAW pCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(a_pNMHeader);
	switch (pCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return m_pThumbData && m_pThumbData->nRefCount == 2 ? CDRF_NOTIFYITEMDRAW : CDRF_DODEFAULT;
	case CDDS_ITEMPREPAINT:
		if (pCD->nmcd.rc.left < pCD->nmcd.rc.right && pCD->nmcd.rc.top < pCD->nmcd.rc.bottom)
		{
			HTHEME hTreeTheme = GetTreeTheme();
			int stateid = TREIS_NORMAL;
			bool bMenuOnThisItem = m_hContextMenuItem == reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec);
			bool bMenuOnDifferentItem = m_hContextMenuItem && !bMenuOnThisItem;
			bool bCheckHot = m_hHotItem == reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec) && (m_nHotPart&TVHT_ONITEMSTATEICON);
			bool bItemHot = m_hHotItem == reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec) || bMenuOnThisItem;
			bool bButtonHot = m_hHotItem == reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec) && (m_nHotPart&TVHT_ONITEMBUTTON);
			if (bMenuOnThisItem)
				stateid = TREIS_HOTSELECTED;
			else
				switch ((pCD->nmcd.uItemState&(CDIS_FOCUS|CDIS_SELECTED|CDIS_MARKED))|(bItemHot ? CDIS_HOT : 0))
				{
				case CDIS_FOCUS|CDIS_HOT|CDIS_SELECTED|CDIS_MARKED:
				case CDIS_FOCUS|CDIS_HOT|CDIS_SELECTED:
				case CDIS_HOT|CDIS_SELECTED|CDIS_MARKED:
				case CDIS_HOT|CDIS_SELECTED:
				case CDIS_HOT|CDIS_MARKED:
				case CDIS_SELECTED|CDIS_MARKED:
				case CDIS_MARKED:
					stateid = TREIS_HOTSELECTED; break;
				case CDIS_FOCUS|CDIS_HOT:
				case CDIS_HOT:
					stateid = TREIS_HOT; break;
				case CDIS_FOCUS|CDIS_SELECTED:
					stateid = bMenuOnDifferentItem ? TREIS_SELECTEDNOTFOCUS : TREIS_SELECTED; break;
				case CDIS_SELECTED:
					stateid = TREIS_SELECTEDNOTFOCUS; break;
				//case CDIS_FOCUS:
				//case 0:
				}
			if (hTreeTheme)
			{
				if (stateid != TREIS_NORMAL)
				{
					DrawThemeBackground(hTreeTheme, pCD->nmcd.hdc, TVP_TREEITEM, stateid, &pCD->nmcd.rc, NULL);
				}
			}

			SItemLayout tSL;
			ComputeItemLayout(reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec), tSL);
			//int nIconSize = m_nIconSize;
			//int nCheckSize = (m_nIconSize*8+5)/10;
			//int nLineButton = nIconSize+3;
			//int nLineStep = nIconSize>>1;

			DrawItemLines(pCD->nmcd.hdc, reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec), pCD->iLevel, pCD->nmcd.rc.top, pCD->nmcd.rc.bottom, m_nIndentSize, m_nButtonSize, m_bHasLines, m_bHasLines, m_bHasLines, bButtonHot);
			IComparable* p = reinterpret_cast<IComparable*>(pCD->nmcd.lItemlParam);
			CComPtr<IUIItem> pUI;
			p->QueryInterface(&pUI);

			std::tstring strVal;
			bool bDataValid = GetItemDataAsString(pUI.p, false, strVal);

			//if (strVal.compare(a_strName) != 0 && bDataValid)
			//{
			//	if (!a_strName.empty())
			//	{
			//		a_strName += _T(": ");
			//	}
			//	a_strName += strVal;
			//}
			//if (a_strName.empty() && !bDataValid)
			//{
			//	TCHAR szItem[64] = _T("");
			//	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_NAMELESSITEM, szItem, itemsof(szItem), LANGIDFROMLCID(m_tLocaleID));
			//	a_strName = szItem;
			//}
			bool bIsBool = false;
			bool bChecked = false;
			CComQIPtr<IItemBool> pBool(p);
			if (pBool != NULL)
			{
				bIsBool = true;
				BYTE bVal = 0;
				if (SUCCEEDED(pBool->ValueGet(&bVal)) && bVal)
					bChecked = true;
			}

			CComBSTR bstrName;
			m_wndTree.GetItemText(reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec), bstrName.m_str);

			SIZE szData = {0, 0};
			if ((pCD->nmcd.uItemState&CDIS_FOCUS) == 0 || m_wndTree.GetEditControl() == NULL)
			{
				if (bDataValid)
					GetTextExtentPoint32(pCD->nmcd.hdc, strVal.c_str(), strVal.length(), &szData);
			}

			bool const bUseThumbnail = pUI->UseThumbnail() == S_OK;
			if (bUseThumbnail)
			{
				ObjectLock cLock(this);
				CThumbnailMap::const_iterator i = m_cThumbnailMap.find(p);
				if (i != m_cThumbnailMap.end())
				{
					if (hTreeTheme)
					{
						DrawThemeIcon(hTreeTheme, pCD->nmcd.hdc, TVP_TREEITEM, bUseThumbnail ? TREIS_DISABLED : stateid, &tSL.rcThumbnail, m_cThumbnailList, i->second.iImageIndex);
					}
					else
					{
						m_cThumbnailList.Draw(pCD->nmcd.hdc, i->second.iImageIndex, tSL.rcThumbnail.left, tSL.rcThumbnail.top, ILD_NORMAL);
					}
				}
			}
			if (bIsBool)
				DrawCheck(GetTheme(), pCD->nmcd.hdc, tSL.rcCheck.left+1, tSL.rcCheck.top+1, tSL.rcCheck.bottom-tSL.rcCheck.top-2, bChecked, m_eDragMode == EDMCheckBox && m_hDragItem == reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec), bCheckHot);

			{
				int iImg = GetItemImage<I_IMAGENONE>(pUI.p);
				if (iImg != I_IMAGENONE)
				{
					if (hTreeTheme)
					{
						DrawThemeIcon(hTreeTheme, pCD->nmcd.hdc, TVP_TREEITEM, bUseThumbnail ? TREIS_DISABLED : stateid, &tSL.rcIcon, m_cImageList, iImg);
					}
					else
					{
						m_cImageList.Draw(pCD->nmcd.hdc, iImg, tSL.rcIcon.left, tSL.rcIcon.top, bUseThumbnail ? ILD_TRANSPARENT : ILD_NORMAL);
					}
				}
			}
			UINT align = SetTextAlign(pCD->nmcd.hdc, TA_LEFT | TA_TOP);
			if (tSL.rcLabel.right > tSL.rcLabel.left)
			{
				if (hTreeTheme)
				{
					DrawThemeText(hTreeTheme, pCD->nmcd.hdc, TVP_TREEITEM, tSL.rcValue.right > tSL.rcValue.left ? TREIS_DISABLED : stateid, bstrName, bstrName.Length(), DT_LEFT|DT_VCENTER|DT_SINGLELINE, 0, &tSL.rcLabel);
				}
				else
				{
					ExtTextOutW(pCD->nmcd.hdc, tSL.rcLabel.left, tSL.rcLabel.top,
								ETO_CLIPPED | ETO_OPAQUE, &tSL.rcLabel,
								bstrName, bstrName.Length(), NULL);
				}
			}
			if (szData.cy > 0)
			{
				if (hTreeTheme)
				{
					DrawThemeText(hTreeTheme, pCD->nmcd.hdc, TVP_TREEITEM, stateid, strVal.c_str(), strVal.length(), DT_LEFT|DT_VCENTER|DT_SINGLELINE, 0, &tSL.rcValue);
				}
				else
				{
					ExtTextOutW(pCD->nmcd.hdc, tSL.rcValue.left, tSL.rcValue.top,
								ETO_CLIPPED | ETO_OPAQUE, &tSL.rcValue,
								strVal.c_str(), strVal.length(), NULL);
				}
			}
			SetTextAlign(pCD->nmcd.hdc, align);
		}
		return CDRF_SKIPDEFAULT;
	}
	return CDRF_DODEFAULT;
}

class ATL_NO_VTABLE CContextMenuOperation :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
	void Init(BSTR a_bstrID, IUnknown* a_pState, ISharedStateManager* a_pOrig)
	{
		m_bstrID = a_bstrID;
		m_pState = a_pState;
		m_pOrig = a_pOrig;
	}
	ILocalizedString* M_ErrorMessage()
	{
		return m_pMessage;
	}


BEGIN_COM_MAP(CContextMenuOperation)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// ISharedStateManager methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		if (m_pState != NULL && m_bstrID == a_bstrCategoryName)
			return m_pState->QueryInterface(a_iid, a_ppState);
		return m_pOrig->StateGet(a_bstrCategoryName, a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (m_pState != NULL && m_bstrID == a_bstrCategoryName)
			m_bstrID.Empty();
		return m_pOrig->StateSet(a_bstrCategoryName, a_pState);
	}
	STDMETHOD(IsCancelled)() { return S_FALSE; }
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		if (a_pItemIndex) *a_pItemIndex = 0;
		if (a_pItemsRemaining) *a_pItemsRemaining = 0;
		if (a_pStepIndex) *a_pStepIndex = 0;
		if (a_pStepsRemaining) *a_pStepsRemaining = 0;
		return S_OK;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		m_pMessage = a_pMessage;
		return S_OK;
	}

private:
	CComBSTR m_bstrID;
	CComPtr<IUnknown> m_pState;
	CComPtr<ISharedStateManager> m_pOrig;
	CComPtr<ILocalizedString> m_pMessage;
};

LRESULT CDesignerViewTree::OnContextMenu(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	try
	{
		CComPtr<ISharedState> pState;
		int xPos = GET_X_LPARAM(a_lParam);
		int yPos = GET_Y_LPARAM(a_lParam);
		HTREEITEM hTI = NULL;
		bool bNotSelected = false;
		if (xPos == -1 || yPos == -1)
		{
			hTI = m_wndTree.GetSelectedItem();
			if (hTI == NULL)
			{
				RECT rcWnd;
				GetWindowRect(&rcWnd);
				xPos = rcWnd.left;
				yPos = rcWnd.top;
			}
			else
			{
				RECT rcTmp = {0, 0, 0, 0};
				m_wndTree.GetItemRect(hTI, &rcTmp, TRUE);
				m_wndTree.ClientToScreen(&rcTmp);
				xPos = rcTmp.right;
				yPos = rcTmp.top;
				RECT rcWnd;
				GetWindowRect(&rcWnd);
				if (xPos > rcWnd.right) xPos = rcWnd.right;
				else if (xPos < rcWnd.left) xPos = rcWnd.left;
				if (yPos > rcWnd.bottom) yPos = rcWnd.bottom;
				else if (yPos < rcWnd.top) yPos = rcWnd.top;
			}
		}
		else
		{
			POINT tP = {xPos, yPos};
			m_wndTree.ScreenToClient(&tP);
			UINT nFlags = 0;
			hTI = m_wndTree.HitTest(tP, &nFlags);
			if (hTI == NULL)
			{
				m_pRoot->StatePack(0, NULL, &pState);
			}
			else //if (nFlags&TVHT_ONITEM) == 0)
			{
				if (TVIS_SELECTED != m_wndTree.GetItemState(hTI, TVIS_SELECTED))
				{
					bNotSelected = true;
					IComparable* p2 = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(hTI));
					m_pRoot->StatePack(1, &p2, &pState);
				}
			}
		}
		CComObject<CContextMenuOperation>* p = NULL;
		CComObject<CContextMenuOperation>::CreateInstance(&p);
		CComPtr<IOperationContext> pStateMgr = p;
		p->Init(m_bstrSyncGroup, pState, m_pStateMgr);

		CConfigValue cOpID;
		CComPtr<IConfig> pOpCfg;
		CComBSTR bstrCFGID_TREE_CONTEXTMENU(CFGID_TREE_CONTEXTMENU);
		m_pConfig->ItemValueGet(bstrCFGID_TREE_CONTEXTMENU, &cOpID);
		m_pConfig->SubConfigGet(bstrCFGID_TREE_CONTEXTMENU, &pOpCfg);
		CComPtr<IEnumUnknowns> pOps;
		m_pCmdMgr->CommandsEnum(m_pCmdMgr, cOpID, pOpCfg, pStateMgr, this, m_pOriginal, &pOps);

		ULONG nSize = 0;
		if (pOps == NULL || FAILED(pOps->Size(&nSize)) || nSize == 0)
			return 0;

		//if (hTI && bNotSelected)//!m_wndTree.GetItemState(hTI, TVIS_FOCUSED))
		{
			m_hContextMenuItem = hTI;
			RECT rcItem = {0, 0, 0, 0};
			m_wndTree.GetItemRect(m_hContextMenuItem, &rcItem, FALSE);
			m_wndTree.InvalidateRect(&rcItem);
		}
		m_pContextSel = pState;
		HRESULT hRes = ProcessContextMenu(pOps, xPos, yPos);
		m_pContextSel = NULL;
		if (m_hContextMenuItem != NULL)
		{
			RECT rcItem = {0, 0, 0, 0};
			m_wndTree.GetItemRect(m_hContextMenuItem, &rcItem, FALSE);
			m_hContextMenuItem = NULL;
			m_wndTree.InvalidateRect(&rcItem);
		}

		if (FAILED(hRes) && hRes != E_RW_CANCELLEDBYUSER)
		{
			CComBSTR bstr;
			if (p->M_ErrorMessage())
				p->M_ErrorMessage()->GetLocalized(m_tLocaleID, &bstr);
			CComPtr<IDesignerCore> pDC;
			RWCoCreateInstance(pDC, __uuidof(DesignerCore));
			CComPtr<ILocalizedString> pCaption;
			if (pDC) pDC->Name(&pCaption);
			CComBSTR bstrCaption;
			if (pCaption) pCaption->GetLocalized(m_tLocaleID, &bstrCaption);
			if (bstrCaption == NULL) bstrCaption = L"Error";
			if (bstr != NULL && bstr[0])
			{
				::MessageBox(m_hWnd, CW2T(bstr), CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
			}
			else
			{
				TCHAR szTemplate[256] = _T("");
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_GENERICERROR, szTemplate, itemsof(szTemplate), LANGIDFROMLCID(m_tLocaleID));
				TCHAR szMsg[256];
				_stprintf(szMsg, szTemplate, hRes);
				::MessageBox(m_hWnd, szMsg, CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
	catch (...)
	{
	}

	return 0;
}

LRESULT CDesignerViewTree::OnMenuSelect(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	WORD wFlags = HIWORD(a_wParam);
	if (wFlags == 0xFFFF && a_lParam == NULL)
	{ // menu closing
		m_bShowCommandDesc = false;
	}
	else
	{
		m_bShowCommandDesc = true;
		m_bstrCommandDesc.Empty();
		if (wFlags & MF_POPUP)
		{
			// TODO: submenu descriptions
		}
		else
		{
			WORD wID = LOWORD(a_wParam)-ID_MENU_FIRST;
			if (wID < m_cContextOps.size())
			{
				CComPtr<ILocalizedString> pStr;
				m_cContextOps[wID]->Description(&pStr);
				if (pStr)
				{
					pStr->GetLocalized(m_tLocaleID, &m_bstrCommandDesc);
					if (m_bstrCommandDesc) for (LPOLESTR p = m_bstrCommandDesc; *p; ++p)
						if (*p == L'\n') { *p = L'\0'; break; }
				}
			}
		}
	}
	if (m_pStatusBar) m_pStatusBar->Notify(0, 0);

	return 1;
}

HRESULT CDesignerViewTree::ProcessContextMenu(IEnumUnknowns* a_pOps, int a_xPos, int a_yPos)
{
	Reset(m_cImageMenu);
	// TODO: check if the image list is too big and eventually delete it

	ATLASSERT(m_cContextOps.empty());
	CMenu cMenu;
	cMenu.CreatePopupMenu();

	UINT nMenuID = ID_MENU_FIRST;
	InsertMenuItems(a_pOps, m_cContextOps, cMenu.m_hMenu, &nMenuID);
	if (nMenuID == ID_MENU_FIRST)
		return S_FALSE;

	UINT nSelection = cMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, a_xPos, a_yPos, m_hWnd, NULL);
	HRESULT hRes = S_OK;
	if (nSelection != 0)
	{
		CWaitCursor cWait;
		hRes = m_cContextOps[nSelection-ID_MENU_FIRST]->Execute(reinterpret_cast<RWHWND>(m_hWnd), m_tLocaleID);
	}
	m_cContextOps.clear();
	return hRes;
}

void CDesignerViewTree::InsertMenuItems(IEnumUnknowns* a_pCmds, CContextOps& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID)
{
	bool bInsertSeparator = false;
	bool bFirst = true;
	ULONG nItems = 0;
	if (a_pCmds)
		a_pCmds->Size(&nItems);
	for (ULONG i = 0; i < nItems; i++)
	{
		CComPtr<IDocumentMenuCommand> pCmd;
		a_pCmds->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
		if (pCmd == NULL)
			continue;
		EMenuCommandState eState = EMCSNormal;
		pCmd->State(&eState);
		if (eState & EMCSSeparator)
		{
			bInsertSeparator = true;
			continue;
		}
		if (eState & EMCSDisabled)
			continue; // do no show disabled items in context menu
		int nIcon = -1;
		GUID tIconID;
		HRESULT hRes;
		if (SUCCEEDED(hRes = pCmd->IconID(&tIconID)) && !IsEqualGUID(tIconID, GUID_NULL))
		{
			CImageMap::const_iterator j = m_cMenuImageMap.find(tIconID);
			if (j != m_cMenuImageMap.end())
			{
				nIcon = j->second;
				if (hRes == S_FALSE)
				{
					HICON hIcon = NULL;
					pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
					if (hIcon)
					{
						m_cImageMenu.ReplaceIcon(nIcon, hIcon);
						DestroyIcon(hIcon);
					}
				}
			}
			else
			{
				HICON hIcon = NULL;
				pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
				if (hIcon)
				{
					m_cImageMenu.AddIcon(hIcon);
					DestroyIcon(hIcon);
					nIcon = m_cMenuImageMap[tIconID] = m_cImageMenu.GetImageCount()-1;
				}
			}
		}
		CComPtr<ILocalizedString> pName;
		pCmd->Name(&pName);
		CComBSTR bstrName;
		if (pName)
			pName->GetLocalized(m_tLocaleID, &bstrName);
		COLE2CT strName(bstrName.Length() ? bstrName.operator BSTR() : L"");
		if (eState & EMCSSubMenu)
		{
			CComPtr<IEnumUnknowns> pSubCmds;
			pCmd->SubCommands(&pSubCmds);
			ULONG nSubCmds = 0;
			if (pSubCmds && SUCCEEDED(pSubCmds->Size(&nSubCmds)) && nSubCmds)
			{
				CMenu cSubMenu;
				cSubMenu.CreatePopupMenu();
				InsertMenuItems(pSubCmds, a_cContextOps, cSubMenu.m_hMenu, a_pnMenuID);
				AddItem(reinterpret_cast<UINT>(cSubMenu.operator HMENU()), strName, nIcon);
				if (bInsertSeparator && !bFirst)
					a_cMenu.AppendMenu(MFT_SEPARATOR);
				a_cMenu.AppendMenu(MF_POPUP|MFT_OWNERDRAW, reinterpret_cast<UINT_PTR>(cSubMenu.Detach()), LPCTSTR(0));
				bInsertSeparator = bFirst = false;
			}
		}
		else
		{
			UINT uFlags = MFT_OWNERDRAW;
			if (eState & EMCSDisabled)
				uFlags |= MFS_DISABLED|MFS_GRAYED;
			if (eState & EMCSChecked)
				uFlags |= MFS_CHECKED;
			if (eState & EMCSRadio)
				uFlags |= MFT_RADIOCHECK;
			if (eState & EMCSBreak)
				uFlags |= MFT_MENUBREAK;

			a_cContextOps.push_back(pCmd); // should rollback it if the next op fails

			AddItem(*a_pnMenuID, strName, nIcon);
			if (bInsertSeparator && !bFirst)
				a_cMenu.AppendMenu(MFT_SEPARATOR);
			a_cMenu.AppendMenu(uFlags, (*a_pnMenuID)++, LPCTSTR(NULL));
			bInsertSeparator = bFirst = false;
		}
	}
}

LRESULT CDesignerViewTree::OnKeyDown(int UNREF(a_nCtrlID), LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	if (m_eKeyDownAction == CFGVAL_TKDA_STARTEDITING)
	{
		HTREEITEM hTI = m_wndTree.GetSelectedItem();
		if (hTI != NULL)
		{
			IComparable* p = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(hTI));
			CComQIPtr<IItemString> pItemString(p);
			CComQIPtr<IItemInt> pItemInt(p);
			CComQIPtr<IItemFloat> pItemFloat(p);
			CComQIPtr<IItemBool> pItemBool(p);
			CComQIPtr<IItemChoice> pItemChoice(p);
			if (pItemString == NULL && pItemInt == NULL && pItemFloat == NULL && pItemBool == NULL && pItemChoice == NULL)
			{
				return 1;
			}

			LPNMTVKEYDOWN pNMTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(a_pNMHeader);

			if (pNMTVKeyDown->wVKey == VK_F2)
			{
				m_wndTree.EditLabel(hTI);
				return 1;
			}
			TCHAR szBuf[10];
			int nCount = 0;
			BYTE szKeyboardState[256];
			GetKeyboardState(szKeyboardState);
#ifdef _UNICODE
			nCount = ToUnicode(pNMTVKeyDown->wVKey,MapVirtualKey(pNMTVKeyDown->wVKey, 2), szKeyboardState, szBuf, itemsof(szBuf), 0);
#else
			nCount = ToAscii(pNMTVKeyDown->wVKey, MapVirtualKey(pNMTVKeyDown->wVKey, 2), szKeyboardState, reinterpret_cast<LPWORD>(szBuf), 0);
#endif
			if (nCount > 0 && szBuf[0] >= _T(' '))
			{
				szBuf[nCount] = _T('\0');
				CEdit wndEdit = m_wndTree.EditLabel(hTI);
				wndEdit.SetWindowText(szBuf);
				wndEdit.SetSel(nCount, nCount);
			}
		}
		return 1;
	}
	else
	{
		a_bHandled = FALSE;
		return 0;
	}
}

LRESULT CDesignerViewTree::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		TCHAR szBuffer[512] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_TV_CONTEXTHELP, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
		RECT rcItem;
		::GetWindowRect(m_hWnd, &rcItem);
		HH_POPUP hhp;
		hhp.cbStruct = sizeof(hhp);
		hhp.hinst = _pModule->get_m_hInst();
		hhp.idString = 0;
		hhp.pszText = szBuffer;
		hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
		hhp.clrForeground = 0xffffffff;
		hhp.clrBackground = 0xffffffff;
		hhp.rcMargins.left = -1;
		hhp.rcMargins.top = -1;
		hhp.rcMargins.right = -1;
		hhp.rcMargins.bottom = -1;
		hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
		HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
		return 0;
	}
	a_bHandled = FALSE;
	return 0;
}

void CDesignerViewTree::Notify(POINT a_tPt, UINT a_nFlags)
{
	if (a_nFlags&(MK_LBUTTON|MK_MBUTTON|MK_RBUTTON))
	{
		if (m_bShowItemDesc)
		{
			m_bShowItemDesc = false;
			m_pItemForDesc  = NULL;
			m_bstrItemDesc.Empty();
			if (m_pStatusBar) m_pStatusBar->Notify(0, 0);
		}
		return ;
	}

	UINT nFlags = 0;
	HTREEITEM hHit = m_wndTree.HitTest(a_tPt, &nFlags);
	if (hHit && (nFlags&TVHT_ONITEM))
	{
		CComPtr<IUIItem> pUIItem;
		m_pRoot->ItemFeatureGet(reinterpret_cast<IComparable*>(m_wndTree.GetItemData(hHit)), __uuidof(IUIItem), reinterpret_cast<void**>(&pUIItem));
		if (m_pItemForDesc != pUIItem)
		{
			CComBSTR bstrItemDesc;
			if (pUIItem) pUIItem->DescriptionGet(m_tLocaleID, &bstrItemDesc);
			m_pItemForDesc.Attach(pUIItem.Detach());
			if (bstrItemDesc != m_bstrItemDesc)
			{
				m_bstrItemDesc.Attach(bstrItemDesc.Detach());
				m_bShowItemDesc = m_bstrItemDesc.Length() != 0;
				if (m_pStatusBar) m_pStatusBar->Notify(0, 0);
			}
		}
	}
	else if (m_bShowItemDesc)
	{
		m_bShowItemDesc = false;
		m_pItemForDesc  = NULL;
		m_bstrItemDesc.Empty();
		if (m_pStatusBar) m_pStatusBar->Notify(0, 0);
	}
}

bool CDesignerViewTree::OnCanEditLabel(HTREEITEM a_hTI)
{
	IComparable* pItem = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(a_hTI));
	CComQIPtr<IItemString> pString(pItem);
	if (pString) return true;
	CComQIPtr<IItemInt> pInt(pItem);
	if (pInt) return true;
	CComQIPtr<IItemFloat> pFloat(pItem);
	if (pFloat) return true;
	CComQIPtr<IItemChoice> pChoice(pItem);
	if (pChoice) return true;
	return false;
}

void CDesignerViewTree::GetItemDisplayProps(IComparable* a_pItem, bool& bRefresh, bool& bExpanded, TVITEMEX& tItem)
{
	// for Thumbnails mode only
	CComQIPtr<IUIItem> pUI(a_pItem);
	bool const bUseThumbnail = pUI && pUI->UseThumbnail() == S_OK;
	if (pUI)
		bExpanded = pUI->ExpandedByDefault() == S_OK;
	bool bUseIcon = false;
	CComBSTR bstrLabel;
	if (bUseThumbnail)
	{
		ObjectLock cLock(this);
		CThumbnailMap::const_iterator i = m_cThumbnailMap.find(a_pItem);
		if (i == m_cThumbnailMap.end())
		{
			bRefresh = true;
		}
		else
		{
			if (m_pRichGUI)
			{
				ULONG nTimeStamp = i->second.nTimestamp+1;
				m_pRichGUI->Thumbnail(a_pItem, 0, 0, NULL, NULL, &nTimeStamp);
				bRefresh = nTimeStamp != i->second.nTimestamp;
			}
			else
			{
				bRefresh = true;
			}
		}
	}
	if (pUI)
	{
		bUseIcon = GetItemImage<I_IMAGENONE>(pUI.p) != I_IMAGENONE;
		pUI->NameGet(m_tLocaleID, &bstrLabel);
	}
	CComQIPtr<IItemBool> pBool(a_pItem);
	bool bHasCheck = pBool;

	tItem.mask |= TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_INTEGRAL;
	tItem.iIntegral = bUseThumbnail ? 3 : 1;
	//tItem.hItem = *j;
	wchar_t dummy[] = L"";
	//tItem.cchTextMax;
	tItem.iImage = (bUseIcon ? 1 : 0)|(bHasCheck ? 2 : 0);
	std::tstring strVal;
	tItem.iSelectedImage = 0;
	bool bDataValid = GetItemDataAsString(pUI.p, false, strVal);
	SIZE szValue = {0, 0};
	SIZE szLabel = {0, 0};
	HDC hDC = GetDC();
	HGDIOBJ hOldFont = SelectObject(hDC, m_wndTree.GetFont());
	if (strVal.length())
	{
		GetTextExtentPoint32(hDC, strVal.c_str(), strVal.length(), &szValue);
		if (bstrLabel.m_str && !bUseThumbnail)
			bstrLabel += L": ";
	}
	if (bDataValid && szValue.cx < LONG(2*m_nButtonSize))
		szValue.cx = 2*m_nButtonSize;
	if (bstrLabel.m_str)
		GetTextExtentPoint32(hDC, bstrLabel.m_str, bstrLabel.Length(), &szLabel);
	SelectObject(hDC, hOldFont);
	ReleaseDC(hDC);
	tItem.pszText = bstrLabel.m_str ? bstrLabel.m_str : dummy;
	tItem.iImage |= szLabel.cx<<2;
	tItem.iSelectedImage = szValue.cx;
}

void CDesignerViewTree::GetItemDisplayProps(IComparable* a_pItem, std::tstring& a_strName, int& a_nImage, bool& a_bExpanded)
{
	CComQIPtr<IUIItem> pUIItem(a_pItem);
	if (pUIItem != NULL)
	{
		a_bExpanded = pUIItem->ExpandedByDefault() == S_OK;
		CComBSTR bstrName;
		pUIItem->NameGet(m_tLocaleID, &bstrName);
		a_nImage = GetItemImage<I_IMAGENONE>(pUIItem.p);
		if (bstrName.Length() != 0)
		{
			USES_CONVERSION;
			a_strName = OLE2CT(bstrName);
		}
	}
	else
	{
		a_nImage = I_IMAGENONE;
	}

	std::tstring strVal;
	bool bDataValid = GetItemDataAsString(a_pItem, true, strVal);

	if (strVal.compare(a_strName) != 0 && bDataValid)
	{
		if (!a_strName.empty())
		{
			a_strName += _T(": ");
		}
		a_strName += strVal;
	}
	if (a_strName.empty() && !bDataValid)
	{
		TCHAR szItem[64] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_NAMELESSITEM, szItem, itemsof(szItem), LANGIDFROMLCID(m_tLocaleID));
		a_strName = szItem;
	}
}

bool CDesignerViewTree::GetItemDataAsString(IComparable* a_pItem, bool a_bIncludeBool, std::tstring& a_strData) const
{
	{
		CComQIPtr<IItemString> pItem(a_pItem);
		if (pItem != NULL)
		{
			CComBSTR bstrString;
			pItem->ValueGet(&bstrString);
			if (bstrString.Length() != 0)
			{
				USES_CONVERSION;
				a_strData = OLE2CT(bstrString);
			}
			return true;
		}
	}
	{
		CComQIPtr<IItemInt> pItem(a_pItem);
		if (pItem != NULL)
		{
			LONG nVal = 0;
			if (SUCCEEDED(pItem->ValueGet(&nVal)))
			{
				TCHAR szTmp[32];
				_stprintf(szTmp, _T("%i"), static_cast<int>(nVal));
				a_strData = szTmp;
			}
			return true;
		}
	}
	{
		CComQIPtr<IItemFloat> pItem(a_pItem);
		if (pItem != NULL)
		{
			float fVal = 0;
			if (SUCCEEDED(pItem->ValueGet(&fVal)))
			{
				TCHAR szTmp[32];
				_stprintf(szTmp, _T("%g"), fVal);
				a_strData = szTmp;
			}
			return true;
		}
	}
	if (a_bIncludeBool)
	{
		CComQIPtr<IItemBool> pItem(a_pItem);
		if (pItem != NULL)
		{
			BYTE bVal = 0;
			if (SUCCEEDED(pItem->ValueGet(&bVal)))
			{
				TCHAR szItem[32] = _T("");
				Win32LangEx::LoadString(_pModule->get_m_hInst(), bVal ? IDS_VALUE_TRUE : IDS_VALUE_FALSE, szItem, itemsof(szItem), LANGIDFROMLCID(m_tLocaleID));
				a_strData = szItem;
			}
			return true;
		}
	}
	{
		CComQIPtr<IItemChoice> pItem(a_pItem);
		if (pItem != NULL)
		{
			ULONG nVal = 0;
			CComPtr<IEnumUnknowns> pOpts;
			if (SUCCEEDED(pItem->ValueGet(&nVal)) && SUCCEEDED(pItem->OptionsEnum(&pOpts)))
			{
				CComPtr<ILocalizedString> pStr;
				pOpts->Get(nVal, __uuidof(ILocalizedString), reinterpret_cast<void**>(&pStr));
				CComBSTR bstr;
				if (pStr != NULL && SUCCEEDED(pStr->GetLocalized(m_tLocaleID, &bstr)) && bstr.Length() != 0)
				{
					USES_CONVERSION;
					a_strData = OLE2CT(bstr);
				}
			}
			return true;
		}
	}
	return false;
}

#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>
void GetUndoName(LPCTSTR a_pszData, ILocalizedString** a_ppName)
{
	CComObject<CPrintfLocalizedString>* p = NULL;
	CComObject<CPrintfLocalizedString>::CreateInstance(&p);
	CComPtr<ILocalizedString> pTmp = p;
	CComPtr<ILocalizedString> pArg;
	int nLen = _tcslen(a_pszData);
	if (nLen < 18)
	{
		pArg.Attach(new CSimpleLocalizedString(CComBSTR(a_pszData).Detach()));
	}
	else
	{
		CComBSTR bstr(18, CT2CW(a_pszData));
		bstr[15] = bstr[16] = bstr[17] = L'.';
		pArg.Attach(new CSimpleLocalizedString(bstr.Detach()));
	}
	p->Init(CMultiLanguageString::GetAuto(L"[0409]Type \"%s\"[0405]Psaní \"%s\""), pArg);
	*a_ppName = pTmp.Detach();
}

bool CDesignerViewTree::SetItemDataFromString(LPCTSTR a_pszData, IComparable* a_pItem) const
{
	{
		CComQIPtr<IItemString> pItem(a_pItem);
		if (pItem != NULL)
		{
			CComPtr<ILocalizedString> pName;
			GetUndoName(a_pszData, &pName);
			CUndoBlock cUndo(m_pOriginal, pName);
			return SUCCEEDED(pItem->ValueSet(CComBSTR(a_pszData)));
		}
	}
	{
		CComQIPtr<IItemInt> pItem(a_pItem);
		if (pItem != NULL)
		{
			// TODO: error detection
			CComPtr<ILocalizedString> pName;
			GetUndoName(a_pszData, &pName);
			CUndoBlock cUndo(m_pOriginal, pName);
			return SUCCEEDED(pItem->ValueSet(_ttoi(a_pszData)));
		}
	}
	{
		CComQIPtr<IItemBool> pItem(a_pItem);
		if (pItem != NULL)
		{
			TCHAR szTrue[32] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_VALUE_TRUE, szTrue, itemsof(szTrue), LANGIDFROMLCID(m_tLocaleID));
			if (_tcsicmp(szTrue, a_pszData) == 0 || _tcscmp(_T("1"), a_pszData) == 0)
			{
				CComPtr<ILocalizedString> pName;
				GetUndoName(a_pszData, &pName);
				CUndoBlock cUndo(m_pOriginal, pName);
				return SUCCEEDED(pItem->ValueSet(1));
			}
			TCHAR szFalse[32] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_VALUE_FALSE, szFalse, itemsof(szFalse), LANGIDFROMLCID(m_tLocaleID));
			if (_tcsicmp(szFalse, a_pszData) == 0 || _tcscmp(_T("0"), a_pszData) == 0)
			{
				CComPtr<ILocalizedString> pName;
				GetUndoName(a_pszData, &pName);
				CUndoBlock cUndo(m_pOriginal, pName);
				return SUCCEEDED(pItem->ValueSet(0));
			}
			return S_FALSE;
		}
	}
	// TODO: other types...
	return false;
}

void CDesignerViewTree::DeleteItem(HTREEITEM a_hItem)
{
	DeleteItems(a_hItem);
	IComparable* const pF = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(a_hItem));
	m_wndTree.DeleteItem(a_hItem);
	CThumbnailMap::iterator iTM = m_cThumbnailMap.find(pF);
	if (iTM != m_cThumbnailMap.end())
	{
		m_cFreeThumbnailIndices.push_back(iTM->second.iImageIndex);
		iTM->first->Release();
		m_cThumbnailMap.erase(iTM);
	}
	pF->Release(); // release the pointer in item data
	for (CBackMap::iterator iEI = m_cBackMap.begin(); iEI != m_cBackMap.end(); ++iEI)
	{
		if (iEI->second == a_hItem)
		{
			m_cBackMap.erase(iEI);
			return;
		}
	}
	ATLASSERT(0);
}

void CDesignerViewTree::DeleteItems(HTREEITEM a_hRoot)
{
	HTREEITEM hItem;
	for (hItem = m_wndTree.GetChildItem(a_hRoot); hItem != NULL; )
	{
		HTREEITEM hThis = hItem;
		hItem = m_wndTree.GetNextSiblingItem(hItem);
		DeleteItem(hThis);
	}
}

STDMETHODIMP CDesignerViewTree::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (a_bBeforeAccel && m_wndTree.IsWindow())
	{
		HWND hEdit = m_wndTree.GetEditControl();
		if (hEdit != NULL && hEdit == a_pMsg->hwnd)
		{
			TranslateMessage(a_pMsg);
			DispatchMessage(a_pMsg);
			return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CDesignerViewTree::RootIDGet(GUID* a_pID)
{
	try
	{
		*a_pID = m_iidStructuredRoot;
		return S_OK;
	}
	catch (...)
	{
		return a_pID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewTree::RootIDSet(REFIID a_tID)
{
	try
	{
		if (IsEqualIID(a_tID, m_iidStructuredRoot) ||
			IsEqualIID(a_tID, GUID_NULL))
			return S_FALSE;

		{
			// update config
			CComBSTR cCFGID_TREE_STRUCTUREROOT(CFGID_TREE_STRUCTUREROOT);
			m_pConfig->ItemValuesSet(1, &(cCFGID_TREE_STRUCTUREROOT.m_str), CConfigValue(a_tID));
		}

		CComPtr<IStructuredRoot> pRoot;
		m_pOriginal->QueryFeatureInterface(a_tID, reinterpret_cast<void**>(&pRoot));
		if (pRoot == NULL)
			return E_FAIL;

		m_bSelectionSending = true;
		try
		{
			if (m_pRoot != NULL)
				m_pRoot->ObserverDel(CObserverImpl<CDesignerViewTree, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
			{
				ObjectLock cLock(this);
				DeleteItems(TVI_ROOT);
				m_pRoot = pRoot;
				m_iidStructuredRoot = a_tID;
				InsertItems(NULL, TVI_ROOT, true);
			}
			m_pRoot->ObserverIns(CObserverImpl<CDesignerViewTree, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
		}
		catch (...) {}
		m_bSelectionSending = false;

		CComPtr<ISharedState> pState;
		m_pStateMgr->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		//if (pState != NULL)
		{
			// set initial selection if it is available
			TSharedStateChange tChangeParams;
			tChangeParams.bstrName = m_bstrSyncGroup;
			tChangeParams.pState = pState;
			OwnerNotify(0, tChangeParams);
			HTREEITEM hSel = GetFirstSelectedItem();
			if (hSel)
			{
				m_wndTree.EnsureVisible(hSel);
			}
			else
			{
				hSel = m_wndTree.GetSelectedItem();
				if (hSel)
				{
					m_wndTree.SetItemState(hSel, 0, TVIS_SELECTED|TVIS_FOCUSED);
				}

			}
			//if (m_wndTree.GetSelectedCount() == 0)
			//{
			//	if (m_wndTree.GetCount() != 0)
			//	{
			//		HTREEITEM hSel = m_wndTree.GetNextItem(TVI_ROOT, TVGN_ROOT);
			//		m_wndTree.SelectItem(hSel);
			//		m_wndTree.EnsureVisible(hSel);
			//	}
			//}
			//else
			//{
			//	HTREEITEM hSel = GetFirstSelectedItem();
			//	m_wndTree.EnsureVisible(hSel);
			//}
		}
		//else
		//{
		//	if (m_wndTree.GetCount() != 0)
		//	{
		//		HTREEITEM hSel = m_wndTree.GetNextItem(TVI_ROOT, TVGN_ROOT);
		//		m_wndTree.SelectItem(hSel);
		//		m_wndTree.EnsureVisible(hSel);
		//	}
		//}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewTree::SelectionGet(IEnumUnknowns** a_ppSelection)
{
	try
	{
		*a_ppSelection = NULL;

		if (m_hWnd == NULL)
			return E_FAIL;

		CTreeItemList cList;
		GetSelectedList(cList);
		CAutoVectorPtr<IUnknown*> apItems(cList.GetSize() > 0 ? new IUnknown*[cList.GetSize()] : NULL);
		for (int i = 0; i < cList.GetSize(); i++)
			apItems[i] = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(cList[i]));

		CComPtr<IEnumUnknownsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumUnknowns));
		pTmp->InsertMultiple(cList.GetSize(), apItems);
		*a_ppSelection = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSelection ? E_UNEXPECTED : E_POINTER;
	}
}

LRESULT CDesignerViewTree::OnBeginDrag(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	NMTREEVIEW* pNMTV = reinterpret_cast<NMTREEVIEW*>(a_pNMHeader);

	if (m_pRichGUI == NULL)
		return 0;

	m_bSelectDropped = true;
	CComPtr<IEnumUnknownsInit> pItems;
	RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
	if (pNMTV->itemNew.state&TVIS_SELECTED)
	{
		CTreeItemList cList;
		GetSelectedList(cList);
		for (int i = 0; i < cList.GetSize(); i++)
			pItems->Insert(reinterpret_cast<IComparable*>(m_wndTree.GetItemData(cList[i])));
	}
	else
	{
		pItems->Insert(reinterpret_cast<IComparable*>(pNMTV->itemNew.lParam));
		m_bSelectDropped = false;
	}
	CComPtr<IDataObject> pDO;
	CComPtr<IDropSource> pDS;
	DWORD dwOKEffects = 0;
	if (SUCCEEDED(m_pRichGUI->Begin(pItems, &pDO, &pDS, &dwOKEffects)))
	{
		DWORD dwEffect = DROPEFFECT_NONE;
		HRESULT hr = ::DoDragDrop(pDO, pDS, dwOKEffects, &dwEffect);
	}
	m_bSelectDropped = true;

	return 0;
}

STDMETHODIMP CDesignerViewTree::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
{
	try
	{
		m_wndTree.ScreenToClient(&a_pt);
		RECT rc;
		m_wndTree.GetClientRect(&rc);
		if (a_pt.x < rc.left || a_pt.y < rc.top || a_pt.x >= rc.right || a_pt.y >= rc.bottom)
		{
			if (m_hDropHighlight != NULL)
			{
				m_hDropHighlight = NULL;
				m_nDropHighlightFlag = 0;
				m_wndTree.SetInsertMark(NULL, FALSE);
			}
			return E_FAIL;
		}
		UINT nFlags = 0;
		HTREEITEM hItem = m_wndTree.HitTest(a_pt, &nFlags);
		CComPtr<IComparable> pItem = hItem ? reinterpret_cast<IComparable*>(m_wndTree.GetItemData(hItem)) : NULL;
		RECT rcItem = {0, 0, 0, 0};
		if (hItem)
			m_wndTree.GetItemRect(hItem, &rcItem, FALSE);
		DWORD dwFlags = a_pdwEffect ? *a_pdwEffect : 0;
		HRESULT hRes = m_pRichGUI->Drag(a_pDataObj, a_pFileNames, a_grfKeyState, pItem, a_pt.y > ((rcItem.bottom+rcItem.top)>>1) ? EDNDPLower : EDNDPUpper, &dwFlags, a_ppFeedback);
		bool bScroll = dwFlags&DROPEFFECT_SCROLL;
		if (a_pdwEffect) *a_pdwEffect = dwFlags&(~DND_INSERTMARK_MASK);
		dwFlags &= DND_INSERTMARK_MASK;
		if (hItem == NULL)// && dwFlags == DND_INSERTMARK_BEFORE)
		{
			hItem = m_wndTree.GetLastVisibleItem();
			dwFlags = DND_INSERTMARK_AFTER;
		}
		if (dwFlags == 0)
			hItem = NULL;
		if (m_hDropHighlight != hItem || m_nDropHighlightFlag != dwFlags)
		{
			m_hDropHighlight = hItem;
			m_nDropHighlightFlag = dwFlags;
			m_wndTree.SetInsertMark(hItem, dwFlags != DND_INSERTMARK_BEFORE);
		}
		// scrolling when near top or bottom (cannot scroll if above or below, unless DoDragDrop is replaced)
		if (hItem || bScroll)
		{
			DWORD dwTime = GetTickCount();
			if ((dwTime-m_dwLastScrollTime) > 250)
			{
				int nItemHeight = m_wndTree.GetItemHeight();
				HTREEITEM hShow = NULL;
				if (a_pt.y <= nItemHeight)
				{
					UINT dw;
					POINT pt = {(rc.right+rc.left)>>1, 0};
					HTREEITEM hItemFirst = m_wndTree.HitTest(pt, &dw);
					if (hItemFirst)
						hShow = m_wndTree.GetPrevVisibleItem(hItemFirst);
				}
				else if (a_pt.y >= rc.bottom-nItemHeight)
				{
					//m_wndTree.SetScrollPos(SB_VERT, m_wndTree.GetScrollPos(SB_VERT)+1);
					//m_dwLastScrollTime = dwTime;
					UINT dw;
					POINT pt = {(rc.right+rc.left)>>1, rc.bottom-nItemHeight+1};
					HTREEITEM hItemLast = m_wndTree.HitTest(pt, &dw);
					if (hItemLast)
						hShow = m_wndTree.GetNextVisibleItem(hItemLast);
				}
				if (hShow)
				{
					m_wndTree.EnsureVisible(hShow);
					m_dwLastScrollTime = dwTime;
				}
			}
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <SharedStateUndo.h>

STDMETHODIMP CDesignerViewTree::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt)
{
	try
	{
		if (m_hDropHighlight != NULL)
		{
			// remove insertion mark
			m_hDropHighlight = NULL;
			m_nDropHighlightFlag = 0;
			m_wndTree.SetInsertMark(NULL, FALSE);
		}

		if (a_pDataObj == NULL && a_pFileNames == NULL)
			return E_RW_CANCELLEDBYUSER; // cancelled d'n'd operation

		m_wndTree.ScreenToClient(&a_pt);
		RECT rc;
		m_wndTree.GetClientRect(&rc);
		if (a_pt.x < rc.left || a_pt.y < rc.top || a_pt.x >= rc.right || a_pt.y >= rc.bottom)
			return E_FAIL;
		UINT nFlags = 0;
		HTREEITEM hItem = m_wndTree.HitTest(a_pt, &nFlags);
		EDNDPoint eDNDP = EDNDPLower;
		IComparable* pItem = NULL;
		if (hItem)
		{
			pItem = reinterpret_cast<IComparable*>(m_wndTree.GetItemData(hItem));
			RECT rcItem = {0, 0, 0, 0};
			m_wndTree.GetItemRect(hItem, &rcItem, FALSE);
			eDNDP = a_pt.y > ((rcItem.bottom+rcItem.top)>>1) ? EDNDPLower : EDNDPUpper;
		}
		CUndoBlock cUndo(m_pOriginal, CMultiLanguageString::GetAuto(L"[0409]Drag and drop[0405]Táhnout a pustit"));
		CComPtr<ISharedState> pSel;
		HRESULT hRes = m_pRichGUI->Drop(a_pDataObj, a_pFileNames, a_grfKeyState, pItem, eDNDP, m_tLocaleID, m_bSelectDropped ? &pSel : NULL);
		if (FAILED(hRes))
			return hRes;
		if (pSel)
		{
			if (cUndo)
				CSharedStateUndo<ISharedStateManager>::SaveState(cUndo, m_pStateMgr, m_bstrSyncGroup);
			m_pStateMgr->StateSet(m_bstrSyncGroup, pSel);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewTree::Priority(BYTE* a_pPrio)
{
	return m_pRichGUI->ClipboardPriority(a_pPrio);
}

STDMETHODIMP CDesignerViewTree::ObjectName(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName)
{
	return m_pRichGUI->ClipboardName(static_cast<ERichGUIClipboardAction>(a_eAction), m_pContextSel.p ? m_pContextSel.p : m_pLastSel.p, a_ppName);
}

STDMETHODIMP CDesignerViewTree::ObjectIconID(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID)
{
	return m_pRichGUI->ClipboardIconID(static_cast<ERichGUIClipboardAction>(a_eAction), m_pContextSel.p ? m_pContextSel.p : m_pLastSel.p, a_pIconID);
}

STDMETHODIMP CDesignerViewTree::ObjectIcon(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay)
{
	return m_pRichGUI->ClipboardIcon(static_cast<ERichGUIClipboardAction>(a_eAction), m_pContextSel.p ? m_pContextSel.p : m_pLastSel.p, a_nSize, a_phIcon, a_pOverlay);
}

HRESULT CDesignerViewTree::ClipboardTest(ERichGUIClipboardAction a_eAction)
{
	try
	{
		if (m_pContextSel.p)
			return m_pRichGUI->ClipboardCheck(a_eAction, m_hWnd, m_pContextSel);
		CComPtr<ISharedState> pState;
		m_pStateMgr->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		return m_pRichGUI->ClipboardCheck(a_eAction, m_hWnd, pState);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDesignerViewTree::ClipboardRun(ERichGUIClipboardAction a_eAction)
{
	try
	{
		if (m_pContextSel.p)
		{
			CComPtr<ISharedState> pDummy;
			return m_pRichGUI->ClipboardRun(a_eAction, m_hWnd, m_tLocaleID, m_pContextSel, &pDummy);
		}
		CComPtr<ISharedState> pState;
		m_pStateMgr->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<ISharedState> pNewState;
		CUndoBlock cUndo(m_pOriginal);
		HRESULT hRes = m_pRichGUI->ClipboardRun(a_eAction, m_hWnd, m_tLocaleID, pState, &pNewState);
		if (pNewState)
		{
			if (cUndo)
				CSharedStateUndo<ISharedStateManager>::SaveState(cUndo, m_pStateMgr, m_bstrSyncGroup, pState);
			m_pStateMgr->StateSet(m_bstrSyncGroup, pNewState);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

