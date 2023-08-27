// MenuCommandsManager.h : Declaration of the CMenuCommandsManager

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"
#include <WeakSingleton.h>
#include "CooperatingObjectsManager.h"


// CMenuCommandsManager

class ATL_NO_VTABLE CMenuCommandsManager :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsManager, &CLSID_MenuCommandsManager>,
	public CCooperatingObjectsManagerImpl<IMenuCommandsManager, IDocumentMenuCommands, &CATID_DocumentMenuCommands, &__uuidof(MenuCommandsVector)>
{
public:
	CMenuCommandsManager()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsManager)


BEGIN_COM_MAP(CMenuCommandsManager)
	COM_INTERFACE_ENTRY(IMenuCommandsManager)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	// IMenuCommandsManager methods (specific only)
public:
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pOverrideForItem, const TConfigValue* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsManager), CMenuCommandsManager)
