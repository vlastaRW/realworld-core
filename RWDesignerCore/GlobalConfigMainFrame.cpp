// GlobalConfigMainFrame.cpp : Implementation of CGlobalConfigMainFrame

#include "stdafx.h"
#include "GlobalConfigMainFrame.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>


// CGlobalConfigMainFrame

ULONG CGlobalConfigMainFrame::m_nStartPages = 0;
CLSID const* CGlobalConfigMainFrame::m_aStartPages = NULL;
CLSID CGlobalConfigMainFrame::m_tDefaultStartPage = GUID_NULL;

STDMETHODIMP CGlobalConfigMainFrame::Interactive(BYTE* a_pPriority)
{
	if (a_pPriority)
		*a_pPriority = 0;
	return S_OK;
}

STDMETHODIMP CGlobalConfigMainFrame::Name(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_GCNAME_MAINFRAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CGlobalConfigMainFrame::Description(ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		*a_ppDesc = _SharedStringTable.GetString(IDS_GCDESC_MAINFRAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

#include "ConfigIDsApp.h"
#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>

typedef std::vector<WORD> CLGIDs;
static CLGIDs g_cLGIDs; // TODO: remove global variable

BOOL CALLBACK EnumAppLangs(LPTSTR lpLocaleString)
{
	DWORD i;
	_stscanf(lpLocaleString, _T("%x"), &i);
	g_cLGIDs.push_back(i);
	return TRUE;
}


class ATL_NO_VTABLE CConfigGUIMainFrameDlg :
	public CCustomConfigWndImpl<CConfigGUIMainFrameDlg>,
	public CDialogResize<CConfigGUIMainFrameDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_MAINFRAME };

	BEGIN_MSG_MAP(CConfigGUIMainFrameDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIMainFrameDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIMainFrameDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_APPOPT_UNDODISABLE, BN_CLICKED, OnUndoClicked)
		COMMAND_HANDLER(IDC_APPOPT_DISABLEUPDATES, BN_CLICKED, OnUpdatesClicked)
		COMMAND_HANDLER(IDC_APPOPT_STARTPAGE, CBN_SELCHANGE, OnStartPageChanges)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIMainFrameDlg)
		DLGRESIZE_CONTROL(IDC_APPOPT_STARTPAGE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_APPOPT_INTERFACE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_APPOPT_CAPTION_GRP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_APPOPT_CAPTAPP, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_APPOPT_CAPTLAYOUT, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_APPOPT_CAPTNAME, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_APPOPT_CAPTTYPE, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_APPOPT_LAYOUTCMDS, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIMainFrameDlg)
		//CONFIGITEM_COMBOBOX(IDC_APPOPT_STARTPAGE, CFGID_STARTPAGE)
		CONFIGITEM_COMBOBOX(IDC_APPOPT_INTERFACE, CFGID_USERINTERFACE)
		CONFIGITEM_CHECKBOX_FLAG(IDC_APPOPT_CAPTAPP, CFGID_CAPTION, CFGFLAG_CAPTION_APP)
		CONFIGITEM_CHECKBOX_FLAG(IDC_APPOPT_CAPTLAYOUT, CFGID_CAPTION, CFGFLAG_CAPTION_LAYOUT)
		CONFIGITEM_CHECKBOX_FLAG(IDC_APPOPT_CAPTNAME, CFGID_CAPTION, CFGFLAG_CAPTION_NAME)
		CONFIGITEM_CHECKBOX_FLAG(IDC_APPOPT_CAPTTYPE, CFGID_CAPTION, CFGFLAG_CAPTION_TYPE)
		CONFIGITEM_CHECKBOX_FLAG(IDC_APPOPT_CAPTPROPS, CFGID_CAPTION, CFGFLAG_CAPTION_PROPS)
		CONFIGITEM_CONTEXTHELP_IDS(IDC_APPOPT_UNDODISABLE, IDS_HELP_APPOPT_UNDODISABLED)
		CONFIGITEM_CONTEXTHELP(IDC_APPOPT_DISABLEUPDATES, CFGID_AUTOUPDATE)
		CONFIGITEM_CHECKBOX(IDC_APPOPT_LAYOUTCMDS, CFGID_LAYOUTCOMMANDS)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		m_wndStart = GetDlgItem(IDC_APPOPT_STARTPAGE);
		//if (CGlobalConfigMainFrame::m_aStartPages)
		//{
		//}
		//else
		{
			CComPtr<IPlugInCache> pPIC;
			RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
			CComPtr<IEnumGUIDs> pGUIDs;
			if (pPIC) pPIC->CLSIDsEnum(CATID_StartViewPage, 0xffffffff, &pGUIDs);
			ULONG nGUIDs = 0;
			if (pGUIDs) pGUIDs->Size(&nGUIDs);
			m_aStartPages.Allocate(nGUIDs+4);
			m_aStartPages[0] = __uuidof(StartViewPageFactoryNewDocument);
			m_aStartPages[1] = __uuidof(StartViewPageFactoryOpenFile);
			m_aStartPages[2] = __uuidof(StartViewPageFactoryRecentFiles);
			pGUIDs->GetMultiple(0, nGUIDs, m_aStartPages.m_p+3);
			m_aStartPages[3+nGUIDs] = __uuidof(StartPageOnline);
			m_nStartPages = nGUIDs+4;
		}
		for (ULONG i = 0; i < m_nStartPages; ++i)
		{
			CComPtr<IStartViewPageFactory> p;
			RWCoCreateInstance(p, m_aStartPages[i]);
			CComBSTR bstr;
			if (p)
			{
				CComPtr<ILocalizedString> pStr;
				p->Name(&pStr);
				if (pStr) pStr->GetLocalized(m_tLocaleID, &bstr);
			}
			m_wndStart.AddString(bstr.m_str ? bstr.m_str : L"");
		}
		//pCfg->ItemIns1ofN(cCFGID_STARTPAGE, _SharedStringTable.GetStringAuto(IDS_CFGID_STARTPAGE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_STARTPAGE_DESC), CConfigValue(m_tDefaultStartPage), NULL);
		//for (ULONG i = 0; i < m_nStartPages; ++i)
		//{
		//	if (!IsEqualCLSID(m_aStartPages[i], GUID_NULL))
		//	{
		//		CComPtr<IStartViewPageFactory> p;
		//		RWCoCreateInstance(p, m_aStartPages[i]);
		//		if (p)
		//		{
		//			CComPtr<ILocalizedString> pStr;
		//			p->Name(&pStr);
		//			pCfg->ItemOptionAdd(cCFGID_STARTPAGE, CConfigValue(m_aStartPages[i]), pStr, 0, NULL);
		//		}
		//	}
		//}
	}
	void ExtraConfigNotify()
	{
		CConfigValue cAU;
		M_Config()->ItemValueGet(CComBSTR(CFGID_AUTOUPDATE), &cAU);
		if (cAU.TypeGet() == ECVTInteger)
			CheckDlgButton(IDC_APPOPT_DISABLEUPDATES, cAU.operator LONG() ? BST_UNCHECKED : BST_CHECKED);
		CConfigValue cUM;
		M_Config()->ItemValueGet(CComBSTR(CFGID_UNDOMODE), &cUM);
		if (cUM.TypeGet() == ECVTInteger)
			CheckDlgButton(IDC_APPOPT_UNDODISABLE, cUM.operator LONG() == EUMDisabled ? BST_CHECKED : BST_UNCHECKED);
		CConfigValue cSP;
		M_Config()->ItemValueGet(CComBSTR(CFGID_STARTPAGE), &cSP);
		if (cSP.TypeGet() == ECVTGUID)
		{
			for (ULONG i = 0; i < m_nStartPages; ++i)
			{
				if (IsEqualGUID(m_aStartPages[i], cSP))
				{
					m_wndStart.SetCurSel(i);
					break;
				}
			}
		}
	}
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnUndoClicked(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		CComBSTR bstr(CFGID_UNDOMODE);
		M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(LONG(IsDlgButtonChecked(IDC_APPOPT_UNDODISABLE) == BST_CHECKED ? EUMDisabled : EUMDefault)));
		return 1;
	}
	LRESULT OnUpdatesClicked(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		CComBSTR bstr(CFGID_AUTOUPDATE);
		M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(IsDlgButtonChecked(IDC_APPOPT_DISABLEUPDATES) == BST_CHECKED ? 0L : 1L));
		return 1;
	}
	LRESULT OnStartPageChanges(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int iSel = m_wndStart.GetCurSel();
		CComBSTR bstr(CFGID_STARTPAGE);
		M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(iSel > 0 && ULONG(iSel) < m_nStartPages ? m_aStartPages[iSel] : GUID_NULL));
		return 1;
	}

private:
	CComboBox m_wndStart;
	CAutoVectorPtr<GUID> m_aStartPages;
	ULONG m_nStartPages;
};

STDMETHODIMP CGlobalConfigMainFrame::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		pCfg->ItemInsSimple(CComBSTR(CFGID_CTXHELPASTIPS), _SharedStringTable.GetStringAuto(IDS_CFGID_CTXHELPASTIPS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CTXHELPASTIPS_DESC), CConfigValue(true), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_APPASKTOSAVE), _SharedStringTable.GetStringAuto(IDS_CFGID_APPASKTOSAVE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_APPASKTOSAVE_DESC), CConfigValue(true), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SAVELAYOUT), _SharedStringTable.GetStringAuto(IDS_CFGID_APPSAVELAYOUT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_APPSAVELAYOUT_DESC), CConfigValue(true), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_LAYOUTCOMMANDS), _SharedStringTable.GetStringAuto(IDS_CFGID_LAYOUTCOMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_LAYOUTCOMMANDS_DESC), CConfigValue(false), NULL, 0, NULL);
		CComBSTR cCFGID_USERINTERFACE(CFGID_USERINTERFACE);
		pCfg->ItemIns1ofN(cCFGID_USERINTERFACE, _SharedStringTable.GetStringAuto(IDS_CFGID_USERINTERFACE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_USERINTERFACE_DESC), CConfigValue(CFGVAL_UI_MSDI), NULL);
		pCfg->ItemOptionAdd(cCFGID_USERINTERFACE, CConfigValue(CFGVAL_UI_MSDI), _SharedStringTable.GetStringAuto(IDS_CFGVAL_UI_MSDI), 0, NULL);
		//pCfg->ItemOptionAdd(cCFGID_USERINTERFACE, CConfigValue(CFGVAL_UI_TABS), _SharedStringTable.GetStringAuto(IDS_CFGVAL_UI_TABS), 0, NULL);
		CComBSTR cCFGID_STARTPAGE(CFGID_STARTPAGE);
		pCfg->ItemInsSimple(cCFGID_STARTPAGE, _SharedStringTable.GetStringAuto(IDS_CFGID_STARTPAGE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_STARTPAGE_DESC), CConfigValue(m_tDefaultStartPage), NULL, 0, NULL);
		//pCfg->ItemIns1ofN(cCFGID_STARTPAGE, _SharedStringTable.GetStringAuto(IDS_CFGID_STARTPAGE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_STARTPAGE_DESC), CConfigValue(m_tDefaultStartPage), NULL);
		//for (ULONG i = 0; i < m_nStartPages; ++i)
		//{
		//	if (!IsEqualCLSID(m_aStartPages[i], GUID_NULL))
		//	{
		//		CComPtr<IStartViewPageFactory> p;
		//		RWCoCreateInstance(p, m_aStartPages[i]);
		//		if (p)
		//		{
		//			CComPtr<ILocalizedString> pStr;
		//			p->Name(&pStr);
		//			pCfg->ItemOptionAdd(cCFGID_STARTPAGE, CConfigValue(m_aStartPages[i]), pStr, 0, NULL);
		//		}
		//	}
		//}

		CComBSTR cCFGID_UNDOMODE(CFGID_UNDOMODE);
		pCfg->ItemIns1ofN(cCFGID_UNDOMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_UNDOMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_UNDOMODE_DESC), CConfigValue(LONG(EUMDefault)), NULL);
		pCfg->ItemOptionAdd(cCFGID_UNDOMODE, CConfigValue(LONG(EUMDefault)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_UNDOMODE_DEFAULT), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_UNDOMODE, CConfigValue(LONG(EUMDisabled)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_UNDOMODE_DISABLED), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_UNDOMODE, CConfigValue(LONG(EUMSingleStep)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_UNDOMODE_SINGLE), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_UNDOMODE, CConfigValue(LONG(EUMMemoryLimited)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_UNDOMODE_MULTIPLE), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_UNDOMODE, CConfigValue(LONG(EUMAllSteps)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_UNDOMODE_ALL), 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_CAPTION), _SharedStringTable.GetStringAuto(IDS_CFGID_CAPTION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CAPTION_DESC), CConfigValue(CFGFLAG_CAPTION_NAME|CFGFLAG_CAPTION_TYPE|CFGFLAG_CAPTION_PROPS|CFGFLAG_CAPTION_APP), NULL, 0, NULL);

		CComBSTR cCFGID_AUTOUPDATE(CFGID_AUTOUPDATE);
		pCfg->ItemIns1ofN(CComBSTR(CFGID_AUTOUPDATE), CMultiLanguageString::GetAuto(L"[0409]Updates[0405]Aktualizace"), CMultiLanguageString::GetAuto(L"[0409]Automatically update the software[0405]Automaticky aktualizovat software"), CConfigValue(1L), NULL);
		pCfg->ItemOptionAdd(cCFGID_AUTOUPDATE, CConfigValue(0L), CMultiLanguageString::GetAuto(L"[0409]Disabled[0405]Zakázáno"), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_AUTOUPDATE, CConfigValue(1L), CMultiLanguageString::GetAuto(L"[0409]Enabled[0405]Povoleno"), 0, NULL);
		//pCfg->ItemOptionAdd(cCFGID_AUTOUPDATE, CConfigValue(2L), CMultiLanguageString::GetAuto(L"[0409]Beta[0405]Beta"), 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_LASTCHECK), NULL, NULL, CConfigValue(0L), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_GlobalConfigMainFrame, CConfigGUIMainFrameDlg>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

