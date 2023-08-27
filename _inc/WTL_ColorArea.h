
#pragma once

#include <math.h>
#include <GammaCorrection.h>

namespace WTL
{

class CColorBaseHLSA
{
public:
	CColorBaseHLSA() : m_fH(0.0f), m_fL(1.0f), m_fS(0.5f), m_fA(1.0f) {}

	void SetH(float a_fVal) { m_fH = a_fVal; }
	void SetL(float a_fVal) { m_fL = a_fVal; }
	void SetS(float a_fVal) { m_fS = a_fVal; }
	void SetHLS(float a_fH, float a_fL, float a_fS) { m_fH = a_fH; m_fL = a_fL; m_fS = a_fS; }
	void SetRGB(float a_fR, float a_fG, float a_fB) { RGB2HLS(a_fR, a_fG, a_fB, m_fH, m_fL, m_fS); }
	void SetRGB(DWORD a_tRGB) { RGB2HLS(CGammaTables::FromSRGB(GetRValue(a_tRGB)), CGammaTables::FromSRGB(GetGValue(a_tRGB)), CGammaTables::FromSRGB(GetBValue(a_tRGB)), m_fH, m_fL, m_fS); }
	void SetBGR(DWORD a_tBGR) { RGB2HLS(CGammaTables::FromSRGB(GetBValue(a_tBGR)), CGammaTables::FromSRGB(GetGValue(a_tBGR)), CGammaTables::FromSRGB(GetRValue(a_tBGR)), m_fH, m_fL, m_fS); }
	void SetA(float a_fVal) { m_fA = a_fVal; }

	float GetH() const { return m_fH; }
	float GetL() const { return m_fL; }
	float GetS() const { return m_fS; }
	void GetHLS(float* a_pH, float* a_pL, float* a_pS) const { *a_pH = m_fH; *a_pL = m_fL; *a_pS = m_fS; }
	void GetRGB(float* a_pR, float* a_pG, float* a_pB) const { HLS2RGB(m_fH, m_fL, m_fS, *a_pR, *a_pG, *a_pB); }
	DWORD GetRGB() const { float fR, fG, fB; HLS2RGB(m_fH, m_fL, m_fS, fR, fG, fB); return RGB(CGammaTables::ToSRGB(fR), CGammaTables::ToSRGB(fG), CGammaTables::ToSRGB(fB)); }
	DWORD GetBGR() const { float fR, fG, fB; HLS2RGB(m_fH, m_fL, m_fS, fR, fG, fB); return RGB(CGammaTables::ToSRGB(fB), CGammaTables::ToSRGB(fG), CGammaTables::ToSRGB(fR)); }
	float GetA() const { return m_fA; }

private:
	static float hls_value(float n1, float n2, float h)
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
	static void HLS2RGB(float h, float l, float s, float& r, float& g, float& b)
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
	static void RGB2HLS(float r, float g, float b, float& h, float& l, float& s)
	{
		float bc, gc, rc, rgbmax, rgbmin;

		// Compute luminosity.
		rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
		rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
		l = (rgbmax + rgbmin) * 0.5f;

		// Compute saturation.
		if (rgbmax == rgbmin)
			s = 0.0f;
		else if (l <= 0.5f)
			s = (rgbmax - rgbmin) / (rgbmax + rgbmin);
		else
			s = (rgbmax - rgbmin) / (2.0f - rgbmax - rgbmin);

		// Compute the hue.
		if (rgbmax == rgbmin)
			/*h = 0.0f*/;
		else
		{
			rc = (rgbmax - r) / (rgbmax - rgbmin);
			gc = (rgbmax - g) / (rgbmax - rgbmin);
			bc = (rgbmax - b) / (rgbmax - rgbmin);

			if (r == rgbmax)
				h = bc - gc;
			else if (g == rgbmax)
				h = 2.0f + rc - bc;
			else
				h = 4.0f + gc - rc;

			h *= 60.0f;
			h = h - 360.0f*(int)(h/360.0f);
		}
	}

private:
	float m_fH;
	float m_fL;
	float m_fS;
	float m_fA;
};

template<template<class> class TColorInterpreter, class TColorBase, LPCTSTR t_pszWndClass>
class CColorArea :
	public CWindowImpl<CColorArea<TColorInterpreter, TColorBase, t_pszWndClass> >,
	public CThemeImpl<CColorArea<TColorInterpreter, TColorBase, t_pszWndClass> >,
	public TColorInterpreter<TColorBase>
{
public:
	CColorArea() : m_pImage(NULL), m_nSizeX(0), m_nSizeY(0)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CColorArea()
	{
		delete[] m_pImage;
	}

	enum { CAN_COLOR_CHANGED = 1 };


	DECLARE_WND_CLASS_EX(t_pszWndClass, CS_HREDRAW | CS_VREDRAW | CS_OWNDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CColorArea)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocusChange)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnFocusChange)
		CHAIN_MSG_MAP(CThemeImpl<CColorArea>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	END_MSG_MAP()

	// operations
public:
	COLORREF GetRGB() const
	{
		return m_tColor.GetRGB();
	}
	BYTE GetA() const
	{
		return static_cast<BYTE>(static_cast<int>(m_tColor.GetA()*255.0f+0.5f));
	}
	void SetRGBA(COLORREF a_tColor, BYTE a_bAlpha)
	{
		TColorBase tColor(m_tColor);
		tColor.SetRGB(a_tColor);
		tColor.SetA(a_bAlpha/255.0f);
		SetColor(tColor);
	}

	void GetHLSA(float* a_pH, float* a_pL, float* a_pS, float* a_pA) const
	{
		*a_pH = m_tColor.GetH();
		*a_pL = m_tColor.GetL();
		*a_pS = m_tColor.GetS();
		*a_pA = m_tColor.GetA();
	}
	void SetHLSA(float a_fH, float a_fL, float a_fS, float a_fA)
	{
		TColorBase tColor(m_tColor);
		tColor.SetH(a_fH);
		tColor.SetL(a_fL);
		tColor.SetS(a_fS);
		tColor.SetA(a_fA);
		SetColor(tColor);
	}

	TColorBase const& GetColor()
	{
		return m_tColor;
	}
	void SetColor(TColorBase const& a_tColor, bool a_bRefreshNow = false)
	{
		if (TColorInterpreter<TColorBase>::InvalidateArea(m_tColor, a_tColor))
		{
			delete[] m_pImage;
			m_pImage = NULL;
			m_tColor = a_tColor;
			if (m_hWnd)
				Invalidate(FALSE);
		}
		else if (TColorInterpreter<TColorBase>::InvalidateThumb(m_tColor, a_tColor) && m_hWnd)
		{
			InvalidateThumb(m_tColor);
			InvalidateThumb(a_tColor);
			m_tColor = a_tColor;
			if (a_bRefreshNow)
				UpdateWindow();
		}
		else
		{
			m_tColor = a_tColor;
		}
	}
	void ResetCache()
	{
		delete[] m_pImage;
		m_pImage = NULL;
		if (m_hWnd)
			Invalidate(FALSE);
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = GetDC();
		m_nHalfSize = (GetDeviceCaps(hDC, LOGPIXELSY)+12)/24;
		ReleaseDC(hDC);
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnFocusChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		InvalidateThumb();
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_uMsg == WM_KILLFOCUS ? NM_KILLFOCUS : NM_SETFOCUS};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
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
		RECT r;
		GetClientRect(&r);
		TColorBase tColor = m_tColor;
		TColorInterpreter<TColorBase>::XY2Color(((float)((((int)a_lParam)<<16)>>16))/(r.right-1), ((float)(((int)a_lParam)>>16))/(r.bottom-1), tColor);
		SetColor(tColor, true);
		SetCapture();
		SendNotification();
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			RECT r;
			GetClientRect(&r);
			TColorBase tColor = m_tColor;
			TColorInterpreter<TColorBase>::XY2Color(((float)((((int)a_lParam)<<16)>>16))/(r.right-1), ((float)(((int)a_lParam)>>16))/(r.bottom-1), tColor);
			SetColor(tColor, true);
			SendNotification();
		}
		return 0;
	}

	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		DWORD dwStyle = GetStyle();
		bool bFocus = m_hWnd == GetFocus();

		RECT r;
		PAINTSTRUCT ps;
		HDC hDC = ::BeginPaint(m_hWnd, &ps);
		GetClientRect(&r);

		if (dwStyle&WS_DISABLED)
		{
			FillRect(hDC, &r, (HBRUSH)(COLOR_3DFACE+1));
		}
		else
		{
			DWORD dwLight2 = GetSysColor(COLOR_3DHIGHLIGHT);
			DWORD dwDark1 = GetSysColor(COLOR_3DSHADOW);

			BITMAPINFO bi;
			bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biWidth=r.right;
			bi.bmiHeader.biHeight=-r.bottom;
			bi.bmiHeader.biPlanes=1;
			bi.bmiHeader.biBitCount=32;
			bi.bmiHeader.biCompression=BI_RGB;
			bi.bmiHeader.biSizeImage=0;
			bi.bmiHeader.biXPelsPerMeter=1000;
			bi.bmiHeader.biYPelsPerMeter=1000;
			bi.bmiHeader.biClrUsed=0;
			bi.bmiHeader.biClrImportant=0;
			BYTE const* const pImage = GetColorAreaImage(r.right, r.bottom);
			SetDIBitsToDevice(hDC, 0,0, r.right, r.bottom, 0, 0, 0, r.bottom, pImage, &bi, DIB_RGB_COLORS);

			float x, y;
			TColorInterpreter<TColorBase>::Color2XY(m_tColor, x, y);
			TColorBase tColor = m_tColor;
			TColorInterpreter<TColorBase>::XY2ColorInternal(x, y, tColor);
			DWORD dwColor = tColor.GetRGB();
			HBRUSH hSave = CreateSolidBrush(dwColor);
			int xs = (int)(x*(r.right-1)), ys = (int)(y*(r.bottom-1));
			RECT rTmp = {xs-m_nHalfSize, ys-m_nHalfSize, xs+1+m_nHalfSize, ys+1+m_nHalfSize};
			FillRect(hDC, &rTmp, hSave);
			DeleteObject(hSave);
			if (m_hTheme)
			{
				Draw3DRect(hDC, rTmp.left-1, rTmp.top-1, rTmp.right, rTmp.bottom, 6);
				Draw3DRect(hDC, rTmp.left-2, rTmp.top-2, rTmp.right+1, rTmp.bottom+1, bFocus ? 4 : 10);
			}
			else
			{
				Draw3DRect(hDC, rTmp.left-1, rTmp.top-1, rTmp.right, rTmp.bottom, bFocus ? 1 : 8);
				Draw3DRect(hDC, rTmp.left-2, rTmp.top-2, rTmp.right+1, rTmp.bottom+1, bFocus ? 3 : 1);
			}
		}

		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rc;
		GetClientRect(&rc);
		float fDX = 1.0f/(rc.right-1);
		float fDY = 1.0f/(rc.bottom-1);
		float fX;
		float fY;
		TColorInterpreter<TColorBase>::Color2XY(m_tColor, fX, fY);
		switch (a_wParam)
		{
		case VK_LEFT:
			if (fX <= 0.0f)
				return 0;
			fX = fX > fDX ? fX-fDX : 0.0f;
			break;
		case VK_RIGHT:
			if (fX >= 1.0f)
				return 0;
			fX = fX+fDX < 1.0f ? fX+fDX : 1.0f;
			break;
		case VK_UP:
			if (fY <= 0.0f)
				return 0;
			fY = fY > fDY ? fY-fDY : 0.0f;
			break;
		case VK_DOWN:
			if (fY >= 1.0f)
				return 0;
			fY = fY+fDY < 1.0f ? fY+fDY : 1.0f;
			break;
		}
		TColorBase tColor = m_tColor;
		TColorInterpreter<TColorBase>::XY2Color(fX, fY, tColor);
		SetColor(tColor, true);
		SendNotification();
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

	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int nDelta = GET_WHEEL_DELTA_WPARAM(wParam);

		TColorBase tColor1;
		ZeroMemory(&tColor1, sizeof tColor1);
		TColorInterpreter<TColorBase>::XY2Color(0.0f, 0.25f, tColor1);
		TColorBase tColor2;
		TColorInterpreter<TColorBase>::XY2Color(0.0f, 0.75f, tColor2);
		ZeroMemory(&tColor2, sizeof tColor2);

		bool bChange = false;
		float fX;
		float fY;
		TColorInterpreter<TColorBase>::Color2XY(m_tColor, fX, fY);
		if (memcmp(&tColor1, &tColor2, sizeof(tColor1)) == 0)
		{
			if (nDelta > 0 && fX < 1.0f)
			{
				bChange = true;
				fX += nDelta*0.02f/WHEEL_DELTA;
				if (fX > 1.0f) fX = 1.0f;
			}
			else if (nDelta < 0 && fX > 0.0f)
			{
				bChange = true;
				fX += nDelta*0.02f/WHEEL_DELTA;
				if (fX < 0.0f) fX = 0.0f;
			}
		}
		else
		{
			nDelta = -nDelta;
			if (nDelta > 0 && fY < 1.0f)
			{
				bChange = true;
				fY += nDelta*0.02f/WHEEL_DELTA;
				if (fY > 1.0f) fY = 1.0f;
			}
			else if (nDelta < 0 && fY > 0.0f)
			{
				bChange = true;
				fY += nDelta*0.02f/WHEEL_DELTA;
				if (fY < 0.0f) fY = 0.0f;
			}
		}
		if (bChange)
		{
			TColorBase tColor = m_tColor;
			TColorInterpreter<TColorBase>::XY2Color(fX, fY, tColor);
			SetColor(tColor, true);
			SendNotification();
		}

		return 0;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

private:
	BYTE const* GetColorAreaImage(int a_nSizeX, int a_nSizeY)
	{
		if (m_pImage != NULL && a_nSizeX == m_nSizeX && a_nSizeY == m_nSizeY)
			return m_pImage;

		DWORD *pBmp = reinterpret_cast<DWORD*>(new BYTE[a_nSizeX * a_nSizeY << 4]);
		if (m_pImage != NULL)
			delete[] m_pImage;
		m_pImage = reinterpret_cast<BYTE*>(pBmp);

		TColorInterpreter<TColorBase>::InitColorArea(pBmp, a_nSizeX, a_nSizeY, 1.0f/(a_nSizeX-1), 1.0f/(a_nSizeY-1), m_tColor);

		return m_pImage;
	}
	void InvalidateThumb()
	{
		InvalidateThumb(m_tColor);
	}
	void InvalidateThumb(TColorBase const& a_tColor)
	{
		RECT r;
		GetClientRect(&r);
		LONG nMaxX = r.right;
		LONG nMaxY = r.bottom;
		float x, y;
		TColorInterpreter<TColorBase>::Color2XY(a_tColor, x, y);
		r.right = (r.right-1)*x;
		r.bottom = (r.bottom-1)*y;
		r.left = r.right-m_nHalfSize-2;
		r.right += m_nHalfSize+3;
		r.top = r.bottom-m_nHalfSize-2;
		r.bottom += m_nHalfSize+3;
		if (r.left < 0) r.left = 0;
		if (r.top < 0) r.top = 0;
		if (r.right > nMaxX) r.right = nMaxX;
		if (r.bottom > nMaxY) r.bottom = nMaxY;
		InvalidateRect(&r, FALSE);
	}
	static void Draw3DRect(HDC hDC, int x1, int y1, int x2, int y2, int inverse)
	{
		static DWORD colors[] =
		{
			COLOR_3DSHADOW, COLOR_3DLIGHT,
			COLOR_3DDKSHADOW, COLOR_3DHILIGHT,
			COLOR_3DDKSHADOW, COLOR_3DDKSHADOW,
			COLOR_WINDOW, COLOR_WINDOW,
			COLOR_3DFACE, COLOR_3DFACE,
			COLOR_3DSHADOW, COLOR_3DSHADOW,
		};
		POINT p;
		HGDIOBJ hSave = SelectObject(hDC, CreatePen(PS_SOLID, 1, GetSysColor(colors[inverse])));
		MoveToEx(hDC, x1, y2, &p);
		LineTo(hDC, x1, y1);
		LineTo(hDC, x2, y1);
		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 1, GetSysColor(colors[inverse^1]))));
		LineTo(hDC, x2, y2);
		LineTo(hDC, x1, y2);
		DeleteObject(SelectObject(hDC, hSave));
	}

	void SendNotification()
	{
		HWND hPar = GetParent();
		NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), CAN_COLOR_CHANGED};
		SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
	}

private:
	BYTE const* m_pImage;
	int m_nSizeX;
	int m_nSizeY;
	TColorBase m_tColor;
	int m_nHalfSize;
};

template<class TColorBase>
struct CColorInterpreterRadialHS
{
	static void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		a_fX -= 0.5f;
		a_fY -= 0.5f;
		float fSat = powf((a_fX*a_fX + a_fY*a_fY)*4.0f, 0.4f);
		if (fSat > 1.0f)
			fSat = 1.0f;
		a_tColor.SetS(fSat);

		float clr;
		float mult = 0.125f*4.0f/3.1415f;
		if ((a_fY < 0.0f) && (a_fY < a_fX) && (a_fY <= -a_fX))
			clr = 0.125f - atanf(a_fX/a_fY)*mult;
		else if ((a_fX > 0.0f) && (a_fX > -a_fY) && (a_fX >= a_fY))
			clr = 0.375f + atanf(a_fY/a_fX)*mult;
		else if ((a_fY > 0.0f) && (a_fY > a_fX) && (a_fY >= -a_fX))
			clr = 0.625f - atanf(a_fX/a_fY)*mult;
		else if ((a_fX < 0.0f) && (a_fX < a_fY) && (a_fX <= -a_fY))
			clr = 0.875f + atanf(a_fY/a_fX)*mult;
		else
			clr = 0.0f;
		clr -= 0.125f;
		if (clr < 0.0f)
			clr += 1.0f;
		a_tColor.SetH(clr*360.0f);
	}
	static void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
	{
		float const r = sqrtf(powf(a_tColor.GetS(), 2.5f)*0.25f);
		a_fX = 0.5f + r*sinf(a_tColor.GetH()*3.1415f/180.0f);
		a_fY = 0.5f - r*cosf(a_tColor.GetH()*3.1415f/180.0f);
	}

	static void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
	{
		TColorBase tColor;
		tColor.SetL(0.5f);
		tColor.SetA(1.0f);
		for (float y = 0.0f; a_nSizeY > 0; y+=a_fStepY, --a_nSizeY)
		{
			int nSizeX = a_nSizeX;
			for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
			{
				XY2Color(x, y, tColor);
				*(a_pDst++) = tColor.GetBGR();
			}
		}
	}

	static void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		XY2Color(a_fX, a_fY, a_tColor);
		a_tColor.SetL(0.5f);
		a_tColor.SetA(1.0f);
	}

	static bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
	{
		return false; // active color has no effect on area
	}
	static bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
	{
		return a1.GetH() != a2.GetH() || a1.GetS() != a2.GetS();
	}
};


extern __declspec(selectany) TCHAR const  COLORAREAHSWNDCLASS[] = _T("WTL_ColorAreaHS");
typedef CColorArea<CColorInterpreterRadialHS, CColorBaseHLSA, COLORAREAHSWNDCLASS> CColorAreaHS;



template<class TColorBase>
struct CColorInterpreterLA
{
	static void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		a_fX = 1.0f-a_fX; if (a_fX < 0.0f) a_fX = 0.0f; else if (a_fX > 1.0f) a_fX = 1.0f;
		a_fY = 1.0f-a_fY; if (a_fY < 0.0f) a_fY = 0.0f; else if (a_fY > 1.0f) a_fY = 1.0f;
		a_tColor.SetL(a_fY);
		a_tColor.SetA(a_fX);
	}
	static void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
	{
		a_fX = 1.0f-a_tColor.GetA();
		a_fY = 1.0f-a_tColor.GetL();
	}

	static void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
	{
		DWORD const clr1 = GetSysColor(COLOR_3DLIGHT);
		DWORD const clr2 = GetSysColor(COLOR_3DSHADOW);
		float const aClr[6] =
		{ GetBValue(clr1)/255.0f, GetGValue(clr1)/255.0f, GetRValue(clr1)/255.0f,
		  GetBValue(clr2)/255.0f, GetGValue(clr2)/255.0f, GetRValue(clr2)/255.0f };

		TColorBase tColor = a_tCurrent;
		for (float y = 0.0f; a_nSizeY > 0; y+=a_fStepY, --a_nSizeY)
		{
			tColor.SetL(1.0f-y);
			float aBase[3];
			tColor.GetRGB(aBase+2, aBase+1, aBase);
			int nSizeX = a_nSizeX;
			for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
			{
				float const nx = 1.0f-x;
				float const* p = aClr + 3*(((nSizeX>>2)^(a_nSizeY>>2))&1);
				XY2Color(x, y, tColor);
				*(a_pDst++) = RGB((aBase[0]*nx+p[0]*x)*255.0f+0.5f, (aBase[1]*nx+p[1]*x)*255.0f+0.5f, (aBase[2]*nx+p[2]*x)*255.0f+0.5f);
			}
		}
	}

	static void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		XY2Color(a_fX, a_fY, a_tColor);
		//a_tColor.SetH(0.5f);
		//a_tColor.SetS(0.0f);
	}

	static bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
	{
		return a1.GetS() != a2.GetS() || a1.GetH() != a2.GetH();
	}
	static bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
	{
		return a1.GetL() != a2.GetL() || a1.GetA() != a2.GetA();
	}
};


extern __declspec(selectany) TCHAR const  COLORAREALAWNDCLASS[] = _T("WTL_ColorAreaLA");
typedef CColorArea<CColorInterpreterLA, CColorBaseHLSA, COLORAREALAWNDCLASS> CColorAreaLA;


template<class TColorBase>
struct CColorInterpreterHue
{
	static void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		if(a_fX > 1.0f) a_fX = 1.0f;
		if(a_fX < 0.0f) a_fX = 0.0f;
		if(a_fY > 1.0f) a_fY = 1.0f;
		if(a_fY < 0.0f) a_fY = 0.0f;
		a_tColor.SetH(a_fX*360);
		a_tColor.SetS(1.0f);
		a_tColor.SetL(0.5f + a_fY / 2.0f);
	}
	static void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
	{
		a_fX = a_tColor.GetH() / 360.0f;
		a_fY = a_tColor.GetL() * 2.0f - 1.0f;
	}

	static void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
	{
		TColorBase tColor = a_tCurrent;
		int nSizeY = a_nSizeY;
		for (float y = 0.0f; nSizeY > 0; y+=a_fStepY, --nSizeY)
		{
			int nSizeX = a_nSizeX;
			for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
			{
				XY2Color(x, y, tColor);
				float rgb[3];
				tColor.GetRGB(rgb+2, rgb+1, rgb);
				*(a_pDst++) = RGB(CGammaTables::ToSRGB(rgb[0]), CGammaTables::ToSRGB(rgb[1]), CGammaTables::ToSRGB(rgb[2]));
			}
		}
	}

	static void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		XY2Color(a_fX, a_fY, a_tColor);
	}

	static bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
	{
		return false;
	}
	static bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
	{
		return a1.GetL() != a2.GetL() || a1.GetH() != a2.GetH();
	}
};


extern __declspec(selectany) TCHAR const  COLORAREAHUEWNDCLASS[] = _T("WTL_ColorAreaHue");
typedef CColorArea<CColorInterpreterHue, CColorBaseHLSA, COLORAREAHUEWNDCLASS> CColorAreaHue;

template<class TColorBase>
struct CColorInterpreterHueSat
{
	static void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		if(a_fX > 1.0f) a_fX = 1.0f;
		if(a_fX < 0.0f) a_fX = 0.0f;
		if(a_fY > 1.0f) a_fY = 1.0f;
		if(a_fY < 0.0f) a_fY = 0.0f;
		a_tColor.SetH(a_fX*360);
		a_tColor.SetS(1.0f-a_fY);
		a_tColor.SetL(0.5f);
	}
	static void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
	{
		a_fX = a_tColor.GetH() / 360.0f;
		a_fY = 1.0f-a_tColor.GetS();
	}

	static void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
	{
		TColorBase tColor = a_tCurrent;
		int nSizeY = a_nSizeY;
		for (float y = 0.0f; nSizeY > 0; y+=a_fStepY, --nSizeY)
		{
			int nSizeX = a_nSizeX;
			for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
			{
				XY2Color(x, y, tColor);
				float rgb[3];
				tColor.GetRGB(rgb+2, rgb+1, rgb);
				*(a_pDst++) = RGB(CGammaTables::ToSRGB(rgb[0]), CGammaTables::ToSRGB(rgb[1]), CGammaTables::ToSRGB(rgb[2]));
			}
		}
	}

	static void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
	{
		XY2Color(a_fX, a_fY, a_tColor);
	}

	static bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
	{
		return false;
	}
	static bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
	{
		return a1.GetS() != a2.GetS() || a1.GetH() != a2.GetH();
	}
};


extern __declspec(selectany) TCHAR const  COLORAREAHUESATWNDCLASS[] = _T("WTL_ColorAreaHueSat");
typedef CColorArea<CColorInterpreterHueSat, CColorBaseHLSA, COLORAREAHUESATWNDCLASS> CColorAreaHueSat;

}; // namespace WTL