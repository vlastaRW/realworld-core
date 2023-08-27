// DesignerViewFactoryChooseBestLayout.cpp : Implementation of CDesignerViewFactoryChooseBestLayout

#include "stdafx.h"
#include "DesignerViewFactoryChooseBestLayout.h"

#include "ConfigIDsChooseBestLayout.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>


// CDesignerViewFactoryChooseBestLayout

STDMETHODIMP CDesignerViewFactoryChooseBestLayout::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_BEST_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CLayoutDocTypeOptions :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigItemCustomOptions
{
public:
	BEGIN_COM_MAP(CLayoutDocTypeOptions)
		COM_INTERFACE_ENTRY(IConfigItemCustomOptions)
		COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
	END_COM_MAP()

	// IEnumConfigItemOptions methods
public:
	STDMETHOD(Size)(ULONG* a_pnSize)
	{
		try
		{
			*a_pnSize = 1;
			ObjectLock cLock(this);
			CComPtr<IPlugInCache> pPIC;
			RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
			m_pGUIDs = NULL;
			pPIC->CLSIDsEnum(CATID_DocumentBuilder, 0, &m_pGUIDs);
			if (m_pGUIDs)
			{
				ULONG nSize = 0;
				m_pGUIDs->Size(&nSize);
				*a_pnSize = 1+nSize;
			}
			return S_OK;
		}
		catch (...)
		{
			return a_pnSize ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Get)(ULONG a_nIndex, TConfigValue* a_ptItem)
	{
		try
		{
			a_ptItem->eTypeID = ECVTGUID;
			if (a_nIndex == 0)
			{
				a_ptItem->guidVal = GUID_NULL;
				return S_OK;
			}
			ObjectLock cLock(this);
			if (m_pGUIDs && SUCCEEDED(m_pGUIDs->Get(a_nIndex-1, &a_ptItem->guidVal)))
				return S_OK;
			return E_FAIL;
		}
		catch (...)
		{
			return a_ptItem ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems)
	{
		try
		{
			ObjectLock cLock(this);
			for (ULONG i = 0; i < a_nCount; ++i)
			{
				HRESULT hRes = Get(a_nIndexFirst+i, a_atItems+i);
				if (FAILED(hRes)) return hRes;
			}
			return S_OK;
		}
		catch (...)
		{
			return a_atItems ? E_UNEXPECTED : E_POINTER;
		}
	}

	// IConfigItemCustomOptions methods
public:
	STDMETHOD(GetValueName)(TConfigValue const* a_pValue, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			if (a_pValue->eTypeID != ECVTGUID)
				return E_FAIL;
			if (IsEqualGUID(a_pValue->guidVal, GUID_NULL))
			{
				*a_ppName = new CMultiLanguageString(L"[0409]< not set >[0405]< nenastaveno >");
				return S_OK;
			}
			CComPtr<IDocumentBuilder> pDB;
			RWCoCreateInstance(pDB, a_pValue->guidVal);
			return pDB ? pDB->TypeName(a_ppName) : E_FAIL;
		}
		catch (...)
		{
			return a_ppName ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	CComPtr<IEnumGUIDs> m_pGUIDs;
};

#include <ConfigCustomGUIImpl.h>


class ATL_NO_VTABLE CConfigGUIChooseLayoutDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIChooseLayoutDlg>,
	public CDialogResize<CConfigGUIChooseLayoutDlg>
{
public:
	enum { IDC_VIEWTYPE = 100, IDC_VIEWTYPE_LABEL, IDC_BUILDERID, IDC_BUILDERID_LABEL, IDC_VIEWCONFIG };

	BEGIN_DIALOG_EX(0, 0, 194, 70, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]View type:[0405]Typ pohledu:"), IDC_VIEWTYPE_LABEL, 0, 2, 48, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_VIEWTYPE, 50, 0, 38, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Document type:[0405]Typ dokumentu:"), IDC_BUILDERID_LABEL, 95, 2, 58, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_BUILDERID, 155, 0, 38, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
		CONTROL_LTEXT(_T(""), IDC_VIEWCONFIG, 0, 16, 194, 54, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIChooseLayoutDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIChooseLayoutDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIChooseLayoutDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIChooseLayoutDlg)
		CONFIGITEM_COMBOBOX(IDC_VIEWTYPE, CFGID_BEST_SUBVIEW)
		CONFIGITEM_COMBOBOX(IDC_BUILDERID, CFGID_BEST_BUILDERID)
		CONFIGITEM_SUBCONFIG(IDC_VIEWCONFIG, CFGID_BEST_SUBVIEW)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIChooseLayoutDlg)
		DLGRESIZE_CONTROL(IDC_VIEWTYPE, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_BUILDERID_LABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_BUILDERID, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_VIEWCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};

#include <ConfigCustomGUIList.h>

class ATL_NO_VTABLE CConfigGUIBestLayoutDlg :
	public CCustomConfigWndImpl<CConfigGUIBestLayoutDlg>,
	public CCustomConfigGUIList<CConfigGUIBestLayoutDlg, IDC_CG_LIST, IDC_CG_TOOLBAR, IDC_CG_ITEM, IDS_CGTOOLBAR_BUTTONNAMES, 0, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON>,
	public CDialogResize<CConfigGUIBestLayoutDlg>
{
public:
	CConfigGUIBestLayoutDlg() :
		CCustomConfigGUIList<CConfigGUIBestLayoutDlg, IDC_CG_LIST, IDC_CG_TOOLBAR, IDC_CG_ITEM, IDS_CGTOOLBAR_BUTTONNAMES, 0, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON>(CFGID_BEST_SUBLAYOUTS)
	{
	}
	enum { IDD = IDD_CONFIGGUI_BESTLAYOUT };

	BEGIN_MSG_MAP(CConfigGUIBestLayoutDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIBestLayoutDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIBestLayoutDlg>)
		CHAIN_MSG_MAP(ListClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIBestLayoutDlg)
		DLGRESIZE_CONTROL(IDC_CGSUBDOCFRM_SYNCGROUP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_LIST, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CG_ITEM, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIBestLayoutDlg)
		CONFIGITEM_EDITBOX(IDC_CGSUBDOCFRM_SYNCGROUP, CFGID_BEST_SUBSYNCID)
		//CONFIGITEM_SUBCONFIG(IDC_CG_VIEWCONFIG, CFGID_TOOLBAR_VIEW)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	void ExtraConfigNotify()
	{
		UpdateList();
	}
};

// {FD859B45-7113-477b-9D6A-E7842B7D3AA5}
extern const GUID tSubCfgID = {0xfd859b45, 0x7113, 0x477b, {0x9d, 0x6a, 0xe7, 0x84, 0x2b, 0x7d, 0x3a, 0xa5}};
STDMETHODIMP CDesignerViewFactoryChooseBestLayout::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComPtr<IConfigWithDependencies> pLayout;
		RWCoCreateInstance(pLayout, __uuidof(ConfigWithDependencies));

		// CFGID_BEST_SUBVIEW
		a_pManager->InsertIntoConfigAs(a_pManager, pLayout.p, CComBSTR(CFGID_BEST_SUBVIEW), _SharedStringTable.GetStringAuto(IDS_CFGID_BEST_SUBVIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BEST_SUBVIEW_DESC), 0, NULL);

		// CFGID_BEST_BUILDERID
		CComObject<CLayoutDocTypeOptions>* p = NULL;
		CComObject<CLayoutDocTypeOptions>::CreateInstance(&p);
		CComPtr<IConfigItemCustomOptions> pDocTypesOpetions = p;
		pLayout->ItemIns1ofNWithCustomOptions(CComBSTR(CFGID_BEST_BUILDERID), CMultiLanguageString::GetAuto(L"[0409]Document type[0405]Typ dokumentu"), CMultiLanguageString::GetAuto(L"[0409]If set, the layout will be usable only with documents of the selected type.[0405]Pokud je nastaveno, bude layout použitelný pouze s dokumenty vybraného typu."), CConfigValue(GUID_NULL), pDocTypesOpetions, NULL, 0, NULL);

		CConfigCustomGUI<&tSubCfgID, CConfigGUIChooseLayoutDlg>::FinalizeConfig(pLayout);

		CComPtr<ISubConfigVector> pLayouts;
		RWCoCreateInstance(pLayouts, __uuidof(SubConfigVector));
		pLayouts->Init(TRUE, pLayout);
		pLayouts->ControllerSet(CConfigValue(1L));
		CComBSTR bstr00000000(L"00000000");
		pLayouts->ItemValuesSet(1, &bstr00000000.m_str, &CConfigValue(L"(Default)"));

		// CFGID_BEST_SUBLAYOUTS
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_BEST_SUBLAYOUTS), _SharedStringTable.GetStringAuto(IDS_CFGID_BEST_SUBLAYOUTS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BEST_SUBLAYOUTS_DESC), CConfigValue(1L), pLayouts, CConfigValue(1L), CConfigValue(256L), CConfigValue(1L), 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BEST_SUBSYNCID), CMultiLanguageString::GetAuto(L"[0409]State Synchronization ID[0405]Synchronizační ID stavu"), CMultiLanguageString::GetAuto(L"[0409]If set, best layout will be selected based on a subdocument.[0405]Pokud je zadáno, nejlepší layout bude vybrán podle poddokumentu."), CConfigValue(L""), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DesignerViewFactoryChooseBestLayout, CConfigGUIBestLayoutDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CCheckSuitabilityCallback : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public ICheckSuitabilityCallback
{
public:
BEGIN_COM_MAP(CCheckSuitabilityCallback)
	COM_INTERFACE_ENTRY(ICheckSuitabilityCallback)
END_COM_MAP()

	// ICheckSuitabilityCallback methods
public:
	STDMETHOD(Used)(REFIID a_iid) { cUsed.insert(a_iid); return S_OK; }
	STDMETHOD(Missing)(REFIID a_iid) { cUsed.insert(a_iid); return S_OK; }

	struct lessIID { bool operator()(IID const& a_1, IID const& a_2) const { return memcmp(&a_1, &a_2, sizeof a_1) < 0; } };
	std::set<IID, lessIID> cUsed;
	std::set<IID, lessIID> cMissing;
};

static LONG GetBestLayout(IDocument* pDoc, LONG nLayouts, IConfig* pLayouts, IViewManager* pViewMgr)
{
	if (pDoc == NULL)
		return -1;

	CLSID tBuilderID = GUID_NULL;
	pDoc->BuilderID(&tBuilderID);

	// have builder ID -> try to find layouts that match
	if (!IsEqualGUID(tBuilderID, GUID_NULL))
	{
		OLECHAR szNameID[64];
		LONG iSelected = -1;
		for (LONG i = 0; i < nLayouts; ++i)
		{
			_snwprintf(szNameID, itemsof(szNameID), L"%08x\\%s", i, CFGID_BEST_BUILDERID);
			CConfigValue cBuilderID;
			pLayouts->ItemValueGet(CComBSTR(szNameID), &cBuilderID);
			if (IsEqualGUID(tBuilderID, cBuilderID))
			{
				iSelected = i;
				break;
			}
		}
		if (iSelected != -1)
			return iSelected;
	}

	// 1. gather information
	struct SRating {int nUsed; int nOthers;};
	SRating* aRatings = reinterpret_cast<SRating*>(_alloca(nLayouts*sizeof(SRating)));

	OLECHAR szNameID[64];
	LONG i;
	for (i = 0; i < nLayouts; i++)
	{
		_snwprintf(szNameID, itemsof(szNameID), L"%08x\\%s", i, CFGID_BEST_SUBVIEW);
		CConfigValue cView;
		pLayouts->ItemValueGet(CComBSTR(szNameID), &cView);
		CComPtr<IConfig> pViewCfg;
		pLayouts->SubConfigGet(CComBSTR(szNameID), &pViewCfg);
		CComObjectStackEx<CCheckSuitabilityCallback> cCallback;
		pViewMgr->CheckSuitability(pViewMgr, cView, pViewCfg, pDoc, &cCallback);
		aRatings[i].nUsed = cCallback.cUsed.size();
		aRatings[i].nOthers = cCallback.cMissing.size();
	}

	// 2. find the best layout
	//   layout comparison conditions:
	//   - number of used interfaces
	//   - no other interface required
	//   - layout position in the config
	LONG iBest = 0;
	for (i = 1; i < nLayouts; i++)
	{
		if (aRatings[iBest].nUsed < aRatings[i].nUsed ||
			(aRatings[iBest].nUsed == aRatings[i].nUsed && aRatings[iBest].nOthers && aRatings[i].nOthers == 0))
		{
			// this one is better
			iBest = i;
		}
	}
	return iBest;
}

#include "DesignerViewChooseBestLayout.h"
#include "DesignerViewChooseBestLayoutSubDoc.h"

STDMETHODIMP CDesignerViewFactoryChooseBestLayout::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;
		CComPtr<IDesignerView> pDummy;

		CComBSTR bstrCFGID_BEST_SUBLAYOUTS(CFGID_BEST_SUBLAYOUTS);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstrCFGID_BEST_SUBLAYOUTS, &cVal);
		LONG nLayouts = cVal;
		if (nLayouts <= 0)
			return E_FAIL;
		CComPtr<IConfig> pLayouts;
		a_pConfig->SubConfigGet(bstrCFGID_BEST_SUBLAYOUTS, &pLayouts);

		a_pConfig->ItemValueGet(CComBSTR(CFGID_BEST_SUBSYNCID), &cVal);
		CComPtr<ISubDocumentsMgr> pSDM;
		if (cVal.operator BSTR() != NULL && cVal.operator BSTR()[0])
			a_pDoc->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM));
		if (pSDM)
		{
			CComObject<CDesignerViewChooseBestLayoutSubDoc>* pWnd = NULL;
			CComObject<CDesignerViewChooseBestLayoutSubDoc>::CreateInstance(&pWnd);
			pDummy = pWnd;

			pWnd->Init(a_pManager, a_pFrame, cVal, a_pStatusBar, nLayouts, pLayouts, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc, pSDM);
		}
		else
		{
			CComObject<CDesignerViewChooseBestLayout>* pWnd = NULL;
			CComObject<CDesignerViewChooseBestLayout>::CreateInstance(&pWnd);
			pDummy = pWnd;

			pWnd->Init(a_pManager, a_pFrame, a_pStatusBar, nLayouts, pLayouts, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc);
		}

		*a_ppDVWnd = pDummy.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryChooseBestLayout::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* UNREF(a_pDocument), ICheckSuitabilityCallback* UNREF(a_pCallback))
{
	return S_OK;
}
