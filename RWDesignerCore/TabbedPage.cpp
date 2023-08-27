#include "stdafx.h"
/*
#include "TabbedPage.h"

#include "StringParsing.h"

#include "ConfigFrameDlg.h"
#include "GlobalOptionsDlg.h"
#include "ConfigIDsApp.h"
#include "ThreadCommand.h"
#include "TabbedWindowThread.h"
#include "DesignerViewWndStart.h"
#include "RecentFiles.h"
#include <DocumentName.h>
#include "ConfigGUILayout.h"
#include "ManageConfigurableItemsDlg.h"

#include <XPGUI.h>
#include <AbbreviateName.h>
#include <HtmlHelp.h>
#include <Win32LangEx.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <RWLocalization.h>


CTabbedPage::CTabbedPage() :
	m_pThread(NULL), m_tStartPage(GUID_NULL), m_bDragHelperCreated(false),
	m_bStatusBarVisible(TRUE),  m_bShowCommandDesc(false),
	m_pOperMgr(NULL), m_nStatusBarTools(0), m_uFreeID(ID_LAYOUT_MENU1),
	m_hIconSmall(NULL), m_hIconLarge(NULL)
{
	RWCoCreateInstance(m_pIM, __uuidof(InputManager));
	CComObject<CDesignerViewStatusBar>::CreateInstance(&m_pDesignerViewStatusBar.p);
	if (m_pDesignerViewStatusBar.p)
		m_pDesignerViewStatusBar.p->AddRef();
}

CTabbedPage::~CTabbedPage()
{
	if (m_pDoc)
		m_pDoc->ObserverDel(CObserverImpl<CTabbedPage, IDocumentObserver, ULONG>::ObserverGet(), 0);
	if (m_pOperMgr)
		m_pOperMgr->Release();
	if (m_pCmdsMgr)
		m_pCmdsMgr->Release();
	if (m_pViewMgr)
		m_pViewMgr->Release();
	m_cMenuImages.Destroy();
	ATLASSERT(m_aObservers.empty());
	for (CObservers::iterator i = m_aObservers.begin(); i != m_aObservers.end(); ++i)
		i->first->Release();
	if (m_hIconSmall)
		DestroyIcon(m_hIconSmall);
	if (m_hIconLarge)
		DestroyIcon(m_hIconLarge);
}

HRESULT CTabbedPage::Init(CThreadCommand* a_pOrder, CTabbedWindowThread* a_pThread)
{
	m_pThread = a_pThread;
	m_tLocaleID = a_pThread->GetLocaleID();

	CComObject<CDesignerFrameOperationManager>::CreateInstance(&m_pOperMgr);
	m_pOperMgr->AddRef();
	m_pOperMgr->Init(m_pThread->M_OperMgr(), this);

	CComObject<CDesignerFrameMenuCommandsManager>::CreateInstance(&m_pCmdsMgr);
	m_pCmdsMgr->AddRef();
	m_pCmdsMgr->Init(m_pThread->M_MenuCmdsMgr(), m_pOperMgr, NULL);//this);

	CComObject<CDesignerFrameViewManager>::CreateInstance(&m_pViewMgr);
	m_pViewMgr->AddRef();
	m_pViewMgr->Init(m_pThread->M_ViewMgr(), m_pCmdsMgr);

	if (a_pOrder->IsIDocument())
	{
		m_pDoc = a_pOrder->M_IDocument();
		m_pDocUndo = m_pDoc;
		UpdateUndoMode();
		if (m_pDoc)
			m_pDoc->ObserverIns(CObserverImpl<CTabbedPage, IDocumentObserver, ULONG>::ObserverGet(), 0);
	}
	else if (a_pOrder->IsSourceName())
	{
		// TODO: implement ??? here ???
		CComPtr<IInputManager> pInMgr;
		RWCoCreateInstance(pInMgr, __uuidof(InputManager));
		pInMgr->DocumentCreate(CStorageFilter(a_pOrder->M_SourceName()), NULL, &m_pDoc);
		m_pDocUndo = m_pDoc;
		UpdateUndoMode();
		if (m_pDoc)
			m_pDoc->ObserverIns(CObserverImpl<CTabbedPage, IDocumentObserver, ULONG>::ObserverGet(), 0);
	}
	else if (a_pOrder->IsStartPage())
	{
		m_tStartPage = a_pOrder->M_StartPage();
	}
	return S_OK;
}

BOOL CTabbedPage::PreTranslateMessage(MSG* pMsg)
{
	if (m_pDVWnd && m_pDVWnd->PreTranslateMessage(pMsg, TRUE) == S_OK)
		return TRUE;
	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
		pMsg->wParam != VK_MENU && pMsg->wParam != VK_SHIFT && pMsg->wParam != VK_CONTROL &&
		pMsg->wParam != VK_LMENU && pMsg->wParam != VK_LSHIFT && pMsg->wParam != VK_LCONTROL &&
		pMsg->wParam != VK_RMENU && pMsg->wParam != VK_RSHIFT && pMsg->wParam != VK_RCONTROL)
	{
		CComPtr<IEnumUnknowns> pCmds;
		GetMainMenu(&pCmds);
		if (ExecuteAccelerator(pCmds, pMsg->wParam, (GetKeyState(VK_MENU)&0x8000 ? FALT : 0) | (GetKeyState(VK_CONTROL)&0x8000 ? FCONTROL : 0) | (GetKeyState(VK_SHIFT)&0x8000 ? FSHIFT : 0)))
			return TRUE;
	}
	if (m_pDVWnd)
	{
		if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
			//pMsg->wParam != VK_RETURN && pMsg->wParam != VK_ESCAPE
			pMsg->wParam == VK_TAB && ::IsDialogMessage(m_hWndClient, pMsg))
			return TRUE;
		if (m_pDVWnd->PreTranslateMessage(pMsg, FALSE) == S_OK)
			return TRUE;
	}

	return FALSE;
}

bool CTabbedPage::ExecuteAccelerator(IEnumUnknowns* a_pCommands, WORD a_wKeyCode, WORD a_fVirtFlags)
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
					m_pErrorMessage = NULL;
					HandleOperationResult(pCmd->Execute(m_hWnd, m_tLocaleID));
					return true;
				}
			}
		}
	}
	return false;
}

BOOL CTabbedPage::OnIdle()
{
	// TODO: watch clipboard and update
	// TODO: update main menu

	UpdateStatusBar();

	return m_pDVWnd && m_pDVWnd->OnIdle() == S_OK;
}

void CTabbedPage::UpdateStatusBar()
{
	// update status bar
	CStatusBarItems cStatusBarItems;
	bool bChange = false;
	if (m_pDesignerViewStatusBar->UpdateStatusBar(m_pDVWnd, this))
	{
		m_pDesignerViewStatusBar->GetPanes(std::back_inserter(cStatusBarItems));
		bChange = true;
		std::copy(m_cStatusBarItems.end()-m_nStatusBarTools, m_cStatusBarItems.end(), std::back_inserter(cStatusBarItems));
		std::swap(m_cStatusBarItems, cStatusBarItems);
	}
	if (m_nStatusBarTools != m_cStatusBarTools.size())
	{
		bChange = true;
		for (CStatusBarItems::const_iterator i = m_cStatusBarItems.end()-m_nStatusBarTools; i != m_cStatusBarItems.end(); ++i)
			if (i->hIcon) ATLVERIFY(DestroyIcon(i->hIcon));
		m_cStatusBarItems.resize(m_cStatusBarItems.size()-m_nStatusBarTools);
		for (CStatusBarTools::iterator i = m_cStatusBarTools.begin(); i != m_cStatusBarTools.end(); ++i)
		{
			SStatusBarItem sItem;
			i->first->TimeStamp(&i->second);
			CComPtr<ILocalizedString> pText;
			i->first->Text(&pText);
			CComBSTR bstrText;
			if (pText) pText->GetLocalized(m_tLocaleID, &bstrText);
			sItem.strText = bstrText ? CW2CT(bstrText) : _T("");
			sItem.hIcon = NULL;
			i->first->Icon(XPGUI::GetSmallIconSize(), &sItem.hIcon); // TODO: resource leak?
			CComPtr<ILocalizedString> pTooltip;
			i->first->Tooltip(&pTooltip);
			CComBSTR bstrTooltip;
			if (pTooltip) pTooltip->GetLocalized(m_tLocaleID, &bstrTooltip);
			sItem.strTooltip = bstrTooltip ? CW2CT(bstrTooltip) : _T("");
			ULONG nWidth = XPGUI::GetSmallIconSize();
			i->first->MaxWidth(&nWidth);
			sItem.nWidth = nWidth;
			m_cStatusBarItems.push_back(sItem);
		}
		m_nStatusBarTools = m_cStatusBarTools.size();
	}
	else
	{
		for (CStatusBarTools::iterator i = m_cStatusBarTools.begin(); i != m_cStatusBarTools.end(); ++i)
		{
			SStatusBarItem& sItem = m_cStatusBarItems[i-m_cStatusBarTools.begin()+m_cStatusBarItems.size()-m_nStatusBarTools];
			ULONG n = i->second;
			i->first->TimeStamp(&i->second);
			if (n != i->second)
			{
				bChange = true;
				CComPtr<ILocalizedString> pText;
				i->first->Text(&pText);
				CComBSTR bstrText;
				if (pText) pText->GetLocalized(m_tLocaleID, &bstrText);
				sItem.strText = bstrText ? CW2CT(bstrText) : _T("");
				if (sItem.hIcon)
					ATLVERIFY(DestroyIcon(sItem.hIcon));
				sItem.hIcon = NULL;
				i->first->Icon(XPGUI::GetSmallIconSize(), &sItem.hIcon); // TODO: resource leak?
				CComPtr<ILocalizedString> pTooltip;
				i->first->Tooltip(&pTooltip);
				CComBSTR bstrTooltip;
				if (pTooltip) pTooltip->GetLocalized(m_tLocaleID, &bstrTooltip);
				sItem.strTooltip = bstrTooltip ? CW2CT(bstrTooltip) : _T("");
			}
		}
	}
	if (m_pDesignerViewStatusBar->ModeChanged())
	{
		m_wndStatusBar.SetSimple(m_pDesignerViewStatusBar->IsSimpleMode());
		bChange = true;
	}
	if (m_pDesignerViewStatusBar->IsSimpleMode())
	{
		if (m_pDesignerViewStatusBar->TextChanged())
		{
			BSTR bstr = m_pDesignerViewStatusBar->GetSimpleMessage();
			COLE2T str(bstr ? bstr : L"");
			m_wndStatusBar.SetText(255, str, SBT_NOBORDERS);
		}
	}
	else
	{
		if (bChange)
		{
			CAutoVectorPtr<int> aWidths(new int[2+m_cStatusBarItems.size()]);

			int arrBorders[3];
			m_wndStatusBar.GetBorders(arrBorders);
			int* p = aWidths;
			p[0] = arrBorders[1]*2;
			for (CStatusBarItems::const_iterator i = m_cStatusBarItems.begin(); i != m_cStatusBarItems.end(); ++i)
			{
				ULONG nWidth = i->nWidth;
				p[1] = p[0] + nWidth + arrBorders[2]+arrBorders[1]*2 + 4; // some additional margins
				++p;
			}
			aWidths[m_cStatusBarItems.size()+1] = aWidths[m_cStatusBarItems.size()] + arrBorders[1] + arrBorders[2];
			RECT rcClient;
			m_wndStatusBar.GetClientRect(&rcClient);
			int cxOff = rcClient.right - (aWidths[m_cStatusBarItems.size()] + ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE));
			for (size_t i = 0; i <= m_cStatusBarItems.size(); ++i)
			{
				aWidths[i] += cxOff;
			}
			m_wndStatusBar.SetParts(m_cStatusBarItems.size()+2, aWidths);

			ULONG j = 1;
			for (CStatusBarItems::const_iterator i = m_cStatusBarItems.begin(); i != m_cStatusBarItems.end(); ++i, ++j)
			{
				m_wndStatusBar.SetIcon(j, i->hIcon);
				m_wndStatusBar.SetText(j, i->strText.c_str());
				m_wndStatusBar.SetTipText(j, i->strTooltip.empty() ? NULL : i->strTooltip.c_str());
			}
		}
	}
}

struct SMenuImage
{
	UINT nCommand;
	UINT nIcon;
};

struct SToolbarItem
{
	UINT nCommand;
	UINT nIcon;
	BYTE bState;
	BYTE bStyle;
};

void CTabbedPage::InitToolBarsAndMenu()
{
	if (m_pThread->M_DesignerAppInfo())
	{
		m_pThread->M_DesignerAppInfo()->Icon(GetSystemMetrics(SM_CXSMICON), &m_hIconSmall);
		m_pThread->M_DesignerAppInfo()->Icon(GetSystemMetrics(SM_CXICON), &m_hIconLarge);
		SetIcon(m_hIconSmall, FALSE);
		SetIcon(m_hIconLarge, TRUE);
		m_pThread->M_DesignerAppInfo()->Name(&m_pAppName);
	}

	// create command bar window
	m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.m_pMenuTextProvider = this;
	m_CmdBar.SetCommandBarExtendedStyle(CBR_EX_TRACKALWAYS|CBR_EX_TRANSPARENT);
	//m_CmdBar.SetImageSize(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize());
	//m_CmdBar.SetAlphaImages(XPGUI::IsXP());
	//m_CmdBar.m_hImageList =	::ImageList_Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 16, 16);

	UpdateRootMenu();

	m_wndMenuBar.Create(m_hWnd);
	m_wndMenuBar.Init(m_CmdBar);
	//RECT rc;
	//m_CmdBar.GetItemRect(0, &rc);
	//m_CmdBar.MoveWindow(0, 0, rc.right, rc.bottom);

	//CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	//AddSimpleReBarBandCtrl(m_hWndToolBar, m_CmdBar, 0, NULL, FALSE, 0, TRUE);
	m_hWndToolBar = m_wndMenuBar;

	// insert external status bar pane handlers
	CComPtr<IEnumUnknowns> pTools;
	m_pThread->M_PlugInCache()->InterfacesEnum(CATID_DesignerFrameTools, __uuidof(IDesignerFrameTools), 0xffffffff, &pTools, NULL);
	CComPtr<IDesignerFrameTools> pToolsItem;
	for (ULONG iToolSet = 0; SUCCEEDED(pTools->Get(iToolSet, __uuidof(IDesignerFrameTools), reinterpret_cast<void**>(&pToolsItem))); iToolSet++, pToolsItem = NULL)
	{
		ULONG nTools = 0;
		pToolsItem->Size(&nTools);
		ULONG i;
		for (i = 0; i < nTools; i++)
		{
			pair<CComPtr<IToolStatusBarControl>, ULONG> tSBTool;
			pToolsItem->StatusBarControl(i, &tSBTool.first);
			if (tSBTool.first)
			{
				tSBTool.second = 0;
				tSBTool.first->TimeStamp(&tSBTool.second);
				--tSBTool.second;
				m_cStatusBarTools.push_back(tSBTool);
			}
		}
	}

	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP | SBARS_TOOLTIPS);
	m_wndStatusBar = m_hWndStatusBar;

	UpdateStatusBar();

	CConfigValue cStatusBar;
	m_pThread->M_Config()->ItemValueGet(CComBSTR(CFGID_STATUSBAR), &cStatusBar);
	m_bStatusBarVisible = cStatusBar;

//	rebar.ShowBand(rebar.IdToIndex(ETBToolBar), m_bToolBar1Visible);
//	rebar.ShowBand(rebar.IdToIndex(ETBOperationsBar), FALSE);

	::ShowWindow(m_hWndStatusBar, m_bStatusBarVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void CTabbedPage::GetMainMenu(IEnumUnknowns** a_pCmds)
{
	CComBSTR cCFGID_MENUCOMMANDS(CFGID_MENUCOMMANDS);
	CConfigValue cItemID;
	CComPtr<IConfig> pItemCfg;
	if (m_pCurrentLayoutCfg)
	{
		m_pCurrentLayoutCfg->ItemValueGet(cCFGID_MENUCOMMANDS, &cItemID);
		m_pCurrentLayoutCfg->SubConfigGet(cCFGID_MENUCOMMANDS, &pItemCfg);
	}
	else
	{
		m_pThread->M_MenuConfig()->ItemValueGet(cCFGID_MENUCOMMANDS, &cItemID);
		m_pThread->M_MenuConfig()->SubConfigGet(cCFGID_MENUCOMMANDS, &pItemCfg);
	}
	m_pCmdsMgr->CommandsEnum(m_pCmdsMgr, cItemID, pItemCfg, this, m_pDVWnd, m_pDoc, a_pCmds);
}

void CTabbedPage::UpdateRootMenu()
{
	m_cMenuCommands.clear();
	m_cSubMenus.clear();
	m_cTopMenus.clear();
	CComPtr<IEnumUnknowns> pCmds;
	GetMainMenu(&pCmds);
	CMenu cMenu;
	cMenu.CreateMenu();
	ULONG nSize = 0;
	pCmds->Size(&nSize);
	for (ULONG i = 0; i < nSize; ++i)
	{
		CComPtr<IDocumentMenuCommand> pCmd;
		if (SUCCEEDED(pCmds->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd))))
		{
			EMenuCommandState eState = EMCSSeparator;
			pCmd->State(&eState);
			if (eState & EMCSSeparator)
				continue; // no separators in main menu
			if (eState & EMCSSubMenu)
			{
				CMenuHandle cSubMenu;
				cSubMenu.CreatePopupMenu();
				CComPtr<IEnumUnknowns> pSubCmds;
				pCmd->SubCommands(&pSubCmds);
				ULONG nSubCmds = 0;
				if (pSubCmds && SUCCEEDED(pSubCmds->Size(&nSubCmds)) && nSubCmds)
				{
					m_cTopMenus[cSubMenu] = i;
					CComPtr<ILocalizedString> pName;
					pCmd->Name(&pName);
					CComBSTR bstrName;
					if (pName)
						pName->GetLocalized(m_tLocaleID, &bstrName);
					cMenu.InsertMenu(-1, MF_POPUP, reinterpret_cast<UINT_PTR>(cSubMenu.m_hMenu), COLE2CT(bstrName));
				}
			}
		}
	}
	m_CmdBar.AttachMenu(cMenu.Detach());
}

bool CTabbedPage::RefreshDocument()
{
	try
	{
		DestroyView();

		m_cViewStates.clear();
		m_strActiveLayout.Empty();

		if (M_Doc())
		{
			// show document
			CComPtr<IStorageFilter> pMainFlt;
			CComBSTR bstrFileName;
			if (SUCCEEDED(M_Doc()->LocationGet(&pMainFlt)) && pMainFlt)
			{
				pMainFlt->ToText(NULL, &bstrFileName);
			}

			USES_CONVERSION;

			CComPtr<IConfig> pBestCfg;
			{
				CTabbedWindowThread::CConfigLock cLock(m_pThread);
				GUID tBuilderID = GUID_NULL;
				if (bstrFileName != NULL)
					RecentFiles::GetRecentFileConfig(m_pThread->M_Config(), bstrFileName, m_strActiveLayout, tBuilderID, m_cViewStates);
				GUID tBuilderIDNew = GUID_NULL;
				if (M_Doc()) M_Doc()->BuilderID(&tBuilderIDNew);

				if (!IsEqualGUID(tBuilderID, GUID_NULL) && !IsEqualGUID(tBuilderID, tBuilderIDNew))
				{
					m_cViewStates.clear();
					m_strActiveLayout.Empty();
				}

				pBestCfg.Attach(FindBestLayout(M_Doc(), m_pViewMgr, m_pThread->M_Config(), m_strActiveLayout));

				pBestCfg->DuplicateCreate(&m_pCurrentLayoutCfg);

				// add to MRU list
				if (bstrFileName != NULL)
					RecentFiles::InsertRecentFile(m_pThread->M_Config(), bstrFileName, m_strActiveLayout, tBuilderIDNew, m_cViewStates);
			}

			// create the view
			m_wndMenuBar.SetCloseButtonID(ID_CLOSEDOCUMENT);
			CreateView(m_pCurrentLayoutCfg, rcDefault);

			UpdateLayout();
			UpdateCaption();
			return true;
		}
		else
		{
			// show start view
			CComObject<CDesignerViewWndStart>* pViewWnd = NULL;
			CComObject<CDesignerViewWndStart>::CreateInstance(&pViewWnd);
			CComPtr<IDesignerView> pTmp = pViewWnd;

			RECT rc;
			GetClientRect(&rc);
			CConfigValue cStartPage;
			{
				CTabbedWindowThread::CConfigLock cLock(m_pThread);
				m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_STARTPAGE), &cStartPage);
				pViewWnd->Init(m_pThread->M_Config(), this, CObserverImpl<CTabbedPage, IStatusBarObserver, BYTE>::ObserverGet(), m_hWnd, &rc, m_tLocaleID, m_pThread->M_StartPageCount(), m_pThread->M_StartPages(), cStartPage, m_tStartPage, m_pThread->M_StorageManager(), m_pAppName);
				m_tStartPage = GUID_NULL;
			}

			m_wndMenuBar.SetCloseButtonID(0);
			UpdateRootMenu();

			m_pDVWnd.Attach(pTmp.Detach());

			m_pDVWnd->Handle(reinterpret_cast<RWHWND*>(&m_hWndClient));
			::SetFocus(m_hWndClient);

			UpdateLayout();
			UpdateCaption();
			return true;
		}
	}
	catch (...)
	{
		return false;
	}
}

LRESULT CTabbedPage::OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	a_bHandled = FALSE;

	HDC hDC = GetDC();
	m_pDesignerViewStatusBar->SetScale(GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f);
	ReleaseDC(hDC);

	m_wndTips.Create(m_hWnd, NULL, _T("DragTip"), WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);

	m_tToolTip.cbSize = TTTOOLINFO_V1_SIZE;//sizeof m_tToolTip;
	m_tToolTip.uFlags = TTF_IDISHWND|TTF_TRACK|TTF_ABSOLUTE;
	m_tToolTip.hwnd = m_hWnd;
	m_tToolTip.hinst = _pModule->get_m_hInst();
	m_tToolTip.lpszText = _T("N/A");
	m_tToolTip.uId = (UINT_PTR)m_hWnd;
	GetClientRect(&m_tToolTip.rect);
	m_wndTips.AddTool(&m_tToolTip);
	m_wndTips.TrackActivate(&m_tToolTip, FALSE);
	m_wndTips.SetMaxTipWidth(800);

	m_cMenuImages.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 16, 16);
	Reset(m_cMenuImages);

	InitToolBarsAndMenu();

	RefreshDocument(); // TODO: error code

	// register object for message filtering and idle updates
	m_pThread->AddMessageFilter(this);
	m_pThread->AddIdleHandler(this);

	RegisterDragDrop(m_hWnd, this);

	return 0;
}

LRESULT CTabbedPage::OnFileExit(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CTabbedPage::OnFileClose(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (AskAndCloseDocument())
	{
		m_pDocUndo.Release();
		if (m_pDoc)
			m_pDoc->ObserverDel(CObserverImpl<CTabbedPage, IDocumentObserver, ULONG>::ObserverGet(), 0);
		m_pDoc.Release();
		RefreshDocument();
	}
	return 0;
}

void CTabbedPage::ShowStatusBar(bool a_bShow)
{
	if (a_bShow != m_bStatusBarVisible)
	{
		m_bStatusBarVisible = a_bShow;
		::ShowWindow(m_hWndStatusBar, m_bStatusBarVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UpdateLayout();
	}
}


HRESULT CTabbedPage::InitConfig(IConfigWithDependencies* a_pMainCfg, IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager, IStorageManager* a_pStorageManager)
{
	CComPtr<ISubConfig> pLayouts;
	pLayouts.Attach(CreateLayoutsConfig(a_pViewManager, a_pOperationManager, a_pMenuCommandsManager, a_pIconsManager));
	a_pMainCfg->ItemInsRanged(CComBSTR(CFGID_VIEWPROFILES), _SharedStringTable.GetStringAuto(IDS_CFGID_VIEWPROFILES_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VIEWPROFILES_DESC), CConfigValue(1L), pLayouts, CConfigValue(1L), CConfigValue(256L), CConfigValue(1L), 0, NULL);

	{
		CComPtr<IConfig> pStorageConfig;
		a_pStorageManager->ConfigGetDefault(&pStorageConfig);
		//{
		//	// ~ hack - extrenal knowledge used
		//	TCHAR szTmp[MAX_PATH+15] =_T("");
		//	::GetModuleFileName(NULL, szTmp, itemsof(szTmp));
		//	TCHAR* p = _tcsrchr(szTmp, _T('\\'));
		//	if (p)
		//	{
		//		_tcscpy(p+1, _T("Library"));
		//		CComBSTR bstr1(L"1F767911-8B60-48F4-A83B-03E3C1B2AD6D\\LastDirectory");
		//		CComBSTR bstr2(L"1F767911-8B60-48F4-A83B-03E3C1B2AD6D\\FavoriteFolders");
		//		pStorageConfig->ItemValuesSet(1, &(bstr1.m_str), CConfigValue(szTmp));
		//		CConfigValue cFF;
		//		pStorageConfig->ItemValueGet(bstr2, &cFF);
		//		CComBSTR bstrFF(cFF.operator BSTR());
		//		bstrFF += L"|";
		//		bstrFF += szTmp;
		//		pStorageConfig->ItemValuesSet(1, &(bstr2.m_str), CConfigValue(bstrFF.m_str));
		//	}
		//}
		CComPtr<ISubConfigSwitch> pStorageSubConfig;
		RWCoCreateInstance(pStorageSubConfig, __uuidof(SubConfigSwitch));
		pStorageSubConfig->ItemInsert(CConfigValue(true), pStorageConfig);
		a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_STORAGE), NULL, NULL, CConfigValue(true), pStorageSubConfig, 0, NULL);
	}
	{
		CComPtr<IConfig> pStorageConfig;
		a_pStorageManager->ConfigGetDefault(&pStorageConfig);
		//{
		//	// ~ hack - extrenal knowledge used
		//	TCHAR szTmp[MAX_PATH+15] =_T("");
		//	::GetModuleFileName(NULL, szTmp, itemsof(szTmp));
		//	TCHAR* p = _tcsrchr(szTmp, _T('\\'));
		//	if (p)
		//	{
		//		_tcscpy(p+1, _T("Library"));
		//		CComBSTR bstr1(L"1F767911-8B60-48F4-A83B-03E3C1B2AD6D\\LastDirectory");
		//		CComBSTR bstr2(L"1F767911-8B60-48F4-A83B-03E3C1B2AD6D\\FavoriteFolders");
		//		pStorageConfig->ItemValuesSet(1, &(bstr1.m_str), CConfigValue(szTmp));
		//		CConfigValue cFF;
		//		pStorageConfig->ItemValueGet(bstr2, &cFF);
		//		CComBSTR bstrFF(cFF.operator BSTR());
		//		bstrFF += L"|";
		//		bstrFF += szTmp;
		//		pStorageConfig->ItemValuesSet(1, &(bstr2.m_str), CConfigValue(bstrFF.m_str));
		//	}
		//}
		CComPtr<ISubConfigSwitch> pStorageSubConfig;
		RWCoCreateInstance(pStorageSubConfig, __uuidof(SubConfigSwitch));
		pStorageSubConfig->ItemInsert(CConfigValue(true), pStorageConfig);
		a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_STORAGESAVE), NULL, NULL, CConfigValue(true), pStorageSubConfig, 0, NULL);
	}

	CComBSTR cCFGID_WIN_X_POS(CFGID_WIN_X_POS);
	CComBSTR cCFGID_WIN_Y_POS(CFGID_WIN_Y_POS);
	CComBSTR cCFGID_WIN_WIDTH(CFGID_WIN_WIDTH);
	CComBSTR cCFGID_WIN_HEIGHT(CFGID_WIN_HEIGHT);
	CComBSTR cCFGID_WIN_MAXIMIZED(CFGID_WIN_MAXIMIZED);
	CConfigValue cValue(100L);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_X_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_X_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_X_POS), CConfigValue(100L), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_Y_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_Y_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_Y_POS), CConfigValue(100L), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_WIDTH), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_WIDTH), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_WIDTH), CConfigValue(static_cast<LONG>(GetSystemMetrics(SM_CXFULLSCREEN)-200)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_HEIGHT), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_HEIGHT), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_HEIGHT), CConfigValue(static_cast<LONG>(GetSystemMetrics(SM_CYFULLSCREEN)-200)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_MAXIMIZED), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_MAXIMIZED), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_MAXIMIZED), CConfigValue(GetSystemMetrics(SM_CXFULLSCREEN) <= 1024), NULL, 0, NULL);

	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_STATUSBAR), _SharedStringTable.GetStringAuto(IDS_CFGID_STATUSBAR), _SharedStringTable.GetStringAuto(IDS_CFGID_STATUSBAR), CConfigValue(true), NULL, 0, NULL);

	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_DLGS_MANAGELAYOUTS_SIZEX), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_DESC), CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_DLGS_MANAGELAYOUTS_SIZEY), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_DESC), CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_DLGS_CONFIGURELAYOUT_SIZEX), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_DESC), CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_DLGS_CONFIGURELAYOUT_SIZEY), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_DESC), CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_DLGS_APPOPTIONS_SIZEX), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_DESC), CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_DLGS_APPOPTIONS_SIZEY), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DLGS_SIZE_DESC), CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_DLGS_APPOPTIONS_LASTPAGE), NULL, NULL, CConfigValue(GUID_NULL), NULL, 0, NULL);

	return S_OK;
}

LRESULT CTabbedPage::OnFileSaveAs(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (M_Doc() == NULL)
	{
		MessageBox(_T("Unable to save this kind of documents."), _T("SaveAs"));
	}
	else
	{
		if (m_pDVWnd)
		{
			CWaitCursor cDummyStackObject;
			m_pDVWnd->OnDeactivate(FALSE);
		}

		// TODO: merge with OnFileSave, change OnClose
		// TODO: first save, then change location

		CComPtr<IStorageFilterWindowListener> pWindowListener;
		CComPtr<IEnumUnknowns> pFlts;
		CComPtr<IConfig> pSaveCfg;
		m_pIM->SaveOptionsGet(M_Doc(), &pSaveCfg, &pFlts, &pWindowListener);

		CComBSTR bstrFlt;
		if (CDocumentName::IsValidFilter(M_Doc()))
		{
			CComPtr<IStorageFilter> pOrigFlt;
			M_Doc()->LocationGet(&pOrigFlt);
			if (pOrigFlt != NULL)
			{
				pOrigFlt->ToText(NULL, &bstrFlt);
			}
		}
		else
		{
			TCHAR szBuffer[256] = _T("");
			CDocumentName::GetDocName(M_Doc(), szBuffer, itemsof(szBuffer));
			bstrFlt = szBuffer;
		}
		if (bstrFlt == NULL)
		{
			bstrFlt = L"";
		}

		CComPtr<IStorageFilter> pFlt;
		{
			CComPtr<IConfig> pStorageContext;
			//CConfigLock cLock(m_pThread);
			m_pThread->M_Config()->SubConfigGet(CComBSTR(CFGID_STORAGESAVE), &pStorageContext);

			m_pThread->M_StorageManager()->FilterCreateInteractivelyCfg(bstrFlt, EFTCreateNew, m_hWnd, pFlts, pSaveCfg, pStorageContext, _SharedStringTable.GetStringAuto(IDS_SAVEDLGCAPTION), pWindowListener, m_tLocaleID, &pFlt);
		}

		if (pFlt != NULL)
		{
			CWaitCursor cDummyStackObject;
			M_Doc()->LocationSet(pFlt);
			if (FAILED(m_pIM->Save(M_Doc(), pSaveCfg, NULL)))
			{
				TCHAR szTmp[256] = _T("");
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_ERROR_SAVE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
				AppMessageBox(szTmp, MB_OK|MB_ICONERROR);
			}
			UpdateCaption();
		}
	}

	return 0;
}

LRESULT CTabbedPage::OnFileSave(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	LRESULT bResult = 0;

	CWaitCursor cDummyStackObject;

	if (M_Doc() == NULL)
	{
		MessageBox(_T("Segment does not support native serialization."), _T("SaveAs"));
	}
	else
	{
		if (m_pDVWnd)
			m_pDVWnd->OnDeactivate(FALSE);

		CComPtr<IStorageFilterWindowListener> pWindowListener;
		CComPtr<IEnumUnknowns> pFlts;
		CComPtr<IConfig> pSaveCfg;
		bool m_bDataTypeOK = S_OK == m_pIM->SaveOptionsGet(M_Doc(), &pSaveCfg, &pFlts, &pWindowListener);

		CComPtr<IStorageFilter> pFlt;
		if (CDocumentName::IsValidFilter(M_Doc()) && m_bDataTypeOK)
		{
			M_Doc()->LocationGet(&pFlt);
		}
		else
		{
			TCHAR szBuffer[256] = _T("");
			CDocumentName::GetDocName(M_Doc(), szBuffer, itemsof(szBuffer));
			{
				CComPtr<IConfig> pStorageContext;
				//CConfigLock cLock(m_pThread);
				m_pThread->M_Config()->SubConfigGet(CComBSTR(CFGID_STORAGESAVE), &pStorageContext);
				m_pThread->M_StorageManager()->FilterCreateInteractivelyCfg(CComBSTR(szBuffer), EFTCreateNew, m_hWnd, pFlts, pSaveCfg, pStorageContext, _SharedStringTable.GetStringAuto(IDS_SAVEDLGCAPTION), pWindowListener, m_tLocaleID, &pFlt);
			}

			if (pFlt)
			{
				M_Doc()->LocationSet(pFlt);
				UpdateCaption();
			}
		}

		if (pFlt != NULL)
		{
			if (FAILED(m_pIM->Save(M_Doc(), pSaveCfg, NULL)))
			{
				TCHAR szTmp[256] = _T("");
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_ERROR_SAVE, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
				AppMessageBox(szTmp, MB_OK|MB_ICONERROR);
			}
			else
			{
				bResult = 1;
			}
		}
	}

	return bResult;
}

LRESULT CTabbedPage::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	a_bHandled = FALSE;

	RevokeDragDrop(m_hWnd);

	WINDOWPLACEMENT sWndPlace;
	sWndPlace.length = sizeof(WINDOWPLACEMENT);
	this->GetWindowPlacement(&sWndPlace);
	{
		BSTR aIDs[6];
		TConfigValue tVals[6];
		CComBSTR i0(CFGID_STATUSBAR); aIDs[0] = i0; CConfigValue v0(static_cast<LONG>(m_bStatusBarVisible)); tVals[0] = v0;
		CComBSTR i1(CFGID_WIN_X_POS); aIDs[1] = i1; CConfigValue v2(static_cast<LONG>(sWndPlace.rcNormalPosition.left)); tVals[1] = v2;
		CComBSTR i2(CFGID_WIN_Y_POS); aIDs[2] = i2; CConfigValue v3(static_cast<LONG>(sWndPlace.rcNormalPosition.top)); tVals[2] = v3;
		CComBSTR i3(CFGID_WIN_WIDTH); aIDs[3] = i3; CConfigValue v4(static_cast<LONG>(sWndPlace.rcNormalPosition.right - sWndPlace.rcNormalPosition.left)); tVals[3] = v4;
		CComBSTR i4(CFGID_WIN_HEIGHT); aIDs[4] = i4; CConfigValue v5(static_cast<LONG>(sWndPlace.rcNormalPosition.bottom - sWndPlace.rcNormalPosition.top)); tVals[4] = v5;
		CComBSTR i5(CFGID_WIN_MAXIMIZED); aIDs[5] = i5; CConfigValue v6(sWndPlace.showCmd == SW_SHOWMAXIMIZED); tVals[5] = v6;
		m_pThread->M_Config()->ItemValuesSet(6, aIDs, tVals);
	}

	DestroyView();
	m_cViewStates.clear();
	m_strActiveLayout.Empty();
	m_cMenuCommands.clear();
	m_cSubMenus.clear();
	m_cTopMenus.clear();

	for (CStatusBarTools::iterator i = m_cStatusBarTools.begin(); i != m_cStatusBarTools.end(); ++i)
	{
		SStatusBarItem& sItem = m_cStatusBarItems[i-m_cStatusBarTools.begin()+m_cStatusBarItems.size()-m_nStatusBarTools];
		if (sItem.hIcon)
			ATLVERIFY(DestroyIcon(sItem.hIcon));
	}

	if (m_wndTips.IsWindow())
		m_wndTips.DestroyWindow();

	return 0;
}

IConfig* CTabbedPage::CreateLayoutConfig(IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager)
{
	try
	{
		CComPtr<IConfigWithDependencies> pProfile;
		if (FAILED(RWCoCreateInstance(pProfile, __uuidof(ConfigWithDependencies))))
			return NULL;

		// window layout
		CComBSTR cCFGID_DESIGNERVIEW(CFGID_DESIGNERVIEW);
		if (FAILED(a_pViewManager->InsertIntoConfigAs(a_pViewManager, pProfile, cCFGID_DESIGNERVIEW, _SharedStringTable.GetStringAuto(IDS_CFGID_DESIGNERVIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DESIGNERVIEW_HELP), 0, NULL)))
			return NULL;

		// window layout icon
		CComBSTR cCFGID_ICONID(CFGID_ICONID);
		CComQIPtr<IConfigItemCustomOptions> pCustIconIDs(a_pIconsManager);
		if (pCustIconIDs != NULL)
			pProfile->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pProfile->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);

		// menu commands
		CComBSTR cCFGID_MENUCOMMANDS(CFGID_MENUCOMMANDS);
		a_pMenuCommandsManager->InsertIntoConfigAs(a_pMenuCommandsManager, pProfile, cCFGID_MENUCOMMANDS, _SharedStringTable.GetStringAuto(IDS_CFGID_COMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_COMMANDS_DESC), 0, NULL);

		// finalize it
		CConfigCustomGUI<&ConfigIDLayout, CConfigGUILayoutDlg, false>::FinalizeConfig(pProfile);

		// return created IConfig
		return pProfile.Detach();
	}
	catch (...)
	{
		return NULL;
	}
}

ISubConfig* CTabbedPage::CreateLayoutsConfig(IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager)
{
	try
	{
		CComPtr<ISubConfigVector> pProfiles;
		if (FAILED(RWCoCreateInstance(pProfiles, __uuidof(SubConfigVector))))
			return NULL;

		CComPtr<IConfig> pProfile;
		pProfile.Attach(CreateLayoutConfig(a_pViewManager, a_pOperationManager, a_pMenuCommandsManager, a_pIconsManager));
		if (pProfile == NULL)
			return NULL;

		if (FAILED(pProfiles->Init(TRUE, pProfile)))
			return NULL;

		if (FAILED(pProfiles->ControllerSet(CConfigValue(1L))))
			return NULL;

		CComBSTR bstr00000000(L"00000000");
		if (FAILED(pProfiles->ItemValuesSet(1, &(bstr00000000.m_str), &CConfigValue(L"(Default)"))))
			return NULL;

		return pProfiles.Detach();
	}
	catch (...)
	{
		return NULL;
	}
}

bool CTabbedPage::CreateView(IConfig* a_pViewProfile, const RECT& a_rcView)
{
	try
	{
		if (m_hWndClient != NULL || m_pDVWnd != NULL || a_pViewProfile == NULL)
			return false;

		// read selected view
		CComBSTR cCFGID_DESIGNERVIEW(CFGID_DESIGNERVIEW);
		CConfigValue cSelectedView;
		if (FAILED(a_pViewProfile->ItemValueGet(cCFGID_DESIGNERVIEW, &cSelectedView)))
			return false;

		// read view's config
		CComPtr<IConfig> pViewCfg;
		if (FAILED(a_pViewProfile->SubConfigGet(CComBSTR(CFGID_DESIGNERVIEW), &pViewCfg)))
			return false;

		if (FAILED(m_pViewMgr->CreateWnd(m_pViewMgr, cSelectedView, pViewCfg, this, CObserverImpl<CTabbedPage, IStatusBarObserver, BYTE>::ObserverGet(), M_Doc(), reinterpret_cast<RWHWND>(m_hWnd), &a_rcView, EDVWSVolatileBorder, m_tLocaleID, &m_pDVWnd)))
			return false;

		m_pDVWnd->Handle(reinterpret_cast<RWHWND*>(&m_hWndClient));
		::SetFocus(m_hWndClient);
		
		if (m_hWndClient == NULL)
		{
			m_pDVWnd = NULL;
			return false;
		}

		UpdateRootMenu();

		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool CTabbedPage::DestroyView()
{
	//TCHAR szHelpPath[MAX_PATH] = _T("");
	//GetHelpFilePath(szHelpPath, itemsof(szHelpPath), m_tLocaleID);
	//m_strLayoutHelpPath = szHelpPath;
	//m_strLayoutHelpPath += _T("::/Layouts/StartView.html>Layout");

	if (m_hWndClient == NULL || m_pDVWnd == NULL)
		return false;

	m_pDVWnd->Destroy();
	m_hWndClient = NULL;
	m_pDVWnd = NULL;

	return true;
}

// configuration dialogs

LRESULT CTabbedPage::OnProfileManage(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	CComPtr<IConfig> pCopy;
	{
		CTabbedWindowThread::CConfigLock cLock(m_pThread);
		m_pThread->M_Config()->DuplicateCreate(&pCopy);
	}
	if (CManageLayoutsDlg(pCopy, m_pThread->M_Config(), m_tLocaleID, m_pThread->M_DesignerFrameIcons()).DoModal() == IDOK)
	{
		CTabbedWindowThread::CConfigLock cLock(m_pThread);
		CopySelectedConfigValues(m_pThread->M_Config(), pCopy, CFGID_VIEWPROFILES);
	}

	return 0;
}

void CTabbedPage::UpdateMenu(CMenuHandle& a_cMenu, IEnumUnknowns* a_pCmds)
{
	for (LONG nCount = a_cMenu.GetMenuItemCount(); nCount > 0; nCount--)
	{
		CMenuItemInfo tMII;
		tMII.fMask = MIIM_ID | MIIM_SUBMENU;
		a_cMenu.GetMenuItemInfo(nCount-1, TRUE, &tMII);
		if (tMII.hSubMenu)
		{
			UpdateMenu(CMenuHandle(tMII.hSubMenu), NULL);
			m_cSubMenus.erase(tMII.hSubMenu);
			DestroyMenu(tMII.hSubMenu);
		}
		else
		{
			m_cMenuCommands.erase(tMII.wID);
		}
		RemoveItem(tMII.wID);
		a_cMenu.RemoveMenu(nCount-1, MF_BYPOSITION);
	}
	if (a_pCmds == NULL)
		return;
	bool bFirst = true;
	bool bSeparator = false;
	ULONG nSize = 0;
	a_pCmds->Size(&nSize);
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
			bSeparator = true;
			continue;
		}

		int nIcon = -1;
		GUID tIconID;
		HRESULT hRes;
		if (SUCCEEDED(hRes = pCmd->IconID(&tIconID)) && !IsEqualGUID(tIconID, GUID_NULL))
		{
			CFrameIcons::const_iterator j = m_cIconMap.find(tIconID);
			if (j != m_cIconMap.end())
			{
				nIcon = j->second;
				if (hRes == S_FALSE)
				{
					HICON hIcon = NULL;
					pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
					if (hIcon)
					{
						m_cMenuImages.ReplaceIcon(nIcon, hIcon);
						DestroyIcon(hIcon);
					}
				}
			}
			else
			{
				HICON hIcon = NULL;
				pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
				if (hIcon)
				{
					m_cMenuImages.AddIcon(hIcon);
					DestroyIcon(hIcon);
					nIcon = m_cIconMap[tIconID] = m_cMenuImages.GetImageCount()-1;
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
				m_cSubMenus[cSubMenu] = pSubCmds;
				if (bSeparator && !bFirst)
					a_cMenu.AppendMenu(MFT_SEPARATOR);
				AddItem(a_cMenu, cSubMenu, strName, nIcon);
				cSubMenu.Detach();
				bSeparator = bFirst = false;
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

			std::tstring str;
			str = static_cast<LPCTSTR>(bstrName.Length() ? COLE2CT(bstrName) : _T(""));
			UINT a_nID = AllocateMenuID();
			TCmdAccel tAccel = {0, 0};
			if (SUCCEEDED(pCmd->Accelerator(&tAccel, NULL)) && (tAccel.fVirtFlags || tAccel.wKeyCode))
			{
				TCHAR szKey[64] = _T("");
				str.append(_T("\t"));
				if (tAccel.fVirtFlags & FALT)
				{
					_tcscpy(szKey+GetKeyNameText(MapVirtualKeyEx(VK_MENU, 0, GetKeyboardLayout(0)) << 16, szKey, itemsof(szKey)-2), _T("+"));
					str.append(szKey);
				}
				if (tAccel.fVirtFlags & FCONTROL)
				{
					_tcscpy(szKey+GetKeyNameText(MapVirtualKeyEx(VK_CONTROL, 0, GetKeyboardLayout(0)) << 16, szKey, itemsof(szKey)-2), _T("+"));
					str.append(szKey);
				}
				if (tAccel.fVirtFlags & FSHIFT)
				{
					_tcscpy(szKey+GetKeyNameText(MapVirtualKeyEx(VK_SHIFT, 0, GetKeyboardLayout(0)) << 16, szKey, itemsof(szKey)-2), _T("+"));
					str.append(szKey);
				}
				UINT scancode = MapVirtualKeyEx(tAccel.wKeyCode, 0, GetKeyboardLayout(0));
				switch(tAccel.wKeyCode)
				{
				case VK_INSERT:
				case VK_DELETE:
				case VK_HOME:
				case VK_END:
				case VK_NEXT:  // Page down
				case VK_PRIOR: // Page up
				case VK_LEFT:
				case VK_RIGHT:
				case VK_UP:
				case VK_DOWN:
					scancode |= 0x100; // Add extended bit
				}
				GetKeyNameText(scancode << 16, szKey, itemsof(szKey)-1);
				str.append(szKey);
			}
			if (bSeparator && !bFirst)
				a_cMenu.AppendMenu(MFT_SEPARATOR);
			AddItem(a_cMenu, a_nID, str.c_str(), nIcon, uFlags);
			bSeparator = bFirst = false;
			m_cMenuCommands[a_nID] = pCmd;
		}
	}
}

UINT CTabbedPage::AllocateMenuID()
{
	if (m_cFreeIDs.empty())
		return m_uFreeID++;
	UINT x = m_cFreeIDs.front();
	m_cFreeIDs.pop();
	return x;
}

void CTabbedPage::ReleaseMenuID(UINT a_nID)
{
	m_cFreeIDs.push(a_nID);
}


LRESULT CTabbedPage::OnInitMenuPopup(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	a_bHandled = FALSE;
	CTopMenus::const_iterator iT = m_cTopMenus.find(reinterpret_cast<HMENU>(a_wParam));
	if (iT != m_cTopMenus.end())
	{
		// expanding top-level menu - reenumerate
		CComPtr<IEnumUnknowns> pCmds;
		GetMainMenu(&pCmds);
		CComPtr<IDocumentMenuCommand> pCmd;
		pCmds->Get(iT->second, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
		pCmds = NULL;
		if (pCmd)
			pCmd->SubCommands(&pCmds);
		if (pCmds == NULL)
			return 0; // probably error
		UpdateMenu(CMenuHandle(iT->first), pCmds);
	}
	else
	{
		CSubMenus::const_iterator i = m_cSubMenus.find(reinterpret_cast<HMENU>(a_wParam));
		if (i == m_cSubMenus.end())
			return 0; // probably error
		UpdateMenu(CMenuHandle(i->first), i->second);
	}

	return 0;
}

void CTabbedPage::SwitchLayout(int a_nIndex)
{
	CComPtr<IConfig> pViewCfg;
	CConfigValue cName;
	{
		CTabbedWindowThread::CConfigLock cLock(m_pThread);
		OLECHAR szNameID[64];
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, a_nIndex);
		if (FAILED(m_pThread->M_Config()->SubConfigGet(CComBSTR(szNameID), &pViewCfg)))
			return; // TODO: report error
		m_pCurrentLayoutCfg = NULL;
		pViewCfg->DuplicateCreate(&m_pCurrentLayoutCfg);
		m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cName);
	}
	RECT rcClient;
	::GetWindowRect(m_hWndClient, &rcClient);
	ScreenToClient(&rcClient);
	if (m_pDVWnd)
		m_pDVWnd->OnDeactivate(FALSE);
	DestroyView();
	CreateView(m_pCurrentLayoutCfg, rcClient);
	m_strActiveLayout = cName;
	UpdateCaption();
}

LRESULT CTabbedPage::OnProfileConfigureCurrent(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	typedef CConfigFrameDlg<CFGID_DLGS_CONFIGURELAYOUT_SIZEX, CFGID_DLGS_CONFIGURELAYOUT_SIZEY, HELPTOPIC_CONFIGURELAYOUT, IDS_CONFIGURECURRENTLAYOUT, IDI_LAYOUT> CConfigureCurrentLayoutDlg;
	CComPtr<IConfig> pCopy;
	m_pCurrentLayoutCfg->DuplicateCreate(&pCopy);
	CConfigureCurrentLayoutDlg cDlg(pCopy, m_pThread->M_Config(), m_tLocaleID, COLE2T(m_strActiveLayout));
	if (cDlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(m_pThread->M_Config())) == IDOK)
	{
		CWaitCursor cWait;
		CopyConfigValues(m_pCurrentLayoutCfg, pCopy);
		if (cDlg.ShouldSaveConfig())
		{
			CTabbedWindowThread::CConfigLock cLock(m_pThread);
			m_strActiveLayout = cDlg.GetConfigName();
			CConfigValue cVal;
			m_pThread->M_Config()->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cVal);
			LONG nLayouts = cVal;

			OLECHAR szNameID[64];
			LONG i;
			for (i = 0; i < nLayouts; i++)
			{
				_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
				CConfigValue cName;
				m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cName);
				if (0 == wcscmp(m_strActiveLayout, cName.operator BSTR()))
				{
					CComPtr<IConfig> pCfg;
					m_pThread->M_Config()->SubConfigGet(CComBSTR(szNameID), &pCfg);
					CopyConfigValues(pCfg, m_pCurrentLayoutCfg);
					break;
				}
			}
			if (i == nLayouts)
			{
				CComBSTR cCFGID_VIEWPROFILES(CFGID_VIEWPROFILES);
				m_pThread->M_Config()->ItemValuesSet(1, &(cCFGID_VIEWPROFILES.m_str), CConfigValue(nLayouts+1L));
				_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, nLayouts);
				CComBSTR cszNameID(szNameID);
				m_pThread->M_Config()->ItemValuesSet(1, &(cszNameID.m_str), CConfigValue(m_strActiveLayout));
				CComPtr<IConfig> pCfg;
				m_pThread->M_Config()->SubConfigGet(CComBSTR(szNameID), &pCfg);
				CopyConfigValues(pCfg, m_pCurrentLayoutCfg);
			}
		}
		RECT rcClient;
		::GetWindowRect(m_hWndClient, &rcClient);
		ScreenToClient(&rcClient);
		if (m_pDVWnd)
			m_pDVWnd->OnDeactivate(FALSE);
		DestroyView();
		CreateView(m_pCurrentLayoutCfg, rcClient);
	}
	return 0;
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

IConfig* CTabbedPage::FindBestLayout(IDocument* a_pDoc, IViewManager* a_pMgr, IConfig* a_pMainConfig, CComBSTR& a_strName)
{
	// 1. gather information
	CConfigValue cVal;
	if (FAILED(a_pMainConfig->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cVal)))
		return NULL;
	LONG nLayouts = cVal;
	if (nLayouts == 0)
		return NULL; // no layouts...very bad

	CLSID tBuilderID = GUID_NULL;
	a_pDoc->BuilderID(&tBuilderID);

	if (!IsEqualGUID(tBuilderID, GUID_NULL))
	{
		// have builder ID -> try to find layouts that match
		OLECHAR szNameID[64];
		LONG iSelected = -1;
		for (LONG i = 0; i < nLayouts; ++i)
		{
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
			CConfigValue cName;
			a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cName);
			if (a_strName.Length())
			{
				if (a_strName == cName.operator BSTR())
				{
					iSelected = i;
				}
			}
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DOCBUILDER);
			CConfigValue cBuilderID;
			a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cBuilderID);
			if (IsEqualGUID(tBuilderID, cBuilderID))
			{
				if (iSelected == -1)
					iSelected = i;
			}
		}
		if (iSelected != -1)
		{
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, iSelected);
			IConfig* pLayoutCfg = NULL;
			a_pMainConfig->SubConfigGet(CComBSTR(szNameID), &pLayoutCfg);
			CConfigValue cName;
			a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cName);
			a_strName = cName;
			return pLayoutCfg;
		}
	}

	struct SRating {int nUsed; int nOthers;};
	SRating* aRatings = reinterpret_cast<SRating*>(_alloca(nLayouts*sizeof(SRating)));

	OLECHAR szNameID[64];
	LONG i;
	LONG iBest = 0;
	LONG iSelected = -1;
	for (i = 0; i < nLayouts; i++)
	{
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
		CConfigValue cName;
		a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cName);
		if (a_strName.Length())
		{
			if (a_strName == cName.operator BSTR())
			{
				iSelected = i;
			}
		}
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DESIGNERVIEW);
		CConfigValue cView;
		a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cView);
		CComPtr<IConfig> pViewCfg;
		a_pMainConfig->SubConfigGet(CComBSTR(szNameID), &pViewCfg);
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_LAYOUTVERB);
		CConfigValue cVerb;
		a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cVerb);
		CComObjectStackEx<CCheckSuitabilityCallback> cCallback;
		a_pMgr->CheckSuitability(a_pMgr, cView, pViewCfg, a_pDoc, &cCallback);
		aRatings[i].nUsed = cCallback.cUsed.size();
		aRatings[i].nOthers = cCallback.cMissing.size();
	}
	if (iSelected > -1)
	{
		iBest = iSelected;
	}
	// 2. find the best layout
	//   layout comparison conditions:
	//   - number of used interfaces
	//   - no other interface required
	//   - profile index
	if (i == nLayouts)
		for (i = 1; i < nLayouts; i++)
		{
			if (aRatings[iBest].nUsed < aRatings[i].nUsed ||
				(aRatings[iBest].nUsed == aRatings[i].nUsed && aRatings[iBest].nOthers && aRatings[i].nOthers < aRatings[iBest].nOthers))
			{
				// this one is better
				iBest = i;
			}
		}

	_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, iBest);
	IConfig* pLayoutCfg = NULL;
	a_pMainConfig->SubConfigGet(CComBSTR(szNameID), &pLayoutCfg);
	CConfigValue cName;
	a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cName);
	a_strName = cName;
	return pLayoutCfg;
}

STDMETHODIMP CTabbedPage::StateSet(BSTR a_bstrCategoryName, ISharedState* a_pState)
{
	try
	{
		if (a_bstrCategoryName == NULL || *a_bstrCategoryName == L'\0')
			return E_RW_INVALIDPARAM;

		ObjectLock cLock(this);

		if (a_pState == NULL)
		{
			CViewStates::iterator i = m_cViewStates.find(a_bstrCategoryName);
			if (i != m_cViewStates.end())
				i->second.p = NULL;
			return S_OK; // no notification
		}
		SViewStateInfo& tInfo = m_cViewStates[a_bstrCategoryName];
		tInfo.p = a_pState;
		if (m_cChangedStateIDs.empty())
		{
			m_cChangedStateIDs.push(a_bstrCategoryName);
			PostMessage(WM_SENDSTATECHANGES);
		}
		else
		{
			m_cChangedStateIDs.push(a_bstrCategoryName);
		}
//		CStateIDs m_cChangedStateIDs;
//#ifdef _DEBUG
//		{
//			CViewStates::iterator i = m_cViewStates.find(a_bstrCategoryName);
//			if (i == m_cViewStates.end())
//			{
//				m_cViewStates[a_bstrCategoryName].nActNotifying = -1;
//			}
//		}
//#endif
//		ATLASSERT(0 > tInfo.nActNotifying);// Fire_Notify is not reentrant (the behavior is affected while in execution)
//#ifdef _DEBUG
//		tInfo.nActNotifying = -1;
//#endif

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CTabbedPage::OnSendStateChanges(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	ObjectLock cLock(this);
	while (!m_cChangedStateIDs.empty())
	{
		CViewStates::iterator i = m_cViewStates.find(m_cChangedStateIDs.front().m_str);
		if (i != m_cViewStates.end() && i->second.p)
		{
			TSharedStateChange tTmp = {m_cChangedStateIDs.front(), i->second.p};
			for (i->second.nActNotifying = 0; i->second.nActNotifying < int(m_aObservers.size()); ++i->second.nActNotifying)
			{
				m_aObservers[i->second.nActNotifying].first->Notify(m_aObservers[i->second.nActNotifying].second, tTmp);
			}
		}
		m_cChangedStateIDs.pop();
	}
	return 0;
}

STDMETHODIMP CTabbedPage::StateGet(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
{
	try
	{
		*a_ppState = NULL;
		ObjectLock cLock(this);
		CViewStates::const_iterator i = m_cViewStates.find(a_bstrCategoryName);
		if (i != m_cViewStates.end())
		{
			return i->second.p ? i->second.p->QueryInterface(a_iid, a_ppState) : E_RW_ITEMNOTFOUND;
		}
		else
		{
			return E_RW_ITEMNOTFOUND;
		}
	}
	catch (...)
	{
		return a_ppState == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CTabbedPage::ObserverIns(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
{
	try
	{
		if (a_pObserver == NULL)
			return E_RW_INVALIDPARAM;

		CComPtr<ISharedStateObserver> p = a_pObserver;

		ObjectLock cLock(this);
		m_aObservers.push_back(std::make_pair(a_pObserver, a_tCookie));
		p.Detach();

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
STDMETHODIMP CTabbedPage::ObserverDel(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
{
	try
	{
		if (a_pObserver == NULL)
			return E_RW_INVALIDPARAM;
		ObjectLock cLock(this);
		for (CObservers::iterator i = m_aObservers.begin(); i != m_aObservers.end(); ++i)
		{
			// comparison via IUnknown ommited
			if (i->first == a_pObserver && i->second == a_tCookie)
			{
				m_aObservers.erase(i);
				a_pObserver->Release();
				return S_OK;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CTabbedPage::OnClose(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (AskAndCloseDocument())
		bHandled = FALSE;

	return 0;
}

void CTabbedPage::UpdateCaption()
{
	TCHAR szCaption[512] = _T("");
	CConfigValue cVal;
	m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_CAPTION), &cVal);
	if ((cVal.operator LONG() & CFGFLAG_CAPTION_NAME) && M_Doc())
	{
		CDocumentName::GetDocName(M_Doc(), szCaption, itemsof(szCaption));
		if (szCaption[0])
		{
			if (M_Doc() && M_Doc()->IsDirty() == S_OK)
			{
				_tcscat(szCaption, _T("*"));
			}
		}
	}
	if ((cVal.operator LONG() & CFGFLAG_CAPTION_TYPE) && M_Doc())
	{
		CComPtr<IConfig> pCfg;
		GUID tEncID = GUID_NULL;
		CComPtr<IDocumentEncoder> pEnc;
		CComPtr<IDocumentType> pDocType;
		CComPtr<ILocalizedString> pName;
		CComBSTR bstrName;
		if (M_Doc() && SUCCEEDED(M_Doc()->EncoderGet(&tEncID, &pCfg)) &&
			!IsEqualGUID(tEncID, GUID_NULL) && SUCCEEDED(RWCoCreateInstance(pEnc, tEncID)) &&
			pEnc && SUCCEEDED(pEnc->DocumentType(&pDocType)) &&
			pDocType && SUCCEEDED(pDocType->TypeNameGet(NULL, &pName)) &&
			pName && SUCCEEDED(pName->GetLocalized(m_tLocaleID, &bstrName)) && bstrName)
		{
			_tcscat(szCaption, szCaption[0] ? _T(" (") : _T("("));
			_tcscat(szCaption, COLE2CT(bstrName));
			_tcscat(szCaption, _T(")"));
		}
	}
	if ((cVal.operator LONG() & CFGFLAG_CAPTION_LAYOUT) && M_Doc() && m_strActiveLayout)
	{
		CComBSTR bstrLoc;
		CMultiLanguageString::GetLocalized(m_strActiveLayout, m_tLocaleID, &bstrLoc);
		if (bstrLoc.m_str)
		{
			_tcscat(szCaption, szCaption[0] ? _T(" [") : _T("["));
#ifdef _UNICODE
			_tcscat(szCaption, bstrLoc.m_str);
#else
			std::wstring str(p, p+n);
			_tcscat(szCaption, COLE2CT(str.c_str()));
#endif
			_tcscat(szCaption, _T("]"));
		}
	}
	if ((cVal.operator LONG() & CFGFLAG_CAPTION_APP) || M_Doc() == NULL || szCaption[0] == _T('\0'))
	{
		if (szCaption[0])
			_tcscat(szCaption, _T(" - "));

		if (m_pAppName)
		{
			CComBSTR bstrCaption;
			m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
			_tcscat(szCaption, COLE2CT(bstrCaption));
		}
		else
		{
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDR_MAINFRAME, szCaption+_tcslen(szCaption), 512-_tcslen(szCaption), LANGIDFROMLCID(m_tLocaleID));
		}
	}

	SetWindowText(szCaption);
}

LRESULT CTabbedPage::OnFileMRU(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	vector<tstring> cMRUList;
	{
		CTabbedWindowThread::CConfigLock cLock(m_pThread);
		RecentFiles::GetRecentFileList(m_pThread->M_Config(), cMRUList);
	}
	size_t nIndex = a_wID-ID_FILE_MRU_FIRST;
	if (nIndex < cMRUList.size())
	{
		CComPtr<IDesignerCore> pDesignerCore;
		RWCoCreateInstance(pDesignerCore, __uuidof(DesignerCore));
		pDesignerCore->NewWindowPath(CComBSTR(cMRUList[nIndex].c_str()), NULL);
	}
	else
	{
		ATLASSERT(0);
	}
	return 0;
}

void CTabbedPage::FilesFromDrop(IDataObject* a_pDataObj, CComPtr<IEnumStringsInit>& a_pFileList)
{
	a_pFileList = NULL;
	if (a_pDataObj == NULL)
		return;
	STGMEDIUM stgm;
	stgm.tymed = TYMED_HGLOBAL;
	stgm.hGlobal = NULL;
	stgm.pUnkForRelease = NULL;
	FORMATETC fmtetc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if (FAILED(a_pDataObj->GetData(&fmtetc, &stgm)))
		return;
	HDROP hDrop = static_cast<HDROP>(GlobalLock(stgm.hGlobal));
	if (hDrop)
	{
		UINT nCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if (nCount)
		{
			for (UINT i = 0; i < nCount; i++)
			{
				TCHAR szFilePath[MAX_PATH] = _T("");
				DragQueryFile(hDrop, i, szFilePath, MAX_PATH);
				if (szFilePath[0] == _T('\0'))
					continue;
				if (a_pFileList == NULL)
				{
					RWCoCreateInstance(a_pFileList, __uuidof(EnumStrings));
					if (a_pFileList == NULL)
						break;
				}
				a_pFileList->Insert(CComBSTR(szFilePath));
			}
		}
	}
	GlobalUnlock(stgm.hGlobal);
	ReleaseStgMedium(&stgm);
}

HRESULT CTabbedPage::DragImpl(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	POINT tPt = {a_pt.x, a_pt.y};
	m_pDragFeedback = NULL;
	bool bHandled = false;
	if (m_pDVWnd)
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pDVWnd->QueryInterfaces(__uuidof(IDragAndDropHandler), EQIFVisible, p);
		CComPtr<IDragAndDropHandler> pDD;
		for (ULONG i = 0; SUCCEEDED(p->Get(i, __uuidof(IDragAndDropHandler), reinterpret_cast<void**>(&pDD))); ++i, pDD = NULL)
		{
			m_pDragFeedback = NULL; // just in case
			if (pDD && SUCCEEDED(pDD->Drag(m_pDragData, m_pDragFiles, a_grfKeyState, tPt, a_pdwEffect, &m_pDragFeedback)))
			{
				bHandled = true;
				break;
			}
		}
	}
	if (!bHandled)
	{
		if (m_pDragFiles)
		{
			*a_pdwEffect = DROPEFFECT_COPY;
			m_pDragFeedback = _SharedStringTable.GetString(IDS_DROPMSG_OPENFILES);
		}
		else
		{
			*a_pdwEffect = DROPEFFECT_NONE;
		}
	}
	if (m_pDragFeedback)
	{
		CComBSTR bstr;
		m_pDragFeedback->GetLocalized(m_tLocaleID, &bstr);
		COLE2T str(bstr.m_str);
		m_wndTips.TrackPosition(tPt.x+26, tPt.y+16);
		if (m_strLastTip.empty())
		{
			m_strLastTip = str;
			m_tToolTip.lpszText = str;
			m_wndTips.SetToolInfo(&m_tToolTip);
			m_wndTips.TrackActivate(&m_tToolTip, TRUE);
		}
		else if (m_strLastTip != str.operator LPTSTR())
		{
			m_strLastTip = str;
			m_tToolTip.lpszText = str;
			m_wndTips.SetToolInfo(&m_tToolTip);
		}
	}
	else
	{
		if (!m_strLastTip.empty())
		{
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
			m_strLastTip.clear();
		}
	}
	return S_OK;
}

STDMETHODIMP CTabbedPage::DragEnter(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	try
	{
		if (GetWindowLong(GWL_STYLE)&WS_DISABLED)
		{
			*a_pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		//FORMATETC fmtetc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		//HRESULT hRes2 = a_pDataObj->QueryGetData(&fmtetc);
		//TCHAR szTmp[32];
		//_stprintf(szTmp, _T("0x%08x"), hRes2);
		//MessageBox(szTmp, _T("Result"), MB_OK);
		m_pDragData = a_pDataObj;
		FilesFromDrop(a_pDataObj, m_pDragFiles);
		HRESULT hRes = DragImpl(a_grfKeyState, a_pt, a_pdwEffect);
		if (FAILED(hRes))
		{
			*a_pdwEffect = DROPEFFECT_NONE;
			return hRes;
		}
		if (GetDropHelper())
		{
			POINT tPt = {a_pt.x, a_pt.y};
			GetDropHelper()->DragEnter(m_hWnd, a_pDataObj, &tPt, *a_pdwEffect);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CTabbedPage::DragOver(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	try
	{
		if (GetWindowLong(GWL_STYLE)&WS_DISABLED)
		{
			*a_pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		HRESULT hRes = DragImpl(a_grfKeyState, a_pt, a_pdwEffect);
		if (GetDropHelper())
		{
			POINT tPt = {a_pt.x, a_pt.y};
			GetDropHelper()->DragOver(&tPt, *a_pdwEffect);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CTabbedPage::DragLeave()
{
	if (GetWindowLong(GWL_STYLE)&WS_DISABLED)
		return S_OK;

	m_pDragData = NULL;
	m_pDragFiles = NULL;
	m_pDragFeedback = NULL;
	if (!m_strLastTip.empty())
	{
		m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		m_strLastTip.clear();
	}
	if (m_pDVWnd)
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pDVWnd->QueryInterfaces(__uuidof(IDragAndDropHandler), EQIFAll, p);
		CComPtr<IDragAndDropHandler> pDD;
		for (ULONG i = 0; SUCCEEDED(p->Get(i, __uuidof(IDragAndDropHandler), reinterpret_cast<void**>(&pDD))); ++i, pDD = NULL)
		{
			CComPtr<ILocalizedString> pDragFeedback;
			static POINT tPt = {-1, -1};
			DWORD dw;
			pDD->Drag(NULL, NULL, 0, tPt, &dw, &pDragFeedback);
		}
	}
	if (GetDropHelper())
		GetDropHelper()->DragLeave();
	return S_OK;
}

STDMETHODIMP CTabbedPage::Drop(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	if (GetWindowLong(GWL_STYLE)&WS_DISABLED)
	{
		*a_pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}

	m_pDragData = NULL;
	m_pDragFiles = NULL;
	m_pDragFeedback = NULL;
	if (!m_strLastTip.empty())
	{
		m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		m_strLastTip.clear();
	}
	POINT tPt = {a_pt.x, a_pt.y};
	if (GetDropHelper())
		GetDropHelper()->Drop(a_pDataObj, &tPt, *a_pdwEffect);
	CComPtr<IEnumStringsInit> pFiles;
	FilesFromDrop(a_pDataObj, pFiles);
	if (m_pDVWnd)
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pDVWnd->QueryInterfaces(__uuidof(IDragAndDropHandler), EQIFVisible, p);
		CComPtr<IDragAndDropHandler> pDD;
		for (ULONG i = 0; SUCCEEDED(p->Get(i, __uuidof(IDragAndDropHandler), reinterpret_cast<void**>(&pDD))); ++i, pDD = NULL)
		{
			if (pDD && pDD->Drop(a_pDataObj, pFiles, a_grfKeyState, tPt) == S_OK)
			{
				if (a_pdwEffect) *a_pdwEffect = (DROPEFFECT_COPY&*a_pdwEffect) ? DROPEFFECT_COPY : ((DROPEFFECT_MOVE&*a_pdwEffect) ? DROPEFFECT_MOVE : DROPEFFECT_LINK);
				return S_OK;
			}
		}
	}
	ULONG nFiles = 0;
	if (pFiles) pFiles->Size(&nFiles);
	if (nFiles)
	{
		CComPtr<IDesignerCore> pDesignerCore;
		RWCoCreateInstance(pDesignerCore, __uuidof(DesignerCore));
		for (ULONG i = nFiles; i > 1; --i)
		{
			CComBSTR bstr;
			pFiles->Get(i-1, &bstr);
			pDesignerCore->NewWindowPath(bstr, NULL);
		}

		CComBSTR bstr;
		pFiles->Get(0, &bstr);
		if (M_Doc())
		{
			pDesignerCore->NewWindowPath(bstr, NULL);
		}
		else
		{
			CComPtr<IInputManager> pInMgr;
			RWCoCreateInstance(pInMgr, __uuidof(InputManager));
			CComPtr<IDocument> pDoc;
			pInMgr->DocumentCreate(CStorageFilter(bstr), NULL, &pDoc);
			SetNewDocument(pDoc);
		}
		if (a_pdwEffect) *a_pdwEffect = (DROPEFFECT_COPY&*a_pdwEffect) ? DROPEFFECT_COPY : ((DROPEFFECT_MOVE&*a_pdwEffect) ? DROPEFFECT_MOVE : DROPEFFECT_LINK);
		return S_OK;
	}
	if (a_pdwEffect) *a_pdwEffect = DROPEFFECT_NONE;
	return S_OK;
}

LRESULT CTabbedPage::OnToolsOptions(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	static bool s_bInit = false;
	if (!s_bInit)
	{
		INITCOMMONCONTROLSEX tICCE = {sizeof(tICCE), ICC_USEREX_CLASSES};
		InitCommonControlsEx(&tICCE);
		s_bInit = true;
	}

	CConfigValue cSizeX;
	CConfigValue cSizeY;
	CConfigValue cLastPage;
	CComBSTR cCFGID_DLGS_APPOPTIONS_SIZEX(CFGID_DLGS_APPOPTIONS_SIZEX);
	CComBSTR cCFGID_DLGS_APPOPTIONS_SIZEY(CFGID_DLGS_APPOPTIONS_SIZEY);
	CComBSTR cCFGID_DLGS_APPOPTIONS_LASTPAGE(CFGID_DLGS_APPOPTIONS_LASTPAGE);
	m_pThread->M_Config()->ItemValueGet(cCFGID_DLGS_APPOPTIONS_SIZEX, &cSizeX);
	m_pThread->M_Config()->ItemValueGet(cCFGID_DLGS_APPOPTIONS_SIZEY, &cSizeY);
	m_pThread->M_Config()->ItemValueGet(cCFGID_DLGS_APPOPTIONS_LASTPAGE, &cLastPage);

	LONG nSizeX = cSizeX;
	LONG nSizeY = cSizeY;
	GUID tLastPage = cLastPage;
	CGlobalOptionsDlg cDlg(m_tLocaleID, GetIcon(FALSE), &nSizeX, &nSizeY, &tLastPage);
	if (cDlg.DoModal() == IDOK)
	{
		// TODO: consider observing config changes
		UpdateUndoMode();
		UpdateCaption();
		CConfigValue cLCID;
		{
			CComPtr<IGlobalConfigManager> pGlobalConfigMgr;
			RWCoCreateInstance(pGlobalConfigMgr, __uuidof(GlobalConfigManager));
			CComPtr<IConfig> pLangCfg;
			pGlobalConfigMgr->Config(__uuidof(Translator), &pLangCfg);
			pLangCfg->ItemValueGet(CComBSTR(CFGID_LANGUAGECODE), &cLCID);
		}
		if (cLCID.TypeGet() == ECVTInteger && cLCID.operator LONG() != m_tLocaleID)
		{
			m_tLocaleID = cLCID.operator LONG();
			m_pThread->SetLocaleID(cLCID.operator LONG());
			RECT rcClient;
			::GetWindowRect(m_hWndClient, &rcClient);
			ScreenToClient(&rcClient);
			if (m_pDVWnd)
				m_pDVWnd->OnDeactivate(FALSE);
			DestroyView();
			if (m_pCurrentLayoutCfg)
			{
				CreateView(m_pCurrentLayoutCfg, rcClient);
			}
			else
			{
				CComObject<CDesignerViewWndStart>* pViewWnd = NULL;
				CComObject<CDesignerViewWndStart>::CreateInstance(&pViewWnd);
				CComPtr<IDesignerView> pTmp = pViewWnd;

				RECT rc;
				GetClientRect(&rc);
				CConfigValue cStartPage;
				{
					CTabbedWindowThread::CConfigLock cLock(m_pThread);
					m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_STARTPAGE), &cStartPage);
					pViewWnd->Init(m_pThread->M_Config(), this, CObserverImpl<CTabbedPage, IStatusBarObserver, BYTE>::ObserverGet(), m_hWnd, &rc, m_tLocaleID, m_pThread->M_StartPageCount(), m_pThread->M_StartPages(), cStartPage, m_tStartPage, m_pThread->M_StorageManager(), m_pAppName);
					m_tStartPage = GUID_NULL;
				}

				m_wndMenuBar.SetCloseButtonID(0);
				UpdateRootMenu();

				m_pDVWnd.Attach(pTmp.Detach());

				m_pDVWnd->Handle(reinterpret_cast<RWHWND*>(&m_hWndClient));
				::SetFocus(m_hWndClient);
			}
			UpdateLayout();
			UpdateCaption();
		}
	}
	if (nSizeX != cSizeX.operator LONG() || nSizeY != cSizeY.operator LONG() || !IsEqualGUID(tLastPage, cLastPage))
	{
		BSTR aIDs[3] = {cCFGID_DLGS_APPOPTIONS_SIZEX, cCFGID_DLGS_APPOPTIONS_SIZEY, cCFGID_DLGS_APPOPTIONS_LASTPAGE};
		TConfigValue aVals[3];
		ZeroMemory(aVals, sizeof aVals);
		aVals[0].eTypeID = ECVTInteger;
		aVals[0].iVal = nSizeX;
		aVals[1].eTypeID = ECVTInteger;
		aVals[1].iVal = nSizeY;
		aVals[2].eTypeID = ECVTGUID;
		aVals[2].guidVal = tLastPage;
		m_pThread->M_Config()->ItemValuesSet(3, aIDs, aVals);
	}

	return 0;
}

LRESULT CTabbedPage::OnLayoutOperation(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled, bool a_bAtMousePos)
{
	CMenuCommands::const_iterator i = m_cMenuCommands.find(a_wID);
	if (i != m_cMenuCommands.end())
	{
		CWaitCursor cDummy;
		m_pErrorMessage = NULL;
		HandleOperationResult(i->second->Execute(m_hWnd, m_tLocaleID));
	}
	return 0;
}

void CTabbedPage::SetNewDocument(IDocument* a_pNewDoc)
{
	if (m_pDoc)
	{
		m_pDoc->ObserverDel(CObserverImpl<CTabbedPage, IDocumentObserver, ULONG>::ObserverGet(), 0);
		SaveLastLayout();
	}
	m_pDoc = a_pNewDoc;
	m_pDocUndo.Release();
	m_pDocUndo = m_pDoc;
	UpdateUndoMode();
	if (m_pDoc)
		m_pDoc->ObserverIns(CObserverImpl<CTabbedPage, IDocumentObserver, ULONG>::ObserverGet(), 0);
	m_cViewStates.clear();

	m_pCurrentLayoutCfg = NULL;
	m_strActiveLayout.Empty();
	RefreshDocument();
}

void CTabbedPage::OwnerNotify(TCookie, ULONG a_nChangeFlags)
{
	if (a_nChangeFlags & (EDCDirtyness|EDCLocation))
	{
		UpdateCaption();
	}
}

void CTabbedPage::OwnerNotify(TCookie, BYTE dummy)
{
	UpdateStatusBar();
}

bool CTabbedPage::AskAndCloseDocument()
{
	if (m_pDVWnd)
		m_pDVWnd->OnDeactivate(FALSE);

	bool bRet = false;

	CConfigValue cAsk;
	m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_APPASKTOSAVE), &cAsk);
	if (cAsk.operator bool() && M_Doc() && M_Doc()->IsDirty() == S_OK)
	{
		TCHAR szTempl[128] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SAVECHANGESTO, szTempl, itemsof(szTempl), LANGIDFROMLCID(m_tLocaleID));
		TCHAR szName[256] = _T("");
		CDocumentName::GetDocName(M_Doc(), szName, itemsof(szName));
		TCHAR szText[384] = _T("");
		_stprintf(szText, szTempl, szName);
		int nResult;
		if (XPGUI::IsVista())
		{
			TCHAR szCaption[256] = _T("");
			if (m_pAppName)
			{
				CComBSTR bstrCaption;
				m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
				USES_CONVERSION;
				_tcscpy(szCaption, OLE2CT(bstrCaption));
			}
			else
			{
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDR_MAINFRAME, szCaption, itemsof(szCaption), LANGIDFROMLCID(m_tLocaleID));
			}
			TASKDIALOGCONFIG tTDC;
			ZeroMemory(&tTDC, sizeof tTDC);
			tTDC.cbSize = sizeof tTDC;
			tTDC.hwndParent = m_hWnd;
			tTDC.hInstance = _pModule->get_m_hInst();
			tTDC.dwFlags = TDF_USE_COMMAND_LINKS|TDF_ALLOW_DIALOG_CANCELLATION|TDF_POSITION_RELATIVE_TO_WINDOW;
			tTDC.pszWindowTitle = szCaption;
			tTDC.pszMainIcon = TD_WARNING_ICON;
			tTDC.pszMainInstruction = szText;
			TCHAR szContent[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SAVECHANGES_CONTENT, szContent, itemsof(szContent), LANGIDFROMLCID(m_tLocaleID));
			tTDC.pszContent = szContent;
			tTDC.cButtons = 3;
			TCHAR szBtnYes[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SAVECHANGES_YES, szBtnYes, itemsof(szBtnYes), LANGIDFROMLCID(m_tLocaleID));
			TCHAR szBtnNo[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SAVECHANGES_NO, szBtnNo, itemsof(szBtnNo), LANGIDFROMLCID(m_tLocaleID));
			TCHAR szBtnCancel[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SAVECHANGES_CANCEL, szBtnCancel, itemsof(szBtnCancel), LANGIDFROMLCID(m_tLocaleID));
			TASKDIALOG_BUTTON aButtons[3] =
			{
				{IDYES, szBtnYes},
				{IDNO, szBtnNo},
				{IDCANCEL, szBtnCancel},
			};
			tTDC.pButtons = aButtons;
			tTDC.nDefaultButton = IDYES;
			TaskDialogIndirect(&tTDC, &nResult, NULL, NULL);
		}
		else
		{
			nResult = AppMessageBox(szText, MB_YESNOCANCEL|MB_ICONWARNING);
		}
		switch (nResult)
		{
		case IDNO:
			bRet = true;
			break;
		case IDCANCEL:
			break;
		case IDYES:
			{
				BOOL bDummy;
				if (OnFileSave(0, 0, 0, bDummy) != 0)
				{
					bRet = true;
				}
			}
			break;
		}
	}
	else
	{
		bRet = true;
	}

	SaveLastLayout();

	if (bRet)
	{
		m_pCurrentLayoutCfg = NULL;
	}

	return bRet;
}

void CTabbedPage::SaveLastLayout() const
{
	CComPtr<IStorageFilter> pMainFlt;
	CComBSTR bstrFileName;
	CTabbedWindowThread::CConfigLock cLock(m_pThread);
	if (CDocumentName::IsValidFilter(M_Doc()) && SUCCEEDED(M_Doc()->LocationGet(&pMainFlt)) && pMainFlt)
	{
		pMainFlt->ToText(NULL, &bstrFileName);
		if (bstrFileName != NULL)
		{
			GUID tBuilderID = GUID_NULL;
			if (M_Doc())
				M_Doc()->BuilderID(&tBuilderID);
			if (RecentFiles::UpdateRecentFile(m_pThread->M_Config(), bstrFileName, m_strActiveLayout, tBuilderID, m_cViewStates) != S_OK)
				RecentFiles::InsertRecentFile(m_pThread->M_Config(), bstrFileName, m_strActiveLayout, tBuilderID, m_cViewStates);
		}
	}
	CConfigValue cSaveLayout;
	m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_SAVELAYOUT), &cSaveLayout);
	if (cSaveLayout)
	{
		// save current config
		CConfigValue cVal;
		m_pThread->M_Config()->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cVal);
		LONG nLayouts = cVal;
		for (LONG i = 0; i < nLayouts; ++i)
		{
			OLECHAR szTmp[64];
			_swprintf(szTmp, L"%s\\%08x", CFGID_VIEWPROFILES, i);
			CComBSTR bstrTmp(szTmp);
			m_pThread->M_Config()->ItemValueGet(bstrTmp, &cVal);
			if (m_strActiveLayout == cVal.operator BSTR())
			{
				CComPtr<IConfig> pLayoutCfg;
				m_pThread->M_Config()->SubConfigGet(bstrTmp, &pLayoutCfg);
				CopyConfigValues(pLayoutCfg, m_pCurrentLayoutCfg);
				break;
			}
		}
	}
}

void CTabbedPage::UpdateUndoMode()
{
	if (m_pDocUndo != NULL)
	{
		CConfigValue cVal;
		m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_UNDOMODE), &cVal);
		m_pDocUndo->UndoModeSet(static_cast<EUndoMode>(cVal.operator LONG()));
	}
}

int CTabbedPage::AppMessageBox(LPCTSTR a_pszText, UINT a_nFlags) const
{
	TCHAR szCaption[256] = _T("");
	if (m_pAppName)
	{
		CComBSTR bstrCaption;
		m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
		USES_CONVERSION;
		_tcscpy(szCaption, OLE2CT(bstrCaption));
	}
	else
	{
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDR_MAINFRAME, szCaption, itemsof(szCaption), LANGIDFROMLCID(m_tLocaleID));
	}
	return ::MessageBox(m_hWnd, a_pszText, szCaption, a_nFlags);
}

void CTabbedPage::UpdateLayout(BOOL bResizeBars)
{
	Win32LangEx::CLangFrameWindowImpl<CTabbedPage>::UpdateLayout(bResizeBars);
	if (m_wndStatusBar.m_hWnd && bResizeBars)
	{
		// get pane positions
		int* pPanesPos = NULL;
		int nPanes = m_wndStatusBar.GetParts(0, NULL);
		ATLTRY(pPanesPos = (int*)_alloca(nPanes * sizeof(int)));
		if(pPanesPos == NULL)
			return;
		m_wndStatusBar.GetParts(nPanes, pPanesPos);
		// calculate offset
		RECT rcClient;
		m_wndStatusBar.GetClientRect(&rcClient);
		int cxOff = rcClient.right - (pPanesPos[nPanes - 1] + ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE));
		for(int i = 0; i < nPanes; i++)
			pPanesPos[i] += cxOff;
		// set pane postions
		m_wndStatusBar.SetParts(nPanes, pPanesPos);
	}
}

LRESULT CTabbedPage::OnMenuSelect(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
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
			WORD wID = LOWORD(a_wParam);
			CMenuCommands::const_iterator i = m_cMenuCommands.find(wID);
			if (i != m_cMenuCommands.end())
			{
				CComPtr<ILocalizedString> pStr;
				i->second->Description(&pStr);
				if (pStr)
				{
					pStr->GetLocalized(m_tLocaleID, &m_bstrCommandDesc);
					if (m_bstrCommandDesc) for (LPOLESTR p = m_bstrCommandDesc; *p; ++p)
						if (*p == L'\n') { *p = L'\0'; break; }
				}
			}
		}
	}
	UpdateStatusBar();

	return 1;
}

STDMETHODIMP CTabbedPage::Update(IDesignerStatusBar* a_pStatusBar)
{
	if (m_bShowCommandDesc)
	{
		a_pStatusBar->SimpleModeSet(m_bstrCommandDesc);
	}
	return S_OK;
}

LRESULT CTabbedPage::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	m_CmdBar.SendMessage(a_uMsg, a_wParam, a_lParam);
	m_wndMenuBar.SendMessage(a_uMsg, a_wParam, a_lParam);
	UpdateLayout();
	if (m_hWndClient)
		::SendMessage(m_hWndClient, a_uMsg, a_wParam, a_lParam);
	return 0;
}

LRESULT CTabbedPage::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		HH_POPUP hhp;
		hhp.cbStruct = sizeof(hhp);
		RECT rcItem;
		UINT uID = IDS_CONTEXTHELP_NONE;
		if (pHelpInfo->hItemHandle == m_wndMenuBar || pHelpInfo->hItemHandle == m_CmdBar)
		{
			uID = IDS_CONTEXTHELP_MENUBAR;
			m_wndMenuBar.GetWindowRect(&rcItem);
			hhp.pt.x = (rcItem.right+rcItem.left)>>1;
			hhp.pt.y = rcItem.bottom;
		}
		else if (pHelpInfo->hItemHandle == m_wndStatusBar)
		{
			uID = IDS_CONTEXTHELP_STATUSBAR;
			m_wndStatusBar.GetWindowRect(&rcItem);
			hhp.pt.x = (rcItem.right+rcItem.left)>>1;
			hhp.pt.y = rcItem.top-100;
		}
		else
		{
			::GetWindowRect((HWND)pHelpInfo->hItemHandle, &rcItem);
			hhp.pt.x = (rcItem.right+rcItem.left)>>1;
			hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
		}
		TCHAR szBuffer[512] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), uID, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
		hhp.hinst = _pModule->get_m_hInst();
		hhp.idString = 0;
		hhp.pszText = szBuffer;
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

void CTabbedPage::HandleOperationResult(HRESULT a_hRes)
{
	if (FAILED(a_hRes) && a_hRes != E_RW_CANCELLEDBYUSER)
	{
		CComBSTR bstr;
		if (m_pErrorMessage)
			m_pErrorMessage->GetLocalized(m_tLocaleID, &bstr);
		CComBSTR bstrCaption;
		m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
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
*/