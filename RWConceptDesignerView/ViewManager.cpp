// ViewManager.cpp : Implementation of CViewManager

#include "stdafx.h"
#include "ViewManager.h"


// CViewManager

STDMETHODIMP CViewManager::CreateWnd(IViewManager* a_pOverrideForItem, TConfigValue const* a_ptItemID, IConfig* a_pItemCfg, ISharedStateManager* a_pStates, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		CComPtr<IDesignerViewFactory> p;
		if (a_ptItemID && a_ptItemID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptItemID->guidVal);
		if (p == NULL)
		{
			RWCoCreateInstance(p, __uuidof(DesignerViewFactoryNULL));
			if (p == NULL) return E_FAIL;
		}
		return p->CreateWnd(a_pOverrideForItem ? a_pOverrideForItem : this, a_pItemCfg, a_pStates, a_pStatusBar, a_pDoc, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_ppDVWnd);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CViewManager::CheckSuitability(IViewManager* a_pOverrideForItem, TConfigValue const* a_ptItemID, IConfig* a_pItemCfg, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	try
	{
		CComPtr<IDesignerViewFactory> p;
		if (a_ptItemID && a_ptItemID->eTypeID == ECVTGUID)
			RWCoCreateInstance(p, a_ptItemID->guidVal);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		return p->CheckSuitability(a_pOverrideForItem ? a_pOverrideForItem : this, a_pItemCfg, a_pDocument, a_pCallback);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

