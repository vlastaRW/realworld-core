

#pragma once

#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <Win32LangEx.h>
#include <ContextHelpDlg.h>
#include <DPIUtils.h>
#include <ReturnedData.h>

typedef HICON (*pfnGetIcon)(ULONG size);

struct SManageItemsSpecs
{
	LPCTSTR pszManageDlgHelpTopic;

	pfnGetIcon fnManageDialogIcon;
	UINT uManageDialogTitle;
	UINT uManageListViewLabel;
	UINT uManageListViewHelp;

	pfnGetIcon fnItemCreateIcon;
	pfnGetIcon fnItemCloneIcon;
	pfnGetIcon fnItemDeleteIcon;
	pfnGetIcon fnItemRenameIcon;
	pfnGetIcon fnItemConfigureIcon;
	pfnGetIcon fnItemMoveDownIcon;
	pfnGetIcon fnItemMoveUpIcon;
	pfnGetIcon fnItemLoad;
	pfnGetIcon fnItemSave;

	UINT uNewItem;
	UINT uCopyItem;

	UINT uManageButtonTooltips;

	LPCOLESTR pszManageDialogSizeX;
	LPCOLESTR pszManageDialogSizeY;

	LPCOLESTR pszRootConfigID;

	bool bAtLeastOneItem;

	UINT uIDSLoadItem;
	UINT uIDSSaveItem;
	UINT uIDSFileType;
	LPCOLESTR pszFileExtension;
	GUID tFileCtxID;
};

template<SManageItemsSpecs const* t_pSpecs, class TConfigureItemDlg>
class CManageConfigurableItemsDlg : 
	public Win32LangEx::CLangDialogImpl<CManageConfigurableItemsDlg<t_pSpecs, typename TConfigureItemDlg> >,
	public CDialogResize<CManageConfigurableItemsDlg<t_pSpecs, typename TConfigureItemDlg> >,
	public CUpdateUI<CManageConfigurableItemsDlg<t_pSpecs, typename TConfigureItemDlg> >,
	public CContextHelpDlg<CManageConfigurableItemsDlg<t_pSpecs, typename TConfigureItemDlg> >
{
public:
	CManageConfigurableItemsDlg(IConfig* a_pProfiles, IConfig* a_pGUIConfig, LCID a_tLocaleID, IDesignerFrameIcons* a_pIcons) :
		Win32LangEx::CLangDialogImpl<CManageConfigurableItemsDlg<t_pSpecs, TConfigureItemDlg> >(a_tLocaleID),
		CContextHelpDlg<CManageConfigurableItemsDlg<t_pSpecs, TConfigureItemDlg> >(t_pSpecs->pszManageDlgHelpTopic),
		m_pProfiles(a_pProfiles), m_pGUIConfig(a_pGUIConfig), m_pIcons(a_pIcons), m_hIcon(NULL)
	{
	}
	typedef CManageConfigurableItemsDlg<t_pSpecs, TConfigureItemDlg> thisClass;

	~CManageConfigurableItemsDlg()
	{
		m_cCommandImages.Destroy();
		m_cItemImages.Destroy();
		if (m_hIcon) ::DestroyIcon(m_hIcon);
	}

	enum
	{
		IDD = IDD_MANAGECONFIGURABLEITEMS,
		ID_CLONE = 100,
		ID_CONFIGURE,
		ID_CREATE,
		ID_DELETE,
		ID_MOVEDOWN,
		ID_MOVEUP,
		ID_RENAME,
		ID_LOAD,
		ID_SAVE,
	};

	BEGIN_MSG_MAP(thisClass)
		CHAIN_MSG_MAP(CContextHelpDlg<thisClass>)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnEndDialog)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnEndDialog)
		NOTIFY_HANDLER(IDC_MANAGE_ITEMLIST, LVN_ENDLABELEDIT, OnListRename)
		NOTIFY_HANDLER(IDC_MANAGE_ITEMLIST, LVN_ITEMCHANGED, OnListItemChanged)
		NOTIFY_HANDLER(IDC_MANAGE_ITEMLIST, LVN_ITEMACTIVATE, OnListItemActivate)
		COMMAND_ID_HANDLER(ID_CLONE, OnClone)
		COMMAND_ID_HANDLER(ID_CONFIGURE, OnConfigure)
		COMMAND_ID_HANDLER(ID_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_MOVEDOWN, OnMoveDown)
		COMMAND_ID_HANDLER(ID_MOVEUP, OnMoveUp)
		COMMAND_ID_HANDLER(ID_RENAME, OnRename)
		COMMAND_ID_HANDLER(ID_LOAD, OnLoadFile)
		COMMAND_ID_HANDLER(ID_SAVE, OnSaveFile)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		CHAIN_MSG_MAP(CDialogResize<thisClass>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_MANAGE_ITEMLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_MANAGE_TOOLBAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_MANAGE_LABEL, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_UPDATE_UI_MAP(thisClass)
		UPDATE_ELEMENT(ID_CLONE, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONFIGURE, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CREATE, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_DELETE, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MOVEDOWN, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MOVEUP, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_RENAME, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_SAVE, UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	BEGIN_CTXHELP_MAP(thisClass)
		CTXHELP_CONTROL_RESOURCE(IDOK, IDS_HELP_IDOK)
		CTXHELP_CONTROL_RESOURCE(IDCANCEL, IDS_HELP_IDCANCEL)
		CTXHELP_CONTROL_RESOURCE(IDHELP, IDS_HELP_IDHELP)
		CTXHELP_CONTROL_RESOURCE(IDC_MANAGE_ITEMLIST, t_pSpecs->uManageListViewHelp)
		CTXHELP_CONTROL_RESOURCE_NOTIP(IDC_MANAGE_TOOLBAR, IDS_HELP_MANAGE_TOOLBAR)
	END_CTXHELP_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		m_hIcon = t_pSpecs->fnManageDialogIcon(GetSystemMetrics(SM_CXSMICON));//DPIUtils::PrepareIconForCaption((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_pSpecs->uManageDialogIcon), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
		// set icons
		if (m_hIcon != NULL)
			SetIcon(m_hIcon, FALSE);

		TCHAR szParamText[128] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), t_pSpecs->uManageDialogTitle, szParamText, itemsof(szParamText), LANGIDFROMLCID(m_tLocaleID));
		SetWindowText(szParamText);
		Win32LangEx::LoadString(_pModule->get_m_hInst(), t_pSpecs->uManageListViewLabel, szParamText, itemsof(szParamText), LANGIDFROMLCID(m_tLocaleID));
		CStatic wndLabel = GetDlgItem(IDC_MANAGE_LABEL);
		wndLabel.SetWindowText(szParamText);

		m_cItemImages.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 8, 4);
		m_wndList = GetDlgItem(IDC_MANAGE_ITEMLIST);
		m_wndList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
		m_wndList.SetImageList(m_cItemImages, LVSIL_SMALL);
		RECT rcTmp;
		m_wndList.GetClientRect(&rcTmp);
		m_wndList.InsertColumn(0, _T(""), LVCFMT_LEFT, rcTmp.right-GetSystemMetrics(SM_CXVSCROLL), 0);
		FillList();

		TBBUTTON aButtons[] =
		{
			{0, ID_CREATE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
			{1, ID_CLONE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 1},
			{2, ID_DELETE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
			{0, 0, 0, BTNS_SEP, {0, 0}, 0, 0},
			{3, ID_LOAD, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 7},
			{4, ID_SAVE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 8},
			{0, 0, 0, BTNS_SEP, {0, 0}, 0, 0},
			{5, ID_RENAME, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 3},
			{6, ID_CONFIGURE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 4},
			{0, 0, 0, BTNS_SEP, {0, 0}, 0, 0},
			{7, ID_MOVEDOWN, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 5},
			{8, ID_MOVEUP, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 6},
		};
		// load tooltip strings (IE 5.01 is required)
		TCHAR szTooltipStrings[1024] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), t_pSpecs->uManageButtonTooltips, szTooltipStrings, itemsof(szTooltipStrings), LANGIDFROMLCID(m_tLocaleID));
		for (size_t i = 0; szTooltipStrings[i]; ++i) if (szTooltipStrings[i] == _T('|')) szTooltipStrings[i] = _T('\0');

		// load toolbar images
		int nIconSize = XPGUI::GetSmallIconSize();
		m_cCommandImages.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 9, 1);
		{HICON h = t_pSpecs->fnItemCreateIcon(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemCloneIcon(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemDeleteIcon(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemLoad(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemSave(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemRenameIcon(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemConfigureIcon(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemMoveDownIcon(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		{HICON h = t_pSpecs->fnItemMoveUpIcon(nIconSize); m_cCommandImages.AddIcon(h); DestroyIcon(h);}
		
		m_wndToolBar = GetDlgItem(IDC_MANAGE_TOOLBAR);
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cCommandImages);
		m_wndToolBar.AddStrings(szTooltipStrings);
		m_wndToolBar.AddButtons(itemsof(aButtons), aButtons);
		m_wndToolBar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);

		// align the toolbar to the right
		RECT rcActual;
		RECT rcDesired;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcDesired);
		m_wndToolBar.MoveWindow(rcActual.right-rcDesired.right, rcActual.top, rcDesired.right, rcActual.bottom-rcActual.top, FALSE);

		RECT rcLabel;
		wndLabel.GetWindowRect(&rcLabel);
		ScreenToClient(&rcLabel);
		wndLabel.MoveWindow(rcLabel.left, rcLabel.top, rcActual.right-rcDesired.right-rcLabel.left-10, rcLabel.bottom-rcLabel.top, FALSE);

		UIAddToolBar(m_wndToolBar);

		m_wndList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		DlgResize_Init();

		if (m_pGUIConfig != 0)
		{
			CConfigValue cSizeX;
			m_pGUIConfig->ItemValueGet(CComBSTR(t_pSpecs->pszManageDialogSizeX), &cSizeX);
			CConfigValue cSizeY;
			m_pGUIConfig->ItemValueGet(CComBSTR(t_pSpecs->pszManageDialogSizeY), &cSizeY);
			if (cSizeX.operator LONG() != CW_USEDEFAULT && cSizeY.operator LONG() != CW_USEDEFAULT)
			{
				RECT rc;
				GetWindowRect(&rc);
				rc.right = rc.left + cSizeX.operator LONG();
				rc.bottom = rc.top + cSizeY.operator LONG();
				MoveWindow(&rc);
			}
		}

		CenterWindow(GetParent());

		return 1;  // Let the system set the focus
	}

	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CDialogResize<thisClass>::OnSize(a_uMsg, a_wParam, a_lParam, a_bHandled);

		RECT rcTmp;
		m_wndList.GetClientRect(&rcTmp);
		m_wndList.SetColumnWidth(0, rcTmp.right-GetSystemMetrics(SM_CXVSCROLL));

		return 0;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (a_wParam)
		{
		case VK_F2:
			OnRename(0, 0, NULL, a_bHandled);
			break;
		case VK_DELETE:
			OnDelete(0, 0, NULL, a_bHandled);
			break;
		default:
			a_bHandled = FALSE;
		}
		return 0;
	}

	LRESULT OnEndDialog(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		if (m_pGUIConfig != 0)
		{
			RECT rc;
			GetWindowRect(&rc);
			BSTR aIDs[2];
			CComBSTR bstrIDX(t_pSpecs->pszManageDialogSizeX);
			CComBSTR bstrIDY(t_pSpecs->pszManageDialogSizeY);
			aIDs[0] = bstrIDX;
			aIDs[1] = bstrIDY;
			TConfigValue aVals[2];
			aVals[0] = CConfigValue(rc.right-rc.left);
			aVals[1] = CConfigValue(rc.bottom-rc.top);
			m_pGUIConfig->ItemValuesSet(2, aIDs, aVals);
		}
		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnListRename(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(a_pNMHDR);

		if (pDispInfo->item.pszText)
		{
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, pDispInfo->item.iItem);
			CComBSTR bstrNameID(szNameID);
			BSTR aIDs[1];
			aIDs[0] = bstrNameID;
			m_pProfiles->ItemValuesSet(1, aIDs, CConfigValue(pDispInfo->item.pszText));
			FillList();
		}

		return 0;
	}
	LRESULT OnListItemChanged(int UNREF(a_idCtrl), LPNMHDR UNREF(a_pNMHDR), BOOL& UNREF(a_bHandled))
	{
		int nCount = m_wndList.GetItemCount();
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel == -1)
		{
			UIEnable(ID_CLONE, FALSE);
			UIEnable(ID_CONFIGURE, FALSE);
			UIEnable(ID_CREATE, TRUE);
			UIEnable(ID_DELETE, FALSE);
			UIEnable(ID_MOVEDOWN, FALSE);
			UIEnable(ID_MOVEUP, FALSE);
			UIEnable(ID_RENAME, FALSE);
			UIEnable(ID_SAVE, FALSE);
		}
		else
		{
			UIEnable(ID_CLONE, TRUE);
			UIEnable(ID_CONFIGURE, TRUE);
			UIEnable(ID_CREATE, TRUE);
			UIEnable(ID_DELETE, t_pSpecs->bAtLeastOneItem ? nCount > 1 : true);
			UIEnable(ID_MOVEDOWN, nSel < (nCount-1));
			UIEnable(ID_MOVEUP, nSel > 0);
			UIEnable(ID_RENAME, TRUE);
			UIEnable(ID_SAVE, TRUE);
		}
		UIUpdateToolBar();

		return 0;
	}
	LRESULT OnListItemActivate(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(a_pNMHDR);

		ConfigureItem(pNMIA->iItem);

		return 0;
	}
	LRESULT OnCreate(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		CConfigValue cVal;
		CComBSTR bstrRootCfgID(t_pSpecs->pszRootConfigID);
		if (SUCCEEDED(m_pProfiles->ItemValueGet(bstrRootCfgID, &cVal)))
		{
			LONG nCount = cVal;
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, nCount);
			CComBSTR bstrProfileName(szNameID);
			CConfigValue cCount(nCount + 1L);
			TCHAR szName[64] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), t_pSpecs->uNewItem, szName, itemsof(szName), LANGIDFROMLCID(m_tLocaleID));
			CConfigValue cName(szName);
			BSTR aIDs[2];
			aIDs[0] = bstrRootCfgID;
			aIDs[1] = bstrProfileName;
			TConfigValue aVals[2];
			aVals[0] = cCount;
			aVals[1] = cName;
			m_pProfiles->ItemValuesSet(2, aIDs, aVals);

			HRSRC hLayouts = FindResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDR_DEFAULTMENU), _T("RT_CONFIG"));
			HGLOBAL hMem = LoadResource(_pModule->get_m_hInst(), hLayouts);
			DWORD nMemSize = SizeofResource(_pModule->get_m_hInst(), hLayouts);
			CComPtr<IConfigInMemory> pMemCfg;
			RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
			pMemCfg->DataBlockSet(nMemSize, static_cast<BYTE const*>(LockResource(hMem)));
			CComPtr<IConfig> pSubCfg;
			m_pProfiles->SubConfigGet(bstrProfileName, &pSubCfg);
			CConfigValue cItemID(__uuidof(MenuCommandsVector));
			CComBSTR cCFGID_MENUCOMMANDS(CFGID_MENUCOMMANDS);
			pSubCfg->ItemValuesSet(1, &(cCFGID_MENUCOMMANDS.m_str), cItemID);
			CComPtr<IConfig> pItemCfg;
			pSubCfg->SubConfigGet(cCFGID_MENUCOMMANDS, &pItemCfg);
			CopyConfigValues(pItemCfg, pMemCfg);

			FillList();
			m_wndList.SetItemState(nCount, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}

		return 0;
	}
	LRESULT OnClone(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			USES_CONVERSION;
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, nSel);
			CComPtr<IConfig> pViewCfg;
			if (SUCCEEDED(m_pProfiles->SubConfigGet(CComBSTR(szNameID), &pViewCfg)))
			{
				CConfigValue cVal;
				CComBSTR bstrRootCfgID(t_pSpecs->pszRootConfigID);
				if (SUCCEEDED(m_pProfiles->ItemValueGet(bstrRootCfgID, &cVal)))
				{
					CConfigValue cOrigName;
					m_pProfiles->ItemValueGet(CComBSTR(szNameID), &cOrigName);
					LONG nCount = cVal;
					_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, nCount);
					CComBSTR bstrProfileName(szNameID);
					CConfigValue cCount(nCount + 1L);
					TCHAR szName[256] = _T("");
					int nLen = Win32LangEx::LoadString(_pModule->get_m_hInst(), t_pSpecs->uCopyItem, szName, itemsof(szName), LANGIDFROMLCID(m_tLocaleID));
					_tcsncpy(szName+nLen, OLE2CT(cOrigName), itemsof(szName)-nLen);
					CConfigValue cName(szName);
					BSTR aIDs[2];
					aIDs[0] = bstrRootCfgID;
					aIDs[1] = bstrProfileName;
					TConfigValue aVals[2];
					aVals[0] = cCount;
					aVals[1] = cName;
					m_pProfiles->ItemValuesSet(2, aIDs, aVals);
					CComPtr<IConfig> pNewViewCfg;
					m_pProfiles->SubConfigGet(CComBSTR(szNameID), &pNewViewCfg);
					CopyConfigValues(pNewViewCfg, pViewCfg);

					FillList();
					m_wndList.SetItemState(nCount, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}
		}

		return 0;
	}
	LRESULT OnDelete(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			CConfigValue cVal;
			CComBSTR bstrRootCfgID(t_pSpecs->pszRootConfigID);
			if (SUCCEEDED(m_pProfiles->ItemValueGet(bstrRootCfgID, &cVal)))
			{
				LONG nCount = cVal;
				OLECHAR szNameID1[64];
				OLECHAR szNameID2[64];
				LONG i;
				for (i = nSel; i < (nCount-1); i++)
				{
					_snwprintf(szNameID1, itemsof(szNameID1), L"%s\\%08x", t_pSpecs->pszRootConfigID, i);
					CComBSTR bstrNameID1(szNameID1);
					_snwprintf(szNameID2, itemsof(szNameID2), L"%s\\%08x", t_pSpecs->pszRootConfigID, i+1);
					CComBSTR bstrNameID2(szNameID2);
					CConfigValue cName;
					m_pProfiles->ItemValueGet(bstrNameID2, &cName);
					m_pProfiles->ItemValuesSet(1, &(bstrNameID1.m_str), cName); // shit &CComBSTR
					CComPtr<IConfig> pCfg1;
					m_pProfiles->SubConfigGet(bstrNameID1, &pCfg1);
					CComPtr<IConfig> pCfg2;
					m_pProfiles->SubConfigGet(bstrNameID2, &pCfg2);
					CopyConfigValues(pCfg1, pCfg2);
				}
				m_pProfiles->ItemValuesSet(1, &(bstrRootCfgID.m_str), CConfigValue(nCount - 1L)); // shit &CComBSTR

				FillList();
				m_wndList.SetItemState(nSel == (nCount-1) ? nSel-1 : nSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}

		return 0;
	}
	LRESULT OnRename(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			m_wndList.SetFocus();
			m_wndList.EditLabel(nSel);
		}

		return 0;
	}
	LRESULT OnConfigure(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		ConfigureItem(m_wndList.GetSelectedIndex());

		return 0;
	}
	LRESULT OnMoveDown(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			CConfigValue cVal;
			CComBSTR bstrRootCfgID(t_pSpecs->pszRootConfigID);
			if (SUCCEEDED(m_pProfiles->ItemValueGet(bstrRootCfgID, &cVal)))
			{
				LONG nCount = cVal;
				if ((nCount-1) > nSel)
				{
					OLECHAR szNameID1[64];
					OLECHAR szNameID2[64];
					_snwprintf(szNameID1, itemsof(szNameID1), L"%s\\%08x", t_pSpecs->pszRootConfigID, nSel);
					CComBSTR bstrNameID1(szNameID1);
					_snwprintf(szNameID2, itemsof(szNameID2), L"%s\\%08x", t_pSpecs->pszRootConfigID, nSel+1);
					CComBSTR bstrNameID2(szNameID2);
					CConfigValue cName1;
					m_pProfiles->ItemValueGet(bstrNameID1, &cName1);
					CConfigValue cName2;
					m_pProfiles->ItemValueGet(bstrNameID2, &cName2);
					BSTR aIDs[2]; aIDs[0] = bstrNameID1; aIDs[1] = bstrNameID2;
					TConfigValue aVals[2]; aVals[0] = cName2; aVals[1] = cName1;
					m_pProfiles->ItemValuesSet(2, aIDs, aVals);
					CComPtr<IConfig> pCfg1;
					m_pProfiles->SubConfigGet(bstrNameID1, &pCfg1);
					CComPtr<IConfig> pCfg2;
					m_pProfiles->SubConfigGet(bstrNameID2, &pCfg2);
					CComPtr<IConfig> pCopy;
					pCfg1->DuplicateCreate(&pCopy);
					CopyConfigValues(pCfg1, pCfg2);
					CopyConfigValues(pCfg2, pCopy);
				}

				FillList();
				m_wndList.SetItemState(nSel+1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}

		return 0;
	}
	LRESULT OnMoveUp(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			CConfigValue cVal;
			CComBSTR bstrRootCfgID(t_pSpecs->pszRootConfigID);
			if (SUCCEEDED(m_pProfiles->ItemValueGet(bstrRootCfgID, &cVal)))
			{
				LONG nCount = cVal;
				if (nSel > 0)
				{
					OLECHAR szNameID1[64];
					OLECHAR szNameID2[64];
					_snwprintf(szNameID1, itemsof(szNameID1), L"%s\\%08x", t_pSpecs->pszRootConfigID, nSel);
					CComBSTR bstrNameID1(szNameID1);
					_snwprintf(szNameID2, itemsof(szNameID2), L"%s\\%08x", t_pSpecs->pszRootConfigID, nSel-1);
					CComBSTR bstrNameID2(szNameID2);
					CConfigValue cName1;
					m_pProfiles->ItemValueGet(bstrNameID1, &cName1);
					CConfigValue cName2;
					m_pProfiles->ItemValueGet(bstrNameID2, &cName2);
					BSTR aIDs[2]; aIDs[0] = bstrNameID1; aIDs[1] = bstrNameID2;
					TConfigValue aVals[2]; aVals[0] = cName2; aVals[1] = cName1;
					m_pProfiles->ItemValuesSet(2, aIDs, aVals);
					CComPtr<IConfig> pCfg1;
					m_pProfiles->SubConfigGet(bstrNameID1, &pCfg1);
					CComPtr<IConfig> pCfg2;
					m_pProfiles->SubConfigGet(bstrNameID2, &pCfg2);
					CComPtr<IConfig> pCopy;
					pCfg1->DuplicateCreate(&pCopy);
					CopyConfigValues(pCfg1, pCfg2);
					CopyConfigValues(pCfg2, pCopy);
				}

				FillList();
				m_wndList.SetItemState(nSel-1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}

		return 0;
	}
	LRESULT OnLoadFile(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(t_pSpecs->pszFileExtension);
		CComBSTR bstrFilter(L"*.");
		bstrFilter += bstrExt;
		pDocType->InitEx(_SharedStringTable.GetStringAuto(t_pSpecs->uIDSFileType), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
		CComPtr<IEnumUnknownsInit> pDocTypes;
		RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
		pDocTypes->Insert(pDocType);
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pStorage;
		pStMgr->FilterCreateInteractivelyUID(NULL, EFTOpenExisting, m_hWnd, pDocTypes, NULL, t_pSpecs->tFileCtxID, _SharedStringTable.GetStringAuto(t_pSpecs->uIDSLoadItem), NULL, m_tLocaleID, &pStorage);
		if (pStorage == NULL)
			return 0;

		CComBSTR bstrPath;
		pStorage->ToText(NULL, &bstrPath);
		if (bstrPath)
		{
			LPCOLESTR p1 = NULL;
			LPCOLESTR p2 = NULL;
			LPCOLESTR p;
			for (p = bstrPath; *p; ++p)
			{
				if (*p == L'\\')
					p1 = p2 = p+1;
				if (*p == L'.')
					p2 = p;
			}
			if (p1 && p1 != p2)
			{
				bstrPath.Attach(SysAllocStringLen(p1, p2-p1));
			}
		}
		if (bstrPath.Length() == 0)
		{
			OLECHAR szName[64] = L"";
			Win32LangEx::LoadStringW(_pModule->get_m_hInst(), t_pSpecs->uNewItem, szName, itemsof(szName), LANGIDFROMLCID(m_tLocaleID));
			bstrPath = szName;
		}
		CComPtr<IDataSrcDirect> pSrc;
		pStorage->SrcOpen(&pSrc);
		ULONG nSize = 0;
		if (pSrc == NULL || FAILED(pSrc->SizeGet(&nSize)) || nSize == 0)
		{
			// TODO: message
			return 0;
		}
		CDirectInputLock cData(pSrc, nSize);

		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		pMemCfg->DataBlockSet(nSize, cData.begin());

		CConfigValue cVal;
		CComBSTR bstrRootCfgID(t_pSpecs->pszRootConfigID);
		if (SUCCEEDED(m_pProfiles->ItemValueGet(bstrRootCfgID, &cVal)))
		{
			LONG nCount = cVal;
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, nCount);
			CComBSTR bstrProfileName(szNameID);
			CConfigValue cCount(nCount + 1L);
			BSTR aIDs[2];
			aIDs[0] = bstrRootCfgID;
			aIDs[1] = bstrProfileName;
			TConfigValue aVals[2];
			aVals[0] = cCount;
			aVals[1].eTypeID = ECVTString;
			aVals[1].bstrVal = bstrPath;
			m_pProfiles->ItemValuesSet(2, aIDs, aVals);
			CComPtr<IConfig> pDstCfg;
			m_pProfiles->SubConfigGet(bstrProfileName, &pDstCfg);
			CopyConfigValues(pDstCfg, pMemCfg);
			FillList();
			m_wndList.SetItemState(nCount, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}

		return 0;
	}
	LRESULT OnSaveFile(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel == -1)
			return 0;

		OLECHAR szNameID[64];
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, nSel);
		CComBSTR bstrNameID(szNameID);
		CConfigValue cName;
		m_pProfiles->ItemValueGet(bstrNameID, &cName);
		CComPtr<IConfig> pSrcCfg;
		m_pProfiles->SubConfigGet(bstrNameID, &pSrcCfg);
		if (pSrcCfg == NULL)
			return 0;

		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		CopyConfigValues(pMemCfg, pSrcCfg);
		CReturnedData dst;
		pMemCfg->TextBlockGet(&dst);
		if (dst.size() == 0)
			return 0;

		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(t_pSpecs->pszFileExtension);
		CComBSTR bstrFilter(L"*.");
		bstrFilter += bstrExt;
		pDocType->InitEx(_SharedStringTable.GetStringAuto(t_pSpecs->uIDSFileType), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
		CComPtr<IEnumUnknownsInit> pDocTypes;
		RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
		pDocTypes->Insert(pDocType);
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pStorage;
		pStMgr->FilterCreateInteractivelyUID(cName, EFTCreateNew, m_hWnd, pDocTypes, NULL, t_pSpecs->tFileCtxID, _SharedStringTable.GetStringAuto(t_pSpecs->uIDSSaveItem), NULL, m_tLocaleID, &pStorage);
		if (pStorage == NULL)
			return 0;
		CComPtr<IDataDstStream> pDst;
		pStorage->DstOpen(&pDst);
		if (pDst == NULL || FAILED(pDst->Write(dst.size(), dst.begin())) || FAILED(pDst->Close()))
		{
			// TODO: message
		}
		return 0;
	}

private:
	bool FillList()
	{
		int iSel = m_wndList.GetSelectedIndex();
		m_wndList.DeleteAllItems();
		m_cItemImages.RemoveAll();

		CConfigValue cVal;
		CConfigValue cValIcon;
		if (FAILED(m_pProfiles->ItemValueGet(CComBSTR(t_pSpecs->pszRootConfigID), &cVal)))
			return false;
		LONG nCount = cVal;

		for (LONG i = 0; i < nCount; i++)
		{
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, i);
			m_pProfiles->ItemValueGet(CComBSTR(szNameID), &cVal);
			wcscat(szNameID, L"\\IconID");
			m_pProfiles->ItemValueGet(CComBSTR(szNameID), &cValIcon);
			int nIcon = I_IMAGENONE;
			if (cValIcon.TypeGet() == ECVTGUID && !IsEqualGUID(cValIcon, GUID_NULL))
			{
				HICON hIcon = NULL;
				m_pIcons->GetIcon(cValIcon, XPGUI::GetSmallIconSize(), &hIcon);
				if (hIcon)
				{
					m_cItemImages.AddIcon(hIcon);
					nIcon = m_cItemImages.GetImageCount()-1;
					DestroyIcon(hIcon);
				}
			}
			m_wndList.AddItem(i, 0, COLE2CT(cVal), nIcon);
		}
		if (iSel > 0 && iSel < nCount)
			m_wndList.SelectItem(iSel);

		return true;
	}
	bool ConfigureItem(LONG a_nIndex)
	{
		OLECHAR szNameID[64];
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", t_pSpecs->pszRootConfigID, a_nIndex);
		CComPtr<IConfig> pItemCfg;
		if (FAILED(m_pProfiles->SubConfigGet(CComBSTR(szNameID), &pItemCfg)))
			return false;
		CConfigValue cName;
		m_pProfiles->ItemValueGet(CComBSTR(szNameID), &cName);

		CComPtr<IConfig> pCopy;
		if (FAILED(pItemCfg->DuplicateCreate(&pCopy)))
			return false;

		if (TConfigureItemDlg(pCopy, m_pGUIConfig, m_tLocaleID, COLE2CT(cName.operator BSTR())).DoModal(m_hWnd) == IDOK)
		{
			CopyConfigValues(pItemCfg, pCopy);
			FillList();
		}

		return true;
	}

private:
	CComPtr<IConfig> m_pProfiles;
	CComPtr<IConfig> m_pGUIConfig;
	CListViewCtrl m_wndList;
	CToolBarCtrl m_wndToolBar;
	CImageList m_cCommandImages;
	CImageList m_cItemImages;
	CComPtr<IDesignerFrameIcons> m_pIcons;
	HICON m_hIcon;
};


HICON GetIconLayouts(ULONG size);
HICON GetIconLayoutNew(ULONG size);
HICON GetIconLayoutEdit(ULONG size);
HICON GetIconLayoutDelete(ULONG size);
HICON GetIconLayoutDuplicate(ULONG size);
HICON GetIconOpenFile(ULONG size);
HICON GetIconSaveFile(ULONG size);
HICON GetIconRename(ULONG size);
HICON GetIconMoveDown(ULONG size);
HICON GetIconMoveUp(ULONG size);

extern __declspec(selectany) SManageItemsSpecs const g_ManageLayouts =
{
	_T("http://www.rw-designer.com/manage-layouts"),

	GetIconLayouts,
	IDS_MANAGELAYOUTS_CAPTION,
	IDS_MANAGELAYOUTS_LISTLABEL,
	IDS_HELP_MANAGE_LAYOUTLIST,

	GetIconLayoutNew,
	GetIconLayoutDuplicate,
	GetIconLayoutDelete,
	GetIconRename,
	GetIconLayoutEdit,
	GetIconMoveDown,
	GetIconMoveUp,
	GetIconOpenFile,
	GetIconSaveFile,

	IDS_NEWLAYOUT,
	IDS_CLONEDVIEWPROFILE,

	IDS_MNGLTS_BUTTONTOOLTIPS,

	CFGID_DLGS_MANAGELAYOUTS_SIZEX,
	CFGID_DLGS_MANAGELAYOUTS_SIZEY,

	CFGID_VIEWPROFILES,

	true,

	IDS_LAYOUT_OPEN,
	IDS_LAYOUT_SAVE,
	IDS_LAYOUT_FILETYPE,
	L"rwlayout",
	{0xdff53f0, 0xb05e, 0x4577, {0xa8, 0x45, 0x5c, 0xdc, 0x46, 0x41, 0xc6, 0xf6}},
};

#include "ConfigFrameDlg.h"

typedef CManageConfigurableItemsDlg<&g_ManageLayouts, CConfigureLayoutDlg> CManageLayoutsDlg;

