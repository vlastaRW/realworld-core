
#pragma once

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
//#include <agg_conv_dash.h>
#include <agg_path_storage.h>
//#include <agg_ellipse.h>
#include <agg_span_allocator.h>
#include <agg_gamma_lut.h>

#include <GammaCorrection.h>


namespace WTL
{

class CGradientColorPickerBase :
	public CWindowImpl<CGradientColorPickerBase>,
	public CThemeImpl<CGradientColorPickerBase>
{
public:
	enum
	{
		GCPN_ACTIVESTOPCHANGED = 37,
	};
	struct NMGRADIENT
	{
		NMHDR hdr;
		WORD pos;
		CButtonColorPicker::SColor clr;
	};

	CGradientColorPickerBase(bool a_bDrawSep = false, int a_nSepMargins = 0) :
		m_fScale(0.0f), m_wSelected(0), m_bDrawSep(a_bDrawSep), m_nSepMargins(a_nSepMargins)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");

		m_cGradient[0] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
		m_cGradient[0xffff] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
	void SetMargin(int a_nSepMargins)
	{
		m_nSepMargins = a_nSepMargins;
		if (m_hWnd) Invalidate(FALSE);
	}

	typedef std::map<WORD, CButtonColorPicker::SColor> CGradient;

	DECLARE_WND_CLASS_EX(_T("GradientColorPickerBase"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CGradientColorPickerBase)
		CHAIN_MSG_MAP(CThemeImpl<CGradientColorPickerBase>)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocusChange)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnFocusChange)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()

public:
	static void ParseGradient(LPCWSTR psz, CGradient& cGrad)
	{
		while (*psz)
		{
			LPWSTR pNext = const_cast<LPWSTR>(psz); // c-lib problem
			double fPos = wcstod(psz, &pNext);
			if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
			float fR = wcstod(psz, &pNext);
			if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
			float fG = wcstod(psz, &pNext);
			if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
			float fB = wcstod(psz, &pNext);
			if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
			float fA = wcstod(psz, &pNext);
			if (pNext == psz) break; else if (*pNext == L';') psz = pNext+1; else psz = pNext;
			cGrad[WORD(fPos)] = CButtonColorPicker::SColor(fR, fG, fB, fA);
		}
	}
	static void FixGradient(CGradient& cGrad)
	{
		if (cGrad.size() < 2)
		{
			cGrad.clear();
			cGrad[WORD(0)] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
			cGrad[WORD(0xffff)] = CButtonColorPicker::SColor(1.0f, 1.0f, 1.0f, 1.0f);
			return;
		}
		if (cGrad.begin()->first != 0)
			cGrad[0] = cGrad.begin()->second;
		if (cGrad.rbegin()->first != 0xffff)
			cGrad[0xffff] = cGrad.rbegin()->second;
	}
	static void SerializeGradient(CGradient const& g, CComBSTR& bstr)
	{
		for (CGradient::const_iterator i = g.begin(); i != g.end(); ++i)
		{
			wchar_t psz[128];
			swprintf(psz, L"%i,%g,%g,%g,%g;", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA);
			bstr.Append(psz);
		}
	}

	// operations
public:
	CGradient const& GetGradient() const
	{
		return m_cGradient;
	}
	bool SetGradient(CGradient const& a_cGradient, int a_nSel = -1, bool a_bSendNotification = false)
	{
		if (a_cGradient.size() < 2 || a_cGradient.begin()->first != 0 || a_cGradient.rbegin()->first != 0xffff)
			return false; // invalid gradient - lacking end point or start point
		if (m_cGradient != a_cGradient)
		{
			m_cGradient = a_cGradient;
			if (a_nSel >= 0 && a_nSel <= int(0xffff))
				m_wSelected = WORD(a_nSel);
			CGradient::const_iterator iSel = m_cGradient.find(m_wSelected);
			if (iSel == m_cGradient.end())
			{
				m_wSelected = m_cGradient.begin()->first;
				iSel = m_cGradient.begin();
			}
			if (a_bSendNotification)
				SendNotification(GCPN_ACTIVESTOPCHANGED, m_wSelected, iSel->second);
			Invalidate(FALSE);
		}
		return true;
	}
	void SetStop(WORD a_wPos, CButtonColorPicker::SColor const& a_sColor)
	{
		CButtonColorPicker::SColor sColor = a_sColor;
		sColor.eCT = CButtonColorPicker::ECTNormal;
		CGradient::iterator i = m_cGradient.find(a_wPos);
		if (i == m_cGradient.end())
		{
			m_cGradient[a_wPos] = sColor;
			SendNotification(GCPN_ACTIVESTOPCHANGED, a_wPos, sColor);
			Invalidate(FALSE);
		}
		else
		{
			if (i->second != sColor)
			{
				i->second = sColor;
				SendNotification(GCPN_ACTIVESTOPCHANGED, a_wPos, sColor);
				Invalidate(FALSE);
			}
		}
	}
	bool MoveStop(WORD a_wOldPos, WORD a_wNewPos)
	{
		if (a_wOldPos == a_wNewPos || a_wOldPos == 0 || a_wOldPos == 0xffff)
			return false;
		CGradient::iterator i = m_cGradient.find(a_wOldPos);
		if (i == m_cGradient.end())
			return false;
		CButtonColorPicker::SColor sColor = i->second;
		m_cGradient.erase(i);
		m_cGradient[a_wNewPos] = sColor;
		if (a_wOldPos == m_wSelected)
			m_wSelected = a_wNewPos;
		SendNotification(GCPN_ACTIVESTOPCHANGED, a_wNewPos, sColor);
		Invalidate(FALSE);
		return true;
	}
	WORD GetStop() const
	{
		return m_wSelected;
	}
	CButtonColorPicker::SColor const& GetStopColor(WORD a_wPos) const
	{
		CGradient::const_iterator i = m_cGradient.find(a_wPos);
		static CButtonColorPicker::SColor const sDefault(CButtonColorPicker::ECTDefault);
		return i == m_cGradient.end() ? sDefault : i->second;
	}
	bool GetStopRect(WORD a_wPos, RECT* a_prc)
	{
		CGradient::iterator i = m_cGradient.find(a_wPos);
		if (i == m_cGradient.end())
			return false;
		RECT rcWin;
		GetClientRect(&rcWin);
		int nBorderWidth = 2*m_fScale;
		int nSelSide = 5*m_fScale;
		int nSelPtrX = nSelSide*2+1;
		int nSelPtrY = nSelSide*3+1;
		int nGradSizeX = rcWin.right-(nSelSide<<1)-(m_nSepMargins<<1);
		int nGradSizeY = 16*m_fScale;
		int nGradOffX = nSelSide+m_nSepMargins;
		int nGradOffY = nBorderWidth;

		LONG nXC = (i->first*(nGradSizeX-1)+0x7fff)/0xffff + nGradOffX;
		RECT rcThumb = {nXC-nSelSide, nGradOffY+(nGradSizeY>>1), nXC+1+nSelSide, nGradOffY+(nGradSizeY>>1)+nSelPtrY};
		if (m_bDrawSep && i->first == m_wSelected)
		{
			rcThumb.left -= nSelSide>>1;
			rcThumb.right += nSelSide>>1;
		}
		*a_prc = rcThumb;
		return true;
	}
	bool StopFromPoint(POINT p, WORD* a_pStop = NULL)
	{
		int nStopCenter = 0;
		CGradient::iterator i = StopHitTest(p, &nStopCenter);
		if (i == m_cGradient.end())
			return false;
		if (a_pStop)
			*a_pStop = i->first;
		return true;
	}


	template<typename TOutIterator>
	static void RenderGradient(CGradient const& a_aGradient, ULONG a_nTotal, ULONG a_nBegin, ULONG a_nEnd, TOutIterator a_tOut)
	{
		if (a_nBegin >= a_nEnd || a_nEnd > a_nTotal)
			return;

		CGradient::const_iterator i = a_aGradient.begin();
		WORD nB = a_nBegin*0xffff/(a_nTotal-1);
		while (true)
		{
			CGradient::const_iterator j = i;
			++j;
			if (j == a_aGradient.end() || j->first > nB)
				break;
			i = j;
		}
		while (a_nBegin < a_nEnd)
		{
			if (i->first == nB)
			{
				*a_tOut = i->second;
			}
			else
			{
				CGradient::const_iterator j = i;
				++j;
				float const fB1 = float(nB-i->first)/float(j->first-i->first);
				float const fB2 = 1.0f-fB1;
				float const fA = i->second.fA*fB2 + j->second.fA*fB1;
				if (fA <= 0.0f)
				{
					*a_tOut = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f);
				}
				else
				{
					float fIA = 1.0f/fA;
					float const fR = (i->second.fR*i->second.fA*fB2 + j->second.fR*j->second.fA*fB1)*fIA;
					float const fG = (i->second.fG*i->second.fA*fB2 + j->second.fG*j->second.fA*fB1)*fIA;
					float const fB = (i->second.fB*i->second.fA*fB2 + j->second.fB*j->second.fA*fB1)*fIA;
					*a_tOut = CButtonColorPicker::SColor(fR, fG, fB, fA);
				}
			}
			++a_tOut;
			++a_nBegin;
			nB = a_nBegin*0xffff/(a_nTotal-1);
			while (true)
			{
				CGradient::const_iterator j = i;
				++j;
				if (j == a_aGradient.end() || j->first > nB)
					break;
				i = j;
			}
		}
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hdc = GetDC();
		m_fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnFocusChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT p = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		int nStopCenter = 0;
		CGradient::iterator i = StopHitTest(p, &nStopCenter);
		if (i == m_cGradient.end())
		{
			WORD w;
			if (PositionFromPoint(p, &w) && m_cGradient.find(w) == m_cGradient.end())
			{
				CButtonColorPicker::SColor sColor;
				RenderGradient(m_cGradient, 0xffff, w, w+1, &sColor);
				m_cGradient[w] = sColor;
				m_wSelected = w;
				SendNotification(GCPN_ACTIVESTOPCHANGED, w, sColor);
				Invalidate(FALSE);
				m_cGradientBackup = m_cGradient;
				m_wSelectedBackup = m_wSelected;
				SetCapture();
				m_nDragOffset = 0;
			}
		}
		else
		{
			if (m_wSelected != i->first)
			{
				m_wSelected = i->first;
				SendNotification(GCPN_ACTIVESTOPCHANGED, i->first, i->second);
				Invalidate(FALSE);
			}
			if (m_wSelected != 0 && m_wSelected != 0xffff)
			{
				m_cGradientBackup = m_cGradient;
				m_wSelectedBackup = m_wSelected;
				m_bRescaling = (MK_CONTROL|MK_SHIFT)&a_wParam;
				SetCapture();
			}
			m_nDragOffset = p.x-nStopCenter;
		}
		return 0;
	}

	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(&p);
		CGradient::iterator i = StopHitTest(p);
		if (i == m_cGradient.end())
		{
			static HCURSOR hArrow = ::LoadCursor(NULL, IDC_ARROW);
			SetCursor(hArrow);
			return TRUE;
		}
		else
		{
			static HCURSOR hHand = ::LoadCursor(NULL, IDC_HAND);
			SetCursor(hHand);
			return TRUE;
		}
	}
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			POINT p = {GET_X_LPARAM(a_lParam)-m_nDragOffset, GET_Y_LPARAM(a_lParam)};
			WORD w;
			bool bValid = PositionFromPoint(p, &w, true);
			if ((m_cGradient.size() < m_cGradientBackup.size() && bValid) ||
				(m_cGradient.size() == m_cGradientBackup.size() && (!bValid || w != m_wSelected)))
			{
				if (bValid)
				{
					if (m_bRescaling)
					{
						m_cGradient.clear();
						if (w < m_wSelectedBackup)
						{
							WORD wMin = 0;
							for (CGradient::const_iterator i = m_cGradientBackup.begin(); i != m_cGradientBackup.end(); ++i)
							{
								WORD wComp = i->first <= m_wSelectedBackup ? DWORD(i->first)*w/m_wSelectedBackup : 0x0000ffff-DWORD((0x0000ffff-i->first)*DWORD(0x0000ffff-w)/DWORD(0x0000ffff-m_wSelectedBackup));
								WORD ww = max(wMin, wComp);
								m_cGradient[ww] = i->second;
								wMin = ww+1;
								if (i->first == m_wSelectedBackup)
									m_wSelected = ww;
							}
						}
						else
						{
							WORD wMax = 0xffff;
							for (CGradient::const_reverse_iterator i = m_cGradientBackup.rbegin(); i != m_cGradientBackup.rend(); ++i)
							{
								WORD wComp = i->first <= m_wSelectedBackup ? DWORD(i->first)*w/m_wSelectedBackup : 0x0000ffff-DWORD((0x0000ffff-i->first)*DWORD(0x0000ffff-w)/DWORD(0x0000ffff-m_wSelectedBackup));
								WORD ww = min(wMax, wComp);
								m_cGradient[ww] = i->second;
								wMax = ww-1;
								if (i->first == m_wSelectedBackup)
									m_wSelected = ww;
							}
						}
					}
					else
					{
						WORD wEnd = m_cGradient.size() < m_cGradientBackup.size() ? m_wSelectedBackup : m_wSelected;
						m_cGradient = m_cGradientBackup;
						m_cGradient.erase(m_wSelectedBackup);

						CGradient::const_iterator i = m_cGradient.find(w);
						if (i != m_cGradient.end())
						{
							if (w > m_wSelected)
							{
								while (w != wEnd && i->first == w)
								{
									--i;
									--w;
								}
							}
							else
							{
								while (w != wEnd && i->first == w)
								{
									++i;
									++w;
								}
							}
						}
						m_cGradient[w] = m_cGradientBackup[m_wSelectedBackup];
						m_wSelected = w;
					}
				}
				else
				{
					CGradient::const_iterator i = m_cGradient.size() <= m_cGradientBackup.size() ? m_cGradientBackup.find(m_wSelectedBackup) : m_cGradient.find(m_wSelected);
					CGradient::const_iterator iB = i; --iB;
					CGradient::const_iterator iN = i; ++iN;
					m_wSelected = iN->first-i->first < i->first-iB->first ? iN->first : iB->first;
					m_cGradient = m_cGradientBackup;
					m_cGradient.erase(m_wSelectedBackup);
				}

				SendNotification(GCPN_ACTIVESTOPCHANGED, m_wSelected, m_cGradient[m_wSelected]);
				Invalidate(FALSE); // TODO: only invalidate changed region
			}
		}
		return 0;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
			return 0; // no keyboard input accepted while dragging with mouse
		switch (a_wParam)
		{
		case VK_LEFT:
			if (m_wSelected == 0 || m_wSelected == 0xffff)
				return 0;
			{
				WORD wNew = m_wSelected < 0x101 ? 1 : m_wSelected-0x100;
				while (m_cGradient.find(wNew) != m_cGradient.end() && wNew != m_wSelected) ++wNew;
				if (wNew != m_wSelected)
				{
					m_cGradient[wNew] = m_cGradient[m_wSelected];
					m_cGradient.erase(m_wSelected);
					m_wSelected = wNew;
					SendNotification(GCPN_ACTIVESTOPCHANGED, m_wSelected, m_cGradient[m_wSelected]);
					Invalidate(FALSE); // TODO: only invalidate changed region
				}
			}
			break;
		case VK_RIGHT:
			if (m_wSelected == 0 || m_wSelected == 0xffff)
				return 0;
			{
				WORD wNew = m_wSelected > 0xfefd ? 0xfffe : m_wSelected+0x100;
				while (m_cGradient.find(wNew) != m_cGradient.end() && wNew != m_wSelected) --wNew;
				if (wNew != m_wSelected)
				{
					m_cGradient[wNew] = m_cGradient[m_wSelected];
					m_cGradient.erase(m_wSelected);
					m_wSelected = wNew;
					SendNotification(GCPN_ACTIVESTOPCHANGED, m_wSelected, m_cGradient[m_wSelected]);
					Invalidate(FALSE); // TODO: only invalidate changed region
				}
			}
			break;
		case VK_UP:
			if (m_wSelected == 0)
				return 0;
			{
				CGradient::iterator i = m_cGradient.find(m_wSelected);
				CGradient::iterator i2 = i;
				--i2;
				m_wSelected = i2->first;
				SendNotification(GCPN_ACTIVESTOPCHANGED, m_wSelected, m_cGradient[m_wSelected]);
				Invalidate(FALSE); // TODO: only invalidate changed region
			}
			break;
		case VK_DOWN:
			if (m_wSelected == 0xffff)
				return 0;
			{
				CGradient::iterator i = m_cGradient.find(m_wSelected);
				CGradient::iterator i2 = i;
				++i2;
				m_wSelected = i2->first;
				SendNotification(GCPN_ACTIVESTOPCHANGED, m_wSelected, m_cGradient[m_wSelected]);
				Invalidate(FALSE); // TODO: only invalidate changed region
			}
			break;
		case VK_DELETE:
			if (m_wSelected == 0 || m_wSelected == 0xffff)
				return 0;
			{
				CGradient::const_iterator i = m_cGradient.find(m_wSelected);
				CGradient::const_iterator iB = i; --iB;
				CGradient::const_iterator iN = i; ++iN;
				m_wSelected = iN->first-i->first < i->first-iB->first ? iN->first : iB->first;
				m_cGradient.erase(i);
				SendNotification(GCPN_ACTIVESTOPCHANGED, m_wSelected, m_cGradient[m_wSelected]);
				Invalidate(FALSE); // TODO: only invalidate changed region
			}
			break;
		}
		return 0;
	}

	LRESULT OnGetDlgCode(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return DLGC_WANTARROWS;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
			return lRes;
		SetFocus();
		return MA_ACTIVATE;
	}

	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		// get window style
		DWORD dwStyle = GetWindowLong(GWL_STYLE);
		bool const bFocus = m_hWnd == GetFocus();

		RECT rcWin;
		GetClientRect(&rcWin);

		COLORREF clrPen = GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrBG = GetSysColor(COLOR_WINDOW);
		COLORREF clrBorder = GetSysColor(COLOR_ACTIVEBORDER);
		if (dwStyle & WS_DISABLED)
		{
			clrPen = (((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
					 (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
					 (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		}
		if (!bFocus)
		{
			clrPen = (((clrPen&0xfe)>>1)+((clrBorder&0xfe)>>1)) |
					 (((clrPen&0xfe00)>>1)+((clrBorder&0xfe00)>>1)) |
					 (((clrPen&0xfe0000)>>1)+((clrBorder&0xfe0000)>>1));
		}
		//COLORREF clrFill = max(clrPen&0xff, clrBG&0xff) | //(((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
		//				   (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
		//				   (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));

		PAINTSTRUCT ps;
		CDCHandle cDC(BeginPaint(&ps));

		BITMAPINFO tBMI;
		ZeroMemory(&tBMI, sizeof tBMI);
		tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
		tBMI.bmiHeader.biWidth = ps.rcPaint.right-ps.rcPaint.left;
		tBMI.bmiHeader.biHeight = ps.rcPaint.bottom-ps.rcPaint.top;
		tBMI.bmiHeader.biPlanes = 1;
		tBMI.bmiHeader.biBitCount = 32;
		tBMI.bmiHeader.biCompression = BI_RGB;

		BYTE* pData = NULL;
		HBITMAP hBmp = CreateDIBSection(cDC, &tBMI, DIB_RGB_COLORS, reinterpret_cast<void**>(&pData), NULL, 0);
		if (pData == NULL)
		{
			EndPaint(&ps);
			return 0;
		}

		CDC cDCPaint;
		cDCPaint.CreateCompatibleDC(cDC);
		hBmp = cDCPaint.SelectBitmap(hBmp);

		RECT rcPaint = {0, 0, ps.rcPaint.right-ps.rcPaint.left, ps.rcPaint.bottom-ps.rcPaint.top};

		FillMemory(pData, 4*rcPaint.right*rcPaint.bottom, 0xff); // force alpha to 255

		if (IsThemingSupported())
		{
			DrawThemeParentBackground(cDCPaint, &rcPaint);
		}
		else
		{
			HBRUSH hBrush = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORDLG, (WPARAM)cDCPaint.m_hDC, 0);
			if (hBrush)
				cDCPaint.FillRect(&rcPaint, hBrush);
			else
				cDCPaint.FillSolidRect(&rcPaint, clrBG);
		}
		GdiFlush();
		BYTE const aBackground[] = {pData[2], pData[1], pData[0]};

		int nBorderWidth = 2*m_fScale;
		int nSelSide = 5*m_fScale;
		int nSelPtrX = nSelSide*2+1;
		int nSelPtrY = nSelSide*3+1;
		int nGradSizeX = rcWin.right-(nSelSide<<1)-(m_nSepMargins<<1);
		int nGradSizeY = 16*m_fScale;
		int nGradOffX = nSelSide+m_nSepMargins;
		int nGradOffY = nBorderWidth;
		RECT rcGradient = {nGradOffX, nGradOffY, nGradOffX+nGradSizeX, nGradOffY+nGradSizeY};

		agg::rendering_buffer rbuf;
		rbuf.attach(pData, rcPaint.right, rcPaint.bottom, -rcPaint.right*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
		// Pixel format and basic primitives renderer
		agg::pixfmt_bgra32 pixf(rbuf);
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		//renb.clear(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
		// Scanline renderer for solid filling.
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		// Rasterizer & scanline
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		if (m_bDrawSep)
		{
			agg::span_allocator<agg::rgba8> span_alloc;
			agg::scanline_p8 sl;

			ras.move_to_d(-ps.rcPaint.left, rcWin.bottom-nGradSizeY-ps.rcPaint.top);
			ras.line_to_d(rcWin.right-ps.rcPaint.left, rcWin.bottom-nGradSizeY-ps.rcPaint.top);
			ras.line_to_d(rcWin.right-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top);
			ras.line_to_d(-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top);
			ras.close_polygon();

			COLORREF clrLine = GetSysColor(COLOR_3DSHADOW);
			BYTE bSepLine[] = {GetRValue(clrLine), GetGValue(clrLine), GetBValue(clrLine), 255};
			COLORREF clrWindow = GetSysColor(COLOR_WINDOW);
			BYTE bBG2[] = {GetRValue(clrWindow), GetGValue(clrWindow), GetBValue(clrWindow), 255};
			agg::render_scanlines_aa(ras, sl, renb, span_alloc, CBackgroundSep(bSepLine, bBG2, nGradSizeY, rcWin.bottom-1-ps.rcPaint.top, -ps.rcPaint.left, rcWin.right));
		}

		//// gamma correction
		//agg::gamma_lut<agg::int8u, agg::int8u, 8, 8> gp(2.2);
		//pixf.apply_gamma_dir(gp);

		// draw gradient border
		{
			agg::path_storage path1;
			path1.move_to(nGradOffX-0.5*nBorderWidth-ps.rcPaint.left, 0.5*nBorderWidth-ps.rcPaint.top);
			path1.line_to(rcWin.right-(nGradOffX-0.5*nBorderWidth)-ps.rcPaint.left, 0.5*nBorderWidth-ps.rcPaint.top);
			path1.line_to(rcWin.right-(nGradOffX-0.5*nBorderWidth)-ps.rcPaint.left, nGradSizeY+nBorderWidth*1.5-ps.rcPaint.top);
			path1.line_to(nGradOffX-0.5*nBorderWidth-ps.rcPaint.left, nGradSizeY+nBorderWidth*1.5-ps.rcPaint.top);
			path1.close_polygon();
			agg::conv_stroke<agg::path_storage> stroke(path1);
			stroke.line_join(agg::miter_join);
			stroke.width(nBorderWidth);
			ren.color(agg::rgba8(GetRValue(clrBorder), GetGValue(clrBorder), GetBValue(clrBorder), 255));
			ras.add_path(stroke);
			agg::render_scanlines(ras, sl, ren);
		}
		{
			agg::path_storage path1;
			path1.move_to(nGradOffX-0.25*nBorderWidth-ps.rcPaint.left, 0.75*nBorderWidth-ps.rcPaint.top);
			path1.line_to(rcWin.right-(nGradOffX-0.25*nBorderWidth)-ps.rcPaint.left, 0.75*nBorderWidth-ps.rcPaint.top);
			path1.line_to(rcWin.right-(nGradOffX-0.25*nBorderWidth)-ps.rcPaint.left, nGradSizeY+nBorderWidth*1.25-ps.rcPaint.top);
			path1.line_to(nGradOffX-0.25*nBorderWidth-ps.rcPaint.left, nGradSizeY+nBorderWidth*1.25-ps.rcPaint.top);
			path1.close_polygon();
			agg::conv_stroke<agg::path_storage> stroke(path1);
			stroke.line_join(agg::miter_join);
			stroke.width(0.5*nBorderWidth);
			ren.color(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
			ras.add_path(stroke);
			agg::render_scanlines(ras, sl, ren);
		}

		COLORREF clr1 = GetSysColor(COLOR_3DLIGHT);
		COLORREF clr2 = GetSysColor(COLOR_3DSHADOW);
		ULONG const nSquare = 4*m_fScale;
		float const fRGBs[] =
		{
			CGammaTables::FromSRGB(GetRValue(clr1)),
			CGammaTables::FromSRGB(GetGValue(clr1)),
			CGammaTables::FromSRGB(GetBValue(clr1)),
			CGammaTables::FromSRGB(GetRValue(clr2)),
			CGammaTables::FromSRGB(GetGValue(clr2)),
			CGammaTables::FromSRGB(GetBValue(clr2)),
		};

		// draw gradient
		if (ps.rcPaint.left < rcGradient.right && ps.rcPaint.top < rcGradient.bottom &&
			rcGradient.left < ps.rcPaint.right && rcGradient.top < ps.rcPaint.bottom)
		{
			RECT r = {max(ps.rcPaint.left, rcGradient.left), max(ps.rcPaint.top, rcGradient.top), min(ps.rcPaint.right, rcGradient.right), min(ps.rcPaint.bottom, rcGradient.bottom)};
			if (r.right > r.left && r.bottom > r.top)
			{
				CAutoVectorPtr<CButtonColorPicker::SColor> aColors(new CButtonColorPicker::SColor[r.right-r.left]);
				RenderGradient(m_cGradient, nGradSizeX, r.left-rcGradient.left, r.right-rcGradient.left, aColors.m_p);
				CButtonColorPicker::SColor const* pColor = aColors;
				BYTE* pD = pData + ((r.left-ps.rcPaint.left + rcPaint.right*(rcPaint.bottom-1-r.top+ps.rcPaint.top))<<2);
				for (LONG x = r.left; x < r.right; ++x, ++pColor, pD+=4)
				{
					BYTE* pD2 = pD;
					if (pColor->fA >= 1.0f)
					{
						COLORREF clr = aColors[x-r.left].ToCOLORREF();
						BYTE bR = GetRValue(clr);
						BYTE bG = GetGValue(clr);
						BYTE bB = GetBValue(clr);
						for (LONG y = r.top; y < r.bottom; ++y, pD2 -= rcPaint.right<<2)
						{
							pD2[2] = bR;
							pD2[1] = bG;
							pD2[0] = bB;
						}
					}
					else
					{
						//LONG n12 = (x/nSquare)&1;
						float const fR = pColor->fR <= 0.0f ? 0.0f : (pColor->fR >= 1.0f ? 1.0f : pColor->fR);
						float const fG = pColor->fG <= 0.0f ? 0.0f : (pColor->fG >= 1.0f ? 1.0f : pColor->fG);
						float const fB = pColor->fB <= 0.0f ? 0.0f : (pColor->fB >= 1.0f ? 1.0f : pColor->fB);
						float const fA = pColor->fA <= 0.0f ? 0.0f : pColor->fA;
						float const fIA = 1.0f-fA;
						BYTE const clrs[] =
						{
							CGammaTables::ToSRGB(fRGBs[0]*fIA + fR*fA),
							CGammaTables::ToSRGB(fRGBs[1]*fIA + fG*fA),
							CGammaTables::ToSRGB(fRGBs[2]*fIA + fB*fA),
							0,
							CGammaTables::ToSRGB(fRGBs[3]*fIA + fR*fA),
							CGammaTables::ToSRGB(fRGBs[4]*fIA + fG*fA),
							CGammaTables::ToSRGB(fRGBs[5]*fIA + fB*fA),
							0,
						};
						for (LONG y = r.top; y < r.bottom; ++y, pD2 -= rcPaint.right<<2)
						{
							BYTE const* pS = clrs+((((x-rcGradient.left)/nSquare+(y-rcGradient.top)/nSquare)&1)<<2);
							pD2[2] = pS[0];
							pD2[1] = pS[1];
							pD2[0] = pS[2];
						}
					}
				}
			}
		}

		// draw thumbs
		for (CGradient::const_iterator i = m_cGradient.begin(); i != m_cGradient.end(); ++i)
		{
			LONG nXC = (i->first*(nGradSizeX-1)+0x7fff)/0xffff + nGradOffX;
			RECT rcThumb = {nXC-nSelSide, (rcGradient.bottom+rcGradient.top)>>1, nXC+1+nSelSide, ((rcGradient.bottom+rcGradient.top)>>1)+nSelPtrY};
			if (m_wSelected == i->first && m_bDrawSep)
			{
				rcThumb.left = nXC-nSelSide-(nSelSide>>1);
				rcThumb.right = nXC+1+nSelSide+(nSelSide>>1);
			}

			if (ps.rcPaint.left < rcThumb.right && rcThumb.left < ps.rcPaint.right &&
				rcThumb.top < ps.rcPaint.bottom && (ps.rcPaint.top < rcThumb.bottom || (m_wSelected == i->first && m_bDrawSep)))
			{
				if (m_wSelected == i->first && m_bDrawSep)
				{
					COLORREF clrLine = GetSysColor(COLOR_3DSHADOW);
					ren.color(agg::rgba8(GetRValue(clrLine), GetGValue(clrLine), GetBValue(clrLine), 255));
					double fSlope = (rcThumb.right-rcThumb.left)*0.5;
					ras.move_to_d(rcThumb.left+fSlope-ps.rcPaint.left, rcThumb.top-ps.rcPaint.top);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left, rcThumb.top+fSlope-ps.rcPaint.top);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left, rcThumb.top+fSlope-ps.rcPaint.top);
					ras.close_polygon();
					agg::render_scanlines(ras, sl, ren);
					ren.color(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
					double fDelta = m_fScale;
					double fDelta2 = fDelta*sqrt(2.0);
					double fDeltaD = fDelta2-fDelta;
					ras.reset();
					ras.move_to_d(rcThumb.left+fSlope-ps.rcPaint.left, rcThumb.top-ps.rcPaint.top+fDelta2);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.close_polygon();
					agg::render_scanlines(ras, sl, ren);
					fDelta2 *= 2.0;
					fDelta *= 2.0;
					fDeltaD *= 2.0;
					ras.reset();
					ras.move_to_d(rcThumb.left+fSlope-ps.rcPaint.left, rcThumb.top-ps.rcPaint.top+fDelta2);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcWin.bottom-ps.rcPaint.top-1);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.close_polygon();

					agg::span_allocator<agg::rgba8> span_alloc;
					agg::scanline_p8 sl;
					float const fR = i->second.fR <= 0.0f ? 0.0f : (i->second.fR >= 1.0f ? 1.0f : i->second.fR);
					float const fG = i->second.fG <= 0.0f ? 0.0f : (i->second.fG >= 1.0f ? 1.0f : i->second.fG);
					float const fB = i->second.fB <= 0.0f ? 0.0f : (i->second.fB >= 1.0f ? 1.0f : i->second.fB);
					float const fA = i->second.fA <= 0.0f ? 0.0f : i->second.fA;
					float const fIA = 1.0f-fA;
					BYTE const clrs[] =
					{
						CGammaTables::ToSRGB(fRGBs[0]*fIA + fR*fA),
						CGammaTables::ToSRGB(fRGBs[1]*fIA + fG*fA),
						CGammaTables::ToSRGB(fRGBs[2]*fIA + fB*fA),
						255,
						CGammaTables::ToSRGB(fRGBs[3]*fIA + fR*fA),
						CGammaTables::ToSRGB(fRGBs[4]*fIA + fG*fA),
						CGammaTables::ToSRGB(fRGBs[5]*fIA + fB*fA),
						255,
					};
					agg::render_scanlines_aa(ras, sl, renb, span_alloc, CChequeredAlpha(clrs, ps.rcPaint.left+nGradSizeX-nXC, ps.rcPaint.top, nSquare, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD, rcWin.bottom-ps.rcPaint.top, aBackground));
				}
				else
				{
					if (m_wSelected == i->first)
						ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
					else
						ren.color(agg::rgba8(GetRValue(clrBorder), GetGValue(clrBorder), GetBValue(clrBorder), 255));
					double fSlope = (rcThumb.right-rcThumb.left)*0.5;
					ras.move_to_d(rcThumb.left+fSlope-ps.rcPaint.left, rcThumb.top-ps.rcPaint.top);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left, rcThumb.top+fSlope-ps.rcPaint.top);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left, rcThumb.bottom-ps.rcPaint.top);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left, rcThumb.bottom-ps.rcPaint.top);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left, rcThumb.top+fSlope-ps.rcPaint.top);
					ras.close_polygon();
					agg::render_scanlines(ras, sl, ren);
					ren.color(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
					double fDelta = m_fScale;
					double fDelta2 = fDelta*sqrt(2.0);
					double fDeltaD = fDelta2-fDelta;
					ras.reset();
					ras.move_to_d(rcThumb.left+fSlope-ps.rcPaint.left, rcThumb.top-ps.rcPaint.top+fDelta2);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcThumb.bottom-ps.rcPaint.top-fDelta);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcThumb.bottom-ps.rcPaint.top-fDelta);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.close_polygon();
					agg::render_scanlines(ras, sl, ren);
					fDelta2 *= 2.0;
					fDelta *= 2.0;
					fDeltaD *= 2.0;
					ras.reset();
					ras.move_to_d(rcThumb.left+fSlope-ps.rcPaint.left, rcThumb.top-ps.rcPaint.top+fDelta2);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.line_to_d(rcThumb.right-ps.rcPaint.left-fDelta, rcThumb.bottom-ps.rcPaint.top-fDelta);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcThumb.bottom-ps.rcPaint.top-fDelta);
					ras.line_to_d(rcThumb.left-ps.rcPaint.left+fDelta, rcThumb.top+fSlope-ps.rcPaint.top+fDeltaD);
					ras.close_polygon();
					if (i->second.fA >= 1.0f)
					{
						// solid color
						ren.color(agg::rgba8(CGammaTables::ToSRGB(i->second.fR), CGammaTables::ToSRGB(i->second.fG), CGammaTables::ToSRGB(i->second.fB), 255));
						agg::render_scanlines(ras, sl, ren);
					}
					else
					{
						// semitransparent color (-> pattern)
						agg::span_allocator<agg::rgba8> span_alloc;
						agg::scanline_p8 sl;
						float const fR = i->second.fR <= 0.0f ? 0.0f : (i->second.fR >= 1.0f ? 1.0f : i->second.fR);
						float const fG = i->second.fG <= 0.0f ? 0.0f : (i->second.fG >= 1.0f ? 1.0f : i->second.fG);
						float const fB = i->second.fB <= 0.0f ? 0.0f : (i->second.fB >= 1.0f ? 1.0f : i->second.fB);
						float const fA = i->second.fA <= 0.0f ? 0.0f : i->second.fA;
						float const fIA = 1.0f-fA;
						BYTE const clrs[] =
						{
							CGammaTables::ToSRGB(fRGBs[0]*fIA + fR*fA),
							CGammaTables::ToSRGB(fRGBs[1]*fIA + fG*fA),
							CGammaTables::ToSRGB(fRGBs[2]*fIA + fB*fA),
							255,
							CGammaTables::ToSRGB(fRGBs[3]*fIA + fR*fA),
							CGammaTables::ToSRGB(fRGBs[4]*fIA + fG*fA),
							CGammaTables::ToSRGB(fRGBs[5]*fIA + fB*fA),
							255,
						};
						agg::render_scanlines_aa(ras, sl, renb, span_alloc, CChequeredAlpha(clrs, ps.rcPaint.left+nGradSizeX-nXC, ps.rcPaint.top, nSquare));
					}
				}
			}
		}

		//pixf.apply_gamma_inv(gp);

		cDC.BitBlt(ps.rcPaint.left, ps.rcPaint.top, tBMI.bmiHeader.biWidth, tBMI.bmiHeader.biHeight, cDCPaint, 0, 0, SRCCOPY);
		DeleteObject(cDCPaint.SelectBitmap(hBmp));

		EndPaint(&ps);
		return 0;
	}

private:
	struct CChequeredAlpha
	{
		CChequeredAlpha(BYTE const* a_pColors, int a_nOffX, int a_nOffY, int a_nSquare, int a_nFadeStart = INT_MAX, int a_nFadeEnd = INT_MAX, BYTE const* const a_pBackground = NULL) :
			m_pColors(a_pColors), m_nOffX(a_nOffX), m_nOffY(a_nOffY), m_nSquare(a_nSquare), m_nFadeStart(a_nFadeStart), m_nFadeEnd(a_nFadeEnd), m_pBackground(a_pBackground)
		{
		}

		void generate(agg::rgba8* span, int x, int y, unsigned len)
		{
			BYTE const* pColors = m_pColors;
			BYTE aColors[8];
			if (m_pBackground && y > m_nFadeStart)
			{
				if (y < m_nFadeEnd)
				{
					float const fA = float(y-m_nFadeStart)/(m_nFadeEnd-m_nFadeStart);
					float const fIA = 1.0f-fA;
					float fBg0 = CGammaTables::FromSRGB(m_pBackground[0]);
					float fBg1 = CGammaTables::FromSRGB(m_pBackground[1]);
					float fBg2 = CGammaTables::FromSRGB(m_pBackground[2]);
					aColors[0] = CGammaTables::ToSRGB(fBg0*fA + CGammaTables::FromSRGB(m_pColors[0])*fIA);
					aColors[1] = CGammaTables::ToSRGB(fBg1*fA + CGammaTables::FromSRGB(m_pColors[1])*fIA);
					aColors[2] = CGammaTables::ToSRGB(fBg2*fA + CGammaTables::FromSRGB(m_pColors[2])*fIA);
					aColors[4] = CGammaTables::ToSRGB(fBg0*fA + CGammaTables::FromSRGB(m_pColors[4])*fIA);
					aColors[5] = CGammaTables::ToSRGB(fBg1*fA + CGammaTables::FromSRGB(m_pColors[5])*fIA);
					aColors[6] = CGammaTables::ToSRGB(fBg2*fA + CGammaTables::FromSRGB(m_pColors[6])*fIA);
					aColors[3] = aColors[7] = 0xff;
				}
				else
				{
					aColors[0] = aColors[4] = m_pBackground[0];
					aColors[1] = aColors[5] = m_pBackground[1];
					aColors[2] = aColors[6] = m_pBackground[2];
					aColors[3] = aColors[7] = 0xff;
				}
				pColors = aColors;
			}
			int const ny = ((y+m_nOffY)/m_nSquare)&1;
			for (int nEnd = x+len; x< nEnd; ++x, ++span)
			{
				int const n = ((((x+m_nOffX)/m_nSquare)&1)^ny)<<2;
				span->r = pColors[n];
				span->g = pColors[n+1];
				span->b = pColors[n+2];
				span->a = pColors[n+3];
			}
		}
		void prepare()
		{
		}
	private:
		BYTE const* const m_pColors;
		int const m_nOffX;
		int const m_nOffY;
		int const m_nSquare;
		int const m_nFadeStart;
		int const m_nFadeEnd;
		BYTE const* const m_pBackground;
	};

	struct CBackgroundSep
	{
		CBackgroundSep(BYTE const* a_pLineColor, BYTE const* a_pGradColor, int a_nGradSize, int a_nLinePos, int a_nOffset, int a_nWindowWidth) :
			m_pLineColor(a_pLineColor), m_pGradColor(a_pGradColor), m_nGradSize(a_nGradSize), m_nLinePos(a_nLinePos),
			m_nGradPos(a_nLinePos-a_nGradSize), m_nSolidStart(a_nGradSize+a_nOffset), m_nSolidEnd(a_nWindowWidth-a_nGradSize+a_nOffset)
		{
		}

		void generateLine(agg::rgba8* span, int x1, int x2, agg::rgba8 const base)
		{
			while (x1 < m_nSolidStart && x1 < x2)
			{
				*span = base;
				span->a = base.a*(m_nGradSize-m_nSolidStart+x1)/m_nGradSize;
				++x1;
				++span;
			}
			while (x1 < m_nSolidEnd && x1 < x2)
			{
				*span = base;
				++x1;
				++span;
			}
			while (x1 < x2)
			{
				*span = base;
				span->a = base.a*(m_nGradSize-x1+m_nSolidEnd)/m_nGradSize;
				++x1;
				++span;
			}
		}
		void generate(agg::rgba8* span, int x, int y, unsigned len)
		{
			if (y > m_nLinePos || y < m_nGradPos)
			{
				agg::rgba8 const empty(0, 0, 0, 0);
				for (agg::rgba8* const end = span+len; span != end; ++span)
					*span = empty;
				return;
			}
			if (y == m_nLinePos)
			{
				generateLine(span, x, x+len, agg::rgba8(m_pLineColor[0], m_pLineColor[1], m_pLineColor[2], m_pLineColor[3]));
				return;
			}
			generateLine(span, x, x+len, agg::rgba8(m_pGradColor[0], m_pGradColor[1], m_pGradColor[2], m_pGradColor[3]*(y-m_nGradPos)/m_nGradSize));
		}
		void prepare()
		{
		}
	private:
		BYTE const* const m_pLineColor;
		BYTE const* const m_pGradColor;
		int const m_nGradSize;
		int const m_nLinePos;
		int const m_nGradPos;
		int const m_nSolidStart;
		int const m_nSolidEnd;
	};

private:
	CGradient::iterator StopHitTest(POINT p, int* a_pStopCenter = NULL)
	{
		RECT rcWin;
		GetClientRect(&rcWin);
		int nBorderWidth = 2*m_fScale;
		int nSelSide = 5*m_fScale;
		int nSelPtrX = nSelSide*2+1;
		int nSelPtrY = nSelSide*3+1;
		int nGradSizeX = rcWin.right-(nSelSide<<1)-(m_nSepMargins<<1);
		int nGradSizeY = 16*m_fScale;
		int nGradOffX = nSelSide+m_nSepMargins;
		int nGradOffY = nBorderWidth;
		bool bFirst = false;
		for (CGradient::iterator i = m_cGradient.begin(); i != m_cGradient.end(); ++i)
		{
			LONG nXC = (i->first*(nGradSizeX-1)+0x7fff)/0xffff + nGradOffX;
			RECT rcThumb = {nXC-nSelSide, nGradOffY+(nGradSizeY>>1), nXC+1+nSelSide, nGradOffY+(nGradSizeY>>1)+nSelPtrY};
			if (m_bDrawSep && i->first == m_wSelected)
			{
				rcThumb.left -= nSelSide>>1;
				rcThumb.right += nSelSide>>1;
			}
			if (p.x >= rcThumb.left && p.x < rcThumb.right && p.y >= rcThumb.top && p.y < rcThumb.bottom)
			{
				if (i == m_cGradient.begin())
				{
					bFirst = true;
					continue;
				}
				else
				{
					if (a_pStopCenter) *a_pStopCenter = (i->first*(nGradSizeX-1)+0x7fff)/0xffff + nGradOffX;
					return i;
				}
			}
			else if (bFirst)
			{
				if (a_pStopCenter) *a_pStopCenter = nGradOffX;
				return m_cGradient.begin();
			}
		}
		if (bFirst)
		{
			if (a_pStopCenter) *a_pStopCenter = nGradOffX;
			return m_cGradient.begin();
		}

		return m_cGradient.end();
	}
	bool PositionFromPoint(POINT p, WORD* a_pPos, bool a_bExtraTolerance = false)
	{
		RECT rcWin;
		GetClientRect(&rcWin);
		int nBorderWidth = 2*m_fScale;
		int nSelSide = 5*m_fScale;
		int nSelPtrX = nSelSide*2+1;
		int nSelPtrY = nSelSide*3+1;
		int nGradSizeX = rcWin.right-(nSelSide<<1)-(m_nSepMargins<<1);
		int nGradSizeY = 16*m_fScale;
		int nGradOffX = nSelSide+m_nSepMargins;
		int nGradOffY = nBorderWidth;
		if (a_bExtraTolerance)
		{
			LONG const nTolerance = LONG(64*m_fScale);
			if (p.y < nGradOffY && p.y > nGradOffY-nTolerance)
				p.y = nGradOffY;
			else if (p.y >= nGradOffY+nGradSizeY && p.y < nGradOffY+nGradSizeY+nTolerance)
				p.y = nGradOffY+nGradSizeY-1;
			if (p.x < nGradOffX && p.x > nGradOffX-nTolerance)
				p.x = nGradOffX;
			else if (p.x >= nGradOffX+nGradSizeX && p.x < nGradOffX+nGradSizeX+nTolerance)
				p.x = nGradOffX+nGradSizeX-1;
		}
		if (p.y < nGradOffY || p.y >= nGradOffY+nGradSizeY || p.x < nGradOffX || p.x >= nGradOffX+nGradSizeX)
			return false;

		if (a_pPos)
			*a_pPos = ((p.x-nGradOffX)*0xffff+(nGradSizeX>>1))/nGradSizeX;
		return true;
	}

	void SendNotification(UINT nCode, WORD pos, CButtonColorPicker::SColor const& clr)
	{
		NMGRADIENT nmgrd;

		nmgrd.hdr.code = nCode;
		nmgrd.hdr.hwndFrom = m_hWnd;
		nmgrd.hdr.idFrom = GetDlgCtrlID();
		nmgrd.pos = pos;
		nmgrd.clr = clr;

		GetParent().SendMessage(WM_NOTIFY, (WPARAM)nmgrd.hdr.idFrom, (LPARAM)&nmgrd);
	}

private:
	CGradient m_cGradient;
	WORD m_wSelected;
	float m_fScale;
	CGradient m_cGradientBackup;
	WORD m_wSelectedBackup;
	bool m_bRescaling;
	bool m_bDrawSep;
	int m_nSepMargins;
	int m_nDragOffset;
};

#ifndef NOGRADIENTCONTEXTMENU
#include <ContextMenuWithIcons.h>
#include <XPGUI.h>

class CGradientColorPicker :
	public CGradientColorPickerBase,
	public CContextMenuWithIcons<CGradientColorPicker>
{
public:
	CGradientColorPicker(bool a_bDrawSep = false, int a_nSepMargins = 0, LCID a_tLocaleID = GetThreadLocale()) : CGradientColorPickerBase(a_bDrawSep, a_nSepMargins), m_tLocaleID(a_tLocaleID)
	{
	}
	~CGradientColorPicker()
	{
		m_cMenuImages.Destroy();
	}
	void SetLocale(LCID a_tLocaleID)
	{
		m_tLocaleID = a_tLocaleID;
	}

	DECLARE_WND_CLASS_EX(_T("GradientColorPicker"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CGradientColorPicker)
		CHAIN_MSG_MAP(CGradientColorPickerBase)
		CHAIN_MSG_MAP(CContextMenuWithIcons<CGradientColorPicker>)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	END_MSG_MAP()

	LRESULT OnRButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		int cx = GetSystemMetrics(SM_CXDRAG);
		int cy = GetSystemMetrics(SM_CXDRAG);
		if (GET_X_LPARAM(a_lParam)+cx < m_tClickedPoint.x || GET_X_LPARAM(a_lParam) > m_tClickedPoint.x+cx ||
			GET_Y_LPARAM(a_lParam)+cy < m_tClickedPoint.y || GET_Y_LPARAM(a_lParam) > m_tClickedPoint.y+cy)
			return 0;

		ClientToScreen(&m_tClickedPoint);

		CMenu cMenu;
		StartContextMenu(cMenu);

		RECT rc;
		if (!m_bClickedStop || !GetStopRect(m_nClickedStop, &rc))
		{
			GetClientRect(&rc);
		}
		else
		{
			AddStopCommands(cMenu, m_nClickedStop);
		}
		AddGlobalCommands(cMenu);

		ClientToScreen(&rc);
		TPMPARAMS tTPMP = { sizeof(TPMPARAMS), rc };
		ProcessCommand(cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, m_tClickedPoint.x, m_tClickedPoint.y, m_hWnd, m_bClickedStop ? &tTPMP : NULL), m_nClickedStop);
		return 0;
	}

	LRESULT OnRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_tClickedPoint.x = GET_X_LPARAM(a_lParam);
		m_tClickedPoint.y = GET_Y_LPARAM(a_lParam);
		m_bClickedStop = StopFromPoint(m_tClickedPoint, &m_nClickedStop);
		return 0;
	}

	LRESULT OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CMenu cMenu;
		StartContextMenu(cMenu);

		WORD cur = GetStop();
		RECT rc;
		if (!GetStopRect(cur, &rc))
		{
			GetClientRect(&rc);
		}
		else
		{
			AddStopCommands(cMenu, cur);
		}
		AddGlobalCommands(cMenu);

		ClientToScreen(&rc);
		TPMPARAMS tTPMP = { sizeof(TPMPARAMS), rc };
		ProcessCommand(cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, (rc.left+rc.right)>>1, rc.top, m_hWnd, &tTPMP), cur);
		return 0;
	}

private:
	enum
	{
		ID_CMD_REVERSE = 1,
		ID_CMD_MIRROR,
		ID_CMD_COPY,
		ID_CMD_PASTE,
		ID_CMD_REMOVESTOP,
		ID_CMD_REMOVELEFT,
		ID_CMD_REMOVERIGHT,
	};

	void StartContextMenu(CMenu& cMenu)
	{
		if (m_cMenuImages.m_hImageList == NULL)
		{
			int nIconSize = XPGUI::GetSmallIconSize();
			m_cMenuImages.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 1, 0);
		}
		Reset(m_cMenuImages);
		cMenu.CreatePopupMenu();
	}
	void AddStopCommands(CMenu& cMenu, WORD wStop)
	{
		if (wStop == 0 || wStop == 0xffff)
			return;

		CComBSTR bstr;
		CMultiLanguageString::GetLocalized(L"[0409]Remove stop[0405]Odstranit zastavení", m_tLocaleID, &bstr);
		AddItem(ID_CMD_REMOVESTOP, bstr, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_CMD_REMOVESTOP, LPCTSTR(NULL));

		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Remove stops left[0405]Odstranit zastavení nalevo", m_tLocaleID, &bstr);
		AddItem(ID_CMD_REMOVELEFT, bstr, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_CMD_REMOVELEFT, LPCTSTR(NULL));

		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Remove stops right[0405]Odstranit zastavení napravo", m_tLocaleID, &bstr);
		AddItem(ID_CMD_REMOVERIGHT, bstr, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_CMD_REMOVERIGHT, LPCTSTR(NULL));

		cMenu.AppendMenu(MFT_SEPARATOR);
	}
	void AddGlobalCommands(CMenu& cMenu)
	{
		CComBSTR bstr;
		CMultiLanguageString::GetLocalized(L"[0409]Reverse gradient[0405]Převrátit gradient", m_tLocaleID, &bstr);
		AddItem(ID_CMD_REVERSE, bstr, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_CMD_REVERSE, LPCTSTR(NULL));

		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Mirror gradient[0405]Zrcadlit gradient", m_tLocaleID, &bstr);
		AddItem(ID_CMD_MIRROR, bstr, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_CMD_MIRROR, LPCTSTR(NULL));

		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Copy gradient[0405]Kopírovat gradient", m_tLocaleID, &bstr);
		AddItem(ID_CMD_COPY, bstr, -1);
		cMenu.AppendMenu(MFT_OWNERDRAW, ID_CMD_COPY, LPCTSTR(NULL));

		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Paste gradient[0405]Vložit gradient", m_tLocaleID, &bstr);
		AddItem(ID_CMD_PASTE, bstr, -1);
		CGradient g;
		GradientFromClipboard(g);
		cMenu.AppendMenu(g.size() >= 2 ? MFT_OWNERDRAW : MFT_OWNERDRAW|MFS_DISABLED, ID_CMD_PASTE, LPCTSTR(NULL));
	}
	void ProcessCommand(UINT cmd, WORD wStop)
	{
		switch (cmd)
		{
		case ID_CMD_REVERSE:
			{
				CGradient const& orig = GetGradient();
				WORD sel = GetStop();
				CGradient next;
				for (CGradient::const_iterator i = orig.begin(); i != orig.end(); ++i)
					next[0xffff-i->first] = i->second;
				SetGradient(next, 0xffff-sel, true);
			}
			break;
		case ID_CMD_MIRROR:
			{
				CGradient const& orig = GetGradient();
				WORD sel = GetStop();
				CGradient next;
				for (CGradient::const_iterator i = orig.begin(); i != orig.end(); ++i)
				{
					next[i->first>>1] = i->second;
					if (i->first != 0xffff)
						next[0xffff-(i->first>>1)] = i->second;
				}
				SetGradient(next, sel>>1, true);
			}
			break;
		case ID_CMD_COPY:
			{
				CComBSTR bstrText(L"RWGRAD1:");
				SerializeGradient(GetGradient(), bstrText);

				CClipboardHandler cCb(m_hWnd);
				EmptyClipboard();
				COLE2CT psz(bstrText);
				DWORD nLen = _tcslen(psz)+1;
				HANDLE hCopy = GlobalAlloc(GMEM_MOVEABLE, nLen*sizeof TCHAR); 
				LPTSTR pszTmp = reinterpret_cast<LPTSTR>(GlobalLock(hCopy)); 
				memcpy(pszTmp, psz, nLen*sizeof TCHAR);
				GlobalUnlock(pszTmp);
				SetClipboardData(sizeof(TCHAR) == sizeof(char) ? CF_TEXT : CF_UNICODETEXT, hCopy);
			}
			break;
		case ID_CMD_PASTE:
			{
				CGradient g;
				GradientFromClipboard(g);
				if (g.size() >= 2)
					SetGradient(g, -1, true);
			}
			break;
		case ID_CMD_REMOVESTOP:
			{
				CGradient g(GetGradient());

				CGradient::const_iterator i = g.find(wStop);
				if (i != g.begin() && i != g.end() && i->first != 0xffff)
				{
					CGradient::const_iterator iB = i; --iB;
					CGradient::const_iterator iN = i; ++iN;
					WORD newSel = iN->first-i->first < i->first-iB->first ? iN->first : iB->first;
					g.erase(i);
					SetGradient(g, newSel, true);
				}
			}
		case ID_CMD_REMOVELEFT:
			{
				CGradient g(GetGradient());

				CGradient::const_iterator i = g.find(wStop);
				if (i != g.begin() && i != g.end() && i->first != 0xffff)
				{
					DWORD scale = 0xffff-wStop;
					CGradient next;
					for (; i != g.end(); ++i)
						next[DWORD((i->first-wStop)*0xffff+(scale>>1))/scale] = i->second;
					SetGradient(next, 0, true);
				}
			}
		case ID_CMD_REMOVERIGHT:
			{
				CGradient g(GetGradient());

				CGradient::const_iterator e = g.find(wStop);
				if (e != g.begin() && e != g.end() && ++e != g.end())
				{
					DWORD scale = wStop;
					CGradient next;
					for (CGradient::const_iterator i = g.begin(); i != e; ++i)
						next[DWORD(i->first*0xffff+(scale>>1))/scale] = i->second;
					SetGradient(next, 0xffff, true);
				}
			}
		}
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
	void GradientFromClipboard(CGradient& g)
	{
		if (!IsClipboardFormatAvailable(sizeof(TCHAR) == sizeof(char) ? CF_TEXT : CF_UNICODETEXT)) 
			return;
		CClipboardHandler cClipboard(m_hWnd);
		HANDLE hData = GetClipboardData(sizeof(TCHAR) == sizeof(char) ? CF_TEXT : CF_UNICODETEXT);
		if (hData == NULL)
			return;
		LPCTSTR psz = reinterpret_cast<LPCTSTR>(GlobalLock(hData));
		if (_tcsncmp(psz, _T("RWGRAD1:"), 8) == 0)
			ParseGradient(psz+8, g);
		GlobalUnlock(hData);
	}

private:
	CImageList m_cMenuImages;
	LCID m_tLocaleID;
	bool m_bClickedStop;
	WORD m_nClickedStop;
	POINT m_tClickedPoint;
};

#endif

}; // namespace WTL
