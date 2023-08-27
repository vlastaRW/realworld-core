// DesignerViewFactoryRolldown.h : Declaration of the CDesignerViewFactoryRolldown

#pragma once
#include "resource.h"       // main symbols
#include "RWViewAreaLayout.h"


// CDesignerViewFactoryRolldown

class ATL_NO_VTABLE CDesignerViewFactoryRolldown : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryRolldown, &CLSID_DesignerViewFactoryRolldown>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryRolldown()
	{
	}

DECLARE_NO_REGISTRY()

BEGIN_CATEGORY_MAP(CDesignerViewFactoryRolldown)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryRolldown)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryRolldown), CDesignerViewFactoryRolldown)
