
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <ContextHelpDlg.h>
#include <math.h>
#include <agg_bspline.h>

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_conv_dash.h>
#include <agg_path_storage.h>
#include <agg_ellipse.h>
#include <agg_gamma_lut.h>
#include <agg_curves.h>

typedef std::vector< std::pair<float, float> > TCurvePoints;

inline void GetCurveValues(TCurvePoints& a_aCurvePts, std::vector<float>& a_aValues, float a_fAt, int a_nValues, float a_fStep=0.0f, bool a_bBSpline = true)
{
	if(a_aCurvePts.size() < 2)
		return;
	if(a_aCurvePts.size() == 2)
		a_bBSpline = false;

	agg::bspline cBSpline;
	if(a_bBSpline)
	{
		cBSpline.init(a_aCurvePts.size());
		for(size_t i=0; i<a_aCurvePts.size(); ++i)
			cBSpline.add_point(a_aCurvePts[i].first, a_aCurvePts[i].second);
		cBSpline.prepare();
	}

	for(int i=0; i<a_nValues; ++i)
	{
		float fAt = a_fAt + i * a_fStep;
		if(fAt <= a_aCurvePts[0].first)
		{
			a_aValues.push_back(a_aCurvePts[0].second);
			continue;
		}
		if(fAt >= a_aCurvePts[a_aCurvePts.size()-1].first)
		{
			a_aValues.push_back(a_aCurvePts[a_aCurvePts.size()-1].second);
			continue;
		}
		if(a_bBSpline)
			a_aValues.push_back(cBSpline.get(fAt));
		else
		{
			int iP1 = 0;
			int iP2 = 0;
			int iP = 0;
			for each(std::pair<float, float> p in a_aCurvePts)
			{
				if(fAt >= p.first)
					iP1 = iP;
				if(fAt <= p.first)
				{
					iP2 = iP;
					break;
				}
				++iP;
			}
			float const fD = a_aCurvePts[iP2].first - a_aCurvePts[iP1].first;
			float fRatio = fD != 0.0f ? static_cast<float>(fAt - a_aCurvePts[iP1].first) / fD : 1.0f;
			a_aValues.push_back(a_aCurvePts[iP2].second * fRatio + a_aCurvePts[iP1].second * (1-fRatio));
		}
	}
}

inline float GetCurveValue(float a_fAt, TCurvePoints& a_aCurvePts, bool a_bBSpline = true)
{
	if(a_aCurvePts.size() < 2)
		return 0;
	if(a_aCurvePts.size() == 2)
		a_bBSpline = false;

	if(a_fAt <= a_aCurvePts[0].first)
		return a_aCurvePts[0].second;

	if(a_fAt >= a_aCurvePts[a_aCurvePts.size()-1].first)
		return a_aCurvePts[a_aCurvePts.size()-1].second;

	for each(std::pair<float, float> p in a_aCurvePts)
	{
		if(p.first == a_fAt)
			return p.second;
	}

	if(a_bBSpline)
	{
		agg::bspline cBSpline;
		cBSpline.init(a_aCurvePts.size());
		for(size_t i=0; i<a_aCurvePts.size(); ++i)
			cBSpline.add_point(a_aCurvePts[i].first, a_aCurvePts[i].second);
		cBSpline.prepare();
		return cBSpline.get(a_fAt);
	}else
	{
		int iP1 = 0;
		int iP2 = 0;
		int iP = 0;
		for each(std::pair<float, float> p in a_aCurvePts)
		{
			if(a_fAt >= p.first)
				iP1 = iP;
			if(a_fAt <= p.first)
			{
				iP2 = iP;
				break;
			}
			++iP;
		}
		float fRatio = static_cast<float>(a_fAt - a_aCurvePts[iP1].first) / (a_aCurvePts[iP2].first - a_aCurvePts[iP1].first);
		return a_aCurvePts[iP2].second * fRatio + a_aCurvePts[iP1].second * (1-fRatio);
	}
	return 0;
}

class CCurveControl :
	public CWindowImpl<CCurveControl>,
	public CThemeImpl<CCurveControl>
{
public:
	CCurveControl() : m_bNoBackBuffer(false), m_nDragIndex(-1),
		m_hCursorMove(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL))),
		m_hCursorStd(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW))),
		m_hCursorCross(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS))),
		m_bBSpline(true)
	{
		m_aPoints.push_back(std::make_pair(0, 0));
		m_aPoints.push_back(std::make_pair(255, 255));
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CCurveControl()
	{
	}

	enum {
		CPC_POINT_MOVED = 1,
		CPC_POINT_DELETED,
		CPC_DRAG_FINISHED,
		CPC_DRAG_START
	};


	DECLARE_WND_CLASS_EX(_T("CurveControlWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CCurveControl)
		CHAIN_MSG_MAP(CThemeImpl<CCurveControl>)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		//MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
	END_MSG_MAP()


	static void ParseCurvePoints(OLECHAR const* a_psz, TCurvePoints& a_aCurvePts)
	{
		float nNumber1;
		while (true)
		{
			OLECHAR* p2 = NULL;
			float f = wcstod(a_psz, &p2);
			a_psz = p2;
			if (*a_psz == L',')
			{
				nNumber1 = f;
			}
			else if (*a_psz == L';')
			{
				a_aCurvePts.push_back(std::make_pair(nNumber1, f));
			}
			else if (*a_psz != L' ')
			{
				break;
			}
			++a_psz;
		}
	}
	static void SerializeCurvePoints(TCurvePoints const& a_aCurvePts, CComBSTR& a_bstr)
	{
		for each(std::pair<float, float> p in a_aCurvePts)
		{
			wchar_t psz[48];
			swprintf(psz, L"%g,%g;", p.first, p.second);
			a_bstr.Append(psz);
		}
	}


	// operations
public:
	void SetPoints(TCurvePoints const& a_aPts)
	{
		if(a_aPts.size() >= 2)
		{
			m_aPoints.clear();
			for each(std::pair<float, float> p in a_aPts)
				m_aPoints.push_back(p);
			Invalidate(FALSE);
		}
	}

	void GetPoints(TCurvePoints* a_pPts) const
	{
		a_pPts->clear();
		for each(std::pair<float, float> p in m_aPoints)
			a_pPts->push_back(p);
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		this->SetFocus();
		return 0;
	}
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (reinterpret_cast<HWND>(a_wParam) == m_hWnd)
		{
			DWORD dwPos = GetMessagePos();
			POINT tPt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
			ScreenToClient(&tPt);
			if(GetPtIndex(tPt.x, tPt.y) >= 0)
				SetCursor(m_hCursorMove);
			else
			{
				RECT rcWin;
				GetClientRect(&rcWin);
				int nY = rcWin.bottom + (rcWin.top - rcWin.bottom)*(GetCurveValue(255*static_cast<float>(tPt.x)/(rcWin.right-1), m_aPoints, m_bBSpline)/255.0f);
				if(nY > tPt.y - abs(rcWin.bottom/10) && nY < tPt.y + abs(rcWin.bottom/10))
					SetCursor(m_hCursorCross);
				else
					SetCursor(m_hCursorStd);
			}
			return TRUE;
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			SendNotification(CPC_DRAG_FINISHED);
			Invalidate(FALSE);
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCapture();

		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		RECT rcWin;
		GetClientRect(&rcWin);
		int nMouseX = 255.0f * tPt.x / rcWin.right;
		int nMouseY = 255.0f * (tPt.y - rcWin.bottom) / -rcWin.bottom;
		int nY = GetCurveValue(nMouseX, m_aPoints, m_bBSpline);
		m_nDragIndex = GetPtIndex(GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		if(m_nDragIndex == -1 &&
			nMouseY > nY - 25 && nMouseY < nY + 25 &&
			nMouseX > m_aPoints[0].first &&
			nMouseX < m_aPoints[m_aPoints.size()-1].first)
		{
			for(TCurvePoints::iterator i=m_aPoints.begin(); i!=m_aPoints.end(); ++i)
			{
				if(i->first > nMouseX)
				{
					m_aPoints.insert(i, std::make_pair(nMouseX, nMouseY));
					break;
				}
			}
			m_nDragIndex = GetPtIndex(GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		}
		SendNotification(CPC_DRAG_START);

		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd && m_nDragIndex >= 0)
		{
			int nX = GET_X_LPARAM(a_lParam);
			int nY = GET_Y_LPARAM(a_lParam);
			RECT rcWin;
			GetClientRect(&rcWin);

			if(nX < 0) nX = 0;
			if(nX > rcWin.right) nX = rcWin.right;
			if(nY < 0) nY = 0;
			if(nY > rcWin.bottom) nY = rcWin.bottom;

			m_aPoints[m_nDragIndex].first = 255.0f * nX / rcWin.right;
			m_aPoints[m_nDragIndex].second = 255 - 255.0f * nY / rcWin.bottom;
			if(m_nDragIndex == 0)
			{
				if(m_aPoints[0].first >= m_aPoints[1].first)
					m_aPoints[0].first = m_aPoints[1].first-1;
			}
			else if(m_nDragIndex == m_aPoints.size()-1)
			{
				if(m_aPoints[m_nDragIndex].first <= m_aPoints[m_nDragIndex-1].first)
					m_aPoints[m_nDragIndex].first = m_aPoints[m_nDragIndex-1].first+1;
			}else
			{
				if(m_aPoints[m_nDragIndex].first <= m_aPoints[m_nDragIndex-1].first)
					m_aPoints[m_nDragIndex].first = m_aPoints[m_nDragIndex-1].first+1;
				if(m_aPoints[m_nDragIndex].first >= m_aPoints[m_nDragIndex+1].first)
					m_aPoints[m_nDragIndex].first = m_aPoints[m_nDragIndex+1].first-1;
			}

			SendNotification();
			Invalidate(FALSE);
		}
		return 0;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRet = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		//LRESULT lRet = DefWindowProc(a_uMsg, a_wParam, a_lParam);
		if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
			::SetFocus(m_hWnd);
//			this->SetFocus();
		return lRet;
	}
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		float fX, fY;
		switch(wParam)
		{
		case VK_LEFT:
			GetPointPosition(fX, fY);
			SetPointPosition(fX-1, fY);
			bHandled = TRUE;
			SendNotification(CPC_POINT_MOVED);
			return 0;
		case VK_RIGHT:
			GetPointPosition(fX, fY);
			SetPointPosition(fX+1, fY);
			bHandled = TRUE;
			SendNotification(CPC_POINT_MOVED);
			return 0;
		case VK_UP:
			GetPointPosition(fX, fY);
			SetPointPosition(fX, fY+1);
			bHandled = TRUE;
			SendNotification(CPC_POINT_MOVED);
			return 0;
		case VK_DOWN:
			GetPointPosition(fX, fY);
			SetPointPosition(fX, fY-1);
			bHandled = TRUE;
			SendNotification(CPC_POINT_MOVED);
			return 0;
		case VK_DELETE:
			DeleteNode();
			bHandled = TRUE;
			return 0;
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		switch(wParam)
		{
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			bHandled = TRUE;
			SendNotification(CPC_DRAG_FINISHED);
			return 0;
		}
		bHandled = FALSE;
		return 0;
	}
	
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		DWORD dwStyle = GetWindowLong(GWL_STYLE);

		PAINTSTRUCT ps;
		HDC hDC2 = BeginPaint(&ps);
		RECT rcWin;
		GetClientRect(&rcWin);

		COLORREF clrPen = GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrBG = GetSysColor(COLOR_WINDOW);
		if (dwStyle & WS_DISABLED)
		{
			clrPen = (((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
					 (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
					 (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		}
		COLORREF clrDimmed = (((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
							 (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
							 (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		COLORREF clrFill = max(clrPen&0xff, clrBG&0xff) | //(((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
						   (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
						   (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));

		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;

		CAutoVectorPtr<BYTE> pData(new BYTE[width*height*4]);

		agg::rendering_buffer rbuf;
		rbuf.attach(pData.m_p, width, height, -width*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
		// Pixel format and basic primitives renderer
		agg::pixfmt_bgra32 pixf(rbuf);
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		renb.clear(agg::rgba8(GetRValue(clrBG), GetGValue(clrBG), GetBValue(clrBG), 255));
		// Scanline renderer for solid filling.
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		// Rasterizer & scanline
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		// Lines
		{
			agg::path_storage path2;
			path2.move_to(0.5, static_cast<int>(height*0.25)+0.5);
			path2.line_to(width-0.5, static_cast<int>(height*0.25)+0.5);
			path2.move_to(0.5, static_cast<int>(height*0.5)+0.5);
			path2.line_to(width-0.5, static_cast<int>(height*0.5)+0.5);
			path2.move_to(0.5, static_cast<int>(height*0.75)+0.5);
			path2.line_to(width-0.5, static_cast<int>(height*0.75)+0.5);
			path2.move_to(static_cast<int>(width*0.25)+0.5, 0.5);
			path2.line_to(static_cast<int>(width*0.25)+0.5, height-0.5);
			path2.move_to(static_cast<int>(width*0.5)+0.5, 0.5);
			path2.line_to(static_cast<int>(width*0.5)+0.5, height-0.5);
			path2.move_to(static_cast<int>(width*0.75)+0.5, 0.5);
			path2.line_to(static_cast<int>(width*0.75)+0.5, height-0.5);

			agg::conv_dash<agg::path_storage> dash(path2);
			agg::conv_stroke<agg::conv_dash<agg::path_storage> > dash_stroke(dash);
			dash.add_dash(5, 3);
			dash_stroke.line_join(agg::miter_join);
			dash_stroke.width(0.5);
			ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
			ras.add_path(dash_stroke);
			agg::render_scanlines(ras, sl, ren);
		}

		// Curve
		std::vector<float> aValues;
		GetCurveValues(m_aPoints, aValues, 0.0f, width, 255.0f/(width-1), m_bBSpline);
		for(std::vector<float>::iterator i = aValues.begin(); i != aValues.end(); i++)
		{
			if(*i < 0.0f) *i=0.0f;
			if(*i > 255.0f) *i=255.0f;
		}
		if(aValues.size() > 0)
		{
			agg::path_storage path1;
			path1.move_to(0.5, height-0.5 - (height-1)*aValues[0]/255.0f);
			for(int i=1; i<width && ULONG(i)<aValues.size(); ++i)
			{
				path1.line_to(i+0.5, height-0.5 - (height-1)*aValues[i]/255.0f);
			}

			agg::conv_stroke<agg::path_storage> dash_stroke(path1);
			dash_stroke.line_join(agg::miter_join);
			dash_stroke.width(1.0);
			ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
			ras.add_path(dash_stroke);
			agg::render_scanlines(ras, sl, ren);
		}

		// Points
		int iPoint=0;
		for each(std::pair<float, float> p in m_aPoints)
		{
			double x = 0.5+(width-1)*p.first/255.0f;
			double y = height-0.5 - (height-1)*p.second/255.0f;

			agg::ellipse e;
			if(iPoint == m_nDragIndex)
				ren.color(agg::rgba8(GetRValue(clrFill), GetGValue(clrFill), GetBValue(clrFill), 255));
			else
				ren.color(agg::rgba8(GetRValue(clrDimmed), GetGValue(clrDimmed), GetBValue(clrDimmed), 255));
			e.init(x, y, 2.5, 2.5, 16);
			ras.add_path(e);
			agg::render_scanlines(ras, sl, ren);
			agg::conv_stroke<agg::ellipse> pge1(e);
			ren.color(agg::rgba8(GetRValue(clrPen), GetGValue(clrPen), GetBValue(clrPen), 255));
			pge1.width(1);
			ras.add_path(pge1);
			agg::render_scanlines(ras, sl, ren);
			++iPoint;
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


	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTARROWS;
	}
	void SetInterpolation(bool a_bBSpline)
	{
		if(a_bBSpline != m_bBSpline)
		{
			m_bBSpline = a_bBSpline;
			if(m_hWnd)
				Invalidate();
		}
	}

	bool SetPointPosition(float a_fX, float a_fY)
	{
		if(m_nDragIndex == -1)
			return false;
		if(a_fX < 0) a_fX = 0;
		if(a_fX > 255) a_fX = 255;
		if(a_fY < 0) a_fY = 0;
		if(a_fY > 255) a_fY = 255;
		m_aPoints[m_nDragIndex].first = a_fX;
		m_aPoints[m_nDragIndex].second = a_fY;
		if(m_nDragIndex == 0)
		{
			if(m_aPoints[0].first >= m_aPoints[1].first)
				m_aPoints[0].first = m_aPoints[1].first-1;
		}
		else if(m_nDragIndex == m_aPoints.size()-1)
		{
			if(m_aPoints[m_nDragIndex].first <= m_aPoints[m_nDragIndex-1].first)
				m_aPoints[m_nDragIndex].first = m_aPoints[m_nDragIndex-1].first+1;
		}else
		{
			if(m_aPoints[m_nDragIndex].first <= m_aPoints[m_nDragIndex-1].first)
				m_aPoints[m_nDragIndex].first = m_aPoints[m_nDragIndex-1].first+1;
			if(m_aPoints[m_nDragIndex].first >= m_aPoints[m_nDragIndex+1].first)
				m_aPoints[m_nDragIndex].first = m_aPoints[m_nDragIndex+1].first-1;
		}
		Invalidate();
		return true;
	}

	bool GetPointPosition(int& a_nX, int& a_nY)
	{
		if(m_nDragIndex == -1)
			return false;
		a_nX = m_aPoints[m_nDragIndex].first;
		a_nY = m_aPoints[m_nDragIndex].second;
		return true;
	}
	bool GetPointPosition(float& a_fX, float& a_fY)
	{
		if(m_nDragIndex == -1)
			return false;
		a_fX = m_aPoints[m_nDragIndex].first;
		a_fY = m_aPoints[m_nDragIndex].second;
		return true;
	}

	void DeleteNode()
	{
		if(m_nDragIndex <= 0 || ULONG(m_nDragIndex+1) >= m_aPoints.size())
			return;
		TCurvePoints::iterator i = m_aPoints.begin();
		i += m_nDragIndex;
		m_aPoints.erase(i);
		m_nDragIndex = -1;
		SendNotification(CPC_POINT_DELETED);
		Invalidate(FALSE);
	}
private:
	void SendNotification(int a_nCode = CPC_POINT_MOVED)
	{
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_nCode};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}
	int GetPtIndex(int a_nX, int a_nY)
	{
		if(m_aPoints.size() < 2)
			return -1;
		RECT rcWin;
		GetClientRect(&rcWin);
		if(a_nX < rcWin.left || a_nX > rcWin.right || a_nY < rcWin.top || a_nY > rcWin.bottom)
			return -1;
		if(a_nX < rcWin.right*m_aPoints[0].first/255.0f)
			return 0;
		if(a_nX > rcWin.right*m_aPoints[m_aPoints.size()-1].first/255.0f)
			return m_aPoints.size()-1;
		int iPoint=0;
		for each(std::pair<float, float> p in m_aPoints)
		{
			int nX = rcWin.right*p.first/255.0f;
			int nY = rcWin.bottom - rcWin.bottom*p.second/255.0f;
			if (((nX-a_nX)*(nX-a_nX) + (nY-a_nY)*(nY-a_nY)) <= 16)
				return iPoint;
			++iPoint;
		}
		return -1;
	}

private:
	bool m_bBSpline;
	TCurvePoints m_aPoints;
	int m_nDragIndex;
	bool m_bNoBackBuffer;
	HCURSOR m_hCursorMove;
	HCURSOR m_hCursorStd;
	HCURSOR m_hCursorCross;
};
