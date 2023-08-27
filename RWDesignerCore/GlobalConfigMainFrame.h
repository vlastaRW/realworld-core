// GlobalConfigMainFrame.h : Declaration of the CGlobalConfigMainFrame

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"



// CGlobalConfigMainFrame

class ATL_NO_VTABLE CGlobalConfigMainFrame :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGlobalConfigMainFrame, &CLSID_GlobalConfigMainFrame>,
	public IGlobalConfigFactory
{
public:
	CGlobalConfigMainFrame()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CGlobalConfigMainFrame)

BEGIN_CATEGORY_MAP(CGlobalConfigMainFrame)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CGlobalConfigMainFrame)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
END_COM_MAP()


	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE* a_pPriority);
	STDMETHOD(Name)(ILocalizedString** a_ppName);
	STDMETHOD(Description)(ILocalizedString** a_ppDesc);
	STDMETHOD(Config)(IConfig** a_ppConfig);

	static void InitStartPages(ULONG a_nStartPages, CLSID const* a_aStartPages, CLSID const& a_tDefaultStartPage)
	{
		m_nStartPages = a_nStartPages;
		m_aStartPages = a_aStartPages;
		m_tDefaultStartPage = a_tDefaultStartPage;
	}

private:
	static ULONG m_nStartPages;
	static CLSID const* m_aStartPages;
	static CLSID m_tDefaultStartPage;
};

OBJECT_ENTRY_AUTO(__uuidof(GlobalConfigMainFrame), CGlobalConfigMainFrame)
