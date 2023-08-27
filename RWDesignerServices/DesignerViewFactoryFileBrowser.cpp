// DesignerViewFactoryFileBrowser.cpp : Implementation of CDesignerViewFactoryFileBrowser

#include "stdafx.h"
#include "DesignerViewFactoryFileBrowser.h"

#include "DesignerViewFileBrowser.h"
#include <MultiLanguageString.h>
#include <IconRenderer.h>


// CDesignerViewFactoryFileBrowser

STDMETHODIMP CDesignerViewFactoryFileBrowser::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]File Browser[0405]Správce souborů");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_FB_STORAGE[] = L"Storage";
static OLECHAR const CFGID_FB_COMMANDNAME[] = L"OpName";
static OLECHAR const CFGID_FB_COMMANDDESC[] = L"OpDesc";
static OLECHAR const CFGID_FB_COMMANDICON[] = L"OpIcon";
static OLECHAR const CFGID_FB_COMMAND[] = L"Operation";

#include <ConfigCustomGUIImpl.h>
#include <ConfigCustomGUIIcons.h>

class ATL_NO_VTABLE CConfigGUIFileBrowserDlg :
	public CCustomConfigWndWithIcons<CConfigGUIFileBrowserDlg, CCustomConfigResourcelessWndImpl<CConfigGUIFileBrowserDlg> >,
	public CDialogResize<CConfigGUIFileBrowserDlg>
{
	typedef CCustomConfigWndWithIcons<CConfigGUIFileBrowserDlg, CCustomConfigResourcelessWndImpl<CConfigGUIFileBrowserDlg> > TBaseClass;
public:
	enum
	{
		IDC_CFGID_FB_COMMANDNAME = 100,
		IDC_CFGID_FB_COMMANDICON_LABEL,
		IDC_CFGID_FB_COMMANDICON,
		IDC_CFGID_FB_COMMANDDESC,
		IDC_CFGID_FB_COMMAND,
		IDC_CFGID_FB_COMMAND_CTRL,
	};

	BEGIN_DIALOG_EX(0, 0, 145, 90, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]&Name:[0405]&Název:"), IDC_STATIC, 0, 2, 49, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CFGID_FB_COMMANDNAME, 50, 0, 25, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]&Icon:[0405]&Ikona:"), IDC_CFGID_FB_COMMANDICON_LABEL, 85, 2, 29, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CFGID_FB_COMMANDICON, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 116, 0, 29, 197, 0)
		CONTROL_LTEXT(_T("[0409]&Description:[0405]&Popis:"), IDC_STATIC, 0, 18, 49, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CFGID_FB_COMMANDDESC, 50, 16, 95, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]&Operation:[0405]&Operace:"), IDC_STATIC, 0, 34, 49, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CFGID_FB_COMMAND, 50, 32, 95, 190, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT, 0)
		CONTROL_CONTROL(_T(""), IDC_CFGID_FB_COMMAND_CTRL, WC_STATIC, SS_GRAYRECT | WS_VISIBLE, 0, 48, 145, 42, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIFileBrowserDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIFileBrowserDlg>)
		CHAIN_MSG_MAP(TBaseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIFileBrowserDlg)
		DLGRESIZE_CONTROL(IDC_CFGID_FB_COMMANDNAME, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CFGID_FB_COMMANDICON_LABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CFGID_FB_COMMANDICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CFGID_FB_COMMANDDESC, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CFGID_FB_COMMAND, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CFGID_FB_COMMAND_CTRL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIFileBrowserDlg)
		CONFIGITEM_EDITBOX(IDC_CFGID_FB_COMMANDNAME, CFGID_FB_COMMANDNAME)
		CONFIGITEM_EDITBOX(IDC_CFGID_FB_COMMANDDESC, CFGID_FB_COMMANDDESC)
		CONFIGITEM_COMBOBOX(IDC_CFGID_FB_COMMAND, CFGID_FB_COMMAND)
		CONFIGITEM_ICONCOMBO(IDC_CFGID_FB_COMMANDICON, CFGID_FB_COMMANDICON)
		CONFIGITEM_SUBCONFIG(IDC_CFGID_FB_COMMAND_CTRL, CFGID_FB_COMMAND)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

STDMETHODIMP CDesignerViewFactoryFileBrowser::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		CComPtr<IStorageManager> pSM;
		RWCoCreateInstance(pSM, __uuidof(StorageManager));
		CComPtr<IConfig> pStCfg;
		pSM->ConfigGetDefault(&pStCfg);
		pCfg->ItemInsSimple(CComBSTR(CFGID_FB_STORAGE), NULL, NULL, CConfigValue(false), CComQIPtr<ISubConfig>(pStCfg), 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_FB_COMMANDNAME), CMultiLanguageString::GetAuto(L"[0409]Name[0405]Jméno"), CMultiLanguageString::GetAuto(L"[0409]Name of the default operation.[0405]Jméno výchozí operace."), CConfigValue(L""), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_FB_COMMANDDESC), CMultiLanguageString::GetAuto(L"[0409]Description[0405]Popis"), CMultiLanguageString::GetAuto(L"[0409]Description of the default operation.[0405]Popis výchozí operace."), CConfigValue(L""), NULL, 0, NULL);

		// icon
		CComBSTR cCFGID_ICONID(CFGID_FB_COMMANDICON);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pCfg->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon[0405]Ikona"), CMultiLanguageString::GetAuto(L"[0409]Icon of the default operation.[0405]Ikona výchozí operace."), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pCfg->ItemInsSimple(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon[0405]Ikona"), CMultiLanguageString::GetAuto(L"[0409]Icon of the default operation.[0405]Ikona výchozí operace."), CConfigValue(GUID_NULL), NULL, 0, NULL);

		CComPtr<IOperationManager> pMgr;
		a_pManager->QueryInterface(&pMgr);
		if (pMgr == NULL)
			RWCoCreateInstance(pMgr, __uuidof(OperationManager));

		pMgr->InsertIntoConfigAs(pMgr, pCfg, CComBSTR(CFGID_FB_COMMAND), CMultiLanguageString::GetAuto(L"[0409]Default operation[0405]Výchozí operace"), CMultiLanguageString::GetAuto(L"[0409]Operation performed when file is double-clicked.[0405]Operacy vykonaná při poklepání na soubor."), 0, NULL);

		CConfigCustomGUI<&CLSID_DesignerViewFactoryFileBrowser, CConfigGUIFileBrowserDlg>::FinalizeConfig(pCfg);
		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryFileBrowser::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewFileBrowser>* pWnd = NULL;
		CComObject<CDesignerViewFileBrowser>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		CComPtr<IConfig> pStorageCfg;
		a_pConfig->SubConfigGet(CComBSTR(CFGID_FB_STORAGE), &pStorageCfg);
		CConfigValue cName;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FB_COMMANDNAME), &cName);
		CConfigValue cDesc;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FB_COMMANDDESC), &cDesc);
		CConfigValue cIcon;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FB_COMMANDICON), &cIcon);
		CConfigValue cOpID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FB_COMMAND), &cOpID);
		CComPtr<IConfig> pOpCfg;
		a_pConfig->SubConfigGet(CComBSTR(CFGID_FB_COMMAND), &pOpCfg);

		CComPtr<IOperationManager> pMgr;
		a_pManager->QueryInterface(&pMgr);
		if (pMgr == NULL)
			RWCoCreateInstance(pMgr, __uuidof(OperationManager));

		if (FAILED(pWnd->Init(a_pDoc, pStorageCfg, cName, cDesc, cIcon, cOpID, pOpCfg, pMgr, a_pFrame, a_hParent, a_tLocaleID, const_cast<RECT*>(a_prcWindow))))
			return E_FAIL;

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryFileBrowser::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* UNREF(a_pDocument), ICheckSuitabilityCallback* UNREF(a_pCallback))
{
	return S_OK; // this view does not affect anything
}

STDMETHODIMP CDesignerViewFactoryFileBrowser::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]File Browser[0405]Správce souborů");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_BROWSERCMDTYPE[] = L"BrowseCmds";

STDMETHODIMP CDesignerViewFactoryFileBrowser::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		CComBSTR bstrType(CFGID_BROWSERCMDTYPE);
		pCfg->ItemIns1ofN(bstrType, CMultiLanguageString::GetAuto(L"[0409]Command type[0405]Typ příkazu"), NULL, CConfigValue(0L), NULL);
		pCfg->ItemOptionAdd(bstrType, CConfigValue(0L), CMultiLanguageString::GetAuto(L"[0409]Storage picker[0405]Výběr úložiště"), 0, NULL);
		pCfg->ItemOptionAdd(bstrType, CConfigValue(1L), CMultiLanguageString::GetAuto(L"[0409]File type[0405]Typ souboru"), 0, NULL);
		pCfg->ItemOptionAdd(bstrType, CConfigValue(2L), CMultiLanguageString::GetAuto(L"[0409]Navigation[0405]Navigace"), 0, NULL);
		pCfg->Finalize(NULL);
		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CMenuCommandSwitchStorage :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	void Init(REFCLSID a_tClsID, IStorageBrowserWindow* a_pSBW)
	{
		m_tClsID = a_tClsID;
		m_pSBW = a_pSBW;
	}

BEGIN_COM_MAP(CMenuCommandSwitchStorage)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		return m_pSBW->StoragesName(m_tClsID, a_ppText);
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			return E_NOTIMPL;
			//(*a_ppText = this)->AddRef();
			//return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		*a_pIconID = m_tClsID;
		return S_OK;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		return m_pSBW->StoragesIcon(m_tClsID, a_nSize, a_phIcon);
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel) { return E_NOTIMPL; }
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			CLSID tID = GUID_NULL;
			m_pSBW->ActiveStorageGet(&tID);
			*a_peState = IsEqualGUID(m_tClsID, tID) ? EMCSRadioChecked : EMCSRadio;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		return m_pSBW->ActiveStorageSet(m_tClsID);
	}

private:
	CLSID m_tClsID;
	CComPtr<IStorageBrowserWindow> m_pSBW;
};

class ATL_NO_VTABLE CMenuCommandSetDocType :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	void Init(IDocumentType* a_pType, IStorageFilterWindow* a_pSBW, GUID const& a_tIconID)
	{
		m_pType = a_pType;
		m_pSBW = a_pSBW;
		m_tIconID = a_tIconID;
	}

BEGIN_COM_MAP(CMenuCommandSetDocType)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		return m_pType->FilterNameGet(a_ppText);
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			return E_NOTIMPL;
			//(*a_ppText = this)->AddRef();
			//return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		*a_pIconID = m_tIconID;
		return S_OK;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		HRESULT hRes = m_pType->IconGet(NULL, a_nSize, a_phIcon);
		if (*a_phIcon == NULL)
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			static IRPathPoint const shape[] =
			{
				{166, 12, 0, 0, 0, -6.62742},
				{166, 62, 0, 0, 0, 0},
				{209, 37, 5.73952, -3.31369, 0, 0},
				{226, 41, 0, 0, -3.31369, -5.73952},
				{252, 87, 3.31369, 5.73952, 0, 0},
				{247, 103, 0, 0, 5.73952, -3.31369},
				{204, 128, 0, 0, 0, 0},
				{247, 153, 5.7395, 3.31374, 0, 0},
				{252, 169, 0, 0, 3.31374, -5.7395},
				{226, 215, -3.31374, 5.7395, 0, 0},
				{209, 219, 0, 0, 5.7395, 3.31374},
				{166, 194, 0, 0, 0, 0},
				{166, 244, 0, 6.62742, 0, 0},
				{154, 256, 0, 0, 6.62742, 0},
				{102, 256, -6.62742, 0, 0, 0},
				{90, 244, 0, 0, 0, 6.62742},
				{90, 194, 0, 0, 0, 0},
				{47, 219, -5.73952, 3.31369, 0, 0},
				{30, 215, 0, 0, 3.31369, 5.73952},
				{4, 169, -3.31369, -5.73952, 0, 0},
				{9, 153, 0, 0, -5.73952, 3.31369},
				{52, 128, 0, 0, 0, 0},
				{9, 103, -5.7395, -3.31374, 0, 0},
				{4, 87, 0, 0, -3.31374, 5.7395},
				{30, 41, 3.31374, -5.7395, 0, 0},
				{47, 37, 0, 0, -5.7395, -3.31374},
				{90, 62, 0, 0, 0, 0},
				{90, 12, 0, -6.62742, 0, 0},
				{102, 0, 0, 0, -6.62742, 0},
				{154, 0, 6.62742, 0, 0, 0},
			};
			static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
			cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMInterior), IRTarget(0.75f));
			*a_phIcon = cRenderer.get();
			hRes = S_OK;
		}
		return hRes;
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel) { return E_NOTIMPL; }
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			CComPtr<IDocumentType> pType;
			m_pSBW->DocTypeGet(&pType);
			*a_peState = pType == m_pType ? EMCSRadioChecked : EMCSRadio;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		return m_pSBW->DocTypeSet(m_pType);
	}

private:
	CComPtr<IDocumentType> m_pType;
	CComPtr<IStorageFilterWindow> m_pSBW;
	GUID m_tIconID;
};

STDMETHODIMP CDesignerViewFactoryFileBrowser::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* a_pConfig, IOperationContext* a_pContext, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BROWSERCMDTYPE), &cType);
		switch (cType.operator LONG())
		{
		case 0L:
			{
				CComPtr<IEnumUnknownsInit> pIFs;
				RWCoCreateInstance(pIFs, __uuidof(EnumUnknowns));
				a_pView->QueryInterfaces(__uuidof(IStorageBrowserWindow), EQIFActive, pIFs);
				CComPtr<IStorageBrowserWindow> pWnd;
				pIFs->Get(0, __uuidof(IStorageBrowserWindow), reinterpret_cast<void**>(&pWnd));
				if (pWnd == NULL)
					return E_FAIL;
				CComPtr<IEnumGUIDs> pIDs;
				pWnd->StoragesEnum(&pIDs);
				ULONG nIDs = 0;
				pIDs->Size(&nIDs);
				for (ULONG i = 0; i < nIDs; ++i)
				{
					CLSID tID = GUID_NULL;
					pIDs->Get(i, &tID);
					CComObject<CMenuCommandSwitchStorage>* p = NULL;
					CComObject<CMenuCommandSwitchStorage>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(tID, pWnd);
					pIFs->Insert(pTmp);
				}
				*a_ppSubCommands = pIFs.Detach();
				return S_OK;
			}
		case 1L:
			{
				CComPtr<IEnumUnknownsInit> pIFs;
				RWCoCreateInstance(pIFs, __uuidof(EnumUnknowns));
				a_pView->QueryInterfaces(__uuidof(IStorageFilterWindow), EQIFActive, pIFs);
				CComPtr<IStorageFilterWindow> pWnd;
				pIFs->Get(0, __uuidof(IStorageFilterWindow), reinterpret_cast<void**>(&pWnd));
				if (pWnd == NULL)
					return E_FAIL;
				pIFs = NULL;
				RWCoCreateInstance(pIFs, __uuidof(EnumUnknowns));
				CComPtr<IEnumUnknowns> pTypes;
				pWnd->DocTypesEnum(&pTypes);
				ULONG nTypes = 0;
				if (pTypes) pTypes->Size(&nTypes);
				for (ULONG i = 0; i < nTypes; ++i)
				{
					CComPtr<IDocumentType> pType;
					pTypes->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pType));
					if (pType == NULL)
						continue;
					CComObject<CMenuCommandSetDocType>* p = NULL;
					CComObject<CMenuCommandSetDocType>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pCmd = p;
					CComBSTR bstrID;
					pType->UniqueIDGet(&bstrID);
					p->Init(pType, pWnd, GetIconID(bstrID));
					pIFs->Insert(pCmd);
				}
				*a_ppSubCommands = pIFs.Detach();
				return S_OK;
			}
		case 2L:
			{
				CComPtr<IEnumUnknownsInit> pIFs;
				RWCoCreateInstance(pIFs, __uuidof(EnumUnknowns));
				a_pView->QueryInterfaces(__uuidof(IStorageFilterWindow), EQIFActive, pIFs);
				CComPtr<IStorageFilterWindow> pWnd;
				pIFs->Get(0, __uuidof(IStorageFilterWindow), reinterpret_cast<void**>(&pWnd));
				if (pWnd == NULL)
					return E_FAIL;
				return pWnd->NavigationCommands(a_ppSubCommands);
			}
		}
		return E_FAIL;
	}
	catch (...)
	{
		return a_ppSubCommands == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

