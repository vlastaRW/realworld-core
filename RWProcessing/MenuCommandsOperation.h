// MenuCommandsOperation.h : Declaration of the CMenuCommandsOperation

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"
#include <WeakSingleton.h>


// CMenuCommandsOperation

class ATL_NO_VTABLE CMenuCommandsOperation :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsOperation, &CLSID_MenuCommandsOperation>,
	public IDocumentMenuCommands,
	public IMenuCommandsOperation
{
public:
	CMenuCommandsOperation()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsOperation)

BEGIN_CATEGORY_MAP(CMenuCommandsOperation)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsOperation)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	COM_INTERFACE_ENTRY(IMenuCommandsOperation)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

	// IMenuCommandsOperation methods
public:
	STDMETHOD(OperationManagerSet)(IOperationManager* a_pOpsMgr)
	{
		if (a_pOpsMgr == NULL)
			return E_INVALIDARG;
		ObjectLock cLock(this);
		m_pOpsMgr = a_pOpsMgr;
		return S_OK;
	}

private:
	IOperationManager* M_OperationManager()
	{
		ObjectLock cLock(this);
		if (m_pOpsMgr)
			return m_pOpsMgr;
		RWCoCreateInstance(m_pOpsMgr, __uuidof(OperationManager));
		if (m_pOpsMgr == NULL)
			throw E_UNEXPECTED;
		return m_pOpsMgr;
	}

private:
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CDocumentMenuCommand()
		{
		}
		void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, LONG a_nShortcut, GUID const& a_tIconID, IOperationManager* a_pOpsMgr, TConfigValue const& a_cItemID, IConfig* a_pItemCfg, IOperationContext* a_pStates, IDocument* a_pDocument, IDesignerView* a_pView)
		{
			m_pName = a_pName;
			m_pDesc = a_pDesc;
			m_nShortcut = a_nShortcut;
			m_tIconID = a_tIconID;
			m_pOpsMgr = a_pOpsMgr;
			m_cItemID = a_cItemID;
			m_pItemCfg = a_pItemCfg;
			m_pStates = a_pStates;
			m_pDocument = a_pDocument;
			m_pView = a_pView;
		}

	BEGIN_COM_MAP(CDocumentMenuCommand)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel);
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands);
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<ILocalizedString> m_pName;
		CComPtr<ILocalizedString> m_pDesc;
		LONG m_nShortcut;
		GUID m_tIconID;
		CComPtr<IOperationManager> m_pOpsMgr;
		CConfigValue m_cItemID;
		CComPtr<IConfig> m_pItemCfg;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDesignerView> m_pView;
	};

private:
	CComPtr<IOperationManager> m_pOpsMgr;
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsOperation), CMenuCommandsOperation)
