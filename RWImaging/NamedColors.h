// NamedColors.h : Declaration of the CNamedColors

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CNamedColors

class ATL_NO_VTABLE CNamedColors :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CNamedColors, &CLSID_NamedColors>,
	public INamedColors
{
public:
	CNamedColors()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CNamedColors)
	COM_INTERFACE_ENTRY(INamedColors)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// INamedColors methods
public:
	STDMETHOD(ColorToName)(DWORD a_dwRGBA, ILocalizedString** a_ppName);

};

OBJECT_ENTRY_AUTO(__uuidof(NamedColors), CNamedColors)
