
#pragma once

#include <math.h>

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_conv_dash.h>
#include <agg_path_storage.h>
#include <agg_ellipse.h>
#include <GammaCorrection.h>

namespace WTL
{

class C2DRotation :
	public CWindowImpl<C2DRotation>,
	public CThemeImpl<C2DRotation>
{
public:
	C2DRotation() : m_fAngle(0.0f), m_bNoBackBuffer(false), m_hCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS))),
		m_pGamma(NULL), m_fCenterX(10.0f), m_fCenterY(10.0f), m_fSize(8.0f), m_bHotVisible(false), m_fHotAngle(0.0f)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~C2DRotation()
	{
		delete m_pGamma;
	}

	enum {
		C2DR_ANGLE_CHANGED = 1,
		C2DR_DRAG_FINISHED = 2
	};


	DECLARE_WND_CLASS_EX(_T("WTL_AngleWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(C2DRotation)
		CHAIN_MSG_MAP(CThemeImpl<C2DRotation>)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnChangeFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnChangeFocus)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave);
	END_MSG_MAP()

	// operations
public:
	void SetAngle(float a_fDegrees)
	{
		m_fAngle = a_fDegrees;
		if (m_hWnd)
			Invalidate(FALSE);
	}

	float GetAngle() const
	{
		return m_fAngle;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hdc = GetDC();
		m_fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);
		RECT rc;
		GetClientRect(&rc);
		BOOL b;
		OnSize(0, 0, MAKELPARAM(rc.right, rc.bottom), b);
		return 0;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LONG nX = LOWORD(a_lParam+1)>>1;
		LONG nY = HIWORD(a_lParam+1)>>1;
		m_fCenterX = nX-0.5f;
		m_fCenterY = nY-0.5f;
		m_fSize = min(m_fCenterX, m_fCenterY);
		if (WS_EX_CLIENTEDGE&GetWindowLong(GWL_EXSTYLE))
			m_fSize -= ceilf(m_fScale+m_fScale);
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
			return lRes;
		SetFocus();
		return MA_ACTIVATE;
	}

	LRESULT OnChangeFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (reinterpret_cast<HWND>(a_wParam) == m_hWnd)
		{
			SetCursor(m_hCursor);
			return TRUE;
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			SendNotification(C2DR_DRAG_FINISHED);
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCapture();
		OnMouseMove(0, a_wParam, a_lParam, a_bHandled);
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		float fX = GET_X_LPARAM(a_lParam)+0.5f;
		float fY = GET_Y_LPARAM(a_lParam)+0.5f;
		if (GetCapture() == m_hWnd)
		{
			if ((fX-m_fCenterX)*(fX-m_fCenterX) + (fY-m_fCenterY)*(fY-m_fCenterY) < 2)
				return 0;
			float fAngle = AngleFromPoint(fX, fY);
			if (fAngle < 0.0f)
				fAngle += 360.0f;
			if (fabsf(fAngle-m_fAngle) < 1e-3f)
				return 0; // too similar
			m_fAngle = fAngle;
			SetHot(false);
			SendNotification();
			Invalidate(FALSE);
		}
		else
		{
			float dist = (fX-m_fCenterX)*(fX-m_fCenterX) + (fY-m_fCenterY)*(fY-m_fCenterY);
			if (dist < 2 || dist > m_fSize*m_fSize)
			{
				SetHot(false);
				return 0;
			}
			else
			{
				float fAngle = AngleFromPoint(fX, fY);
				if (fAngle < 0.0f)
					fAngle += 360.0f;
				SetHot(true, fAngle);
			}
		}
		return 0;
	}
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		SetHot(false);
		return FALSE;
	}

	LRESULT OnGetDlgCode(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return DLGC_WANTARROWS;
	}
	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_wParam != VK_LEFT && a_wParam != VK_RIGHT &&
			a_wParam != VK_UP && a_wParam != VK_DOWN)
			return 0;

		float const step = GetAsyncKeyState(VK_SHIFT)&0x8000 ? 1.0f : (GetAsyncKeyState(VK_CONTROL)&0x8000 ? 30.0f : 5.0f);
		switch (a_wParam)
		{
		case VK_LEFT:
		case VK_UP:
			m_fAngle = (ceilf(m_fAngle/step-0.1f)-1)*step;
			if (m_fAngle < 0.0f) m_fAngle += 360.0f;
			break;
		case VK_RIGHT:
		case VK_DOWN:
			m_fAngle = (floorf(m_fAngle/step+0.1f)+1)*step;
			if (m_fAngle > 360.0f) m_fAngle -= 360.0f;
			break;
		}
		SetHot(false);
		Invalidate(FALSE);
		SendNotification();
		SendNotification(C2DR_DRAG_FINISHED);
		return 0;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		// get window style
		DWORD dwStyle = GetWindowLong(GWL_STYLE);
		DWORD dwExStyle = GetWindowLong(GWL_EXSTYLE);

		RECT rcWin;
		GetClientRect(&rcWin);

		COLORREF clrPen = GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrBG = GetSysColor(dwExStyle&WS_EX_CLIENTEDGE ? COLOR_WINDOW : COLOR_3DFACE);
		COLORREF clr = GetSysColor(COLOR_HIGHLIGHT);
		if (dwStyle & WS_DISABLED)
		{
			clrPen = (((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
					 (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
					 (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		}
		COLORREF clrFill = GetSysColor(dwExStyle&WS_EX_CLIENTEDGE ? COLOR_3DFACE : COLOR_WINDOW);

		PAINTSTRUCT ps;
		HDC hDC2 = BeginPaint(&ps);
		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;

		CAutoVectorPtr<BYTE> pData(new BYTE[width*height*4]);
		{
			DWORD const fill = ((clrBG&0xff)<<16)|(clrBG&0xff00)|((clrBG>>16)&0xff)|0xff000000;
			DWORD* p = reinterpret_cast<DWORD*>(pData.m_p);
			for (DWORD* const e = p+width*height; p != e; ++p)
				*p = fill;
		}

		if (m_pGamma == NULL)
			m_pGamma = new CGammaTables(2.2f);

		CGammaBuffer pixf(m_pGamma, pData, width, height, -width*4);
		agg::renderer_base<CGammaBuffer> renb(pixf);
		agg::renderer_scanline_aa_solid<agg::renderer_base<CGammaBuffer> > ren(renb);
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		// ellipse in LU corner
        agg::ellipse e;
        ren.color(TLinearColor(clrFill, m_pGamma));
        e.init(m_fCenterX, m_fCenterY, m_fSize-m_fScale*0.5f, m_fSize-m_fScale*0.5f);
        ras.add_path(e);
        agg::render_scanlines(ras, sl, ren);
		agg::conv_stroke<agg::ellipse> pge1(e);
		ren.color(TLinearColor(clrPen, m_pGamma));
		pge1.width(m_fScale);
		ras.add_path(pge1);
		agg::render_scanlines(ras, sl, ren);

		e.init(m_fCenterX, m_fCenterY, m_fScale*2.0f, m_fScale*2.0f, 16);
		{
			float fCos = cosf(m_fAngle*3.14159265359f/180.0f);
			float fSin = sinf(m_fAngle*3.14159265359f/180.0f);
			float d1x = -fCos*0.5f;
			float d1y = -fSin*0.5f;
			float d2x = fSin*m_fSize;
			float d2y = -fCos*m_fSize;
			ras.add_path(e);
			ras.move_to_d(m_fCenterX-d1x, m_fCenterY-d1y);
			ras.line_to_d(m_fCenterX+d1x, m_fCenterY+d1y);
			ras.line_to_d(m_fCenterX+d1x+d2x, m_fCenterY+d1y+d2y);
			ras.line_to_d(m_fCenterX-d1x+d2x, m_fCenterY-d1y+d2y);
			ras.close_polygon();
		}
		agg::render_scanlines(ras, sl, ren);

		if (m_bHotVisible)
		{
			float fCos = cosf(m_fHotAngle*3.14159265359f/180.0f);
			float fSin = sinf(m_fHotAngle*3.14159265359f/180.0f);
			float d1x = -fCos*0.5f;
			float d1y = -fSin*0.5f;
			float d2x = fSin*m_fSize;
			float d2y = -fCos*m_fSize;
			ras.move_to_d(m_fCenterX-d1x, m_fCenterY-d1y);
			ras.line_to_d(m_fCenterX+d1x, m_fCenterY+d1y);
			ras.line_to_d(m_fCenterX+d1x+d2x, m_fCenterY+d1y+d2y);
			ras.line_to_d(m_fCenterX-d1x+d2x, m_fCenterY-d1y+d2y);
			ras.close_polygon();
			ren.color(TLinearColor(clrPen, 0x80, m_pGamma));
			agg::render_scanlines(ras, sl, ren);
			int n = GetPresetCount();
			for (int i = 0; i < n; ++i)
			{
				e.init(m_fCenterX+m_fSize*0.3f*sinf(3.14159265359f*i/n*2), m_fCenterY+m_fSize*0.3f*cosf(3.14159265359f*i/n*2), m_fScale*2.0f, m_fScale*2.0f);
				ras.add_path(e);
			}
			agg::render_scanlines(ras, sl, ren);
		}

		//// transparent fill
  //      ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 64));
  //      ras.add_path(path2);
  //      agg::render_scanlines(ras, sl, ren);

		//// solid outline (width 3)
		//agg::conv_stroke<agg::path_storage> pg(path2);
		//pg.line_join(agg::miter_join);
		//pg.width(fWidth);
		//ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
		//ras.add_path(pg);
		//agg::render_scanlines(ras, sl, ren);

		//// ellipse 2
  //      ren.color(agg::rgba8(GetRValue(clrFill), GetGValue(clrFill), GetBValue(clrFill), 255));
  //      e.init(
		//	nCenterX + (p[0].x+fHalfW-nCenterX)*cosf(m_fAngle*3.141528f/180.0f) - (p[0].y+fHalfW-nCenterY)*sinf(m_fAngle*3.141528f/180.0f),
		//	nCenterY + (p[0].x+fHalfW-nCenterX)*sinf(m_fAngle*3.141528f/180.0f) + (p[0].y+fHalfW-nCenterY)*cosf(m_fAngle*3.141528f/180.0f),
		//	m_fScale*3, m_fScale*3, 16);
  //      ras.add_path(e);
  //      agg::render_scanlines(ras, sl, ren);
		//agg::conv_stroke<agg::ellipse> pge2(e);
		//ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
		//pge2.width(fWidth);
		//ras.add_path(pge2);
		//agg::render_scanlines(ras, sl, ren);

		//// gamma correction
		//agg::gamma_lut<agg::int8u, agg::int8u, 8, 8> gp(2.2);
		//pixf.apply_gamma_inv(gp);

		if (width > 5 && height > 5 && GetFocus() == m_hWnd && (SendMessage(WM_QUERYUISTATE)&UISF_HIDEFOCUS) == 0)
		{
			for (int x = 2; x < width-2; x+=2)
				reinterpret_cast<DWORD*>(pData.m_p)[x+width+width] = 0;
			for (int x = 2; x < width-2; x+=2)
				reinterpret_cast<DWORD*>(pData.m_p)[x+width*(height-2)] = 0;
			for (int y = 4; y < height-2; y+=2)
				reinterpret_cast<DWORD*>(pData.m_p)[2+width*y] = 0;
			for (int y = 4; y < height-2; y+=2)
				reinterpret_cast<DWORD*>(pData.m_p)[width-2+width*y] = 0;
		}

		BITMAPINFO bmp_info;
		ZeroMemory(&bmp_info, sizeof bmp_info);
		bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.bmiHeader.biWidth = width;
		bmp_info.bmiHeader.biHeight = height;
		bmp_info.bmiHeader.biPlanes = 1;
		bmp_info.bmiHeader.biBitCount = 32;
		bmp_info.bmiHeader.biCompression = BI_RGB;

		::SetDIBitsToDevice(hDC2, rcWin.left, rcWin.top, width, height, 0, 0, 0, height, pData.m_p, &bmp_info, 0);
		EndPaint(&ps);

		return 0;
	}
	LRESULT OnEraseBkgnd(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

private:
	void SetHot(bool visible, float angle = 0.0f)
	{
		if (m_bHotVisible != visible || (visible && fabsf(angle-m_fHotAngle) >= 1e-3f))
		{
			m_bHotVisible = visible;
			m_fHotAngle = angle;
			if (visible)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof tme;
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = m_hWnd;
				_TrackMouseEvent(&tme);
			}
			Invalidate(FALSE);
		}
	}
	int GetPresetCount()
	{
		float size = m_fSize/m_fScale;
		return size < 20.0f ? 4 : (size < 40.0f ? 8 : 16);
	}
	float AngleFromPoint(float fX, float fY)
	{
		float angle = atan2f(fX-m_fCenterX, m_fCenterY-fY)*180.0f/3.14159265359f;
		float distsq = (fX-m_fCenterX)*(fX-m_fCenterX) + (fY-m_fCenterY)*(fY-m_fCenterY);
		if (distsq*6 > m_fSize*m_fSize)
			return angle;
		float as = 360.0f/GetPresetCount();
		return floorf((angle/as)+0.5f)*as;
	}

	void SendNotification(int a_nCode = C2DR_ANGLE_CHANGED)
	{
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_nCode};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}

	struct TLinearColor
	{
		WORD wR;
		WORD wG;
		WORD wB;
		BYTE bA;
		DWORD dwSRGB;

		TLinearColor() : wR(0), wG(0), wB(0), bA(0), dwSRGB(0) {}
		TLinearColor(COLORREF clrSRGB, CGammaTables const* pGT) :
			wR(pGT->m_aGamma[GetRValue(clrSRGB)]),
			wG(pGT->m_aGamma[GetGValue(clrSRGB)]),
			wB(pGT->m_aGamma[GetBValue(clrSRGB)]),
			bA(0xff), dwSRGB(((clrSRGB&0xff)<<16)|(clrSRGB&0xff00)|((clrSRGB>>16)&0xff)|0xff000000)
		{
		}
		TLinearColor(COLORREF clrSRGB, BYTE bA, CGammaTables const* pGT) :
			wR(pGT->m_aGamma[GetRValue(clrSRGB)]),
			wG(pGT->m_aGamma[GetGValue(clrSRGB)]),
			wB(pGT->m_aGamma[GetBValue(clrSRGB)]),
			bA(bA), dwSRGB(((clrSRGB&0xff)<<16)|(clrSRGB&0xff00)|((clrSRGB>>16)&0xff)|(DWORD(bA)<<24))
		{
		}
	};
	class CGammaBuffer // assumes completely opaque background
	{
	public:
		typedef TLinearColor color_type;
		typedef void row_data;
		typedef void span_data;

		CGammaBuffer(CGammaTables const* a_pGamma, BYTE* a_pData, unsigned a_nSizeX, unsigned a_nSizeY, int a_nStride) :
			m_pGT(a_pGamma), m_pData(a_nStride < 0 ? a_pData+(a_nSizeY-1)*(-a_nStride) : a_pData), m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY), m_nStride(a_nStride)
		{
		}
		~CGammaBuffer()
		{
		}

		// AGG image target methods
	public:
		unsigned width() const { return m_nSizeX; }
		unsigned height() const { return m_nSizeY; }

		//void blend_pixel(int x, int y, const color_type& c, agg::int8u cover)
		//{
		//}
		void blend_hline(int x, int y, unsigned len, color_type const& c, agg::int8u cover)
		{
			BYTE* p = m_pData + m_nStride*y + (x<<2);
			ULONG const a = ULONG(cover)*c.bA;
			if (a != 255*255)
			{
				ULONG const fa = a*0x10000/(255*255);
				ULONG const ia = 0x10000-fa;
				for (BYTE* const e = p+(len<<2); p != e; p+=4)
				{
					p[0] = m_pGT->InvGamma((m_pGT->m_aGamma[p[0]]*ia+c.wB*fa)>>16);
					p[1] = m_pGT->InvGamma((m_pGT->m_aGamma[p[1]]*ia+c.wG*fa)>>16);
					p[2] = m_pGT->InvGamma((m_pGT->m_aGamma[p[2]]*ia+c.wR*fa)>>16);
				}
			}
			else
			{
				DWORD const f = 0xff000000|c.dwSRGB;
				for (BYTE* const e = p+(len<<2); p != e; p+=4)
					*reinterpret_cast<DWORD*>(p) = f;
			}
		}
		void blend_solid_hspan(int x, int y, unsigned len, color_type const& c, agg::int8u const* covers)
		{
			BYTE* p = m_pData + m_nStride*y + (x<<2);
			for (BYTE* const e = p+(len<<2); p != e; p+=4, ++covers)
			{
				ULONG const a = ULONG(*covers)*c.bA;
				ULONG const fa = a*0x10000/(255*255);
				ULONG const ia = 0x10000-fa;
				p[0] = m_pGT->InvGamma((m_pGT->m_aGamma[p[0]]*ia+c.wB*fa)>>16);
				p[1] = m_pGT->InvGamma((m_pGT->m_aGamma[p[1]]*ia+c.wG*fa)>>16);
				p[2] = m_pGT->InvGamma((m_pGT->m_aGamma[p[2]]*ia+c.wR*fa)>>16);
			}
		}
		//void blend_color_hspan(int x, int y, unsigned len, const color_type* colors, const agg::int8u* covers, agg::int8u cover)
		//{
		//	if (covers)
		//	{
		//	}
		//	else
		//	{
		//	}
		//}

	private:
		unsigned m_nSizeX;
		unsigned m_nSizeY;
		int m_nStride;
		CGammaTables const* m_pGT;
		BYTE* const m_pData;
	};

private:
	float m_fAngle;

	bool m_bHotVisible;
	float m_fHotAngle;

	float m_fCenterX;
	float m_fCenterY;
	float m_fSize;

	bool m_bNoBackBuffer;
	HCURSOR m_hCursor;
	float m_fScale;
	CGammaTables* m_pGamma;
};

}; // namespace WTL