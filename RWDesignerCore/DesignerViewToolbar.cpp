// DesignerViewToolbar.cpp : Implementation of CDesignerViewToolbar

#include "stdafx.h"
#include "DesignerViewToolbar.h"
#include <MultiLanguageString.h>
#include <htmlhelp.h>
#include <SharedStringTable.h>


// CDesignerViewToolbar


STDMETHODIMP CDesignerViewToolbar::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (!a_bBeforeAccel)
	{
		if ((a_pMsg->message == WM_KEYDOWN || a_pMsg->message == WM_SYSKEYDOWN) &&
			a_pMsg->wParam != VK_MENU && a_pMsg->wParam != VK_SHIFT && a_pMsg->wParam != VK_CONTROL &&
			a_pMsg->wParam != VK_LMENU && a_pMsg->wParam != VK_LSHIFT && a_pMsg->wParam != VK_LCONTROL &&
			a_pMsg->wParam != VK_RMENU && a_pMsg->wParam != VK_RSHIFT && a_pMsg->wParam != VK_RCONTROL)
		{
			for (CToolbars::const_iterator i = m_cToolbars.begin(); i != m_cToolbars.end(); ++i)
			{
				if (i->pCommands && ExecuteAccelerator(i->pCommands, a_pMsg->wParam, (GetKeyState(VK_MENU)&0x8000 ? FALT : 0) | (GetKeyState(VK_CONTROL)&0x8000 ? FCONTROL : 0) | (GetKeyState(VK_SHIFT)&0x8000 ? FSHIFT : 0)))
					return S_OK;
			}
		}
	}
	return m_pView ? m_pView->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
}

bool CDesignerViewToolbar::ExecuteAccelerator(IEnumUnknowns* a_pCommands, WORD a_wKeyCode, WORD a_fVirtFlags)
{
	CComPtr<IDocumentMenuCommand> pCmd;
	for (ULONG i = 0; SUCCEEDED(a_pCommands->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd))); ++i, pCmd = NULL)
	{
		EMenuCommandState eState;
		if (SUCCEEDED(pCmd->State(&eState)))
		{
			if (eState & EMCSSubMenu)
			{
				CComPtr<IEnumUnknowns> pSubCmds;
				pCmd->SubCommands(&pSubCmds);
				if (pSubCmds && ExecuteAccelerator(pSubCmds, a_wKeyCode, a_fVirtFlags))
					return true;
			}
			else if ((eState & (EMCSSeparator|EMCSDisabled)) == 0)
			{
				TCmdAccel t1 = {0, 0}, t2 = {0, 0};
				pCmd->Accelerator(&t1, &t2);
				t1.fVirtFlags &= FALT|FCONTROL|FSHIFT;
				t2.fVirtFlags &= FALT|FCONTROL|FSHIFT;
				if ((t1.fVirtFlags == a_fVirtFlags && t1.wKeyCode == a_wKeyCode) ||
					(t2.fVirtFlags == a_fVirtFlags && t2.wKeyCode == a_wKeyCode))
				{
					CWaitCursor cDummy;
					m_pContext->ResetErrorMessage();
					HandleOperationResult(pCmd->Execute(m_hWnd, m_tLocaleID));
					return true;
				}
				if (eState & EMCSExecuteSubMenu)
				{
					CComPtr<IEnumUnknowns> pSubCmds;
					pCmd->SubCommands(&pSubCmds);
					if (pSubCmds && ExecuteAccelerator(pSubCmds, a_wKeyCode, a_fVirtFlags))
						return true;
				}
			}
		}
	}
	return false;
}

STDMETHODIMP CDesignerViewToolbar::OnIdle()
{
	try
	{
		REBARBANDINFO tRBBI;
		ZeroMemory(&tRBBI, sizeof tRBBI);
		tRBBI.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
		tRBBI.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_STYLE;
		// update toolbar
		for (CToolbars::iterator i = m_cToolbars.begin(); i != m_cToolbars.end(); ++i)
		{
			int iBand = m_wndReBar.IdToIndex((i-m_cToolbars.begin())+0xeb00);
			if (iBand < 0)
				continue; // should not happen

			i->pCommands = NULL;
			m_pMenuCmds->CommandsEnum(m_pMenuCmds, i->cCmdsID, i->pCmdsCfg, m_pContext, this, m_pDocument, &i->pCommands);
			UpdateToolbar(i->wndToolBar, i->pCustomWindow, i->pCommands, i-m_cToolbars.begin(), m_cToolbars.size());

			if (i->pCustomWindow)
				continue;

			m_wndReBar.GetBandInfo(iBand, &tRBBI);
			int iPrevXMin = tRBBI.cxMinChild;
			int nCount = i->wndToolBar.GetButtonCount();
			if (nCount)
			{
				RECT rc;
				i->wndToolBar.GetItemRect(nCount-1, &rc);
				tRBBI.cxIdeal = tRBBI.cx = rc.right;
				tRBBI.cyMinChild = tRBBI.cyChild = tRBBI.cyMaxChild = rc.bottom;
				tRBBI.cxMinChild = m_cToolbars.size() > 1 ? tRBBI.cxIdeal : 0;
			}
			else
			{
				tRBBI.cxMinChild = tRBBI.cxIdeal = tRBBI.cx = 0;
				tRBBI.cyChild = tRBBI.cyMinChild = tRBBI.cyMaxChild = 0;
			}
			if (tRBBI.cxIdeal == 0)
				tRBBI.fStyle |= RBBS_HIDDEN;
			else if (i->bVisible)
				tRBBI.fStyle &= ~RBBS_HIDDEN;
			m_wndReBar.SetBandInfo(iBand, &tRBBI);
		}
	}
	catch (...) {}
	return m_pView ? m_pView->OnIdle() : S_OK;
}

STDMETHODIMP CDesignerViewToolbar::OnDeactivate(BOOL a_bCancelChanges)
{
	return m_pView ? m_pView->OnDeactivate(a_bCancelChanges) : S_OK;
}

STDMETHODIMP CDesignerViewToolbar::QueryInterfaces(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
{
	try
	{
		CComPtr<IUnknown> p;
		QueryInterface(a_iid, reinterpret_cast<void**>(&p));
		if (p)
		{
			HRESULT hRes = a_pInterfaces->Insert(p);
			if (FAILED(hRes))
				return hRes;
		}
		return m_pView ? m_pView->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces) : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewToolbar::OptimumSize(SIZE* a_pSize)
{
	try
	{
		if (m_pView == NULL)
			return E_NOTIMPL;
		SIZE tSize = *a_pSize;
		HRESULT hRes = m_pView->OptimumSize(&tSize);
		if (FAILED(hRes))
			return hRes;
		if (m_hWnd == NULL)
		{
			*a_pSize = tSize;
			return hRes;
		}
		RECT rc1;
		m_wndReBar.GetWindowRect(&rc1);
		tSize.cy += rc1.bottom-rc1.top;
		if (GetWindowLong(GWL_EXSTYLE)&WS_EX_CLIENTEDGE)
		{
			int cxEdge = GetSystemMetrics(SM_CXEDGE);
			int cyEdge = GetSystemMetrics(SM_CYEDGE);
			tSize.cx = cxEdge + cxEdge;
			tSize.cy = cyEdge + cyEdge;
		}

		a_pSize->cx = tSize.cx;
		a_pSize->cy = tSize.cy;

		return S_OK;
	}
	catch (...)
	{
		return a_pSize ? E_UNEXPECTED : E_POINTER;
	}
}

HRESULT CDesignerViewToolbar::CheckDropPoint(int a_nX, int a_nY, CComPtr<IConfig>& a_pCfg)
{
	if (m_wndReBar.m_hWnd == NULL)
		return S_FALSE;
	RECT rc;
	m_wndReBar.GetWindowRect(&rc);
	if (a_nX < rc.left || a_nX >= rc.right || a_nY < rc.top || a_nY >= rc.bottom)
		return S_FALSE;
	RBHITTESTINFO tRBHTI;
	tRBHTI.pt.x = a_nX;
	tRBHTI.pt.y = a_nY;
	m_wndReBar.ScreenToClient(&tRBHTI.pt);
	tRBHTI.flags = 0;
	tRBHTI.iBand = m_cToolbars.size();
	m_wndReBar.HitTest(&tRBHTI);
	if (tRBHTI.iBand < 0 || ULONG(tRBHTI.iBand) >= m_cToolbars.size())
		return S_FALSE;

	REBARBANDINFO tRBBI;
	ZeroMemory(&tRBBI, sizeof tRBBI);
	tRBBI.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
	tRBBI.fMask = RBBIM_ID;
	m_wndReBar.GetBandInfo(tRBHTI.iBand, &tRBBI);
	if (tRBBI.wID < 0xeb00 || tRBBI.wID >= (0xeb00+m_cToolbars.size()))
		return S_FALSE;

	// add commands to this toolbar (if they are commands)
	SToolbar& sTB = m_cToolbars[tRBBI.wID-0xeb00];

	if (IsEqualGUID(sTB.cCmdsID, __uuidof(MenuCommandsVector)))
	{
		// good - we'll add the new commands to the root
		a_pCfg = sTB.pCmdsCfg;
	}
	else
	{
		// we'll try to find a vector in the configuration
		// Usually if there is a meta-commands, its internal commands
		// are called ...uhm... "Commands" or L"SubCommands"
		CComBSTR bstrSub1(L"SubCommands");
		CComBSTR bstrSub2(L"Commands");
		CComPtr<IConfig> pTmp = sTB.pCmdsCfg;
		while (pTmp)
		{
			CConfigValue cVal;
			if (SUCCEEDED(pTmp->ItemValueGet(bstrSub1, &cVal)) && cVal.TypeGet() == ECVTGUID)
			{
				if (IsEqualGUID(cVal, __uuidof(MenuCommandsVector)))
				{
					pTmp->SubConfigGet(bstrSub1, &a_pCfg);
					break;
				}
				CComPtr<IConfig> pNext;
				pTmp->SubConfigGet(bstrSub1, &pNext);
				pTmp = pNext;
			}
			else if (SUCCEEDED(pTmp->ItemValueGet(bstrSub2, &cVal)) && cVal.TypeGet() == ECVTGUID)
			{
				if (IsEqualGUID(cVal, __uuidof(MenuCommandsVector)))
				{
					pTmp->SubConfigGet(bstrSub2, &a_pCfg);
					break;
				}
				CComPtr<IConfig> pNext;
				pTmp->SubConfigGet(bstrSub2, &pNext);
				pTmp = pNext;
			}
			else
			{
				break;
			}
		}
	}
	if (a_pCfg == NULL)
		return S_FALSE; // commands cannot be added to this toolbar
	return S_OK;
}

STDMETHODIMP CDesignerViewToolbar::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
{
	try
	{
		if (a_pFileNames == NULL || (DROPEFFECT_COPY&*a_pdwEffect) == 0)
			return E_FAIL;
		CComPtr<IConfig> pCfg;
		if (S_OK != CheckDropPoint(a_pt.x, a_pt.y, pCfg))
			return E_FAIL;
		ULONG nFiles = 0;
		a_pFileNames->Size(&nFiles);
		for (ULONG i = 0; i < nFiles; ++i)
		{
			CComBSTR bstr;
			a_pFileNames->Get(i, &bstr);
			if (bstr.Length() > 11 && _wcsicmp(bstr.m_str+bstr.Length()-11, L".rwcommands") == 0)
			{
				*a_pdwEffect = DROPEFFECT_COPY;
				if (a_ppFeedback)
					*a_ppFeedback = _SharedStringTable.GetString(IDS_DROPMSG_ADDCOMMANDS);
				return S_OK;
			}
		}
		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewToolbar::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt)
{
	try
	{
		if (a_pDataObj == NULL && a_pFileNames == NULL)
			return E_RW_CANCELLEDBYUSER; // cancelled d'n'd operation

		if (a_pFileNames == NULL)
			return E_FAIL;
		CComPtr<IConfig> pCfg;
		if (S_OK != CheckDropPoint(a_pt.x, a_pt.y, pCfg))
			return E_FAIL;
		HRESULT hRes = E_FAIL;
		ULONG nFiles = 0;
		a_pFileNames->Size(&nFiles);
		for (ULONG i = 0; i < nFiles; ++i)
		{
			CComBSTR bstr;
			a_pFileNames->Get(i, &bstr);
			if (bstr.Length() <= 11 || _wcsicmp(bstr.m_str+bstr.Length()-11, L".rwcommands"))
				continue;
			HANDLE hFile = CreateFile(COLE2CT(bstr.m_str), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
				continue;
			ULONG nFile = GetFileSize(hFile, NULL);
			if (nFile == 0)
			{
				CloseHandle(hFile);
				continue;
			}
			CAutoVectorPtr<BYTE> pFile(new BYTE[nFile]);
			DWORD dwRead = 0;
			ReadFile(hFile, pFile.m_p, nFile, &dwRead, NULL);
			CloseHandle(hFile);
			if (nFile != dwRead)
				continue;
			CComPtr<IConfigInMemory> pNewCmd;
			RWCoCreateInstance(pNewCmd, __uuidof(ConfigInMemory));
			if (FAILED(pNewCmd->DataBlockSet(nFile, pFile)))
				continue;

			// TODO: hack - using internal infromation of MenuCommandsVector
			CConfigValue cItems;
			CComBSTR bstrItems(L"Items");
			pCfg->ItemValueGet(bstrItems, &cItems);
			if (cItems.TypeGet() != ECVTInteger)
				continue;
			OLECHAR szCmd[64] = L"";
			swprintf(szCmd, L"Items\\%08x", cItems.operator LONG());
			cItems = cItems.operator LONG()+1;
			pCfg->ItemValuesSet(1, &(bstrItems.m_str), cItems);
			CComPtr<IConfig> pDstCfg;
			pCfg->SubConfigGet(CComBSTR(szCmd), &pDstCfg);
			if (pDstCfg == NULL || FAILED(CopyConfigValues(pDstCfg, pNewCmd)))
				continue;
			hRes = S_OK;
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CDesignerViewToolbar::OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	try
	{
		// create child view
		CConfigValue cViewID;
		CComBSTR cCFGID_TOOLBAR_VIEW(CFGID_TOOLBAR_VIEW);
		m_pConfig->ItemValueGet(cCFGID_TOOLBAR_VIEW, &cViewID);
		CComPtr<IConfig> pViewCfg;
		m_pConfig->SubConfigGet(cCFGID_TOOLBAR_VIEW, &pViewCfg);
		RECT rc;
		GetClientRect(&rc);
		m_pViewManager->CreateWnd(m_pViewManager, cViewID, pViewCfg, m_pStateMgr, m_pStatusBar, m_pDocument, m_hWnd, &rc, EDVWSNoBorder, m_tLocaleID, &m_pView);

		// Create rebar window
		m_wndReBar = ::CreateWindowEx(0, REBARCLASSNAME, NULL, ATL_SIMPLE_REBAR_NOBORDER_STYLE, 0, 0, 100, 100, m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR), ModuleHelper::GetModuleInstance(), NULL);
		// Initialize and send the REBARINFO structure
		REBARINFO rbi = { 0 };
		rbi.cbSize = sizeof(REBARINFO);
		rbi.fMask  = 0;
		m_wndReBar.SetBarInfo(&rbi);
		int const nIconSize = XPGUI::GetSmallIconSize();
		m_nIconDelta = (nIconSize>>1)-8;
		m_cImageList.Create(nIconSize+m_nIconDelta, nIconSize, XPGUI::GetImageListColorFlags(), 16, 16);

		// init toolbars
		CConfigValue cItems;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_TOOLBAR_ITEMS), &cItems);
		m_cToolbars.resize(cItems.operator LONG());
		for (LONG i = 0; i < cItems.operator LONG(); ++i)
		{
			OLECHAR sz[64];
			swprintf(sz, L"%s\\%08x", CFGID_TOOLBAR_ITEMS, i);
			CComPtr<IConfig> pTBCfg;
			m_pConfig->SubConfigGet(CComBSTR(sz), &pTBCfg);
			CConfigValue cID;
			pTBCfg->ItemValueGet(CComBSTR(CFGID_TOOLBAR_ID), &cID);
			CConfigValue cName;
			pTBCfg->ItemValueGet(CComBSTR(CFGID_TOOLBAR_NAME), &cName);
			CComBSTR cCFGID_TOOLBAR_COMMANDS(CFGID_TOOLBAR_COMMANDS);
			pTBCfg->ItemValueGet(cCFGID_TOOLBAR_COMMANDS, &m_cToolbars[i].cCmdsID);
			pTBCfg->SubConfigGet(cCFGID_TOOLBAR_COMMANDS, &m_cToolbars[i].pCmdsCfg);
			m_cToolbars[i].bstrID.Attach(cID.Detach().bstrVal);
			m_pMenuCmds->CommandsEnum(m_pMenuCmds, m_cToolbars[i].cCmdsID, m_cToolbars[i].pCmdsCfg, m_pContext, this, m_pDocument, &m_cToolbars[i].pCommands);
			m_cToolbars[i].wndToolBar = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0,0,100,0,
				m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR+i), _pModule->get_m_hInst(), NULL);
			m_cToolbars[i].wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DRAWDDARROWS);
			m_cToolbars[i].wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
			m_cToolbars[i].wndToolBar.SetImageList(m_cImageList);
			if (m_nButtonSize == 0)
				m_nButtonSize = nIconSize+(nIconSize>>1)+1;
			if (IsThemingSupported() && !IsThemeNull())
			{
				m_cToolbars[i].wndToolBar.SetButtonWidth(m_nButtonSize, m_nButtonSize*16);
				//SIZE sz = {0, 0};
				//m_cToolbars[i].wndToolBar.GetPadding(&sz);
				//m_cToolbars[i].wndToolBar.SetPadding(10, 10);
				//TBMETRICS tbm = {0};
				//tbm.cbSize = sizeof(tbm);
				//tbm.dwMask = TBMF_PAD;
				//tbm.cxPad = 10;
				//tbm.cyPad = 0;
				//m_cToolbars[i].wndToolBar.SetMetrics(&tbm);
				//m_cToolbars[i].wndToolBar.SetButtonSize(40, 30);
			}
			UpdateToolbar(m_cToolbars[i].wndToolBar, m_cToolbars[i].pCustomWindow, m_cToolbars[i].pCommands, i, cItems.operator LONG());
			CConfigValue cHidden;
			pTBCfg->ItemValueGet(CComBSTR(CFGID_TOOLBAR_HIDDEN), &cHidden);
			m_cToolbars[i].bVisible = !cHidden.operator bool();
			if (m_cToolbars[i].bstrID.Length())
			{
				CComPtr<ISharedStateToolbar> pState;
				m_pStateMgr->StateGet(m_cToolbars[i].bstrID, __uuidof(ISharedStateToolbar), reinterpret_cast<void**>(&pState));
				if (pState)
				{
					m_cToolbars[i].bVisible = pState->IsVisible() != S_FALSE;
				}
				else
				{
					RWCoCreateInstance(pState, __uuidof(SharedStateToolbar));
					pState->SetVisible(m_cToolbars[i].bVisible);
					m_pStateMgr->StateSet(m_cToolbars[i].bstrID, CComQIPtr<ISharedState>(pState));
				}
			}
			REBARBANDINFO rbBand;
			ZeroMemory(&rbBand, sizeof(rbBand));
			rbBand.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
			rbBand.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE | RBBIM_IDEALSIZE /*| RBBIM_HEADERSIZE*/;
			rbBand.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS | RBBS_NOVERT | RBBS_USECHEVRON;
			rbBand.wID = 0xeb00+i;
			//rbBand.cxHeader = (nIconSize+2)>>2;
			RECT rcTmp = {0, 0, 0, 0};
			if (m_cToolbars[i].pCustomWindow)
			{
				m_cToolbars[i].pCustomWindow->Handle(&rbBand.hwndChild);
				::GetWindowRect(rbBand.hwndChild, &rcTmp);
				rcTmp.bottom -= rcTmp.top;
				rcTmp.right -= rcTmp.left;
				rcTmp.top = 0;
				rcTmp.left = 0;
				rbBand.fStyle |= RBBS_VARIABLEHEIGHT;
			}
			else
			{
				rbBand.hwndChild = m_cToolbars[i].wndToolBar;
				int nBtns = m_cToolbars[i].wndToolBar.GetButtonCount();
				if (nBtns) m_cToolbars[i].wndToolBar.GetItemRect(nBtns-1, &rcTmp);
			}
			rbBand.cx = rbBand.cxIdeal = rcTmp.right;
			rbBand.cxMinChild = cItems.operator LONG() > 1 ? rbBand.cx : 0;
			rbBand.cyMinChild = rcTmp.bottom;
			if (rbBand.cxIdeal == 0 || !m_cToolbars[i].bVisible)
				rbBand.fStyle |= RBBS_HIDDEN;
			if (cName.operator BSTR() != NULL && cName.operator BSTR()[0])
			{
				rbBand.fMask |= RBBIM_TEXT;
				CComBSTR bstrLoc;
				CMultiLanguageString::GetLocalized(cName, m_tLocaleID, &bstrLoc);
				COLE2T str(bstrLoc.m_str);
				rbBand.lpText = str;
				m_wndReBar.InsertBand(i, &rbBand);
				if (rbBand.fStyle & RBBS_HIDDEN && m_cToolbars[i].pCustomWindow)
				{
					// fixing strange visibility problems
					::ShowWindow(rbBand.hwndChild, SW_HIDE);
				}
			}
			else
			{
				m_wndReBar.InsertBand(i, &rbBand);
				if (rbBand.fStyle & RBBS_HIDDEN && m_cToolbars[i].pCustomWindow)
				{
					// fixing strange visibility problems
					::ShowWindow(rbBand.hwndChild, SW_HIDE);
				}
			}
			UpdateToolbar(m_cToolbars[i].wndToolBar, m_cToolbars[i].pCustomWindow, m_cToolbars[i].pCommands, i, cItems.operator LONG());
		}

		RWHWND hCl = NULL;
		if (m_pView) m_pView->Handle(&hCl);
		if (hCl)
			return 0;
	}
	catch (...)
	{
	}

	if (m_pView)
		m_pView->Destroy();
	for (CToolbars::iterator i = m_cToolbars.begin(); i != m_cToolbars.end(); ++i)
	{
		if (i->wndToolBar.IsWindow())
		{
			i->wndToolBar.DestroyWindow();
		}
	}
	if (m_wndReBar.IsWindow())
	{
		m_wndReBar.DestroyWindow();
	}
	return -1;
}

LRESULT CDesignerViewToolbar::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_pStateMgr)
		m_pStateMgr->ObserverDel(ObserverGet(), 0);

	for (CToolbars::iterator i = m_cToolbars.begin(); i != m_cToolbars.end(); ++i)
	{
		if (i->pCustomWindow) i->pCustomWindow->Destroy();
	}
	m_cToolbars.clear();
	m_cCommands.clear();

	if (m_pView)
	{
		m_pView->Destroy();
		m_pView = NULL;
	}

	a_bHandled = FALSE;
	return 0;
}

LRESULT CDesignerViewToolbar::OnMenuSelect(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	WORD wFlags = HIWORD(a_wParam);
	if (wFlags == 0xFFFF && a_lParam == NULL)
	{ // menu closing
		m_bShowCommandDesc = false;
	}
	else
	{
		m_bShowCommandDesc = true;
		m_bstrCommandDesc.Empty();
		if (wFlags & MF_POPUP)
		{
			// TODO: submenu descriptions
		}
		else
		{
			WORD wID = LOWORD(a_wParam)-1;
			if (wID < m_cCommands.size())
			{
				CComPtr<ILocalizedString> pStr;
				m_cCommands[wID]->Description(&pStr);
				if (pStr)
				{
					pStr->GetLocalized(m_tLocaleID, &m_bstrCommandDesc);
					if (m_bstrCommandDesc) for (LPOLESTR p = m_bstrCommandDesc; *p; ++p)
						if (*p == L'\n') { *p = L'\0'; break; }
				}
			}
		}
	}
	if (m_pStatusBar) m_pStatusBar->Notify(0, 0);

	return 1;
}

LRESULT CDesignerViewToolbar::OnButtonClicked(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	try
	{
		ATLASSERT(m_cToolbars.size());

		int nTB = (a_wID-ID_FIRST_BUTTON)%m_cToolbars.size();
		int nCmd = (a_wID-ID_FIRST_BUTTON)/m_cToolbars.size();
		CComPtr<IDocumentMenuCommand> pCmd;
		m_cToolbars[nTB].pCommands->Get(nCmd, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
		ATLASSERT(pCmd);
		CWaitCursor cWait;
		m_pContext->ResetErrorMessage();
		HandleOperationResult(pCmd->Execute(m_hWnd, m_tLocaleID));
	}
	catch (...)	{}

	return 0;
}

LRESULT CDesignerViewToolbar::OnButtonDropDown(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
{
	NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);
	if (pTB->iItem >= ID_FIRST_BUTTON && pTB->iItem <= (ID_FIRST_BUTTON+1000))
	{
		int nTB = (pTB->iItem-ID_FIRST_BUTTON)%m_cToolbars.size();
		int nCmd = (pTB->iItem-ID_FIRST_BUTTON)/m_cToolbars.size();
		CComPtr<IDocumentMenuCommand> pCmd;
		m_cToolbars[nTB].pCommands->Get(nCmd, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
		CComPtr<IEnumUnknowns> pCmds;
		pCmd->SubCommands(&pCmds);

		POINT ptBtn = {pTB->rcButton.left, pTB->rcButton.bottom};
		m_cToolbars[nTB].wndToolBar.ClientToScreen(&ptBtn);

		Reset(m_cImageList);

		m_cCommands.clear();
		CMenu cMenu;
		cMenu.CreatePopupMenu();

		UINT nMenuID = 1;
		InsertMenuItems(pCmds, m_cCommands, cMenu.m_hMenu, &nMenuID);
		if (nMenuID == 1)
			return TBDDRET_DEFAULT;

		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		tPMParams.rcExclude = pTB->rcButton;
		::MapWindowPoints(pTB->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&tPMParams.rcExclude), 2);
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, ptBtn.x, ptBtn.y, m_hWnd, &tPMParams);
		if (nSelection != 0)
		{
			CWaitCursor cWait;
			m_pContext->ResetErrorMessage();
			HandleOperationResult(m_cCommands[nSelection-1]->Execute(m_hWnd, m_tLocaleID));
		}
		m_cCommands.clear();
	}
	return TBDDRET_DEFAULT;
}

LRESULT CDesignerViewToolbar::OnHotItemChange(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	NMTBHOTITEM* p = reinterpret_cast<NMTBHOTITEM*>(a_pNMHdr);
	if (p->dwFlags & HICF_LEAVING)
	{
		m_bShowButtonDesc = false;
	}
	else
	{
		m_bShowButtonDesc = false;
		m_bstrButtonDesc.Empty();
		int nTB = (p->idNew-ID_FIRST_BUTTON)%m_cToolbars.size();
		int nCmd = (p->idNew-ID_FIRST_BUTTON)/m_cToolbars.size();
		CComPtr<IDocumentMenuCommand> pCmd;
		m_cToolbars[nTB].pCommands->Get(nCmd, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
		EMenuCommandState eState = EMCSDisabled;
		if (pCmd && SUCCEEDED(pCmd->State(&eState)) && (eState&EMCSDisabled) == 0)
		{
			m_bShowButtonDesc = true;
			CComPtr<ILocalizedString> pStr;
			pCmd->Description(&pStr);
			if (pStr)
			{
				pStr->GetLocalized(m_tLocaleID, &m_bstrButtonDesc);
				if (m_bstrButtonDesc) for (LPOLESTR p = m_bstrButtonDesc; *p; ++p)
					if (*p == L'\n') { *p = L'\0'; break; }
			}
		}
	}
	if (m_pStatusBar) m_pStatusBar->Notify(0, 0);

	return 0;
}

LRESULT CDesignerViewToolbar::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	UpdateLayout();
	if (m_pView)
		m_pView->SendMessage(a_uMsg, a_wParam, a_lParam);
	return 0;
}

LRESULT CDesignerViewToolbar::OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	GetParent().SendMessage(WM_RW_GOTFOCUS, a_wParam ? reinterpret_cast<WPARAM>(m_hWnd) : 0, a_lParam ? reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)) : 0);
	return 0;
}

LRESULT CDesignerViewToolbar::OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
}


LRESULT CDesignerViewToolbar::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	RWHWND hWndCl = NULL;
	if (m_pView) m_pView->Handle(&hWndCl);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW && pHelpInfo->hItemHandle != hWndCl)
	{
		UINT uID = 0;
		if (pHelpInfo->hItemHandle == m_wndReBar.m_hWnd)
		{
			uID = IDS_CONTEXTHELP_REBAR;
		}
		else
		{
			for (CToolbars::iterator i = m_cToolbars.begin(); i != m_cToolbars.end(); ++i)
			{
				if (pHelpInfo->hItemHandle == i->wndToolBar.m_hWnd)
				{
					uID = IDS_CONTEXTHELP_TOOLBAR;
					break;
				}
			}
			if (uID == 0)
			{
				a_bHandled = FALSE;
				return 0;
			}
		}
		TCHAR szBuffer[512] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), uID, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
		RECT rcItem;
		m_wndReBar.GetWindowRect(&rcItem);
		HH_POPUP hhp;
		hhp.cbStruct = sizeof(hhp);
		hhp.hinst = _pModule->get_m_hInst();
		hhp.idString = 0;
		hhp.pszText = szBuffer;
		hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		hhp.pt.y = rcItem.bottom;
		hhp.clrForeground = 0xffffffff;
		hhp.clrBackground = 0xffffffff;
		hhp.rcMargins.left = -1;
		hhp.rcMargins.top = -1;
		hhp.rcMargins.right = -1;
		hhp.rcMargins.bottom = -1;
		hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
		HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
		return 0;
	}
	a_bHandled = FALSE;
	return 0;
}

extern __declspec(selectany) const TCHAR HELPTOPIC_CONFIGURETOOLBAR[] = _T("http://www.rw-designer.com/configure-toolbar");
#include "ConfigFrameDlg.h"
HICON GetIconLayout(ULONG size);

LRESULT CDesignerViewToolbar::OnRightClick(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
{
	NMMOUSE* pNMMouse = reinterpret_cast<NMMOUSE*>(a_pnmh);
	ULONG nTB = pNMMouse->hdr.idFrom-0xe800;
	if (nTB >= m_cToolbars.size())
		return 1;

	Reset(m_cImageList);

	m_cCommands.clear();
	CMenu cMenu;
	cMenu.CreatePopupMenu();
	TCHAR szCommand[128] = _T("");
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CONFIGURETOOLBAR, szCommand, itemsof(szCommand), LANGIDFROMLCID(m_tLocaleID));
	AddItem(cMenu, 1, szCommand, -1, 0);

	POINT pt = pNMMouse->pt;
	::ClientToScreen(pNMMouse->hdr.hwndFrom, &pt);
	TPMPARAMS tPMParams;
	ZeroMemory(&tPMParams, sizeof tPMParams);
	tPMParams.cbSize = sizeof tPMParams;
	UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD|TPM_VERPOSANIMATION, pt.x, pt.y, m_hWnd);
	if (nSelection != 0)
	{
		OLECHAR sz[64];
		swprintf(sz, L"%s\\%08x", CFGID_TOOLBAR_ITEMS, nTB);
		CComPtr<IConfig> pTBCfg;
		m_pConfig->SubConfigGet(CComBSTR(sz), &pTBCfg);
		CComPtr<IConfig> pCommands;
		pTBCfg->DuplicateCreate(&pCommands);
		CConfigFrameDlg<NULL, NULL, HELPTOPIC_CONFIGURETOOLBAR, IDS_CONFIGURETOOLBAR, GetIconLayout> cDlg(pCommands, NULL, m_tLocaleID, _T(""));
		if (IDOK == cDlg.DoModal(m_hWnd))
		{
			pTBCfg->CopyFrom(pCommands, NULL);
			m_cToolbars[nTB].cCmdsID = GUID_NULL;
			m_cToolbars[nTB].pCmdsCfg = NULL;
			pTBCfg->ItemValueGet(CComBSTR(CFGID_TOOLBAR_COMMANDS), &m_cToolbars[nTB].cCmdsID);
			pTBCfg->SubConfigGet(CComBSTR(CFGID_TOOLBAR_COMMANDS), &m_cToolbars[nTB].pCmdsCfg);
		}
	}
	m_cCommands.clear();
	return 0;
}

LRESULT CDesignerViewToolbar::OnChevronPushed(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
{
	NMREBARCHEVRON* pChevron = reinterpret_cast<NMREBARCHEVRON*>(a_pnmh);
	ULONG nTB = pChevron->wID-0xeb00;
	if (nTB >= m_cToolbars.size())
		return 1;
	SToolbar& cTB(m_cToolbars[nTB]);
	if (!cTB.bVisible || cTB.wndToolBar.m_hWnd == NULL)
		return 1;

	CComPtr<IEnumUnknownsInit> pHiddenCmds;
	RWCoCreateInstance(pHiddenCmds, __uuidof(EnumUnknowns));

	RECT rcClient;
	cTB.wndToolBar.GetClientRect(&rcClient);
	int nBtns = cTB.wndToolBar.GetButtonCount();
	for (int i = 0; i < nBtns; ++i)
	{
		TBBUTTON tBtn;
		cTB.wndToolBar.GetButton(i, &tBtn);
		// skip hidden buttons
		if (tBtn.fsState & TBSTATE_HIDDEN)
			continue;
		RECT rcButton;
		cTB.wndToolBar.GetItemRect(i, &rcButton);
		if(rcButton.right > rcClient.right)
		{
			CComPtr<IUnknown> p;
			cTB.pCommands->Get((tBtn.idCommand-ID_FIRST_BUTTON)/m_cToolbars.size(), __uuidof(IUnknown), reinterpret_cast<void**>(&p));
			pHiddenCmds->Insert(p);
		}
	}

	Reset(m_cImageList);

	m_cCommands.clear();
	CMenu cMenu;
	cMenu.CreatePopupMenu();

	UINT nMenuID = 1;
	InsertMenuItems(pHiddenCmds, m_cCommands, cMenu.m_hMenu, &nMenuID);
	if (nMenuID == 1)
		return 0;

	RECT rc = pChevron->rc;
	::MapWindowPoints(pChevron->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&rc), 2);

	TPMPARAMS tPMParams;
	ZeroMemory(&tPMParams, sizeof tPMParams);
	tPMParams.cbSize = sizeof tPMParams;
	tPMParams.rcExclude = rc;
	UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD|TPM_VERPOSANIMATION, rc.left, rc.bottom, m_hWnd, &tPMParams);
	if (nSelection != 0)
	{
		CWaitCursor cWait;
		m_pContext->ResetErrorMessage();
		HandleOperationResult(m_cCommands[nSelection-1]->Execute(m_hWnd, m_tLocaleID));
	}
	m_cCommands.clear();

	// eat next message if click is on the same button
	MSG msg;
	if (::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE) && ::PtInRect(&rc, msg.pt))
		::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);
	else if (::PeekMessage(&msg, m_hWnd, WM_LBUTTONDBLCLK, WM_LBUTTONDBLCLK, PM_NOREMOVE) && ::PtInRect(&rc, msg.pt))
		::PeekMessage(&msg, m_hWnd, WM_LBUTTONDBLCLK, WM_LBUTTONDBLCLK, PM_REMOVE);

	return 0;
}

void CDesignerViewToolbar::InsertMenuItems(IEnumUnknowns* a_pCmds, CCommands& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID)
{
	bool bInsertSeparator = false;
	bool bFirst = true;
	ULONG nItems = 0;
	if (a_pCmds)
		a_pCmds->Size(&nItems);
	for (ULONG i = 0; i < nItems; i++)
	{
		CComPtr<IDocumentMenuCommand> pCmd;
		a_pCmds->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
		if (pCmd == NULL)
			continue;
		EMenuCommandState eState = EMCSNormal;
		pCmd->State(&eState);
		if (eState & EMCSSeparator)
		{
			bInsertSeparator = true;
			continue;
		}
		int nIcon = -1;
		GUID tIconID;
		HRESULT hRes;
		if (SUCCEEDED(hRes = pCmd->IconID(&tIconID)) && !IsEqualGUID(tIconID, GUID_NULL))
		{
			CIconMap::const_iterator j = m_cIconMap.find(tIconID);
			if (j != m_cIconMap.end())
			{
				nIcon = j->second;
				if (hRes == S_FALSE)
				{
					HICON hIcon = NULL;
					pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
					hIcon = AdjustIcon(hIcon);
					if (hIcon)
					{
						m_cImageList.ReplaceIcon(nIcon, hIcon);
						DestroyIcon(hIcon);
					}
				}
			}
			else
			{
				HICON hIcon = NULL;
				pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
				hIcon = AdjustIcon(hIcon);
				if (hIcon)
				{
					m_cImageList.AddIcon(hIcon);
					DestroyIcon(hIcon);
					nIcon = m_cIconMap[tIconID] = m_cImageList.GetImageCount()-1;
				}
			}
		}
		CComPtr<ILocalizedString> pName;
		pCmd->Name(&pName);
		CComBSTR bstrName;
		if (pName)
			pName->GetLocalized(m_tLocaleID, &bstrName);
		COLE2CT strName(bstrName.Length() ? bstrName.operator BSTR() : L"");
		if (eState & EMCSSubMenu)
		{
			CComPtr<IEnumUnknowns> pSubCmds;
			pCmd->SubCommands(&pSubCmds);
			ULONG nSubCmds = 0;
			if (pSubCmds && SUCCEEDED(pSubCmds->Size(&nSubCmds)) && nSubCmds)
			{
				CMenu cSubMenu;
				cSubMenu.CreatePopupMenu();
				InsertMenuItems(pSubCmds, a_cContextOps, cSubMenu.m_hMenu, a_pnMenuID);
				if (bInsertSeparator && !bFirst)
					a_cMenu.AppendMenu(MFT_SEPARATOR);
				AddItem(a_cMenu, cSubMenu, strName, nIcon);
				cSubMenu.Detach();
				bInsertSeparator = bFirst = false;
			}
		}
		else
		{
			UINT uFlags = 0;
			if (eState & EMCSDisabled)
				uFlags |= MFS_DISABLED|MFS_GRAYED;
			if (eState & EMCSChecked)
				uFlags |= MFS_CHECKED;
			if (eState & EMCSRadio)
				uFlags |= MFT_RADIOCHECK;
			if (eState & EMCSBreak)
				uFlags |= MFT_MENUBREAK;

			a_cContextOps.push_back(pCmd); // should rollback it if the next op fails

			if (bInsertSeparator && !bFirst)
				a_cMenu.AppendMenu(MFT_SEPARATOR);
			AddItem(a_cMenu, (*a_pnMenuID)++, strName, nIcon, uFlags);
			bInsertSeparator = bFirst = false;
		}
	}
}

void CDesignerViewToolbar::UpdateToolbar(CToolBarCtrl& a_wndToolbar, CComPtr<IChildWindow>& a_pCustomWindow, IEnumUnknowns* a_pCmds, int a_nFirstID, int a_nIDStep)
{
	bool bInsertSeparator = false;
	bool bFirst = true;
	int nButtonIndex = 0;

	ULONG nSize = 0;
	if (a_pCmds) a_pCmds->Size(&nSize);
	if (nSize == 1)
	{
		// custom GUI is supported only if there is only one command
		CComPtr<ICustomCommandBar> pBar;
		a_pCmds->Get(0, __uuidof(ICustomCommandBar), reinterpret_cast<void**>(&pBar));
		if (pBar)
		{
			pBar->UpdateWindow(m_hWnd, m_tLocaleID, &a_pCustomWindow.p);
			return;
		}
	}

	//a_wndToolbar.SetRedraw(FALSE);

	TBBUTTONINFO tButtonInfo;
	ZeroMemory(&tButtonInfo, sizeof tButtonInfo);
#ifdef WIN64
	tButtonInfo.cbSize = 0x30;//sizeof tButtonInfo; // Win 6.0 adds item to the struct
#else
	tButtonInfo.cbSize = 0x20;//sizeof tButtonInfo; // Win 6.0 adds item to the struct
#endif

	std::vector<std::pair<int, int> > aSetSize;
	for (ULONG i = 0; i < nSize; ++i)
	{
		CComPtr<IDocumentMenuCommand> pCmd;
		a_pCmds->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
		if (pCmd == NULL)
			continue;
		EMenuCommandState eState = EMCSNormal;
		pCmd->State(&eState);
		if (eState & EMCSSeparator)
		{
			bInsertSeparator = true;
			continue;
		}
		if (eState & EMCSSubMenu)
		{
			CComPtr<IEnumUnknowns> pSubCmds;
			pCmd->SubCommands(&pSubCmds);
			ULONG nSubCmds = 0;
			if (pSubCmds == NULL || FAILED(pSubCmds->Size(&nSubCmds)) || nSubCmds == 0)
				continue;
		}
		int nIcon = I_IMAGENONE;
		bool bForceIconUpdate = false;
		GUID tIconID;
		HRESULT hRes;
		if (SUCCEEDED(hRes = pCmd->IconID(&tIconID)) && !IsEqualGUID(tIconID, GUID_NULL))
		{
			CIconMap::const_iterator j = m_cIconMap.find(tIconID);
			if (j != m_cIconMap.end())
			{
				nIcon = j->second;
				if (hRes == S_FALSE)
				{
					HICON hIcon = NULL;
					pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
					hIcon = AdjustIcon(hIcon);
					if (hIcon)
					{
						m_cImageList.ReplaceIcon(nIcon, hIcon);
						DestroyIcon(hIcon);
						bForceIconUpdate = true;
					}
				}
			}
			else
			{
				HICON hIcon = NULL;
				pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
				hIcon = AdjustIcon(hIcon);
				if (hIcon)
				{
					m_cImageList.AddIcon(hIcon);
					DestroyIcon(hIcon);
					nIcon = m_cIconMap[tIconID] = m_cImageList.GetImageCount()-1;
				}
			}
		}
		CComPtr<ILocalizedString> pName;
		pCmd->Name(&pName);
		CComBSTR bstrName;
		if (pName)
			pName->GetLocalized(m_tLocaleID, &bstrName);
		int nNameLen = bstrName.Length();
		if (nNameLen > 3 && 0 == wcscmp(L"...", bstrName.m_str+nNameLen-3))
			bstrName.m_str[nNameLen-3] = L'\0';
		else if (nNameLen > 5 && bstrName[nNameLen-5] == L' ' && bstrName[nNameLen-4] == L'(' && bstrName[nNameLen-3] == L'&' && bstrName[nNameLen-1] == L')')
			bstrName.m_str[nNameLen-5] = L'\0';
		if (bstrName.Length() > 1)
		{
			LPOLESTR p = wcschr(bstrName, L'&');
			if (p)
				wcscpy(p, p+1);
		}
		COLE2T strName(bstrName.Length() ? bstrName.operator BSTR() : L"");

		TBBUTTON tButton;
		ZeroMemory(&tButton, sizeof tButton);

		if (bInsertSeparator && !bFirst)
		{
			tButtonInfo.dwMask = TBIF_STYLE;
			TBBUTTON tBtn;
			ZeroMemory(&tBtn, sizeof tBtn);
			if (a_wndToolbar.GetButton(nButtonIndex, &tBtn))
			{
				// button found
				if ((tBtn.fsStyle & BTNS_SEP) == 0)
				{
					// update button
					a_wndToolbar.DeleteButton(nButtonIndex);
					tButton.fsStyle = BTNS_SEP;
					a_wndToolbar.InsertButton(nButtonIndex, &tButton);
				}
			}
			else
			{
				// add new button
				tButton.fsStyle = BTNS_SEP;
				a_wndToolbar.AddButtons(1, &tButton);
			}
			++nButtonIndex;
		}

		tButtonInfo.dwMask = TBIF_IMAGE|TBIF_STATE|TBIF_STYLE|TBIF_TEXT|TBIF_SIZE;
		tButton.idCommand = ID_FIRST_BUTTON+a_nFirstID+i*a_nIDStep;
		tButton.iBitmap = nIcon;
		tButton.fsState = ((eState & EMCSDisabled) ? 0 : TBSTATE_ENABLED) | ((eState & EMCSChecked) ? TBSTATE_CHECKED : 0);
		tButton.fsStyle = ((eState & EMCSSubMenu) ? BTNS_BUTTON|BTNS_NOPREFIX|BTNS_WHOLEDROPDOWN : ((eState&EMCSExecuteSubMenu) ? BTNS_BUTTON|BTNS_NOPREFIX|BTNS_DROPDOWN : BTNS_BUTTON|BTNS_NOPREFIX)) | (nIcon == I_IMAGENONE || (eState & EMCSShowButtonText) ? BTNS_SHOWTEXT|BTNS_AUTOSIZE : 0);
		tButton.iString = reinterpret_cast<INT_PTR>(static_cast<LPCTSTR>(strName));
		int nSize = ((eState & EMCSConstButtonSizeMask)>>24)*XPGUI::GetSmallIconSize();
		if (nSize)
		{
			if (m_nButtonSize)
				nSize += m_nButtonSize-XPGUI::GetSmallIconSize();
			else
				nSize += 6;
			tButton.fsStyle &= ~BTNS_AUTOSIZE;
		}
		TBBUTTON tButtonOld;
		ZeroMemory(&tButtonOld, sizeof tButtonOld);
		if (a_wndToolbar.GetButton(nButtonIndex, &tButtonOld) && tButtonOld.idCommand == tButton.idCommand)
		{
			// button found
			tButtonInfo.dwMask = 0;
			if (tButton.iBitmap != tButtonOld.iBitmap)
			{
				tButtonInfo.iImage = tButton.iBitmap;
				tButtonInfo.dwMask |= TBIF_IMAGE;
			}
			if (tButton.fsState != (tButtonOld.fsState&(TBSTATE_ENABLED|TBSTATE_CHECKED)))
			{
				tButtonInfo.fsState = tButton.fsState;
				tButtonInfo.dwMask |= TBIF_STATE;
			}
			if (tButton.fsStyle != tButtonOld.fsStyle)
			{
				tButtonInfo.fsStyle = tButton.fsStyle;
				tButtonInfo.dwMask |= TBIF_STYLE;
			}
			if (IS_INTRESOURCE(tButtonOld.iString) || _tcscmp(strName, reinterpret_cast<LPCTSTR>(tButtonOld.iString)))
			{
				tButtonInfo.pszText = strName;
				tButtonInfo.dwMask |= TBIF_TEXT;
			}
			if (nSize)
			{
				tButtonInfo.cx = nSize;
				tButtonInfo.dwMask |= TBIF_SIZE;
			}
			if (tButtonInfo.dwMask)
			{
				if (tButtonInfo.dwMask & (TBIF_TEXT|TBIF_IMAGE) && nSize)
				{
					bool bDropDown = tButton.fsStyle & BTNS_DROPDOWN;
					a_wndToolbar.DeleteButton(nButtonIndex);
					tButton.fsStyle &= ~BTNS_DROPDOWN;
					a_wndToolbar.InsertButton(nButtonIndex, &tButton);
					if (nSize)
					{
						tButtonInfo.dwMask = TBIF_SIZE;
						tButtonInfo.cx = nSize;
						a_wndToolbar.SetButtonInfo(tButton.idCommand, &tButtonInfo);
					}
					//if (bDropDown)
					//{
					//	tButtonInfo.dwMask = TBIF_STYLE;
					//	tButtonInfo.fsStyle = tButton.fsStyle | BTNS_DROPDOWN;
					//	a_wndToolbar.SetButtonInfo(tButton.idCommand, &tButtonInfo);
					//}
				}
				else
				{
					a_wndToolbar.SetButtonInfo(tButton.idCommand, &tButtonInfo);
				}
			}
			else if (bForceIconUpdate)
			{
				RECT rc;
				a_wndToolbar.GetItemRect(nButtonIndex, &rc);
				a_wndToolbar.InvalidateRect(&rc);;
			}
			//if (bAddDropDown)
			//{
			//	a_wndToolbar.SetButtonInfo(tButton.idCommand, &tButtonInfo);
			//}
		}
		else
		{
			for (int j = a_wndToolbar.GetButtonCount()-1; j >= nButtonIndex; --j)
				a_wndToolbar.DeleteButton(j);

			bool bDropDown = tButton.fsStyle & BTNS_DROPDOWN;
			tButton.fsStyle &= ~BTNS_DROPDOWN;
			a_wndToolbar.AddButtons(1, &tButton);
			aSetSize.push_back(std::make_pair(tButton.idCommand, nSize));
			//if (nSize)
			//{
			//	tButtonInfo.dwMask = TBIF_SIZE;
			//	tButtonInfo.cx = nSize;
			//	a_wndToolbar.SetButtonInfo(tButton.idCommand, &tButtonInfo);
			//}
			//if (bDropDown)
			//{
			//	tButtonInfo.dwMask = TBIF_STYLE;
			//	tButtonInfo.fsStyle = tButton.fsStyle | BTNS_DROPDOWN;
			//	a_wndToolbar.SetButtonInfo(tButton.idCommand, &tButtonInfo);
			//}
		}
		++nButtonIndex;
		bInsertSeparator = bFirst = false;
	}

	for (int j = a_wndToolbar.GetButtonCount()-1; j >= nButtonIndex; --j)
		a_wndToolbar.DeleteButton(j);

	for (std::vector<std::pair<int, int> >::const_iterator i = aSetSize.begin(); i != aSetSize.end(); ++i)
	{
		tButtonInfo.dwMask = TBIF_SIZE;
		tButtonInfo.cx = i->second;
		a_wndToolbar.SetButtonInfo(i->first, &tButtonInfo);
	}

	//a_wndToolbar.SetRedraw(TRUE);

	if (m_nButtonSize)
		a_wndToolbar.SetButtonSize(XPGUI::GetSmallIconSize()+8, m_nButtonSize);
}

void CDesignerViewToolbar::UpdateLayout(BOOL bResizeBars)
{
	RECT rc = {0, 0, 0, 0};
	GetClientRect(&rc);

	if (m_wndReBar.m_hWnd == NULL)
	{
		if (m_pView) m_pView->Move(&rc);
		//::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
		//	rect.right - rect.left, rect.bottom - rect.top,
		//	SWP_NOZORDER | SWP_NOACTIVATE);
		return;
	}

	bool bVisible = GetStyle() & WS_VISIBLE;
	if (bVisible)
		m_wndReBar.SetRedraw(FALSE);
	if (m_wndReBar.GetStyle() & WS_VISIBLE)
	{
		if (bResizeBars)
		{
			m_wndReBar.SendMessage(WM_SIZE, 0, 0);
			m_wndReBar.InvalidateRect(NULL, FALSE);
		}
		RECT rcTB = {0, 0, 0, 0};
		m_wndReBar.GetWindowRect(&rcTB);
		rc.top += rcTB.bottom - rcTB.top;
	}
	if (m_pView) m_pView->Move(&rc);
	REBARBANDINFO rebarbandinfo;
	ZeroMemory(&rebarbandinfo, sizeof rebarbandinfo);
	rebarbandinfo.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
	rebarbandinfo.fMask = RBBIM_STYLE;
	bool bLast = true;
	RECT rcBand = {0, 0, 0, 0};
	for(int i = int(m_wndReBar.GetBandCount())-1; i >= 0; --i)
	{
		m_wndReBar.GetBandInfo(i, &rebarbandinfo);
		if (rebarbandinfo.fStyle & RBBS_HIDDEN)
			continue;
		int nTop = rcBand.top;
		m_wndReBar.GetRect(i, &rcBand);
		if (nTop != rcBand.top)
			bLast = true;
		if (bLast)
		{
			m_wndReBar.MaximizeBand(i);
			bLast = false;
		}
		if (rebarbandinfo.fStyle & RBBS_BREAK)
		{
			bLast = true;
		}
	}
	if (bVisible)
	{
		m_wndReBar.SetRedraw(TRUE);
		m_wndReBar.Invalidate(FALSE);
	}
}

void CDesignerViewToolbar::HandleOperationResult(HRESULT a_hRes)
{
	if (FAILED(a_hRes) && a_hRes != E_RW_CANCELLEDBYUSER)
	{
		CComBSTR bstr;
		if (m_pContext->M_ErrorMessage())
			m_pContext->M_ErrorMessage()->GetLocalized(m_tLocaleID, &bstr);
		CComPtr<IDesignerCore> pDC;
		RWCoCreateInstance(pDC, __uuidof(DesignerCore));
		CComPtr<ILocalizedString> pCaption;
		if (pDC) pDC->Name(&pCaption);
		CComBSTR bstrCaption;
		if (pCaption) pCaption->GetLocalized(m_tLocaleID, &bstrCaption);
		if (bstrCaption == NULL) bstrCaption = L"Error";
		if (bstr != NULL && bstr[0])
		{
			::MessageBox(m_hWnd, CW2T(bstr), CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
		}
		else
		{
			TCHAR szTemplate[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_GENERICERROR, szTemplate, itemsof(szTemplate), LANGIDFROMLCID(m_tLocaleID));
			TCHAR szMsg[256];
			_stprintf(szMsg, szTemplate, a_hRes);
			::MessageBox(m_hWnd, szMsg, CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

HICON CDesignerViewToolbar::AdjustIcon(HICON orig) const
{
	if (orig == NULL || m_nIconDelta == 0)
		return orig;
	ICONINFO tInfo;
	ZeroMemory(&tInfo, sizeof tInfo);
	GetIconInfo(orig, &tInfo);
	if (tInfo.hbmMask)
		DeleteObject(tInfo.hbmMask);
	if (tInfo.hbmColor == NULL)
		return orig;
	BITMAP tBmp;
	ZeroMemory(&tBmp, sizeof tBmp);
	GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
	if (tBmp.bmBitsPixel != 32)
	{
		DeleteObject(tInfo.hbmColor);
		return orig; // do not bother low quality icons
	}

	CAutoVectorPtr<DWORD> pSource(new DWORD[tBmp.bmWidth*tBmp.bmHeight]);
	if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pSource.m_p))
	{
		DeleteObject(tInfo.hbmColor);
		return orig; // failure
	}

	DeleteObject(tInfo.hbmColor);

	int nSmSizeX = tBmp.bmWidth+m_nIconDelta;
	int nSmSizeY = tBmp.bmHeight;

	DWORD nXOR = nSmSizeY*nSmSizeX<<2;
	DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
	ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);
	BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
	pHead->biSize = sizeof(BITMAPINFOHEADER);
	pHead->biWidth = nSmSizeX;
	pHead->biHeight = nSmSizeY<<1;
	pHead->biPlanes = 1;
	pHead->biBitCount = 32;
	pHead->biCompression = BI_RGB;
	pHead->biSizeImage = nXOR+nAND;
	pHead->biXPelsPerMeter = 0;
	pHead->biYPelsPerMeter = 0;
	DWORD *pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
	ULONG nRowDelta = nSmSizeX-tBmp.bmWidth;
	pXOR += ((nSmSizeX-tBmp.bmWidth)>>1) + ((nSmSizeY-tBmp.bmHeight)>>1)*nSmSizeX;
	DWORD* pXORSrc = pSource+tBmp.bmWidth*(tBmp.bmHeight-1);
	for (int y = 0; y < tBmp.bmHeight; ++y)
	{
		for (int x = 0; x < tBmp.bmWidth; ++x)
		{
			*pXOR = *pXORSrc;
			++pXORSrc;
			++pXOR;
		}
		pXOR += nRowDelta;
		pXORSrc -= tBmp.bmWidth<<1;
	}
	pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
	BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(nSmSizeY*nSmSizeX));
	int nANDLine = ((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	for (int y = 0; y < nSmSizeY; ++y)
	{
		for (int x = 0; x < nSmSizeX; ++x)
		{
			if (0 == (0xff000000&*pXOR))
			{
				pAND[x>>3] |= 0x80 >> (x&7);
			}
			++pXOR;
		}
		pAND += nANDLine;
	}
	HICON hTmp = CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, nSmSizeX, nSmSizeY, LR_DEFAULTCOLOR);
	DestroyIcon(orig);
	return hTmp;
}

//LRESULT CDesignerViewToolbar::OnCustomDraw(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
//{
//	LPNMCUSTOMDRAW lpNMCustomDraw = reinterpret_cast<LPNMCUSTOMDRAW>(a_pnmh);
//	DWORD dwRet = 0;
//	switch(lpNMCustomDraw->dwDrawStage)
//	{
//	case CDDS_PREPAINT:
//		{
//			CDCHandle dc(lpNMCustomDraw->hdc);
//			dc.FillSolidRect(&lpNMCustomDraw->rc, CCommandBarXPCtrl::m_xpstyle.clrMenu);
//			return CDRF_NOTIFYITEMDRAW;   // We need per-item notifications
//		}
//	case CDDS_POSTPAINT:
//	case CDDS_PREERASE:
//	case CDDS_POSTERASE:
//	case CDDS_ITEMPREPAINT:
//		{
//			CDCHandle dc(lpNMCustomDraw->hdc);
//			HFONT hOldFont = dc.SelectFont(GetFont());
//			CCommandBarXPCtrl::_DrawToolbarButton( (LPNMTBCUSTOMDRAW) lpNMCustomDraw );
//			dc.SelectFont(hOldFont);
//			return CDRF_SKIPDEFAULT;
//		}
//	case CDDS_ITEMPOSTPAINT:
//	case CDDS_ITEMPREERASE:
//	case CDDS_ITEMPOSTERASE:
//	case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
//		return CDRF_DODEFAULT;
//	default:
//		a_bHandled = FALSE;
//		return 0;
//	}
//}
//
