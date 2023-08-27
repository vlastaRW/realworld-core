// MenuCommandsStructuredItemValue.h : Declaration of the CMenuCommandsStructuredItemValue

#pragma once
#include "resource.h"       // main symbols
#include "RWViewStructure.h"


// CMenuCommandsStructuredItemValue

class ATL_NO_VTABLE CMenuCommandsStructuredItemValue :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsStructuredItemValue, &CLSID_MenuCommandsStructuredItemValue>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsStructuredItemValue()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsStructuredItemValue)

BEGIN_CATEGORY_MAP(CMenuCommandsStructuredItemValue)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsStructuredItemValue)
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
	class ATL_NO_VTABLE CBoolItem :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IDocument* a_pDoc, std::vector<CComPtr<IItemBool> >& a_cItems, boolean a_bVal)
		{
			m_pDoc = a_pDoc;
			std::swap(a_cItems, m_cItems);
			m_bVal = a_bVal;
		}

	BEGIN_COM_MAP(CBoolItem)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel) { return E_NOTIMPL; }
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IDocument> m_pDoc;
		std::vector<CComPtr<IItemBool> > m_cItems;
		boolean m_bVal;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsStructuredItemValue), CMenuCommandsStructuredItemValue)
