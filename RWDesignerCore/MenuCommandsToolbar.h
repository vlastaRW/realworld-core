// MenuCommandsToolbar.h : Declaration of the CMenuCommandsToolbar

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <RWProcessing.h>
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>


// CMenuCommandsToolbar

class ATL_NO_VTABLE CMenuCommandsToolbar :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsToolbar, &CLSID_MenuCommandsToolbar>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsToolbar()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsToolbar)

BEGIN_CATEGORY_MAP(CMenuCommandsToolbar)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsToolbar)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CDocumentMenuCommandImpl<CDocumentMenuCommand, 0, IDS_MENUTOOLBAR_CMDDESC, NULL, 0>
	{
	public:
		CDocumentMenuCommand()
		{
		}
		void Init(IOperationContext* a_pStates, BSTR a_bstrID, ILocalizedString* a_pName)
		{
			m_pStates = a_pStates;
			m_bstrID = a_bstrID;
			m_pName = a_pName;
		}

	BEGIN_COM_MAP(CDocumentMenuCommand)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand methods
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrID;
		CComPtr<ILocalizedString> m_pName;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsToolbar), CMenuCommandsToolbar)
