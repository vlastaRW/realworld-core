// DesignerViewNULL.h : Declaration of the CDesignerViewNULL

#pragma once

#include <htmlhelp.h>


// CDesignerViewNULL

class ATL_NO_VTABLE CDesignerViewNULL : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDesignerViewWndImpl<CDesignerViewNULL, IDesignerView>,
	public CWindowImpl<CDesignerViewNULL>
{
public:
	CDesignerViewNULL() : m_pszText(NULL), m_pszHelp(NULL)
	{
	}
	~CDesignerViewNULL()
	{
		if (m_pszText) free(m_pszText);
		if (m_pszHelp) free(m_pszHelp);
	}
	void SetTexts(LPCTSTR a_pszText, LPCTSTR a_pszHelp, HFONT a_hFont)
	{
		m_pszText = _tcsdup(a_pszText == NULL ? _T("") : a_pszText);
		m_pszHelp = _tcsdup(a_pszHelp == NULL ? _T("") : a_pszHelp);
		m_hFont = a_hFont;
	}

	DECLARE_WND_CLASS_EX(_T("DesignerViewNULLClass"), CS_VREDRAW|CS_HREDRAW|CS_PARENTDC, COLOR_3DFACE);

	BEGIN_COM_MAP(CDesignerViewNULL)
		COM_INTERFACE_ENTRY(IDesignerView)
		COM_INTERFACE_ENTRY(IChildWindow)
	END_COM_MAP()

	BEGIN_MSG_MAP(CDesignerViewNULL)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
	END_MSG_MAP()

	// message handlers
public:
	LRESULT OnPaint(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		PAINTSTRUCT tPS;
		HDC hDC = BeginPaint(&tPS);
		HGDIOBJ hFont = SelectObject(hDC, m_hFont);
		COLORREF tClr = SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
		DrawText(hDC, m_pszText, -1, &rcClient, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		SetBkColor(hDC, tClr);
		SelectObject(hDC, hFont);
		EndPaint(&tPS);
		return 0;
	}

	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pszHelp)
		{
			HH_POPUP hhp;
			hhp.cbStruct = sizeof(hhp);
			RECT rcItem;
			::GetWindowRect((HWND)pHelpInfo->hItemHandle, &rcItem);
			hhp.pt.x = (rcItem.right+rcItem.left)>>1;
			hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
			hhp.hinst = _pModule->get_m_hInst();
			hhp.idString = 0;
			hhp.pszText = m_pszHelp;
			hhp.clrForeground = 0xffffffff;
			hhp.clrBackground = 0xffffffff;
			hhp.rcMargins.left = -1;
			hhp.rcMargins.top = -1;
			hhp.rcMargins.right = -1;
			hhp.rcMargins.bottom = -1;
			hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
			HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
			return 0;
		}
		a_bHandled = FALSE;
		return 0;
	}

private:
	LPTSTR m_pszText;
	LPTSTR m_pszHelp;
	HFONT m_hFont;
};

