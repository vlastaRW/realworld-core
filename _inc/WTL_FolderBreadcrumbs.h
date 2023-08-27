
#pragma once

#include <math.h>

namespace WTL
{

class CDataFolder : public std::vector<std::tstring>
{
public:
	static LPCTSTR WndClass() { return _T("FolderBreadcrumbs"); }

	HICON CreateIcon(int a_nSize);

	void SetText(LPCTSTR a_pszText)
	{
		clear();
		LPCTSTR psz = _tcschr(a_pszText, _T('\\'));
		while (psz)
		{
			if (psz > a_pszText)
			{
				push_back(std::tstring(a_pszText, psz));
			}
			a_pszText = psz+1;
			psz = _tcschr(a_pszText, _T('\\'));
		}
		if (*a_pszText)
		{
			push_back(a_pszText);
		}
	}
	int GetText(int a_nBuffer, LPTSTR a_pszBuffer)
	{
		int nLen = max(size(), 1);
		for (const_iterator i = begin(); i != end(); ++i)
			nLen += i->length();
		if (a_nBuffer < nLen)
			return nLen-1;
		for (const_iterator i = begin(); i != end(); ++i)
		{
			if (i != begin())
				*(a_pszBuffer++) = _T('\\');
			LPCTSTR p = i->c_str();
			while (*(a_pszBuffer++) = *(p++)) ;
		}
		return nLen-1;
	}
};

template<class TData>
class CBreadcrumbCtrl :
	public CWindowImpl<CBreadcrumbCtrl<TData> >,
	public CThemeImpl<CBreadcrumbCtrl<TData> >
{
public:
	CBreadcrumbCtrl()
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CBreadcrumbCtrl()
	{
	}

	enum { CAN_COLOR_CHANGED = 1 };


	DECLARE_WND_CLASS_EX(TData::WndClass(), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CBreadcrumbCtrl)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		CHAIN_MSG_MAP(CThemeImpl<CBreadcrumbCtrl<TData> >)
		MESSAGE_HANDLER(WM_SETFONT, OnSetFont)
		MESSAGE_HANDLER(WM_GETFONT, OnGetFont)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
	END_MSG_MAP()

	// operations
public:
	void Data(TData const& a_tData)
	{
		m_tData = a_tData;
	}
	TData const& Data() const
	{
		return m_tData;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CREATESTRUCT const* a_pCS = reinterpret_cast<CREATESTRUCT const*>(a_lParam);
		if (a_pCS && a_pCS->lpszName && *a_pCS->lpszName)
			m_tData.SetText(a_pCS->lpszName);
		RECT rc;
		GetClientRect(&rc);
		m_wndEdit.Create(m_hWnd, rc, a_pCS->lpszName, WS_CHILD);
		return 0;
	}

	LRESULT OnSetFont(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return m_wndEdit.SendMessage(a_uMsg, a_wParam, a_lParam);
	}

	LRESULT OnGetFont(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return m_wndEdit.SendMessage(a_uMsg, a_wParam, a_lParam);
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
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
		SetCapture();
		SendNotification();
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
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
		//DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);

		RECT r;
		PAINTSTRUCT ps;
		HDC hDC = ::BeginPaint(m_hWnd, &ps);
		GetClientRect(&r);

		HGDIOBJ hOldFont = SelectObject(hDC, m_wndEdit.GetFont());
		for (TData::const_iterator i = m_tData.begin(); i != m_tData.end(); ++i)
		{
			TextOut(hDC, 0, 0, i->c_str(), i->length());
		}
		SelectObject(hDC, hOldFont);

		::EndPaint(m_hWnd, &ps);
		return 0;
	}

private:
	void SendNotification()
	{
		//HWND hPar = GetParent();
		//NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), CAN_COLOR_CHANGED};
		//SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
	}

private:
	TData m_tData;
	CEdit m_wndEdit;
	HFONT m_hFont;
};

}; // namespace WTL