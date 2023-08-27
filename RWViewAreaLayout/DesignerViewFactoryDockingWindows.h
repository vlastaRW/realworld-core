// DesignerViewFactoryDockingWindows.h : Declaration of the CDesignerViewFactoryDockingWindows

#pragma once
#include "resource.h"       // main symbols
#include "RWViewAreaLayout.h"


// CDesignerViewFactoryDockingWindows

class ATL_NO_VTABLE CDesignerViewFactoryDockingWindows : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryDockingWindows, &CLSID_DesignerViewFactoryDockingWindows>,
	public IDesignerViewFactory,
	public IDocumentMenuCommands
{
public:
	CDesignerViewFactoryDockingWindows()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryDockingWindows)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryDockingWindows)
#ifdef _DEBUG
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
#endif
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryDockingWindows)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryDockingWindows), CDesignerViewFactoryDockingWindows)
