// MenuCommandsVector.h : Declaration of the CMenuCommandsVector

#pragma once
#include "resource.h"       // main symbols
#include "RWProcessing.h"


// CMenuCommandsVector

class ATL_NO_VTABLE CMenuCommandsVector :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsVector, &CLSID_MenuCommandsVector>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsVector()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsVector)

BEGIN_CATEGORY_MAP(CMenuCommandsVector)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsVector)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CConfigPattern :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IConfig
	{
	public:
		void Init(IMenuCommandsManager* a_pManager)
		{
			m_pManager = a_pManager;
		}

	BEGIN_COM_MAP(CConfigPattern)
		COM_INTERFACE_ENTRY(IConfig)
	END_COM_MAP()

		// IConfig methods
	public:
		STDMETHOD(ItemIDsEnum)(IEnumStrings**) {return E_NOTIMPL;}
		STDMETHOD(ItemValueGet)(BSTR, TConfigValue*) {return E_NOTIMPL;}
		STDMETHOD(ItemValuesSet)(ULONG, BSTR*, TConfigValue const*) {return E_NOTIMPL;}
		STDMETHOD(ItemGetUIInfo)(BSTR, REFIID, void**) {return E_NOTIMPL;}
		STDMETHOD(SubConfigGet)(BSTR, IConfig**) {return E_NOTIMPL;}
		STDMETHOD(DuplicateCreate)(IConfig** a_ppCopiedConfig);
		STDMETHOD(CopyFrom)(IConfig* a_pSource, BSTR a_bstrIDPrefix) { return E_NOTIMPL; }
		STDMETHOD(ObserverIns)(IConfigObserver*, TCookie) {return E_NOTIMPL;}
		STDMETHOD(ObserverDel)(IConfigObserver*, TCookie) {return E_NOTIMPL;}

	private:
		CComPtr<IConfigWithDependencies> m_pPattern;
		CComPtr<IMenuCommandsManager> m_pManager;
	};

	class ATL_NO_VTABLE CCustomNameString :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ILocalizedString
	{
	public:
		void Init(ULONG a_nIndex, IConfig* a_pItemConfig)
		{
			m_nIndex = a_nIndex;
			m_pItemConfig = a_pItemConfig;
		}

	BEGIN_COM_MAP(CCustomNameString)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

		// ILocalizedString methods
	public:
		STDMETHOD(Get)(BSTR *a_pbstrString) { return GetLocalized(GetThreadLocale(), a_pbstrString); }
		STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR *a_pbstrString);

	private:
		ULONG m_nIndex;
		CComPtr<IConfig> m_pItemConfig;
	};

	class ATL_NO_VTABLE CCustomName :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IVectorItemName
	{
	public:

	BEGIN_COM_MAP(CCustomName)
		COM_INTERFACE_ENTRY(IVectorItemName)
	END_COM_MAP()

		// IVectorItemName methods
	public:
		STDMETHOD(Get)(ULONG a_nIndex, IConfig* a_pItemConfig, ILocalizedString** a_ppName)
		{
			try
			{
				*a_ppName = NULL;
				CComObject<CCustomNameString>* p = NULL;
				CComObject<CCustomNameString>::CreateInstance(&p);
				CComPtr<ILocalizedString> pTmp = p;
				p->Init(a_nIndex, a_pItemConfig);
				*a_ppName = pTmp.Detach();
				return S_OK;
			}
			catch (...)
			{
				return a_ppName ? E_UNEXPECTED: E_POINTER;
			}
		}
	};

};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsVector), CMenuCommandsVector)
