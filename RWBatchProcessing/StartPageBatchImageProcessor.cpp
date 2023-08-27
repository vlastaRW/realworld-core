// StartPageBatchImageProcessor.cpp : Implementation of CStartPageBatchImageProcessor

#include "stdafx.h"
#include "StartPageBatchImageProcessor.h"
#include <XPGUI.h>
#include <DPIUtils.h>
#include <MultiLanguageString.h>
#include "BatchOperationContext.h"
#include "../RWViewStructure/RWViewStructure.h"
#include <math.h>


// CStartPageBatchImageProcessor

#include <ConfigCustomGUIIcons.h>
#include <ConfigCustomGUIML.h>

class ATL_NO_VTABLE CConfigGUICustomBatchDlg :
	public CCustomConfigWndMultiLang<CConfigGUICustomBatchDlg, CCustomConfigWndWithIcons<CConfigGUICustomBatchDlg, CCustomConfigResourcelessWndImpl<CConfigGUICustomBatchDlg> > >,
	public CDialogResize<CConfigGUICustomBatchDlg>
{
public:
	CConfigGUICustomBatchDlg() :
		CCustomConfigWndMultiLang<CConfigGUICustomBatchDlg, CCustomConfigWndWithIcons<CConfigGUICustomBatchDlg, CCustomConfigResourcelessWndImpl<CConfigGUICustomBatchDlg> > >(CFGID_NAME, CFGID_DESCRIPTION)
	{
	}

	enum
	{
		IDC_CG_NAMELABEL = 100,
		IDC_CG_NAME,
		IDC_CG_ICONLABEL,
		IDC_CG_ICON,
		IDC_CG_DESCRIPTIONLABEL,
		IDC_CG_DESCRIPTION,
		IDC_CG_BUILDERLABEL,
		IDC_CG_BUILDER,
		IDC_CG_OUTPUTLABEL,
		IDC_CG_OUTPUT,
		IDC_CG_FILTERLABEL,
		IDC_CG_FILTER,
		IDC_CG_OPERATIONLABEL,
		IDC_CG_OPERATION,
		IDC_CG_OPCONFIG,
		IDC_CG_TAB,
	};

	BEGIN_DIALOG_EX(0, 0, 314, 207, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Name:[0405]Jméno:"), IDC_CG_NAMELABEL, 7, 18, 53, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_NAME, 62, 16, 170, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Icon:[0405]Ikona:"), IDC_CG_ICONLABEL, 247, 18, 24, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CG_ICON, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 278, 16, 29, 200, 0)
		CONTROL_LTEXT(_T("[0409]Description:[0405]Popis:"), IDC_CG_DESCRIPTIONLABEL, 7, 34, 53, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_DESCRIPTION, 62, 32, 244, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Open as:[0405]Otevřít jako:"), IDC_CG_BUILDERLABEL, 7, 50, 53, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CG_BUILDER, 62, 48, 244, 146, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Custom filter:[0405]Vlastní filtr:"), IDC_CG_FILTERLABEL, 7, 66, 53, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_FILTER, 62, 64, 244, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Output folder:[0405]Cílová složka:"), IDC_CG_OUTPUTLABEL, 7, 82, 53, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_OUTPUT, 62, 80, 244, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]&Operation:[0405]&Operace:"), IDC_CG_OPERATIONLABEL, 7, 18, 53, 8, 0, 0)
		CONTROL_COMBOBOX(IDC_CG_OPERATION, 62, 16, 245, 160, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP, 0)
		CONTROL_CONTROL(_T(""), IDC_CG_OPCONFIG, WC_STATIC, SS_GRAYRECT, 7, 32, 300, 168, 0)
	END_CONTROLS_MAP()

	typedef CCustomConfigWndMultiLang<CConfigGUICustomBatchDlg, CCustomConfigWndWithIcons<CConfigGUICustomBatchDlg, CCustomConfigResourcelessWndImpl<CConfigGUICustomBatchDlg> > > CCCWWIbase;
	BEGIN_MSG_MAP(CConfigGUICustomBatchDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUICustomBatchDlg>)
		CHAIN_MSG_MAP(CCCWWIbase)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CG_TAB, CTCN_SELCHANGE, OnTabChange)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUICustomBatchDlg)
		DLGRESIZE_CONTROL(IDC_CG_TAB, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_NAME, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_DESCRIPTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICONLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CG_FILTER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_BUILDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_OUTPUT, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUICustomBatchDlg)
		CONFIGITEM_EDITBOX(IDC_CG_NAME, CFGID_NAME)
		CONFIGITEM_EDITBOX(IDC_CG_DESCRIPTION, CFGID_DESCRIPTION)
		CONFIGITEM_ICONCOMBO(IDC_CG_ICON, CFGID_ICONID)
		CONFIGITEM_COMBOBOX(IDC_CG_OPERATION, CFGID_OPERATION)
		CONFIGITEM_SUBCONFIG_NOMARGINS(IDC_CG_OPCONFIG, CFGID_OPERATION)
		CONFIGITEM_COMBOBOX(IDC_CG_BUILDER, CFGID_FACTORY)
		CONFIGITEM_EDITBOX(IDC_CG_OUTPUT, CFGID_OUTPUT)
		CONFIGITEM_EDITBOX(IDC_CG_FILTER, CFGID_CUSTOMFILTER)
	END_CONFIGITEM_MAP()

	void SplitArea(RECT const& a_rcClient, RECT* a_prcTab, RECT* a_prcWindow, CWindow& a_wndTab)
	{
		RECT rc = {0, 0, 12, 12};
		MapDialogRect(&rc);
		int nNewTabAreaHeight = rc.bottom;

		a_prcTab->left = a_prcWindow->left = a_rcClient.left;
		a_prcTab->right = a_prcWindow->right = a_rcClient.right;

		a_prcTab->top = a_rcClient.top;
		a_prcWindow->top = a_prcTab->bottom = a_rcClient.top+nNewTabAreaHeight;
		a_prcWindow->bottom = max(a_prcWindow->top, a_rcClient.bottom);
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_wndTab.Create(m_hWnd, rcDefault, _T("Tab"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE, 0, IDC_CG_TAB);
		RECT rcTab;
		RECT rcWnd;
		{
			RECT rcClient;
			GetClientRect(&rcClient);
			SplitArea(rcClient, &rcTab, &rcWnd, m_wndTab);
		}
		m_wndTab.MoveWindow(&rcTab);

		CEdit out(GetDlgItem(IDC_CG_OUTPUT));
		CComBSTR bstr;
		CMultiLanguageString::GetLocalized(L"[0409]Leave empty for default output folder.[0405]Nechte prázdné pro výchozí výstupní složku.", m_tLocaleID, &bstr);
		out.SetCueBannerText(bstr, TRUE);

		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Appearance[0405]Vzhled", m_tLocaleID, &bstr);
		m_wndTab.InsertItem(0, bstr);
		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Execution[0405]Provádění", m_tLocaleID, &bstr);
		m_wndTab.InsertItem(1, bstr);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnTabChange(int UNREF(a_idCtrl), LPNMHDR a_pnmh, BOOL& UNREF(a_bHandled))
	{
		BOOL b1 = m_wndTab.GetCurSel() == 0;
		BOOL b2 = m_wndTab.GetCurSel() == 1;
		GetDlgItem(IDC_CG_NAME).ShowWindow(b1);
		GetDlgItem(IDC_CG_NAMELABEL).ShowWindow(b1);
		GetDlgItem(IDC_CG_DESCRIPTION).ShowWindow(b1);
		GetDlgItem(IDC_CG_DESCRIPTIONLABEL).ShowWindow(b1);
		GetDlgItem(IDC_CG_ICON).ShowWindow(b1);
		GetDlgItem(IDC_CG_ICONLABEL).ShowWindow(b1);
		GetDlgItem(IDC_CG_BUILDER).ShowWindow(b1);
		GetDlgItem(IDC_CG_BUILDERLABEL).ShowWindow(b1);
		GetDlgItem(IDC_CG_FILTER).ShowWindow(b1);
		GetDlgItem(IDC_CG_FILTERLABEL).ShowWindow(b1);
		GetDlgItem(IDC_CG_OUTPUT).ShowWindow(b1);
		GetDlgItem(IDC_CG_OUTPUTLABEL).ShowWindow(b1);
		GetDlgItem(IDC_CG_OPERATION).ShowWindow(b2);
		GetDlgItem(IDC_CG_OPERATIONLABEL).ShowWindow(b2);
		GetDlgItem(IDC_CG_OPCONFIG).ShowWindow(b2);

		return 0;
	}

private:
	CDotNetTabCtrl<CCustomTabItem> m_wndTab;
};

#include <PlugInCache.h>

class ATL_NO_VTABLE CDocumentFactoriesOptions : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigItemCustomOptions,
	public IEnumConfigItemOptions
{
public:

BEGIN_COM_MAP(CDocumentFactoriesOptions)
	COM_INTERFACE_ENTRY(IConfigItemCustomOptions)
	COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
END_COM_MAP()

	// IConfigItemCustomOptions methods
public:
	STDMETHOD(GetValueName)(const TConfigValue *a_pValue, ILocalizedString **a_ppName)
	{
		if (a_pValue == NULL || a_ppName == NULL) return E_POINTER;
		if (a_pValue->eTypeID != ECVTGUID) return E_RW_ITEMNOTFOUND;
		if (IsEqualGUID(GUID_NULL, a_pValue->guidVal))
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Not set[0405]Nenastaveno");
			return S_OK;
		}
		ObjectLock lock(this);
		Cache::map_type const& m = cache.Map();
		Cache::map_type::const_iterator i = m.find(a_pValue->guidVal);
		if (i == m.end()) return E_RW_ITEMNOTFOUND;
		return i->second->TypeName(a_ppName);
	}

	// IEnumConfigItemOptions methods
public:
	STDMETHOD(Size)(ULONG *a_pnSize)
	{
		if (a_pnSize == NULL) return E_POINTER;
		ObjectLock lock(this);
		Cache::map_type const& m = cache.Map();
		*a_pnSize = m.size()+1;
		return S_OK;
	}
	STDMETHOD(Get)(ULONG a_nIndex, TConfigValue *a_ptItem)
	{
		if (a_ptItem == NULL) return E_POINTER;
		if (a_nIndex == 0)
		{
			a_ptItem->eTypeID = ECVTGUID;
			a_ptItem->guidVal = GUID_NULL;
			return S_OK;
		}
		--a_nIndex;
		ObjectLock lock(this);
		Cache::map_type const& m = cache.Map();
		if (a_nIndex >= m.size()) return E_RW_INDEXOUTOFRANGE;
		a_ptItem->eTypeID = ECVTGUID;
		Cache::map_type::const_iterator i = m.begin();
		std::advance(i, a_nIndex);
		a_ptItem->guidVal = i->first;
		return S_OK;
	}
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue *a_atItems)
	{
		if (a_atItems == NULL) return E_POINTER;
		if (a_nIndexFirst == 0)
		{
			if (a_nCount == 0) return E_RW_INDEXOUTOFRANGE;
			a_atItems->eTypeID = ECVTGUID;
			a_atItems->guidVal = GUID_NULL;
			--a_nCount;
			++a_atItems;
		}
		else
			--a_nIndexFirst;
		ObjectLock lock(this);
		Cache::map_type const& m = cache.Map();
		if (a_nIndexFirst >= m.size() || a_nIndexFirst+a_nCount > m.size()) return E_RW_INDEXOUTOFRANGE;
		Cache::map_type::const_iterator i = m.begin();
		std::advance(i, a_nIndexFirst);
		while (a_nCount)
		{
			a_atItems->eTypeID = ECVTGUID;
			a_atItems->guidVal = i->first;
			++a_atItems;
			++i;
			--a_nCount;
		}
		return S_OK;
	}

private:
	typedef CPlugInCache<&CATID_DocumentBuilder, IDocumentBuilder> Cache;

private:
	Cache cache;
};

HRESULT CStartPageBatchImageProcessor::Init1OpConfig(IOperationManager* a_pOpMgr, IConfig** a_pOut)
{
	CComPtr<IConfigWithDependencies> p1Op;
	RWCoCreateInstance(p1Op, __uuidof(ConfigWithDependencies));
	p1Op->ItemInsSimple(CComBSTR(CFGID_NAME), CMultiLanguageString::GetAuto(L"[0409]Name[0405]Jméno"), CMultiLanguageString::GetAuto(L"[0409]Name of this batch operation. Displayed in the Operations box. Can use multilanguage format.[0405]Jméno této dávkové operace. Jméno je zobrazeno v poli Operace a může využívat vícejazyčný formát."), CConfigValue(L"[0409]New operation[0405]Nová operace"), NULL, 0, NULL);
	p1Op->ItemInsSimple(CComBSTR(CFGID_DESCRIPTION), CMultiLanguageString::GetAuto(L"[0409]Description[0405]Popis"), CMultiLanguageString::GetAuto(L"[0409]A short description of this operation displayed in status bar or tooltip.[0405]Krátký popis této operace zobrazovaný ve stavovém řádku."), CConfigValue(L""), NULL, 0, NULL);
	p1Op->ItemInsSimple(CComBSTR(CFGID_PREVNAME), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);

	// icon
	CComBSTR cCFGID_ICONID(CFGID_ICONID);
	CComPtr<IConfigItemCustomOptions> pCustIconIDs;
	RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
	if (pCustIconIDs != NULL)
		p1Op->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon ID[0405]ID ikony"), CMultiLanguageString::GetAuto(L"[0409]Icon displayed in the Operations box.[0405]Ikona zobazená v poli Operace."), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
	else
		p1Op->ItemInsSimple(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon ID[0405]ID ikony"), CMultiLanguageString::GetAuto(L"[0409]Icon displayed in the Operations box.[0405]Ikona zobazená v poli Operace."), CConfigValue(GUID_NULL), NULL, 0, NULL);

	// document factory
	CComBSTR cCFGID_FACTORY(CFGID_FACTORY);
	CComPtr<IConfigItemCustomOptions> pCustBuilders;
	{
		CComObject<CDocumentFactoriesOptions>* p = NULL;
		CComObject<CDocumentFactoriesOptions>::CreateInstance(&p);
		pCustBuilders = p;
	}
	p1Op->ItemIns1ofNWithCustomOptions(cCFGID_FACTORY, CMultiLanguageString::GetAuto(L"[0409]Document type[0405]Typ dokumentu"), CMultiLanguageString::GetAuto(L"[0409]Limits the types of accepted files for this batch operation.[0405]Omezuje typ přijímaných souborů pro tuto dávkovou operaci."), CConfigValue(GUID_NULL), pCustBuilders, NULL, 0, NULL);
	p1Op->ItemInsSimple(CComBSTR(CFGID_CUSTOMFILTER), CMultiLanguageString::GetAuto(L"[0409]Custom filter[0405]Vlastní filtr"), CMultiLanguageString::GetAuto(L"[0409]Filter for allowed files using wildcards format (for example: *.jpg;*.jpeg). If left empty, filter of the selected document type applies.[0405]Filtr pro povolené soubory ve formátu wildcards (např. *.jpg;*.jpeg). Není-li zadáno, použije se filtr vybraného typu dokumentu."), CConfigValue(L""), NULL, 0, NULL);

	p1Op->ItemInsSimple(CComBSTR(CFGID_OUTPUT), CMultiLanguageString::GetAuto(L"[0409]Output[0405]Výstup"), CMultiLanguageString::GetAuto(L"[0409]Output[0405]Výstup"), CConfigValue(L""), NULL, 0, NULL);

	CComBSTR cCFGID_OPERATION(CFGID_OPERATION);
	a_pOpMgr->InsertIntoConfigAs(a_pOpMgr, p1Op, cCFGID_OPERATION, CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]The root operation - it is recommended to use \"Sequence\" here and place \"Batch - Save to Destination Folder\" as the last operation of the sequance.[0405]Kořenová operace - je doporučeno zvolit zde operaci \"Sekvence\" a umístit operaci \"Dávka - uložit do cílové složky\" jako poslední operaci v sekvenci."), 0, NULL);
	CConfigCustomGUI<&CLSID_StartPageBatchImageProcessor, CConfigGUICustomBatchDlg, false>::FinalizeConfig(p1Op);

	// modify the pattern to contain sequence with save to destination folder as the last operation
	// HACK: internal knowledge used to set proper defaults
	{
		BSTR aIDs[3];
		TConfigValue aVals[sizeof(aIDs)/sizeof(*aIDs)];
		aIDs[0] = cCFGID_OPERATION;
		aVals[0] = CConfigValue(__uuidof(DocumentOperationSequence));
		CComBSTR bstrSteps(L"Operation\\SeqSteps");
		aIDs[1] = bstrSteps;
		aVals[1] = CConfigValue(2L);
		CComBSTR bstr2ndOp(L"Operation\\SeqSteps\\00000001\\SeqOperation");
		aIDs[2] = bstr2ndOp;
		aVals[2] = CConfigValue(BatchOperationManagerID);
		p1Op->ItemValuesSet(sizeof(aIDs)/sizeof(*aIDs), aIDs, aVals);
	}

	*a_pOut = p1Op.Detach();
	return S_OK;
}

HRESULT CStartPageBatchImageProcessor::InitConfig(IConfigWithDependencies* a_pConfig, IOperationManager* a_pOpMgr)
{
	CConfigValue cTrue(true);
	CComPtr<ILocalizedStringInit> pDummy;
	RWCoCreateInstance(pDummy, __uuidof(LocalizedString));

	CComPtr<IConfig> p1Op;
	Init1OpConfig(a_pOpMgr, &p1Op);

	CComPtr<ISubConfigVector> pVec;
	RWCoCreateInstance(pVec, __uuidof(SubConfigVector));
	pVec->Init(FALSE, p1Op);

	CComPtr<IConfigWithDependencies> pCfg;
	RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

	pCfg->ItemInsSimple(CComBSTR(CFGID_CURRENT), pDummy, pDummy, CConfigValue(-1L), NULL, 0, NULL);
	pCfg->ItemInsSimple(CComBSTR(CFGID_CURRENTSUB), pDummy, pDummy, CConfigValue(0L), NULL, 0, NULL);
	pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATIONS), pDummy, pDummy, CConfigValue(0L), pVec.p, 0, NULL);

	CComPtr<IStorageFilterFactory> pFct;
	RWCoCreateInstance(pFct, __uuidof(StorageFilterFactoryFileSystem));
	CComPtr<IConfig> pCfgStorage;
	pFct->ContextConfigGetDefault(&pCfgStorage);
	pCfg->ItemInsSimple(CComBSTR(CFGID_STG_DROPLET), pDummy, pDummy, CConfigValue(false), CComQIPtr<ISubConfig>(pCfgStorage), 0, NULL);
	pCfgStorage = NULL;
	pFct->ContextConfigGetDefault(&pCfgStorage);
	pCfg->ItemInsSimple(CComBSTR(CFGID_STG_BROWSER), pDummy, pDummy, CConfigValue(false), CComQIPtr<ISubConfig>(pCfgStorage), 0, NULL);

	pCfg->ItemInsRanged(CComBSTR(CFGID_SPLITTER), pDummy, pDummy, CConfigValue(0.5f), NULL, CConfigValue(0.1f), CConfigValue(0.9f), CConfigValue(0.0f), 0, NULL);

	pCfg->Finalize(NULL);

	CComPtr<ISubConfigSwitch> pY;
	RWCoCreateInstance(pY, __uuidof(SubConfigSwitch));
	pY->ItemInsert(cTrue, pCfg);
	return a_pConfig->ItemInsSimple(CComBSTR(CFGID_ROOT), pDummy, pDummy, cTrue, pY, 0, NULL);
}

void CStartPageBatchImageProcessor::WindowCreate(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig, IOperationManager* a_pOpMgr)
{
	m_pOpMgr = a_pOpMgr;
	m_tLocaleID = a_tLocaleID;
	if (a_pAppConfig)
		a_pAppConfig->SubConfigGet(CComBSTR(CFGID_ROOT), &m_pConfig);
	if (m_pConfig == NULL)
	{
		CComPtr<IConfigWithDependencies> p;
		RWCoCreateInstance(p, __uuidof(ConfigWithDependencies));
		InitConfig(p, m_pOpMgr);
		p->Finalize(NULL);
		p->SubConfigGet(CComBSTR(CFGID_ROOT), &m_pConfig);
	}
	m_hWorkThread = (HANDLE)_beginthreadex(NULL, 0, WorkerProc, this, 0, &m_uWorkThreadID);
	SHGetMalloc(&m_pShellAlloc);
	SHGetDesktopFolder(&m_pDesktopFolder);

	Create(a_hParent);
	MoveWindow(a_prc);
}

STDMETHODIMP CStartPageBatchImageProcessor::Activate()
{
	ShowWindow(SW_SHOW);
	m_wndStartStop.SetFocus();
	UpdateStatusText();
	//if (m_pActiveShellView)
	//{
	//	HWND h = NULL;
	//	m_pActiveShellView->GetWindow(&h);
	//	if (h)
	//	{
	//		::SetFocus(h);
	//	}
	//}
	if (m_bHidden)
	{
		m_bHidden = false;
		BOOL b;
		OnConfigChanged(0, 0, 0, b);
	}
	return S_OK;
}

STDMETHODIMP CStartPageBatchImageProcessor::Deactivate()
{
	if (IsWindow())
	{
		m_bHidden = true;
		ShowWindow(SW_HIDE);
	}
	return S_OK;
}

STDMETHODIMP CStartPageBatchImageProcessor::ClickedDefault()
{
	BOOL b;
	OnClickedStop(0, 0, NULL, b);
	return S_FALSE;
}

LRESULT CStartPageBatchImageProcessor::OnClickedStart(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	ForwardOK();
	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnClickedStop(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_wndStartStop.EnableButton(ID_BP_STOP, FALSE);
	m_wndProgress.SetState(PBST_ERROR);
	{
		ObjectLock cLock(this);
		m_cPaths.clear();
	}
	return 0;
}

void CStartPageBatchImageProcessor::UpdateStatusText()
{
	if (!IsWindow())
		return;

	m_wndStartStop.EnableButton(ID_BP_STOP, m_bRunning);
}

void CStartPageBatchImageProcessor::OnFinalMessage(HWND UNREF(a_hWnd))
{
	Release();
}

void CStartPageBatchImageProcessor::GetCogwheelPoints(IRPolyPoint* aInner, IRPolyPoint* aOuter)
{
	IRPolyPoint* p = aInner;
	for (int i = 0; i < 5; ++i)
	{
		float const a1 = (i * 0.4f + 0.05f) * 3.14159265359f;
		float const a2 = (i * 0.4f + 0.13f) * 3.14159265359f;
		float const a3 = (i * 0.4f + 0.27f) * 3.14159265359f;
		float const a4 = (i * 0.4f + 0.35f) * 3.14159265359f;
		p->x = 0.5f + cosf(a1) * 0.29f;
		p->y = 0.5f + sinf(a1) * 0.29f;
		++p;
		p->x = 0.5f + cosf(a2) * 0.45f;
		p->y = 0.5f + sinf(a2) * 0.45f;
		++p;
		p->x = 0.5f + cosf(a3) * 0.45f;
		p->y = 0.5f + sinf(a3) * 0.45f;
		++p;
		p->x = 0.5f + cosf(a4) * 0.29f;
		p->y = 0.5f + sinf(a4) * 0.29f;
		++p;
	}
	p = aOuter;
	for (int i = 0; i < 10; ++i)
	{
		float const a1 = (i * 0.2f + 0.05f) * 3.14159265359f;
		p->x = 0.5f + cosf(a1) * 0.11f;
		p->y = 0.5f + sinf(a1) * 0.11f;
		++p;
	}
}

LRESULT CStartPageBatchImageProcessor::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	AddRef();

	{BOOL b; CThemeImpl<CStartPageBatchImageProcessor>::OnCreate(0, 0, 0, b);}

	HDC hdc = GetDC();
	m_scale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
	ReleaseDC(hdc);

	RECT rcWhole;
	GetClientRect(&rcWhole);

	// progress bar
	m_wndProgress = GetDlgItem(IDC_BP_PROGRESS);
	m_wndProgress.SetRange(0, 0xffff);

	// message log
	m_wndTasks = GetDlgItem(IDC_BP_LOG);
	LV_COLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = 800;
	m_wndTasks.InsertColumn(0, &lvc);

	// initialize image lists
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CComPtr<IIconRenderer> pIR;
	RWCoCreateInstance(pIR, __uuidof(IconRenderer));
	IRCanvas canvas = {0, 0, 1, 1, 0, NULL, 0, NULL};

	// cogwheel
	IRPolyPoint aInner[5*4];
	IRPolyPoint aOuter[10];
	GetCogwheelPoints(aInner, aOuter);
	IRPolygon const aCogwheel[] = { {itemsof(aInner), aInner}, {itemsof(aOuter), aOuter} };
	static IRFill matCogwheelFill(0xffa88a62);
	IROutlinedFill matCogwheel(&matCogwheelFill, pSI->GetMaterial(ESMContrast));

	int const smallIcon = XPGUI::GetSmallIconSize();
	m_cToolbarImages.Create(smallIcon, smallIcon, XPGUI::GetImageListColorFlags(), 9, 1);
	{
		static IRPolyPoint const aVtx[] = { {0.1f, 0.55f}, {0.5f, 0.15f}, {0.9f, 0.55f}, {0.6f, 0.55f}, {0.9f, 0.85f}, {0.1f, 0.85f}, {0.4f, 0.55f}, };
		CIconRendererReceiver cRenderer(smallIcon);
		cRenderer(&canvas, itemsof(aVtx), aVtx, pSI->GetMaterial(ESMInterior));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		static IRPolyPoint const aVtx[] = { {0.1f, 0.45f}, {0.5f, 0.85f}, {0.9f, 0.45f}, {0.6f, 0.45f}, {0.9f, 0.15f}, {0.1f, 0.15f}, {0.4f, 0.45f}, };
		CIconRendererReceiver cRenderer(smallIcon);
		cRenderer(&canvas, itemsof(aVtx), aVtx, pSI->GetMaterial(ESMInterior));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		static IRPolyPoint const aVtx[] = { {0.2f, 0.3f}, {0.3f, 0.1f}, {0.9f, 0.1f}, {0.8f, 0.3f}, {0.8f, 0.7f}, {0.7f, 0.9f}, {0.1f, 0.9f}, {0.2f, 0.7f}, };
		CIconRendererReceiver cRenderer(smallIcon);
		cRenderer(&canvas, itemsof(aVtx), aVtx, pSI->GetMaterial(ESMInterior));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		CIconRendererReceiver cRenderer(smallIcon);
		pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.8f));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		CIconRendererReceiver cRenderer(smallIcon);
		pSI->GetLayers(ESIDuplicate, cRenderer, IRTarget(0.8f));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		CIconRendererReceiver cRenderer(smallIcon);
		pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		CIconRendererReceiver cRenderer(smallIcon);
		pSI->GetLayers(ESIModify, cRenderer, IRTarget(1.0f));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		CIconRendererReceiver cRenderer(smallIcon);
		cRenderer(&canvas, itemsof(aCogwheel), aCogwheel, &matCogwheel, IRTarget(0.8f, -1, 1));
		pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.65f, 1, -1));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		CIconRendererReceiver cRenderer(smallIcon);
		cRenderer(&canvas, itemsof(aCogwheel), aCogwheel, &matCogwheel, IRTarget(0.8f, -1, 1));
		pSI->GetLayers(ESIFloppySimple, cRenderer, IRTarget(0.65f, 1, -1));
		HICON h = cRenderer.get();
		m_cToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	m_cLargeToolbarImages.Create(smallIcon*3, smallIcon*3, XPGUI::GetImageListColorFlags(), 4, 1);
	{
		static IRPolyPoint const aVtx[] = { {0.9f, 0.5f}, {0.3f, 0.1f}, {0.3f, 0.9f}, };
		static IRPolygon const tPoly = { itemsof(aVtx), aVtx };
		HICON h = pIR->CreateIcon(smallIcon*3, &canvas, 1, &tPoly, pSI->GetMaterial(ESMConfirm));
		m_cLargeToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		static IRPolyPoint const aVtx[] = { {0.2f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.8f}, {0.2f, 0.8f}, };
		static IRPolygon const tPoly = { itemsof(aVtx), aVtx };
		HICON h = pIR->CreateIcon(smallIcon*3, &canvas, 1, &tPoly, pSI->GetMaterial(ESMCancel));
		m_cLargeToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		HICON h = pIR->CreateIcon(smallIcon*3, &canvas, itemsof(aCogwheel), aCogwheel, &matCogwheel);
		m_cLargeToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}
	{
		CIconRendererReceiver cRenderer(smallIcon*3);
		pSI->GetLayers(ESIHelp, cRenderer, IRTarget(0.8f));
		HICON h = cRenderer.get();
		m_cLargeToolbarImages.AddIcon(h);
		DestroyIcon(h);
	}


	RECT rc;
	CWindow cWnd = GetDlgItem(IDC_BP_OUTPUT);
	cWnd.GetWindowRect(&rc);
	ScreenToClient(&rc);

	RECT rcLeft;
	{
		CWindow cWnd = GetDlgItem(IDC_BP_BROWSE);
		cWnd.GetWindowRect(&rcLeft);
		ScreenToClient(&rcLeft);
	}

	CWindow wndTab = GetDlgItem(IDC_BP_HEADER1);
	wndTab.DestroyWindow();
	RECT rcTmp = {0, 0, 12, 12};
	MapDialogRect(&rcTmp);
	m_headerY = rcTmp.bottom;
	RECT rcTab = {rcLeft.left, rcLeft.top-m_headerY, rcLeft.right, rcLeft.top};
	m_wndHeader1.Create(m_hWnd, rcTab, _T("Header1"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE|CTCS_TOOLTIPS, 0, IDC_BP_HEADER1);
	m_wndHeader1.SetImageList(m_cToolbarImages);
	{
		CComBSTR bstrName; CMultiLanguageString::GetLocalized(L"[0409]Source files[0405]Zdrojové soubory", m_tLocaleID, &bstrName);
		CComBSTR bstrDesc; CMultiLanguageString::GetLocalized(L"[0409]Browse file system for files to process.[0405]Vybrat soubory pro zpracování na tomto počítači.", m_tLocaleID, &bstrDesc);
		m_wndHeader1.InsertItem(0, bstrName, 0, bstrDesc, true);
	}

	rcTab.left = rc.left;
	rcTab.right = rc.right;
	m_wndHeader2.Create(m_hWnd, rcTab, _T("Header2"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE|CTCS_TOOLTIPS, 0, IDC_BP_HEADER2);
	m_wndHeader2.SetImageList(m_cToolbarImages);
	{
		CComBSTR bstrName; CMultiLanguageString::GetLocalized(L"[0409]Processed files[0405]Zpracované soubory", m_tLocaleID, &bstrName);
		CComBSTR bstrDesc; CMultiLanguageString::GetLocalized(L"[0409]A temporary folder, where processed files are put by default.[0405]Dočasná složka pro uložení zpracovaných souborů.", m_tLocaleID, &bstrDesc);
		m_wndHeader2.InsertItem(0, bstrName, 1, bstrDesc, true);
	}

	RECT rcBottom;
	m_wndTasks.GetWindowRect(&rcBottom);
	ScreenToClient(&rcBottom);
	m_logY = rcBottom.bottom-rcBottom.top;

	rcTab.top = rcBottom.top - m_headerY;
	rcTab.bottom = rcBottom.top;
	m_wndHeader3.Create(m_hWnd, rcTab, _T("Header3"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE|CTCS_TOOLTIPS, 0, IDC_BP_HEADER3);
	m_wndHeader3.SetImageList(m_cToolbarImages);
	{
		CComBSTR bstrName; CMultiLanguageString::GetLocalized(L"[0409]Log[0405]Žurnál", m_tLocaleID, &bstrName);
		//CComBSTR bstrDesc; CMultiLanguageString::GetLocalized(L"[0409]A temporary folder, where processed files are put by default.[0405]Dočasná složka pro uložení zpracovaných souborů.", m_tLocaleID, &bstrDesc);
		m_wndHeader3.InsertItem(0, bstrName, 2, NULL, true);
	}

	// create the output folder window
	TCHAR szTmp[MAX_PATH+20] = _T("");
	GetTempPath(MAX_PATH, szTmp);
	_tcscat(szTmp, _T("RWBatch"));
	CreateDirectory(szTmp, NULL);
	CT2W strTmp(szTmp);
	LPITEMIDLIST pIDList = nullptr;
	ULONG nAttribs = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER;
	m_pDesktopFolder->ParseDisplayName(nullptr, nullptr, strTmp, nullptr, &pIDList, &nAttribs);

	CComPtr<IShellFolder> pFolder;
	m_pDesktopFolder->BindToObject(pIDList, NULL, IID_IShellFolder, (void**)&pFolder);

	m_pShellAlloc->Free(pIDList);

	if (pFolder)
	{
		cWnd.DestroyWindow();

		m_pLastFolder = pFolder;
		m_pLastFolder->CreateViewObject(m_hWnd, IID_IShellView, (void**)&m_pActiveShellView);
		if (m_pActiveShellView)
		{
			m_pActiveShellView->CreateViewWindow(m_pActiveShellView, &m_tViewSettings, this, &rc, &m_hActiveShellWnd);
			::SetWindowLong(m_hActiveShellWnd, GWL_ID, IDC_BP_OUTPUT);
			m_pActiveShellView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
		}
	}

	// initialize operation list
	m_cOperationImages.Create(smallIcon, smallIcon, XPGUI::GetImageListColorFlags(), 8, 4);
	{
		CIconRendererReceiver cRenderer(smallIcon);
		HICON h = cRenderer.get();
		m_cOperationImages.AddIcon(h);
		DestroyIcon(h);
	}
	m_cLargeOperationImages.Create(3 * smallIcon, 3 * smallIcon, XPGUI::GetImageListColorFlags(), 8, 4);
	{
		CIconRendererReceiver cRenderer(3 * smallIcon);
		IRFill matSemiCogwheelFill(0x80a88a62);
		IRFill matSemiCogwheelOut(0x80ffffff&pSI->GetSRGBColor(ESMContrast));
		IROutlinedFill matSemiCogwheel(&matSemiCogwheelFill, &matSemiCogwheelOut);
		cRenderer(&canvas, itemsof(aCogwheel), aCogwheel, &matSemiCogwheel);
		HICON h = cRenderer.get();
		m_cLargeOperationImages.AddIcon(h);
		DestroyIcon(h);
	}

	// start-stop button
	m_wndStartStop = GetDlgItem(IDC_BP_STARTSTOP);
	CComBSTR bstrStart;
	CMultiLanguageString::GetLocalized(L"[0409]Start[0405]Spustit", m_tLocaleID, &bstrStart);
	CComBSTR bstrStop;
	CMultiLanguageString::GetLocalized(L"[0409]Stop[0405]Zastavit", m_tLocaleID, &bstrStop);
	CComBSTR bstrOptions;
	CMultiLanguageString::GetLocalized(L"[0409]Options[0405]Možnosti", m_tLocaleID, &bstrOptions);
	CComBSTR bstrHelp;
	CMultiLanguageString::GetLocalized(L"[0409]Help[0405]Nápověda", m_tLocaleID, &bstrHelp);
	TBBUTTON aStartStopButtons[] =
	{
		{I_IMAGENONE, ID_BP_SELECT, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, NULL},
		{0, ID_BP_START, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, INT_PTR(bstrStart.m_str)},
		{1, ID_BP_STOP, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, INT_PTR(bstrStop.m_str)},
		{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
		{2, ID_BP_OPTIONS, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, INT_PTR(bstrOptions.m_str)},
		{3, ID_BP_HELP, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, INT_PTR(bstrHelp.m_str)},
	};

	m_wndStartStop.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_wndStartStop.SetImageList(m_cLargeToolbarImages);
	m_wndStartStop.AddButtons(itemsof(aStartStopButtons), aStartStopButtons);

	// increase hint font size
	LOGFONT lf = {0};
	::GetObject(GetFont(), sizeof(lf), &lf);
	lf.lfHeight += lf.lfHeight>>1;
	m_cLargeFont.CreateFontIndirect(&lf);

	{
		CComPtr<IStorageFilterFactory> pFct;
		RWCoCreateInstance(pFct, __uuidof(StorageFilterFactoryFileSystem));
		CComPtr<IConfig> pCfg;
		m_pConfig->SubConfigGet(CComBSTR(CFGID_STG_BROWSER), &pCfg);
		//CComObject<CStorageWindowCallback>* pCallback = NULL;
		//CComObject<CStorageWindowCallback>::CreateInstance(&pCallback);
		//CComPtr<IStorageFilterWindowCallback> pCb = pCallback;
		//pCallback->Init();
		pFct->WindowCreate(NULL, EFTFileBrowser|EFTMultiselection, m_hWnd, NULL, pCfg, this, NULL, m_tLocaleID, &m_pBrowseWnd);
		CWindow wnd = GetDlgItem(IDC_BP_BROWSE);
		RECT rc;
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_pBrowseWnd->Move(&rc);
		m_pBrowseWnd->Show(TRUE);
		wnd.DestroyWindow();
		RWHWND h = NULL;
		m_pBrowseWnd->Handle(&h);
		::SetWindowLong(h, GWLP_ID, IDC_BP_BROWSE);
	}

	if (m_pConfig)
		m_pConfig->ObserverIns(ObserverGet(), 0);

	// init resizing constants
	RECT rcGaps = {4, 4, 7, 7};
	MapDialogRect(&rcGaps);
	m_gap4.cx = rcGaps.left;
	m_gap4.cy = rcGaps.top;
	m_gap7.cx = rcGaps.right;
	m_gap7.cy = rcGaps.bottom;

	m_wndProgress.GetWindowRect(&rc);
	m_progressY = rc.bottom-rc.top;

	// toolbar
	RECT rcInitial;
	m_wndStartStop.GetWindowRect(&rcInitial);
	ScreenToClient(&rcInitial);
	RECT rcLastBtn;
	m_wndStartStop.GetItemRect(m_wndStartStop.GetButtonCount()-1, &rcLastBtn);
	RECT rcVariableBtn;
	m_wndStartStop.GetItemRect(0, &rcVariableBtn);
	m_toolbar.cy = rcLastBtn.bottom-rcLastBtn.top;
	m_toolbar.cx = rcLastBtn.right-rcVariableBtn.right;
	// align the toolbar to the right
	m_wndStartStop.SetButtonInfo(ID_BP_SELECT, TBIF_SIZE, 0, 0, 0, 0, rcWhole.right-m_gap7.cx-m_gap7.cx-m_toolbar.cx, 0, 0);
	m_wndStartStop.MoveWindow(m_gap7.cx, m_gap7.cy, rcWhole.right-m_gap7.cx-m_gap7.cx, m_gap7.cy+m_toolbar.cy, FALSE);

	m_wndLine1 = GetDlgItem(IDC_BP_LINE1);
	m_wndLine2 = GetDlgItem(IDC_BP_LINE2);
	m_wndLine3 = GetDlgItem(IDC_BP_LINE3);
	m_wndLine4 = GetDlgItem(IDC_BP_LINE4);
	m_wndLine5 = GetDlgItem(IDC_BP_LINE5);

	SIZE const sz = {rcWhole.right, rcWhole.bottom};
	RepositionControls(sz);

	BOOL b;
	OnConfigChanged(0, 0, 0, b);

	return 1;  // Let the system set the focus
}

LRESULT CStartPageBatchImageProcessor::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	SIZE const sz = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};

	bool visible = IsWindowVisible();
	if (visible)
		SetRedraw(FALSE);

	RepositionControls(sz);

	if (visible)
	{
		SetRedraw(TRUE);
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	return 0;
}

void CStartPageBatchImageProcessor::RepositionControls(SIZE const sz)
{
	m_wndStartStop.SetButtonInfo(ID_BP_SELECT, TBIF_SIZE, 0, 0, 0, 0, sz.cx-m_gap7.cx-m_gap7.cx-m_toolbar.cx, 0, 0);
	m_wndStartStop.MoveWindow(m_gap7.cx, m_gap7.cy, sz.cx-m_gap7.cx-m_gap7.cx, m_toolbar.cy, FALSE);
	int y = m_gap7.cy+m_toolbar.cy+m_gap7.cy;

	m_wndProgress.MoveWindow(m_gap7.cx*2, y, sz.cx-m_gap7.cx*4, m_progressY, FALSE);
	y += m_progressY+m_gap7.cy;

	int const sepPos0 = m_gap7.cx+static_cast<int>((sz.cx-m_gap7.cx-m_gap4.cx)*m_splitPos+0.5f);
	int const sepPos1 = sepPos0+m_gap4.cx;

	m_wndHeader1.MoveWindow(m_gap7.cx, y, sepPos0-m_gap7.cx, m_headerY, FALSE);
	RECT const rcBW = {m_gap7.cx, y+m_headerY, sepPos0, sz.cy};
	m_pBrowseWnd->Move(&rcBW);
	m_wndLine1.MoveWindow(m_gap7.cx-1, y+m_headerY-3, 1, sz.cy-y-m_headerY+3, FALSE);
	m_wndLine2.MoveWindow(sepPos0, y+m_headerY-3, 1, sz.cy-y-m_headerY+3, FALSE);

	m_wndHeader2.MoveWindow(sepPos1, y, sz.cx-sepPos1, m_headerY, FALSE);
	HWND hOutWnd = NULL;
	m_pActiveShellView->GetWindow(&hOutWnd);
	::MoveWindow(hOutWnd, sepPos1, y+m_headerY, sz.cx-sepPos1, sz.cy-m_logY-m_headerY-m_gap4.cy-y-m_headerY, FALSE);
	m_wndLine3.MoveWindow(sepPos1-1, y+m_headerY-3, 1, sz.cy-m_logY-m_headerY-m_gap4.cy-y-m_headerY+3, FALSE);
	m_wndLine5.MoveWindow(sepPos1-1, sz.cy-m_logY-m_headerY-m_gap4.cy, sz.cx-sepPos1+1, 1, FALSE);
	y = sz.cy-m_logY-m_headerY;

	m_wndHeader3.MoveWindow(sepPos1, y, sz.cx-sepPos1, m_headerY, FALSE);
	m_wndTasks.MoveWindow(sepPos1, y+m_headerY, sz.cx-sepPos1, m_logY, FALSE);
	m_wndLine4.MoveWindow(sepPos1-1, y+m_headerY-3, 1, m_logY+3, FALSE);
}

LRESULT CStartPageBatchImageProcessor::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != IDC_BP_LINE1 && wParam != IDC_BP_LINE2 && wParam != IDC_BP_LINE3 && wParam != IDC_BP_LINE4 && wParam != IDC_BP_LINE5)
	{
		bHandled = FALSE;
		return 0;
	}

	DRAWITEMSTRUCT const* const pDIS = reinterpret_cast<DRAWITEMSTRUCT const*>(lParam);
	FillRect(pDIS->hDC, &pDIS->rcItem, reinterpret_cast<HBRUSH>(COLOR_3DSHADOW+1));

	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	int const sepPos0 = m_gap7.cx+static_cast<int>((rc.right-m_gap7.cx-m_gap4.cx)*m_splitPos+0.5f);
	int const sepPos1 = sepPos0+m_gap4.cx;

	POINT tPos;
	GetCursorPos(&tPos);
	ScreenToClient(&tPos);
	if (tPos.x < sepPos0 || tPos.x >= sepPos1 ||
		tPos.y < LONG(m_gap7.cy+m_toolbar.cy+m_gap7.cy+m_progressY+m_gap7.cy+m_headerY-3))
	{
		bHandled = FALSE;
		return FALSE;
	}

	if (m_hVertical == NULL)
		m_hVertical = LoadCursor(NULL, IDC_SIZEWE);
	SetCursor(m_hVertical);
	return TRUE;
}

LRESULT CStartPageBatchImageProcessor::OnLButtonDown(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	m_lastMousePos = GET_X_LPARAM(a_lParam);

	RECT rc;
	GetClientRect(&rc);
	int const sepPos0 = m_gap7.cx+static_cast<int>((rc.right-m_gap7.cx-m_gap4.cx)*m_splitPos+0.5f);
	int const sepPos1 = sepPos0+m_gap4.cx;

	if (m_lastMousePos >= sepPos0 && m_lastMousePos < sepPos1 &&
		GET_Y_LPARAM(a_lParam) >= LONG(m_gap7.cy+m_toolbar.cy+m_gap7.cy+m_progressY+m_gap7.cy+m_headerY-3))
	{
		m_dragOffset = m_lastMousePos-sepPos0;
		SetCapture();
	}

	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnLButtonUp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	ReleaseCapture();

	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnMouseMove(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (GetCapture() == m_hWnd)
	{
		RECT rc;
		GetClientRect(&rc);
		LONG const workable = rc.right-m_gap7.cx-m_gap4.cx;

		LONG const mousePos = GET_X_LPARAM(a_lParam);
		if (mousePos != m_lastMousePos)
		{
			m_lastMousePos = mousePos;
			float pos = float(mousePos-m_dragOffset-m_gap7.cx)/workable;
			if (pos < 0.1f) pos = 0.1f;
			else if (pos > 0.9f) pos = 0.9f;
			if (m_splitPos != pos)
			{
				m_splitPos = pos;
				m_bEnableUpdates = false;
				CComBSTR id(CFGID_SPLITTER);
				m_pConfig->ItemValuesSet(1, &(id.m_str), CConfigValue(pos));
				m_bEnableUpdates = true;
				SIZE const sz = {rc.right, rc.bottom};
				RepositionControls(sz);
				RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
		}
	}

	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	if (m_pConfig)
		m_pConfig->ObserverDel(ObserverGet(), 0);

	if (m_pActiveShellView != NULL)
	{
		m_pActiveShellView->UIActivate(SVUIA_DEACTIVATE);
		m_pActiveShellView->DestroyViewWindow();
		m_pActiveShellView = NULL;
	}

	m_pBrowseWnd = NULL;

	a_bHandled = FALSE;
	return 0;
}

void CStartPageBatchImageProcessor::OwnerNotify(TCookie, IUnknown*)
{
	try
	{
		// TODO: dead-lock -> post an update message to self;
		if (!IsWindow() || m_bHidden || !m_bEnableUpdates || m_bRequestPosted)
			return;
		m_bRequestPosted = true;
		PostMessage(WM_CONFIGCHANGED);
	}
	catch (...)
	{
	}
}

LRESULT CStartPageBatchImageProcessor::OnConfigChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	try
	{
		m_bRequestPosted = false;

		UpdateActiveOperationCache();

		RECT rc;
		m_wndStartStop.GetItemRect(0, &rc);
		m_wndStartStop.InvalidateRect(&rc, FALSE);

		// update file filter according to supported doc type
		if (!IsEqualGUID(m_activeBuilder, m_tOpBuilder))
		{
			m_tOpBuilder = m_activeBuilder;
			CComPtr<IDocumentBuilder> pBuilder;
			if (!IsEqualGUID(GUID_NULL, m_tOpBuilder))
				RWCoCreateInstance(pBuilder, m_tOpBuilder);
			CComPtr<IEnumUnknowns> pFilters;
			if (pBuilder)
			{
				CComPtr<IInputManager> pIM;
				RWCoCreateInstance(pIM, __uuidof(InputManager));
				pIM->DocumentTypesEnumEx(pBuilder, &pFilters);
			}
			if (pFilters)
			{
				CComPtr<IDocumentTypeComposed> pType;
				RWCoCreateInstance(pType, __uuidof(DocumentTypeComposed));
				pType->DocTypesAddFromEnum(pFilters);
				m_pBrowseWnd->DocTypeSet(pType);
			}
			else
			{
				m_pBrowseWnd->DocTypeSet(NULL);
			}
		}

		// update output folder
		if (m_lastActiveOutput != m_activeOutput)
		{
			CComBSTR bstrPath;
			if (m_activeOutput.Length() == 0)
			{
				// create the output folder window
				TCHAR szTmp[MAX_PATH+20] = _T("");
				GetTempPath(MAX_PATH, szTmp);
				_tcscat(szTmp, _T("RWBatch"));
				CreateDirectory(szTmp, NULL);
				bstrPath = szTmp;
			}
			else
			{
				bstrPath = m_activeOutput;
			}
			LPITEMIDLIST pIDList = nullptr;
			ULONG nAttribs = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER;
			m_pDesktopFolder->ParseDisplayName(nullptr, nullptr, bstrPath, nullptr, &pIDList, &nAttribs);

			CComPtr<IShellFolder> pFolder;
			m_pDesktopFolder->BindToObject(pIDList, NULL, IID_IShellFolder, (void**)&pFolder);

			m_pShellAlloc->Free(pIDList);

			if (pFolder)
			{
				bool bSetFocus = false;
				if (m_pActiveShellView)
				{
					HWND hFoc = GetFocus();
					if (hFoc)
						bSetFocus = m_hActiveShellWnd == hFoc || ::IsChild(m_hActiveShellWnd, hFoc);
				}

				CComPtr<IShellView> pNewView;
				pFolder->CreateViewObject(m_hWnd, IID_IShellView, (void**)&pNewView);
				if (pNewView)
				{
					CWindow wnd = GetDlgItem(IDC_BP_OUTPUT);
					RECT rc;
					wnd.GetWindowRect(&rc);
					ScreenToClient(&rc);

					if (SUCCEEDED(pNewView->CreateViewWindow(m_pActiveShellView, &m_tViewSettings, this, &rc, &m_hActiveShellWnd)))
					{
						::SetWindowLong(m_hActiveShellWnd, GWL_ID, IDC_BP_OUTPUT);

						if (m_pActiveShellView != NULL)
						{
							m_pActiveShellView->UIActivate(SVUIA_DEACTIVATE);
							m_pActiveShellView->DestroyViewWindow();
						}
						else
						{
							if (wnd.m_hWnd)
								wnd.DestroyWindow();
						}
						m_pActiveShellView = pNewView;
						m_pActiveShellView->UIActivate(bSetFocus ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS);

						m_pLastFolder = pFolder;
						m_lastActiveOutput = m_activeOutput;
					}
				}
			}
		}
	}
	catch (...)
	{
	}
	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnAddLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!m_wndTasks.IsWindow() || lParam < 0x10000)
		return 0;
	int const lenW = wParam ? _tcslen(reinterpret_cast<LPCTSTR>(wParam)) : 0;
	int const lenL = lParam ? _tcslen(reinterpret_cast<LPCTSTR>(lParam)) : 0;
	if (lenW && lenL)
	{
		TCHAR* p = new TCHAR[lenW + lenL + 2];
		_tcscpy(p, reinterpret_cast<LPCTSTR>(lParam));
		p[lenL] = _T(' ');
		_tcscpy(p+lenL+1, reinterpret_cast<LPCTSTR>(wParam));
		m_wndTasks.AddItem(0, 0, p);
	}
	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnUpdateStatus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_wndProgress.SetPos(lParam < 0xffff ? lParam : 0);
	m_wndProgress.SetMarquee(lParam < 0xffff, 100);
	m_wndProgress.SetState(PBST_NORMAL);
	m_bRunning = wParam;

	UpdateStatusText();
	return 0;
}

bool CStartPageBatchImageProcessor::OnOperationDelete()
{
	CComBSTR bstrText;
	CMultiLanguageString::GetLocalized(L"[0409]Really delete the selected batch operation? The deletion cannot be undone.[0405]Opravdu smazat vybranou dávkovou operaci? Smazání nelze vrátit zpět.", m_tLocaleID, &bstrText);
	CComBSTR bstrCaption;
	CMultiLanguageString::GetLocalized(L"[0409]Confirm Delete[0405]Potvrdit smazání", m_tLocaleID, &bstrCaption);
	if (IDYES != MessageBox(bstrText, bstrCaption, MB_YESNO|MB_ICONQUESTION))
		return true;
	CConfigValue cCur;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENT), &cCur);
	CConfigValue cOps;
	CComBSTR cCFGID_OPERATIONS(CFGID_OPERATIONS);
	m_pConfig->ItemValueGet(cCFGID_OPERATIONS, &cOps);
	if (cCur.operator LONG() < 0 || cCur.operator LONG() >= cOps.operator LONG())
		return false;
	CComPtr<IConfig> pSubCfg;
	m_pConfig->SubConfigGet(cCFGID_OPERATIONS, &pSubCfg);
	CComQIPtr<IConfigVector> pVec(pSubCfg);
	if (pVec == NULL)
		return false;
	cOps = cOps.operator LONG()-1L;
	pVec->Move(cCur.operator LONG(), cOps.operator LONG());
	BSTR bstrs[3];
	bstrs[0] = cCFGID_OPERATIONS;
	CComBSTR cCFGID_CURRENT(CFGID_CURRENT);
	bstrs[2] = cCFGID_CURRENT;
	CComBSTR cCFGID_CURRENTSUB(CFGID_CURRENTSUB);
	bstrs[1] = cCFGID_CURRENTSUB;
	TConfigValue vals[3];
	vals[0] = cOps;
	vals[1] = CConfigValue(0L);
	vals[2] = CConfigValue(cCur.operator LONG()-1L);
	m_pConfig->ItemValuesSet(cOps.operator LONG() <= cCur.operator LONG() ? 3 : 2, bstrs, vals);
	return true;
}

HICON CStartPageBatchImageProcessor::GetCogwheelIcon(ULONG size)
{
	// initialize image lists
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = { 0, 0, 1, 1, 0, NULL, 0, NULL };

	// cogwheel
	IRPolyPoint aInner[5 * 4];
	IRPolyPoint aOuter[10];
	GetCogwheelPoints(aInner, aOuter);
	IRPolygon const aCogwheel[] = { {itemsof(aInner), aInner}, {itemsof(aOuter), aOuter} };
	static IRFill matCogwheelFill(0xffa88a62);
	IROutlinedFill matCogwheel(&matCogwheelFill, pSI->GetMaterial(ESMContrast));

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(aCogwheel), aCogwheel, &matCogwheel);
	return cRenderer.get();
}

void CStartPageBatchImageProcessor::GetRWBatchOpDocType(CComPtr<IDocumentTypeWildcards>& pDocType)
{
	RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
	CComBSTR bstrExt(L"rwbatchop");
	CComBSTR bstrFilter(L"*.rwbatchop");
	pDocType->InitEx2(CMultiLanguageString::GetAuto(L"[0409]RealWorld Batch Operations[0405]Dávkové operace RealWorldu"), NULL, 1, &(bstrExt.m_str), NULL, GetCogwheelIcon, bstrFilter);
}

bool CStartPageBatchImageProcessor::OnOperationImport()
{
	CComPtr<IDocumentTypeWildcards> pDocType;
	GetRWBatchOpDocType(pDocType);
	CComPtr<IEnumUnknownsInit> pDocTypes;
	RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
	pDocTypes->Insert(pDocType);
	CComPtr<IStorageManager> pStMgr;
	RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
	CComPtr<IStorageFilter> pStorage;
	static GUID const tBatchOpStorageCtx = {0xcdfc8178, 0x581d, 0x4371, {0xae, 0x61, 0x80, 0x7c, 0x76, 0xeb, 0xf5, 0x5c}};//{0xf0bdc31e, 0x7ea2, 0x4b08, {0x8b, 0x7c, 0x3f, 0x87, 0x20, 0xfe, 0x2f, 0x53}};
	pStMgr->FilterCreateInteractivelyUID(NULL, EFTOpenExisting, m_hWnd, pDocTypes, NULL, tBatchOpStorageCtx, CMultiLanguageString::GetAuto(L"[0409]Open Batch Operation[0405]Otevřít dávkovou operaci"), NULL, m_tLocaleID, &pStorage);
	if (pStorage == NULL)
		return true;

	CComPtr<IDataSrcDirect> pSrc;
	pStorage->SrcOpen(&pSrc);
	ULONG nSize = 0;
	if (pSrc == NULL || FAILED(pSrc->SizeGet(&nSize)) || nSize == 0)
	{
		// TODO: message
		return false;
	}
	CDirectInputLock cData(pSrc, nSize);

	CComPtr<IConfigInMemory> pMemCfg;
	RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
	pMemCfg->DataBlockSet(nSize, cData.begin());

	CConfigValue cOps;
	CComBSTR cCFGID_OPERATIONS(CFGID_OPERATIONS);
	m_pConfig->ItemValueGet(cCFGID_OPERATIONS, &cOps);
	OLECHAR szTmp[64];
	swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, cOps.operator LONG());
	CConfigValue cOps2(cOps.operator LONG()+1L);
	BSTR bstrs[2];
	bstrs[0] = cCFGID_OPERATIONS;
	CComBSTR cCFGID_CURRENT(CFGID_CURRENT);
	bstrs[1] = cCFGID_CURRENT;
	TConfigValue vals[2];
	vals[0] = cOps2;
	vals[1] = cOps;
	m_pConfig->ItemValuesSet(2, bstrs, vals);
	CComPtr<IConfig> p1Op;
	if (FAILED(m_pConfig->SubConfigGet(CComBSTR(szTmp), &p1Op)))
		return false;
	CopyConfigValues(p1Op, pMemCfg);
	return true;
}

#include <ReturnedData.h>

bool CStartPageBatchImageProcessor::OnOperationExport()
{
	CConfigValue cCur;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENT), &cCur);
	OLECHAR szTmp[64];
	swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, cCur.operator LONG());
	CComPtr<IConfig> p1Op;
	if (FAILED(m_pConfig->SubConfigGet(CComBSTR(szTmp), &p1Op)))
		return false;

	CComPtr<IConfigInMemory> pMemCfg;
	RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
	CopyConfigValues(pMemCfg, p1Op);
	CReturnedData dst;
	pMemCfg->TextBlockGet(&dst);
	if (dst.size() == 0)
		return 0;

	CConfigValue cName;
	p1Op->ItemValueGet(CComBSTR(CFGID_NAME), &cName);

	CComPtr<IDocumentTypeWildcards> pDocType;
	GetRWBatchOpDocType(pDocType);
	CComPtr<IEnumUnknownsInit> pDocTypes;
	RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
	pDocTypes->Insert(pDocType);
	CComPtr<IStorageManager> pStMgr;
	RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
	CComPtr<IStorageFilter> pStorage;
	static GUID const tBatchOpStorageCtx = {0xcdfc8178, 0x581d, 0x4371, {0xae, 0x61, 0x80, 0x7c, 0x76, 0xeb, 0xf5, 0x5c}};//{0xf0bdc31e, 0x7ea2, 0x4b08, {0x8b, 0x7c, 0x3f, 0x87, 0x20, 0xfe, 0x2f, 0x53}};
	CComBSTR bstrLoc;
	CMultiLanguageString::GetLocalized(cName, m_tLocaleID, &bstrLoc);
	bstrLoc += L".rwbatchop";
	pStMgr->FilterCreateInteractivelyUID(bstrLoc, EFTCreateNew, m_hWnd, pDocTypes, NULL, tBatchOpStorageCtx, CMultiLanguageString::GetAuto(L"[0409]Save Batch Operation[0405]Uložit dávkovou operaci"), NULL, m_tLocaleID, &pStorage);
	if (pStorage == NULL)
		return true;
	CComPtr<IDataDstStream> pDst;
	pStorage->DstOpen(&pDst);
	if (pDst == NULL || FAILED(pDst->Write(dst.size(), dst.begin())) || FAILED(pDst->Close()))
	{
		// TODO: message
		return false;
	}
	return true;
}


class CCustCfgDlg :
	public Win32LangEx::CLangIndirectDialogImpl<CCustCfgDlg>,
	public CDialogResize<CCustCfgDlg>
{
public:
	enum { IDC_RT_CONFIG = 150 };

	CCustCfgDlg(LCID a_tLocaleID, IConfig* a_pCfg, CLSID a_tConfigWindow) : m_hIcon(NULL),
		Win32LangEx::CLangIndirectDialogImpl<CCustCfgDlg>(a_tLocaleID), m_pConfig(a_pCfg), m_tConfigWindow(a_tConfigWindow)
	{
	}
	~CCustCfgDlg()
	{
		if (m_hIcon) DestroyIcon(m_hIcon);
	}

	BEGIN_DIALOG_EX(0, 0, 415, 260, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT)
		DIALOG_CAPTION(_T("[0409]Configure Batch Operation[0405]Konfigurovat dávkovou operaci"))
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_RT_CONFIG, WC_STATIC, SS_BLACKRECT, 0, 0, 415, 239, 0)
		CONTROL_DEFPUSHBUTTON(_T("[0409]OK[0405]OK"), IDOK, 305, 239, 50, 14, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Cancel[0405]Storno"), IDCANCEL, 359, 239, 50, 14, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Help[0405]Nápověda"), IDHELP, 7, 239, 50, 14, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CCustCfgDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOKOrCancel)
		COMMAND_ID_HANDLER(IDCANCEL, OnOKOrCancel)
		COMMAND_ID_HANDLER(IDHELP, OnHelp)
		CHAIN_MSG_MAP(CDialogResize<CCustCfgDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CCustCfgDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_RT_CONFIG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_hIcon = CStartPageBatchImageProcessor::GetCogwheelIcon(GetSystemMetrics(SM_CXSMICON));
		// set icons
		if (m_hIcon != NULL)
			SetIcon(m_hIcon, FALSE);

		HWND hPlaceHolder = GetDlgItem(IDC_RT_CONFIG);
		RECT rc;
		::GetWindowRect(hPlaceHolder, &rc);
		ScreenToClient(&rc);
		// TODO: remove this hack for tree ctrl - it does not support margins
		RWCoCreateInstance(m_pConfigWnd, m_tConfigWindow);
		if (IsEqualGUID(m_tConfigWindow, __uuidof(TreeConfigWnd)))
		{
			RECT rcTmp = {7, 7, 14, 14};
			MapDialogRect(&rcTmp);
			rc.left += rcTmp.left;
			rc.top += rcTmp.top;
			rc.right -= rcTmp.left;
			rc.bottom -= rcTmp.top;
			m_pConfigWnd->Create(m_hWnd, &rc, IDC_RT_CONFIG, m_tLocaleID, TRUE, ECWBMNothing);
		}
		else
		{
			m_pConfigWnd->Create(m_hWnd, &rc, IDC_RT_CONFIG, m_tLocaleID, TRUE, ECWBMMargin);
		}
		m_pConfigWnd->ConfigSet(m_pConfig, ECPMSimplified);
		RWHWND h = NULL;
		m_pConfigWnd->Handle(&h);
		if (h)
		{
			::SetWindowPos(h, hPlaceHolder, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
			::SetFocus(h);
		}
		::DestroyWindow(hPlaceHolder);

		// center the dialog on the screen
		CenterWindow();

		DlgResize_Init();

		return TRUE;
	}
	LRESULT OnOKOrCancel(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		EndDialog(wID);
		return 0;
	}
	LRESULT OnHelp(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		ShellExecute(NULL, _T("open"), _T("http://www.rw-designer.com/configure-batch-operation"), NULL, NULL, SW_SHOW);
		return 0;
	}


private:
	CComPtr<IConfigWnd> m_pConfigWnd;
	CComPtr<IConfig> m_pConfig;
	CLSID m_tConfigWindow;
	HICON m_hIcon;
};

bool CStartPageBatchImageProcessor::OnOperationConfigure()
{
	CConfigValue cCur;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENT), &cCur);
	OLECHAR szTmp[64];
	swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, cCur.operator LONG());
	CComBSTR bstrTmp(szTmp);
	CComPtr<IConfig> p1Op;
	if (FAILED(m_pConfig->SubConfigGet(bstrTmp, &p1Op)))
		return false;

	CComPtr<IConfig> pCopy;
	p1Op->DuplicateCreate(&pCopy);
	CCustCfgDlg cDlg(m_tLocaleID, pCopy, __uuidof(AutoConfigWnd));//__uuidof(TreeConfigWnd));
	if (cDlg.DoModal(m_hWnd) == IDOK)
	{
		CopyConfigValues(p1Op, pCopy);
	}
	return true;
}

bool CStartPageBatchImageProcessor::OnOperationCreate()
{
	CComPtr<IConfig> copy;
	m_pConfig->DuplicateCreate(&copy);

	CConfigValue cOps;
	CComBSTR cCFGID_OPERATIONS(CFGID_OPERATIONS);
	copy->ItemValueGet(cCFGID_OPERATIONS, &cOps);
	OLECHAR szTmp[64];
	swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, cOps.operator LONG());
	BSTR bstrs[2];
	bstrs[0] = cCFGID_OPERATIONS;
	CComBSTR cCFGID_CURRENT(CFGID_CURRENT);
	bstrs[1] = cCFGID_CURRENT;
	TConfigValue vals[2];
	vals[1] = cOps;
	cOps = cOps.operator LONG()+1L;
	vals[0] = cOps;
	copy->ItemValuesSet(2, bstrs, vals);

	CComPtr<IConfig> pNewOp;
	if (FAILED(copy->SubConfigGet(CComBSTR(szTmp), &pNewOp)))
		return false;

	CCustCfgDlg cDlg(m_tLocaleID, pNewOp, __uuidof(AutoConfigWnd));//__uuidof(TreeConfigWnd));
	if (cDlg.DoModal(m_hWnd) == IDOK)
	{
		CopyConfigValues(m_pConfig, copy);
	}
	return true;
}

bool CStartPageBatchImageProcessor::OnOperationDuplicate()
{
	CComPtr<IConfig> copy;
	m_pConfig->DuplicateCreate(&copy);

	CConfigValue cCur;
	copy->ItemValueGet(CComBSTR(CFGID_CURRENT), &cCur);
	OLECHAR szTmp[64];
	swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, cCur.operator LONG());
	CComPtr<IConfig> p1Op;
	if (FAILED(copy->SubConfigGet(CComBSTR(szTmp), &p1Op)))
		return false;

	CConfigValue cOps;
	CComBSTR cCFGID_OPERATIONS(CFGID_OPERATIONS);
	copy->ItemValueGet(cCFGID_OPERATIONS, &cOps);
	swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, cOps.operator LONG());
	BSTR bstrs[2];
	bstrs[0] = cCFGID_OPERATIONS;
	CComBSTR cCFGID_CURRENT(CFGID_CURRENT);
	bstrs[1] = cCFGID_CURRENT;
	TConfigValue vals[2];
	vals[1] = cOps;
	cOps = cOps.operator LONG()+1L;
	vals[0] = cOps;
	copy->ItemValuesSet(2, bstrs, vals);

	CComPtr<IConfig> p2Op;
	if (FAILED(copy->SubConfigGet(CComBSTR(szTmp), &p2Op)))
		return false;
	CopyConfigValues(p2Op, p1Op);

	CCustCfgDlg cDlg(m_tLocaleID, p2Op, __uuidof(AutoConfigWnd));//__uuidof(TreeConfigWnd));
	if (cDlg.DoModal(m_hWnd) == IDOK)
	{
		CopyConfigValues(m_pConfig, copy);
	}
	return true;
}


class ATL_NO_VTABLE CCrc32FromRetData : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IReturnedData
{
public:
	CCrc32FromRetData() : crc(~0UL) {}
	operator ULONG() const { return ~crc; }

BEGIN_COM_MAP(CCrc32FromRetData)
	COM_INTERFACE_ENTRY(IReturnedData)
END_COM_MAP()

	// IReturnedData methods
public:
	STDMETHOD(Write)(ULONG size, BYTE const* data)
	{
		static ULONG const crc32tab[256] = 
		{
		   0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
		   0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
		   0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		   0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
		   0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		   0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		   0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
		   0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
		   0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		   0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		   0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
		   0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		   0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
		   0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
		   0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		   0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
		   0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
		   0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		   0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
		   0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		   0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		   0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
		   0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
		   0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		   0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		   0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
		   0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		   0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
		   0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
		   0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		   0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
		   0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
		   0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		   0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
		   0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		   0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		   0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
		   0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
		   0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		   0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		   0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
		   0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		   0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
		   0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
		   0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		   0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
		   0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
		   0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		   0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
		   0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		   0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		   0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
		   0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
		   0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		   0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		   0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
		   0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		   0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
		   0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
		   0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		   0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
		   0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
		   0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		   0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
		};

        for (BYTE const* end = data+size; data != end; ++data) 
            crc = (crc >> 8) ^ crc32tab[(crc ^ *data) & 0xff];

        return S_OK;
	}

private:
	ULONG crc;
};

ULONG CStartPageBatchImageProcessor::ComputeCfgHash(IConfig* cfg)
{
	CComPtr<IConfigInMemory> mem;
	RWCoCreateInstance(mem, __uuidof(ConfigInMemory));
	mem->CopyFrom(cfg, NULL);
	CComObjectStackEx<CCrc32FromRetData> crc32;
	mem->DataBlockGetData(&crc32);
	return crc32;
}

void CStartPageBatchImageProcessor::AddOpInfo(IConfigDescriptor* opAss, IConfig* opCfg, CSubOps& subOps, IConfig* cloneBase, BSTR cloneId, TConfigValue const* opId)
{
	SSubOp t;
	t.hash = ComputeCfgHash(opCfg)|1;
	cloneBase->DuplicateCreate(&t.config);
	t.config->ItemValuesSet(1, &cloneId, opId);
	CComPtr<IConfig> sub;
	t.config->SubConfigGet(cloneId, &sub);
	CopyConfigValues(sub, opCfg);
	opAss->Name(NULL, opCfg, &t.name);
	subOps.push_back(t);
}

void CStartPageBatchImageProcessor::AddOpsFromHistory(IConfig* templ, IConfig* history, IConfigDescriptor* opAss, CSubOps& subOps, IConfig* cloneBase, BSTR cloneId, TConfigValue const* opId)
{
	CComPtr<IEnumStrings> pIDs;
	templ->ItemIDsEnum(&pIDs);
	ULONG nIDs = 0;
	if (pIDs) pIDs->Size(&nIDs);
	std::vector<CComBSTR> aIDs;
	aIDs.resize(nIDs);
	for (ULONG i = 0; i < nIDs; ++i)
		pIDs->Get(i, &aIDs[i]);
	std::vector<CConfigValue> aVals;
	aVals.resize(nIDs);

	for (LONG i = 0; i < 10; ++i)
	{
		wchar_t szID[10] = L" ";
		szID[0] = L'0'+i;
		CConfigValue cMarker;
		history->ItemValueGet(CComBSTR(szID), &cMarker);
		if (cMarker.TypeGet() == ECVTEmpty)
			break;
		std::vector<BSTR> aNewIDs;
		std::vector<TConfigValue> aNewVals;
		for (size_t j = 0; j != nIDs; ++j)
		{
			CComBSTR bstrH(1+aIDs[j].Length());
			bstrH[0] = szID[0];
			wcscpy(bstrH.m_str+1, aIDs[j]);
			history->ItemValueGet(bstrH, &aVals[j]);
			if (aVals[j].TypeGet() != ECVTEmpty)
			{
				aNewIDs.push_back(aIDs[j]);
				aNewVals.push_back(aVals[j]);
			}
		}
		CComPtr<IConfig> older;
		templ->DuplicateCreate(&older);
		if (!aNewIDs.empty())
			older->ItemValuesSet(aNewIDs.size(), &(aNewIDs[0]), &(aNewVals[0]));
		 AddOpInfo(opAss, older, subOps, cloneBase, cloneId, opId);
	}
}

bool CStartPageBatchImageProcessor::GetSubOps(BSTR opId, IConfig* cfg, CSubOps& subOps, IConfig* cloneBase, BSTR cloneId)
{
	CConfigValue mainOp;
	cfg->ItemValueGet(opId, &mainOp);
	if (mainOp.TypeGet() != ECVTGUID)
		return false;
	CComPtr<IConfig> mainCfg;
	cfg->SubConfigGet(opId, &mainCfg);
	if (mainCfg == NULL)
		return false;
	if (IsEqualGUID(mainOp, __uuidof(DocumentOperationSequence)))
	{
		CComBSTR cCFGID_SEQ_STEPS(L"SeqSteps");
		CConfigValue steps;
		mainCfg->ItemValueGet(cCFGID_SEQ_STEPS, &steps);
		for (LONG i = 0; i < steps.operator LONG(); ++i)
		{
			OLECHAR sz[64];
			_swprintf(sz, L"SeqSteps\\%08x", i);
			CComPtr<IConfig> step;
			CComBSTR bstrSubstPos1(sz);
			mainCfg->SubConfigGet(bstrSubstPos1, &step);
			if (step == NULL)
				return false;
			CConfigValue type;
			step->ItemValueGet(CComBSTR(L"SeqType"), &type);
			if (type.TypeGet() != ECVTInteger)
				return false;

			CComBSTR cCFGID_SEQ_OPERATION;
			if (type.operator LONG() == 0) // operation
				cCFGID_SEQ_OPERATION = L"SeqOperation";
			else if (type.operator LONG() == 1) // transformation
				cCFGID_SEQ_OPERATION = L"SeqTransformation";
			else
				continue;

			CComBSTR newCloneId;
			if (cloneId)
			{
				newCloneId = cloneId;
				newCloneId += L"\\";
			}
			newCloneId += bstrSubstPos1;
			newCloneId += L"\\";
			newCloneId += cCFGID_SEQ_OPERATION;
			if (GetSubOps(cCFGID_SEQ_OPERATION, step, subOps, cloneBase, newCloneId))
				return true;
		}
		return false;
	}
	if (IsEqualGUID(mainOp, __uuidof(DocumentOperationShowConfig)) || IsEqualGUID(mainOp, __uuidof(DocumentTransformationShowConfig)))
	{
		CComBSTR cCFGID_CFG_OPERATION(IsEqualGUID(mainOp, __uuidof(DocumentOperationShowConfig)) ? L"CfgOperation" : L"CfgTransformation");

		CConfigValue realOpId;
		mainCfg->ItemValueGet(cCFGID_CFG_OPERATION, &realOpId);
		if (realOpId.TypeGet() != ECVTGUID)
			return false;
		CComPtr<IConfigDescriptor> opAss;
		RWCoCreateInstance(opAss, realOpId);
		if (opAss == NULL)
			return false;

		CComPtr<IConfig> realOpCfg;
		mainCfg->SubConfigGet(cCFGID_CFG_OPERATION, &realOpCfg);
		AddOpInfo(opAss, realOpCfg, subOps, cloneBase, cloneId, realOpId);

		CComBSTR cCFGID_CFG_HISTORY(L"History");
		CConfigValue cHistory;
		mainCfg->ItemValueGet(cCFGID_CFG_HISTORY, &cHistory);
		if (cHistory.TypeGet() == ECVTBool && cHistory)
		{
			CComPtr<IConfig> pHistoryCfg;
			mainCfg->SubConfigGet(cCFGID_CFG_HISTORY, &pHistoryCfg);
			if (pHistoryCfg)
				AddOpsFromHistory(realOpCfg, pHistoryCfg, opAss, subOps, cloneBase, cloneId, realOpId);
		}

		return true;
	}
	if (IsEqualGUID(mainOp, __uuidof(DocumentOperationExtractSubDocument)))
	{
		CComBSTR newCloneId;
		if (cloneId)
		{
			newCloneId = cloneId;
			newCloneId += L"\\";
		}
		newCloneId += L"SubOperation";
		return GetSubOps(CComBSTR(L"SubOperation"), mainCfg, subOps, cloneBase, newCloneId);
	}
	return false;
}

void CStartPageBatchImageProcessor::GetSubOps(IConfig* cfg, CSubOps& subOps)
{
	CConfigValue mainOp;
	CComBSTR cCFGID_OPERATION(CFGID_OPERATION);
	cfg->ItemValueGet(cCFGID_OPERATION, &mainOp);
	if (mainOp.TypeGet() != ECVTGUID)
		return;
	CComPtr<IConfig> mainCfg;
	cfg->SubConfigGet(cCFGID_OPERATION, &mainCfg);
	if (mainCfg == NULL)
		return;
	GetSubOps(cCFGID_OPERATION, cfg, subOps, mainCfg, NULL);
}

LRESULT CStartPageBatchImageProcessor::OnDropdownOptions(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
{
	NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);

	POINT ptBtn = {pTB->rcButton.left, pTB->rcButton.bottom};
	m_wndStartStop.ClientToScreen(&ptBtn);

	if (pTB->iItem == ID_BP_OPTIONS)
	{
		struct SCtxMap
		{
			wchar_t const* name;
			UINT icon;
			bool (CStartPageBatchImageProcessor::*handler)();
			bool advanced;
		};
		static SCtxMap const s_ctxMap[] =
		{
			{ L"[0409]Create new batch operation[0405]Vytvořit novou dávkovou operaci", 3, &CStartPageBatchImageProcessor::OnOperationCreate, true },
			{ L"[0409]Duplicate batch operation[0405]Duplikovat dávkovou operaci", 4, &CStartPageBatchImageProcessor::OnOperationDuplicate, true },
			{ L"[0409]Delete batch operation[0405]Smazat dávkovou operaci", 5, &CStartPageBatchImageProcessor::OnOperationDelete, false },
			{ L"[0409]Configure batch operation[0405]Konfigurovat dávkovou operaci", 6, &CStartPageBatchImageProcessor::OnOperationConfigure, true },
			{ NULL, I_IMAGENONE, NULL, false },
			{ L"[0409]Import batch operation[0405]Importovat dávkovou operaci", 7, &CStartPageBatchImageProcessor::OnOperationImport, false },
			{ L"[0409]Export batch operation[0405]Exportovat dávkovou operaci", 8, &CStartPageBatchImageProcessor::OnOperationExport, false },
		};

		Reset(m_cToolbarImages);

		CMenu cMenu;
		cMenu.CreatePopupMenu();

		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		CConfigValue cAdv;
		static GUID const tGlobalConfigMainFrame = {0x2e85563c, 0x4ff0, 0x4820, {0xa8, 0xba, 0x1b, 0x47, 0x63, 0xab, 0xcc, 0x1c}};
		if (pGCM) pGCM->GetValue(tGlobalConfigMainFrame, CComBSTR(L"LayoutCommands"), &cAdv);
		bool advanced = cAdv.TypeGet() == ECVTBool && cAdv.operator bool();

		for (size_t i = 0; i < itemsof(s_ctxMap); ++i)
		{
			if (s_ctxMap[i].name)
			{
				// TODO: re-enable when simplified GUI for operation addition is added
				//if (s_ctxMap[i].advanced && !advanced)
				//	continue;
				CComBSTR bstr;
				CMultiLanguageString::GetLocalized(s_ctxMap[i].name, m_tLocaleID, &bstr);
				AddItem(cMenu, i+1, bstr.m_str, s_ctxMap[i].icon);
			}
			else
			{
				cMenu.AppendMenu(MFT_SEPARATOR);
			}
		}

		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		tPMParams.rcExclude = pTB->rcButton;
		::MapWindowPoints(pTB->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&tPMParams.rcExclude), 2);
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, ptBtn.x, ptBtn.y, m_hWnd, &tPMParams);
		if (nSelection != 0)
		{
			CWaitCursor cWait;
			(this->*s_ctxMap[nSelection-1].handler)();
		}
		return 0;
	}

	if (pTB->iItem == ID_BP_SELECT)
	{
		Reset(m_cOperationImages);

		CMenu cMenu;
		cMenu.CreatePopupMenu();

		CConfigValue cCur;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENT), &cCur);

		CConfigValue cOps;
		CComBSTR cCFGID_OPERATIONS(CFGID_OPERATIONS);
		m_pConfig->ItemValueGet(cCFGID_OPERATIONS, &cOps);
		for (LONG i = 0; i < cOps.operator LONG(); ++i)
		{
			OLECHAR szTmp[64];
			swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, i);
			CComBSTR bstrTmp(szTmp);
			CComPtr<IConfig> p1Op;
			m_pConfig->SubConfigGet(bstrTmp, &p1Op);
			CConfigValue cName;
			p1Op->ItemValueGet(CComBSTR(CFGID_NAME), &cName);
			CConfigValue cIconID;
			p1Op->ItemValueGet(CComBSTR(CFGID_ICONID), &cIconID);
			int nIcon = 0;
			if (!IsEqualGUID(cIconID, GUID_NULL))
			{
				CIconMap::const_iterator j = m_cOperationIconMap.find(cIconID.operator const GUID &());
				if (j != m_cOperationIconMap.end())
				{
					nIcon = j->second;
				}
				else
				{
					CComPtr<IDesignerFrameIcons> pIcMgr;
					RWCoCreateInstance(pIcMgr, __uuidof(DesignerFrameIconsManager));
					HICON hIcon = NULL;
					pIcMgr->GetIcon(cIconID, XPGUI::GetSmallIconSize(), &hIcon);
					if (hIcon)
					{
						m_cOperationImages.AddIcon(hIcon);
						DestroyIcon(hIcon);
						nIcon = m_cOperationIconMap[cIconID.operator const GUID &()] = m_cOperationImages.GetImageCount()-1;
					}
				}
			}
			CComBSTR bstrLoc;
			CMultiLanguageString::GetLocalized(cName.operator BSTR(), m_tLocaleID, &bstrLoc);
			AddItem(cMenu, 1+i, bstrLoc.m_str, nIcon, cCur.operator LONG() == i ? MFT_RADIOCHECK|MFS_CHECKED : MFT_RADIOCHECK);
		}

		if (cOps.operator LONG())
		{
			cMenu.AppendMenu(MFT_SEPARATOR);
		}
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(L"[0409]Get more batch operations[0405]Získat další dávkové operace", m_tLocaleID, &bstr);
			AddItem(cMenu, 0x7fffffff, bstr.m_str, I_IMAGENONE);
		}

		// submenus
		std::map<LONG, CSubOps> subOpsMap;
		for (LONG i = 0; i < cOps.operator LONG(); ++i)
		{
			OLECHAR szTmp[64];
			swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, i);
			CComBSTR bstrTmp(szTmp);
			CComPtr<IConfig> p1Op;
			m_pConfig->SubConfigGet(bstrTmp, &p1Op);
			CSubOps subOps;
			GetSubOps(p1Op, subOps);
			if (subOps.empty())
			{
				//AddItem(cMenu, 1+i+1*4096, L"", I_IMAGENONE);
				AddItem(1+i+1*256, L"", I_IMAGENONE);
				cMenu.AppendMenu(MF_POPUP|MFT_OWNERDRAW|MFS_DISABLED|(i == 0 ? MF_MENUBREAK : 0), 1+i+1*256, L"");
				continue;
			}
			CMenuHandle subMenu;
			subMenu.CreatePopupMenu();
			LONG subsel = 0;
			if (cCur.operator LONG() == i)
			{
				CConfigValue cSub;
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENTSUB), &cSub);
				subsel = cSub;
			}
			for (CSubOps::const_iterator j = subOps.begin(); j != subOps.end(); ++j)
			{
				CComBSTR bstrLoc;
				if (j->name) j->name->GetLocalized(m_tLocaleID, &bstrLoc); else bstrLoc = L"";
				AddItem(subMenu, 1+i+(j-subOps.begin()+1)*256, bstrLoc, I_IMAGENONE, j->hash == subsel ? MFT_RADIOCHECK|MFS_CHECKED : MFT_RADIOCHECK);
			}
			AddItem(reinterpret_cast<UINT>(subMenu.m_hMenu), L"", I_IMAGENONE);
			cMenu.AppendMenu(MF_POPUP|MFT_OWNERDRAW|(i == 0 ? MF_MENUBREAK : 0), reinterpret_cast<UINT>(subMenu.m_hMenu), L"");
			std::swap(subOpsMap[i], subOps);
		}

		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		tPMParams.rcExclude = pTB->rcButton;
		::MapWindowPoints(pTB->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&tPMParams.rcExclude), 2);
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD|TPM_VERPOSANIMATION, ptBtn.x, ptBtn.y, m_hWnd, &tPMParams);
		if (nSelection != 0)
		{
			if (nSelection == 0x7fffffff)
			{
				::ShellExecute(0, _T("open"), _T("http://www.rw-designer.com/batch-operation-archive"), 0, 0, SW_SHOWNORMAL);
			}
			else
			{
				LONG nSel = (nSelection-1)&0xff;
				LONG nSelSub = nSelection>>8;
				if (nSelSub)
				{
					CSubOps const& c = subOpsMap[nSel];
					if (ULONG(nSelSub-1) < c.size())
						nSelSub = c[nSelSub-1].hash;
					else
						nSelSub = 0;
				}
				CConfigValue cCur;
				CComBSTR cCFGID_CURRENT(CFGID_CURRENT);
				m_pConfig->ItemValueGet(cCFGID_CURRENT, &cCur);
				LONG nCur = cCur;
				CConfigValue cCurSub;
				CComBSTR cCFGID_CURRENTSUB(CFGID_CURRENTSUB);
				m_pConfig->ItemValueGet(cCFGID_CURRENTSUB, &cCurSub);
				LONG nCurSub = cCurSub;
				if (nSel != nCur || nSelSub != nCurSub)
				{
					cCur = nSel;
					cCurSub = nSelSub;
					BSTR ids[2] = {cCFGID_CURRENT, cCFGID_CURRENTSUB};
					TConfigValue vals[2] = {cCur, cCurSub};
					m_bEnableUpdates = false;
					m_pConfig->ItemValuesSet(2, ids, vals);
					m_bEnableUpdates = true;
					BOOL b;
					OnConfigChanged(0, 0, 0, b);
				}
			}
		}
	}

	return 0;
}

LRESULT CStartPageBatchImageProcessor::OnDrawOperation(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	NMTBCUSTOMDRAW* pTBCD = reinterpret_cast<NMTBCUSTOMDRAW*>(a_pNMHdr);
	
	if (pTBCD->nmcd.dwDrawStage == CDDS_PREPAINT)
		return CDRF_NOTIFYITEMDRAW;

	if (pTBCD->nmcd.dwItemSpec != ID_BP_SELECT)
		return CDRF_DODEFAULT;

	if (pTBCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
		return m_hTheme ? CDRF_SKIPDEFAULT|CDRF_NOTIFYPOSTPAINT : CDRF_NOTIFYPOSTPAINT;
	if (pTBCD->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
	{
		int const buttonWidth = ::GetSystemMetrics(SM_CXHSCROLL);
		if (m_hTheme)
		{
			DrawThemeBackground(pTBCD->nmcd.hdc, CP_READONLY, (pTBCD->nmcd.uItemState&(CDIS_CHECKED|CDIS_SELECTED)) ? CBRO_PRESSED : (pTBCD->nmcd.uItemState&(CDIS_HOT)) ? CBRO_HOT : CBRO_NORMAL, &pTBCD->nmcd.rc, NULL);
			if (XPGUI::IsVista())
			{
				RECT const rc = {pTBCD->nmcd.rc.right-buttonWidth, pTBCD->nmcd.rc.top, pTBCD->nmcd.rc.right, pTBCD->nmcd.rc.bottom};
				DrawThemeBackground(pTBCD->nmcd.hdc, CP_DROPDOWNBUTTONRIGHT, /*(pTBCD->nmcd.uItemState&(CDIS_CHECKED|CDIS_SELECTED)) ? CBXSR_PRESSED : (pTBCD->nmcd.uItemState&(CDIS_HOT)) ? CBXSR_HOT :*/ CBXSR_NORMAL, &rc, NULL);
			}
			else
			{
				int const borderX = ::GetSystemMetrics(SM_CXBORDER);
				int const borderY = ::GetSystemMetrics(SM_CYBORDER);
				RECT const rc = {pTBCD->nmcd.rc.right-borderX-buttonWidth, pTBCD->nmcd.rc.top+borderY, pTBCD->nmcd.rc.right-borderX, pTBCD->nmcd.rc.bottom-borderY};
				DrawThemeBackground(pTBCD->nmcd.hdc, CP_DROPDOWNBUTTON, (pTBCD->nmcd.uItemState&(CDIS_CHECKED|CDIS_SELECTED)) ? CBXS_PRESSED : (pTBCD->nmcd.uItemState&(CDIS_HOT)) ? CBXS_HOT : CBXS_NORMAL, &rc, NULL);
			}
		}

		CComBSTR bstrName;
		CComBSTR bstrDesc;
		if (m_activeName) m_activeName->GetLocalized(m_tLocaleID, &bstrName);
		if (m_activeSubtitle && bstrName.m_str && bstrName[0])
		{
			CComBSTR bstrSubtitle;
			m_activeSubtitle->GetLocalized(m_tLocaleID, &bstrSubtitle);
			bstrName += L" - ";
			bstrName += bstrSubtitle;
		}
		if (bstrName.m_str == NULL || bstrName[0] == L'\0')
		{
			CMultiLanguageString::GetLocalized(L"[0409]No batch operation installed[0405]Není nainstalována žádná dávková operace", m_tLocaleID, &bstrName);
			CMultiLanguageString::GetLocalized(L"[0409]Download batch operations from an online archive and install them.[0405]Stáhněte si dávkové operace z online archívu a nainstalujte je.", m_tLocaleID, &bstrDesc);
		}
		else
		{
			if (m_activeDesc) m_activeDesc->GetLocalized(m_tLocaleID, &bstrDesc);
			if (bstrDesc.m_str == NULL || bstrDesc[0] == L'\0') bstrDesc = L"";
		}

		int nIcon = 0;
		int const iconSize = XPGUI::GetSmallIconSize()*3;
		if (!IsEqualGUID(m_activeIconId, GUID_NULL))
		{
			CIconMap::const_iterator j = m_cLargeOperationIconMap.find(m_activeIconId);
			if (j != m_cLargeOperationIconMap.end())
			{
				nIcon = j->second;
			}
			else
			{
				CComPtr<IDesignerFrameIcons> pIcMgr;
				RWCoCreateInstance(pIcMgr, __uuidof(DesignerFrameIconsManager));
				HICON hIcon = NULL;
				pIcMgr->GetIcon(m_activeIconId, iconSize, &hIcon);
				if (hIcon)
				{
					m_cLargeOperationImages.AddIcon(hIcon);
					DestroyIcon(hIcon);
					nIcon = m_cLargeOperationIconMap[m_activeIconId] = m_cLargeOperationImages.GetImageCount()-1;
				}
			}
		}

		RECT rcItem = pTBCD->nmcd.rc;

		RECT rcDesc = rcItem;
		HFONT hFont = m_wndStartStop.GetFont();

		HGDIOBJ hPrevFont = SelectObject(pTBCD->nmcd.hdc, hFont);
		DrawText(pTBCD->nmcd.hdc, bstrDesc, -1, &rcDesc, DT_SINGLELINE|DT_CALCRECT);
		ULONG nDescY = rcDesc.bottom-rcDesc.top;

		SelectObject(pTBCD->nmcd.hdc, m_cLargeFont);
		RECT rcText = rcItem;
		DrawText(pTBCD->nmcd.hdc, bstrName, -1, &rcText, DT_SINGLELINE|DT_CALCRECT);
		ULONG nTextY = rcText.bottom-rcText.top;
		ULONG nItemY = rcItem.bottom-rcItem.top;
		ULONG nSpareY = rcItem.bottom-rcItem.top-nDescY-nTextY-(nTextY>>1);

		int iPrevMode = SetBkMode(pTBCD->nmcd.hdc, TRANSPARENT);

		rcText.left += iconSize + 8*m_scale;
		rcText.right = rcItem.right-buttonWidth-4*m_scale;
		rcText.top = rcItem.top + ((nItemY*3/5)>>1) - (nTextY>>1);
		rcText.bottom = rcText.top+nTextY;
		DrawText(pTBCD->nmcd.hdc, bstrName, -1, &rcText, DT_SINGLELINE|DT_LEFT|DT_END_ELLIPSIS);

		SelectObject(pTBCD->nmcd.hdc, hFont);
		rcDesc.top = rcItem.top + (nItemY*3/5);
		rcDesc.top += ((rcItem.bottom-rcDesc.top)>>1) - (nTextY>>1);
		rcDesc.bottom = rcDesc.top+nTextY;
		rcDesc.left = rcText.left;
		rcDesc.right = rcText.right;
		COLORREF clr1 = GetSysColor(COLOR_3DFACE);
		COLORREF clr2 = GetSysColor(COLOR_BTNTEXT);
		COLORREF clrMix = RGB((GetRValue(clr1)*3+GetRValue(clr2)*5)>>3, (GetGValue(clr1)*3+GetGValue(clr2)*5)>>3, (GetBValue(clr1)*3+GetBValue(clr2)*5)>>3);
		COLORREF prev = SetTextColor(pTBCD->nmcd.hdc, clrMix);
		DrawText(pTBCD->nmcd.hdc, bstrDesc, -1, &rcDesc, DT_SINGLELINE|DT_LEFT|DT_END_ELLIPSIS);
		SetTextColor(pTBCD->nmcd.hdc, prev);

		SelectObject(pTBCD->nmcd.hdc, hPrevFont);
		SetBkMode(pTBCD->nmcd.hdc, iPrevMode);

		ImageList_Draw(m_cLargeOperationImages, nIcon, pTBCD->nmcd.hdc, 4*m_scale, (rcItem.bottom+rcItem.top-iconSize)>>1, ILD_NORMAL);

			//RECT rcItem = pTBCD->nmcd.rc;
			//ULONG nOff = pTBCD->nmcd.uItemState&CDIS_CHECKED ? m_nTBGaps>>1 : m_nTBGaps;
			//rcItem.left += nOff;
			//rcItem.right -= m_nTBGaps-nOff;

		return m_hTheme ? CDRF_SKIPDEFAULT : CDRF_DODEFAULT;
	}
	return CDRF_DODEFAULT;
}

LRESULT CStartPageBatchImageProcessor::OnClickedHelp(WORD, WORD, HWND, BOOL&)
{
	::ShellExecute(0, _T("open"), _T("http://www.rw-designer.com/batch-processing"), 0, 0, SW_SHOWNORMAL);
	return 0;
}


#include <PrintfLocalizedString.h>
#include <MultilanguageString.h>

STDMETHODIMP CStartPageBatchImageProcessor::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
{
	if (!IsWindow() || a_pFileNames == NULL)
		return E_FAIL;
	CWindow wndExp(GetDlgItem(IDC_BP_OUTPUT));
	if (wndExp.IsWindow())
	{
		RECT rc;
		wndExp.GetWindowRect(&rc);
		if (a_pt.x >= rc.left && a_pt.x < rc.right && a_pt.y >= rc.top && a_pt.y < rc.bottom)
		{
			return S_OK; // dropped on explorer window - let it be
		}
	}
	if (a_pdwEffect)
		*a_pdwEffect = DROPEFFECT_COPY;
	if (a_ppFeedback)
	{
		*a_ppFeedback = NULL;
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> p = pStr;
		CConfigValue cCurrent;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENT), &cCurrent);
		OLECHAR szTmp[64];
		swprintf(szTmp, L"%s\\%08x\\%s", CFGID_OPERATIONS, cCurrent.operator LONG(), CFGID_NAME);
		CComBSTR bstrOp(szTmp);
		CConfigValue cOpName;
		m_pConfig->ItemValueGet(bstrOp, &cOpName);
		CComPtr<ILocalizedString> pOpName;
		pOpName.Attach(new CMultiLanguageString(cOpName.Detach().bstrVal));
		pStr->Init(CMultiLanguageString::GetAuto(L"[0409]Process the file(s) using the operation \"%s\".[0405]Zpracovat soubor(y) pomocí operace \"%s\"."), pOpName);
		*a_ppFeedback = p.Detach();
	}
	return S_OK;
}

STDMETHODIMP CStartPageBatchImageProcessor::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt)
{
	if (!IsWindow())
		return S_FALSE;
	CWindow wndExp(GetDlgItem(IDC_BP_OUTPUT));
	if (wndExp.IsWindow())
	{
		RECT rc;
		wndExp.GetWindowRect(&rc);
		if (a_pt.x >= rc.left && a_pt.x < rc.right && a_pt.y >= rc.top && a_pt.y < rc.bottom)
			return S_OK; // dropped on explorer window - let it be
	}

	if (a_pDataObj == NULL && a_pFileNames == NULL)
		return E_RW_CANCELLEDBYUSER; // cancelled d'n'd operation

	if (IsEqualGUID(m_activeOpId, GUID_NULL))
	{
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		if (pAI)
		{
			CComPtr<ILocalizedString> name;
			pAI->Name(&name);
			CComBSTR bstrName;
			if (name) name->GetLocalized(m_tLocaleID, &bstrName);
			CComBSTR bstrMsg;
			CMultiLanguageString::GetLocalized(L"[0409]There is no batch operation installed. Click the \"Help\" button to learn more about batch processing.[0405]Není nainstalována žádná dávková operace. Klikněte na tlačítko \"Nápověda\", abyste se dozvěděli více o dávkovém zpracování.", m_tLocaleID, &bstrMsg);
			MessageBox(bstrMsg, bstrName, MB_OK|MB_ICONINFORMATION);
		}
		return E_RW_CANCELLEDBYUSER;
	}

	ULONG nItems = 0;
	a_pFileNames->Size(&nItems);
	ObjectLock cLock(this);
	SPathInfo sInfo;

	sInfo.tOperationID = m_activeOpId;
	sInfo.pOperationCfg = m_activeOpCfg;
	sInfo.tBuilderID = m_activeBuilder;
	sInfo.jobId = m_jobId;
	sInfo.strOutput = m_activeOutput;

	for (ULONG i = 0; i < nItems; ++i)
	{
		CComBSTR bstr;
		a_pFileNames->Get(i, &bstr);
		COLE2CT str(bstr.m_str);
		sInfo.strPath = str;
		LPCOLESTR psz1 = wcsrchr(bstr, L'\\');
		LPCOLESTR psz2 = wcsrchr(bstr, L'/');
		if (psz1 < psz2) psz1 = psz2;
		sInfo.nRootLength = psz1 ? psz1-bstr.m_str : 0;
		m_cPaths.push_back(sInfo);
	}
	SetEvent(m_hWakeWorker);
	++m_jobId;
	return S_OK;
}

STDMETHODIMP CStartPageBatchImageProcessor::ForwardOK()
{
	try
	{
		if (IsEqualGUID(GUID_NULL, m_activeOpId))
		{
			CComPtr<IApplicationInfo> pAI;
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			if (pAI)
			{
				CComPtr<ILocalizedString> name;
				pAI->Name(&name);
				CComBSTR bstrName;
				if (name) name->GetLocalized(m_tLocaleID, &bstrName);
				CComBSTR bstrMsg;
				CMultiLanguageString::GetLocalized(L"[0409]There is no batch operation installed. Click the \"Help\" button to learn more about batch processing.[0405]Není nainstalována žádná dávková operace. Klikněte na tlačítko \"Nápověda\", abyste se dozvěděli více o dávkovém zpracování.", m_tLocaleID, &bstrMsg);
				MessageBox(bstrMsg, bstrName, MB_OK|MB_ICONINFORMATION);
			}
			return S_FALSE;
		}
		CComPtr<IEnumUnknowns> pFlts;
		m_pBrowseWnd->FiltersCreate(&pFlts);
		ULONG nFlts = 0;
		if (pFlts) pFlts->Size(&nFlts);
		if (nFlts == 0)
		{
			CComPtr<IApplicationInfo> pAI;
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			if (pAI)
			{
				CComPtr<ILocalizedString> name;
				pAI->Name(&name);
				CComBSTR bstrName;
				if (name) name->GetLocalized(m_tLocaleID, &bstrName);
				CComBSTR bstrMsg;
				CMultiLanguageString::GetLocalized(L"[0409]Select files or folders in the \"Source files\" panel before clicking the \"Start\" icon. Alternatively, drag and drop files or folders from any file manager onto the \"Start\" button to process them.[0405]Vyberte soubory nebo složky v panelu \"Zdrojové soubory\" a pak stiskněte tlačítko \"Start\". Další možností je přetáhnout sobory nebo složky z jakéhokoli manažeru souborů přímo na ikonu \"Start\".", m_tLocaleID, &bstrMsg);
				MessageBox(bstrMsg, bstrName, MB_OK|MB_ICONINFORMATION);
			}
			return S_FALSE;
		}
		for (ULONG i = 0; i < nFlts; ++i)
		{
			CComPtr<IStorageFilter> pFlt;
			pFlts->Get(i, __uuidof(IStorageFilter), reinterpret_cast<void**>(&pFlt));
			CComBSTR bstrPath;
			if (pFlt && SUCCEEDED(pFlt->ToText(NULL, &bstrPath)) && bstrPath.Length())
			{
				ObjectLock cLock(this);
				SPathInfo sInfo;

				sInfo.tOperationID = m_activeOpId;
				sInfo.pOperationCfg = m_activeOpCfg;
				sInfo.tBuilderID = m_activeBuilder;
				sInfo.jobId = m_jobId;
				sInfo.strOutput = m_activeOutput;

				COLE2CT str(bstrPath.m_str);
				sInfo.strPath = str;
				LPCOLESTR psz1 = wcsrchr(bstrPath, L'\\');
				LPCOLESTR psz2 = wcsrchr(bstrPath, L'/');
				if (psz1 < psz2) psz1 = psz2;
				sInfo.nRootLength = psz1 ? psz1-bstrPath.m_str : 0;
				m_cPaths.push_back(sInfo);
				SetEvent(m_hWakeWorker);
			}
		}
		++m_jobId;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStartPageBatchImageProcessor::DefaultCommand(ILocalizedString** a_ppName, ILocalizedString** UNREF(a_ppDesc), GUID* UNREF(a_pIconID))
{
	try
	{
		if (a_ppName)
		{
			*a_ppName = NULL;
			*a_ppName = new CMultiLanguageString(L"[0409]Process file(s)[0405]Zpracovat soubor(y)");
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

unsigned __stdcall CStartPageBatchImageProcessor::WorkerProc(void* a_pThis)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	try
	{
		CStartPageBatchImageProcessor* pThis = reinterpret_cast<CStartPageBatchImageProcessor*>(a_pThis);

		CComPtr<IInputManager> pInMgr;
		RWCoCreateInstance(pInMgr, __uuidof(InputManager));
		CComObject<CBatchOperationContext>* pBOC = NULL;
		CComObject<CBatchOperationContext>::CreateInstance(&pBOC);
		CComPtr<IOperationContext> pBOCTmp = pBOC;
		ULONG nIndex = 0;
		ULONG nRemaining = 0;
		int jobId = -1;

		while (true)
		{
			WaitForSingleObject(pThis->m_hWakeWorker, INFINITE);
			if (pThis->m_bStopWorker)
				break;
			SPathInfo sPathInfo;
			CConfigValue cOpID;
			CComPtr<IConfig> pOpCfg;
			GUID tBuilder = GUID_NULL;
			CComPtr<IDocumentBuilder> pBuilder;
			// fetch one file to process
			{
				ObjectLock cLock(pThis);
				bool remainingInvalid = false;
				while (pThis->m_cPaths.size() && sPathInfo.strPath.empty())
				{
					sPathInfo = pThis->m_cPaths.front();
					pThis->m_cPaths.pop_front();
					DWORD dw = GetFileAttributes(sPathInfo.strPath.c_str());
					if (dw != INVALID_FILE_ATTRIBUTES && (dw&FILE_ATTRIBUTE_DIRECTORY))
					{
						// enum files in directory
						TCHAR szCurrMask[MAX_PATH+16];
						_tcscpy(szCurrMask, sPathInfo.strPath.c_str());
						sPathInfo.strPath.clear();
						size_t i =_tcslen(szCurrMask);
						_tcscat(szCurrMask, szCurrMask[i-1] != _T('\\') ? _T("\\*.*") : _T("*.*"));
						std::vector<std::tstring> cFiles;
						std::vector<std::tstring> cDirs;
						WIN32_FIND_DATA w32fd;
						HANDLE hFindData = FindFirstFile(szCurrMask, &w32fd);
						if (hFindData != INVALID_HANDLE_VALUE)
						{
							do
							{
								TCHAR szSub[MAX_PATH];
								_tcscpy(szSub, szCurrMask);
								size_t i =_tcslen(szSub);
								if ((i+_tcslen(w32fd.cFileName)-3) < MAX_PATH)
								{
									_tcscpy(szSub+i-3, w32fd.cFileName);
									if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
									{
										if (_tcscmp(w32fd.cFileName, _T(".")) != 0 && _tcscmp(w32fd.cFileName, _T("..")) != 0)
											cDirs.push_back(szSub);
									}
									else
									{
										cFiles.push_back(szSub);
									}
								}
							} while (FindNextFile(hFindData, &w32fd));

							FindClose(hFindData);
						}
						for (std::vector<std::tstring>::const_reverse_iterator i = cFiles.rbegin(); i != cFiles.rend(); ++i)
						{
							SPathInfo s = sPathInfo;
							s.strPath = *i;
							pThis->m_cPaths.push_front(s);
						}
						for (std::vector<std::tstring>::const_reverse_iterator i = cDirs.rbegin(); i != cDirs.rend(); ++i)
						{
							SPathInfo s = sPathInfo;
							s.strPath = *i;
							pThis->m_cPaths.push_front(s);
						}
						remainingInvalid = true;
						continue;
					}
					SetEvent(pThis->m_hWakeWorker);
					cOpID = sPathInfo.tOperationID;
					pOpCfg = sPathInfo.pOperationCfg;
				}
				if (jobId != sPathInfo.jobId)
				{
					nIndex = 0;
					jobId = sPathInfo.jobId;
					pBOC->EndJob(); // starting a new job
					remainingInvalid = true;
					if (!IsEqualGUID(tBuilder, sPathInfo.tBuilderID))
					{
						GUID tBuilder = sPathInfo.tBuilderID;
						pBuilder = NULL;
						if (!IsEqualGUID(tBuilder, GUID_NULL))
							RWCoCreateInstance(pBuilder, tBuilder);
					}
				}
				if (remainingInvalid)
				{
					nRemaining = 0;
					CPaths::const_iterator const end = pThis->m_cPaths.end();
					for (CPaths::const_iterator i = pThis->m_cPaths.begin(); i != end && i->jobId == sPathInfo.jobId; ++i)
						++nRemaining;
				}
			}
			::SendMessage(pThis->m_hWnd, WM_UPDATESTATUS, !sPathInfo.strPath.empty(), sPathInfo.strPath.empty() ? 0xffff : (0xffff*(nIndex+1))/(nIndex+nRemaining+2));
			if (!sPathInfo.strPath.empty())
			{
				HRESULT hRes = S_OK;
				CComBSTR bstrLogMessage;
				try
				{
					CStorageFilter cInPath(sPathInfo.strPath.c_str());
					CComPtr<IDocument> pDoc;
					hRes = pInMgr->DocumentCreateEx(pBuilder, cInPath, NULL, &pDoc);
					if (pDoc == NULL)
					{
						CComPtr<ILocalizedString> typeName;
						pBuilder->TypeName(&typeName);
						CComBSTR bstrTypeName;
						if (typeName) typeName->GetLocalized(pThis->m_tLocaleID, &bstrTypeName);
						if (bstrTypeName)
						{
							CComBSTR bstrTempl;
							CMultiLanguageString::GetLocalized(L"[0409]Failed to open file as \"%s\".[0405]Nepodařilo se otevřít soubor jako typ \"%s\".", pThis->m_tLocaleID, &bstrTempl);
							wchar_t szTmp[256] = L"";
							_snwprintf(szTmp, 256, bstrTempl, bstrTypeName);
							bstrLogMessage = szTmp;
						}
						else
						{
							CMultiLanguageString::GetLocalized(L"[0409]Failed to open file.[0405]Nepodařilo se otevřít soubor.", pThis->m_tLocaleID, &bstrLogMessage);
						}
					}
					else
					{
						CComQIPtr<IDocumentUndo> pUndo(pDoc);
						if (pUndo) pUndo->UndoModeSet(EUMDisabled);
						pBOC->Step(nIndex++, nRemaining, sPathInfo.nRootLength, sPathInfo.strOutput);
						hRes = pThis->m_pOpMgr->Activate(pThis->m_pOpMgr, pDoc, cOpID, pOpCfg, pBOCTmp, pThis->m_hWnd, pThis->m_tLocaleID);
					}
				}
				catch (...)
				{
					bstrLogMessage.Empty();
					CMultiLanguageString::GetLocalized(L"[0409]Internal error occured.[0405]Došlo k vnitřní chybě.", pThis->m_tLocaleID, &bstrLogMessage);
				}
				if (pBOC->M_ErrorMessage())
				{
					bstrLogMessage.Empty();
					pBOC->M_ErrorMessage()->GetLocalized(pThis->m_tLocaleID, &bstrLogMessage);
				}
				if (bstrLogMessage == NULL || bstrLogMessage[0] == L'\0')
				{
					if (SUCCEEDED(hRes))
					{
						CMultiLanguageString::GetLocalized(L"[0409]Processing succeeded.[0405]Zpracování úspěšné.", pThis->m_tLocaleID, &bstrLogMessage);
					}
					else
					{
						CComBSTR bstrTempl;
						CMultiLanguageString::GetLocalized(L"[0409]Processing failed (0x%08x).[0405]Zpracování selhalo (0x%08x).", pThis->m_tLocaleID, &bstrTempl);
						wchar_t szTmp[256] = L"";
						_snwprintf(szTmp, 256, bstrTempl, hRes);
						bstrLogMessage = szTmp;
					}
				}
				::SendMessage(pThis->m_hWnd, WM_ADDLOGMESSAGE, reinterpret_cast<WPARAM>(sPathInfo.strPath.c_str()), reinterpret_cast<LPARAM>(static_cast<TCHAR const*>(bstrLogMessage)));
			}
		}
	}
	catch (...)
	{
	}
	CoUninitialize();
	return 0;
}

STDMETHODIMP CStartPageBatchImageProcessor::GetWindow(HWND* a_phWnd)
{
	CHECKPOINTER(a_phWnd);

	*a_phWnd = m_hWnd;

	return S_OK; 
}

STDMETHODIMP CStartPageBatchImageProcessor::ContextSensitiveHelp(BOOL a_fEnterMode)
{
	return E_NOTIMPL;
}


STDMETHODIMP CStartPageBatchImageProcessor::InsertMenusSB(HMENU a_hMenuShared, LPOLEMENUGROUPWIDTHS a_pMenuWidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStartPageBatchImageProcessor::SetMenuSB(HMENU a_hMenuShared, HOLEMENU a_hOleMenuReserved, HWND a_hWndActiveObject)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStartPageBatchImageProcessor::RemoveMenusSB(HMENU a_hMenuShared)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStartPageBatchImageProcessor::SetStatusTextSB(LPCOLESTR a_pszStatusText)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStartPageBatchImageProcessor::EnableModelessSB(BOOL a_fEnable)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStartPageBatchImageProcessor::BrowseObject(LPCITEMIDLIST a_pIDL, UINT a_wFlags)
{
	return S_FALSE;
}

STDMETHODIMP CStartPageBatchImageProcessor::GetViewStateStream(DWORD a_dwGrfMode, LPSTREAM *a_ppStrm)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStartPageBatchImageProcessor::OnViewWindowActive(IShellView* a_pShellView)
{
	return S_OK;
}

STDMETHODIMP CStartPageBatchImageProcessor::SetToolbarItems(LPTBBUTTON a_pButtons, UINT a_nButtons, UINT a_uFlags)
{
	return S_OK;
}

STDMETHODIMP CStartPageBatchImageProcessor::TranslateAcceleratorSB(LPMSG a_pMsg, WORD a_wID)
{
	return S_FALSE;
}

STDMETHODIMP CStartPageBatchImageProcessor::QueryActiveShellView(IShellView** a_ppShellView)
{
	CHECKPOINTER(a_ppShellView);

	if (m_pActiveShellView != NULL)
	{
		(*a_ppShellView = m_pActiveShellView)->AddRef();
	}
	else
	{
		*a_ppShellView = NULL;
	}

	return S_OK; 
}

STDMETHODIMP CStartPageBatchImageProcessor::GetControlWindow(UINT a_uID, HWND* a_phWnd)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStartPageBatchImageProcessor::SendControlMsg(UINT a_uID, UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, LRESULT* a_pResult)
{
	return E_NOTIMPL;
}

#define GetPIDLFolder(pida) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define GetPIDLItem(pida, i) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

LRESULT CStartPageBatchImageProcessor::OnGetIShellBrowser(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	//AddRef(); //not addrefed
	SetWindowLongPtr(DWLP_MSGRESULT, (LONG_PTR)(IShellBrowser*)this); //use this if dialog
	return (LRESULT)(IShellBrowser*)this;
}

LRESULT CStartPageBatchImageProcessor::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL b;
	m_wndHeader1.OnSettingChange(uMsg, wParam, lParam, b);
	m_wndHeader2.OnSettingChange(uMsg, wParam, lParam, b);
	m_wndHeader3.OnSettingChange(uMsg, wParam, lParam, b);
	return 0;
}

void CStartPageBatchImageProcessor::UpdateActiveOperationCache()
{
	m_activeOpId = GUID_NULL;
	m_activeOpCfg = NULL;
	m_activeIconId = GUID_NULL;
	m_activeName = NULL;
	m_activeSubtitle = NULL;
	m_activeDesc = NULL;
	m_activeBuilder = GUID_NULL;
	m_activeOutput.Empty();

	CConfigValue current;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENT), &current);
	CConfigValue currentSub;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_CURRENTSUB), &currentSub);

	CConfigValue count;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATIONS), &count);

	if (count.operator LONG() == 0 || current.operator LONG() < 0 || current.operator LONG() >= count.operator LONG())
		return;

	OLECHAR szTmp[64];
	swprintf(szTmp, L"%s\\%08x", CFGID_OPERATIONS, current.operator LONG());
	CComPtr<IConfig> p1Op;
	m_pConfig->SubConfigGet(CComBSTR(szTmp), &p1Op);
	if (p1Op == NULL)
		return;

	CComBSTR cCFGID_OPERATION(CFGID_OPERATION);
	CConfigValue opId;
	p1Op->ItemValueGet(cCFGID_OPERATION, &opId);
	CComPtr<IConfig> pOpCfg;
	p1Op->SubConfigGet(cCFGID_OPERATION, &pOpCfg);

	CConfigValue opName;
	p1Op->ItemValueGet(CComBSTR(CFGID_NAME), &opName);
	CConfigValue opDesc;
	p1Op->ItemValueGet(CComBSTR(CFGID_DESCRIPTION), &opDesc);
	CConfigValue opIconId;
	p1Op->ItemValueGet(CComBSTR(CFGID_ICONID), &opIconId);

	m_activeOpId = opId;
	m_activeIconId = opIconId;
	if (opName.operator BSTR())
		m_activeName.Attach(new CMultiLanguageString(opName.Detach().bstrVal));
	if (opDesc.operator BSTR())
		m_activeDesc.Attach(new CMultiLanguageString(opDesc.Detach().bstrVal));

	CConfigValue output;
	p1Op->ItemValueGet(CComBSTR(CFGID_OUTPUT), &output);
	m_activeOutput.Attach(output.Detach().bstrVal);

	CConfigValue opBuilder;
	p1Op->ItemValueGet(CComBSTR(CFGID_FACTORY), &opBuilder);
	m_activeBuilder = opBuilder;

	if (currentSub.operator LONG())
	{
		CSubOps subOps;
		GetSubOps(p1Op, subOps);
		for (CSubOps::const_iterator i = subOps.begin(); i != subOps.end(); ++i)
		{
			if (i->hash == currentSub.operator LONG())
			{
				// using sub-config
				m_activeOpCfg = i->config;
				//i->config->DuplicateCreate(&m_activeOpCfg);
				m_activeSubtitle = i->name;
				return;
			}
		}
	}
	m_activeOpCfg = pOpCfg;
}

