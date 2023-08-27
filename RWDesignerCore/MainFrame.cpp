#include "stdafx.h"
#include "MainFrame.h"

#include "StringParsing.h"

#include "ConfigFrameDlg.h"
#include "GlobalOptionsDlg.h"
#include "ConfigIDsApp.h"
#include "ThreadCommand.h"
#include "WindowThread.h"
#include "DesignerViewWndStart.h"
#include "RecentFiles.h"
#include <DocumentName.h>
#include "ConfigGUILayout.h"
#include "ManageConfigurableItemsDlg.h"

#include <XPGUI.h>
#include "AbbreviateName.h"
#include <HtmlHelp.h>
#include <Win32LangEx.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <RWLocalization.h>


CMainFrame::CMainFrame() :
	m_pThread(NULL), m_tStartPage(GUID_NULL), m_bDragHelperCreated(false),
	m_bStatusBarVisible(TRUE),  m_bShowCommandDesc(false), m_dwLastOnIdle(0),
	m_pOperMgr(NULL), m_nStatusBarTools(0), m_uFreeID(ID_LAYOUT_MENU1),
	m_hIconSmall(NULL), m_hIconLarge(NULL), m_bLangChangePosted(false)
{
	RWCoCreateInstance(m_pIM, __uuidof(InputManager));
	CComObject<CDesignerViewStatusBar>::CreateInstance(&m_pDesignerViewStatusBar.p);
	if (m_pDesignerViewStatusBar.p)
		m_pDesignerViewStatusBar.p->AddRef();
}

CMainFrame::~CMainFrame()
{
	if (m_pDoc)
		m_pDoc->ObserverDel(CObserverImpl<CMainFrame, IDocumentObserver, ULONG>::ObserverGet(), 0);
	if (m_pLanguageCfg)
		m_pLanguageCfg->ObserverDel(CObserverImpl<CMainFrame, IConfigObserver, IUnknown*>::ObserverGet(), 0);
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

#include <wininet.h>

HRESULT CMainFrame::Init(CThreadCommand* a_pOrder, CWindowThread* a_pThread)
{
	m_pThread = a_pThread;
	m_tLocaleID = a_pThread->GetLocaleID();

	CComObject<CDesignerFrameOperationManager>::CreateInstance(&m_pOperMgr);
	m_pOperMgr->AddRef();
	m_pOperMgr->Init(m_pThread->M_OperMgr(), this);

	CComObject<CDesignerFrameMenuCommandsManager>::CreateInstance(&m_pCmdsMgr);
	m_pCmdsMgr->AddRef();
	m_pCmdsMgr->Init(m_pThread->M_MenuCmdsMgr(), m_pOperMgr, this);

	CComObject<CDesignerFrameViewManager>::CreateInstance(&m_pViewMgr);
	m_pViewMgr->AddRef();
	m_pViewMgr->Init(m_pThread->M_ViewMgr(), m_pCmdsMgr);

	if (a_pOrder->IsIDocument())
	{
		SetNewDocument(a_pOrder->M_IDocument());
	}
	else if (a_pOrder->IsSourceName())
	{
		// TODO: implement ??? here ???
		CComPtr<IInputManager> pInMgr;
		RWCoCreateInstance(pInMgr, __uuidof(InputManager));
		CComPtr<IDocument> pDoc;
		pInMgr->DocumentCreateEx(GetBuilders(), CStorageFilter(a_pOrder->M_SourceName()), NULL, &pDoc);
		if (pDoc)
		{
			pDoc->ClearDirty();
			SetNewDocument(pDoc);
		}
	}
	else if (a_pOrder->IsStartPage())
	{
		m_tStartPage = a_pOrder->M_StartPage();
		static bool bConnected = false;
		if (!bConnected && IsEqualGUID(m_tStartPage, __uuidof(StartPageOnline)))
		{
			static DWORD dwCheckedAt = GetTickCount()-1000*60*60; // checked hourt ago
			if (GetTickCount()-dwCheckedAt > 300000UL) // checked more than 5 minutes ago
			{
				DWORD dwFlags = 0;
				bConnected = InternetGetConnectedState(&dwFlags, 0);
				dwCheckedAt = GetTickCount();
			}
			if (!bConnected)
			{
				CConfigValue cCount;
				a_pThread->M_Config()->ItemValueGet(CComBSTR(RecentFiles::CFGID_RECENTFILES), &cCount);
				m_tStartPage = cCount.operator LONG() ? __uuidof(StartViewPageFactoryRecentFiles) : __uuidof(StartViewPageFactoryNewDocument);
			}
		}
	}

	CComPtr<IGlobalConfigManager> pGlobalConfigMgr;
	RWCoCreateInstance(pGlobalConfigMgr, __uuidof(GlobalConfigManager));
	if (pGlobalConfigMgr)
		pGlobalConfigMgr->Config(__uuidof(Translator), &m_pLanguageCfg);
	if (m_pLanguageCfg)
		m_pLanguageCfg->ObserverIns(CObserverImpl<CMainFrame, IConfigObserver, IUnknown*>::ObserverGet(), 0);

	return S_OK;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
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
	if (pMsg->message == WM_MOUSEWHEEL)
	{
		// forward mouse wheel message to the window under the mouse pointer
		POINT tPt = {GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam)};
		HWND hWnd = WindowFromPoint(tPt);
		if (hWnd)
		{
			DWORD dwPID = 0;
			GetWindowThreadProcessId(hWnd, &dwPID);
			if (dwPID && GetCurrentProcessId() == dwPID)
			{
				MSG tMsg = *pMsg;
				tMsg.hwnd = hWnd;
				DispatchMessage(&tMsg);
				return TRUE;
			}
		}
	}
	if (m_pDVWnd)
	{
		if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
			/*pMsg->wParam != VK_RETURN && pMsg->wParam != VK_ESCAPE*/pMsg->wParam == VK_TAB && ::IsDialogMessage(m_hWndClient, pMsg))
			return TRUE;
		if (m_pDVWnd->PreTranslateMessage(pMsg, FALSE) == S_OK)
			return TRUE;
	}

	return FALSE;
}

bool CMainFrame::ExecuteAccelerator(IEnumUnknowns* a_pCommands, WORD a_wKeyCode, WORD a_fVirtFlags)
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

BOOL CMainFrame::OnIdle()
{
	// TODO: watch clipboard and update
	// TODO: update main menu

	UpdateStatusBar();

	DWORD dwLastOnIdle = m_dwLastOnIdle;
	m_dwLastOnIdle = 0;
	if (dwLastOnIdle != 0 && (GetTickCount()-dwLastOnIdle) < 20)
		return S_FALSE;
	return m_pDVWnd && m_pDVWnd->OnIdle() == S_OK;
}

void CMainFrame::UpdateStatusBar()
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
	m_nStatusBarTools = 0;
	for (size_t i = 0; i < m_cStatusBarTools.size(); ++i)
	{
		// find related item in items
		CStatusBarItems::iterator s = m_cStatusBarItems.begin();
		while (s != m_cStatusBarItems.end() && s->nPane != int(i)) ++s;

		ULONG nStamp = m_cStatusBarTools[i].second;
		if (S_OK == m_cStatusBarTools[i].first->IsVisible(m_pDoc, &m_cStatusBarTools[i].second) && m_cStatusBarTools[i].second)
		{
			++m_nStatusBarTools;
			IStatusBarPane* pPane = m_cStatusBarTools[i].first;
			// visible
			if (s == m_cStatusBarItems.end())
			{
				// insert new item
				bChange = true;

				SStatusBarItem sItem;
				CComPtr<ILocalizedString> pText;
				pPane->Text(&pText);
				CComBSTR bstrText;
				if (pText) pText->GetLocalized(m_tLocaleID, &bstrText);
				sItem.strText = bstrText.m_str ? bstrText.m_str : L"";
				sItem.hIcon = NULL;
				pPane->Icon(XPGUI::GetSmallIconSize(), &sItem.hIcon);
				CComPtr<ILocalizedString> pTooltip;
				pPane->Tooltip(&pTooltip);
				CComBSTR bstrTooltip;
				if (pTooltip) pTooltip->GetLocalized(m_tLocaleID, &bstrTooltip);
				sItem.strTooltip = bstrTooltip ? bstrTooltip.m_str : L"";
				ULONG nWidth = XPGUI::GetSmallIconSize();
				pPane->MaxWidth(&nWidth);
				sItem.nWidth = nWidth;
				sItem.nPane = i;

				m_cStatusBarItems.push_back(sItem);
			}
			else if (nStamp != m_cStatusBarTools[i].second)
			{
				// update item
				bChange = true;

				CComPtr<ILocalizedString> pText;
				pPane->Text(&pText);
				CComBSTR bstrText;
				if (pText) pText->GetLocalized(m_tLocaleID, &bstrText);
				s->strText = bstrText ? bstrText.m_str : L"";
				if (s->hIcon) ATLVERIFY(DestroyIcon(s->hIcon));
				s->hIcon = NULL;
				pPane->Icon(XPGUI::GetSmallIconSize(), &s->hIcon);
				CComPtr<ILocalizedString> pTooltip;
				pPane->Tooltip(&pTooltip);
				CComBSTR bstrTooltip;
				if (pTooltip) pTooltip->GetLocalized(m_tLocaleID, &bstrTooltip);
				s->strTooltip = bstrTooltip ? bstrTooltip.m_str : L"";
				ULONG nWidth = XPGUI::GetSmallIconSize();
				pPane->MaxWidth(&nWidth);
				s->nWidth = nWidth;
			}
			// else no change
		}
		else
		{
			// invisible
			if (nStamp)
			{
				// was visible, remove it
				if (s != m_cStatusBarItems.end()) // should be always
				{
					bChange = true;
					if (s->hIcon) ATLVERIFY(DestroyIcon(s->hIcon));
					m_cStatusBarItems.erase(s);
				}
			}
		}
	}
	// TODO: when dynamic status bar pane plug-in loading is implemented, remove extra items from the vector and free icons

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
			COLE2CT str(bstr ? bstr : L"");
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

void CMainFrame::InitToolBarsAndMenu(int dpi)
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
	m_CmdBar.SetPadding((6*dpi+48)/96, (6*dpi+48)/96);
	m_CmdBar.m_pMenuTextProvider = this;
	m_CmdBar.SetCommandBarExtendedStyle(CBR_EX_TRACKALWAYS|CBR_EX_TRANSPARENT);
	//m_CmdBar.SetImageSize(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize());
	//m_CmdBar.SetAlphaImages(XPGUI::IsXP());
	//m_CmdBar.m_hImageList =	::ImageList_Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 16, 16);

	UpdateRootMenu();

	m_wndMenuBar.Create(m_hWnd, 0, 0, 0, 0, ATL_IDW_COMMAND_BAR+1);
	m_wndMenuBar.Init(m_CmdBar);
	//RECT rc;
	//m_CmdBar.GetItemRect(0, &rc);
	//m_CmdBar.MoveWindow(0, 0, rc.right, rc.bottom);

	//CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	//AddSimpleReBarBandCtrl(m_hWndToolBar, m_CmdBar, 0, NULL, FALSE, 0, TRUE);
	m_hWndToolBar = m_wndMenuBar;

	// insert external status bar pane handlers
	CComPtr<IEnumUnknowns> pTools;
	m_pThread->M_PlugInCache()->InterfacesEnum(CATID_StatusBarPane, __uuidof(IStatusBarPane), 0xffffffff, &pTools, NULL);
	CComPtr<IStatusBarPane> pPane;
	for (ULONG iToolSet = 0; SUCCEEDED(pTools->Get(iToolSet, __uuidof(IStatusBarPane), reinterpret_cast<void**>(&pPane))); iToolSet++, pPane = NULL)
	{
		pair<CComPtr<IStatusBarPane>, ULONG> tSBTool;
		tSBTool.first = pPane;
		tSBTool.second = 0;
		//if (S_OK != tSBTool.first->IsVisible(m_pDoc, &tSBTool.second))
		//	tSBTool.second = 0;
		//--tSBTool.second;
		m_cStatusBarTools.push_back(tSBTool);
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

void CMainFrame::GetMainMenu(IEnumUnknowns** a_pCmds)
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

void CMainFrame::UpdateRootMenu()
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
				// TODO: when menu are updated on every change, uncomment this:
				//CComPtr<IEnumUnknowns> pSubCmds;
				//pCmd->SubCommands(&pSubCmds);
				//ULONG nSubCmds = 0;
				//if (pSubCmds && SUCCEEDED(pSubCmds->Size(&nSubCmds)) && nSubCmds)
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

bool CMainFrame::RefreshDocument()
{
	if (m_hWnd == NULL)
		return false;

	try
	{
		DestroyView();

		m_cViewStates.clear();
		m_cChangedStateIDs.clear();
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
				CConfigLock cLock(m_pThread);
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

				std::vector<std::pair<CComBSTR, CComBSTR> > cCompatible;
				pBestCfg.Attach(FindBestLayout(M_Doc(), m_pViewMgr, m_pThread->M_Config(), m_strActiveLayout, cCompatible));
				if (cCompatible.size() > 1)
				{
					m_wndMenuBar.SetOptions(cCompatible, m_strActiveLayout, m_tLocaleID);
				}
				else
				{
					std::vector<std::pair<CComBSTR, CComBSTR> > cNone;
					m_wndMenuBar.SetOptions(cNone, NULL, m_tLocaleID);
				}

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
				CConfigLock cLock(m_pThread);
				m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_STARTPAGE), &cStartPage);
				pViewWnd->Init(m_pThread->M_Config(), this, CObserverImpl<CMainFrame, IStatusBarObserver, BYTE>::ObserverGet(), m_hWnd, &rc, m_tLocaleID, m_pThread->M_StartPageCount(), m_pThread->M_StartPages(), cStartPage, m_tStartPage, m_pThread->M_StorageManager(), m_pAppName);
				m_tStartPage = GUID_NULL;
			}

			std::vector<std::pair<CComBSTR, CComBSTR> > cNone;
			m_wndMenuBar.SetOptions(cNone, NULL, m_tLocaleID);
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

LRESULT CMainFrame::OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	a_bHandled = FALSE;

	HDC hDC = GetDC();
	int dpi = GetDeviceCaps(hDC, LOGPIXELSX);
	m_pDesignerViewStatusBar->SetScale(dpi / 96.0f);
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

	InitToolBarsAndMenu(dpi);

	RefreshDocument(); // TODO: error code

	// register object for message filtering and idle updates
	m_pThread->AddMessageFilter(this);
	m_pThread->AddIdleHandler(this);

	RegisterDragDrop(m_hWnd, this);

	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileClose(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (AskAndCloseDocument())
	{
		m_pDocUndo.Release();
		if (m_pDoc)
			m_pDoc->ObserverDel(CObserverImpl<CMainFrame, IDocumentObserver, ULONG>::ObserverGet(), 0);
		m_pDoc.Release();
		RefreshDocument();
	}
	return 0;
}

void CMainFrame::ShowStatusBar(bool a_bShow)
{
	if (a_bShow != m_bStatusBarVisible)
	{
		m_bStatusBarVisible = a_bShow;
		::ShowWindow(m_hWndStatusBar, m_bStatusBarVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UpdateLayout();
	}
}


HRESULT CMainFrame::InitConfig(IConfigWithDependencies* a_pMainCfg, IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager, IStorageManager* a_pStorageManager)
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

	LONG const nFSX = GetSystemMetrics(SM_CXFULLSCREEN);
	LONG const nFSY = GetSystemMetrics(SM_CYFULLSCREEN);
	LONG const nWinX = (nFSY-200L)*8L/5L < nFSX-200L ? (nFSY-200L)*8L/5L : nFSX-200L;
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_X_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_X_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_X_POS), CConfigValue((nFSX-nWinX)>>1), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_Y_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_Y_POS), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_Y_POS), CConfigValue(100L), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_WIDTH), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_WIDTH), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_WIDTH), CConfigValue(nWinX), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_HEIGHT), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_HEIGHT), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_HEIGHT), CConfigValue(nFSY-200L), NULL, 0, NULL);
	a_pMainCfg->ItemInsSimple(CComBSTR(CFGID_WIN_MAXIMIZED), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_MAXIMIZED), _SharedStringTable.GetStringAuto(IDS_CFGID_WIN_MAXIMIZED), CConfigValue(nFSY <= 768), NULL, 0, NULL);

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

LRESULT CMainFrame::OnFileSave(bool a_bForceInteractive)
{
	if (m_pDVWnd)
	{
		CWaitCursor cDummyStackObject;
		m_pDVWnd->OnDeactivate(FALSE);
	}

	CComPtr<IStorageFilterWindowListener> pWindowListener;
	CComPtr<IEnumUnknowns> pFlts;
	CComPtr<IConfig> pSaveCfg;
	bool m_bDataTypeOK = S_OK == m_pIM->SaveOptionsGet(M_Doc(), &pSaveCfg, &pFlts, &pWindowListener);

	CComPtr<IStorageFilter> pFlt;
	CComBSTR bstrFlt;
	if (CDocumentName::IsValidFilter(M_Doc()) && m_bDataTypeOK)
	{
		if (a_bForceInteractive)
		{
			CComPtr<IStorageFilter> pOrigFlt;
			M_Doc()->LocationGet(&pOrigFlt);
			if (pOrigFlt != NULL)
				pOrigFlt->ToText(NULL, &bstrFlt);
		}
		else
		{
			M_Doc()->LocationGet(&pFlt);
		}
	}
	else
	{
		OLECHAR szBuffer[MAX_PATH] = L"";
		CDocumentName::GetDocName(M_Doc(), szBuffer, itemsof(szBuffer));
		bstrFlt = szBuffer;
	}

	bool bChangeLoc = false;
	if (pFlt == NULL)
	{
		if (bstrFlt == NULL)
			bstrFlt = L"";

		{
			CComPtr<IConfig> pStorageContext;
			//CConfigLock cLock(m_pThread);
			m_pThread->M_Config()->SubConfigGet(CComBSTR(CFGID_STORAGESAVE), &pStorageContext);
			m_pThread->M_StorageManager()->FilterCreateInteractivelyCfgHelp(bstrFlt, EFTCreateNew, m_hWnd, pFlts, pSaveCfg, pStorageContext, _SharedStringTable.GetStringAuto(IDS_SAVEDLGCAPTION), CComBSTR(L"save-file"), pWindowListener, m_tLocaleID, &pFlt);
		}

		if (pFlt)
			bChangeLoc = true;
			//M_Doc()->LocationSet(pFlt);
	}

	if (pFlt != NULL)
	{
		LRESULT lRes = SaveFile(M_Doc(), pSaveCfg, m_tLocaleID, pFlt);

		if (lRes && bChangeLoc)
		{
			M_Doc()->LocationSet(pFlt);
			GUID tEncID = GUID_NULL;
			CComPtr<IConfig> pEncCfg;
			m_pIM->SaveEncoder(pSaveCfg, &tEncID, &pEncCfg);
			M_Doc()->EncoderSet(tEncID, pEncCfg);
		}
		if (lRes)
			M_Doc()->ClearDirty();

		UpdateCaption();
		return lRes;
	}

	return 0;
}

LRESULT CMainFrame::SaveFile(IDocument* a_pDoc, IConfig* a_pSaveCfg, LCID a_tLocaleID, IStorageFilter* a_pDst)
{
	CComPtr<IApplicationInfo> pAI;
	RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
	ULONG nDays = 0;
	CComBSTR bstrMsg;
	ELicensingMode eMode = ELMDonate;
	pAI->LicensingMode(&eMode);
	if (eMode != ELMEnterSerial || S_OK == pAI->License(NULL, NULL, NULL, NULL, &nDays) || nDays <= 30)
	{
		CWaitCursor cDummyStackObject;
		HRESULT hRes = m_pIM->Save(a_pDoc, a_pSaveCfg, a_pDst);
		if (SUCCEEDED(hRes))
			return 1;
		if (hRes == ERROR_OUTOFMEMORY)
		{
			CMultiLanguageString::GetLocalized(L"[0409]Saving of file failed.\n\nThe destination drive is out of space. Please try a different location.[0405]Ukládání souboru selhalo.\n\nNa cílovém disku není dostatek místa. Prosím zkuste soubor uložit jinam.", a_tLocaleID, &bstrMsg);
		}
		else if (hRes == E_ACCESSDENIED)
		{
			CMultiLanguageString::GetLocalized(L"[0409]Saving of file failed.\n\nYou do not have write permissions in this folder. Please try a different location.[0405]Ukládání souboru selhalo.\n\nV této složce nemáte potřebné zapisovací oprávnění. Prosím zkuste soubor uložit jinam.", a_tLocaleID, &bstrMsg);
		}
		else
		{
			CMultiLanguageString::GetLocalized(L"[0409]Saving of file failed.\n\nPlease try a different location.[0405]Ukládání souboru selhalo.\n\nProsím zkuste soubor uložit jinam.", a_tLocaleID, &bstrMsg);
		}
	}
	else
	{
		CMultiLanguageString::GetLocalized(L"[0409]Evaluation period expired.\n\nSaving functionality will be restored after valid license is entered.[0405]Zkušební doba vypršela.\n\nUkládání souborů bude umožněno po vložení platné licence.", a_tLocaleID, &bstrMsg);
	}
	AppMessageBox(COLE2CT(bstrMsg.m_str), MB_OK|MB_ICONERROR);
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
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

	for (CStatusBarItems::iterator i = m_cStatusBarItems.begin(); i != m_cStatusBarItems.end(); ++i)
	{
		if (i->nPane >= 0 && i->hIcon)
			ATLVERIFY(DestroyIcon(i->hIcon));
	}

	if (m_wndTips.IsWindow())
		m_wndTips.DestroyWindow();

	return 0;
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

IConfig* CMainFrame::CreateLayoutConfig(IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager)
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

		pProfile->ItemInsSimple(CComBSTR(CFGID_LAYOUTNAME), CMultiLanguageString::GetAuto(L"[0409]Layout name[0405]Jméno layoutu"), CMultiLanguageString::GetAuto(L"[0409]Descriptive name of the window layout.[0405]Popisné jméno tohoto rozložení okna."), CConfigValue(L"[0409]Layout name[0405]Jméno layoutu"), NULL, 0, NULL);
		pProfile->ItemInsSimple(CComBSTR(CFGID_LAYOUTVERB), CMultiLanguageString::GetAuto(L"[0409]Layout verb[0405]Jméno příkazu"), CMultiLanguageString::GetAuto(L"[0409]Short command used to quickly switch layouts.[0405]Krátký příkaz používaný k rychlému přepnutí layoutu."), CConfigValue(L""), NULL, 0, NULL);

		CComObject<CLayoutDocTypeOptions>* p = NULL;
		CComObject<CLayoutDocTypeOptions>::CreateInstance(&p);
		CComPtr<IConfigItemCustomOptions> pDocTypesOpetions = p;
		pProfile->ItemIns1ofNWithCustomOptions(CComBSTR(CFGID_DOCBUILDER), CMultiLanguageString::GetAuto(L"[0409]Document type[0405]Typ dokumentu"), CMultiLanguageString::GetAuto(L"[0409]If set, the layout will be usable only with documents of the selected type.[0405]Pokud je nastaveno, bude layout použitelný pouze s dokumenty vybraného typu."), CConfigValue(GUID_NULL), pDocTypesOpetions, NULL, 0, NULL);

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

ISubConfig* CMainFrame::CreateLayoutsConfig(IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager)
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

bool CMainFrame::CreateView(IConfig* a_pViewProfile, const RECT& a_rcView)
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

		if (FAILED(m_pViewMgr->CreateWnd(m_pViewMgr, cSelectedView, pViewCfg, this, CObserverImpl<CMainFrame, IStatusBarObserver, BYTE>::ObserverGet(), M_Doc(), reinterpret_cast<RWHWND>(m_hWnd), &a_rcView, EDVWSVolatileBorder, m_tLocaleID, &m_pDVWnd)))
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

bool CMainFrame::DestroyView()
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

LRESULT CMainFrame::OnProfileManage(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	CComPtr<IConfig> pCopy;
	{
		CConfigLock cLock(m_pThread);
		m_pThread->M_Config()->DuplicateCreate(&pCopy);
	}
	if (CManageLayoutsDlg(pCopy, m_pThread->M_Config(), m_tLocaleID, m_pThread->M_DesignerFrameIcons()).DoModal() == IDOK)
	{
		CConfigLock cLock(m_pThread);
		CopySelectedConfigValues(m_pThread->M_Config(), pCopy, CFGID_VIEWPROFILES);
	}

	return 0;
}

void CMainFrame::UpdateMenu(CMenuHandle& a_cMenu, IEnumUnknowns* a_pCmds)
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
			str = bstrName.Length() ? bstrName.m_str : L"";
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

UINT CMainFrame::AllocateMenuID()
{
	if (m_cFreeIDs.empty())
		return m_uFreeID++;
	UINT x = m_cFreeIDs.front();
	m_cFreeIDs.pop();
	return x;
}

void CMainFrame::ReleaseMenuID(UINT a_nID)
{
	m_cFreeIDs.push(a_nID);
}


LRESULT CMainFrame::OnInitMenuPopup(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM UNREF(a_lParam), BOOL& a_bHandled)
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

void CMainFrame::SwitchLayout(LPCOLESTR a_pszLayoutName, bool a_bChangeOrder)
{
	CComBSTR bstrNew(a_pszLayoutName);

	CConfigValue cVal;
	if (FAILED(m_pThread->M_Config()->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cVal)))
		return;
	LONG nLayouts = cVal;
	if (nLayouts == 0)
		return;

	std::vector<std::pair<CComBSTR, CComBSTR> > cCompatible;

	OLECHAR szNameID[64];
	LONG iNew = -1;
	LONG iOld = -1;

	CLSID tBuilderID = GUID_NULL;
	m_pDoc->BuilderID(&tBuilderID);

	if (!IsEqualGUID(tBuilderID, GUID_NULL))
	{
		// have builder ID -> try to find layouts that match
		LONG iSelected = -1;
		for (LONG i = 0; i < nLayouts; ++i)
		{
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
			CConfigValue cName;
			m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cName);
			if (bstrNew == cName.operator BSTR())
				iNew = i;
			if (m_strActiveLayout == cName.operator BSTR())
				iOld = i;
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DOCBUILDER);
			CConfigValue cBuilderID;
			m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cBuilderID);
			if (IsEqualGUID(tBuilderID, cBuilderID))
			{
				_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DESIGNERVIEW);
				CConfigValue cView;
				m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cView);
				_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_LAYOUTVERB);
				CConfigValue cVerb;
				m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cVerb);
				std::pair<CComBSTR, CComBSTR> t;
				t.first = cName;
				t.second = cVerb;
				cCompatible.push_back(t);
			}
		}
	}
	if (cCompatible.empty())
	{
		for (LONG i = 0; i < nLayouts; i++)
		{
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
			CConfigValue cName;
			m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cName);
			if (bstrNew == cName.operator BSTR())
				iNew = i;
			if (m_strActiveLayout == cName.operator BSTR())
				iOld = i;
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DESIGNERVIEW);
			CConfigValue cView;
			m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cView);
			CComPtr<IConfig> pViewCfg;
			m_pThread->M_Config()->SubConfigGet(CComBSTR(szNameID), &pViewCfg);
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_LAYOUTVERB);
			CConfigValue cVerb;
			m_pThread->M_Config()->ItemValueGet(CComBSTR(szNameID), &cVerb);
			CComObjectStackEx<CCheckSuitabilityCallback> cCallback;
			m_pViewMgr->CheckSuitability(m_pViewMgr, cView, pViewCfg, m_pDoc, &cCallback);
			if (cCallback.cMissing.empty() && !cCallback.cUsed.empty())
			{
				std::pair<CComBSTR, CComBSTR> t;
				t.first = cName;
				t.second = cVerb;
				cCompatible.push_back(t);
			}
		}
	}
	if (iNew == -1)
		return;
	if (a_bChangeOrder && iOld > -1 && iNew > iOld)
	{
		CComPtr<IConfig> pCfg;
		m_pThread->M_Config()->SubConfigGet(CComBSTR(CFGID_VIEWPROFILES), &pCfg);
		CComQIPtr<IConfigVector> pVec(pCfg);
		if (pVec)
		{
			pVec->Move(iNew, iOld);
			iNew = iOld;
		}
	}
	{
		CComPtr<IConfig> pViewCfg;
		CConfigLock cLock(m_pThread);
		OLECHAR szNameID[64];
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, iNew);
		if (FAILED(m_pThread->M_Config()->SubConfigGet(CComBSTR(szNameID), &pViewCfg)))
			return; // TODO: report error
		m_pCurrentLayoutCfg = NULL;
		pViewCfg->DuplicateCreate(&m_pCurrentLayoutCfg);
	}
	RECT rcClient;
	::GetWindowRect(m_hWndClient, &rcClient);
	ScreenToClient(&rcClient);
	if (m_pDVWnd)
		m_pDVWnd->OnDeactivate(FALSE);
	DestroyView();
	CreateView(m_pCurrentLayoutCfg, rcClient);
	m_strActiveLayout = a_pszLayoutName;
	UpdateCaption();
	if (cCompatible.size() > 1)
	{
		m_wndMenuBar.SetOptions(cCompatible, m_strActiveLayout, m_tLocaleID);
	}
	else
	{
		std::vector<std::pair<CComBSTR, CComBSTR> > cNone;
		m_wndMenuBar.SetOptions(cNone, NULL, m_tLocaleID);
	}
}

LRESULT CMainFrame::OnLayoutChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	if (m_wndMenuBar.m_hWnd == a_pNMHdr->hwndFrom && m_pDoc)
	{
		SwitchLayout(*reinterpret_cast<LPCOLESTR*>(a_pNMHdr+1), true);
	}
	return 0;
}

LRESULT CMainFrame::OnProfileConfigureCurrent(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	typedef CConfigFrameDlg<CFGID_DLGS_CONFIGURELAYOUT_SIZEX, CFGID_DLGS_CONFIGURELAYOUT_SIZEY, HELPTOPIC_CONFIGURELAYOUT, IDS_CONFIGURECURRENTLAYOUT, GetIconLayout> CConfigureCurrentLayoutDlg;
	CComPtr<IConfig> pCopy;
	m_pCurrentLayoutCfg->DuplicateCreate(&pCopy);
	CConfigureCurrentLayoutDlg cDlg(pCopy, m_pThread->M_Config(), m_tLocaleID, COLE2T(m_strActiveLayout));
	if (cDlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(m_pThread->M_Config())) == IDOK)
	{
		CWaitCursor cWait;
		CopyConfigValues(m_pCurrentLayoutCfg, pCopy);
		if (cDlg.ShouldSaveConfig())
		{
			CConfigLock cLock(m_pThread);
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

IConfig* CMainFrame::FindBestLayout(IDocument* a_pDoc, IViewManager* a_pMgr, IConfig* a_pMainConfig, CComBSTR& a_strName, std::vector<std::pair<CComBSTR, CComBSTR> >& a_cCompatible)
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
					_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DESIGNERVIEW);
					CConfigValue cView;
					a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cView);
					_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_LAYOUTVERB);
					CConfigValue cVerb;
					a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cVerb);
					std::pair<CComBSTR, CComBSTR> t;
					t.first = cName;
					t.second = cVerb;
					a_cCompatible.push_back(t);
					continue;
				}
			}
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DOCBUILDER);
			CConfigValue cBuilderID;
			a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cBuilderID);
			if (IsEqualGUID(tBuilderID, cBuilderID))
			{
				if (iSelected == -1)
					iSelected = i;
				_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_DESIGNERVIEW);
				CConfigValue cView;
				a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cView);
				_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x\\%s", CFGID_VIEWPROFILES, i, CFGID_LAYOUTVERB);
				CConfigValue cVerb;
				a_pMainConfig->ItemValueGet(CComBSTR(szNameID), &cVerb);
				std::pair<CComBSTR, CComBSTR> t;
				t.first = cName;
				t.second = cVerb;
				a_cCompatible.push_back(t);
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
		if (aRatings[i].nOthers == 0 && aRatings[i].nUsed)
		{
			std::pair<CComBSTR, CComBSTR> t;
			t.first = cName;
			t.second = cVerb;
			a_cCompatible.push_back(t);
		}
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

STDMETHODIMP CMainFrame::StateSet(BSTR a_bstrCategoryName, ISharedState* a_pState)
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
			m_cChangedStateIDs.insert(a_bstrCategoryName);
			PostMessage(WM_SENDSTATECHANGES);
		}
		else
		{
			m_cChangedStateIDs.insert(a_bstrCategoryName);
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

LRESULT CMainFrame::OnSendStateChanges(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	ObjectLock cLock(this);
	if (m_pDVWnd)
		UpdateRootMenu();
	while (!m_cChangedStateIDs.empty())
	{
		CStateIDs::iterator j = m_cChangedStateIDs.begin();
		CViewStates::iterator i = m_cViewStates.find(std::wstring(*j));
		if (i != m_cViewStates.end() && i->second.p)
		{
			TSharedStateChange tTmp = {*j, i->second.p};
			CComPtr<ISharedState> pRef(tTmp.pState);
			for (i->second.nActNotifying = 0; i->second.nActNotifying < int(m_aObservers.size()); ++i->second.nActNotifying)
			{
				m_aObservers[i->second.nActNotifying].first->Notify(m_aObservers[i->second.nActNotifying].second, tTmp);
			}
		}
		m_cChangedStateIDs.erase(j);
	}
	m_cChangedStateIDs.clear();
	if (m_pDVWnd)
	{
		m_pDVWnd->OnIdle();
		m_dwLastOnIdle = GetTickCount();
	}
	return 0;
}

STDMETHODIMP CMainFrame::StateGet(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
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

STDMETHODIMP CMainFrame::ObserverIns(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
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
STDMETHODIMP CMainFrame::ObserverDel(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
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

LRESULT CMainFrame::OnClose(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (AskAndCloseDocument())
		bHandled = FALSE;

	return 0;
}

void CMainFrame::UpdateCaption()
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
	CComBSTR bstrProps;
	if ((cVal.operator LONG() & CFGFLAG_CAPTION_PROPS) && M_Doc())
	{
		CComPtr<ILocalizedString> pQI;
		M_Doc()->QuickInfo(0, &pQI);
		if (pQI) pQI->GetLocalized(m_tLocaleID, &bstrProps);
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
			if (bstrProps.Length())
			{
				_tcscat(szCaption, _T(", "));
				_tcscat(szCaption, COLE2CT(bstrProps));
			}
			_tcscat(szCaption, _T(")"));
		}
	}
	else if (bstrProps.Length())
	{
			_tcscat(szCaption, szCaption[0] ? _T(" (") : _T("("));
			_tcscat(szCaption, COLE2CT(bstrProps));
			_tcscat(szCaption, _T(")"));
	}
	CComBSTR bstrLN;
	if ((cVal.operator LONG() & CFGFLAG_CAPTION_LAYOUT) && M_Doc() && m_pCurrentLayoutCfg)
	{
		CConfigValue cLN;
		m_pCurrentLayoutCfg->ItemValueGet(CComBSTR(CFGID_LAYOUTNAME), &cLN);
		CMultiLanguageString::GetLocalized(cLN.operator BSTR(), m_tLocaleID, &bstrLN);
	}
	bool bHaveAppName = false;
	if ((cVal.operator LONG() & CFGFLAG_CAPTION_APP) || M_Doc() == NULL || (szCaption[0] == _T('\0') && bstrLN.Length() == 0))
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
			_tcscat(szCaption, _T("RealWorld Designer"));
		}
		bHaveAppName = true;
	}
	if (bstrLN.Length() && M_Doc())
	{
		if (bHaveAppName)
			_tcscat(szCaption, _T(" / "));
		else if (szCaption[0])
			_tcscat(szCaption, _T(" - "));
#ifdef _UNICODE
		_tcscat(szCaption, bstrLN.m_str);
#else
		std::wstring str(p, p+n);
		_tcscat(szCaption, COLE2CT(str.c_str()));
#endif
	}

	SetWindowText(szCaption);
}

LRESULT CMainFrame::OnFileMRU(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	vector<tstring> cMRUList;
	{
		CConfigLock cLock(m_pThread);
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

void CMainFrame::FilesFromDrop(IDataObject* a_pDataObj, CComPtr<IEnumStringsInit>& a_pFileList)
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

HRESULT CMainFrame::DragImpl(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

STDMETHODIMP CMainFrame::DragEnter(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

STDMETHODIMP CMainFrame::DragOver(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

STDMETHODIMP CMainFrame::DragLeave()
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

STDMETHODIMP CMainFrame::Drop(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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
			pInMgr->DocumentCreateEx(GetBuilders(), CStorageFilter(bstr), NULL, &pDoc);
			if (pDoc) pDoc->ClearDirty();
			SetNewDocument(pDoc);
		}
		if (a_pdwEffect) *a_pdwEffect = (DROPEFFECT_COPY&*a_pdwEffect) ? DROPEFFECT_COPY : ((DROPEFFECT_MOVE&*a_pdwEffect) ? DROPEFFECT_MOVE : DROPEFFECT_LINK);
		return S_OK;
	}
	if (a_pdwEffect) *a_pdwEffect = DROPEFFECT_NONE;
	return S_OK;
}

LRESULT CMainFrame::OnToolsOptions(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
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
		OwnerNotify(NULL, (IUnknown*)NULL);
		UpdateUndoMode();
		UpdateCaption();
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

LRESULT CMainFrame::OnLayoutOperation(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled, bool a_bAtMousePos)
{
	CComPtr<IDocumentMenuCommand> pCmd = m_cMenuCommands[a_wID];
	if (pCmd)
	{
		CWaitCursor cDummy;
		m_pErrorMessage = NULL;
		HandleOperationResult(pCmd->Execute(m_hWnd, m_tLocaleID));
	}
	return 0;
}

void CMainFrame::SetNewDocument(IDocument* a_pNewDoc)
{
	if (m_pDoc)
	{
		m_pDoc->ObserverDel(CObserverImpl<CMainFrame, IDocumentObserver, ULONG>::ObserverGet(), 0);
		SaveLastLayout();
	}
	m_pDoc = a_pNewDoc;
	m_pDocUndo.Release();
	m_pDocUndo = m_pDoc;
	UpdateUndoMode();
	if (m_pDoc)
		m_pDoc->ObserverIns(CObserverImpl<CMainFrame, IDocumentObserver, ULONG>::ObserverGet(), 0);
	m_cViewStates.clear();

	m_pCurrentLayoutCfg = NULL;
	m_strActiveLayout.Empty();
	RefreshDocument();
}

void CMainFrame::OwnerNotify(TCookie, ULONG a_nChangeFlags)
{
	if (a_nChangeFlags & (EDCDirtyness|EDCLocation|EDCQuickInfo))
	{
		UpdateCaption();
	}
}

void CMainFrame::OwnerNotify(TCookie, BYTE dummy)
{
	UpdateStatusBar();
}

bool CMainFrame::AskAndCloseDocument()
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
			TCHAR szCaption[256] = _T("RealWorld Designer");
			if (m_pAppName)
			{
				CComBSTR bstrCaption;
				m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
				USES_CONVERSION;
				_tcscpy(szCaption, OLE2CT(bstrCaption));
			}
			TASKDIALOGCONFIG tTDC;
			ZeroMemory(&tTDC, sizeof tTDC);
			tTDC.cbSize = sizeof tTDC;
			tTDC.hwndParent = m_hWnd;
			//tTDC.hInstance = _pModule->get_m_hInst();
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
			for (LPTSTR p = szBtnYes; *p; ++p) if (*p == _T('|')) *p = _T('\n');
			TCHAR szBtnNo[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SAVECHANGES_NO, szBtnNo, itemsof(szBtnNo), LANGIDFROMLCID(m_tLocaleID));
			for (LPTSTR p = szBtnNo; *p; ++p) if (*p == _T('|')) *p = _T('\n');
			TCHAR szBtnCancel[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_SAVECHANGES_CANCEL, szBtnCancel, itemsof(szBtnCancel), LANGIDFROMLCID(m_tLocaleID));
			for (LPTSTR p = szBtnCancel; *p; ++p) if (*p == _T('|')) *p = _T('\n');
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
				if (OnFileSave(false) != 0)
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

void CMainFrame::SaveLastLayout() const
{
	CComPtr<IStorageFilter> pMainFlt;
	CComBSTR bstrFileName;
	CConfigLock cLock(m_pThread);
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

void CMainFrame::UpdateUndoMode()
{
	if (m_pDocUndo != NULL)
	{
		CConfigValue cVal;
		m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_UNDOMODE), &cVal);
		m_pDocUndo->UndoModeSet(static_cast<EUndoMode>(cVal.operator LONG()));
	}
}

int CMainFrame::AppMessageBox(LPCTSTR a_pszText, UINT a_nFlags) const
{
	TCHAR szCaption[256] = _T("RealWorld Designer");
	if (m_pAppName)
	{
		CComBSTR bstrCaption;
		m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
		USES_CONVERSION;
		_tcscpy(szCaption, OLE2CT(bstrCaption));
	}
	return ::MessageBox(m_hWnd, a_pszText, szCaption, a_nFlags);
}

void CMainFrame::UpdateLayout(BOOL bResizeBars)
{
	Win32LangEx::CLangFrameWindowImpl<CMainFrame>::UpdateLayout(bResizeBars);
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

LRESULT CMainFrame::OnMenuSelect(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
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

STDMETHODIMP CMainFrame::Update(IDesignerStatusBar* a_pStatusBar)
{
	if (m_bShowCommandDesc)
	{
		a_pStatusBar->SimpleModeSet(m_bstrCommandDesc);
	}
	return S_OK;
}

LRESULT CMainFrame::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	m_CmdBar.SendMessage(a_uMsg, a_wParam, a_lParam);
	m_wndMenuBar.SendMessage(a_uMsg, a_wParam, a_lParam);
	UpdateLayout();
	if (m_hWndClient)
		::SendMessage(m_hWndClient, a_uMsg, a_wParam, a_lParam);
	return 0;
}

LRESULT CMainFrame::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
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

LRESULT CMainFrame::OnLanguageChange(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_pLanguageCfg == NULL)
		return 0;
	try
	{
		m_bLangChangePosted = false;
		CConfigValue cLCID;
		m_pLanguageCfg->ItemValueGet(CComBSTR(CFGID_LANGUAGECODE), &cLCID);
		if (cLCID.TypeGet() == ECVTInteger && (cLCID.operator LONG() != m_tLocaleID || (cLCID.operator LONG() != 0x0409 && cLCID.operator LONG() != 0x0405)))
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
					CConfigLock cLock(m_pThread);
					m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_STARTPAGE), &cStartPage);
					pViewWnd->Init(m_pThread->M_Config(), this, CObserverImpl<CMainFrame, IStatusBarObserver, BYTE>::ObserverGet(), m_hWnd, &rc, m_tLocaleID, m_pThread->M_StartPageCount(), m_pThread->M_StartPages(), cStartPage, m_tStartPage, m_pThread->M_StorageManager(), m_pAppName);
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
			{
				const int cchMax = 128;   // max text length is 127 for status bars (+1 for null)
				TCHAR szText[cchMax];
				szText[0] = _T('\0');
				Win32LangEx::LoadString(_pModule->get_m_hInst(), ATL_IDS_IDLEMESSAGE, szText, cchMax, LANGIDFROMLCID(m_tLocaleID));
				m_wndStatusBar.SetWindowText(szText);
			}
		}
	}
	catch (...)
	{
	}
	return 0;
}

void CMainFrame::HandleOperationResult(HRESULT a_hRes)
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

#include "OpenAsChoices.h"

CComPtr<IEnumUnknownsInit> CMainFrame::GetBuilders()
{
	CComPtr<IEnumUnknownsInit> pBuilders;
	RWCoCreateInstance(pBuilders, __uuidof(EnumUnknowns));
	COpenAsChoicesHelper cList;
	cList.GetBuilders(m_pThread->M_Config(), pBuilders);
	return pBuilders;
}
