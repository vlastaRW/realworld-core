// AutoConfigWnd.cpp : Implementation of CAutoConfigWnd

#include "stdafx.h"
#include "AutoConfigWnd.h"

#include "FullConfigWnd.h"
#include "ConfigMessages.h"


// CAutoConfigWnd

STDMETHODIMP CAutoConfigWnd::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (a_bBeforeAccel && m_hWnd && a_pMsg->hwnd && a_pMsg->message >= WM_KEYFIRST && a_pMsg->message <= WM_KEYLAST)
	{
		TCHAR szBuf[64];
		if (GetClassName(a_pMsg->hwnd, szBuf, itemsof(szBuf)) && _tcsicmp(szBuf, _T("EDIT")) == 0)
		{
			if (IsDialogMessage(const_cast<LPMSG>(a_pMsg)))
				return S_OK;
			TranslateMessage(a_pMsg);
			DispatchMessage(a_pMsg);
			return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CAutoConfigWnd::TopWindowSet(BOOL a_bIsTopWindow, DWORD a_clrBackground)
{
	if (m_hBGBrush)
		DeleteObject(m_hBGBrush);
	m_bTopLevel = a_bIsTopWindow;
	if (m_bTopLevel)
	{
		m_hBGBrush = CreateSolidBrush(a_clrBackground);
		m_clrBG = a_clrBackground;
	}
	return S_OK;
}

STDMETHODIMP CAutoConfigWnd::Create(RWHWND a_hParent, RECT const* a_prcPosition, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode)
{
	m_tLocaleID = a_tLocaleID;
	m_eBorderMode = a_eBorderMode;
	m_ptOrigin.x = a_prcPosition->left;
	m_ptOrigin.y = a_prcPosition->top;
	m_szClient.cx = a_prcPosition->right-a_prcPosition->left;
	m_szClient.cy = a_prcPosition->bottom-a_prcPosition->top;
	Win32LangEx::CLangIndirectDialogImpl<CAutoConfigWnd>::Create(a_hParent);
	SetWindowLong(GWL_ID, a_nCtlID);
	MoveWindow(a_prcPosition);
	if (a_bVisible)
		ShowWindow(SW_SHOW);
	return S_OK;
}

STDMETHODIMP CAutoConfigWnd::ConfigSet(IConfig* a_pConfig, EConfigPanelMode a_eMode)
{
	m_eMode = a_eMode;
	if (a_pConfig == m_pConfig.p)
		return S_FALSE;

	if (m_pConfig != NULL)
	{
		m_pConfig->ObserverDel(ObserverGet(), 0);
	}

	m_pConfig = a_pConfig;

	if (m_pConfig != NULL)
	{
		m_pConfig->ObserverIns(ObserverGet(), 0);
	}

	m_pCustGUI = NULL;

	// destroy old window
	if (m_wndCannotFit.IsWindow())
		m_wndCannotFit.DestroyWindow();
	if (m_pChildWindow != NULL)
	{
		m_pChildWindow->Destroy();
		m_pChildWindow = NULL;
	}

	if (a_pConfig == NULL)
		return S_FALSE;

	a_pConfig->QueryInterface(&m_pCustGUI);

	m_tLastConfigUID = GUID_NULL;
	if (m_pCustGUI != NULL)
		m_pCustGUI->UIDGet(&m_tLastConfigUID);

	if (!IsWindow())
		return S_OK;

	// create main window
	RECT rcClient;
	GetClientRect(&rcClient);
	UpdateConfigGUI2(rcClient.right, rcClient.bottom);

	return S_OK;
}

STDMETHODIMP CAutoConfigWnd::OptimumSize(SIZE *a_pSize)
{
	if (m_pCustGUI == NULL)
		return E_NOTIMPL;

	try
	{
		ULONG nReqX = 0;
		ULONG nReqY = 0;
		if (m_pChildWindow == NULL && m_hWnd != NULL)
		{
			RECT rcClient = {0, 0, 0, 0};
			GetClientRect(&rcClient);
			m_pCustGUI->WindowCreate(m_hWnd, &rcClient, IDC_CUSTOM_GUI_WINDOW, m_tLocaleID, TRUE, m_eBorderMode != ECWBMNothing, m_pConfig, m_eMode, &m_pChildWindow);
		}
		if (m_pChildWindow)
		{
			CComQIPtr<IResizableConfigWindow> pRCW(m_pChildWindow);
			if (pRCW)
			{
				SIZE sz = *a_pSize;
				if (SUCCEEDED(pRCW->OptimumSize(&sz)))
				{
					nReqX = sz.cx;
					nReqY = sz.cy;
				}
			}
		}
		if (nReqX == 0 && nReqY == 0)
			m_pCustGUI->MinSizeGet(m_pConfig, m_tLocaleID, m_eMode, &nReqX, &nReqY);
		if (S_FALSE != m_pCustGUI->RequiresMargins() && m_eBorderMode != ECWBMNothing)
		{
			RECT rcGap = {7, 7, 14, 14};
			MapDialogRect(&rcGap);
			a_pSize->cx = rcGap.right+nReqX;
			a_pSize->cy = rcGap.bottom+nReqY;
		}
		else
		{
			a_pSize->cx = nReqX;
			a_pSize->cy = nReqY;
		}
		return S_OK;
	}
	catch (...)
	{
		return a_pSize ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CAutoConfigWnd::ChangeLanguage(LCID a_tLocaleID)
{
	try
	{
		m_tLocaleID = a_tLocaleID;

		if (m_pConfig == NULL)
			return S_FALSE;

		//m_pCustGUI = NULL;

		// destroy old window
		if (m_wndCannotFit.IsWindow())
			m_wndCannotFit.DestroyWindow();
		if (m_pChildWindow != NULL)
		{
			m_pChildWindow->Destroy();
			m_pChildWindow = NULL;
		}

		//a_pConfig->QueryInterface(&m_pCustGUI);

		m_tLastConfigUID = GUID_NULL;
		if (m_pCustGUI != NULL)
			m_pCustGUI->UIDGet(&m_tLastConfigUID);

		if (!IsWindow())
			return S_OK;

		// create main window
		RECT rcClient;
		GetClientRect(&rcClient);
		UpdateConfigGUI2(rcClient.right, rcClient.bottom);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


LRESULT CAutoConfigWnd::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	BOOL b;
	CThemeImpl<CAutoConfigWnd>::OnCreate(0, 0, 0, b);
	RECT rcClient;
	GetClientRect(&rcClient);
	RECT rcWindow;
	GetWindowRect(&rcWindow);
	UpdateConfigGUI(m_szClient.cx-rcWindow.right-rcWindow.left-rcClient.right, m_szClient.cy-rcWindow.bottom-rcWindow.top-rcClient.bottom);
	AddRef();
	return TRUE;
}

LRESULT CAutoConfigWnd::OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	UpdateConfigGUI(LOWORD(a_lParam), HIWORD(a_lParam));
	return 0;
}

LRESULT CAutoConfigWnd::OnSwitchToTableClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (!ConfigSelect(m_hWnd, m_pConfig))
	{
		m_pCustGUI = NULL;
		RECT rcClient;
		GetClientRect(&rcClient);
		UpdateConfigGUI(rcClient.right, rcClient.bottom);
	}
	return 0;
}

LRESULT CAutoConfigWnd::OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (a_lParam != NULL && HWND(a_lParam) == m_wndCannotFit.m_hWnd)
	{
		::SetTextColor((HDC)a_wParam, 0xff6060);
		::SetBkColor((HDC)a_wParam, GetSysColor(COLOR_3DFACE));
		return (LRESULT)GetSysColorBrush(COLOR_3DFACE);
	}
	a_bHandled = FALSE;
	return 0;
}

LRESULT CAutoConfigWnd::OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (a_wParam != NULL && HWND(a_wParam) == m_wndCannotFit.m_hWnd)
	{
		HCURSOR hCur = ::LoadCursor(NULL, IDC_HAND);
		if (hCur != NULL)
		{
			::SetCursor(hCur);
			return TRUE;
		}
	}
	a_bHandled = FALSE;
	return 0;
}

void CAutoConfigWnd::OwnerNotify(TCookie a_tCookie, IUnknown* a_pChangedParam)
{
	if (!IsWindow())
		return;

	CComQIPtr<IConfigCustomGUI> pCustNew(m_pConfig);
	if (pCustNew != NULL)
	{
		GUID tGUID = GUID_NULL;
		pCustNew->UIDGet(&tGUID);
		if (IsEqualGUID(m_tLastConfigUID, tGUID))
			return;
		m_tLastConfigUID = tGUID;
	}
	else
	{
		if (m_pCustGUI == NULL)
			return;
		m_tLastConfigUID = GUID_NULL;
	}

	m_pCustGUI = pCustNew;

	// destroy old window
	if (m_wndCannotFit.IsWindow())
		m_wndCannotFit.DestroyWindow();
	if (m_pChildWindow != NULL)
	{
		m_pChildWindow->Destroy();
		m_pChildWindow = NULL;
	}

	RECT rcClient;
	GetClientRect(&rcClient);
	UpdateConfigGUI(rcClient.right, rcClient.bottom);
}

void CAutoConfigWnd::UpdateConfigGUI2(int a_nSizeX, int a_nSizeY)
{
	m_bUpdatePosted = false;

	if (!IsWindow())
		return;

	if (m_pConfig == NULL)
		return;

	if (m_pCustGUI == NULL)
	{
		RECT rcClient = {0, 0, a_nSizeX, a_nSizeY};

		if (m_wndCannotFit.IsWindow())
			m_wndCannotFit.DestroyWindow();

		if (m_pChildWindow != NULL)
		{
			// resize FullConfigWnd
			m_pChildWindow->Move(&rcClient);
			return;
		}
		else
		{
			// create FullConfigWnd
			CComObject<CFullConfigWnd>* pFullWnd = NULL;
			CComObject<CFullConfigWnd>::CreateInstance(&pFullWnd);
			m_pChildWindow = pFullWnd;
			pFullWnd->ConfigSet(m_pConfig, m_eMode);
			pFullWnd->Create(m_hWnd, &rcClient, IDC_FULL_GUI_WINDOW, m_tLocaleID, TRUE, m_eBorderMode == ECWBMMargin ? ECWBMMargin : ECWBMNothing);
		}
	}

	if (m_pCustGUI != NULL)
	{
		RECT rcGap = {7, 7, 14, 14};
		MapDialogRect(&rcGap);

		ULONG nReqX = 0;
		ULONG nReqY = 0;
		if (m_pChildWindow)
		{
			CComQIPtr<IResizableConfigWindow> pRCW(m_pChildWindow);
			if (pRCW)
			{
				SIZE sz = {0, 0};
				if (SUCCEEDED(pRCW->OptimumSize(&sz)))
				{
					nReqX = sz.cx;
					nReqY = sz.cy;
				}
			}
		}
		if (nReqX == 0 && nReqY == 0)
			m_pCustGUI->MinSizeGet(m_pConfig, m_tLocaleID, m_eMode, &nReqX, &nReqY);
		bool bMargins = S_FALSE != m_pCustGUI->RequiresMargins() && m_eBorderMode != ECWBMNothing;
		bool bFit = bMargins ?
			(a_nSizeX >= int(nReqX+rcGap.right) && a_nSizeY >= int(nReqY+rcGap.bottom)) :
			(a_nSizeX >= int(nReqX) && a_nSizeY >= int(nReqY));

		if (bFit)
		{
			if (m_wndCannotFit.IsWindow())
				m_wndCannotFit.DestroyWindow();

			RECT rcClient =
			{
				bMargins ? rcGap.left : 0,
				bMargins ? rcGap.top : 0,
				bMargins ? a_nSizeX-rcGap.right+rcGap.left : a_nSizeX,
				bMargins ? a_nSizeY-rcGap.bottom+rcGap.top : a_nSizeY
			};

			if (m_pChildWindow != NULL)
			{
				// resize cutom GUI
				m_pChildWindow->Move(&rcClient);
				return;
			}
			else
			{
				// switch from temp GUI to custom GUI
				m_pCustGUI->WindowCreate(m_hWnd, &rcClient, IDC_CUSTOM_GUI_WINDOW, m_tLocaleID, TRUE, m_eBorderMode != ECWBMNothing, m_pConfig, m_eMode, &m_pChildWindow);
				return;
			}
		}
		else
		{
			if (m_pChildWindow != NULL)
			{
				m_pChildWindow->Destroy();
				m_pChildWindow = NULL;
			}

			RECT rcClient =
			{
				0,
				0,
				a_nSizeX,
				a_nSizeY
			};

			if (m_wndCannotFit.IsWindow())
			{
				// resize temp GUI
				m_wndCannotFit.MoveWindow(&rcClient);
				m_wndCannotFit.Invalidate();
				return;
			}
			else
			{
				// switch from custom GUI to temp GUI
				TCHAR szTmp[128];
				Win32LangEx::LoadString(_pModule->get_m_hInst(), SupportsConfigSelect(m_hWnd) ? IDS_SWITCH2SUBCONFIG : IDS_WONTFIT, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
				szTmp[itemsof(szTmp)-1] = _T('\0');
				m_wndCannotFit.Create(m_hWnd, &rcClient, szTmp, WS_CHILDWINDOW|WS_VISIBLE|SS_CENTER|SS_NOTIFY, 0, IDC_CONFIGURE_BUTTON);
				m_wndCannotFit.SetFont(GetFont());
				return;
			}
		}
	}
}

