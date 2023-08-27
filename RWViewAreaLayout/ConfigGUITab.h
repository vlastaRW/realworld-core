#pragma once

#include "resource.h"
#include "RWViewAreaLayout.h"

#include <Win32LangEx.h>
#include <ContextHelpDlg.h>
#include <XPGUI.h>
#include "ConfigIDsTab.h"
#include <ConfigCustomGUIImpl.h>
#include <IconRenderer.h>


class ATL_NO_VTABLE CConfigGUITabDlg :
	public CCustomConfigWndImpl<CConfigGUITabDlg>,
	public CDialogResize<CConfigGUITabDlg>
{
public:
	~CConfigGUITabDlg()
	{
		m_cCommandImages.Destroy();
	}

	enum
	{
		IDD = IDD_CONFIGGUI_TAB,
		ID_CLONE = 200,
		ID_CONFIGURE,
		ID_CREATE,
		ID_DELETE,
		ID_MOVEDOWN,
		ID_MOVEUP,
		ID_RENAME,
		IDC_TAB_CFG = IDC_TAB_LIST+10,
	};

	BEGIN_COM_MAP(CConfigGUITabDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
	END_COM_MAP()

	BEGIN_MSG_MAP(CConfigGUITabDlg)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUITabDlg>)
		COMMAND_ID_HANDLER(ID_CLONE, OnClone)
		COMMAND_ID_HANDLER(ID_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_MOVEDOWN, OnMoveDown)
		COMMAND_ID_HANDLER(ID_MOVEUP, OnMoveUp)
		COMMAND_ID_HANDLER(ID_RENAME, OnRename)
		NOTIFY_HANDLER(IDC_TAB_LIST, LVN_ENDLABELEDIT, OnOperationListRename)
		NOTIFY_HANDLER(IDC_TAB_LIST, LVN_ITEMCHANGED, OnOperationListItemChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUITabDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUITabDlg)
		DLGRESIZE_CONTROL(IDC_TAB_LIST, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_TAB_CFG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_TAB_POSITION, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_TAB_ID, DLSZ_DIVSIZE_X(2)|DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_TAB_IDLABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_TAB_ACTIVE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TAB_ACTIVELABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TAB_HIDE, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUITabDlg)
		CONFIGITEM_EDITBOX(IDC_TAB_ACTIVE, CFGID_TABS_ACTIVEINDEX)
		CONFIGITEM_EDITBOX(IDC_TAB_ID, CFGID_TABS_TABID)
		CONFIGITEM_COMBOBOX(IDC_TAB_POSITION, CFGID_TABS_HEADERPOS)
		CONFIGITEM_CHECKBOX(IDC_TAB_HIDE, CFGID_TABS_INVISIBLE)
	END_CONFIGITEM_MAP()

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		static TBBUTTON const aButtons[] =
		{
			{0, ID_CREATE, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
			{1, ID_CLONE, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 1},
			{2, ID_DELETE, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{3, ID_RENAME, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 3},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{4, ID_MOVEDOWN, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 4},
			{5, ID_MOVEUP, 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 5},
		};
		// load tooltip strings (IE 5.01 is required)
		TCHAR szTooltipStrings[1024] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_ITEMLIST_BUTTONS, szTooltipStrings, itemsof(szTooltipStrings), LANGIDFROMLCID(m_tLocaleID));
		for (TCHAR* p = szTooltipStrings; *p; ++p) if (*p == _T('|')) *p = _T('\0');

		// load toolbar images
		int nIconSize = XPGUI::GetSmallIconSize();
		m_cCommandImages.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 6, 1);
		HICON h;
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.8f));
			m_cCommandImages.AddIcon(h = cRenderer.get());
			DestroyIcon(h);
		}
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIDuplicate, cRenderer, IRTarget(0.8f));
			m_cCommandImages.AddIcon(h = cRenderer.get());
			DestroyIcon(h);
		}
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
			m_cCommandImages.AddIcon(h = cRenderer.get());
			DestroyIcon(h);
		}
		{
			static IRPolyPoint const box[] =
			{
				{0, 80}, {256, 80}, {256, 176}, {0, 176},
			};
			static IRPolyPoint const cursor[] =
			{
				{152, 48}, {168, 48}, {168, 208}, {152, 208}, {152, 232}, {208, 232}, {208, 208}, {192, 208}, {192, 48}, {208, 48}, {208, 24}, {152, 24},
			};
			static IRGridItem const boxGridX[] = {{0, 0}, {0, 256}};
			static IRGridItem const boxGridY[] = {{0, 80}, {0, 176}};
			static IRCanvas const canvasBox = {0, 0, 256, 256, itemsof(boxGridX), itemsof(boxGridY), boxGridX, boxGridY};
			static IRGridItem const cursorGridX[] = {{0, 152}, {0, 168}, {0, 192}, {0, 208}};
			static IRGridItem const cursorGridY[] = {{0, 24}, {0, 48}, {0, 208}, {0, 232}};
			static IRCanvas const canvasCursor = {0, 0, 256, 256, itemsof(cursorGridX), itemsof(cursorGridY), cursorGridX, cursorGridY};

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(nIconSize);
			cRenderer(&canvasBox, itemsof(box), box, pSI->GetMaterial(ESMInterior));
			cRenderer(&canvasCursor, itemsof(cursor), cursor, pSI->GetMaterial(ESMInterior));
			m_cCommandImages.AddIcon(h = cRenderer.get());
			DestroyIcon(h);
		}
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIMoveDown, cRenderer);
			m_cCommandImages.AddIcon(h = cRenderer.get());
			DestroyIcon(h);
		}
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIMoveUp, cRenderer);
			m_cCommandImages.AddIcon(h = cRenderer.get());
			DestroyIcon(h);
		}

		m_wndToolBar = GetDlgItem(IDC_TAB_TOOLBAR);
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cCommandImages);
		m_wndToolBar.AddStrings(szTooltipStrings);
		m_wndToolBar.AddButtons(itemsof(aButtons), const_cast<TBBUTTON*>(aButtons));

		// make the list same width as the toolbar
		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcTmp;
		m_wndToolBar.GetWindowRect(&rcTmp);
		ScreenToClient(&rcTmp);
		rcToolbar.left = rcTmp.left;
		rcToolbar.right += rcTmp.left;
		rcToolbar.top = rcTmp.top;
		rcToolbar.bottom += rcTmp.top;
		m_wndToolBar.MoveWindow(&rcToolbar);

		m_wndList = GetDlgItem(IDC_TAB_LIST);
		m_wndList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
		RECT rcList;
		m_wndList.GetWindowRect(&rcList);
		ScreenToClient(&rcList);
		rcList.right = rcToolbar.right;
		rcList.top = rcToolbar.bottom;
		m_wndList.MoveWindow(&rcList);
		m_wndList.GetClientRect(&rcTmp);
		m_wndList.InsertColumn(0, _T(""), LVCFMT_LEFT, rcTmp.right-GetSystemMetrics(SM_CXVSCROLL), 0);

		RECT rcMap = {0, 0, 7, 7};
		MapDialogRect(&rcMap);
		// create subwindow
		RECT rcCfg;
		GetClientRect(&rcCfg);
		rcCfg.left = rcToolbar.right+rcMap.right;
		rcCfg.top = rcToolbar.top;
		RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
		m_pConfigWnd->Create(m_hWnd, &rcCfg, IDC_TAB_CFG, m_tLocaleID, TRUE, ECWBMMarginAndOutline);

		// init controls
		FillList();
		m_wndList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		DlgResize_Init(false, false, 0);

		return TRUE;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CDialogResize<CConfigGUITabDlg>::OnSize(a_uMsg, a_wParam, a_lParam, a_bHandled);

		RECT rcTmp;
		m_wndList.GetClientRect(&rcTmp);
		m_wndList.SetColumnWidth(0, rcTmp.right-GetSystemMetrics(SM_CXVSCROLL));

		return 0;
	}

	LRESULT OnOperationListRename(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(a_pNMHDR);

		if (pDispInfo->item.pszText)
		{
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_TABS_ITEMS, pDispInfo->item.iItem);
			CComBSTR bstrNameID(szNameID);
			BSTR aIDs[1];
			aIDs[0] = bstrNameID;
			M_Config()->ItemValuesSet(1, aIDs, CConfigValue(pDispInfo->item.pszText));
			FillList();
		}
		return 0;
	}
	LRESULT OnOperationListItemChanged(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		int nCount = m_wndList.GetItemCount();
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel == -1)
		{
			m_wndToolBar.EnableButton(ID_CLONE, FALSE);
			m_wndToolBar.EnableButton(ID_CONFIGURE, FALSE);
			m_wndToolBar.EnableButton(ID_CREATE, TRUE);
			m_wndToolBar.EnableButton(ID_DELETE, FALSE);
			m_wndToolBar.EnableButton(ID_MOVEDOWN, FALSE);
			m_wndToolBar.EnableButton(ID_MOVEUP, FALSE);
			m_wndToolBar.EnableButton(ID_RENAME, FALSE);
			m_pConfigWnd->ConfigSet(NULL, M_Mode());
		}
		else
		{
			m_wndToolBar.EnableButton(ID_CLONE, TRUE);
			m_wndToolBar.EnableButton(ID_CONFIGURE, TRUE);
			m_wndToolBar.EnableButton(ID_CREATE, TRUE);
			m_wndToolBar.EnableButton(ID_DELETE, TRUE);
			m_wndToolBar.EnableButton(ID_MOVEDOWN, nSel < (nCount-1));
			m_wndToolBar.EnableButton(ID_MOVEUP, nSel > 0);
			m_wndToolBar.EnableButton(ID_RENAME, TRUE);

			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_TABS_ITEMS, nSel);
			CComPtr<IConfig> pOperationCfg;
			if (FAILED(M_Config()->SubConfigGet(CComBSTR(szNameID), &pOperationCfg)))
				return 0;

			m_pConfigWnd->ConfigSet(pOperationCfg, M_Mode());
		}

		return 0;
	}
	LRESULT OnClone(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			USES_CONVERSION;
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_TABS_ITEMS, nSel);
			CComPtr<IConfig> pViewCfg;
			if (SUCCEEDED(M_Config()->SubConfigGet(CComBSTR(szNameID), &pViewCfg)))
			{
				CConfigValue cVal;
				CComBSTR cCFGID_TABS_ITEMS(CFGID_TABS_ITEMS);
				if (SUCCEEDED(M_Config()->ItemValueGet(cCFGID_TABS_ITEMS, &cVal)))
				{
					CConfigValue cOrigName;
					M_Config()->ItemValueGet(CComBSTR(szNameID), &cOrigName);
					LONG nCount = cVal;
					_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_TABS_ITEMS, nCount);
					CComBSTR bstrProfileName(szNameID);
					CConfigValue cCount(nCount + 1L);
					TCHAR szName[256] = _T("");
					int nLen = Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CLONEDSUBITEM, szName, itemsof(szName), LANGIDFROMLCID(m_tLocaleID));
					_tcsncpy(szName+nLen, OLE2CT(cOrigName), itemsof(szName)-nLen);
					CConfigValue cName(szName);
					BSTR aIDs[2];
					aIDs[0] = cCFGID_TABS_ITEMS;
					aIDs[1] = bstrProfileName;
					TConfigValue aVals[2];
					aVals[0] = cCount;
					aVals[1] = cName;
					M_Config()->ItemValuesSet(2, aIDs, aVals);
					CComPtr<IConfig> pNewViewCfg;
					M_Config()->SubConfigGet(CComBSTR(szNameID), &pNewViewCfg);
					CopyConfigValues(pNewViewCfg, pViewCfg);

					FillList();
					m_wndList.SetItemState(nCount, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}
		}

		return 0;
	}
	LRESULT OnCreate(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CConfigValue cVal;
		CComBSTR cCFGID_TABS_ITEMS(CFGID_TABS_ITEMS);
		if (SUCCEEDED(M_Config()->ItemValueGet(cCFGID_TABS_ITEMS, &cVal)))
		{
			LONG nCount = cVal;
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_TABS_ITEMS, nCount);
			CComBSTR bstrProfileName(szNameID);
			CConfigValue cCount(nCount + 1L);
			TCHAR szName[64] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_NEWSUBWINDOW, szName, itemsof(szName), LANGIDFROMLCID(m_tLocaleID));
			CConfigValue cName(szName);
			BSTR aIDs[2];
			aIDs[0] = cCFGID_TABS_ITEMS;
			aIDs[1] = bstrProfileName;
			TConfigValue aVals[2];
			aVals[0] = cCount;
			aVals[1] = cName;
			M_Config()->ItemValuesSet(2, aIDs, aVals);
			FillList();
			m_wndList.SetItemState(nCount, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
		return 0;
	}
	LRESULT OnDelete(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			CConfigValue cVal;
			CComBSTR cCFGID_TABS_ITEMS(CFGID_TABS_ITEMS);
			if (SUCCEEDED(M_Config()->ItemValueGet(cCFGID_TABS_ITEMS, &cVal)))
			{
				LONG nCount = cVal;
				OLECHAR szNameID1[64];
				OLECHAR szNameID2[64];
				LONG i;
				for (i = nSel; i < (nCount-1); i++)
				{
					_snwprintf(szNameID1, itemsof(szNameID1), L"%s\\%08x", CFGID_TABS_ITEMS, i);
					CComBSTR bstrNameID1(szNameID1);
					_snwprintf(szNameID2, itemsof(szNameID2), L"%s\\%08x", CFGID_TABS_ITEMS, i+1);
					CComBSTR bstrNameID2(szNameID2);
					CConfigValue cName;
					M_Config()->ItemValueGet(bstrNameID2, &cName);
					M_Config()->ItemValuesSet(1, &(bstrNameID1.m_str), cName);
					CComPtr<IConfig> pCfg1;
					M_Config()->SubConfigGet(bstrNameID1, &pCfg1);
					CComPtr<IConfig> pCfg2;
					M_Config()->SubConfigGet(bstrNameID2, &pCfg2);
					CopyConfigValues(pCfg1, pCfg2);
				}
				M_Config()->ItemValuesSet(1, &(cCFGID_TABS_ITEMS.m_str), CConfigValue(nCount - 1L));

				FillList();
				m_wndList.SetItemState(nSel == (nCount-1) ? nSel-1 : nSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}

		return 0;
	}
	LRESULT OnMoveDown(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			CConfigValue cVal;
			CComBSTR cCFGID_TABS_ITEMS(CFGID_TABS_ITEMS);
			if (SUCCEEDED(M_Config()->ItemValueGet(cCFGID_TABS_ITEMS, &cVal)))
			{
				LONG nCount = cVal;
				if ((nCount-1) > nSel)
				{
					OLECHAR szNameID1[64];
					OLECHAR szNameID2[64];
					_snwprintf(szNameID1, itemsof(szNameID1), L"%s\\%08x", CFGID_TABS_ITEMS, nSel);
					CComBSTR bstrNameID1(szNameID1);
					_snwprintf(szNameID2, itemsof(szNameID2), L"%s\\%08x", CFGID_TABS_ITEMS, nSel+1);
					CComBSTR bstrNameID2(szNameID2);
					CConfigValue cName1;
					M_Config()->ItemValueGet(bstrNameID1, &cName1);
					CConfigValue cName2;
					M_Config()->ItemValueGet(bstrNameID2, &cName2);
					BSTR aIDs[2]; aIDs[0] = bstrNameID1; aIDs[1] = bstrNameID2;
					TConfigValue aVals[2]; aVals[0] = cName2; aVals[1] = cName1;
					M_Config()->ItemValuesSet(2, aIDs, aVals);
					CComPtr<IConfig> pCfg1;
					M_Config()->SubConfigGet(bstrNameID1, &pCfg1);
					CComPtr<IConfig> pCfg2;
					M_Config()->SubConfigGet(bstrNameID2, &pCfg2);
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
	LRESULT OnMoveUp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			CConfigValue cVal;
			CComBSTR cCFGID_TABS_ITEMS(CFGID_TABS_ITEMS);
			if (SUCCEEDED(M_Config()->ItemValueGet(cCFGID_TABS_ITEMS, &cVal)))
			{
				LONG nCount = cVal;
				if (nSel > 0)
				{
					OLECHAR szNameID1[64];
					OLECHAR szNameID2[64];
					_snwprintf(szNameID1, itemsof(szNameID1), L"%s\\%08x", CFGID_TABS_ITEMS, nSel);
					CComBSTR bstrNameID1(szNameID1);
					_snwprintf(szNameID2, itemsof(szNameID2), L"%s\\%08x", CFGID_TABS_ITEMS, nSel-1);
					CComBSTR bstrNameID2(szNameID2);
					CConfigValue cName1;
					M_Config()->ItemValueGet(bstrNameID1, &cName1);
					CConfigValue cName2;
					M_Config()->ItemValueGet(bstrNameID2, &cName2);
					BSTR aIDs[2]; aIDs[0] = bstrNameID1; aIDs[1] = bstrNameID2;
					TConfigValue aVals[2]; aVals[0] = cName2; aVals[1] = cName1;
					M_Config()->ItemValuesSet(2, aIDs, aVals);
					CComPtr<IConfig> pCfg1;
					M_Config()->SubConfigGet(bstrNameID1, &pCfg1);
					CComPtr<IConfig> pCfg2;
					M_Config()->SubConfigGet(bstrNameID2, &pCfg2);
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
	LRESULT OnRename(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel != -1)
		{
			m_wndList.EditLabel(nSel);
		}

		return 0;
	}

private:
	bool FillList()
	{
		m_wndList.DeleteAllItems();

		CConfigValue cVal;
		if (FAILED(M_Config()->ItemValueGet(CComBSTR(CFGID_TABS_ITEMS), &cVal)))
			return false;
		LONG nCount = cVal;

		USES_CONVERSION;
		LONG i;
		for (i = 0; i < nCount; i++)
		{
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_TABS_ITEMS, i);
			M_Config()->ItemValueGet(CComBSTR(szNameID), &cVal);
			m_wndList.AddItem(i, 0, OLE2CT(cVal));
		}

		return true;
	}

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
	CListViewCtrl m_wndList;
	CToolBarCtrl m_wndToolBar;
	CImageList m_cCommandImages;
};
