// RecentFileDlg.cpp : Implementation of CRecentFileDlg

#include "stdafx.h"
#include "RecentFileDlg.h"

#include <XPGUI.h>
#include "ConfigIDsApp.h"
#include <IconRenderer.h>


HRESULT CRecentFileDlg::Activate()
{
	ShowWindow(SW_SHOW);
	GotoDlgCtrl(m_wndList);
	return S_OK;
}

HRESULT CRecentFileDlg::Deactivate()
{
	if (IsWindow())
	{
		ShowWindow(SW_HIDE);
	}
	return S_OK;
}

HRESULT CRecentFileDlg::ClickedDefault()
{
	if (m_iActiveItem == -1)
	{
		return S_FALSE;
	}

	TCHAR szTmp[MAX_PATH] = _T("");
	m_wndList.GetItemText(m_iActiveItem, 1, szTmp, itemsof(szTmp));

	CComPtr<IInputManager> pInMgr;
	RWCoCreateInstance(pInMgr, __uuidof(InputManager));

	CComPtr<IEnumUnknownsInit> pBlds;
	RWCoCreateInstance(pBlds, __uuidof(EnumUnknowns));
	GetBuilders(M_AppConfig(), pBlds);
	CComPtr<IDocument> pDoc;
	pInMgr->DocumentCreateEx(pBlds, CStorageFilter(szTmp), NULL, &pDoc);

	if (pDoc)
	{
		m_pClbk->OpenDocument(pDoc);
		return S_OK;
	}

	CComPtr<IApplicationInfo> pAI;
	RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
	CComPtr<ILocalizedString> pCaption;
	if (pAI) pAI->Name(&pCaption);
	CComBSTR bstr;
	if (pCaption) pCaption->GetLocalized(m_tLocaleID, &bstr);
	CComBSTR bstrTempl;
	CMultiLanguageString::GetLocalized(L"[0409]Failed to open %s.[0405]Nepodařilo se otevřít %s.", m_tLocaleID, &bstrTempl);
	CComBSTR bstrQuestion;
	CMultiLanguageString::GetLocalized(L"[0409]Remove it from the list of recent files?[0405]Odstranit soubor ze seznamu nedávno použitých?", m_tLocaleID, &bstrQuestion);

	LPTSTR p1 = _tcsrchr(szTmp, _T('\\'));
	LPTSTR p2 = _tcsrchr(szTmp, _T('/'));
	if (p2 > p1) p1 = p2;
	if (p1 == NULL) p1 = szTmp; else ++p1;
	CAutoVectorPtr<TCHAR> pszText(new TCHAR[_tcslen(p1)+_tcslen(bstrTempl)+_tcslen(bstrQuestion)+10]);
	_stprintf(pszText.m_p, bstrTempl, p1);
	_tcscat(pszText.m_p, _T("\r\n\r\n"));
	_tcscat(pszText.m_p, bstrQuestion);
	if (IDYES == MessageBox(pszText, bstr, MB_YESNO|MB_ICONQUESTION))
	{
		RecentFiles::RemoveRecentFile(m_pCfg, m_iActiveItem);
		IStorageFilter* pLoc = reinterpret_cast<IStorageFilter*>(m_wndList.GetItemData(m_iActiveItem));
		m_wndList.DeleteItem(m_iActiveItem);
		if (pLoc)
			pLoc->Release();
		int nCount = m_wndList.GetItemCount();
		if (nCount > 1)
			m_wndList.SetItemState(m_iActiveItem < nCount-1 ? m_iActiveItem : m_iActiveItem-1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
	m_bIgnoreNext = true;

	return S_FALSE;
}

HRESULT CRecentFileDlg::ClickedChoice(IDocumentBuilder* a_pBuilder)
{
	if (m_iActiveItem == -1)
	{
		return S_FALSE;
	}

	TCHAR szTmp[MAX_PATH] = _T("");
	m_wndList.GetItemText(m_iActiveItem, 1, szTmp, itemsof(szTmp));

	CComPtr<IInputManager> pInMgr;
	RWCoCreateInstance(pInMgr, __uuidof(InputManager));

	CComPtr<IDocument> pDoc;
	pInMgr->DocumentCreateEx(a_pBuilder, CStorageFilter(szTmp), NULL, &pDoc);

	if (pDoc)
	{
		m_pClbk->OpenDocument(pDoc);
		return S_OK;
	}

	return E_FAIL;
}

void CRecentFileDlg::OnFinalMessage(HWND UNREF(a_hWnd))
{
	Release();
}

LRESULT CRecentFileDlg::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	AddRef();
	m_iActiveItem = -1;

	// initialize toolbar
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));

		int nIconSize = XPGUI::GetSmallIconSize();
		m_cCtxMenu.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 3, 3);
		//HICON hIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_RECENT_REFRESH), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
		//m_cCtxMenu.AddIcon(hIcon);
		//DestroyIcon(hIcon);
		{
			static IRGridItem const grid1 = {0, 112};
			static IRGridItem const grid2 = {0, 144};
			static IRCanvas const canvas1 = {0, 0, 256, 256, 0, 1, NULL, &grid1};
			static IRCanvas const canvas2 = {0, 0, 256, 256, 0, 1, NULL, &grid2};
			static IRPathPoint const pts1[] =
			{
				{136, 112, 0, 0, 0, 0},
				{196, 52, 0, 0, 0, 0},
				{256, 112, 0, 0, 0, 0},
				{224, 112, 7, 71, 0, 0},
				{74, 227, 79, -25, 93, 33},
				{168, 112, 0, 0, 6, 52},
			};
			static IRPathPoint const pts2[] =
			{
				{120, 144, 0, 0, 0, 0},
				{60, 204, 0, 0, 0, 0},
				{0, 144, 0, 0, 0, 0},
				{32, 144, -7, -71, 0, 0},
				{182, 29, -79, 25, -93, -33},
				{88, 144, 0, 0, -6, -52},
			};
			CIconRendererReceiver cRenderer(nIconSize);
			cRenderer(&canvas1, itemsof(pts1), pts1, pSI->GetMaterial(ESMManipulate));
			cRenderer(&canvas2, itemsof(pts2), pts2, pSI->GetMaterial(ESMManipulate));
			HICON hIcon = cRenderer.get();
			m_cCtxMenu.AddIcon(hIcon);
			DestroyIcon(hIcon);
		}
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
			HICON hIcon = cRenderer.get();
			//hIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_RECENT_REMOVE), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			m_cCtxMenu.AddIcon(hIcon);
			DestroyIcon(hIcon);
		}
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.9f, -1, -1));
			pSI->GetLayers(ESIMagnifier, cRenderer, IRTarget(0.85f, 1, 1));
			HICON hIcon = cRenderer.get();
			m_cCtxMenu.AddIcon(hIcon);
			DestroyIcon(hIcon);
		}
	}

	m_wndList = GetDlgItem(IDC_LIST);
	m_wndList.SetBkColor(GetSysColor(COLOR_3DFACE));
	m_wndList.SetTextBkColor(GetSysColor(COLOR_3DFACE));

	// init thumbnails
	HDC hdc = GetDC();
	int nScale = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(hdc);
	m_szThumbnail.cx = (128/*64*/ * nScale + 48) / 96;
	m_szThumbnail.cy = (96/*48*/ * nScale + 48) / 96;
	m_cThumbnails.Create(m_szThumbnail.cx, m_szThumbnail.cy, XPGUI::GetImageListColorFlags(), 16, 1);

	if (XPGUI::IsVista() && CTheme::IsThemingSupported())
	{
		m_wndList.SetExtendedListViewStyle(LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT|LVS_EX_ONECLICKACTIVATE|LVS_EX_DOUBLEBUFFER|LVS_EX_JUSTIFYCOLUMNS);
		::SetWindowTheme(m_wndList, L"explorer", NULL);
	}
	else
	{
		m_wndList.SetExtendedListViewStyle(LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT|LVS_EX_ONECLICKACTIVATE|(XPGUI::IsXP() ? LVS_EX_DOUBLEBUFFER|LVS_EX_BORDERSELECT|LVS_EX_JUSTIFYCOLUMNS:0));
	}

	TCHAR szColumn[128] = _T("");
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = szColumn;
	lvc.cx = 220*nScale/96;
	lvc.iSubItem = 0;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RECENTFILES_COLNAME, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	m_wndList.InsertColumn(0, &lvc);
	lvc.cx = 500*nScale/96;
	lvc.iSubItem++;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RECENTFILES_COLPATH, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	m_wndList.InsertColumn(1, &lvc);
	lvc.iSubItem++;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RECENTFILES_COLTYPE, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 200*nScale/96;
	m_wndList.InsertColumn(2, &lvc);
	lvc.iSubItem++;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RECENTFILES_COLSIZE, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 100*nScale/96;
	m_wndList.InsertColumn(3, &lvc);
	lvc.iSubItem++;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RECENTFILES_COLDATEMOD, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 150*nScale/96;
	m_wndList.InsertColumn(4, &lvc);
	lvc.iSubItem++;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RECENTFILES_COLNOTE, szColumn, itemsof(szColumn), LANGIDFROMLCID(m_tLocaleID));
	lvc.cx = 100*nScale/96;
	m_wndList.InsertColumn(4, &lvc);

	CToolTipCtrl wndTT = m_wndList.GetToolTips();
	if (wndTT.m_hWnd)
		wndTT.SetDelayTime(TTDT_AUTOPOP, 2*wndTT.GetDelayTime(TTDT_AUTOPOP));

	CComPtr<IThumbnailRenderer> pRenderer;
	RWCoCreateInstance(pRenderer, __uuidof(ThumbnailRenderer));
	RWCoCreateInstance(m_pCache, __uuidof(ThumbnailCache));
	TCHAR szTmpPath[MAX_PATH+30];
	GetTempPath(MAX_PATH, szTmpPath);
	CComPtr<IApplicationInfo> pAI;
	RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
	CComBSTR bstrAppID;
	pAI->Identifier(&bstrAppID);
	bstrAppID += L"Thumbnails";
	CComBSTR bstrTmpPath(szTmpPath);
	bstrTmpPath += bstrAppID;
	CreateDirectory(COLE2CT(bstrTmpPath.m_str), NULL);
	m_pCache->Init(pRenderer, 0, 256, bstrTmpPath);
	RWCoCreateInstance(m_pThumbnails, __uuidof(AsyncThumbnailRenderer));
	m_pThumbnails->Init(m_pCache);
	CAutoVectorPtr<DWORD> cNoThumbnail(new DWORD[m_szThumbnail.cx*m_szThumbnail.cy]);
	pRenderer->GetThumbnail(NULL, m_szThumbnail.cx, m_szThumbnail.cy, cNoThumbnail, NULL, 0, NULL);
	HICON h = IconFromThumbnail(m_szThumbnail.cx, m_szThumbnail.cy, cNoThumbnail);
	m_cThumbnails.AddIcon(h);
	DestroyIcon(h);

	CComPtr<IInputManager> pIM;
	RWCoCreateInstance(pIM, __uuidof(InputManager));
	CComPtr<IEnumUnknowns> pDocTypes;
	if (pIM)
		pIM->DocumentTypesEnum(&pDocTypes);
	ULONG nDocTypes = 0;
	if (pDocTypes)
		pDocTypes->Size(&nDocTypes);

	m_wndList.SetImageList(m_cThumbnails, LVSIL_NORMAL);

	std::vector<std::tstring> cMRUList;
	RecentFiles::GetRecentFileList(m_pCfg, cMRUList);
	std::vector<std::tstring>::const_iterator i;
	ULONG nIcons = 0;
	int iList = 0;
	for (i = cMRUList.begin(); i != cMRUList.end(); ++i)
	{
		size_t iTmp = i->find_last_of(_T("\\/"));
		LPCTSTR pszName = iTmp == tstring::npos ? i->c_str() : i->c_str()+iTmp+1;
		size_t iTmp2 = i->find_last_of(_T('.'));
		LPCTSTR pszExt = iTmp2 == tstring::npos || iTmp2 < iTmp ? NULL : i->c_str()+iTmp2+1;

		CStorageFilter cFilter(i->c_str());
		IStorageFilter* pFilter = cFilter;
		if (pFilter == NULL)
			continue;
		//ObjectLock cLock(this);
		m_wndList.InsertItem(iList, pszName, 0);
		m_wndList.SetItemData(iList, reinterpret_cast<DWORD_PTR>(pFilter));
		pFilter->AddRef();
		m_wndList.SetItem(iList, 1, LVIF_TEXT, i->c_str(), -1, 0, 0, 0);
		bool bExtSet = false;
		if (pszExt)
		{
			for (ULONG j = 0; j < nDocTypes && !bExtSet; ++j)
			{
				CComPtr<IDocumentType> pDocType;
				pDocTypes->Get(j, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDocType));
				if (pDocType == NULL)
					continue;
				CComPtr<IEnumStrings> pExts;
				pDocType->SupportedExtensionsGet(&pExts);
				ULONG nExts = 0;
				if (pExts)
					pExts->Size(&nExts);
				for (ULONG k = 0; k < nExts && !bExtSet; ++k)
				{
					CComBSTR bstr;
					pExts->Get(k, &bstr);
					if (bstr == NULL)
						continue;
					CW2T str(bstr);
					if (_tcsicmp(str, pszExt) == 0)
					{
						CComPtr<ILocalizedString> pStr;
						pDocType->TypeNameGet(bstr, &pStr);
						if (pStr)
						{
							CComBSTR bstrDocType;
							pStr->GetLocalized(m_tLocaleID, &bstrDocType);
							m_wndList.SetItem(iList, 2, LVIF_TEXT, CW2T(bstrDocType), -1, 0, 0, 0);
							bExtSet = true;
						}
					}
				}
			}
		}
		if (!bExtSet)
			m_wndList.SetItem(iList, 2, LVIF_TEXT, _T("N/A"), -1, 0, 0, 0);
		m_wndList.SetItem(iList, 3, LVIF_TEXT, _T(""), -1, 0, 0, 0); // size
		m_wndList.SetItem(iList, 4, LVIF_TEXT, _T(""), -1, 0, 0, 0); // modified date
		m_wndList.SetItem(iList, 5, LVIF_TEXT, _T(""), -1, 0, 0, 0); // note
		m_pThumbnails->PrepareThumbnail(cFilter, m_tLocaleID, this);
		++iList;
	}
	if (iList > 0)
		m_wndList.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

	return 1;  // Let the system set the focus
}

LRESULT CRecentFileDlg::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_wndList.SetWindowPos(NULL, 0, 0, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam), SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	return 0;
}

LRESULT CRecentFileDlg::OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_wndList.m_hWnd == reinterpret_cast<HWND>(a_wParam))
	{
		int iSel = m_wndList.GetSelectedIndex();
		if (iSel < 0)
			return 0;

		int xPos = GET_X_LPARAM(a_lParam);
		int yPos = GET_Y_LPARAM(a_lParam);
		if (xPos == -1 || yPos == -1)
		{
			RECT rcTmp = {0, 0, 0, 0};
			m_wndList.GetItemRect(iSel, &rcTmp, LVIR_ICON);
			m_wndList.ClientToScreen(&rcTmp);
			xPos = rcTmp.right;
			yPos = rcTmp.top;
		}

		while (m_cCtxMenu.GetImageCount() > 3) m_cCtxMenu.Remove(m_cCtxMenu.GetImageCount()-1);
		Reset(m_cCtxMenu);

		CMenu cMenu;
		cMenu.CreatePopupMenu();

		TCHAR szCtxCmds[256] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_STARTVIEWBTN_RECENT, szCtxCmds, itemsof(szCtxCmds), LANGIDFROMLCID(m_tLocaleID));
		AddItem(ID_OPENFILE, szCtxCmds, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW|MFS_DEFAULT, ID_OPENFILE, LPCTSTR(NULL));

		cMenu.AppendMenu(MFT_SEPARATOR);

		CComPtr<IEnumUnknownsInit> pBuilders;
		RWCoCreateInstance(pBuilders, __uuidof(EnumUnknowns));
		GetBuilders(M_AppConfig(), pBuilders);
		ULONG nBuilders = 0;
		pBuilders->Size(&nBuilders);
		if (nBuilders > 1)
		{
			SIZE tSize = {16, 16};
			m_cCtxMenu.GetIconSize(tSize);
			CComBSTR bstrTempl;
			m_pSubmenuTemplate->GetLocalized(m_tLocaleID, &bstrTempl);
			for (ULONG i = 0; i < nBuilders; ++i)
			{
				CComPtr<IDocumentBuilder> pBuilder;
				pBuilders->Get(i, &pBuilder);
				if (pBuilder == NULL) continue;
				int iImage = -1;
				HICON hIc = 0;
				pBuilder->Icon(tSize.cx, &hIc);
				if (hIc)
				{
					iImage = m_cCtxMenu.AddIcon(hIc);
					DestroyIcon(hIc);
				}
				CComPtr<ILocalizedString> pName1;
				pBuilder->TypeName(&pName1);
				CComBSTR bstrName1;
				pName1->GetLocalized(m_tLocaleID, &bstrName1);
				OLECHAR szTmp[256] = L"";
				swprintf(szTmp, 256, bstrTempl.m_str, bstrName1);
				AddItem(ID_OPENFILEAS0+i, szTmp, iImage);
				cMenu.AppendMenu(MFT_OWNERDRAW, ID_OPENFILEAS0+i, LPCTSTR(NULL));
			}

			cMenu.AppendMenu(MFT_SEPARATOR);
		}

		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RECENTFILES_TOOLBAR, szCtxCmds, itemsof(szCtxCmds), LANGIDFROMLCID(m_tLocaleID));
		LPCTSTR pszRemove = szCtxCmds;
		for (size_t i = 0; szCtxCmds[i]; ++i) if (szCtxCmds[i] == _T('|')) { szCtxCmds[i] = _T('\0');  if (pszRemove == szCtxCmds) pszRemove = szCtxCmds+i+1; }
		AddItem(cMenu, ID_REFRESHTHUMBNAIL, szCtxCmds, 0);
		AddItem(cMenu, ID_REMOVERECENTFILE, pszRemove, 1);
		IStorageFilter* pLoc = reinterpret_cast<IStorageFilter*>(m_wndList.GetItemData(iSel));
		CComQIPtr<IStorageFilterBrowsable> pFS(pLoc);
		if (pFS)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(L"[0409]Open file location[0405]Otevřít ve složce", m_tLocaleID, &bstr);
			AddItem(cMenu, ID_OPENFOLDER, bstr.m_str, 2);
		}

		UINT nSelection = cMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, xPos, yPos, m_hWnd, NULL);
		if (nSelection != 0)
		{
			if (nSelection == ID_REFRESHTHUMBNAIL)
			{
				//ObjectLock cLock(this);
				IStorageFilter* pLoc = reinterpret_cast<IStorageFilter*>(m_wndList.GetItemData(iSel));
				if (m_pCache && pLoc)//&& szTmp[0])
				{
					m_wndList.SetItem(iSel, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
					m_pCache->Remove(pLoc, m_szThumbnail.cx, m_szThumbnail.cy);
					m_pThumbnails->PrepareThumbnail(pLoc, m_tLocaleID, this);
				}
			}
			else if (nSelection == ID_REMOVERECENTFILE)
			{
				//ObjectLock cLock(this);
				int nCount = m_wndList.GetItemCount();
				if (iSel >= 0 && iSel < nCount)
				{
					RecentFiles::RemoveRecentFile(m_pCfg, iSel);
					IStorageFilter* pLoc = reinterpret_cast<IStorageFilter*>(m_wndList.GetItemData(iSel));
					m_wndList.DeleteItem(iSel);
					if (pLoc)
						pLoc->Release();
					if (nCount > 1)
						m_wndList.SetItemState(iSel < nCount-1 ? iSel : iSel-1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
				}
			}
			else if (nSelection == ID_OPENFOLDER)
			{
				pFS->OpenInFolder();
			}
			else if (nSelection == ID_OPENFILE)
			{
				m_iActiveItem = iSel;
				m_pClbk->OnOKEx();
			}
			else if (nSelection >= ID_OPENFILEAS0 && nSelection < ID_OPENFILEAS0+nBuilders)
			{
				m_iActiveItem = iSel;
				CComPtr<IDocumentBuilder> pBuilder;
				pBuilders->Get(nSelection-ID_OPENFILEAS0, &pBuilder);
				ClickedChoice(pBuilder);
			}
		}
	}
	return 0;
}

LRESULT CRecentFileDlg::OnItemClicked(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	m_bIgnoreNext = false;
	return OnItemActivateList(a_idCtrl, a_pNMHDR, a_bHandled);
}

LRESULT CRecentFileDlg::OnItemActivateList(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(a_pNMHDR);

	m_iActiveItem = pNMIA->iItem;

	if (m_bIgnoreNext)
		m_bIgnoreNext = false;
	else
		m_pClbk->OnOKEx();

	return 0;
}

LRESULT CRecentFileDlg::OnItemChangedList(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
{
	int iSel = m_wndList.GetSelectedIndex();
	if (m_iActiveItem != iSel)
	{
		m_iActiveItem = iSel;
		//m_pClbk->SetDefaultButtonState(NULL, m_iActiveItem != -1, M_OpenAsTempl());
	}

	return 0;
}

LRESULT CRecentFileDlg::OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(a_pNMHDR);
	TCHAR szTmp[MAX_PATH+64+64+32+256] = _T("");
	int nLen = m_wndList.GetItemText(pGetInfoTip->iItem, 1, szTmp, itemsof(szTmp));

	if (m_wndList.GetItemText(pGetInfoTip->iItem, 2, szTmp+nLen+1, itemsof(szTmp)-nLen-1))
	{
		szTmp[nLen] = _T('\n');
	}
	nLen = _tcslen(szTmp);
	szTmp[nLen++] = _T(',');
	szTmp[nLen++] = _T(' ');
	szTmp[nLen] = _T('\0');
	m_wndList.GetItemText(pGetInfoTip->iItem, 3, szTmp+nLen, itemsof(szTmp)-nLen-1);
	nLen = _tcslen(szTmp);
	if (m_wndList.GetItemText(pGetInfoTip->iItem, 4, szTmp+nLen+1, itemsof(szTmp)-nLen-1))
	{
		szTmp[nLen] = _T('\n');
	}
	nLen = _tcslen(szTmp);
	if (m_wndList.GetItemText(pGetInfoTip->iItem, 5, szTmp+nLen+1, itemsof(szTmp)-nLen-1))
	{
		szTmp[nLen] = _T('\n');
	}
	_tcsncpy(pGetInfoTip->pszText, szTmp, pGetInfoTip->cchTextMax);
	pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');

	return 0;
}

LRESULT CRecentFileDlg::OnGetEmptyMarkup(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	NMLVEMPTYMARKUP* markupInfo = reinterpret_cast<NMLVEMPTYMARKUP*>(a_pNMHDR);
	markupInfo->dwFlags = EMF_CENTERED;
	Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_NORECENTFILES, markupInfo->szMarkup, itemsof(markupInfo->szMarkup), LANGIDFROMLCID(m_tLocaleID));
	return TRUE; // set the markup
}

HICON CRecentFileDlg::IconFromThumbnail(ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData)
{
	ULONG nMaskLineSize = (((a_nSizeX+7)>>3)+3)&~3;
	CAutoVectorPtr<BYTE> pIconRes(new BYTE[sizeof(BITMAPINFOHEADER)+a_nSizeX*a_nSizeY*4+nMaskLineSize*a_nSizeY]);
	BITMAPINFOHEADER* pBIH = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
	pBIH->biSize = sizeof*pBIH;
	pBIH->biWidth = a_nSizeX;
	pBIH->biHeight = a_nSizeY<<1;
	pBIH->biPlanes = 1;
	pBIH->biBitCount = 32;
	pBIH->biCompression = BI_RGB;
	pBIH->biSizeImage = a_nSizeX*a_nSizeY*4+nMaskLineSize*a_nSizeY;
	pBIH->biXPelsPerMeter = 0x8000;
	pBIH->biYPelsPerMeter = 0x8000;
	pBIH->biClrUsed = 0;
	pBIH->biClrImportant = 0;
	DWORD* pXOR = reinterpret_cast<DWORD*>(pBIH+1);
	for (ULONG y = 0; y < a_nSizeY; ++y)
		CopyMemory(pXOR+a_nSizeX*(a_nSizeY-y-1), a_pRGBAData+a_nSizeX*y, a_nSizeX*4);
	BYTE* pAND = reinterpret_cast<BYTE*>(pXOR+a_nSizeX*a_nSizeY);
	// create mask
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		BYTE* pA = pAND+nMaskLineSize*y;
		DWORD* pC = pXOR+a_nSizeX*y;
		for (ULONG x = 0; x < a_nSizeX; ++x, ++pC)
		{
			BYTE* p = pA+(x>>3);
			if (*pC&0xff000000)
				*p &= ~(0x80 >> (x&7));
			else
				*p |= 0x80 >> (x&7);
		}
	}

	return CreateIconFromResourceEx(pIconRes, sizeof(BITMAPINFOHEADER)+a_nSizeX*a_nSizeY*4+nMaskLineSize*a_nSizeY, TRUE, 0x00030000, a_nSizeX, a_nSizeY, LR_DEFAULTCOLOR);
}

STDMETHODIMP CRecentFileDlg::SetThumbnail(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData, RECT const* UNREF(a_prcBounds), BSTR a_bstrInfo)
{
	try
	{
		if (m_bDestroyed || a_nSizeX != m_szThumbnail.cx || a_nSizeY != m_szThumbnail.cy)
			return E_FAIL;

		std::pair<HICON, BSTR> t(IconFromThumbnail(a_nSizeX, a_nSizeY, a_pRGBAData), a_bstrInfo);
		{
			//ObjectLock cLock(this);
			if (!m_bDestroyed)
				CWindow::SendMessage(WM_THUMBNAILREADY, reinterpret_cast<WPARAM>(&t), reinterpret_cast<LPARAM>(a_pFile));
		}
		DestroyIcon(t.first);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CRecentFileDlg::OnThumbnailReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HICON hIcon = reinterpret_cast<std::pair<HICON, BSTR>*>(wParam)->first;
	BSTR bstrInfo = reinterpret_cast<std::pair<HICON, BSTR>*>(wParam)->second;
	IStorageFilter* pFilter = reinterpret_cast<IStorageFilter*>(lParam);
	{
		//ObjectLock cLock(this);
		int iItem = IndexFromPath(pFilter);
		if (iItem >= 0)
		{
			int iImage = m_cThumbnails.AddIcon(hIcon);
			m_wndList.SetItem(iItem, 0, LVIF_IMAGE, NULL, iImage, 0, 0, 0);
			CComPtr<IDataSrcDirect> pSrc;
			pFilter->SrcOpen(&pSrc);
			if (pSrc)
			{
				ULONG nSize = 0;
				pSrc->SizeGet(&nSize);
				TCHAR szTmp[64];
				TCHAR szTempl[32] = _T("");
				if (nSize < 1024)
				{
					Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_FILELEN_BYTES, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
					_stprintf(szTmp, szTempl, nSize);
				}
				else if (nSize < 1024*1024)
				{
					Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_FILELEN_KBYTES, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
					_stprintf(szTmp, szTempl, 0.01f*int(nSize*100.0f/1024.0f));
				}
				else
				{
					Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_FILELEN_MBYTES, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
					_stprintf(szTmp, szTempl, 0.01f*int(nSize*100.0f/(1024.0f*1024.0f)));
				}
				m_wndList.SetItem(iItem, 3, LVIF_TEXT, szTmp, -1, 0, 0, 0);
			}
			CComQIPtr<IStorageLocatorAttrs> pAttrs(pFilter);
			if (pAttrs)
			{
				ULONGLONG nTime = 0;
				if (SUCCEEDED(pAttrs->GetTime(ESTTModification, &nTime)))
				{
					FILETIME tTime = *reinterpret_cast<FILETIME*>(&nTime);
					SYSTEMTIME tSysTime;
					FileTimeToSystemTime(&tTime, &tSysTime);
					TCHAR szDate[128];
					if (GetDateFormat(m_tLocaleID, DATE_LONGDATE, &tSysTime, NULL, szDate, itemsof(szDate)) ||
						GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &tSysTime, NULL, szDate, itemsof(szDate)))
					{
						TCHAR szTmp[192];
						TCHAR szTempl[64] = _T("");
						Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_FILEDATE_MODIFIED, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
						_stprintf(szTmp, szTempl, szDate);
						m_wndList.SetItem(iItem, 4, LVIF_TEXT, szTmp, -1, 0, 0, 0); // modified date
					}
				}
			}
			if (bstrInfo && *bstrInfo)
			{
				m_wndList.SetItem(iItem, 5, LVIF_TEXT, COLE2CT(bstrInfo), -1, 0, 0, 0); // note
			}
		}
	}
	return 0;
}

int CRecentFileDlg::IndexFromPath(IStorageFilter* a_pFilter)
{
	int nItems = m_wndList.GetItemCount();
	for (int i = 0; i < nItems; ++i)
	{
		if (reinterpret_cast<IStorageFilter*>(m_wndList.GetItemData(i)) == a_pFilter)
			return i;
	}
	return -1;
}

