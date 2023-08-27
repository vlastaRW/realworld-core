
#pragma once

#ifdef __cplusplus

} // pause: extern "C"{

template<class T, class TBase>
class ATL_NO_VTABLE CDesignerViewWndImpl : public CChildWindowImpl<T, TBase>
{
public:
	STDMETHOD(OnIdle)() {return S_FALSE;}
	STDMETHOD(OnDeactivate)(BOOL /*a_bCancelChanges*/) {return S_OK;}
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter /*a_eFilter*/, IEnumUnknownsInit* a_pInterfaces)
	{
		try
		{
			CComPtr<IUnknown> p;
			static_cast<T*>(this)->QueryInterface(a_iid, reinterpret_cast<void**>(&p));
			if (p == NULL)
				return S_FALSE;
			return a_pInterfaces->Insert(p);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(OptimumSize)(SIZE* /*a_pSize*/) {return E_NOTIMPL;}
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{
		static_cast<T*>(this)->GetParent().SendMessage(WM_RW_DEACTIVATE, a_bCancelChanges, 0);
		return S_OK;
	}
};

extern "C"{ // continue: extern "C"{

#endif//__cplusplus