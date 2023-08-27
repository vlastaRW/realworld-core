// MenuCommandsCondition.cpp : Implementation of CMenuCommandsCondition

#include "stdafx.h"
#include "MenuCommandsCondition.h"
#include <MultiLanguageString.h>
#include <SharedStringTable.h>

static const OLECHAR CFGID_COND_SELECTION[] =		L"SyncID";
static const OLECHAR CFGID_COND_COMMANDS[] =		L"SubCommands";
static const OLECHAR CFGID_COND_BUILDERID[] =		L"BuilderID";


// CMenuCommandsCondition

STDMETHODIMP CMenuCommandsCondition::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Document - Condition[0405]Dokument - podmínka");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
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


class ATL_NO_VTABLE CConfigGUIMenuConditionDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIMenuConditionDlg>,
	public CDialogResize<CConfigGUIMenuConditionDlg>
{
public:
	enum { IDC_SYNCID = 100, IDC_SYNCID_LABEL, IDC_COMMANDS, IDC_COMMANDS_LABEL, IDC_BUILDERID, IDC_BUILDERID_LABEL, IDC_COMMANDS_CONFIG };

	BEGIN_DIALOG_EX(0, 0, 194, 70, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Selection ID:[0405]ID výběru:"), IDC_SYNCID_LABEL, 0, 2, 48, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_SYNCID, 50, 0, 38, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Document type:[0405]Typ dokumentu:"), IDC_BUILDERID_LABEL, 95, 2, 58, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_BUILDERID, 155, 0, 38, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT, 0)
		CONTROL_LTEXT(_T("[0409]Commands:[0405]Příkazy:"), IDC_COMMANDS_LABEL, 0, 18, 48, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_COMMANDS, 50, 16, 143, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL, 0)
		CONTROL_LTEXT(_T(""), IDC_COMMANDS_CONFIG, 0, 32, 194, 38, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIMenuConditionDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIMenuConditionDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIMenuConditionDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIMenuConditionDlg)
		CONFIGITEM_EDITBOX(IDC_SYNCID, CFGID_COND_SELECTION)
		CONFIGITEM_COMBOBOX(IDC_BUILDERID, CFGID_COND_BUILDERID)
		CONFIGITEM_COMBOBOX(IDC_COMMANDS, CFGID_COND_COMMANDS)
		CONFIGITEM_SUBCONFIG(IDC_COMMANDS_CONFIG, CFGID_COND_COMMANDS)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIMenuConditionDlg)
		DLGRESIZE_CONTROL(IDC_SYNCID, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_BUILDERID_LABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_BUILDERID, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_COMMANDS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_COMMANDS_CONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};

STDMETHODIMP CMenuCommandsCondition::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		// condition
		CComObject<CLayoutDocTypeOptions>* p = NULL;
		CComObject<CLayoutDocTypeOptions>::CreateInstance(&p);
		CComPtr<IConfigItemCustomOptions> pDocTypesOptions = p;
		pCfg->ItemIns1ofNWithCustomOptions(CComBSTR(CFGID_COND_BUILDERID), CMultiLanguageString::GetAuto(L"[0409]Document type[0405]Typ dokumentu"), CMultiLanguageString::GetAuto(L"[0409]If set, the layout will be usable only with documents of the selected type.[0405]Pokud je nastaveno, bude layout použitelný pouze s dokumenty vybraného typu."), CConfigValue(GUID_NULL), pDocTypesOptions, NULL, 0, NULL);

		// sync id
		pCfg->ItemInsSimple(CComBSTR(CFGID_COND_SELECTION), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCCATID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBDOCCATID_DESC), CConfigValue(L"SUBDOC"), NULL, 0, NULL);

		// sub-operation
		a_pManager->InsertIntoConfigAs(a_pManager, pCfg, CComBSTR(CFGID_COND_COMMANDS), _SharedStringTable.GetStringAuto(IDS_CFGID_EXTSD_SUBOP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXTSD_SUBOP_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_MenuCommandsCondition, CConfigGUIMenuConditionDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsCondition::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IDocument> pSubDoc;

		CConfigValue cStateID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_COND_SELECTION), &cStateID);

		if (cStateID.operator BSTR()[0])
		{
			CComPtr<ISubDocumentsMgr> pSDM;
			a_pDocument->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM));
			if (pSDM)
				GetSubDoc(pSDM, cStateID, a_pStates, &pSubDoc);

			if (pSubDoc == NULL)
				return E_FAIL;
		}
		else
		{
			pSubDoc = a_pDocument;
		}

		CConfigValue cBuilderID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_COND_BUILDERID), &cBuilderID);
		GUID tBuilderID = GUID_NULL;
		pSubDoc->BuilderID(&tBuilderID);
		if (!IsEqualGUID(tBuilderID, cBuilderID))
			return S_FALSE;

		CComBSTR cCFGID_COND_COMMANDS(CFGID_COND_COMMANDS);
		CConfigValue cCmdsID;
		a_pConfig->ItemValueGet(cCFGID_COND_COMMANDS, &cCmdsID);
		CComPtr<IConfig> pCmdsCfg;
		a_pConfig->SubConfigGet(cCFGID_COND_COMMANDS, &pCmdsCfg);

		return a_pManager->CommandsEnum(a_pManager, cCmdsID, pCmdsCfg, a_pStates, a_pView, a_pDocument, a_ppSubCommands);
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

void CMenuCommandsCondition::GetSubDoc(ISubDocumentsMgr* a_pSDM, LPWSTR a_pszIDs, IOperationContext* a_pStates, IDocument** a_ppDoc)
{
	while (*a_pszIDs == L' ' || *a_pszIDs == L',') ++a_pszIDs;
	LPWSTR psz = a_pszIDs;
	while (*psz && *psz != L' ' && *psz != L',') ++psz;
	if (psz == a_pszIDs)
		return;

	CComBSTR bstrPref;
	a_pSDM->StatePrefix(&bstrPref);
	ULONG const nPrefLen = bstrPref.Length();
	CComBSTR bstrID(nPrefLen+(psz-a_pszIDs));
	CopyMemory(bstrID.m_str, bstrPref.m_str, nPrefLen*sizeof(wchar_t));
	CopyMemory(bstrID.m_str+nPrefLen, a_pszIDs, (psz-a_pszIDs)*sizeof(wchar_t));

	while (*psz == L' ' || *psz == L',') ++psz;


	CComPtr<ISharedState> pState;
	a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
	CComPtr<IEnumUnknowns> pItems;
	if (pState)
		a_pSDM->StateUnpack(pState, &pItems);
	CComPtr<IComparable> pItem;
	if (pItems)
		pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
	CComPtr<ISubDocumentID> pID;
	if (pItem)
		a_pSDM->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pID));
	if (pID == NULL)
	{
		pItems = NULL;
		a_pSDM->ItemsEnum(NULL, &pItems);
		if (pItems == NULL)
			return;
		pItem = NULL;
		pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
		if (pItem == NULL)
			return;
		a_pSDM->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pID));
	}
	if (pID != NULL)
	{
		if (*psz)
		{
			CComPtr<IDocument> pDoc;
			pID->SubDocumentGet(&pDoc);
			if (pDoc)
			{
				CComPtr<ISubDocumentsMgr> pSDM;
				pDoc->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM));
				if (pSDM)
					GetSubDoc(pSDM, psz, a_pStates, a_ppDoc);
			}
		}
		else
		{
			pID->SubDocumentGet(a_ppDoc);
		}
	}
}

