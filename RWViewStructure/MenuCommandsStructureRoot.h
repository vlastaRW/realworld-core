// MenuCommandsStructureRoot.h : Declaration of the CMenuCommandsStructureRoot

#pragma once
#include "resource.h"       // main symbols
#include "RWViewStructure.h"


// CMenuCommandsStructureRoot

class ATL_NO_VTABLE CMenuCommandsStructureRoot :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsStructureRoot, &CLSID_MenuCommandsStructureRoot>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsStructureRoot()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsStructureRoot)

BEGIN_CATEGORY_MAP(CMenuCommandsStructureRoot)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsStructureRoot)
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
	class ATL_NO_VTABLE CDocumentMenuRootID :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IDesignerViewStructure* a_pView, IStructuredDocument* a_pDoc, IID a_iid)
		{
			m_pView = a_pView;
			m_pDoc = a_pDoc;
			m_iid = a_iid;
		}

	BEGIN_COM_MAP(CDocumentMenuRootID)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText) { return E_NOTIMPL; }
		STDMETHOD(IconID)(GUID* a_pIconID) { return E_NOTIMPL; }
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon) { return E_NOTIMPL; }
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel) { return E_NOTIMPL; }
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IDesignerViewStructure> m_pView;
		CComPtr<IStructuredDocument> m_pDoc;
		IID m_iid;
	};

};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsStructureRoot), CMenuCommandsStructureRoot)
