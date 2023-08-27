// DesignerViewRolldown.cpp : Implementation of CDesignerViewRolldown

#include "stdafx.h"
#include "DesignerViewRolldown.h"


// CDesignerViewRolldown

HRESULT CDesignerViewRolldown::Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pSubSpec, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, IDocument* a_pDoc, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
{
	m_pStateMgr = a_pFrame;
	m_pStatusBar = a_pStatusBar;
	m_pViewMgr = a_pSubSpec;
	m_pConfig = a_pConfig;
	m_pDoc = a_pDoc;
	m_tLocaleID = a_tLocaleID;

	m_hSizeNS = LoadCursor(NULL, IDC_SIZENS);
	m_hArrow = LoadCursor(NULL, IDC_ARROW);

	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("Rolldown View"), 0, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT : WS_EX_CONTROLPARENT) == NULL)
	{
		// creation failed
		return E_FAIL; // TODO: error code
	}
	HDC hdc = GetDC();
	m_fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0;
	ReleaseDC(hdc);
	m_nHeaderSizeY = m_nHeaderSizeY*m_fScale+0.5f;

	UpdateDrawConstants();

	CConfigValue cCount;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_RD_ITEMS), &cCount);
	m_nControls = cCount.operator LONG();
	m_nTotalHeight = m_nGapY;
	if (m_nControls)
	{
		m_cControls.Attach(new CPanelCtrl[m_nControls]);
		for (ULONG i = 0; i < m_nControls; ++i)
		{
			OLECHAR szTmp[64];
			swprintf(szTmp, L"%s\\%08x", CFGID_RD_ITEMS, i);
			CComPtr<IConfig> pItemCfg;
			a_pConfig->SubConfigGet(CComBSTR(szTmp), &pItemCfg);
			CConfigValue cWindowName;
			a_pConfig->ItemValueGet(CComBSTR(szTmp), &cWindowName);
			m_cControls[i].Init(this, m_tLocaleID, pItemCfg, cWindowName, m_nGapX, max(m_nGapX, a_rcWindow->right-a_rcWindow->left-m_nGapX), m_nTotalHeight, m_fScale);
			bool const bVisible = m_cControls[i].GetWindowLong(GWL_STYLE)&WS_VISIBLE;
			if (m_pLastActive == NULL && bVisible && m_cControls[i].IsOpen())
				m_pLastActive = m_cControls+i;
			if (bVisible)
			{
				RECT rc;
				m_cControls[i].GetWindowRect(&rc);
				m_nTotalHeight += rc.bottom-rc.top+m_nGapY;
			}
		}
	}

	return S_OK;
}

void CDesignerViewRolldown::PanelChanged(CPanelCtrl* a_pPanel, bool a_bOpen)
{
	int nOldOffset = m_nOffset;
	RECT rc;
	GetClientRect(&rc);
	int nHeight = rc.bottom;
	int nY = m_nGapY;
	for (ULONG i = 0; i < m_nControls; ++i)
	{
		m_cControls[i].GetWindowRect(&rc);
		int nPanel = rc.bottom-rc.top;
		if (a_pPanel == m_cControls+i && a_bOpen)
		{
			if (m_nOffset+nHeight < nY+nPanel+m_nGapY)
			{
				m_nOffset = nY+nPanel+m_nGapY-nHeight;
			}
			if (m_nOffset > nY-m_nGapY)
			{
				m_nOffset = nY-m_nGapY;
			}
		}
		nY += nPanel+m_nGapY;
	}

	int nMax = max(0, nY-nHeight);
	if (m_nOffset > nMax)
	{
		m_nOffset = nMax;
	}
	RepositionPanels();
	if (a_pPanel && !a_bOpen)
	{
		a_pPanel->OnDeactivate(FALSE);
		SetFocus();
	}
}

STDMETHODIMP CDesignerViewRolldown::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	return m_pLastActive ? m_pLastActive->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
}

STDMETHODIMP CDesignerViewRolldown::OnIdle()
{
	for (size_t i = 0; i != m_nControls; ++i)
		if (m_pLastActive != m_cControls+i)
			m_cControls[i].OnIdle();
	if (m_pLastActive)
		m_pLastActive->OnIdle();

	bool bReposition = false;
	for (size_t i = 0; i != m_nControls; ++i)
		//if (m_cControls[i].IsOpen())
			bReposition |= m_cControls[i].UpdateAutoSize();
	if (bReposition)
		PanelChanged(m_pLastActive, m_pLastActive ? m_pLastActive->IsOpen() : false);
	return S_OK;
}

STDMETHODIMP CDesignerViewRolldown::OnDeactivate(BOOL a_bCancelChanges)
{
	for (size_t i = 0; i != m_nControls; ++i)
		if (m_cControls[i].IsOpen())
			m_cControls[i].OnDeactivate(a_bCancelChanges);
	return S_OK;
}

STDMETHODIMP CDesignerViewRolldown::QueryInterfaces(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
{
	if (m_pLastActive)
		m_pLastActive->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
	if (a_eFilter != EQIFActive)
		for (size_t i = 0; i != m_nControls; ++i)
			if (m_pLastActive != m_cControls+i)
				m_cControls[i].QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
	return S_OK;
}

STDMETHODIMP CDesignerViewRolldown::OptimumSize(SIZE* a_pSize)
{
	if (a_pSize)
	{
		if (a_pSize->cx < 100) a_pSize->cx = 100; // this could be better
		if (a_pSize->cy > m_nTotalHeight) a_pSize->cy = m_nTotalHeight;
	}
	return S_OK;
}

LRESULT CDesignerViewRolldown::OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (IsThemingSupported())
	{
		m_hThemeBar = ::OpenThemeData(m_hWnd, L"ExplorerBar");
		m_hThemeGripper = ::OpenThemeData(m_hWnd, L"ReBar");
	}
	m_cPanelBGBrush.CreateSolidBrush(((GetSysColor(COLOR_3DLIGHT)&0xfefefe) + (GetSysColor(COLOR_3DFACE)&0xfefefe))>>1/*0xe4d3da*/);
	return 0;
}

LRESULT CDesignerViewRolldown::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (m_hThemeBar)
	{
		::CloseThemeData(m_hThemeBar);
		m_hThemeBar = NULL;
	}
	if (m_hThemeGripper)
	{
		::CloseThemeData(m_hThemeGripper);
		m_hThemeGripper = NULL;
	}
	return 0;
}

LRESULT CDesignerViewRolldown::OnThemeChanged(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (m_hThemeBar)
		::CloseThemeData(m_hThemeBar);
	m_hThemeBar = IsThemingSupported() ? ::OpenThemeData(m_hWnd, L"ExplorerBar") : NULL;
	if (m_hThemeGripper)
		::CloseThemeData(m_hThemeGripper);
	m_hThemeGripper = IsThemingSupported() ? ::OpenThemeData(m_hWnd, L"ReBar") : NULL;
	return 0;
}

LRESULT CDesignerViewRolldown::OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	if (m_pLastActive)
		m_pLastActive->SetFocus();

	return 0;
}

LRESULT CDesignerViewRolldown::OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	LRESULT lRet = DefWindowProc(a_uMsg, a_wParam, a_lParam);

	if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
	{
		DWORD dwPos = ::GetMessagePos();
		POINT pt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};

		for (size_t i = 0; i != m_nControls; ++i)
		{
			RECT rc;
			m_cControls[i].GetClientRect(&rc);
			m_cControls[i].ClientToScreen(&rc);
			if (::PtInRect(&rc, pt))
			{
				m_pLastActive = m_cControls+i;
				//m_pLastActive->SetFocus();
				break;
			}
		}
	}

	return lRet;
}

LRESULT CDesignerViewRolldown::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	int nHeight = HIWORD(a_lParam);
	int nY = m_nGapY;
	for (ULONG i = 0; i < m_nControls; ++i)
	{
		RECT rc;
		m_cControls[i].GetWindowRect(&rc);
		nY += rc.bottom-rc.top+m_nGapY;
	}

	int nMax = max(0, nY-nHeight);
	if (m_nOffset > nMax)
		m_nOffset = nMax;

	m_nTotalHeight = m_nGapY-m_nOffset;
	int nX1 = m_nGapX;
	int nX2 = LOWORD(a_lParam) < (m_nGapX<<1) ? m_nGapX : (LOWORD(a_lParam)-m_nGapX);

	for (ULONG i = 0; i < m_nControls; ++i)
	{
		RECT rc;
		m_cControls[i].GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.left = nX1;
		rc.right = nX2;
		rc.bottom = m_nTotalHeight+rc.bottom-rc.top;
		rc.top = m_nTotalHeight;
		m_cControls[i].MoveWindow(&rc);
		m_nTotalHeight = rc.bottom+m_nGapY;
	}
	m_nTotalHeight += m_nOffset;

	return 0;
}

LRESULT CDesignerViewRolldown::OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	SetCursor(rc.bottom >= m_nTotalHeight ? m_hArrow : m_hSizeNS);
	return 0;
}

LRESULT CDesignerViewRolldown::OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	SetCapture();
	m_nDrag = GET_Y_LPARAM(a_lParam);
	return 0;
}

LRESULT CDesignerViewRolldown::OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (GetCapture() == m_hWnd)
		ReleaseCapture();
	return 0;
}

LRESULT CDesignerViewRolldown::OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	int nDelta = m_nDrag - GET_Y_LPARAM(a_lParam);
	if (GetCapture() == m_hWnd && nDelta)
	{
		int nOldOffset = m_nOffset;
		RECT rc;
		GetClientRect(&rc);
		int nMax = max(0, m_nTotalHeight-rc.bottom);
		int nProposed = m_nOffset+nDelta;
		if (nProposed < 0)
		{
			m_nDrag += m_nOffset;
			m_nOffset = 0;
		}
		else if (nProposed > nMax)
		{
			m_nDrag += nMax-m_nOffset;
			m_nOffset = nMax;
		}
		else
		{
			m_nDrag -= nDelta;
			m_nOffset = nProposed;
		}

		if (m_nOffset != nOldOffset)
		{
			RepositionPanels();
		}
	}
	return 0;
}

LRESULT CDesignerViewRolldown::OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_nWheelDelta += GET_WHEEL_DELTA_WPARAM(a_wParam);
	int nDelta = m_nWheelDelta < 0 ? -((-m_nWheelDelta)>>2) : (m_nWheelDelta>>2);
	if (nDelta)
	{
		m_nWheelDelta -= nDelta<<2;

		int nOldOffset = m_nOffset;
		RECT rc;
		GetClientRect(&rc);
		int nMax = max(0, m_nTotalHeight-rc.bottom);
		int nProposed = m_nOffset-nDelta;
		if (nProposed < 0)
		{
			m_nDrag += m_nOffset;
			m_nOffset = 0;
		}
		else if (nProposed > nMax)
		{
			m_nDrag += nMax-m_nOffset;
			m_nOffset = nMax;
		}
		else
		{
			m_nDrag -= nDelta;
			m_nOffset = nProposed;
		}

		if (m_nOffset != nOldOffset)
		{
			RepositionPanels();
		}
	}
	return 0;
}

LRESULT CDesignerViewRolldown::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	for (size_t i = 0; i != m_nControls; ++i)
	{
		BOOL b;
		m_cControls[i].OnSettingChange(a_uMsg, a_wParam, a_lParam, b);
	}
	return 0;
}

LRESULT CDesignerViewRolldown::OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_hThemeBar == NULL || m_bDisableStyle)
	{
		a_bHandled = FALSE;
		return 0;
	}
	CDCHandle cDC = reinterpret_cast<HDC>(a_wParam);
	RECT rc;
	GetClientRect(&rc);
//	DrawThemeBackground(cDC, EBP_HEADERBACKGROUND, 0, &rc, NULL);
	COLORREF clr1 = GetSysColor(COLOR_WINDOW);
	::GetThemeColor(m_hThemeBar, EBP_HEADERBACKGROUND, 0, TMT_GRADIENTCOLOR1, &clr1);
	COLORREF clr2 = clr1;
	::GetThemeColor(m_hThemeBar, EBP_HEADERBACKGROUND, 0, TMT_GRADIENTCOLOR2, &clr2);
	TRIVERTEX tVtx[] =
	{
		{rc.left, rc.top, GetRValue(clr1)*256, GetGValue(clr1)*256, GetBValue(clr1)*256, 0xffff},
		{rc.right, rc.bottom, GetRValue(clr2)*256, GetGValue(clr2)*256, GetBValue(clr2)*256, 0xffff}
	};
	GRADIENT_RECT tRct = {0, 1};
	if (Proc())
	{
		Proc()(cDC, tVtx, 2, &tRct, 1, GRADIENT_FILL_RECT_V);
		return 1;
	}
	a_bHandled = FALSE;
	return 0;
}

void CDesignerViewRolldown::UpdateDrawConstants()
{
	LOGFONT lf;
	::GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	lf.lfWeight = FW_BOLD;
	if (!m_cHeaderFont.IsNull())
		m_cHeaderFont.DeleteObject();
	m_cHeaderFont.CreateFontIndirect(&lf);

	m_clrHeaderHot = m_clrHeader = GetSysColor(COLOR_BTNTEXT);
}

void CDesignerViewRolldown::RepositionPanels()
{
	RECT rcAll;
	GetClientRect(&rcAll);
	m_nTotalHeight = m_nGapY-m_nOffset;
	int nX1 = m_nGapX;
	int nX2 = max(m_nGapX, rcAll.right-m_nGapX);

	for (ULONG i = 0; i < m_nControls; ++i)
	{
		RECT rc;
		m_cControls[i].GetWindowRect(&rc);
		bool const bVisible = m_cControls[i].GetWindowLong(GWL_STYLE)&WS_VISIBLE;
		ScreenToClient(&rc);
		if (rc.top != m_nTotalHeight)
		{
			rc.bottom = m_nTotalHeight+rc.bottom-rc.top;
			rc.top = m_nTotalHeight;
			m_cControls[i].MoveWindow(&rc);
		}
		if (bVisible)
			m_nTotalHeight = rc.bottom+m_nGapY;
	}
	m_nTotalHeight += m_nOffset;
}

void CDesignerViewRolldown::OnRWGotFocus(CPanelCtrl* a_pPanel, bool a_bForward)
{
	m_pLastActive = a_pPanel;
	if (a_bForward)
		GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
}

LRESULT CDesignerViewRolldown::OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
{
	return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
}