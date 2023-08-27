// ViewManager.h : Declaration of the CViewManager

#pragma once
#include "resource.h"       // main symbols
#include "RWConceptDesignerView.h"
#include <WeakSingleton.h>
#include <CooperatingObjectsManager.h>


// CViewManager

class ATL_NO_VTABLE CViewManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CViewManager, &CLSID_ViewManager>,
	public CCooperatingObjectsManagerImpl<IViewManager, IDesignerViewFactory, &CATID_DesignerViewFactory, &__uuidof(DesignerViewFactoryNULL)>
{
public:
	CViewManager()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CViewManager)

BEGIN_COM_MAP(CViewManager)
	COM_INTERFACE_ENTRY(IViewManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	// IViewManager methods
public:
	STDMETHOD(CreateWnd)(IViewManager* a_pOverrideForItem, TConfigValue const* a_ptItemID, IConfig* a_pItemCfg, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pOverrideForItem, TConfigValue const* a_ptItemID, IConfig* a_pItemCfg, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

};

OBJECT_ENTRY_AUTO(__uuidof(ViewManager), CViewManager)
