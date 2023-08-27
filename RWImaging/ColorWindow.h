// ColorWindow.h : Declaration of the CColorWindow

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CColorWindow

class ATL_NO_VTABLE CColorWindow :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CColorWindow, &CLSID_ColorWindow>,
	public IColorWindow,
	public IGlobalConfigFactory
{
public:
	CColorWindow()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CColorWindow)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryColorSwatch)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CColorWindow)
	COM_INTERFACE_ENTRY(IColorWindow)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IColorWindow methods
public:
	STDMETHOD(DoModal)(RWHWND a_hParent, LCID a_tLocaleID, TColor* a_pColor, BYTE a_bAlpha);

	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE* a_pPriority) { if (a_pPriority) *a_pPriority = 150; return S_OK; }
	STDMETHOD(Name)(ILocalizedString** a_ppName);
	STDMETHOD(Description)(ILocalizedString** a_ppDesc);
	STDMETHOD(Config)(IConfig** a_ppConfig);
};

OBJECT_ENTRY_AUTO(__uuidof(ColorWindow), CColorWindow)
