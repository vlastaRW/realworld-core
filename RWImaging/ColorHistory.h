
#pragma once

#include <ObserverImpl.h>

inline float hls_value(float n1, float n2, float h)
{
	h += 360.0f;
    float hue = h - 360.0f*(int)(h/360.0f);

	if (hue < 60.0f)
		return n1 + ( n2 - n1 ) * hue / 60.0f;
	else if (hue < 180.0f)
		return n2;
	else if (hue < 240.0f)
		return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
	else
		return n1;
}

inline void HLS2RGB(float h, float l, float s, float& r, float& g, float& b)
{ // h from <0, 360)
	float m1, m2;
	m2 = l + (l <= 0.5f ? l*s : s - l*s);
	m1 = 2.0f * l - m2;
	if (s == 0.0f)
		r = g = b = l;
	else
	{
		r = hls_value(m1, m2, h+120.0f);
		g = hls_value(m1, m2, h);
		b = hls_value(m1, m2, h-120.0f);
	}
	if (r > 1.0f) r = 1.0f;
	if (g > 1.0f) g = 1.0f;
	if (b > 1.0f) b = 1.0f;
	if (r < 0.0f) r = 0.0f;
	if (g < 0.0f) g = 0.0f;
	if (b < 0.0f) b = 0.0f;
}

class CColorHistory2 : 
	public CWindowImpl<CColorHistory2>,
	public CObserverImpl<CColorHistory2, IConfigObserver, IUnknown*>
{
public:
	CColorHistory2() : m_bEnableUpdates(true), m_bTrackingMouse(false), m_nLastColor(HTEmptySpace)
	{
		m_tActiveColor.eSpace = ESCSRGB;
		m_tActiveColor.bstrName = NULL;
		m_tActiveColor.f1 = m_tActiveColor.f2 = m_tActiveColor.f3 = m_tActiveColor.f4 = 0.0f;
		m_tActiveColor.fA = 1.0f;
	}
	~CColorHistory2()
	{
		if (m_pColorsCfg)
			m_pColorsCfg->ObserverDel(CObserverImpl<CColorHistory2, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (i->bstrName)
				SysFreeString(i->bstrName);
	}
	enum { CH_COLOR_CHANGED = 1, MAXLASTCOLORS = 24, HTEmptySpace = -2, HTActiveColor = -1, };

	DECLARE_WND_CLASS_EX(_T("ColorHistory2Class"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_3DFACE);

	void Init(IConfig* a_pColorsCfg, RWHWND a_hParent, RECT const* a_rcWindow, LCID a_tLocaleID, UINT a_nIDC)
	{
		m_pColorsCfg = a_pColorsCfg;
		m_pColorsCfg->ObserverIns(CObserverImpl<CColorHistory2, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		ParseColors();

		m_tLocaleID = a_tLocaleID;

		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
		SetWindowLong(GWL_ID, a_nIDC);
	}

	void SetColor(float a_fR, float a_fG, float a_fB, float a_fA)
	{
		TSwatchColor t;
		t.eSpace = ESCSRGB;
		t.bstrName = NULL;
		t.f1 = a_fR;
		t.f2 = a_fG;
		t.f3 = a_fB;
		t.f4 = 0.0f;
		t.fA = a_fA;
		if (!CompareColors(t, m_tActiveColor))
		{
			m_tActiveColor = t;
			Invalidate(FALSE);
		}
	}
	TSwatchColor GetColor() const
	{
		return m_tActiveColor;
	}
	void AddColor()
	{
		AddColor(&m_tActiveColor);
	}


BEGIN_MSG_MAP(CColorHistory2)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
END_MSG_MAP()

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, IUnknown*)
	{
		try
		{
			if (ParseColors())
				Invalidate(FALSE);
		}
		catch (...)
		{
		}
	}

	static void DrawSquare(COLORREF clr1, COLORREF clr2, CDCHandle a_cDC, RECT const& a_rc, TSwatchColor const a_clr)
	{
		float fR = a_clr.f1;
		float fG = a_clr.f2;
		float fB = a_clr.f3;
		if (a_clr.eSpace == ESCSHLS)
			HLS2RGB(a_clr.f1, a_clr.f2, a_clr.f3, fR, fG, fB);
		if (a_clr.fA >= 1.0f)
		{
			COLORREF clr = RGB(CGammaTables::ToSRGB(fR), CGammaTables::ToSRGB(fG), CGammaTables::ToSRGB(fB));
			HBRUSH hBr = CreateSolidBrush(clr);
			a_cDC.FillRect(&a_rc, hBr);
			DeleteObject(hBr);
		}
		else
		{
			float fA = a_clr.fA < 0.0f ? 0.0f : a_clr.fA;
			float fLR = CGammaTables::FromSRGB(GetRValue(clr1));
			float fLG = CGammaTables::FromSRGB(GetGValue(clr1));
			float fLB = CGammaTables::FromSRGB(GetBValue(clr1));
			float fSR = CGammaTables::FromSRGB(GetRValue(clr2));
			float fSG = CGammaTables::FromSRGB(GetGValue(clr2));
			float fSB = CGammaTables::FromSRGB(GetBValue(clr2));
			DWORD dwL = RGB(
				CGammaTables::ToSRGB(fLR*(1.0f-fA)+fR*fA),
				CGammaTables::ToSRGB(fLG*(1.0f-fA)+fG*fA),
				CGammaTables::ToSRGB(fLB*(1.0f-fA)+fB*fA));
			DWORD dwS = RGB(
				CGammaTables::ToSRGB(fSR*(1.0f-fA)+fR*fA),
				CGammaTables::ToSRGB(fSG*(1.0f-fA)+fG*fA),
				CGammaTables::ToSRGB(fSB*(1.0f-fA)+fB*fA));
			DWORD dwM = RGB(
				CGammaTables::ToSRGB((fSR+fLR)*0.5f*(1.0f-fA)+fR*fA),
				CGammaTables::ToSRGB((fSG+fLG)*0.5f*(1.0f-fA)+fG*fA),
				CGammaTables::ToSRGB((fSB+fLB)*0.5f*(1.0f-fA)+fB*fA));

			HBRUSH hBr1 = CreateSolidBrush(dwL);
			HBRUSH hBr2 = CreateSolidBrush(dwS);
			HBRUSH hBr3 = CreateSolidBrush(dwM);
			int nX1 = (a_rc.left+a_rc.right)>>1;
			int nX2 = (a_rc.left+a_rc.right+1)>>1;
			int nY1 = (a_rc.top+a_rc.bottom)>>1;
			int nY2 = (a_rc.top+a_rc.bottom+1)>>1;
			RECT rc;
			rc.left = a_rc.left;
			rc.top = a_rc.top;
			rc.right = nX1;
			rc.bottom = nY1;
			a_cDC.FillRect(&rc, hBr1);
			if (nX1 != nX2)
			{
				rc.left = nX1;
				rc.right = nX2;
				a_cDC.FillRect(&rc, hBr3);
			}
			rc.left = nX2;
			rc.right = a_rc.right;
			a_cDC.FillRect(&rc, hBr2);
			rc.top = nY2;
			rc.bottom = a_rc.bottom;
			a_cDC.FillRect(&rc, hBr1);
			if (nX1 != nX2)
			{
				rc.left = nX1;
				rc.right = nX2;
				a_cDC.FillRect(&rc, hBr3);
			}
			rc.left = a_rc.left;
			rc.right = nX1;
			a_cDC.FillRect(&rc, hBr2);
			if (nY1 != nY2)
			{
				rc.left = a_rc.left;
				rc.right = a_rc.right;
				rc.top = nY1;
				rc.bottom = nY2;
				a_cDC.FillRect(&rc, hBr3);
			}
			DeleteObject(hBr1);
			DeleteObject(hBr2);
			DeleteObject(hBr3);
		}
	}
	void DoPaint(CDCHandle a_cDC)
	{
		RECT rcWnd;
		GetClientRect(&rcWnd);
		HBRUSH hBgBr = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORDLG, (WPARAM)a_cDC.m_hDC);
		if (hBgBr)
		{
			a_cDC.FillRect(&rcWnd, hBgBr);
		}
		else
		{
			a_cDC.FillSolidRect(&rcWnd, GetSysColor(COLOR_WINDOW));
		}
		RECT rc;
		rc.left = rc.top = m_nGapSize;
		rc.bottom = m_nGapSize+m_nSquareSize;
		COLORREF clr1 = GetSysColor(COLOR_3DLIGHT);
		COLORREF clr2 = GetSysColor(COLOR_3DSHADOW);
		if (!m_aColors.empty() && !CompareColors(m_tActiveColor, m_aColors[0]))
		{
			RECT rcSq = {rc.left, rc.top, rc.left+m_nSquareSize, rc.bottom};
			DrawSquare(clr1, clr2, a_cDC, rcSq, m_tActiveColor);
			if (m_nLastColor == HTActiveColor)
				a_cDC.Draw3dRect(rc.left-2, rc.top-2, m_nSquareSize+4, m_nSquareSize+4, clr2, clr2);
			rc.left += m_nSquareSize+m_nSmallGapSize;
			a_cDC.FillSolidRect(rc.left, rc.top-(m_nGapSize>>1), 1, rc.bottom-rc.top+m_nGapSize, GetSysColor(COLOR_3DSHADOW));
			rc.left += 1+m_nSmallGapSize;
		}
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
		{
			if (rc.left >= rcWnd.right)
				break;

			rc.right = rc.left+m_nSquareSize;
			DrawSquare(clr1, clr2, a_cDC, rc, *i);
			if (m_nLastColor == i-m_aColors.begin())
				a_cDC.Draw3dRect(rc.left-2, rc.top-2, m_nSquareSize+4, m_nSquareSize+4, clr2, clr2);
			rc.left += m_nSquareSize+m_nSmallGapSize;
		}
	}

	// message handlers
protected:
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		PAINTSTRUCT ps;
		CDCHandle cDC(BeginPaint(&ps));

		DoPaint(cDC);

		EndPaint(&ps);
		return 0;
	}

	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = GetDC();
		int nDPI = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(hDC);
		m_nSquareSize = (nDPI+6)/8;
		m_nGapSize = (nDPI+12)/24;
		m_nSmallGapSize = (nDPI+16)/32;
		m_wndTips.Create(m_hWnd, NULL, _T("ColorTip"), WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);

		m_tToolTip.cbSize = TTTOOLINFO_V1_SIZE;//sizeof m_tToolTip;
		m_tToolTip.uFlags = TTF_IDISHWND|TTF_TRACK|TTF_ABSOLUTE;
		m_tToolTip.hwnd = m_hWnd;
		m_tToolTip.hinst = _pModule->get_m_hInst();
		m_tToolTip.lpszText = _T("N/A");
		m_tToolTip.uId = (UINT_PTR)m_hWnd;
		GetClientRect(&m_tToolTip.rect);
		m_wndTips.AddTool(&m_tToolTip);
		m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		m_wndTips.SetMaxTipWidth(800);

		return 0;
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		if (m_wndTips.IsWindow())
			m_wndTips.DestroyWindow();
		if (m_pColorsCfg)
		{
			m_pColorsCfg->ObserverDel(CObserverImpl<CColorHistory2, IConfigObserver, IUnknown*>::ObserverGet(), 0);
			m_pColorsCfg = NULL;
		}
		return 0;
	}
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bTrackingMouse)
		{
			// request notification when the mouse leaves
			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0 };
			TrackMouseEvent(&tme);
			m_bTrackingMouse = true;
		}

		POINT pt = { GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam) };

		UpdateTooltip(pt);

		return 0;
	}
	void UpdateTooltip(POINT pt)
	{
		int nColor = ColorHitTest(pt);

		if (nColor != HTEmptySpace)
		{
			if (m_nLastColor != nColor)
			{
				if (m_nLastColor == HTEmptySpace)
				{
					// activate the ToolTip
					m_wndTips.TrackActivate(&m_tToolTip, TRUE);
				}
				else
				{
					RECT rcSq;
					GetExtSquareRect(m_nLastColor, &rcSq);
					InvalidateRect(&rcSq, FALSE);
				}
				CComBSTR bstrTmp;
				if (nColor == HTActiveColor)
				{
					CMultiLanguageString::GetLocalized(L"[0409]Click to add current color to last used colors.[0405]Kliknutím přidáte aktuální barvu mezi naposled použité.", m_tLocaleID, &bstrTmp);
				}
				else
				{
					TSwatchColor const& t = m_aColors[nColor];
					CConfigValue cRng;
					m_pColorsCfg->ItemValueGet(CComBSTR(L"Factor"), &cRng);
					CConfigValue cDec;
					m_pColorsCfg->ItemValueGet(CComBSTR(L"Decimal"), &cDec);
					float const fMul1 = cRng.operator float()*powf(10, cDec.operator LONG());
					float const fMul2 = powf(10, -cDec.operator LONG());
					float const fHMul1 = 10.0f;
					float const fHMul2 = 0.1f;
					float const f[4] =
					{
						t.eSpace == ESCSRGB ? int(t.f1*fMul1+0.5f)*fMul2 : int(t.f1*fHMul1+0.5f)*fHMul2,
						int(t.f2*fMul1+0.5f)*fMul2,
						int(t.f3*fMul1+0.5f)*fMul2,
						int(t.fA*fMul1+0.5f)*fMul2,
					};
					if (t.bstrName && SysStringLen(t.bstrName) < 420)
					{
						bstrTmp = t.bstrName;
						wchar_t sz[128] = L"";
						swprintf_s(sz, itemsof(sz), t.eSpace == ESCSRGB ? _T("\r\nR: %g\r\nG: %g\r\nB: %g\r\nA: %g") : _T("%s\r\nH: %g\r\nL: %g\r\nS: %g\r\nA: %g"), f[0], f[1], f[2], f[3]);
						bstrTmp += sz;
					}
					else
					{
						wchar_t sz[128] = L"";
						swprintf_s(sz, itemsof(sz), t.eSpace == ESCSRGB ? _T("R: %g\r\nG: %g\r\nB: %g\r\nA: %g") : _T("H: %g\r\nL: %g\r\nS: %g\r\nA: %g"), f[0], f[1], f[2], f[3]);
						bstrTmp = sz;
					}
				}
				m_tToolTip.lpszText = bstrTmp;
				m_wndTips.SetToolInfo(&m_tToolTip);
				{
					RECT rcSq;
					GetExtSquareRect(nColor, &rcSq);
					InvalidateRect(&rcSq, FALSE);
				}

				POINT pt =
				{
					nColor == HTActiveColor ? m_nSquareSize+m_nGapSize+2 :
					(m_aColors.empty() || !CompareColors(m_tActiveColor, m_aColors[0]) ? m_nGapSize+m_nSquareSize+2+m_nSquareSize+m_nSmallGapSize+m_nSmallGapSize+1+(m_nSquareSize+m_nSmallGapSize)*nColor :m_nGapSize+m_nSquareSize+2+(m_nSquareSize+m_nSmallGapSize)*nColor),
					m_nSquareSize+m_nGapSize+2
				}; 
				ClientToScreen(&pt);
				m_wndTips.TrackPosition(pt.x, pt.y);
				m_nLastColor = nColor;
			}
		}
		else if (m_nLastColor != HTEmptySpace)
		{
			RECT rcSq;
			GetExtSquareRect(m_nLastColor, &rcSq);
			InvalidateRect(&rcSq, FALSE);
			m_nLastColor = HTEmptySpace;
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		}
	}
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_nLastColor != HTEmptySpace)
		{
			RECT rcSq;
			GetExtSquareRect(m_nLastColor, &rcSq);
			InvalidateRect(&rcSq, FALSE);
			m_nLastColor = HTEmptySpace;
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		}
		m_bTrackingMouse = false;
		return 0;
	}
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		int nColor = ColorHitTest(pt);
		if (nColor == HTActiveColor)
		{
			AddColor(&m_tActiveColor);
		}
		else if (ULONG(nColor) < m_aColors.size())
		{
			m_tActiveColor = m_aColors[nColor];
			HWND hPar = GetParent();
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), CH_COLOR_CHANGED};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
			Invalidate(FALSE);
		}
		UpdateTooltip(pt);
		return 0;
	}

	void AddColor(TSwatchColor const* a_pColor)
	{
		CColors::iterator i;
		for (i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (CompareColors(*i, *a_pColor))
				break;
		bool bChange = false;
		if (i == m_aColors.end())
		{
			// add new color
			TSwatchColor t = *a_pColor;
			CComBSTR bstr(t.bstrName);
			t.bstrName = bstr;
			m_aColors.insert(m_aColors.begin(), t);
			bstr.Detach();
			bChange = true;
		}
		else if (i != m_aColors.begin())
		{
			// already in last colors, move it to top
			TSwatchColor t = *i;
			m_aColors.erase(i);
			m_aColors.insert(m_aColors.begin(), t);
			bChange = true;
		}
		// delete old colors
		while (m_aColors.size() > MAXLASTCOLORS)
		{
			BSTR bstr = m_aColors[m_aColors.size()-1].bstrName;
			if (bstr) SysFreeString(bstr);
			m_aColors.resize(m_aColors.size()-1);
			bChange = true;
		}
		if (bChange)
		{
			ColorsToCofig();
			Invalidate();
		}
	}


public:
	LCID m_tLocaleID;

private:
	int ColorHitTest(POINT a_pt)
	{
		if (a_pt.y < LONG(m_nGapSize) || a_pt.y >= LONG(m_nGapSize+m_nSquareSize) || a_pt.x < LONG(m_nGapSize))
			return HTEmptySpace;
		if (m_aColors.empty() || !CompareColors(m_tActiveColor, m_aColors[0]))
		{
			if (a_pt.x < LONG(m_nGapSize+m_nSquareSize))
				return HTActiveColor;
			a_pt.x -= m_nSmallGapSize+m_nSmallGapSize+m_nSquareSize+1;
			if (a_pt.x < LONG(m_nGapSize))
				return HTEmptySpace;
		}
		ULONG const nColor = (a_pt.x-m_nGapSize)/(m_nSquareSize+m_nSmallGapSize);
		ULONG const nReminder = (a_pt.x-m_nGapSize)%(m_nSquareSize+m_nSmallGapSize);
		if (nReminder >= m_nSquareSize || nColor >= m_aColors.size())
			return HTEmptySpace;
		return nColor;
	}
	void GetExtSquareRect(int a_nColor, RECT* a_prc)
	{
		a_prc->top = m_nGapSize-2;
		a_prc->bottom = m_nGapSize+m_nSquareSize+2;
		if (a_nColor == HTActiveColor)
		{
			a_prc->left = m_nGapSize-2;
			a_prc->right = m_nGapSize+m_nSquareSize+2;
		}
		else
		{
			a_prc->left = m_aColors.empty() || !CompareColors(m_tActiveColor, m_aColors[0]) ? m_nSmallGapSize+m_nSmallGapSize+1+m_nSquareSize : 0;
			a_prc->left += m_nGapSize-2+a_nColor*(m_nSmallGapSize+m_nSquareSize);
			a_prc->right = a_prc->left+m_nSquareSize+4;
		}
	}
	static bool CompareColors(TSwatchColor const& a_clr1, TSwatchColor const& a_clr2)
	{
		if (a_clr1.eSpace != a_clr2.eSpace)
			return false;
		if ((a_clr1.bstrName && !a_clr1.bstrName) || (!a_clr1.bstrName && a_clr1.bstrName) ||
			(a_clr1.bstrName && a_clr1.bstrName && wcscmp(a_clr1.bstrName, a_clr1.bstrName)))
			return false;
		if (fabsf(a_clr1.f1-a_clr2.f1) > 1e-3f || fabsf(a_clr1.f2-a_clr2.f2) > 1e-3f ||
			fabsf(a_clr1.f3-a_clr2.f3) > 1e-3f || fabsf(a_clr1.fA-a_clr2.fA) > 1e-3f)
			return false;
		return true;
	}
	void ColorsToCofig()
	{
		std::vector<wchar_t> cData;
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
		{
			TSwatchColor tColor = *i;
			wchar_t const* pChnls = tColor.eSpace == ESCSRGB ? L"RGBA" : L"HLSA";
			float pVals[4] = {tColor.f1, tColor.f2, tColor.f3, tColor.fA};
			static float const pDefs[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			wchar_t szTmp[128] = L""; // RG:0.5,B:1    G:1|Green
			wchar_t* psz = szTmp;
			for (int j = 0; j < 4; ++j)
			{
				if (pVals[j] != pDefs[j])
				{
					if (psz != szTmp)
						*(psz++) = L',';
					*(psz++) = pChnls[j];
					for (int k = j+1; k < 4; ++k)
					{
						if (pVals[k] != pDefs[k] && pVals[k] == pVals[j])
						{
							*(psz++) = pChnls[k];
							pVals[k] = pDefs[k];
						}
					}
					*(psz++) = L':';
					swprintf(psz, L"%g", pVals[j]);
					psz += wcslen(psz);
				}
			}
			if (tColor.bstrName && tColor.bstrName[0])
			{
				*(psz++) = L'|';
				for (wchar_t* psz2 = szTmp; psz2 < psz; ++psz2)
					cData.push_back(*psz2);
				for (wchar_t* psz2 = tColor.bstrName; *psz2; ++psz2)
					cData.push_back(*psz2);
				cData.push_back(L'\r');
				cData.push_back(L'\n');
			}
			else
			{
				if (i+1 != m_aColors.end())
				{
					*(psz++) = L'\r';
					*(psz++) = L'\n';
				}
				for (wchar_t* psz2 = szTmp; psz2 < psz; ++psz2)
					cData.push_back(*psz2);
			}
		}
		cData.push_back(L'\0');
		CComBSTR bstr(L"LastColors");
		m_pColorsCfg->ItemValuesSet(1, &(bstr.m_str), CConfigValue(&(cData[0])));
	}
	bool ParseColors()
	{
		CConfigValue cClrs;
		m_pColorsCfg->ItemValueGet(CComBSTR(L"LastColors"), &cClrs);
		if (cClrs == m_cClrs)
			return false;
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (i->bstrName)
				SysFreeString(i->bstrName);
		m_aColors.clear();
		if (cClrs.TypeGet() == ECVTString && cClrs.operator BSTR() && cClrs.operator BSTR()[0])
		{
			wchar_t const* psz = cClrs.operator BSTR();
			wchar_t const* pszEnd = psz+SysStringLen(cClrs.operator BSTR());
			TSwatchColor tColor;
			tColor.eSpace = ESCSRGB;
			tColor.bstrName = NULL;
			tColor.f1 = tColor.f2 = tColor.f3 = tColor.f4 = 0.0f;
			tColor.fA = 1.0f;
			float* aDsts[10];
			int nDsts = 0;
			while (psz < pszEnd)
			{
				switch (*psz)
				{
				case L'R':
					aDsts[nDsts++] = &tColor.f1;
					break;
				case L'G':
					aDsts[nDsts++] = &tColor.f2;
					break;
				case L'B':
					aDsts[nDsts++] = &tColor.f3;
					break;
				case L'A':
					aDsts[nDsts++] = &tColor.fA;
					break;
				case L'H':
					tColor.eSpace = ESCSHLS;
					aDsts[nDsts++] = &tColor.f1;
					break;
				case L'L':
					tColor.eSpace = ESCSHLS;
					aDsts[nDsts++] = &tColor.f2;
					break;
				case L'S':
					tColor.eSpace = ESCSHLS;
					aDsts[nDsts++] = &tColor.f3;
					break;
				case L',':
					break;
				case L'|':
					{
						wchar_t const* psz2 = psz+1;
						while (psz2 < pszEnd && *psz != L'\r' && *psz != L'\n')
							++psz2;
						if (psz2 > psz)
						{
							tColor.bstrName = SysAllocStringLen(psz, psz2-psz);
							psz = psz2;
						}
						else
						{
							break;
						}
					}
				case L'\r':
				case L'\n':
					if (psz+1 < pszEnd && psz[1] == *psz^('\r'^'\n'))
						++psz;
					m_aColors.push_back(tColor);
					nDsts = 0;
					tColor.eSpace = ESCSRGB;
					tColor.f1 = tColor.f2 = tColor.f3 = tColor.f4 = 0.0f;
					tColor.fA = 1.0f;
					tColor.bstrName = NULL;
					break;
				case L':':
					if (psz+1 < pszEnd)
						++psz;
				default:
					{
						wchar_t* pszNext = const_cast<wchar_t*>(psz);
						float f = wcstod(psz, &pszNext);
						if (pszNext > psz)
						{
							for (int i = 0; i < nDsts; ++i)
								*(aDsts[i]) = f;
							psz = pszNext-1;
						}
						nDsts = 0;
					}
					break;
				}
				++psz;
			}
			m_aColors.push_back(tColor);
		}
		m_cClrs = cClrs;
		return true;
	}

	void Data2GUI();
	void SendUpdate();

	typedef std::vector<TSwatchColor> CColors;

private:
	bool m_bEnableUpdates;
	CColors m_aColors;
	ULONG m_nSquareSize;
	ULONG m_nGapSize;
	ULONG m_nSmallGapSize;
	CToolTipCtrl m_wndTips;
	TTTOOLINFO m_tToolTip;
	bool m_bTrackingMouse;
	int m_nLastColor;
	CComPtr<IConfig> m_pColorsCfg;
	TSwatchColor m_tActiveColor;
	CConfigValue m_cClrs;
};
