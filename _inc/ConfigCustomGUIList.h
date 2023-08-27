
#pragma once

#include <Win32LangEx.h>
#include <XPGUI.h>
#include <ConfigCustomGUIImpl.h>
#include <IconRenderer.h>

#define CCGUILIST_DEFAULTICON 0xffffffff
template<class T, UINT t_uIDCList, UINT t_uIDCToolbar, UINT t_uIDCSubconfig, UINT t_uIDSTooltips, UINT t_uIDSNewItemName, UINT t_uIDSClonePefix, UINT t_IDICreate, UINT t_IDIDuplicate, UINT t_IDIDelete, UINT t_IDIRename, UINT t_IDIMoveUp, UINT t_IDIMoveDown, EConfigWindowBorderMode t_eMargins = ECWBMMarginAndOutline>
class CCustomConfigGUIList
{
public:
	typedef CCustomConfigGUIList<T, t_uIDCList, t_uIDCToolbar, t_uIDCSubconfig, t_uIDSTooltips, t_uIDSNewItemName, t_uIDSClonePefix, t_IDICreate, t_IDIDuplicate, t_IDIDelete, t_IDIRename, t_IDIMoveUp, t_IDIMoveDown, t_eMargins> ListClass;
	CCustomConfigGUIList(LPCOLESTR const a_pszCfgID) : m_pszCfgID(a_pszCfgID), m_pSelected(NULL), m_bUpdating(false) {}
	~CCustomConfigGUIList()
	{
		m_cCommandImages.Destroy();
	}

	enum
	{
		ID_CLONE = 2000,
		ID_CREATE,
		ID_DELETE,
		ID_MOVEDOWN,
		ID_MOVEUP,
		ID_RENAME,
	};

	BEGIN_MSG_MAP(ListClass)
		COMMAND_ID_HANDLER(ID_CLONE, OnClone)
		COMMAND_ID_HANDLER(ID_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_MOVEDOWN, OnMoveDown)
		COMMAND_ID_HANDLER(ID_MOVEUP, OnMoveUp)
		COMMAND_ID_HANDLER(ID_RENAME, OnRename)
		NOTIFY_HANDLER(t_uIDCList, LVN_ENDLABELEDIT, OnOperationListRename)
		NOTIFY_HANDLER(t_uIDCList, LVN_ITEMCHANGED, OnOperationListItemChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		TBBUTTON aButtons[8];
		ZeroMemory(aButtons, sizeof aButtons);
		int nButtons = 0;
		// load tooltip strings (IE 5.01 is required)
		TCHAR szTooltipStrings[1024] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), t_uIDSTooltips, szTooltipStrings, itemsof(szTooltipStrings), LANGIDFROMLCID(static_cast<T*>(this)->m_tLocaleID));
		for (TCHAR* p = szTooltipStrings; *p; ++p) if (*p == _T('|')) *p = _T('\0');

		// load toolbar images
		int nIconSize = XPGUI::GetSmallIconSize();
		int nIconDelta = (nIconSize>>1)-8;
		m_cCommandImages.Create(nIconSize+nIconDelta, nIconSize, XPGUI::GetImageListColorFlags(), 6, 1);
		RECT padding = {nIconDelta>>1, 0, nIconDelta-(nIconDelta>>1), 0};
		int i = 0;
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		if (t_IDICreate)
		{
			HICON h = NULL;
			if (t_IDICreate == CCGUILIST_DEFAULTICON)
			{
				CIconRendererReceiver cRenderer(nIconSize);
				pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.8f));
				h = cRenderer.get(padding);
			}
			else
				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_IDICreate), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			m_cCommandImages.AddIcon(h);
			DestroyIcon(h);
			aButtons[nButtons].iBitmap = aButtons[nButtons].iString = i;
			aButtons[nButtons].idCommand = ID_CREATE;
			aButtons[nButtons].fsState = TBSTATE_ENABLED;
			aButtons[nButtons].fsStyle = BTNS_BUTTON;
			++nButtons; ++i;
		}
		if (t_IDIDuplicate)
		{
			HICON h = NULL;
			if (t_IDIDuplicate == CCGUILIST_DEFAULTICON)
			{
				CIconRendererReceiver cRenderer(nIconSize);
				pSI->GetLayers(ESIDuplicate, cRenderer, IRTarget(0.8f));
				h = cRenderer.get(padding);
			}
			else
				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_IDIDuplicate), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			m_cCommandImages.AddIcon(h);
			DestroyIcon(h);
			aButtons[nButtons].iBitmap = aButtons[nButtons].iString = i;
			aButtons[nButtons].idCommand = ID_CLONE;
			aButtons[nButtons].fsState = TBSTATE_ENABLED;
			aButtons[nButtons].fsStyle = BTNS_BUTTON;
			++nButtons; ++i;
		}
		if (t_IDIDelete)
		{
			HICON h = NULL;
			if (t_IDIDelete == CCGUILIST_DEFAULTICON)
			{
				CIconRendererReceiver cRenderer(nIconSize);
				pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
				h = cRenderer.get(padding);
			}
			else
				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_IDIDelete), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			m_cCommandImages.AddIcon(h);
			DestroyIcon(h);
			aButtons[nButtons].iBitmap = aButtons[nButtons].iString = i;
			aButtons[nButtons].idCommand = ID_DELETE;
			aButtons[nButtons].fsState = TBSTATE_ENABLED;
			aButtons[nButtons].fsStyle = BTNS_BUTTON;
			++nButtons; ++i;
		}
		if (t_IDIRename)
		{
			if (i)
			{
				aButtons[nButtons++].fsStyle = BTNS_SEP;
			}
			HICON h = NULL;
			if (t_IDICreate == CCGUILIST_DEFAULTICON)
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
				h =  cRenderer.get(padding);
			}
			else
				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_IDIRename), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			m_cCommandImages.AddIcon(h);
			DestroyIcon(h);
			aButtons[nButtons].iBitmap = aButtons[nButtons].iString = i;
			aButtons[nButtons].idCommand = ID_RENAME;
			aButtons[nButtons].fsState = TBSTATE_ENABLED;
			aButtons[nButtons].fsStyle = BTNS_BUTTON;
			++nButtons; ++i;
		}
		if (i && (t_IDIMoveUp || t_IDIMoveDown))
		{
			aButtons[nButtons++].fsStyle = BTNS_SEP;
		}
		if (t_IDIMoveUp)
		{
			HICON h = NULL;
			if (t_IDIMoveUp == CCGUILIST_DEFAULTICON)
			{
				CIconRendererReceiver cRenderer(nIconSize);
				pSI->GetLayers(ESIMoveUp, cRenderer);
				h = cRenderer.get(padding);
			}
			else
				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_IDIMoveUp), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			m_cCommandImages.AddIcon(h);
			DestroyIcon(h);
			aButtons[nButtons].iBitmap = aButtons[nButtons].iString = i;
			aButtons[nButtons].idCommand = ID_MOVEUP;
			aButtons[nButtons].fsState = TBSTATE_ENABLED;
			aButtons[nButtons].fsStyle = BTNS_BUTTON;
			++nButtons; ++i;
		}
		if (t_IDIMoveDown)
		{
			HICON h = NULL;
			if (t_IDIMoveDown == CCGUILIST_DEFAULTICON)
			{
				CIconRendererReceiver cRenderer(nIconSize);
				pSI->GetLayers(ESIMoveDown, cRenderer);
				h = cRenderer.get(padding);
				//if (hLib == NULL) hLib = LoadLibraryEx(_T("RWDesignerServices.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
				//if (hLib) h = (HICON)::LoadImage(hLib, MAKEINTRESOURCE(223), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			}
			else
				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_IDIMoveDown), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
			m_cCommandImages.AddIcon(h);
			DestroyIcon(h);
			aButtons[nButtons].iBitmap = aButtons[nButtons].iString = i;
			aButtons[nButtons].idCommand = ID_MOVEDOWN;
			aButtons[nButtons].fsState = TBSTATE_ENABLED;
			aButtons[nButtons].fsStyle = BTNS_BUTTON;
			++nButtons; ++i;
		}

		m_wndToolBar = static_cast<T*>(this)->GetDlgItem(t_uIDCToolbar);
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cCommandImages);
		m_wndToolBar.AddStrings(szTooltipStrings);
		m_wndToolBar.AddButtons(nButtons, aButtons);
		m_wndToolBar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);

		// move the list below the toolbar
		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcTmp;
		m_wndToolBar.GetWindowRect(&rcTmp);
		static_cast<T*>(this)->ScreenToClient(&rcTmp);
		rcToolbar.left = rcTmp.left;
		rcToolbar.right = rcTmp.right;
		rcToolbar.top = rcTmp.top;
		rcToolbar.bottom += rcTmp.top;
		m_wndToolBar.MoveWindow(&rcToolbar);

		m_wndList = static_cast<T*>(this)->GetDlgItem(t_uIDCList);
		m_wndList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
		RECT rcList;
		m_wndList.GetWindowRect(&rcList);
		static_cast<T*>(this)->ScreenToClient(&rcList);
		rcList.top = rcToolbar.bottom;
		m_wndList.MoveWindow(&rcList);
		m_wndList.GetClientRect(&rcTmp);
		m_wndList.InsertColumn(0, _T(""), LVCFMT_LEFT, rcTmp.right-GetSystemMetrics(SM_CXVSCROLL), 0);

		// subconfig
		CWindow wnd(static_cast<T*>(this)->GetDlgItem(t_uIDCSubconfig));
		RECT rc;
		wnd.GetWindowRect(&rc);
		static_cast<T*>(this)->ScreenToClient(&rc);
		RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
		m_pConfigWnd->Create(static_cast<T*>(this)->m_hWnd, &rc, t_uIDCSubconfig, static_cast<T*>(this)->m_tLocaleID, TRUE, t_eMargins);
		RWHWND h = NULL;
		m_pConfigWnd->Handle(&h);
		if (h)
		{
			::SetWindowPos(h, wnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
		}
		wnd.DestroyWindow();

		// init controls
		UpdateList();
		m_wndList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		a_bHandled = FALSE;

		return TRUE;
	}

	LRESULT OnOperationListRename(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(a_pNMHDR);

		if (pDispInfo->item.pszText)
		{
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", m_pszCfgID, pDispInfo->item.iItem);
			CComBSTR bstrNameID(szNameID);
			BSTR aIDs[1];
			aIDs[0] = bstrNameID;
			static_cast<T*>(this)->M_Config()->ItemValuesSet(1, aIDs, CConfigValue(pDispInfo->item.pszText));
		}
		return 0;
	}
	LRESULT OnOperationListItemChanged(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		if (m_bUpdating)
			return 0;
		int nCount = m_wndList.GetItemCount();
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel == -1)
		{
			m_wndToolBar.EnableButton(ID_CLONE, FALSE);
			m_wndToolBar.EnableButton(ID_CREATE, TRUE);
			m_wndToolBar.EnableButton(ID_DELETE, FALSE);
			m_wndToolBar.EnableButton(ID_MOVEDOWN, FALSE);
			m_wndToolBar.EnableButton(ID_MOVEUP, FALSE);
			m_wndToolBar.EnableButton(ID_RENAME, FALSE);
			m_pConfigWnd->ConfigSet(NULL, ECPMFull);
			m_pSelected = NULL;
		}
		else
		{
			m_wndToolBar.EnableButton(ID_CLONE, TRUE);
			m_wndToolBar.EnableButton(ID_CREATE, TRUE);
			m_wndToolBar.EnableButton(ID_DELETE, TRUE);
			m_wndToolBar.EnableButton(ID_MOVEDOWN, nSel < (nCount-1));
			m_wndToolBar.EnableButton(ID_MOVEUP, nSel > 0);
			m_wndToolBar.EnableButton(ID_RENAME, TRUE);

			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", m_pszCfgID, nSel);
			CComPtr<IConfig> pOperationCfg;
			if (FAILED(static_cast<T*>(this)->M_Config()->SubConfigGet(CComBSTR(szNameID), &pOperationCfg)))
				return 0;

			m_pSelected = pOperationCfg;
			m_pConfigWnd->ConfigSet(pOperationCfg, ECPMFull);
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
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", m_pszCfgID, nSel);
			CComPtr<IConfig> pViewCfg;
			if (SUCCEEDED(static_cast<T*>(this)->M_Config()->SubConfigGet(CComBSTR(szNameID), &pViewCfg)))
			{
				CConfigValue cVal;
				CComBSTR cm_pszCfgID(m_pszCfgID);
				if (SUCCEEDED(static_cast<T*>(this)->M_Config()->ItemValueGet(cm_pszCfgID, &cVal)))
				{
					CConfigValue cOrigName;
					static_cast<T*>(this)->M_Config()->ItemValueGet(CComBSTR(szNameID), &cOrigName);
					LONG nCount = cVal;
					_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", m_pszCfgID, nCount);
					CComBSTR bstrProfileName(szNameID);
					CConfigValue cCount(nCount + 1L);
					TCHAR szName[256] = _T("");
					if (t_uIDSClonePefix)
					{
						int nLen = Win32LangEx::LoadString(_pModule->get_m_hInst(), t_uIDSClonePefix, szName, itemsof(szName), LANGIDFROMLCID(static_cast<T*>(this)->m_tLocaleID));
						_tcsncpy(szName+nLen, OLE2CT(cOrigName), itemsof(szName)-nLen);
					}
					CConfigValue cName(szName);
					BSTR aIDs[2];
					aIDs[0] = cm_pszCfgID;
					aIDs[1] = bstrProfileName;
					TConfigValue aVals[2];
					aVals[0] = cCount;
					aVals[1] = cName;
					static_cast<T*>(this)->M_Config()->ItemValuesSet(t_uIDSClonePefix ? 2 : 1, aIDs, aVals);
					CComPtr<IConfig> pNewViewCfg;
					static_cast<T*>(this)->M_Config()->SubConfigGet(CComBSTR(szNameID), &pNewViewCfg);
					CopyConfigValues(pNewViewCfg, pViewCfg);
					CComPtr<IConfig> pCfg;
					static_cast<T*>(this)->M_Config()->SubConfigGet(cm_pszCfgID, &pCfg);
					CComQIPtr<IConfigVector> pVecCfg(pCfg);
					if (nSel != nCount-1)
						pVecCfg->Move(nCount, nSel+1);

					m_wndList.SetItemState(nSel+1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}
		}

		return 0;
	}
	LRESULT OnCreate(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CConfigValue cVal;
		CComBSTR cm_pszCfgID(m_pszCfgID);
		if (SUCCEEDED(static_cast<T*>(this)->M_Config()->ItemValueGet(cm_pszCfgID, &cVal)))
		{
			LONG nCount = cVal;
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", m_pszCfgID, nCount);
			CComBSTR bstrProfileName(szNameID);
			CConfigValue cCount(nCount + 1L);
			TCHAR szName[64] = _T("");
			if (t_uIDSNewItemName)
				Win32LangEx::LoadString(_pModule->get_m_hInst(), t_uIDSNewItemName, szName, itemsof(szName), LANGIDFROMLCID(static_cast<T*>(this)->m_tLocaleID));
			CConfigValue cName(szName);
			BSTR aIDs[2];
			aIDs[0] = cm_pszCfgID;
			aIDs[1] = bstrProfileName;
			TConfigValue aVals[2];
			aVals[0] = cCount;
			aVals[1] = cName;
			static_cast<T*>(this)->M_Config()->ItemValuesSet(t_uIDSNewItemName ? 2 : 1, aIDs, aVals);
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
			CComBSTR cm_pszCfgID(m_pszCfgID);
			if (SUCCEEDED(static_cast<T*>(this)->M_Config()->ItemValueGet(cm_pszCfgID, &cVal)))
			{
				CComPtr<IConfig> pCfg;
				static_cast<T*>(this)->M_Config()->SubConfigGet(cm_pszCfgID, &pCfg);
				CComQIPtr<IConfigVector> pVecCfg(pCfg);
				LONG nCount = cVal;
				if (nSel != nCount-1)
					pVecCfg->Move(nSel, nCount-1);
				static_cast<T*>(this)->M_Config()->ItemValuesSet(1, &(cm_pszCfgID.m_str), CConfigValue(nCount - 1L));

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
			CComBSTR cm_pszCfgID(m_pszCfgID);
			if (SUCCEEDED(static_cast<T*>(this)->M_Config()->ItemValueGet(cm_pszCfgID, &cVal)))
			{
				LONG nCount = cVal;
				if ((nCount-1) > nSel)
				{
					CComPtr<IConfig> pCfg;
					static_cast<T*>(this)->M_Config()->SubConfigGet(cm_pszCfgID, &pCfg);
					CComQIPtr<IConfigVector> pVecCfg(pCfg);
					pVecCfg->Swap(nSel, nSel+1);
				}

				//m_wndList.SetItemState(nSel+1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
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
			CComBSTR cm_pszCfgID(m_pszCfgID);
			if (SUCCEEDED(static_cast<T*>(this)->M_Config()->ItemValueGet(cm_pszCfgID, &cVal)))
			{
				LONG nCount = cVal;
				if (nSel > 0)
				{
					CComPtr<IConfig> pCfg;
					static_cast<T*>(this)->M_Config()->SubConfigGet(cm_pszCfgID, &pCfg);
					CComQIPtr<IConfigVector> pVecCfg(pCfg);
					pVecCfg->Swap(nSel, nSel-1);
				}

				//m_wndList.SetItemState(nSel-1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
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

public:
	bool UpdateList()
	{
		if (m_wndList.m_hWnd == 0)
			return false;
		m_bUpdating = true;
		m_wndList.SetRedraw(FALSE);
		m_wndList.DeleteAllItems();

		CConfigValue cVal;
		if (FAILED(static_cast<T*>(this)->M_Config()->ItemValueGet(CComBSTR(m_pszCfgID), &cVal)))
			return false;
		LONG nCount = cVal;

		USES_CONVERSION;
		LONG iSel = -1;
		LONG i;
		for (i = 0; i < nCount; i++)
		{
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", m_pszCfgID, i);
			static_cast<T*>(this)->M_Config()->ItemValueGet(CComBSTR(szNameID), &cVal);
			CComPtr<IConfig> pConfig;
			static_cast<T*>(this)->M_Config()->SubConfigGet(CComBSTR(szNameID), &pConfig);
			CComPtr<IConfigItem> pItem;
			static_cast<T*>(this)->M_Config()->ItemGetUIInfo(CComBSTR(szNameID), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
			if (pItem)
			{
				CComPtr<ILocalizedString> pStr;
				pItem->ValueGetName(cVal, &pStr);
				if (pStr)
				{
					CComBSTR bstr;
					pStr->GetLocalized(static_cast<T*>(this)->m_tLocaleID, &bstr);
					if (bstr != NULL)
					{
						m_wndList.AddItem(i, 0, OLE2CT(bstr));
						if (m_pSelected && pConfig == m_pSelected) iSel = i;
						continue;
					}
				}
			}
			m_wndList.AddItem(i, 0, OLE2CT(cVal));
			if (m_pSelected && pConfig == m_pSelected) iSel = i;
		}
		m_bUpdating = false;
		if (iSel >= 0) m_wndList.SelectItem(iSel);

		m_wndList.SetRedraw(TRUE);

		return true;
	}

private:
	CComPtr<IConfigWnd> m_pConfigWnd;
	IConfig* m_pSelected; // used only to compare pointers
	CListViewCtrl m_wndList;
	CToolBarCtrl m_wndToolBar;
	CImageList m_cCommandImages;
	LPCOLESTR const m_pszCfgID;
	bool m_bUpdating;
};
