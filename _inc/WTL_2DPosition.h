
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
#include <agg_gamma_lut.h>

namespace WTL
{

template<typename TVisualizer>
class C2DPosition :
	public CWindowImpl<C2DPosition<TVisualizer> >,
	public CThemeImpl<C2DPosition<TVisualizer> >
{
	typedef CThemeImpl<C2DPosition<TVisualizer> > themeBase;
public:
	C2DPosition(TVisualizer vis = TVisualizer()) : m_vis(vis), m_hotSpots(true), m_fPosX(0.0f), m_fPosY(0.0f), m_nFromX(0), m_nFromY(0), m_over(EHSNowhere),
		m_hCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS))),
		m_hCursorHot(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)))
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}

	enum {
		C2DP_POSITION_CHANGED = 1,
		C2DP_DRAG_FINISHED = 2
	};


	DECLARE_WND_CLASS_EX(TVisualizer::ClassName(), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(C2DPosition)
		CHAIN_MSG_MAP(themeBase)
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
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave);
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()

	// operations
public:
	void SetPosition(float a_fPosX, float a_fPosY)
	{
		if (a_fPosX != m_fPosX || a_fPosY != m_fPosY)
		{
			m_fPosX = a_fPosX;
			m_fPosY = a_fPosY;
			Invalidate(FALSE);
		}
	}

	float GetPositionX() const { return m_fPosX; }
	float GetPositionY() const { return m_fPosY; }
	void SetVisualizer(TVisualizer const& vis) { m_vis = vis; Invalidate(FALSE); }

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hdc = GetDC();
		m_fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);
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

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
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
			RECT rc;
			GetClientRect(&rc);
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			if (HitTestHotSpots(rc, pt) != EHSNowhere)
				SetCursor(m_hCursorHot);
			else
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
			if (m_clicked != EHSNowhere)
			{
				if (m_clicked == m_over)
				{
					switch (m_clicked&(EHSLeft|EHSRight))
					{
					case EHSLeft: m_fPosX = -1.0f; break;
					case EHSCenter: m_fPosX = 0.0f; break;
					case EHSRight: m_fPosX = 1.0f; break;
					}
					switch (m_clicked&(EHSTop|EHSBottom))
					{
					case EHSTop: m_fPosY = -1.0f; break;
					case EHSCenter: m_fPosY = 0.0f; break;
					case EHSBottom: m_fPosY = 1.0f; break;
					}
					Invalidate(FALSE);
					SendNotification();
					SendNotification(C2DP_DRAG_FINISHED);
				}
			}
			else
			{
				SendNotification(C2DP_DRAG_FINISHED);
			}
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCapture();
		m_nFromX = GET_X_LPARAM(a_lParam);
		m_nFromY = GET_Y_LPARAM(a_lParam);
		RECT rc;
		GetClientRect(&rc);
		POINT pt = {m_nFromX, m_nFromY};
		m_clicked = HitTestHotSpots(rc, pt);
		m_fStartX = m_fPosX;
		m_fStartY = m_fPosY;
		OnMouseMove(0, a_wParam, a_lParam, a_bHandled);
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			if (m_clicked == EHSNowhere)
			{
				int nFromX = GET_X_LPARAM(a_lParam);
				int nFromY = GET_Y_LPARAM(a_lParam);
				RECT rcWin;
				GetClientRect(&rcWin);
				float x = (2.0f*float(nFromX)/rcWin.right-1.0f)/m_vis.FactorX();
				float y = (2.0f*float(nFromY)/rcWin.bottom-1.0f)/m_vis.FactorY();
				if (x < -1.0f) x = -1.0f; else if (x > 1.0f) x = 1.0f;
				if (y < -1.0f) y = -1.0f; else if (y > 1.0f) y = 1.0f;
				if (x != m_fPosX || y != m_fPosY)
				{
					m_fPosX = x;
					m_fPosY = y;
					SendNotification();
					Invalidate(FALSE);
				}
			}
			else
			{
				RECT rc;
				GetClientRect(&rc);
				POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
				EHotSpot over = HitTestHotSpots(rc, pt);
				if (over != m_clicked)
					over = EHSNowhere;
				if (m_over != over)
				{
					m_over = over;
					Invalidate(FALSE);
				}
			}
		}
		else
		{
			RECT rc;
			GetClientRect(&rc);
			POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
			EHotSpot over = HitTestHotSpots(rc, pt);
			if (m_over != over)
			{
				m_over = over;
				if (over != EHSNowhere)
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
		return 0;
	}

	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if (m_over != EHSNowhere)
		{
			m_over = EHSNowhere;
			Invalidate(FALSE);
		}
		return FALSE;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_wParam != VK_LEFT && a_wParam != VK_RIGHT &&
			a_wParam != VK_UP && a_wParam != VK_DOWN)
			return 0;

		float const step = GetAsyncKeyState(VK_SHIFT)&0x8000 ? 50.0f : (GetAsyncKeyState(VK_CONTROL)&0x8000 ? 2.0f : 10.0f);
		switch (a_wParam)
		{
		case VK_LEFT:
			if (m_fPosX <= -1.0f) return 0;
			m_fPosX = (ceilf(m_fPosX*step-0.1f)-1)/step;
			if (m_fPosX < -1.0f) m_fPosX = -1.0f;
			break;
		case VK_RIGHT:
			if (m_fPosX >= 1.0f) return 0;
			m_fPosX = (floorf(m_fPosX*step+0.1f)+1)/step;
			if (m_fPosX > 1.0f) m_fPosX = 1.0f;
			break;
		case VK_UP:
			if (m_fPosY <= -1.0f) return 0;
			m_fPosY = (ceilf(m_fPosY*step-0.1f)-1)/step;
			if (m_fPosY < -1.0f) m_fPosY = -1.0f;
			break;
		case VK_DOWN:
			if (m_fPosY >= 1.0f) return 0;
			m_fPosY = (floorf(m_fPosY*step+0.1f)+1)/step;
			if (m_fPosY > 1.0f) m_fPosY = 1.0f;
			break;
		}
		Invalidate(FALSE);
		SendNotification();
		SendNotification(C2DP_DRAG_FINISHED);
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

		RECT rcWin;
		GetClientRect(&rcWin);

		COLORREF clr = GetSysColor(COLOR_WINDOWTEXT);
		float penR = powf(GetRValue(clr)/255.0f, 2.2f);
		float penG = powf(GetGValue(clr)/255.0f, 2.2f);
		float penB = powf(GetBValue(clr)/255.0f, 2.2f);
		clr = GetSysColor(COLOR_WINDOW);
		float bgR = powf(GetRValue(clr)/255.0f, 2.2f);
		float bgG = powf(GetGValue(clr)/255.0f, 2.2f);
		float bgB = powf(GetBValue(clr)/255.0f, 2.2f);
		clr = GetSysColor(COLOR_HIGHLIGHT);
		float hotR = powf(GetRValue(clr)/255.0f, 2.2f);
		float hotG = powf(GetGValue(clr)/255.0f, 2.2f);
		float hotB = powf(GetBValue(clr)/255.0f, 2.2f);
		if (dwStyle & WS_DISABLED)
		{
			penR = penR*0.5f+bgR*0.5f;
			penG = penG*0.5f+bgG*0.5f;
			penB = penB*0.5f+bgB*0.5f;
		}
		float fillR = penR*0.5f+bgR*0.5f;
		float fillG = penG*0.5f+bgG*0.5f;
		float fillB = penB*0.5f+bgB*0.5f;

		PAINTSTRUCT ps;
		HDC hDC2 = BeginPaint(&ps);
		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;

		CAutoVectorPtr<BYTE> pData(new BYTE[width*height*4]);

		agg::rendering_buffer rbuf;
		rbuf.attach(pData, width, height, -width*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
		// Pixel format and basic primitives renderer
		agg::pixfmt_bgra32 pixf(rbuf);

		m_vis.Render(m_fPosX, m_fPosY,
			agg::rgba8(bgR*255.0f+0.5f, bgG*255.0f+0.5f, bgB*255.0f+0.5f, 255),
			agg::rgba8(penR*255.0f+0.5f, penG*255.0f+0.5f, penB*255.0f+0.5f, 255),
			agg::rgba8(fillR*255.0f+0.5f, fillG*255.0f+0.5f, fillB*255.0f+0.5f, 255), pixf, width, height, m_fScale);
		float const size = sqrtf(width*width+height*height);
		if (m_hotSpots && size >= 10.0f && (dwStyle&WS_DISABLED) == 0)
		{
			agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
			// Scanline renderer for solid filling.
			agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
			// Rasterizer & scanline
			agg::rasterizer_scanline_aa<> ras;
			agg::scanline_p8 sl;

			agg::ellipse eTL(0, 0, size*0.1f, size*0.1f);
			agg::ellipse eTR(width, 0, size*0.1f, size*0.1f);
			agg::ellipse eBL(0, height, size*0.1f, size*0.1f);
			agg::ellipse eBR(width, height, size*0.1f, size*0.1f);
			agg::ellipse eT(width*0.5f, 0, size*0.08f, size*0.08f);
			agg::ellipse eB(width*0.5f, height, size*0.08f, size*0.08f);
			agg::ellipse eL(0, height*0.5f, size*0.08f, size*0.08f);
			agg::ellipse eR(width, height*0.5f, size*0.08f, size*0.08f);
			agg::ellipse e(width*0.5f, height*0.5f, size*0.06f, size*0.06f);
			//agg::path_storage path1;
			//path1.move_to(x0+scale*0.5f, y0+scale*0.5f);
			//path1.line_to(x1-scale*0.5f, y0+scale*0.5f);
			//path1.line_to(x1-scale*0.5f, y1-scale*0.5f);
			//path1.line_to(x0+scale*0.5f, y1-scale*0.5f);
			//path1.close_polygon();

			//agg::conv_dash<agg::path_storage> dash(path1);
		 //   agg::conv_stroke<agg::conv_dash<agg::path_storage> > dash_stroke(dash);
			//dash.add_dash(2*scale, 2*scale);
			//dash_stroke.line_join(agg::miter_join);
			//dash_stroke.width(scale);
			//ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
			//ras.add_path(dash_stroke);
			//agg::render_scanlines(ras, sl, ren);

			agg::path_storage normal;
			agg::path_storage hot;
			(m_over == EHSTopLeft		? hot : normal).concat_path(eTL);
			(m_over == EHSTopRight		? hot : normal).concat_path(eTR);
			(m_over == EHSBottomLeft	? hot : normal).concat_path(eBL);
			(m_over == EHSBottomRight	? hot : normal).concat_path(eBR);
			(m_over == EHSTop			? hot : normal).concat_path(eT);
			(m_over == EHSBottom		? hot : normal).concat_path(eB);
			(m_over == EHSLeft			? hot : normal).concat_path(eL);
			(m_over == EHSRight			? hot : normal).concat_path(eR);
			(m_over == EHSCenter		? hot : normal).concat_path(e);
			ren.color(agg::rgba8(hotR*255.0f+0.5f, hotG*255.0f+0.5f, hotB*255.0f+0.5f, 64));
			ras.add_path(normal);
			agg::render_scanlines(ras, sl, ren);
			agg::conv_stroke<agg::path_storage> pge1(normal);
			ren.color(agg::rgba8(penR*255.0f+0.5f, penG*255.0f+0.5f, penB*255.0f+0.5f, 64));
			pge1.width(m_fScale);
			ras.add_path(pge1);
			agg::render_scanlines(ras, sl, ren);

			ren.color(agg::rgba8(hotR*255.0f+0.5f, hotG*255.0f+0.5f, hotB*255.0f+0.5f, 255));
			ras.add_path(hot);
			agg::render_scanlines(ras, sl, ren);
			pge1.attach(hot);
			ren.color(agg::rgba8(penR*255.0f+0.5f, penG*255.0f+0.5f, penB*255.0f+0.5f, 255));
			pge1.width(m_fScale);
			ras.add_path(pge1);
			agg::render_scanlines(ras, sl, ren);
		}

		// gamma correction
		agg::gamma_lut<agg::int8u, agg::int8u, 8, 8> gp(2.2);
		pixf.apply_gamma_inv(gp);

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
	void SendNotification(int a_nCode = C2DP_POSITION_CHANGED)
	{
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_nCode};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}

	enum EHotSpot
	{
		EHSCenter = 0,
		EHSLeft = 1,
		EHSRight = 2,
		EHSTop = 4,
		EHSBottom = 8,
		EHSTopLeft = EHSTop+EHSLeft,
		EHSTopRight = EHSTop+EHSRight,
		EHSBottomLeft = EHSBottom+EHSLeft,
		EHSBottomRight = EHSBottom+EHSRight,
		EHSNowhere = 16
	};
	float Dist(float x1, float y1, float x2, float y2)
	{
		return sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	}
	EHotSpot HitTestHotSpots(RECT const& wnd, POINT pt)
	{
		if (!m_hotSpots)
			return EHSNowhere;
		float const size = sqrtf((wnd.right-wnd.left)*(wnd.right-wnd.left)+(wnd.bottom-wnd.top)*(wnd.bottom-wnd.top));
		if (size < 10.0f)
			return EHSNowhere;
		if (Dist(pt.x, pt.y, wnd.left, wnd.top) < size*0.1f)
			return EHSTopLeft;
		if (Dist(pt.x, pt.y, wnd.right, wnd.top) < size*0.1f)
			return EHSTopRight;
		if (Dist(pt.x, pt.y, wnd.left, wnd.bottom) < size*0.1f)
			return EHSBottomLeft;
		if (Dist(pt.x, pt.y, wnd.right, wnd.bottom) < size*0.1f)
			return EHSBottomRight;
		if (Dist(pt.x, pt.y, (wnd.left+wnd.right)*0.5f, wnd.top) < size*0.08f)
			return EHSTop;
		if (Dist(pt.x, pt.y, (wnd.left+wnd.right)*0.5f, wnd.bottom) < size*0.08f)
			return EHSBottom;
		if (Dist(pt.x, pt.y, wnd.left, (wnd.top+wnd.bottom)*0.5f) < size*0.08f)
			return EHSLeft;
		if (Dist(pt.x, pt.y, wnd.right, (wnd.top+wnd.bottom)*0.5f) < size*0.08f)
			return EHSRight;
		if (Dist(pt.x, pt.y, (wnd.left+wnd.right)*0.5f, (wnd.top+wnd.bottom)*0.5f) < size*0.06f)
			return EHSCenter;
		return EHSNowhere;
	}

private:
	TVisualizer m_vis;
	float m_fPosX;
	float m_fPosY;
	float m_fStartX;
	float m_fStartY;
	int m_nFromX;
	int m_nFromY;
	HCURSOR m_hCursor;
	HCURSOR m_hCursorHot;
	float m_fScale;
	bool m_hotSpots;
	EHotSpot m_over;
	EHotSpot m_clicked;
};

struct CRectangleVisualizer
{
	CRectangleVisualizer(float a_fSizeX = 0.5f, float a_fSizeY = 0.5f) : m_fSizeX(a_fSizeX), m_fSizeY(a_fSizeY) {}

	static LPCWSTR ClassName() { return _T("WTL_2DPositionWndClass"); }

	void Render(float posX, float posY, agg::rgba8 clrBG, agg::rgba8 clrPen, agg::rgba8 clrFill, agg::pixfmt_bgra32& pixf, int width, int height, float scale)
	{
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		renb.clear(clrBG);
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		float x0 = width*(1.0f-m_fSizeX)*(posX+1.0f)*0.5f;
		float x1 = x0+width*m_fSizeX;
		float y0 = height*(1.0f-m_fSizeY)*(posY+1.0f)*0.5f;
		float y1 = y0+height*m_fSizeY;

		// fill
        agg::path_storage path1;
		path1.move_to(x0+scale*0.5f, y0+scale*0.5f);
		path1.line_to(x1-scale*0.5f, y0+scale*0.5f);
		path1.line_to(x1-scale*0.5f, y1-scale*0.5f);
		path1.line_to(x0+scale*0.5f, y1-scale*0.5f);
        path1.close_polygon();

		// outline
        ren.color(clrFill);
        ras.add_path(path1);
        agg::render_scanlines(ras, sl, ren);
		agg::conv_stroke<agg::path_storage> pge1(path1);
		ren.color(clrPen);
		pge1.width(scale);
		ras.add_path(pge1);
		agg::render_scanlines(ras, sl, ren);
	}

	float FactorX() const { return 1.0f-m_fSizeX; }
	float FactorY() const { return 1.0f-m_fSizeY; }

	float m_fSizeX;
	float m_fSizeY;
};

typedef C2DPosition<CRectangleVisualizer> CRectanglePosition;

}; // namespace WTL