// DesignerViewDockingWindows.cpp : Implementation of CDesignerViewDockingWindows

#include "stdafx.h"
#include "DesignerViewDockingWindows.h"

#include "ConfigIDsDockingWindows.h"
#include <MultiLanguageString.h>


// CDesignerViewDockingWindows

HRESULT CDesignerViewDockingWindows::Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pSubSpec, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, IDocument* a_pDoc, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
{
	m_tLocaleID = a_tLocaleID;
	m_pConfig = a_pConfig;

	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("DockingWindows Frame"), WS_CHILDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, 0, 0) == NULL)
	{
		// creation failed
		return E_FAIL; // TODO: error code
	}

	RECT rcWhole;
	GetClientRect(&rcWhole);

	CConfigValue cValue;
	CComPtr<IConfig> pSubCfg;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_DOCK_SUBVIEW), &cValue);
	a_pConfig->SubConfigGet(CComBSTR(CFGID_DOCK_SUBVIEW), &pSubCfg);
	a_pSubSpec->CreateWnd(a_pSubSpec, cValue, pSubCfg, a_pFrame, a_pStatusBar, a_pDoc, m_hWnd, &rcWhole, a_nStyle, a_tLocaleID, &m_pMainWnd);
	m_pLastActive = m_pMainWnd;

	m_pMainWnd->Show(TRUE);
	m_pMainWnd->Handle(&m_hWndClient);

	// create docking windows
	CConfigValue cWindows;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_DOCK_WINDOWS), &cWindows);

	for (LONG i = 0; i < cWindows.operator LONG(); i++)
	{
		OLECHAR szID[64];
		swprintf(szID, L"%s\\%08x", CFGID_DOCK_WINDOWS, i);
		CComPtr<IConfig> pWindowCfg;
		CComBSTR bstrID(szID);
		a_pConfig->SubConfigGet(bstrID, &pWindowCfg);
		CConfigValue cCaption;
		a_pConfig->ItemValueGet(bstrID, &cCaption);

		CComBSTR cCFGID_DOCK_WINDOWVIEW(CFGID_DOCK_WINDOWVIEW);
		CDockableWindow::SInit sInit;
		sInit.pViewMgr = a_pSubSpec;
		pWindowCfg->ItemValueGet(cCFGID_DOCK_WINDOWVIEW, &sInit.cViewID);
		pWindowCfg->SubConfigGet(cCFGID_DOCK_WINDOWVIEW, &sInit.pViewCfg);
		sInit.pStateMgr = a_pFrame;
		sInit.pStatusBar = a_pStatusBar;
		sInit.pDoc = a_pDoc;
		sInit.tLocaleID = a_tLocaleID;

		RECT rcWnd;
		CConfigValue cTmp;
		pWindowCfg->ItemValueGet(CComBSTR(CFGID_DOCK_FLOATPOSX), &cTmp);
		rcWnd.left = cTmp.operator LONG();
		pWindowCfg->ItemValueGet(CComBSTR(CFGID_DOCK_FLOATPOSY), &cTmp);
		rcWnd.top = cTmp.operator LONG();
		pWindowCfg->ItemValueGet(CComBSTR(CFGID_DOCK_FLOATSIZEX), &cTmp);
		rcWnd.right = rcWnd.left + cTmp.operator LONG();
		pWindowCfg->ItemValueGet(CComBSTR(CFGID_DOCK_FLOATSIZEY), &cTmp);
		rcWnd.bottom = rcWnd.top + cTmp.operator LONG();

		CDockableWindow* pWnd = new CDockableWindow;
		m_cWindows.push_back(pWnd);
		pWnd->Create(sInit, m_hWnd, rcWnd, COLE2CT(cCaption));

		pWindowCfg->ItemValueGet(CComBSTR(CFGID_DOCK_DOCKED), &cTmp);
		if (cTmp.operator bool())
		{
			pWindowCfg->ItemValueGet(CComBSTR(CFGID_DOCK_DOCKSIDE), &cTmp);
			DWORD dwSide;
			switch (cTmp.operator LONG())
			{
			case CFGVAL_DOCKSIDE_LEFT: dwSide = dockwins::CDockingSide::sLeft; break;
			case CFGVAL_DOCKSIDE_TOP: dwSide = dockwins::CDockingSide::sTop; break;
			case CFGVAL_DOCKSIDE_RIGHT: dwSide = dockwins::CDockingSide::sRight; break;
			case CFGVAL_DOCKSIDE_BOTTOM: dwSide = dockwins::CDockingSide::sBottom; break;
			default: throw E_FAIL;
			}
			DockWindow(*pWnd, dockwins::CDockingSide(dwSide), 0, 0.0f, 100, 100);
		}
	}

	ShowWindow(SW_SHOW);

	return S_OK;
}

STDMETHODIMP CDesignerViewDockingWindows::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (m_pLastActive)
	{
		return m_pLastActive->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
	}
	else
	{
		return S_FALSE;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::OnIdle()
{
	if (m_pLastActive)
	{
		return m_pLastActive->OnIdle();
	}
	else
	{
		return S_FALSE;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::OnDeactivate(BOOL a_bCancelChanges)
{
	if (m_pMainWnd)
		m_pMainWnd->OnDeactivate(a_bCancelChanges);
	//for (CWindows::const_iterator i = m_cWindows.begin(); i != m_cWindows.end(); ++i)
	//{
	//	//(*i)->m_ OnDea
	//}
	return S_OK;
}

STDMETHODIMP CDesignerViewDockingWindows::QueryInterfaces(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
{
	CComPtr<IUnknown> pThis;
	QueryInterface(a_iid, reinterpret_cast<void**>(&pThis));
	if (pThis)
		a_pInterfaces->Insert(pThis);

	if (m_pLastActive)
	{
		return m_pLastActive->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
	}
	else
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::ItemCount(ULONG* a_pnCount)
{
	try
	{
		*a_pnCount = m_cWindows.size();
		return S_OK;
	}
	catch (...)
	{
		return a_pnCount ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::NameGet(ULONG a_nIndex, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		OLECHAR szID[64];
		swprintf(szID, L"%s\\%08x", CFGID_DOCK_WINDOWS, a_nIndex);
		CConfigValue cViewName;
		m_pConfig->ItemValueGet(CComBSTR(szID), &cViewName);
		if (cViewName.TypeGet() != ECVTString)
			return E_FAIL;
		*a_ppName = new CMultiLanguageString(cViewName.Detach().bstrVal);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::IconIDGet(ULONG a_nIndex, GUID* a_ptIconID)
{
	try
	{
		*a_ptIconID = GUID_NULL;
		return S_FALSE;
		//OLECHAR szID[64];
		//swprintf(szID, L"%s\\%08x\\IconID", CFGID_TABS_ITEMS, a_nIndex);
		//CConfigValue cViewIconID;
		//m_pConfig->ItemValueGet(CComBSTR(szID), &cViewIconID);
		//if (cViewIconID.TypeGet() != ECVTGUID)
		//	return E_FAIL;
		//*a_ptIconID = cViewIconID;
		//return S_OK;
	}
	catch (...)
	{
		return a_ptIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::IconGet(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		return E_NOTIMPL;
		//OLECHAR szID[64];
		//swprintf(szID, L"%s\\%08x\\IconID", CFGID_TABS_ITEMS, a_nIndex);
		//CConfigValue cViewIconID;
		//m_pConfig->ItemValueGet(CComBSTR(szID), &cViewIconID);
		//if (cViewIconID.TypeGet() != ECVTGUID)
		//	return E_FAIL;
		//CComPtr<IDesignerFrameIcons> pIconsManager;
		//RWCoCreateInstance(pIconsManager, __uuidof(DesignerFrameIconsManager));
		//return pIconsManager->GetIcon(cViewIconID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::IsVisible(ULONG a_nIndex)
{
	try
	{
		if (a_nIndex >= m_cWindows.size())
			return E_RW_INDEXOUTOFRANGE;
		return (m_cWindows[a_nIndex]->GetStyle()&WS_VISIBLE) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewDockingWindows::SetVisible(ULONG a_nIndex, BOOL a_bVisible)
{
	try
	{
		if (a_nIndex >= m_cWindows.size())
			return E_RW_INDEXOUTOFRANGE;
		if (a_bVisible)
			m_cWindows[a_nIndex]->Show();
		else
			m_cWindows[a_nIndex]->Hide();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


LRESULT CDesignerViewDockingWindows::OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	InitializeDockingFrame(CStyle::sIgnoreSysSettings|CStyle::sFullDrag|CStyle::sAnimation);
	return 0;
}

