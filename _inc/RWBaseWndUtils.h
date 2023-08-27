
#pragma once

#ifdef __cplusplus

} // pause: extern "C"{

template<class T, class TBase>
class ATL_NO_VTABLE CChildWindowImpl : public TBase
{
public:
	STDMETHOD(Handle)(RWHWND *a_pHandle)
	{
		try
		{
			*a_pHandle = reinterpret_cast<RWHWND>(static_cast<T*>(this)->m_hWnd);
			return S_OK;
		}
		catch (...)
		{
			return a_pHandle == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
	{
		try
		{
			::SendMessage(static_cast<T*>(this)->m_hWnd, a_uMsg, a_wParam, a_lParam);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Show)(BOOL a_bShow)
	{
		static_cast<T*>(this)->ShowWindow(a_bShow ? SW_SHOW : SW_HIDE);
		return S_OK;
	}
	STDMETHOD(Move)(RECT const* a_prcPosition)
	{
		static_cast<T*>(this)->MoveWindow(a_prcPosition);
		return S_OK;
	}
	STDMETHOD(Destroy)()
	{
		static_cast<T*>(this)->DestroyWindow();
		return S_OK;
	}
	STDMETHOD(PreTranslateMessage)(MSG const* /*a_pMsg*/, BOOL /*a_bBeforeAccel*/)
	{
		return E_NOTIMPL;
	}
};

extern "C"{ // continue: extern "C"{

#endif//__cplusplus