// SharedStateToolbar.h : Declaration of the CSharedStateToolbar

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"


// CSharedStateToolbar

class ATL_NO_VTABLE CSharedStateToolbar :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateToolbar, &CLSID_SharedStateToolbar>,
	public ISharedState,
	public ISharedStateToolbar
{
public:
	CSharedStateToolbar() : m_bVisible(TRUE)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateToolbar)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY(ISharedStateToolbar)
END_COM_MAP()


	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID);
	STDMETHOD(ToText)(BSTR* a_pbstrText);
	STDMETHOD(FromText)(BSTR a_bstrText);

	// ISharedStateToolbar methods
public:
	STDMETHOD(IsVisible)();
	STDMETHOD(SetVisible)(BOOL a_bVisible);

private:
	BOOL m_bVisible;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateToolbar), CSharedStateToolbar)
