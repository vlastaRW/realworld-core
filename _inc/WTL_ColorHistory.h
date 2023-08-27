// WTL_ColorHistory.h : Declaration of the CColorHistory

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"
#include <ObserverImpl.h>
#include <math.h>
#include <htmlhelp.h>
#include <MultiLanguageString.h>
#include <GammaCorrection.h>

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

// CColorHistory

class CColorHistory : 
	public CWindowImpl<CColorHistory>,
//	public CThemeImpl<CColorHistory>,
	public CObserverImpl<CColorHistory, IConfigObserver, IUnknown*>
{
public:
	CColorHistory() : m_bTrackingMouse(false), m_bHotColor(false)
	{
		//SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		//SetThemeClassList(L"ListView");
		m_tActiveColor.eSpace = ESCSRGB;
		m_tActiveColor.bstrName = NULL;
		m_tActiveColor.f1 = m_tActiveColor.f2 = m_tActiveColor.f3 = m_tActiveColor.f4 = 0.0f;
		m_tActiveColor.fA = 1.0f;
	}
	~CColorHistory()
	{
		if (m_pColorsCfg)
			m_pColorsCfg->ObserverDel(CObserverImpl<CColorHistory, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (i->bstrName)
				SysFreeString(i->bstrName);
	}
	enum { MAXLASTCOLORS = 24, CHN_COLORCHANGED = 43 };
	struct NMCOLORHISTORY
	{
		NMHDR hdr;
		TColor clr;
	};

	DECLARE_WND_CLASS_EX(_T("ColorHistory"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	void Init(HWND a_hParent, RECT const* a_rcWindow, LCID a_tLocaleID, UINT a_nID = 0)
	{
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		static GUID const tColorGCID = {0x2CBE06C7, 0x4847, 0x4766, {0xAA, 0x01, 0x22, 0x6A, 0xF5, 0x2D, 0x54, 0x88}}; // CLSID_DesignerViewFactoryColorSwatch
		pMgr->Config(tColorGCID, &m_pColorsCfg);
		//pMgr->Config(CLSID_DesignerViewFactoryColorSwatch, &m_pColorsCfg);
		m_pColorsCfg->ObserverIns(CObserverImpl<CColorHistory, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		ParseColors();

		m_tLocaleID = a_tLocaleID;

		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("ColorHistory"), WS_CHILD | WS_VISIBLE) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
		if (a_nID)
			SetWindowLong(GWLP_ID, a_nID);
	}

BEGIN_MSG_MAP(CColorHistory)
//	CHAIN_MSG_MAP(CThemeImpl<CColorHistory>)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
//	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
END_MSG_MAP()

public:
	void SetActiveColor(TColor const& a_tColor)
	{
		TSwatchColor t;
		t.eSpace = ESCSRGB;
		t.bstrName = NULL;
		t.f1 = a_tColor.fR;
		t.f2 = a_tColor.fG;
		t.f3 = a_tColor.fB;
		t.f4 = 0.0f;
		t.fA = a_tColor.fA;
		if (!CompareColors(t, m_tActiveColor))
		{
			m_tActiveColor = t;
			Invalidate(FALSE);
		}
	}

	void AddActiveColorToHistory()
	{
		AddColor(&m_tActiveColor);
	}

	void OwnerNotify(TCookie, IUnknown*)
	{
		if (m_hWnd == NULL)
			return;
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
		if (fR < 0.0f) fR = 0.0f; else if (fR > 1.0f) fR = 1.0f;
		if (fG < 0.0f) fG = 0.0f; else if (fG > 1.0f) fG = 1.0f;
		if (fB < 0.0f) fB = 0.0f; else if (fB > 1.0f) fB = 1.0f;
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
		LONG nOffset = (rcWnd.bottom-m_nSquareSize)>>1;
		RECT rc;
		rc.left = m_nGapSize;
		rc.top = nOffset;
		rc.bottom = nOffset+m_nSquareSize;
		COLORREF clr1 = GetSysColor(COLOR_3DLIGHT);
		COLORREF clr2 = GetSysColor(COLOR_3DSHADOW);
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
		{
			if (rc.left >= rcWnd.right)
				break;

			if (CompareColors(m_tActiveColor, *i))
				continue;

			rc.right = rc.left+m_nSquareSize;
			DrawSquare(clr1, clr2, a_cDC, rc, *i);
			if (m_bHotColor && CompareColors(m_tHotColor, *i))
				a_cDC.Draw3dRect(rc.left-2, rc.top-2, m_nSquareSize+4, m_nSquareSize+4, clr2, clr2);
			rc.left += m_nSquareSize+m_nGapSize;
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
		m_nGapSize = (nDPI+16)/32;
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

		RECT rc = {0, 0, 0, 0};
		GetClientRect(&rc);
		POINT pt = { GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam) };

		UpdateTooltip(rc, pt);

		return 0;
	}
	void UpdateTooltip(RECT const& a_rcWnd, POINT pt)
	{
		TSwatchColor tHotColor;
		bool bHotColor = ColorHitTest(a_rcWnd, pt, &tHotColor);

		if (bHotColor)
		{
			if (!m_bHotColor || !CompareColors(m_tHotColor, tHotColor))
			{
				if (!m_bHotColor)
				{
					// activate the ToolTip
					m_wndTips.TrackActivate(&m_tToolTip, TRUE);
				}
				else
				{
					RECT rcSq;
					GetExtSquareRect(a_rcWnd, m_tHotColor, &rcSq);
					InvalidateRect(&rcSq, FALSE);
				}
				CComBSTR bstrTip;
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
					tHotColor.eSpace == ESCSRGB ? int(tHotColor.f1*fMul1+0.5f)*fMul2 : int(tHotColor.f1*fHMul1+0.5f)*fHMul2,
					int(tHotColor.f2*fMul1+0.5f)*fMul2,
					int(tHotColor.f3*fMul1+0.5f)*fMul2,
					int(tHotColor.fA*fMul1+0.5f)*fMul2,
				};
				if (tHotColor.bstrName)
				{
					bstrTip += tHotColor.bstrName;
					bstrTip += L"\r\n";
				}
				wchar_t szTmp[128] = L"";
				_swprintf_p(szTmp, itemsof(szTmp), tHotColor.eSpace == ESCSRGB ? L"R: %g\r\nG: %g\r\nB: %g\r\nA: %g" : L"H: %g\r\nL: %g\r\nS: %g\r\nA: %g", f[0], f[1], f[2], f[3]);
				bstrTip += szTmp;

				m_tToolTip.lpszText = bstrTip;
				m_wndTips.SetToolInfo(&m_tToolTip);

				RECT rcSq;
				GetExtSquareRect(a_rcWnd, tHotColor, &rcSq);
				InvalidateRect(&rcSq, FALSE);

				POINT pt = { rcSq.right, rcSq.bottom };
				ClientToScreen(&pt);
				m_wndTips.TrackPosition(pt.x, pt.y);
				m_bHotColor = true;
				m_tHotColor = tHotColor;
			}
		}
		else if (m_bHotColor)
		{
			RECT rcSq;
			GetExtSquareRect(a_rcWnd, m_tHotColor, &rcSq);
			InvalidateRect(&rcSq, FALSE);
			m_bHotColor = false;
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		}
	}
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_bHotColor)
		{
			RECT rc = {0, 0, 0, 0};
			GetClientRect(&rc);
			RECT rcSq;
			GetExtSquareRect(rc, m_tHotColor, &rcSq);
			InvalidateRect(&rcSq, FALSE);
			m_bHotColor = false;
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		}
		m_bTrackingMouse = false;
		return 0;
	}
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		//HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		//if (pHelpInfo->iContextType == HELPINFO_WINDOW/* && pHelpInfo->iCtrlId >= 0*/)
		//{
		//	CComBSTR bstrBuffer;
		//	CMultiLanguageString::GetLocalized(L"[0409]This is a list of recently used colors. Click a color box to activate it. Right-click a color box to remove it from the list.[0405]Toto je seznam naposled použitých barev. Kliknìta na barevný ètvereèek pro aktivaci barvy. Kliknìte pravým pro odstranìní ze seznamu.", m_tLocaleID, &bstrBuffer);
		//	RECT rcItem;
		//	GetWindowRect(&rcItem);
		//	HH_POPUP hhp;
		//	hhp.cbStruct = sizeof(hhp);
		//	hhp.hinst = _pModule->get_m_hInst();
		//	hhp.idString = 0;
		//	hhp.pszText = bstrBuffer;
		//	hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		//	hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
		//	hhp.clrForeground = 0xffffffff;
		//	hhp.clrBackground = 0xffffffff;
		//	hhp.rcMargins.left = -1;
		//	hhp.rcMargins.top = -1;
		//	hhp.rcMargins.right = -1;
		//	hhp.rcMargins.bottom = -1;
		//	hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
		//	HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
		//	return 0;
		//}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rc = {0, 0, 0, 0};
		GetClientRect(&rc);
		POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		TSwatchColor clr;
		bool bColor = ColorHitTest(rc, pt, &clr);
		if (bColor)
		{
			TColor tClr;
			tClr.fA = clr.fA;
			if (clr.eSpace == ESCSHLS)
			{
				HLS2RGB(clr.f1, clr.f2, clr.f3, tClr.fR, tClr.fG, tClr.fB);
			}
			else
			{
				tClr.fR = clr.f1;
				tClr.fG = clr.f2;
				tClr.fB = clr.f3;
			}
			SetActiveColor(tClr);
			AddActiveColorToHistory();
			SendNotification(CHN_COLORCHANGED, tClr);
		}
		UpdateTooltip(rc, pt);
		return 0;
	}
	LRESULT OnRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		//RECT rc;
		//GetClientRect(&rc);
		//int nPerLine = GetPerLineCount(rc.right);
		//int nLines = (m_aColors.size()+nPerLine-1)/nPerLine;
		//int nLastLine = m_aColors.size()-(nLines-1)*nPerLine;
		//int x = GET_X_LPARAM(a_lParam)/m_nSquareSize;
		//int y = (GET_Y_LPARAM(a_lParam)+m_ptOffset.y)/m_nSquareSize;
		//if (x >= nPerLine || y >= nLines || (y == nLines-1 && x >= nLastLine))
		//	return 0;
		//int nClr = x+y*nPerLine;
		//ATLASSERT(nClr < m_aColors.size());
		//BSTR bstr = m_aColors[nClr].bstrName;
		//m_aColors.erase(m_aColors.begin()+nClr);
		//if (bstr) SysFreeString(bstr);
		//if (m_nLastColor >= nClr)
		//	--m_nLastColor;
		//Invalidate();
		//SetScrollSize(nPerLine*m_nSquareSize, ((m_aColors.size()+nPerLine-1)/nPerLine)*m_nSquareSize);
		//POINT tPt;
		//GetCursorPos(&tPt);
		//ScreenToClient(&tPt);
		//BOOL b;
		//OnMouseMove(WM_MOUSEMOVE, 0, MAKELPARAM(tPt.x, tPt.y), b);
		return 0;
	}
//	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		if (a_pSize)
		{
			a_pSize->cx = m_nSquareSize*16+m_nGapSize*15;
			a_pSize->cy = m_nSquareSize+m_nGapSize+m_nGapSize+4;
		}
		return S_OK;
	}

//	// IDesignerViewStatusBar methods
//public:
//	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
//	{
//		try
//		{
//			if (m_bTrackingMouse && m_nLastColor >= 0)
//			{
//				WCHAR szBuffer[1024] = L"";
//				Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_HELP_COLORSWATCH, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
//				a_pStatusBar->SimpleModeSet(CComBSTR(szBuffer));
//			}
//			return S_OK;
//		}
//		catch (...)
//		{
//			return E_UNEXPECTED;
//		}
//	}

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
	void SendNotification(UINT a_nCode, TColor const& a_tColor)
	{
		NMCOLORHISTORY nmch;

		nmch.hdr.code = a_nCode;
		nmch.hdr.hwndFrom = m_hWnd;
		nmch.hdr.idFrom = GetDlgCtrlID();
		nmch.clr = a_tColor;

		GetParent().SendMessage(WM_NOTIFY, (WPARAM)nmch.hdr.idFrom, (LPARAM)&nmch);
	}

	bool ColorHitTest(RECT const& a_rcWnd, POINT a_pt, TSwatchColor* a_pHitColor)
	{
		LONG nBase = (a_rcWnd.bottom-m_nSquareSize)>>1;
		if (a_pt.y < nBase || a_pt.y >= LONG(nBase+m_nSquareSize) || a_pt.x < LONG(m_nGapSize))
			return false;
		ULONG nColor = (a_pt.x-m_nGapSize)/(m_nSquareSize+m_nGapSize);
		ULONG const nReminder = (a_pt.x-m_nGapSize)%(m_nSquareSize+m_nGapSize);
		if (nReminder >= m_nSquareSize || nColor >= m_aColors.size())
			return false;
		for (ULONG i = 0; i <= nColor; ++i)
			if (CompareColors(m_aColors[i], m_tActiveColor))
			{
				++nColor;
				if (nColor == m_aColors.size())
					return false;
				break;
			}
		*a_pHitColor = m_aColors[nColor];
		return true;
	}
	void GetExtSquareRect(RECT const& a_rcWnd, TSwatchColor const& a_tColor, RECT* a_prc)
	{
		int j = 0;
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (CompareColors(*i, a_tColor))
				break;
			else if (!CompareColors(*i, m_tActiveColor))
				++j;
		a_prc->top = ((a_rcWnd.bottom-m_nSquareSize)>>1)-2;
		a_prc->bottom = a_prc->top+m_nSquareSize+4;
		a_prc->left = m_nGapSize-2+j*(m_nGapSize+m_nSquareSize);
		a_prc->right = a_prc->left+m_nSquareSize+4;
	}
	static bool CompareColors(TSwatchColor const& a_clr1, TSwatchColor const& a_clr2)
	{
		if (a_clr1.eSpace != a_clr2.eSpace)
			return false;
		if ((a_clr1.bstrName && !a_clr1.bstrName) || (!a_clr1.bstrName && a_clr1.bstrName) ||
			(a_clr1.bstrName && a_clr1.bstrName && wcscmp(a_clr1.bstrName, a_clr1.bstrName)))
			return false;
		if (fabsf(a_clr1.f1-a_clr2.f1) > 1e-4f || fabsf(a_clr1.f2-a_clr2.f2) > 1e-4f ||
			fabsf(a_clr1.f3-a_clr2.f3) > 1e-4f || fabsf(a_clr1.fA-a_clr2.fA) > 1e-4f)
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
	CColors m_aColors;
	ULONG m_nSquareSize;
	ULONG m_nGapSize;
	CToolTipCtrl m_wndTips;
	TTTOOLINFO m_tToolTip;
	bool m_bTrackingMouse;
	bool m_bHotColor;
	TSwatchColor m_tHotColor;
	TSwatchColor m_tActiveColor;
	CComPtr<IConfig> m_pColorsCfg;
	CConfigValue m_cClrs;
};

