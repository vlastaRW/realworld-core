// MenuCommandsCondition.h : Declaration of the CMenuCommandsCondition

#pragma once
#include "resource.h"       // main symbols

#include "RWViewStructure.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CMenuCommandsCondition

class ATL_NO_VTABLE CMenuCommandsCondition :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsCondition, &CLSID_MenuCommandsCondition>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsCondition()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsCondition)

BEGIN_CATEGORY_MAP(CMenuCommandsCondition)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsCondition)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	static void GetSubDoc(ISubDocumentsMgr* a_pSDM, LPWSTR a_pszIDs, IOperationContext* a_pStates, IDocument** a_ppDoc);
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsCondition), CMenuCommandsCondition)
