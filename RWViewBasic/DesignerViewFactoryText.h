// DesignerViewFactoryText.h : Declaration of the CDesignerViewFactoryText

#pragma once
#include "resource.h"       // main symbols
#include "RWViewBasic.h"


// CDesignerViewFactoryText

class ATL_NO_VTABLE CDesignerViewFactoryText : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryText, &CLSID_DesignerViewFactoryText>,
	public IDesignerViewFactory,
	public IScintillaFactory
{
public:
	CDesignerViewFactoryText() : m_nScintillaRefs(0), m_bValid(false)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryText)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryText)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryText)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
	COM_INTERFACE_ENTRY(IScintillaFactory)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

	// IScintillaFactory methods
public:
	STDMETHOD(RegisterClasses)(); // is count-refed
	STDMETHOD(UnregisterClasses)();
	//STDMETHOD(CreateWindow)(RWHWND a_hParent, RECT const* a_prc, RWHWND* a_phWnd);

private:
	LONG m_nScintillaRefs;
	bool m_bValid;
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryText), CDesignerViewFactoryText)
