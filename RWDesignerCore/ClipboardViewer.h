
#pragma once


template<typename T>
class CClipboardViewer
{
public:
	CClipboardViewer() : m_hWndNextViewer(NULL)
	{
	}

	BEGIN_MSG_MAP(CClipboardViewer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCBChain)
		MESSAGE_HANDLER(WM_DRAWCLIPBOARD, OnDrawClipBoard)
	END_MSG_MAP()

	LRESULT OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		m_hWndNextViewer = SetClipboardViewer(static_cast<T*>(this)->m_hWnd);
		return 0;
	}
	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		ChangeClipboardChain(static_cast<T*>(this)->m_hWnd, m_hWndNextViewer);
		return 0;
	}
	LRESULT OnChangeCBChain(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (reinterpret_cast<HWND>(a_wParam) == m_hWndNextViewer)
		{
			m_hWndNextViewer = reinterpret_cast<HWND>(a_lParam);
			return 0;
		}
		if (m_hWndNextViewer != NULL)
		{
			::SendMessage(m_hWndNextViewer, WM_CHANGECBCHAIN, a_wParam, a_lParam);
		}
		return 0;
	}
	LRESULT OnDrawClipBoard(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		static_cast<T*>(this)->OnIdle();
		if (m_hWndNextViewer != NULL)
		{
			::SendMessage(m_hWndNextViewer, WM_DRAWCLIPBOARD, a_wParam, a_lParam);
		}
		return 0;
	}

private:
	HWND m_hWndNextViewer;
};

