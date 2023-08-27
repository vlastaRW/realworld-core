
#pragma once

#define WM_SUPPORTSCONFIGSELECT (WM_APP+5727)
#define WM_CONFIGSELECT (WM_APP+5728)
#define SUPPORT_RESPONSE_OK 0x59a7f095
#define SUPPORT_RESPONSE_FAILED 0x59a7f096

inline bool SupportsConfigSelect(HWND a_hWnd)
{
	while (a_hWnd)
	{
		if (::SendMessage(a_hWnd, WM_SUPPORTSCONFIGSELECT, 0, 0) == SUPPORT_RESPONSE_OK)
			return true;
		if ((::GetWindowLong(a_hWnd, GWL_STYLE)&WS_CHILD) != WS_CHILD)
			return false;
		a_hWnd = ::GetParent(a_hWnd);
	}
	return false;
}

inline bool ConfigSelect(HWND a_hWnd, IConfig* a_pConfig)
{
	while (a_hWnd)
	{
		LRESULT lRes = ::SendMessage(a_hWnd, WM_CONFIGSELECT, 0, reinterpret_cast<LPARAM>(a_pConfig));
		if (lRes == SUPPORT_RESPONSE_OK)
			return true;
		else if (lRes == SUPPORT_RESPONSE_FAILED)
			return false;
		if ((::GetWindowLong(a_hWnd, GWL_STYLE)&WS_CHILD) != WS_CHILD)
			return false;
		a_hWnd = ::GetParent(a_hWnd);
	}
	return false;
}
