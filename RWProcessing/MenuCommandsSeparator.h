// MenuCommandsSeparator.h : Declaration of the CMenuCommandsSeparator

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"


// CMenuCommandsSeparator

class ATL_NO_VTABLE CMenuCommandsSeparator :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsSeparator, &CLSID_MenuCommandsSeparator>,
	public IDocumentMenuCommands,
	public IDocumentMenuCommand
{
public:
	CMenuCommandsSeparator()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsSeparator)

BEGIN_CATEGORY_MAP(CMenuCommandsSeparator)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsSeparator)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

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

};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsSeparator), CMenuCommandsSeparator)
