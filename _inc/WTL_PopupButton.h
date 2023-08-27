
#pragma once


#define BN_SUBMENUCLICKED 20


class CButtonWithPopup :
	public CWindowImpl<CButtonWithPopup>,
	public CThemeImpl<CButtonWithPopup>
{
public:
	CButtonWithPopup() : m_fMouseOver(false), m_fWithMenu(true), m_nMenuStart(1000), m_p(NULL), m_fDPIScale(1.0f), m_fDefault(false)
	{
	}
	~CButtonWithPopup()
	{
		OnFinalMessage(NULL);
	}

	BOOL SubclassWindow(HWND hWnd, IUnknown* a_p)
	{
		CWindowImpl<CButtonWithPopup>::SubclassWindow(hWnd);
		m_fDefault = (GetStyle()&BS_TYPEMASK) == BS_DEFPUSHBUTTON;
		ModifyStyle(BS_TYPEMASK, BS_OWNERDRAW);
		HDC hdc = GetDC();
		m_fDPIScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);
		OpenThemeData(L"Button");
		ATLASSERT(m_p == NULL);
		m_p = a_p;
		if (m_p)
			m_p->AddRef();
		return TRUE;
	}
	void EnableMenu(bool a_bEnable)
	{
		if (m_fWithMenu != a_bEnable)
		{
			m_fWithMenu = a_bEnable;
			Invalidate();
		}
	}
	virtual void OnFinalMessage(HWND a_hWnd)
	{
		IUnknown* p = m_p;
		m_p = NULL;
		if (p)
			p->Release();
	}

	BEGIN_MSG_MAP(CButtonWithPopup)
		CHAIN_MSG_MAP (CThemeImpl<CButtonWithPopup>)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove);
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave);
	    MESSAGE_HANDLER(OCM__BASE + WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDown)
		MESSAGE_HANDLER(BM_SETSTYLE, OnSetStyle)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
	END_MSG_MAP()

	LRESULT OnDrawItem (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		LPDRAWITEMSTRUCT lpItem = (LPDRAWITEMSTRUCT) lParam;
		CDC dc (lpItem->hDC);

		UINT uState = lpItem->itemState;
		CRect rcDraw = lpItem->rcItem;

		UINT uFrameState = 0;
		if (m_hTheme != NULL)
		{
			DrawThemeParentBackground(dc, &rcDraw);
			if ((uState & ODS_SELECTED) != 0)
				uFrameState |= PBS_PRESSED;
			if ((uState & ODS_DISABLED) != 0)
				uFrameState |= PBS_DISABLED;
			if ((uState & ODS_HOTLIGHT) != 0 || m_fMouseOver)
				uFrameState |= PBS_HOT;
			else if ((uState & ODS_DEFAULT) != 0)
				uFrameState |= PBS_DEFAULTED;
			DrawThemeBackground(dc, BP_PUSHBUTTON, 
				m_fDefault && (uState & ODS_HOTLIGHT) == 0 && !m_fMouseOver ? PBS_DEFAULTED|uFrameState : uFrameState, &rcDraw, NULL);
			GetThemeBackgroundContentRect(dc, BP_PUSHBUTTON, 
				uFrameState, &rcDraw, &rcDraw);
		}
		else
		{
			dc.FillRect(&rcDraw, COLOR_3DFACE);

			//
			// Draw the outer edge
			//

			UINT uFrameState = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
			if ((uState & ODS_SELECTED) != 0)
				uFrameState |= DFCS_PUSHED;
			if ((uState & ODS_DISABLED) != 0)
				uFrameState |= DFCS_INACTIVE;
			dc.DrawFrameControl (&rcDraw, DFC_BUTTON, uFrameState);

			//
			// Adjust the position if we are selected (gives a 3d look)
			//
			
			if ((uState & ODS_SELECTED) != 0)
				rcDraw.OffsetRect(1, 1);
		}

		// Draw focus
		if ((uState & ODS_FOCUS) != 0 && (uState & ODS_NOFOCUSRECT) == 0) 
		{
			CRect rcFocus(rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom);
			dc.DrawFocusRect(&rcFocus);
		}
		rcDraw.InflateRect(-::GetSystemMetrics(SM_CXEDGE), -::GetSystemMetrics(SM_CYEDGE));

		if (m_fWithMenu)
		{
			// Draw the arrow
			{
				CRect rcArrow;
				RECT rc;
				GetClientRect(&rc);
				int ciArrowSizeY = int(rc.bottom*0.15+0.5f);
				int ciArrowSizeX = ciArrowSizeY+ciArrowSizeY;
				rcArrow.left   = rcDraw. right - ciArrowSizeX - ::GetSystemMetrics(SM_CXEDGE);
				rcArrow.top    = (rcDraw.bottom + rcDraw.top)/2 - ciArrowSizeY/2;
				rcArrow.right  = rcArrow.left + ciArrowSizeX;
				rcArrow.bottom = rcArrow.top + ciArrowSizeY;

				DrawArrow(dc, rcArrow, 0, (uState & ODS_DISABLED) ? ::GetSysColor (COLOR_GRAYTEXT) : RGB (0,0,0));

				rcDraw.right = rcArrow.left - ::GetSystemMetrics(SM_CXEDGE);
			}

			// Draw separator
			dc.DrawEdge(&rcDraw, EDGE_ETCHED, BF_RIGHT);
			m_nMenuStart = rcDraw.right-2;
			rcDraw.right -= ::GetSystemMetrics(SM_CXEDGE) + 2;
		}

		// Draw text
		TCHAR szText[256] = _T("");
		GetWindowText(szText, 256);
		if (m_hTheme != NULL)
		{
			DrawThemeText(dc, BP_PUSHBUTTON, uFrameState, CT2CW(szText), -1, DT_SINGLELINE|DT_CENTER|DT_VCENTER, NULL, &rcDraw);
		}
		else
		{
			int nFlags = (uState & ODS_NOACCEL) ? DT_NOPREFIX|DT_SINGLELINE|DT_CENTER|DT_VCENTER : DT_SINGLELINE|DT_CENTER|DT_VCENTER;
			dc.SetBkColor(GetSysColor(COLOR_3DFACE));
			if (uState & ODS_DISABLED)
			{
				rcDraw.OffsetRect(1,1);
				dc.SetTextColor(GetSysColor(COLOR_WINDOW));
				dc.DrawText(szText, -1, rcDraw, nFlags);

				rcDraw.OffsetRect(-1,-1);
				dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
				dc.DrawText(szText, -1, rcDraw, nFlags);
			}
			else
			{
				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
				dc.DrawText(szText, -1, rcDraw, nFlags);
			}
		}
		//if ((uState & ODS_DISABLED) == 0)
		//{
		//	dc .SetBkColor ((m_clrCurrent == CLR_DEFAULT) ? m_clrDefault : m_clrCurrent);
		//	dc .ExtTextOut (0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, NULL);
		//	dc .FrameRect (&rcDraw, (HBRUSH)::GetStockObject (BLACK_BRUSH));
		//}
		return 1;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (!m_fMouseOver)
		{
			m_fMouseOver = TRUE;
			TRACKMOUSEEVENT tme;
			tme .cbSize = sizeof (tme);
			tme .dwFlags = TME_LEAVE;
			tme .hwndTrack = m_hWnd;
			_TrackMouseEvent (&tme);
			InvalidateRect (NULL);
		}
		bHandled = FALSE;
		return FALSE;
	}

	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (m_fMouseOver)
		{
			m_fMouseOver = FALSE;
			InvalidateRect(NULL);
		}
		bHandled = FALSE;
		return FALSE;
	}

	LRESULT OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		RECT rc;
		GetClientRect(&rc);
		rc.left = m_nMenuStart;
		POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
		if (m_fWithMenu && ::PtInRect(&rc, pt))
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetWindowLong(GWLP_ID), BN_SUBMENUCLICKED), reinterpret_cast<LPARAM>(m_hWnd));
		else
			bHandled = FALSE;
		return 0;
	}

	LRESULT OnSetStyle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		UINT const nNewType = wParam&BS_TYPEMASK;
		if (nNewType == BS_DEFPUSHBUTTON)
			m_fDefault = true;
		else if (nNewType == BS_PUSHBUTTON)
			m_fDefault = false;

		return DefWindowProc(BM_SETSTYLE, (wParam & ~BS_TYPEMASK) | BS_OWNERDRAW, lParam);
	}

	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		return DefWindowProc() | (m_fDefault ? DLGC_DEFPUSHBUTTON : DLGC_UNDEFPUSHBUTTON);
	}

protected:
	static void DrawArrow (CDC &dc, const RECT &rect,
		int iDirection = 0, COLORREF clrArrow = RGB (0, 0, 0))
	{
		POINT ptsArrow[3];
		
		switch (iDirection)
		{
			case 0 : // Down
				{
					ptsArrow [0] .x = rect .left;
					ptsArrow [0] .y = rect .top;
					ptsArrow [1] .x = rect .right;
					ptsArrow [1] .y = rect .top;
					ptsArrow [2] .x = (rect .left + rect .right) / 2;
					ptsArrow [2] .y = rect .bottom;
					break;
				}
				
			case 1 : // Up
				{
					ptsArrow [0] .x = rect .left;
					ptsArrow [0] .y = rect .bottom;
					ptsArrow [1] .x = rect .right;
					ptsArrow [1] .y = rect .bottom;
					ptsArrow [2] .x = (rect .left + rect .right) / 2;
					ptsArrow [2] .y = rect .top;
					break;
				}
				
			case 2 : // Left
				{
					ptsArrow [0] .x = rect .right;
					ptsArrow [0] .y = rect .top;
					ptsArrow [1] .x = rect .right;
					ptsArrow [1] .y = rect .bottom;
					ptsArrow [2] .x = rect .left;
					ptsArrow [2] .y = (rect .top + rect .bottom) / 2;
					break;
				}
				
			case 3 : // Right
				{
					ptsArrow [0] .x = rect .left;
					ptsArrow [0] .y = rect .top;
					ptsArrow [1] .x = rect .left;
					ptsArrow [1] .y = rect .bottom;
					ptsArrow [2] .x = rect .right;
					ptsArrow [2] .y = (rect .top + rect .bottom) / 2;
					break;
				}
		}
		
		CBrush brArrow;
		brArrow.CreateSolidBrush(clrArrow);
		CPen penArrow;
		penArrow.CreatePen(PS_SOLID, 0, clrArrow);

		HBRUSH hbrOld = dc.SelectBrush (brArrow);
		HPEN hpenOld = dc.SelectPen (penArrow);

		dc.SetPolyFillMode (WINDING);
		dc.Polygon (ptsArrow, 3);

		dc.SelectBrush (hbrOld);
		dc.SelectPen (hpenOld);
		return;
	}

private:
	float m_fDPIScale;
	int m_nMenuStart;
	bool m_fMouseOver;
	bool m_fWithMenu;
	IUnknown* m_p;
	bool m_fDefault;
};

