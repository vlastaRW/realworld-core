
#pragma once

#include <SharedStringTable.h>

class CMainFrame;

class ATL_NO_VTABLE CDesignerFrameMenuCommandsManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IMenuCommandsManager
{
public:
	CDesignerFrameMenuCommandsManager()
	{
	}
	void Init(IMenuCommandsManager* a_pManager, IOperationManager* a_pOperMgr, CMainFrame* a_pFrame = NULL)
	{
		if (a_pManager == NULL)
			throw E_POINTER;
		m_pManager = a_pManager;
		m_pOperMgr = a_pOperMgr;
		m_pFrame = a_pFrame;
	}

	static HRESULT WINAPI QIOperMgr(void* a_pThis, REFIID a_iid, void** a_ppOut, DWORD_PTR UNREF(dw))
	{
		CDesignerFrameMenuCommandsManager* pThis = reinterpret_cast<CDesignerFrameMenuCommandsManager*>(a_pThis);
		return pThis->m_pOperMgr ? pThis->m_pOperMgr->QueryInterface(a_iid, a_ppOut) : E_NOINTERFACE;
	}

BEGIN_COM_MAP(CDesignerFrameMenuCommandsManager)
	COM_INTERFACE_ENTRY(IMenuCommandsManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IOperationManager), 0, QIOperMgr)
END_COM_MAP()


	// ILateConfigCreator methods
public:
	STDMETHOD(CreateConfig)(TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		return CreateConfigEx(this, a_ptControllerID, a_ppConfig);
	}

	// IMenuCommandsManager methods
public:
	STDMETHOD(CreateConfigEx)(IMenuCommandsManager* a_pOverrideForItem, TConfigValue const* a_ptControllerID, IConfig** a_ppConfig);
	STDMETHOD(ItemGetCount)(ULONG* a_pnCount);
	STDMETHOD(ItemIDGetDefault)(TConfigValue* a_ptDefaultOpID);
	STDMETHOD(ItemIDGet)(IMenuCommandsManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName);
	STDMETHOD(InsertIntoConfigAs)(IMenuCommandsManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, TConfigOptionCondition const* a_aItemConditions);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	CComPtr<IMenuCommandsManager> m_pManager;
	CComPtr<IOperationManager> m_pOperMgr;
	CMainFrame* m_pFrame;
};
