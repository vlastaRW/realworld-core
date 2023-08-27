// OpenFilterDlg.cpp : implementation of the COpenFilterDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpenFilterDlg.h"

#include <uxtheme.h>
#include <XPGUI.h>
#include <StringParsing.h>
#include "ConfigIDsStorage.h"


// COpenFilterDlg

STDMETHODIMP COpenFilterDlg::Destroy()
{
	DestroyWindow();
	return S_OK;
}

STDMETHODIMP COpenFilterDlg::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	try
	{
		STabContext* pCtx = m_wndTab.GetItem(m_wndTab.GetCurSel());
		return pCtx->pWindow ? pCtx->pWindow->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::FilterCreate(IStorageFilter** a_ppFilter)
{
	try
	{
		return m_wndTab.GetItem(m_wndTab.GetCurSel())->pWindow->FilterCreate(a_ppFilter);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::FiltersCreate(IEnumUnknowns** a_ppFilters)
{
	try
	{
		return m_wndTab.GetItem(m_wndTab.GetCurSel())->pWindow->FiltersCreate(a_ppFilters);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::DocTypesEnum(IEnumUnknowns** a_pFormatFilters)
{
	try
	{
		return m_wndTab.GetItem(m_wndTab.GetCurSel())->pWindow->DocTypesEnum(a_pFormatFilters);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::DocTypeGet(IDocumentType** a_pFormatFilter)
{
	try
	{
		return m_wndTab.GetItem(m_wndTab.GetCurSel())->pWindow->DocTypeGet(a_pFormatFilter);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::DocTypeSet(IDocumentType* a_pFormatFilter)
{
	try
	{
		return m_wndTab.GetItem(m_wndTab.GetCurSel())->pWindow->DocTypeSet(a_pFormatFilter);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::NavigationCommands(IEnumUnknowns** a_ppCommands)
{
	try
	{
		return m_wndTab.GetItem(m_wndTab.GetCurSel())->pWindow->NavigationCommands(a_ppCommands);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::OnIdle()
{
	try
	{
		int const n = m_wndTab.GetItemCount();
		for (int i = 0; i < n; ++i)
		{
			IStorageFilterWindow* p = m_wndTab.GetItem(i)->pWindow;
			if (p)
				p->OnIdle();
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::ActiveStorageGet(CLSID* a_pclsidPage)
{
	try
	{
		*a_pclsidPage = m_wndTab.GetItem(m_wndTab.GetCurSel())->tID;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::ActiveStorageSet(REFCLSID a_clsidPage)
{
	try
	{
		int const n = m_wndTab.GetItemCount();
		for (int i = 0; i < n; ++i)
		{
			if (IsEqualGUID(m_wndTab.GetItem(i)->tID, a_clsidPage))
			{
				if (m_wndTab.GetCurSel() != i)
				{
					m_wndTab.SetCurSel(i);
					BOOL bDummy;
					OnTcnSelchangeSwitchSource(IDC_OF_SWITCHSOURCE, NULL, bDummy);
				}
				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COpenFilterDlg::StoragesEnum(IEnumGUIDs** a_ppStorageIDs)
{
	try
	{
		*a_ppStorageIDs = NULL;
		CComPtr<IEnumGUIDsInit> pEGI;
		RWCoCreateInstance(pEGI, __uuidof(EnumGUIDs));
		int const n = m_wndTab.GetItemCount();
		for (int i = 0; i < n; ++i)
			pEGI->Insert(m_wndTab.GetItem(i)->tID);
		*a_ppStorageIDs = pEGI.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppStorageIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP COpenFilterDlg::StoragesName(REFCLSID a_clsidPage, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		int const n = m_wndTab.GetItemCount();
		for (int i = 0; i < n; ++i)
			if (IsEqualGUID(m_wndTab.GetItem(i)->tID, a_clsidPage))
				return m_wndTab.GetItem(i)->pFactory->NameGet(a_ppName);
		return E_FAIL;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP COpenFilterDlg::StoragesIcon(REFCLSID a_clsidPage, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		int const n = m_wndTab.GetItemCount();
		for (int i = 0; i < n; ++i)
			if (IsEqualGUID(m_wndTab.GetItem(i)->tID, a_clsidPage))
				return m_wndTab.GetItem(i)->pFactory->IconGet(a_nSize, a_phIcon);
		return E_FAIL;
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

void COpenFilterDlg::Create(HWND a_hParent, BSTR a_bstrInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID)
{
	m_tLocaleID = a_tLocaleID;
	m_pFormatFilters = a_pFormatFilters;
	m_pContextConfig = a_pContextConfig;
	m_pUserConfig = a_pUserConfig;
	m_pCallback = a_pCallback;
	m_pListener = a_pListener;
	m_dwFlags = a_dwFlags;
	m_bstrInitial = a_bstrInitial;
	Win32LangEx::CLangIndirectDialogImpl<COpenFilterDlg>::Create(a_hParent, reinterpret_cast<LPARAM>(a_pUserConfig));
}

struct initdialog
{
	UINT& nCtlID;
	int& nSelectedPage;
	int& iIns;
	int& m_nActivePage;
	COpenFilterDlg* pThis;
	RECT rcWindow;
	GUID tActivePage;
	initdialog(UINT& nCtlID, int& nSelectedPage, int& iIns, int& a_nActivePage, COpenFilterDlg* pThis, RECT rcWindow, GUID tActivePage) :
	nCtlID(nCtlID), nSelectedPage(nSelectedPage), iIns(iIns), m_nActivePage(a_nActivePage), pThis(pThis), rcWindow(rcWindow), tActivePage(tActivePage) {}

	void operator()(CLSID const* begin, CLSID const* const end) const
	{
		while (begin != end)
		{
			CComPtr<IStorageFilterFactory> p;
			RWCoCreateInstance(p, *begin);
			++begin;
			if (p == NULL) continue;

			if (p->SupportsGUI(pThis->m_dwFlags) != S_OK)
				continue;

			CComPtr<IStorageFilter> pLoc;
			p->FilterCreate(pThis->m_bstrInitial, 0, &pLoc);
			if (pLoc)
			{
				nSelectedPage = iIns;
				pLoc = NULL;
			}
			else if (nSelectedPage < 0 && IsEqualGUID(begin[-1], tActivePage))
			{
				nSelectedPage = iIns;
			}

			CStatic wndPlacehodler;
			wndPlacehodler.Create(pThis->m_hWnd, const_cast<RECT*>(&rcWindow), NULL, WS_CHILD, 0, nCtlID);
			nCtlID++;

			CComBSTR bstrName;
			CComPtr<ILocalizedString> pStr;
			p->NameGet(&pStr);
			pStr->GetLocalized(pThis->m_tLocaleID, &bstrName);
			COpenFilterDlg::STabContext* pItem = new COpenFilterDlg::STabContext(begin[-1], p);
			pItem->SetText(COLE2T(bstrName));
			HICON hIcon = NULL;
			p->IconGet(XPGUI::GetSmallIconSize(), &hIcon);
			if (hIcon != NULL)
			{
				pItem->SetImageIndex(pThis->m_cImageList.AddIcon(hIcon));
				DestroyIcon(hIcon);
			}
			pThis->m_wndTab.InsertItem(iIns, pItem);

			iIns++;
		}
	}
};

LRESULT COpenFilterDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM a_lParam, BOOL& /*bHandled*/)
{
	IConfig* pUserConfig = reinterpret_cast<IConfig*>(a_lParam);

	AddRef(); // released in OnFinalMessage

	RECT rcClient;
	GetClientRect(&rcClient);
	RECT rcTab = {0, 0, 278, 12};
	MapDialogRect(&rcTab);
	rcTab.right = rcClient.right;
	RECT rcWindow = rcClient;
	if (m_dwFlags&EFTFileBrowser)
	{
		rcTab.bottom = rcTab.top;
		m_nTabHeight = 0;
	}
	else
	{
		m_nTabHeight = rcTab.bottom;
	}
	rcWindow.top = rcTab.bottom;
	m_wndTab.Create(m_hWnd, rcTab, _T("Tab"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_SCROLL|CTCS_NOEDGE, 0, IDC_OF_SWITCHSOURCE);

	m_cImageList.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 4, 4);
	m_wndTab.SetImageList(m_cImageList);

	// create the config control if required
	if (pUserConfig)
	{
		// reduce the area if user config is present
		m_rcGaps.left = m_rcGaps.top = 4;
		m_rcGaps.right = m_rcGaps.right = 7;
		MapDialogRect(&m_rcGaps);

		RWCoCreateInstance(m_pMiniCfg, __uuidof(AutoConfigWnd));//__uuidof(MiniConfigWnd));
		m_pMiniCfg->ConfigSet(pUserConfig, ECPMFull);
		RECT rcConfig = rcWindow;
		SIZE tSize = {0, 0};
		m_pMiniCfg->OptimumSize(&tSize);
		if (tSize.cy == 0)
		{
			rcConfig.top = rcConfig.bottom;
		}
		else
		{
			m_nConfigHeight = /*rcGap.top+*/tSize.cy;
			rcWindow.bottom -= m_nConfigHeight;
			rcConfig.left = m_rcGaps.right;
			rcConfig.right -= m_rcGaps.right;
			rcConfig.top = rcConfig.bottom - m_nConfigHeight;
		}
		m_pMiniCfg->Create(m_hWnd, &rcConfig, IDC_OF_SWITCHSOURCE+1, m_tLocaleID, TRUE, ECWBMNothing);

		pUserConfig->ObserverIns(CObserverImpl<COpenFilterDlg, IConfigObserver, IUnknown*>::ObserverGet(), 0);
	}

	CConfigValue cActivePage;
	m_pContextConfig->ItemValueGet(CComBSTR(CFGID_ACTIVEPAGE), &cActivePage);

	UINT nCtlID = IDC_OF_TABPAGE0;
	// create the tab pages
	int nSelectedPage = -1;
	int iIns = 0;
	m_nActivePage = 0; // hack to prevent premature creation
	if (m_dwFlags&EFTFileSystemOnly)
	{
		CLSID t[] = {CLSID_StorageFilterFactoryFileSystem};
		initdialog(nCtlID, nSelectedPage, iIns, m_nActivePage, this, rcWindow, cActivePage)(t, t+1);
	}
	else
	{
		CPlugInEnumerator::EnumCategoryCLSIDs(CATID_StorageFilterFactory, initdialog(nCtlID, nSelectedPage, iIns, m_nActivePage, this, rcWindow, cActivePage));
	}

	m_nActivePage = -1;

	m_wndTab.SetCurSel(max(nSelectedPage, 0));
	BOOL bDummy;
	OnTcnSelchangeSwitchSource(IDC_OF_SWITCHSOURCE, NULL, bDummy);

	if (m_wndTab.GetItemCount() == 1 || (m_dwFlags&EFTFileBrowser))
	{
		m_wndTab.ShowWindow(SW_HIDE);
		HWND hCtl = 0;
		m_wndTab.GetItem(m_wndTab.GetCurSel())->pWindow->Handle(&hCtl);
		::MoveWindow(hCtl, rcWindow.left, 0, rcWindow.right-rcWindow.left, rcWindow.bottom, FALSE);
		m_nTabHeight = 0;
	}
	//m_wndTab.SetWindowPos(HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	//DlgResize_Init(false, false, 0);

	return TRUE;
}

void COpenFilterDlg::OnFinalMessage(HWND a_hWnd)
{
	if (m_pUserConfig)
		m_pUserConfig->ObserverDel(CObserverImpl<COpenFilterDlg, IConfigObserver, IUnknown*>::ObserverGet(), 0);
	Release();
}

LRESULT COpenFilterDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& a_bHandled)
{
	m_pCallback = NULL;

	int nCount = m_wndTab.GetItemCount();
	int nSelected = m_wndTab.GetCurSel();

	for (int i = 0; i < m_wndTab.GetItemCount(); i++)
	{
		if (m_wndTab.GetItem(i)->pWindow)
			m_wndTab.GetItem(i)->pWindow->Destroy();
	}

	a_bHandled = FALSE;
	return 0;
}

LRESULT COpenFilterDlg::OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	int nCount = m_wndTab.GetItemCount();
	int nSel = m_wndTab.GetCurSel();

	if (nSel >= 0 && nSel < nCount && m_wndTab.GetItem(nSel)->pWindow)
	{
		RWHWND hWnd;
		m_wndTab.GetItem(nSel)->pWindow->Handle(&hWnd);
		::SetFocus(hWnd);
	}
	return 0;
}

LRESULT COpenFilterDlg::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	for (int i = 0; i < m_wndTab.GetItemCount(); i++)
	{
		if (m_wndTab.GetItem(i)->pWindow)
			m_wndTab.GetItem(i)->pWindow->SendMessage(uMsg, wParam, lParam);
	}
	BOOL b;
	m_wndTab.OnSettingChange(uMsg, wParam, lParam, b);
	return 0;
}

LRESULT COpenFilterDlg::OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	LONG nX = GET_X_LPARAM(a_lParam);
	LONG nY = GET_Y_LPARAM(a_lParam);
	BOOL bVisible = IsWindowVisible();
	if(bVisible)
		SetRedraw(FALSE);
	RECT rcTab = {0, 0, nX, m_nTabHeight};
	m_wndTab.SetWindowPos(NULL, &rcTab, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	int nPages = m_wndTab.GetItemCount();
	RECT rcPage = {0, m_nTabHeight, nX, nY-m_nConfigHeight};
	for (int i = 0; i < nPages; ++i)
		GetDlgItem(IDC_OF_TABPAGE0+i).SetWindowPos(NULL, &rcPage, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	if (m_pMiniCfg)
	{
		RECT rcConfig = {m_rcGaps.right, nY-m_nConfigHeight, nX-m_rcGaps.right, nY};
		m_pMiniCfg->Move(&rcConfig);
	}
	if (bVisible)
	{
		SetRedraw(TRUE);
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	return 0;
}

//void COpenFilterDlg::DlgResize_UpdateLayout(int cxWidth, int cyHeight)
//{
//	return CDialogResize<COpenFilterDlg>::DlgResize_UpdateLayout(cxWidth, cyHeight+m_nInitial-m_nReserved);
//}

LRESULT COpenFilterDlg::OnOK(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetParent().SendMessage(WM_COMMAND, wID | (wNotifyCode<<16), 0);
	return 0;
}

LRESULT COpenFilterDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetParent().SendMessage(WM_COMMAND, wID | (wNotifyCode<<16), 0);
	return 0;
}

LRESULT COpenFilterDlg::OnTcnSelchangeSwitchSource(int /*idCtrl*/, LPNMHDR a_pNMHDR, BOOL& /*bHandled*/)
{
	int nCount = m_wndTab.GetItemCount();
	int nSel = m_wndTab.GetCurSel();

	if (nSel == m_nActivePage || nSel < 0 || nSel >= nCount)
		return 0;

	bool bSetFocus = true;

	if (m_nActivePage >= 0 && m_nActivePage < nCount)
	{
		RWHWND hPage = NULL;
		HWND hFocus = GetFocus();
		m_wndTab.GetItem(m_nActivePage)->pWindow->Handle(&hPage);
		bSetFocus = hPage && hFocus && ::IsChild(hPage, hFocus);
		m_wndTab.GetItem(m_nActivePage)->pWindow->Show(FALSE);
	}

	STabContext* pCtx = m_wndTab.GetItem(nSel);
	if (pCtx->pWindow == NULL)
	{
		CWindow wnd = GetDlgItem(IDC_OF_TABPAGE0+nSel);

		TCHAR szCLSID[64];
		StringFromGUID(pCtx->tID, szCLSID);
		CComPtr<IConfig> pWindowContextConfig;
		m_pContextConfig->SubConfigGet(CComBSTR(szCLSID), &pWindowContextConfig);
		//CComPtr<IStorageFilterWindow> pWnd;

		pCtx->pFactory->WindowCreate(m_bstrInitial, m_dwFlags, m_hWnd, m_pFormatFilters, pWindowContextConfig, m_pCallback, m_pListener, m_tLocaleID, &pCtx->pWindow);
		if (pCtx->pWindow == NULL)
			return 0;
		HWND hCtl = 0;
		pCtx->pWindow->Handle(&hCtl);
		RECT rc;
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		::MoveWindow(hCtl, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		::SetWindowLong(hCtl, GWL_ID, IDC_OF_TABPAGE0+nSel);
		wnd.DestroyWindow();
	}
	pCtx->pWindow->Show(TRUE);
	if (bSetFocus)
	{
		RWHWND h = NULL;
		pCtx->pWindow->Handle(&h);
		::SetFocus(h);
	}
	m_nActivePage = nSel;

	CComBSTR cCFGID_ACTIVEPAGE(CFGID_ACTIVEPAGE);
	m_pContextConfig->ItemValuesSet(1, &(cCFGID_ACTIVEPAGE.m_str), CConfigValue(m_wndTab.GetItem(m_nActivePage)->tID));

	return 0;
}

