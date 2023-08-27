// MenuCommandsPopup.h : Declaration of the CMenuCommandsPopup

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"


// CMenuCommandsPopup

class ATL_NO_VTABLE CMenuCommandsPopup :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsPopup, &CLSID_MenuCommandsPopup>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsPopup()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsPopup)

BEGIN_CATEGORY_MAP(CMenuCommandsPopup)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsPopup)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CDocumentMenuCommand() : m_tLastIconID(GUID_NULL)
		{
		}
		void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, GUID const& a_tIconID, IEnumUnknowns* a_pSubItems, bool a_bRadioIcon, bool a_bToolbarText)
		{
			m_pName = a_pName;
			m_pDesc = a_pDesc;
			m_tIconID = a_tIconID;
			m_pSubItems = a_pSubItems;
			m_bRadioIcon = a_bRadioIcon;
			m_bToolbarText = a_bToolbarText;
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
		GUID m_tIconID;
		GUID m_tLastIconID;
		CComPtr<IEnumUnknowns> m_pSubItems;
		bool m_bRadioIcon;
		bool m_bToolbarText;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsPopup), CMenuCommandsPopup)
