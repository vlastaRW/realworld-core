// DesignerViewFactoryCommandPanel.cpp : Implementation of CDesignerViewFactoryCommandPanel

#include "stdafx.h"
#include "DesignerViewFactoryCommandPanel.h"
#include <MultiLanguageString.h>


// CDesignerViewFactoryCommandPanel

STDMETHODIMP CDesignerViewFactoryCommandPanel::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]» Command panel «[0405]» Panel pøíkazù «");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

extern __declspec(selectany) const OLECHAR CFGID_CMDPANEL_COMMANDS[] = L"Commands";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUICommandPanelDlg :
	public CCustomConfigWndImpl<CConfigGUICommandPanelDlg>,
	public CDialogResize<CConfigGUICommandPanelDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_CMDPANEL };

	BEGIN_MSG_MAP(CConfigGUICommandPanelDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUICommandPanelDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUICommandPanelDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUICommandPanelDlg)
		DLGRESIZE_CONTROL(IDC_CG_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUICommandPanelDlg)
		CONFIGITEM_COMBOBOX(IDC_CG_OPERATION, CFGID_CMDPANEL_COMMANDS)
		CONFIGITEM_SUBCONFIG(IDC_CG_OPCONFIG, CFGID_CMDPANEL_COMMANDS)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};

#include "DesignerViewCommandPanel.h"


STDMETHODIMP CDesignerViewFactoryCommandPanel::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComPtr<IMenuCommandsManager> pMenuCmds;
		a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&pMenuCmds));
		if (pMenuCmds == NULL)
		{
			RWCoCreateInstance(pMenuCmds, __uuidof(MenuCommandsManager));
		}
		pMenuCmds->InsertIntoConfigAs(pMenuCmds, pCfgInit, CComBSTR(CFGID_CMDPANEL_COMMANDS), CMultiLanguageString::GetAuto(L"[0409]Commands[0405]Pøíkazy"), CMultiLanguageString::GetAuto(L"[0409]Object determining the commands displayed in the command panel.[0405]Objekt urèující, které pøíkazy budou zobrazeny v pøíkazovém panelu."), 0, NULL);

		// name
		//CComBSTR cCFGID_CMDPANEL_NAME(CFGID_CMDPANEL_NAME);
		//pConfigPattern->ItemInsSimple(cCFGID_CMDPANEL_NAME, _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_NAME_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_NAME_DESC), CConfigValue(L""), NULL, 0, NULL);


		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DesignerViewFactoryCommandPanel, CConfigGUICommandPanelDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryCommandPanel::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewCommandPanel>* pWnd = NULL;
		CComObject<CDesignerViewCommandPanel>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		pWnd->Init(a_pManager, a_pDoc, a_pFrame, a_pStatusBar, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID);

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryCommandPanel::CheckSuitability(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	return S_FALSE; // TODO: implement view suitability for commands
}

