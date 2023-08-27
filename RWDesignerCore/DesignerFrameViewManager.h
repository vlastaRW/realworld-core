
#pragma once

#include <SharedStringTable.h>


class ATL_NO_VTABLE CDesignerFrameViewManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IViewManager
{
public:
	CDesignerFrameViewManager()
	{
	}
	void Init(IViewManager* a_pManager, IMenuCommandsManager* a_pMenuCmds)
	{
		if (a_pManager == NULL)
			throw E_POINTER;
		m_pManager = a_pManager;
		m_pMenuCmds = a_pMenuCmds;
	}

	static HRESULT WINAPI QIMenuCmds(void* a_pThis, REFIID a_iid, void** a_ppOut, DWORD_PTR UNREF(dw))
	{
		CDesignerFrameViewManager* pThis = reinterpret_cast<CDesignerFrameViewManager*>(a_pThis);
		return pThis->m_pMenuCmds ? pThis->m_pMenuCmds->QueryInterface(a_iid, a_ppOut) : E_NOINTERFACE;
	}

BEGIN_COM_MAP(CDesignerFrameViewManager)
	COM_INTERFACE_ENTRY(IViewManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IMenuCommandsManager), 0, QIMenuCmds)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// ILateConfigCreator methods
public:
	STDMETHOD(CreateConfig)(TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		return m_pManager->CreateConfig(a_ptControllerID, a_ppConfig);
	}

	// IViewManager methods
public:
	STDMETHOD(ItemGetCount)(ULONG* a_pnCount)
	{
		return m_pManager->ItemGetCount(a_pnCount);
	}
	STDMETHOD(ItemIDGetDefault)(TConfigValue* a_ptDefaultOpID)
	{
		return m_pManager->ItemIDGetDefault(a_ptDefaultOpID);
	}
	STDMETHOD(ItemIDGet)(IViewManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
	{
		return m_pManager->ItemIDGet(a_pOverrideForItem ? a_pOverrideForItem : this, a_nIndex-1, a_ptOperationID, a_ppName);
	}
	STDMETHOD(InsertIntoConfigAs)(IViewManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, TConfigOptionCondition const* a_aItemConditions)
	{
		return m_pManager->InsertIntoConfigAs(a_pOverrideForItem, a_pConfig, a_bstrID, a_pItemName, a_pItemDesc, a_nItemConditions, a_aItemConditions);
	}
	STDMETHOD(CreateWnd)(IViewManager* a_pOverrideForItem, TConfigValue const* a_ptItemID, IConfig* a_pItemCfg, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
	{
		return m_pManager->CreateWnd(a_pOverrideForItem, a_ptItemID, a_pItemCfg, a_pFrame, a_pStatusBar, a_pDoc, a_hParent, a_rcWindow, a_nStyle, a_tLocaleID, a_ppDVWnd);
	}
	STDMETHOD(CheckSuitability)(IViewManager* a_pOverrideForItem, TConfigValue const* a_ptItemID, IConfig* a_pItemCfg, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
	{
		return m_pManager->CheckSuitability(a_pOverrideForItem, a_ptItemID, a_pItemCfg, a_pDocument, a_pCallback);
	}

private:
	CComPtr<IViewManager> m_pManager;
	CComPtr<IMenuCommandsManager> m_pMenuCmds;
};
