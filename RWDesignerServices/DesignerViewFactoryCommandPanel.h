// DesignerViewFactoryCommandPanel.h : Declaration of the CDesignerViewFactoryCommandPanel

#pragma once
#include "resource.h"       // main symbols

#include "RWDesignerServices.h"


// E6A85651-6786-4132-A273-C7004A726CBD
extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryCommandPanel = { 0xe6a85651, 0x6786, 0x4132, { 0xa2, 0x73, 0xc7, 0x00, 0x4a, 0x72, 0x6c, 0xbd } };

// CDesignerViewFactoryCommandPanel

class ATL_NO_VTABLE CDesignerViewFactoryCommandPanel :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryCommandPanel, &CLSID_DesignerViewFactoryCommandPanel>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryCommandPanel()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryCommandPanel)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryCommandPanel)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryCommandPanel)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
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
};

OBJECT_ENTRY_AUTO(CLSID_DesignerViewFactoryCommandPanel, CDesignerViewFactoryCommandPanel)
