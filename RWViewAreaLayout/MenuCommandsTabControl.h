// MenuCommandsTabControl.h : Declaration of the CMenuCommandsTabControl

#pragma once
#include "resource.h"       // main symbols
#include "RWViewAreaLayout.h"



// CMenuCommandsTabControl

class ATL_NO_VTABLE CMenuCommandsTabControl :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsTabControl, &CLSID_MenuCommandsTabControl>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsTabControl()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsTabControl)

BEGIN_CATEGORY_MAP(CMenuCommandsTabControl)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsTabControl)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuItem :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand,
		public ILocalizedString
	{
	public:
		void Init(IDesignerViewTabControl* a_pDVTC, int a_nIndex, bool a_bTextInToolbar, bool a_bSingleItem)
		{
			m_pDVTC = a_pDVTC;
			m_nIndex = a_nIndex;
			m_bTextInToolbar = a_bTextInToolbar;
			m_bSingleItem = a_bSingleItem;
		}

	BEGIN_COM_MAP(CDocumentMenuItem)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

		// IDocumentMenuCommand methods
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel) { return E_NOTIMPL; }
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

		// ILocalizedString methods
	public:
		STDMETHOD(Get)(BSTR *a_pbstrString) { return GetLocalized(GetThreadLocale(), a_pbstrString); }
		STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR *a_pbstrString);

	private:
		CComPtr<IDesignerViewTabControl> m_pDVTC;
		int m_nIndex;
		bool m_bTextInToolbar;
		bool m_bSingleItem;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsTabControl), CMenuCommandsTabControl)
