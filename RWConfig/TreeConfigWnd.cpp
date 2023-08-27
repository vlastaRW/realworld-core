// TreeConfigWnd.cpp : Implementation of CTreeConfigWnd

#include "stdafx.h"
#include "TreeConfigWnd.h"

#include <Win32LangEx.h>
#include <XPGUI.h>
#include <StringParsing.h>
#include <IconRenderer.h>


// CTreeConfigWnd

float CTreeConfigWnd::s_fInitialTreeSize = 25.0f;

void CTreeConfigWnd::OwnerNotify(TCookie a_tCookie, IUnknown* a_pIDs)
{
	RefreshItemIDs();
}

STDMETHODIMP CTreeConfigWnd::TopWindowSet(BOOL a_bIsTopWindow, DWORD a_clrBackground)
{
	return S_FALSE;
}

STDMETHODIMP CTreeConfigWnd::Create(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode)
{
	m_tLocaleID = a_tLocaleID;
	m_fTreeSize = s_fInitialTreeSize;
	static INITCOMMONCONTROLSEX tICCEx = { sizeof(INITCOMMONCONTROLSEX), ICC_TREEVIEW_CLASSES|ICC_STANDARD_CLASSES };
	static BOOL bDummy = InitCommonControlsEx(&tICCEx);
	Win32LangEx::CLangDialogImpl<CTreeConfigWnd>::Create(reinterpret_cast<HWND>(a_hParent));
	SetWindowLong(GWL_ID, a_nCtlID);
	MoveWindow(a_prcPositon);
	RefreshItemIDs();
	ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
	return S_OK;
}

STDMETHODIMP CTreeConfigWnd::ConfigSet(IConfig* a_pConfig, EConfigPanelMode a_eMode)
{
	m_eMode = a_eMode;
	if (m_pConfig != a_pConfig)
	{
		if (m_pConfig != NULL)
		{
			m_pConfig->ObserverDel(ObserverGet(), 0);
		}
		m_pConfig = a_pConfig;
		while (!m_cBack.empty()) m_cBack.pop();
		while (!m_cForward.empty()) m_cForward.pop();
		m_wndToolbar.EnableButton(ID_TREE_BACK, FALSE);
		m_wndToolbar.EnableButton(ID_TREE_FORWARD, FALSE);
		m_wndToolbar.EnableButton(ID_TREE_ROOT, FALSE);
		m_pCurCfg = m_pConfig;
		RefreshItemIDs();
		if (a_pConfig != NULL)
		{
			m_pConfig->ObserverIns(ObserverGet(), 0);
		}
	}
	return S_OK;
}

LRESULT CTreeConfigWnd::OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_nClipboardFormat = RegisterClipboardFormat(_T("RWCONFIG"));

	m_wndTree = GetDlgItem(IDC_TCF_TREE);
	int nIconSize = XPGUI::GetSmallIconSize();
	int nIconDelta = (nIconSize>>1)-8;
	m_cImageList.Create(nIconSize+nIconDelta, nIconSize, XPGUI::GetImageListColorFlags(), 6, 1);
	RECT padding = {nIconDelta>>1, 0, nIconDelta-(nIconDelta>>1), 0};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	UINT aIconIDs[] = {IDI_CUSTOMGUI};//, IDI_TREE_ROOT, IDI_TREE_BACK, IDI_TREE_FORWARD, IDI_TREE_COPY, IDI_TREE_PASTE};
	for (size_t i = 0; i < itemsof(aIconIDs); ++i)
	{
		HICON h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(aIconIDs[i]), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
		m_cImageList.AddIcon(h);
		DestroyIcon(h);
	}
	{
		static IRGridItem const gridx[] = { {EGIFInteger, 32.0f}, {EGIFInteger, 64.0f}, {EGIFInteger, 96.0f}, {EGIFInteger, 112.0f}, {EGIFInteger, 142.0f}, {EGIFInteger, 192.0f}, {EGIFInteger, 224.0f} };
		static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 16.0f}, {EGIFInteger, 128.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 208.0f}, {EGIFInteger, 256.0f} };
		static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

		static IRPolyPoint const wall[] = { {32, 112}, {224, 112}, {224, 256}, {32, 256} };
		static IRPolyPoint const chmn[] = { {64, 16}, {96, 16}, {96, 76}, {64, 76} };
		static IRPolyPoint const roof[] = { {256, 128}, {0, 128}, {128, 0} };
		static IRPolyPoint const door[] = { {142, 160}, {192, 160}, {192, 256}, {142, 256} };
		static IRPolyPoint const wndw[] = { {64, 160}, {112, 160}, {112, 208}, {64, 208} };

		CIconRendererReceiver cRenderer(nIconSize);
		cRenderer(&canvas, itemsof(wall), wall, pSI->GetMaterial(ESMAltBackground));
		cRenderer(&canvas, itemsof(chmn), chmn, pSI->GetMaterial(ESMBackground));
		cRenderer(&canvas, itemsof(roof), roof, pSI->GetMaterial(ESMDelete));
		cRenderer(&canvas, itemsof(door), door, pSI->GetMaterial(ESMBackground));
		cRenderer(&canvas, itemsof(wndw), wndw, pSI->GetMaterial(ESMBackground));
		HICON hIcon = cRenderer.get(padding);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIDirectionLeft, cRenderer, IRTarget(0.75f));
		HICON hIcon = cRenderer.get(padding);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		CIconRendererReceiver cRenderer(nIconSize);
		pSI->GetLayers(ESIDirectionRight, cRenderer, IRTarget(0.75f));
		HICON hIcon = cRenderer.get(padding);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		static IRPolyPoint const poly[] = {{0, 160}, {0, 256}, {256, 256}, {256, 160}, {176, 160}, {176, 99}, {227, 99}, {128, 0}, {29, 99}, {80, 99}, {80, 160}};
		static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
		static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 99.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
		static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

		CIconRendererReceiver cRenderer(nIconSize);
		cRenderer(&canvas, itemsof(poly), poly, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
		HICON hIcon = cRenderer.get(padding);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	{
		static IRPolyPoint const poly0[] = {{0, 160}, {0, 256}, {256, 256}, {256, 160}};
		static IRPolyPoint const poly1[] = {{80, 0}, {80, 114}, {29, 114}, {128, 213}, {227, 114}, {176, 114}, {176, 0}};
		static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
		static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 114.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
		static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

		CIconRendererReceiver cRenderer(nIconSize);
		cRenderer(&canvas, itemsof(poly0), poly0, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
		cRenderer(&canvas, itemsof(poly1), poly1, pSI->GetMaterial(ESMManipulate), IRTarget(0.8f));
		HICON hIcon = cRenderer.get(padding);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
	}
	m_wndTree.SetImageList(m_cImageList, TVSIL_NORMAL);

	m_wndToolbar = GetDlgItem(IDC_TCF_TOOLBAR);
	m_wndToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_wndToolbar.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndToolbar.SetImageList(m_cImageList);
	TCHAR szTooltipStrings[1024] = _T("");
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_TFC_BUTTONS, szTooltipStrings, itemsof(szTooltipStrings), LANGIDFROMLCID(m_tLocaleID));
	for (TCHAR* p = szTooltipStrings; *p; ++p)
		if (*p == _T('|')) *p = _T('\0');
	m_wndToolbar.AddStrings(szTooltipStrings);
	TBBUTTON atButtons[] =
	{
		{1, ID_TREE_ROOT, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
		{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
		{2, ID_TREE_BACK, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 1},
		{3, ID_TREE_FORWARD, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
		{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
		{4, ID_TREE_COPY, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 3},
		{5, ID_TREE_PASTE, IsClipboardFormatAvailable(m_nClipboardFormat) ? TBSTATE_ENABLED : 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 4}
	};
	m_wndToolbar.AddButtons(itemsof(atButtons), atButtons);
	m_wndToolbar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);
	SIZE nSize = {0, 0};
	m_wndToolbar.GetMaxSize(&nSize);
	m_nTBHeight = nSize.cy;

	CComObject<CAutoConfigWnd>::CreateInstance(&m_pConfigWnd);
	m_pConfigWndRef = m_pConfigWnd;

	RECT rc;
	GetClientRect(&rc);
	m_pConfigWnd->Create(m_hWnd, &rc, IDC_TCF_TREE+1, m_tLocaleID, TRUE, ECWBMNothing);
	m_pConfigWnd->ConfigSet(m_pConfig, m_eMode);
	m_pCurCfg = m_pConfig;
	ResizeSubWindows(rc.right, rc.bottom);

	TCHAR szTmp[64] = _T("");
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_WHOLECONFIG, szTmp, itemsof(szTmp), m_tLocaleID);
	CComQIPtr<IConfigCustomGUI> pCustGUI(m_pConfig);
	m_hRootItem = m_wndTree.InsertItem(TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE, szTmp, pCustGUI ? 0 : I_IMAGENONE, pCustGUI ? 0 : I_IMAGENONE, 0, 0, NULL, TVI_ROOT, TVI_LAST);

	m_hWndNextViewer = SetClipboardViewer();

	AddRef();
	return 0;
}

void CTreeConfigWnd::OnFinalMessage(HWND a_hWnd)
{
	Release();
}

LRESULT CTreeConfigWnd::OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	ChangeClipboardChain(m_hWndNextViewer);

	s_fInitialTreeSize = m_fTreeSize;
	ClearTree();
	//if (m_pValueCtl)
	//{
	//	m_pValueCtl->Delete();
	//	m_pValueCtl = NULL;
	//}

	a_bHandled = FALSE;
	return 0;
}

LRESULT CTreeConfigWnd::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	ResizeSubWindows(rc.right, rc.bottom);
	return 0;
}

void CTreeConfigWnd::ResizeSubWindows(int a_nSizeX, int a_nSizeY)
{
	RECT rc = {0, m_nTBHeight, (a_nSizeX-GAP)*m_fTreeSize*0.01f+0.5f, a_nSizeY};
	m_wndTree.MoveWindow(&rc);
	rc.top = 0;
	rc.bottom = m_nTBHeight;
	m_wndToolbar.MoveWindow(&rc);
	rc.bottom = a_nSizeY;
	if (m_pConfigWnd != NULL)
	{
		rc.left = rc.right+GAP;
		rc.right = a_nSizeX;
		m_pConfigWnd->Move(&rc);
	}
}

LRESULT CTreeConfigWnd::OnSelectionChanged(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	if (!m_bUpdating)
	{
		NM_TREEVIEW* pNMTreeView = reinterpret_cast<NM_TREEVIEW*>(a_pNMHeader);
		if (m_pConfigWnd != NULL)
		{
			IConfig* pNew = pNMTreeView->itemNew.lParam == NULL ? m_pConfig.p : reinterpret_cast<IConfig*>(pNMTreeView->itemNew.lParam);
			if (pNew != m_pCurCfg)
			{
				while (!m_cForward.empty()) m_cForward.pop();
				m_wndToolbar.EnableButton(ID_TREE_BACK, TRUE);
				m_wndToolbar.EnableButton(ID_TREE_FORWARD, FALSE);
				m_cBack.push(m_pCurCfg);
				m_pCurCfg = pNew;
				m_wndToolbar.EnableButton(ID_TREE_ROOT, m_pCurCfg != m_pConfig);
				m_pConfigWnd->ConfigSet(pNew, m_eMode);
			}
		}
	}
	return 0;
}

LRESULT CTreeConfigWnd::OnCommandRoot(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_wndTree.SetFocus();

	if (!m_bUpdating)
	{
		if (m_pConfigWnd && m_pConfig != m_pCurCfg)
		{
			m_bUpdating = true;
			while (!m_cForward.empty()) m_cForward.pop();
			m_wndToolbar.EnableButton(ID_TREE_BACK, TRUE);
			m_wndToolbar.EnableButton(ID_TREE_FORWARD, FALSE);
			m_cBack.push(m_pCurCfg);
			m_pCurCfg = m_pConfig.p;
			m_wndToolbar.EnableButton(ID_TREE_ROOT, FALSE);
			m_pConfigWnd->ConfigSet(m_pConfig.p, m_eMode);
			SelectTreeItem(TVI_ROOT, 0);
			m_bUpdating = false;
		}
	}
	return 0;
}

LRESULT CTreeConfigWnd::OnCommandBack(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_wndTree.SetFocus();

	if (!m_bUpdating)
	{
		if (m_pConfigWnd && !m_cBack.empty())
		{
			m_bUpdating = true;
			while (!m_cBack.empty())
			{
				if (SelectTreeItem(TVI_ROOT, m_cBack.top() == m_pConfig ? 0 : reinterpret_cast<INT_PTR>(m_cBack.top().p)))
				{
					m_cForward.push(m_pCurCfg);
					m_pCurCfg = m_cBack.top();
					m_cBack.pop();
					break;
				}
				m_cBack.pop();
			}
			m_wndToolbar.EnableButton(ID_TREE_BACK, !m_cBack.empty());
			m_wndToolbar.EnableButton(ID_TREE_FORWARD, !m_cForward.empty());
			m_wndToolbar.EnableButton(ID_TREE_ROOT, m_pCurCfg != m_pConfig);
			m_pConfigWnd->ConfigSet(m_pCurCfg, m_eMode);
			m_bUpdating = false;
		}
	}
	return 0;
}

LRESULT CTreeConfigWnd::OnCommandForward(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_wndTree.SetFocus();

	if (!m_bUpdating)
	{
		if (m_pConfigWnd && !m_cForward.empty())
		{
			m_bUpdating = true;
			while (!m_cForward.empty())
			{
				if (SelectTreeItem(TVI_ROOT, m_cForward.top() == m_pConfig ? 0 : reinterpret_cast<INT_PTR>(m_cForward.top().p)))
				{
					m_cBack.push(m_pCurCfg);
					m_pCurCfg = m_cForward.top();
					m_cForward.pop();
					break;
				}
				m_cForward.pop();
			}
			m_wndToolbar.EnableButton(ID_TREE_BACK, !m_cBack.empty());
			m_wndToolbar.EnableButton(ID_TREE_FORWARD, !m_cForward.empty());
			m_wndToolbar.EnableButton(ID_TREE_ROOT, m_pCurCfg != m_pConfig);
			m_pConfigWnd->ConfigSet(m_pCurCfg, m_eMode);
			m_bUpdating = false;
		}
	}
	return 0;
}

static void DeleteItems(CTreeViewCtrl& a_wndTree, HTREEITEM a_hRoot);

static void DeleteItem(CTreeViewCtrl& a_wndTree, HTREEITEM a_hItem)
{
	DeleteItems(a_wndTree, a_hItem);
	IConfig* pCfg = reinterpret_cast<IConfig*>(a_wndTree.GetItemData(a_hItem));
	if (pCfg != NULL)
		pCfg->Release();
	a_wndTree.DeleteItem(a_hItem);
}

static void DeleteItems(CTreeViewCtrl& a_wndTree, HTREEITEM a_hRoot)
{
	for (HTREEITEM hItem = a_wndTree.GetChildItem(a_hRoot); hItem != NULL; )
	{
		HTREEITEM hThis = hItem;
		hItem = a_wndTree.GetNextSiblingItem(hItem);
		DeleteItem(a_wndTree, hThis);
	}
}

static void DeleteSiblingsFrom(CTreeViewCtrl& a_wndTree, HTREEITEM a_hFirst)
{
	for (HTREEITEM hItem = a_hFirst; hItem != NULL; )
	{
		HTREEITEM hThis = hItem;
		hItem = a_wndTree.GetNextSiblingItem(hItem);
		DeleteItem(a_wndTree, hThis);
	}
}

void CTreeConfigWnd::GetItemText(IConfig* a_pConfig, size_t nPrefix, BSTR a_bstrID, LCID a_tLocaleID, std::tstring& a_strText)
{
	CComPtr<IConfigItem> pUIInfo;
	a_pConfig->ItemGetUIInfo(a_bstrID, __uuidof(IConfigItem), reinterpret_cast<void**>(&pUIInfo));
	CComPtr<ILocalizedString> pName;
	if (pUIInfo != NULL)
		pUIInfo->NameGet(&pName, NULL);
	CComBSTR bstrName;
	if (pName != NULL)
		pName->GetLocalized(a_tLocaleID, &bstrName);
	if (bstrName == NULL)
		bstrName = a_bstrID+nPrefix;
	CConfigValue cVal;
	a_pConfig->ItemValueGet(a_bstrID, &cVal);
	if (cVal.TypeGet() == ECVTEmpty)
	{
		a_strText = COLE2CT(bstrName);
		return;
	}
	if (pUIInfo != NULL)
	{
		CComPtr<ILocalizedString> pValName;
		pUIInfo->ValueGetName(cVal, &pValName);
		CComBSTR bstrValText;
		if (pValName != NULL)
			pValName->GetLocalized(a_tLocaleID, &bstrValText);
		if (bstrValText != NULL)
		{
			a_strText = COLE2CT(bstrName);
			a_strText += _T(": ");
			a_strText += COLE2CT(bstrValText);
			return;
		}
	}
	a_strText = COLE2CT(bstrName);
	a_strText += _T(": ");
	switch (cVal.TypeGet())
	{
	case ECVTBool:
		a_strText += cVal.operator bool() ? _T("true") : _T("false");
		break;
	case ECVTFloat:
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), cVal.operator float());
			a_strText += szTmp;
		}
		break;
	case ECVTGUID:
		{
			TCHAR szTmp[40] = _T("");
			StringFromGUID(cVal.operator const GUID &(), szTmp);
			a_strText += szTmp;
		}
		break;
	case ECVTInteger:
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%i"), cVal.operator LONG());
			a_strText += szTmp;
		}
		break;
	case ECVTString:
		a_strText += COLE2CT(cVal.operator BSTR());
		break;
	}
}

void CTreeConfigWnd::InsertSubItems(IConfig* a_pConfig, CTreeViewCtrl& a_wndTree, IEnumStrings* a_pItemIDs, int& a_nNextItem, HTREEITEM a_hParent, LPCOLESTR a_pszPrefix, LCID a_tLocaleID)
{
	size_t const nPrefix = wcslen(a_pszPrefix);
	HTREEITEM hOldItem = a_wndTree.GetChildItem(a_hParent);
	HTREEITEM hInsertAfter = TVI_FIRST;

	while (1)
	{
		CComBSTR bstrID;
		a_pItemIDs->Get(a_nNextItem, &bstrID);
		if (bstrID == NULL || bstrID[0] == L'\0')
			break;
		if (wcsncmp(bstrID, a_pszPrefix, nPrefix) != 0)
			break;
		++a_nNextItem;
		size_t i;
		for (i = nPrefix; bstrID[i]; ++i)
			if (bstrID[i] == L'\\')
				break;
		if (bstrID[i])
			continue;
		CComPtr<IConfig> pConfig;
		a_pConfig->SubConfigGet(bstrID, &pConfig);
		if (pConfig == NULL)
			continue;

		std::tstring str;
		GetItemText(a_pConfig, nPrefix, bstrID, a_tLocaleID, str);

		HTREEITEM hNewItem = NULL;
		HTREEITEM hTmp = hOldItem;
		while (hTmp != NULL)
		{
			IConfig* pCfg = reinterpret_cast<IConfig*>(a_wndTree.GetItemData(hTmp));
			if (pCfg == pConfig.p)
				break;
			hTmp = a_wndTree.GetNextSiblingItem(hTmp);
		}
		if (hTmp != NULL)
		{
			HTREEITEM hDel = hOldItem;
			while (hDel != hTmp && hDel != NULL)
			{
				HTREEITEM hThis = hDel;
				hDel = a_wndTree.GetNextSiblingItem(hDel);
				DeleteItem(a_wndTree, hThis);
			}
			CComQIPtr<IConfigCustomGUI> pCustGUI(pConfig);
			a_wndTree.SetItem(hTmp, TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE, str.c_str(), pCustGUI ? 0 : I_IMAGENONE, pCustGUI ? 0 : I_IMAGENONE, 0, 0, NULL);
			hNewItem = hTmp;
			hOldItem = a_wndTree.GetNextSiblingItem(hTmp);
		}
		else
		{
			CComQIPtr<IConfigCustomGUI> pCustGUI(pConfig);
			hNewItem = a_wndTree.InsertItem(TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE, str.c_str(), pCustGUI ? 0 : I_IMAGENONE, pCustGUI ? 0 : I_IMAGENONE, 0, 0, reinterpret_cast<LPARAM>(pConfig.p), a_hParent, hInsertAfter);
			pConfig.Detach();
		}
		hInsertAfter = hNewItem;
		bstrID += L"\\";
		InsertSubItems(a_pConfig, a_wndTree, a_pItemIDs, a_nNextItem, hNewItem, bstrID, a_tLocaleID);
	}

	DeleteSiblingsFrom(a_wndTree, hOldItem);
}

void CTreeConfigWnd::ClearTree()
{
	m_bUpdating = true;
	DeleteItems(m_wndTree, TVI_ROOT);
	m_bUpdating = false;
}

bool CTreeConfigWnd::RefreshItemIDs()
{
	if (!IsWindow() || !m_wndTree.IsWindow())
		return false;

	if (m_pConfig == NULL)
	{
		m_bUpdating = true;
		bool bIsVisible = (m_wndTree.GetWindowLong(GWL_STYLE) & WS_VISIBLE) != 0;
		if (bIsVisible)
			m_wndTree.SetRedraw(FALSE);
		DeleteItems(m_wndTree, m_hRootItem);
		m_wndTree.SetItem(m_hRootItem, TVIF_IMAGE|TVIF_SELECTEDIMAGE, NULL, I_IMAGENONE, I_IMAGENONE, 0, 0, NULL);
		m_bstrSelected.Empty();
		m_bUpdating = false;
		if (bIsVisible)
			m_wndTree.SetRedraw(TRUE);
		if (m_pConfigWnd != NULL)
			m_pConfigWnd->ConfigSet(NULL, m_eMode);
		return false;
	}

	CComPtr<IEnumStrings> m_pItemIDs;
	//m_pItemIDs = NULL;
	m_pConfig->ItemIDsEnum(&m_pItemIDs);

	m_bUpdating = true;
	bool bIsVisible = (m_wndTree.GetWindowLong(GWL_STYLE) & WS_VISIBLE) != 0;
	if (bIsVisible)
		m_wndTree.SetRedraw(FALSE);

	// update root icon
	CComQIPtr<IConfigCustomGUI> pCustGUI(m_pConfig);
	m_wndTree.SetItem(m_hRootItem, TVIF_IMAGE|TVIF_SELECTEDIMAGE, NULL, pCustGUI ? 0 : I_IMAGENONE, pCustGUI ? 0 : I_IMAGENONE, 0, 0, NULL);

	int nNextItem = 0;
	InsertSubItems(m_pConfig, m_wndTree, m_pItemIDs, nNextItem, m_hRootItem, L"", m_tLocaleID);

	m_bUpdating = false;
	if (bIsVisible)
		m_wndTree.SetRedraw(TRUE);

	HTREEITEM hSel = m_wndTree.GetSelectedItem();
	if (hSel == NULL)
	{
		m_wndTree.Select(m_hRootItem, TVGN_CARET);
		m_pCurCfg = m_pConfig;
		m_pConfigWnd->ConfigSet(m_pCurCfg, m_eMode);
	}
	else
	{
		IConfig* pCfg = reinterpret_cast<IConfig*>(m_wndTree.GetItemData(hSel));
		if (m_pConfigWnd != NULL)
		{
			m_pCurCfg = pCfg == NULL ? m_pConfig : pCfg;
			m_pConfigWnd->ConfigSet(m_pCurCfg, m_eMode);
		}
	}

	return true;
}

bool CTreeConfigWnd::SelectTreeItem(HTREEITEM a_hRoot, INT_PTR a_xItemData)
{
	HTREEITEM h = m_wndTree.GetChildItem(a_hRoot);
	while (h)
	{
		if (m_wndTree.GetItemData(h) == a_xItemData)
		{
			m_wndTree.SelectItem(h);
			m_wndTree.EnsureVisible(h);
			return true;
		}
		if (SelectTreeItem(h, a_xItemData))
			return true;
		h = m_wndTree.GetNextSiblingItem(h);
	}
	return false;
}

LRESULT CTreeConfigWnd::OnSetCursor(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	POINT tPos;
	GetCursorPos(&tPos);
	ScreenToClient(&tPos);
	int nTreeEdge = static_cast<int>((rc.right-GAP)*m_fTreeSize*0.01f+0.5f);
	if (tPos.x >= nTreeEdge && tPos.x < (nTreeEdge+GAP))
	{
		if (m_hResizeCursor == NULL)
			m_hResizeCursor = LoadCursor(NULL, IDC_SIZEWE);
		SetCursor(m_hResizeCursor);
		return TRUE;
	}

	a_bHandled = FALSE;
	return FALSE;
}

LRESULT CTreeConfigWnd::OnLButtonDown(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	SetCapture();

	RECT rc;
	GetClientRect(&rc);
	m_nResizeDelta = GET_X_LPARAM(a_lParam)-static_cast<int>((rc.right-GAP)*m_fTreeSize*0.01f+0.5f);

	return 0;
}

LRESULT CTreeConfigWnd::OnLButtonUp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	ReleaseCapture();

	return 0;
}

LRESULT CTreeConfigWnd::OnMouseMove(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (GetCapture() == m_hWnd)
	{
		RECT rc;
		GetClientRect(&rc);
		if (rc.right <= GAP)
			return 0;

		int nPrevious = static_cast<int>((rc.right-GAP)*m_fTreeSize*0.01f+0.5f);

		bool bChanged = false;
		POINT tMousePos = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		int nNew = tMousePos.x-m_nResizeDelta;
		if (nPrevious == nNew)
			return 0;

		m_fTreeSize = 100.0f*static_cast<float>(nNew)/(rc.right-GAP);
		if (m_fTreeSize < 0.0f)
			m_fTreeSize = 0.0f;
		else if (m_fTreeSize > 100.0f)
			m_fTreeSize = 100.0f;
		ResizeSubWindows(rc.right, rc.bottom);
	}

	return 0;
}

class CClipboardHandler
{
public:
	CClipboardHandler(HWND a_hWnd)
	{
		if (!::OpenClipboard(a_hWnd))
			throw E_FAIL;
	}
	~CClipboardHandler()
	{
		::CloseClipboard();
	}
};

LRESULT CTreeConfigWnd::OnCommandCopy(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCurCfg == NULL)
		return 0;

	CComPtr<IConfigInMemory> pMemCfg;
	RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
	CopyConfigValues(pMemCfg, m_pCurCfg);
	ULONG nSize = 0;
	pMemCfg->DataBlockGetSize(&nSize);
	if (nSize == 0)
		return 0;

	CClipboardHandler cClipboard(*this);
	EmptyClipboard();

	HANDLE hMem = GlobalAlloc(GHND, nSize+sizeof nSize);
	if (hMem == NULL)
		return 0;

	BYTE* pMem = reinterpret_cast<BYTE*>(GlobalLock(hMem));
	*reinterpret_cast<ULONG*>(pMem) = nSize;
	pMemCfg->DataBlockGet(nSize, pMem+sizeof nSize);

	GlobalUnlock(hMem);

	SetClipboardData(m_nClipboardFormat, hMem);

	return 0;
}

LRESULT CTreeConfigWnd::OnCommandPaste(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCurCfg == NULL || m_nClipboardFormat == 0 || !IsWindow())
		return 0;

	CClipboardHandler cClipboard(*this);

	// try internal format first
	HANDLE hMem = GetClipboardData(m_nClipboardFormat);
	if (hMem == NULL)
		return 0;

	ULONG const* pMem = reinterpret_cast<ULONG const*>(GlobalLock(hMem));
	CComPtr<IConfigInMemory> pMemCfg;
	RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
	pMemCfg->DataBlockSet(*pMem, reinterpret_cast<BYTE const*>(pMem+1));
	CopyConfigValues(m_pCurCfg, pMemCfg);

	GlobalUnlock(hMem);

	return 0;
}

LRESULT CTreeConfigWnd::OnChangeCBChain(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (reinterpret_cast<HWND>(a_wParam) == m_hWndNextViewer)
	{
		m_hWndNextViewer = reinterpret_cast<HWND>(a_lParam);
		return 0;
	}
	if (m_hWndNextViewer != NULL)
	{
		::SendMessage(m_hWndNextViewer, WM_CHANGECBCHAIN, a_wParam, a_lParam);
	}
	return 0;
}

LRESULT CTreeConfigWnd::OnDrawClipBoard(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	m_wndToolbar.EnableButton(ID_TREE_PASTE, IsClipboardFormatAvailable(m_nClipboardFormat));
	if (m_hWndNextViewer != NULL)
	{
		::SendMessage(m_hWndNextViewer, WM_DRAWCLIPBOARD, a_wParam, a_lParam);
	}
	return 0;
}

LRESULT CTreeConfigWnd::OnSupportsConfigSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	return SUPPORT_RESPONSE_OK;
}

LRESULT CTreeConfigWnd::OnConfigSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	return SelectTreeItem(TVI_ROOT, a_lParam) ? SUPPORT_RESPONSE_OK : SUPPORT_RESPONSE_FAILED;
}

