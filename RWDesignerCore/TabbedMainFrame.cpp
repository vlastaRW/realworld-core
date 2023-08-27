#include "stdafx.h"
/*
#include "TabbedMainFrame.h"

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

CTabbedMainFrame::CTabbedMainFrame() : m_bGlass(false), m_bTracking(false),
	m_pThread(NULL), m_tStartPage(GUID_NULL), m_bDragHelperCreated(false),
	m_bGlobeHot(false), m_bIntelliTipHot(false), m_bContextHelpHot(false),
	m_hIconSmall(NULL), m_hIconLarge(NULL)
{
}

CTabbedMainFrame::~CTabbedMainFrame()
{
	m_cQuickToolsImages.Destroy();
	m_cMenuImages.Destroy();
	if (m_hIconSmall)
		DestroyIcon(m_hIconSmall);
	if (m_hIconLarge)
		DestroyIcon(m_hIconLarge);
}

HRESULT CTabbedMainFrame::Init(CThreadCommand* a_pOrder, CTabbedWindowThread* a_pThread)
{
	m_pThread = a_pThread;
	m_pOrder = a_pOrder;
	m_tLocaleID = a_pThread->GetLocaleID();

	//if (a_pOrder->IsIDocument())
	//{
	//	m_pDoc = a_pOrder->M_IDocument();
	//	m_pDocUndo = m_pDoc;
	//	UpdateUndoMode();
	//	if (m_pDoc)
	//		m_pDoc->ObserverIns(CObserverImpl<CTabbedMainFrame, IDocumentObserver, ULONG>::ObserverGet(), 0);
	//}
	//else if (a_pOrder->IsSourceName())
	//{
	//	// TODO: implement ??? here ???
	//	CComPtr<IInputManager> pInMgr;
	//	RWCoCreateInstance(pInMgr, __uuidof(InputManager));
	//	pInMgr->DocumentCreate(CStorageFilter(a_pOrder->M_SourceName()), NULL, &m_pDoc);
	//	m_pDocUndo = m_pDoc;
	//	UpdateUndoMode();
	//	if (m_pDoc)
	//		m_pDoc->ObserverIns(CObserverImpl<CTabbedMainFrame, IDocumentObserver, ULONG>::ObserverGet(), 0);
	//}
	//else if (a_pOrder->IsStartPage())
	//{
	//	m_tStartPage = a_pOrder->M_StartPage();
	//}
	return S_OK;
}

BOOL CTabbedMainFrame::PreTranslateMessage(MSG* pMsg)
{
	//if (m_pDVWnd && m_pDVWnd->PreTranslateMessage(pMsg, TRUE) == S_OK)
	//	return TRUE;
	//if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
	//	pMsg->wParam != VK_MENU && pMsg->wParam != VK_SHIFT && pMsg->wParam != VK_CONTROL &&
	//	pMsg->wParam != VK_LMENU && pMsg->wParam != VK_LSHIFT && pMsg->wParam != VK_LCONTROL &&
	//	pMsg->wParam != VK_RMENU && pMsg->wParam != VK_RSHIFT && pMsg->wParam != VK_RCONTROL)
	//{
	//	CComPtr<IEnumUnknowns> pCmds;
	//	GetMainMenu(&pCmds);
	//	if (ExecuteAccelerator(pCmds, pMsg->wParam, (GetKeyState(VK_MENU)&0x8000 ? FALT : 0) | (GetKeyState(VK_CONTROL)&0x8000 ? FCONTROL : 0) | (GetKeyState(VK_SHIFT)&0x8000 ? FSHIFT : 0)))
	//		return TRUE;
	//}
	//if (m_pDVWnd)
	//{
	//	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
	//		//pMsg->wParam != VK_RETURN && pMsg->wParam != VK_ESCAPE
			pMsg->wParam == VK_TAB && ::IsDialogMessage(m_hWndClient, pMsg))
	//		return TRUE;
	//	if (m_pDVWnd->PreTranslateMessage(pMsg, FALSE) == S_OK)
	//		return TRUE;
	//}

	return FALSE;
}

BOOL CTabbedMainFrame::OnIdle()
{
	// TODO: watch clipboard and update
	// TODO: update main menu

	return FALSE;
	//UpdateStatusBar();

	//return m_pDVWnd && m_pDVWnd->OnIdle() == S_OK;
}

bool IsDWMAvailable()
{
    HMODULE hLib = ::LoadLibrary(_T("dwmapi.dll"));
	if (hLib)
	{
		BOOL bDummy;
		::DwmIsCompositionEnabled(&bDummy); // dummy call to make the delay-load bindings before freeing the lib
		FreeLibrary(hLib);
		return true;
	}
	return false;
}

void CTabbedMainFrame::UpdateDrawingConstants()
{
	static bool const s_bDWMAvailable = IsDWMAvailable();
	if (s_bDWMAvailable)
	{
		BOOL bActive = FALSE;
		::DwmIsCompositionEnabled(&bActive);
		m_bGlass = bActive != FALSE;
	}
	m_rcBorder.left = m_rcBorder.top = m_rcBorder.right = m_rcBorder.bottom = 0;
	//AdjustWindowRectEx(&m_rcBorder, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);
	RECT rcCaption = {0, 0, 0, 0};
	//AdjustWindowRectEx(&rcCaption, WS_OVERLAPPEDWINDOW, FALSE, NULL);
	m_nCaptionHeight = m_rcBorder.top-rcCaption.top;
	//m_rcBorder.left = -m_rcBorder.left;
	//m_rcBorder.top = -m_rcBorder.top;
	//m_rcBorder.right += 1;
	//m_rcBorder.bottom += 1;
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

//bool CTabbedMainFrame::RefreshDocument()
//{
//	try
//	{
//		DestroyView();
//
//		m_cViewStates.clear();
//		m_strActiveLayout.Empty();
//
//		if (M_Doc())
//		{
//			// show document
//			CComPtr<IStorageFilter> pMainFlt;
//			CComBSTR bstrFileName;
//			if (SUCCEEDED(M_Doc()->LocationGet(&pMainFlt)) && pMainFlt)
//			{
//				pMainFlt->ToText(NULL, &bstrFileName);
//			}
//
//			USES_CONVERSION;
//
//			CComPtr<IConfig> pBestCfg;
//			{
//				CConfigLock cLock(m_pThread);
//				CComBSTR bstrDocTypeID;
//				if (bstrFileName != NULL)
//					RecentFiles::GetRecentFileConfig(m_pThread->M_Config(), bstrFileName, m_strActiveLayout, bstrDocTypeID, m_cViewStates);
//
//				CComPtr<IDocumentType> pDocType;
//				if (M_Doc())
//					M_Doc()->DocumentTypeGet(&pDocType);
//				CComBSTR bstrNewDocTypeID;
//				if (pDocType)
//					pDocType->UniqueIDGet(&bstrNewDocTypeID);
//				if (bstrDocTypeID != bstrNewDocTypeID && bstrDocTypeID.Length())
//				{
//					m_cViewStates.clear();
//					m_strActiveLayout.Empty();
//				}
//
//				pBestCfg.Attach(FindBestLayout(M_Doc(), m_pViewMgr, m_pThread->M_Config(), m_strActiveLayout));
//
//				pBestCfg->DuplicateCreate(&m_pCurrentLayoutCfg);
//
//				// add to MRU list
//				if (bstrFileName != NULL)
//					RecentFiles::InsertRecentFile(m_pThread->M_Config(), bstrFileName, m_strActiveLayout, bstrNewDocTypeID, m_cViewStates);
//			}
//
//			// create the view
//			m_wndMenuBar.SetCloseButtonID(ID_CLOSEDOCUMENT);
//			CreateView(m_pCurrentLayoutCfg, rcDefault);
//
//			UpdateLayout();
//			UpdateCaption();
//			return true;
//		}
//		else
//		{
//			// show start view
//			CComObject<CDesignerViewWndStart>* pViewWnd = NULL;
//			CComObject<CDesignerViewWndStart>::CreateInstance(&pViewWnd);
//			CComPtr<IDesignerView> pTmp = pViewWnd;
//
//			RECT rc;
//			GetClientRect(&rc);
//			CConfigValue cStartPage;
//			{
//				CConfigLock cLock(m_pThread);
//				m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_STARTPAGE), &cStartPage);
//				pViewWnd->Init(m_pThread->M_Config(), this, CObserverImpl<CTabbedMainFrame, IStatusBarObserver, BYTE>::ObserverGet(), m_hWnd, &rc, m_tLocaleID, m_pThread->M_StartPageCount(), m_pThread->M_StartPages(), cStartPage, m_tStartPage, m_pThread->M_StorageManager(), m_pAppName);
//				m_tStartPage = GUID_NULL;
//			}
//
//			m_wndMenuBar.SetCloseButtonID(0);
//			UpdateRootMenu();
//
//			m_pDVWnd.Attach(pTmp.Detach());
//
//			m_pDVWnd->Handle(reinterpret_cast<RWHWND*>(&m_hWndClient));
//			::SetFocus(m_hWndClient);
//
//			UpdateLayout();
//			UpdateCaption();
//			return true;
//		}
//	}
//	catch (...)
//	{
//		return false;
//	}
//}

LRESULT CTabbedMainFrame::OnNCCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	HDC hDC = GetDC();
	m_fScale = GetDeviceCaps(hDC, LOGPIXELSX) / 96.0f;
	ReleaseDC(hDC);
	m_nThumbnailSizeX = m_fScale*64+0.5f;
	m_nThumbnailSizeY = m_nGlobeSize = m_fScale*48+0.5f;
	m_nLargeSize = m_fScale*32+0.5f;
	m_nSmallSize = m_fScale*16+0.5f;
	m_nGapSize = m_fScale*4+0.5f;
	m_nTabPanelWidth = m_nGlobeSize+m_nGapSize+m_nSmallSize+m_nGapSize+m_nSmallSize+m_nGapSize;
	UpdateDrawingConstants();

	a_bHandled = FALSE;
	return 0;
}

LRESULT CTabbedMainFrame::OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	//m_pDesignerViewStatusBar->SetScale(m_fScale);

	// inform application of the frame change
    RECT rcClient;
    GetWindowRect(&rcClient);
	SetWindowPos(NULL, rcClient.left, rcClient.top, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, SWP_FRAMECHANGED);

	int nSmallIcon = XPGUI::GetSmallIconSize();
	m_cQuickToolsImages.Create(nSmallIcon, nSmallIcon, XPGUI::GetImageListColorFlags(), 3, 0);
	HICON h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_HELP_LAYOUT), IMAGE_ICON, nSmallIcon, nSmallIcon, LR_DEFAULTCOLOR);
	m_cQuickToolsImages.AddIcon(h);
	DestroyIcon(h);
	h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_HELP_CONTEXT), IMAGE_ICON, nSmallIcon, nSmallIcon, LR_DEFAULTCOLOR);
	m_cQuickToolsImages.AddIcon(h);
	DestroyIcon(h);
	h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_FILE_SAVE), IMAGE_ICON, nSmallIcon, nSmallIcon, LR_DEFAULTCOLOR);
	m_cQuickToolsImages.AddIcon(h);
	DestroyIcon(h);
	m_wndQuickTools = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0,0,100,30,
				m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR), _pModule->get_m_hInst(), NULL);
	m_wndQuickTools.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DRAWDDARROWS);
	m_wndQuickTools.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndQuickTools.SetImageList(m_cQuickToolsImages);
	static TBBUTTON aButtons[] =
	{
		{2, 2, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, 0},
		{0, 0, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
		{1, 1, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
	};
	m_wndQuickTools.AddButtons(3, aButtons);
	SIZE sz;
	m_wndQuickTools.GetMaxSize(&sz);
	m_wndQuickTools.MoveWindow(0, 0, sz.cx, sz.cy);
	//CStatic wnd;
	//wnd.Create(m_hWnd, &rcClient, _T("client window"), WS_CHILDWINDOW|WS_VISIBLE);
	CComObject<CTabbedPage>* pFrameWnd = NULL;
	CComObject<CTabbedPage>::CreateInstance(&pFrameWnd);
	CComPtr<ISharedStateManager> pTmp(pFrameWnd);
	pFrameWnd->Init(m_pOrder, m_pThread);

	if (pFrameWnd->Create(m_hWnd, &rcClient, NULL, WS_CHILDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE) == NULL)
	{
		throw (HRESULT)E_FAIL;
	}
	pTmp.Detach();
	m_wndClient = *pFrameWnd;

	if (m_pThread->M_DesignerAppInfo())
	{
		m_pThread->M_DesignerAppInfo()->Icon(GetSystemMetrics(SM_CXSMICON), &m_hIconSmall);
		m_pThread->M_DesignerAppInfo()->Icon(GetSystemMetrics(SM_CXICON), &m_hIconLarge);
		SetIcon(m_hIconSmall, FALSE);
		SetIcon(m_hIconLarge, TRUE);
		m_pThread->M_DesignerAppInfo()->Name(&m_pAppName);
	}

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
	//Reset(m_cMenuImages);

	//RefreshDocument(); // TODO: error code

	// register object for message filtering and idle updates
	m_pThread->AddMessageFilter(this);
	m_pThread->AddIdleHandler(this);

	RegisterDragDrop(m_hWnd, this);

	return 0;
}

LRESULT CTabbedMainFrame::OnActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (m_bGlass)
	{
		// Extend the frame into the client area.
		MARGINS margins;

		margins.cxLeftWidth = m_rcBorder.left+m_nTabPanelWidth;
		margins.cxRightWidth = m_rcBorder.right;
		margins.cyBottomHeight = m_rcBorder.bottom;
		margins.cyTopHeight = m_rcBorder.top+m_nCaptionHeight;

		//hr =
		::DwmExtendFrameIntoClientArea(m_hWnd, &margins);
	}

	//fCallDWP = true;
	return 0;
}

LRESULT CTabbedMainFrame::OnNCCalcSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (a_wParam == TRUE)
	{
		// Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
		NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(a_lParam);

		pncsp->rgrc[0].left   = pncsp->rgrc[0].left   + 0;
		pncsp->rgrc[0].top    = pncsp->rgrc[0].top    + 0;
		pncsp->rgrc[0].right  = pncsp->rgrc[0].right  - 0;
		pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

		return 0;
	}
	a_bHandled = FALSE;
	return 0;
}

LRESULT CTabbedMainFrame::OnNCHitTest(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	LRESULT lRes = 0;
	if (::DwmDefWindowProc(m_hWnd, a_uMsg, a_wParam, a_lParam, &lRes))
		return lRes;

	POINT ptMouse = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	RECT rcWindow;
	GetWindowRect(&rcWindow);

	// Determine if hit test is for resizing, default middle (1,1).
	USHORT uRow = 1;
	USHORT uCol = 1;
	bool fOnCaption = false;

	// Determine if the point is at the top or bottom of the window.
	if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top+m_rcBorder.top+m_nCaptionHeight)
	{
		if ((ptMouse.y < (rcWindow.top+m_rcBorder.top)))
			uRow = 0;
		else
			fOnCaption = true;
	}
	else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom-m_rcBorder.bottom)
	{
		uRow = 2;
	}

	// Determine if the point is at the left or right of the window.
	if ( ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left+m_rcBorder.left+m_nTabPanelWidth)
	{
		if ((ptMouse.x < (rcWindow.left+m_rcBorder.left)))
			uCol = 0; //left side
		else
			fOnCaption = true; // that would be the tabs
	}
	else if ( ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right-m_rcBorder.right)
	{
		uCol = 2; //right side
	}

	// Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
	LRESULT hitTests[3][3] = 
	{
		{ HTTOPLEFT,	HTTOP,								HTTOPRIGHT    },
		{ HTLEFT,		fOnCaption ? HTCAPTION : HTCLIENT,	HTRIGHT       },
		{ HTBOTTOMLEFT, HTBOTTOM,							HTBOTTOMRIGHT },
	};

	return hitTests[uRow][uCol];
}

LRESULT CTabbedMainFrame::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_wndClient.m_hWnd)
		m_wndClient.MoveWindow(m_rcBorder.left+m_nTabPanelWidth, m_rcBorder.top+m_nCaptionHeight, GET_X_LPARAM(a_lParam)-m_rcBorder.left-m_rcBorder.right-m_nTabPanelWidth, GET_Y_LPARAM(a_lParam)-m_rcBorder.top-m_rcBorder.bottom-m_nCaptionHeight);
	return 0;
}

LRESULT CTabbedMainFrame::OnNCMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	ScreenToClient(&tPt);
	//m_bGlobeHot(false), m_bIntelliTipHot(false), m_bContextHelpHot(false),
	float fGlobeDeltaX = tPt.x-m_rcBorder.left-m_nGlobeSize*0.5f+0.5f;
	float fGlobeDeltaY = tPt.y-m_rcBorder.top-m_nGlobeSize*0.5f+0.5f;
	bool bGlobeHot = fGlobeDeltaX*fGlobeDeltaX + fGlobeDeltaY*fGlobeDeltaY < m_nGlobeSize*m_nGlobeSize*0.25f;
	if (bGlobeHot != m_bGlobeHot)
	{
		m_bGlobeHot = bGlobeHot;
		Invalidate(FALSE);
	}
	if (!m_bTracking)
	{
		TRACKMOUSEEVENT tTME;
		tTME.cbSize = sizeof tTME;
		tTME.dwFlags = TME_NONCLIENT|TME_LEAVE;
		tTME.hwndTrack = m_hWnd;
		tTME.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tTME);
		m_bTracking = true;
	}
	return 0;
}

LRESULT CTabbedMainFrame::OnNCMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	m_bTracking = false;
	if (m_bGlobeHot)
	{
		m_bGlobeHot = false;
		Invalidate(FALSE);
	}
	return 0;
}

LRESULT CTabbedMainFrame::OnNCLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	LRESULT lRes = 0;
	if (::DwmDefWindowProc(m_hWnd, a_uMsg, a_wParam, a_lParam, &lRes))
		return lRes;

	POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	ScreenToClient(&tPt);
	//m_bGlobeHot(false), m_bIntelliTipHot(false), m_bContextHelpHot(false),
	float fGlobeDeltaX = tPt.x-m_rcBorder.left-m_nGlobeSize*0.5f+0.5f;
	float fGlobeDeltaY = tPt.y-m_rcBorder.top-m_nGlobeSize*0.5f+0.5f;
	if (fGlobeDeltaX*fGlobeDeltaX + fGlobeDeltaY*fGlobeDeltaY < m_nGlobeSize*m_nGlobeSize*0.25f)
	{
		return 0;
	}

	a_bHandled = FALSE;
	return 0;
}

LRESULT CTabbedMainFrame::OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);

    RECT rcClient;
    GetClientRect(&rcClient);

    HTHEME hTheme = OpenThemeData(NULL, L"CompositedWindow::Window");
    if (hTheme)
    {
        HDC hdcPaint = CreateCompatibleDC(hdc);
        if (hdcPaint)
        {
			int cx = rcClient.right-rcClient.left;
			int cy = rcClient.bottom-rcClient.top;

            // Define the BITMAPINFO structure used to draw text.
            // Note that biHeight is negative. This is done because
            // DrawThemeTextEx() needs the bitmap to be in top-to-bottom
            // order.
            BITMAPINFO dib = { 0 };
            dib.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
            dib.bmiHeader.biWidth           = cx;
            dib.bmiHeader.biHeight          = -cy;
            dib.bmiHeader.biPlanes          = 1;
            dib.bmiHeader.biBitCount        = 32;
            dib.bmiHeader.biCompression     = BI_RGB;
			DWORD* pBuffer = NULL;

            HBITMAP hbm = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, reinterpret_cast<void**>(&pBuffer), NULL, 0);
            if (hbm)
            {
				DWORD const dwFill = GetSysColor(COLOR_3DFACE)|0xff000000;
				{
					DWORD* p = pBuffer+28*cx+m_rcBorder.left+1;
					for (DWORD* pEnd = p+m_nTabPanelWidth-3; p < pEnd; ++p)
						*p = 0x88888888;
				}
				{
					DWORD* p = pBuffer+29*cx+m_rcBorder.left+1;
					for (DWORD* pEnd = p+m_nTabPanelWidth-2; p < pEnd; ++p)
						*p = 0x88000000;
				}
				for (ULONG y = 30; y < 110; ++y)
				{
					DWORD* p = pBuffer+y*cx+m_rcBorder.left;
					*p = 0x88888888; ++p;
					*p = 0x88000000; ++p;
					for (DWORD* pEnd = p+m_nTabPanelWidth-2; p < pEnd; ++p)
						*p = dwFill;
				}
				{
					DWORD* p = pBuffer+110*cx+m_rcBorder.left+1;
					for (DWORD* pEnd = p+m_nTabPanelWidth-2; p < pEnd; ++p)
						*p = 0x88000000;
				}
				{
					DWORD* p = pBuffer+111*cx+m_rcBorder.left+1;
					for (DWORD* pEnd = p+m_nTabPanelWidth-3; p < pEnd; ++p)
						*p = 0x88888888;
				}

                HBITMAP hbmOld = (HBITMAP)SelectObject(hdcPaint, hbm);

                // Setup the theme drawing options.
                DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
                DttOpts.iGlowSize = 15;

                // Select a font.
                LOGFONT lgFont;
                HFONT hFontOld = NULL;
                if (SUCCEEDED(GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
                {
                    HFONT hFont = CreateFontIndirect(&lgFont);
                    hFontOld = (HFONT) SelectObject(hdcPaint, hFont);
                }

                // Draw the title.
                RECT rcPaint = rcClient;
                rcPaint.top += 8;
                //rcPaint.right -= 125;
                rcPaint.left += 60;
                //rcPaint.bottom = 50;
                //DrawThemeTextEx(hTheme, hdcPaint, 0, 0, _T("sample text"), -1, DT_LEFT | DT_WORD_ELLIPSIS, &rcPaint, &DttOpts);

				//HICON h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_FRAME_GLOBE_TOP), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
				//if (m_bGlobeHot)
				//	DrawIconEx(hdcPaint, m_rcBorder.left, m_rcBorder.top, h, 48, 48, 0, NULL, DI_NORMAL);
				//DestroyIcon(h);
				//DrawIcon(hdcPaint, m_rcBorder.left+8, m_rcBorder.left+8, m_hIconLarge);
				HICON h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_HELP_LAYOUT), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
				//DrawIconEx(hdcPaint, 56, 36, h, 16, 16, 0, NULL, DI_NORMAL);
				DestroyIcon(h);
				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_HELP_CONTEXT), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
				//DrawIconEx(hdcPaint, 76, 36, h, 16, 16, 0, NULL, DI_NORMAL);
				DestroyIcon(h);

				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_FILE_NEW), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
				//DrawThemeBackground(hTheme, hdcPaint, 0, 0, 24, 66, h, 48, 48, 0, NULL, DI_NORMAL);
				DrawIconEx(hdcPaint, m_rcBorder.left+((m_nTabPanelWidth-m_nGlobeSize)>>1), 36, h, 48, 48, 0, NULL, DI_NORMAL);
				DestroyIcon(h);

				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_FILE_OPEN), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
				//DrawThemeBackground(hTheme, hdcPaint, 0, 0, 24, 66, h, 48, 48, 0, NULL, DI_NORMAL);
				DrawIconEx(hdcPaint, m_rcBorder.left+((m_nTabPanelWidth-m_nGlobeSize)>>1), 116, h, 48, 48, 0, NULL, DI_NORMAL);
				DestroyIcon(h);

				h = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_FILE_OPENRECENT), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
				//DrawThemeBackground(hTheme, hdcPaint, 0, 0, 24, 66, h, 48, 48, 0, NULL, DI_NORMAL);
				DrawIconEx(hdcPaint, m_rcBorder.left+((m_nTabPanelWidth-m_nGlobeSize)>>1), 196, h, 48, 48, 0, NULL, DI_NORMAL);
				DestroyIcon(h);

				{
					DttOpts.dwFlags = DTT_COMPOSITED;// | DTT_GLOWSIZE;
					//DttOpts.iGlowSize = 15;
					RECT rcCreate = {m_rcBorder.left+m_nGapSize, 90, m_rcBorder.left+m_nTabPanelWidth-m_nGapSize, 140};
					DrawThemeTextEx(hTheme, hdcPaint, 0, 0, _T("Create"), -1, DT_CENTER | DT_WORD_ELLIPSIS, &rcCreate, &DttOpts);
				}

				{
					DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
					DttOpts.iGlowSize = 15;
					RECT rcCreate = {m_rcBorder.left+m_nGapSize, 170, m_rcBorder.left+m_nTabPanelWidth-m_nGapSize, 220};
					DrawThemeTextEx(hTheme, hdcPaint, 0, 0, _T("Open"), -1, DT_CENTER | DT_WORD_ELLIPSIS, &rcCreate, &DttOpts);
				}

				{
					DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
					DttOpts.iGlowSize = 15;
					RECT rcCreate = {m_rcBorder.left+m_nGapSize, 250, m_rcBorder.left+m_nTabPanelWidth-m_nGapSize, 300};
					DrawThemeTextEx(hTheme, hdcPaint, 0, 0, _T("Recent"), -1, DT_CENTER | DT_WORD_ELLIPSIS, &rcCreate, &DttOpts);
				}

                // Blit text to the frame.
                BitBlt(hdc, 0, 0, cx, cy, hdcPaint, 0, 0, SRCCOPY);

                SelectObject(hdcPaint, hbmOld);
                if (hFontOld)
                {
                    SelectObject(hdcPaint, hFontOld);
                }
                DeleteObject(hbm);
            }
            DeleteDC(hdcPaint);
        }
        CloseThemeData(hTheme);
    }

	EndPaint(&ps);

	return 0;
}

LRESULT CTabbedMainFrame::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	a_bHandled = FALSE;

	RevokeDragDrop(m_hWnd);

	WINDOWPLACEMENT sWndPlace;
	sWndPlace.length = sizeof(WINDOWPLACEMENT);
	this->GetWindowPlacement(&sWndPlace);
	{
		BSTR aIDs[6];
		TConfigValue tVals[6];
		//CComBSTR i0(CFGID_STATUSBAR); aIDs[0] = i0; CConfigValue v0(static_cast<LONG>(m_bStatusBarVisible)); tVals[0] = v0;
		CComBSTR i1(CFGID_WIN_X_POS); aIDs[1] = i1; CConfigValue v2(static_cast<LONG>(sWndPlace.rcNormalPosition.left)); tVals[1] = v2;
		CComBSTR i2(CFGID_WIN_Y_POS); aIDs[2] = i2; CConfigValue v3(static_cast<LONG>(sWndPlace.rcNormalPosition.top)); tVals[2] = v3;
		CComBSTR i3(CFGID_WIN_WIDTH); aIDs[3] = i3; CConfigValue v4(static_cast<LONG>(sWndPlace.rcNormalPosition.right - sWndPlace.rcNormalPosition.left)); tVals[3] = v4;
		CComBSTR i4(CFGID_WIN_HEIGHT); aIDs[4] = i4; CConfigValue v5(static_cast<LONG>(sWndPlace.rcNormalPosition.bottom - sWndPlace.rcNormalPosition.top)); tVals[4] = v5;
		CComBSTR i5(CFGID_WIN_MAXIMIZED); aIDs[5] = i5; CConfigValue v6(sWndPlace.showCmd == SW_SHOWMAXIMIZED); tVals[5] = v6;
		m_pThread->M_Config()->ItemValuesSet(6-1, aIDs+1, tVals);
	}

	//DestroyView();

	//for (CStatusBarTools::iterator i = m_cStatusBarTools.begin(); i != m_cStatusBarTools.end(); ++i)
	//{
	//	SStatusBarItem& sItem = m_cStatusBarItems[i-m_cStatusBarTools.begin()+m_cStatusBarItems.size()-m_nStatusBarTools];
	//	if (sItem.hIcon)
	//		ATLVERIFY(DestroyIcon(sItem.hIcon));
	//}

	if (m_wndTips.IsWindow())
		m_wndTips.DestroyWindow();

	return 0;
}

//bool CTabbedMainFrame::CreateView(IConfig* a_pViewProfile, const RECT& a_rcView)
//{
//	try
//	{
//		if (m_hWndClient != NULL || m_pDVWnd != NULL || a_pViewProfile == NULL)
//			return false;
//
//		// read selected view
//		CComBSTR cCFGID_DESIGNERVIEW(CFGID_DESIGNERVIEW);
//		CConfigValue cSelectedView;
//		if (FAILED(a_pViewProfile->ItemValueGet(cCFGID_DESIGNERVIEW, &cSelectedView)))
//			return false;
//
//		// read view's config
//		CComPtr<IConfig> pViewCfg;
//		if (FAILED(a_pViewProfile->SubConfigGet(CComBSTR(CFGID_DESIGNERVIEW), &pViewCfg)))
//			return false;
//
//		if (FAILED(m_pViewMgr->CreateWnd(m_pViewMgr, cSelectedView, pViewCfg, this, CObserverImpl<CTabbedMainFrame, IStatusBarObserver, BYTE>::ObserverGet(), M_Doc(), reinterpret_cast<RWHWND>(m_hWnd), &a_rcView, EDVWSVolatileBorder, m_tLocaleID, &m_pDVWnd)))
//			return false;
//
//		m_pDVWnd->Handle(reinterpret_cast<RWHWND*>(&m_hWndClient));
//		::SetFocus(m_hWndClient);
//		
//		if (m_hWndClient == NULL)
//		{
//			m_pDVWnd = NULL;
//			return false;
//		}
//
//		UpdateRootMenu();
//
//		return true;
//	}
//	catch (...)
//	{
//		return false;
//	}
//}

//bool CTabbedMainFrame::DestroyView()
//{
//	//TCHAR szHelpPath[MAX_PATH] = _T("");
//	//GetHelpFilePath(szHelpPath, itemsof(szHelpPath), m_tLocaleID);
//	//m_strLayoutHelpPath = szHelpPath;
//	//m_strLayoutHelpPath += _T("::/Layouts/StartView.html>Layout");
//
//	if (m_hWndClient == NULL || m_pDVWnd == NULL)
//		return false;
//
//	m_pDVWnd->Destroy();
//	m_hWndClient = NULL;
//	m_pDVWnd = NULL;
//
//	return true;
//}

// configuration dialogs



void CTabbedMainFrame::UpdateCaption()
{
//	TCHAR szCaption[512] = _T("");
//	CConfigValue cVal;
//	m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_CAPTION), &cVal);
//	if ((cVal.operator LONG() & CFGFLAG_CAPTION_NAME) && M_Doc())
//	{
//		CDocumentName::GetDocName(M_Doc(), szCaption, itemsof(szCaption));
//		if (szCaption[0])
//		{
//			if (M_DocSave() && M_DocSave()->IsDirty() == S_OK)
//			{
//				_tcscat(szCaption, _T("*"));
//			}
//		}
//	}
//	if ((cVal.operator LONG() & CFGFLAG_CAPTION_TYPE) && M_Doc())
//	{
//		CComPtr<IDocumentType> pDocType;
//		CComPtr<ILocalizedString> pName;
//		CComBSTR bstrName;
//		if (M_Doc() && SUCCEEDED(M_Doc()->DocumentTypeGet(&pDocType)) &&
//			pDocType && SUCCEEDED(pDocType->TypeNameGet(NULL, &pName)) &&
//			pName && SUCCEEDED(pName->GetLocalized(m_tLocaleID, &bstrName)) && bstrName)
//		{
//			_tcscat(szCaption, szCaption[0] ? _T(" (") : _T("("));
//			_tcscat(szCaption, COLE2CT(bstrName));
//			_tcscat(szCaption, _T(")"));
//		}
//	}
//	if ((cVal.operator LONG() & CFGFLAG_CAPTION_LAYOUT) && M_Doc() && m_strActiveLayout)
//	{
//		LPCOLESTR p = NULL;
//		ULONG n = 0;
//		CMultiLanguageString::GetLocalized(m_strActiveLayout, m_tLocaleID, p, n);
//		if (p && n)
//		{
//			_tcscat(szCaption, szCaption[0] ? _T(" [") : _T("["));
//#ifdef _UNICODE
//			_tcsncat(szCaption, p, n);
//#else
//			std::wstring str(p, p+n);
//			_tcscat(szCaption, COLE2CT(str.c_str()));
//#endif
//			_tcscat(szCaption, _T("]"));
//		}
//	}
//	if ((cVal.operator LONG() & CFGFLAG_CAPTION_APP) || M_Doc() == NULL || szCaption[0] == _T('\0'))
//	{
//		if (szCaption[0])
//			_tcscat(szCaption, _T(" - "));
//
//		if (m_pAppName)
//		{
//			CComBSTR bstrCaption;
//			m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
//			_tcscat(szCaption, COLE2CT(bstrCaption));
//		}
//		else
//		{
//			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDR_MAINFRAME, szCaption+_tcslen(szCaption), 512-_tcslen(szCaption), LANGIDFROMLCID(m_tLocaleID));
//		}
//	}
//
//	SetWindowText(szCaption);
}

void CTabbedMainFrame::FilesFromDrop(IDataObject* a_pDataObj, CComPtr<IEnumStringsInit>& a_pFileList)
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

HRESULT CTabbedMainFrame::DragImpl(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
{
	POINT tPt = {a_pt.x, a_pt.y};
	m_pDragFeedback = NULL;
	bool bHandled = false;
	//if (m_pDVWnd)
	//{
	//	CComPtr<IEnumUnknownsInit> p;
	//	RWCoCreateInstance(p, __uuidof(EnumUnknowns));
	//	m_pDVWnd->QueryInterfaces(__uuidof(IDragAndDropHandler), EQIFVisible, p);
	//	CComPtr<IDragAndDropHandler> pDD;
	//	for (ULONG i = 0; SUCCEEDED(p->Get(i, __uuidof(IDragAndDropHandler), reinterpret_cast<void**>(&pDD))); ++i, pDD = NULL)
	//	{
	//		m_pDragFeedback = NULL; // just in case
	//		if (pDD && SUCCEEDED(pDD->Drag(m_pDragData, m_pDragFiles, a_grfKeyState, tPt, a_pdwEffect, &m_pDragFeedback)))
	//		{
	//			bHandled = true;
	//			break;
	//		}
	//	}
	//}
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

STDMETHODIMP CTabbedMainFrame::DragEnter(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

STDMETHODIMP CTabbedMainFrame::DragOver(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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

STDMETHODIMP CTabbedMainFrame::DragLeave()
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
	//if (m_pDVWnd)
	//{
	//	CComPtr<IEnumUnknownsInit> p;
	//	RWCoCreateInstance(p, __uuidof(EnumUnknowns));
	//	m_pDVWnd->QueryInterfaces(__uuidof(IDragAndDropHandler), EQIFAll, p);
	//	CComPtr<IDragAndDropHandler> pDD;
	//	for (ULONG i = 0; SUCCEEDED(p->Get(i, __uuidof(IDragAndDropHandler), reinterpret_cast<void**>(&pDD))); ++i, pDD = NULL)
	//	{
	//		CComPtr<ILocalizedString> pDragFeedback;
	//		static POINT tPt = {-1, -1};
	//		DWORD dw;
	//		pDD->Drag(NULL, NULL, 0, tPt, &dw, &pDragFeedback);
	//	}
	//}
	if (GetDropHelper())
		GetDropHelper()->DragLeave();
	return S_OK;
}

STDMETHODIMP CTabbedMainFrame::Drop(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect)
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
	//if (m_pDVWnd)
	//{
	//	CComPtr<IEnumUnknownsInit> p;
	//	RWCoCreateInstance(p, __uuidof(EnumUnknowns));
	//	m_pDVWnd->QueryInterfaces(__uuidof(IDragAndDropHandler), EQIFVisible, p);
	//	CComPtr<IDragAndDropHandler> pDD;
	//	for (ULONG i = 0; SUCCEEDED(p->Get(i, __uuidof(IDragAndDropHandler), reinterpret_cast<void**>(&pDD))); ++i, pDD = NULL)
	//	{
	//		if (pDD && pDD->Drop(a_pDataObj, pFiles, a_grfKeyState, tPt) == S_OK)
	//		{
	//			if (a_pdwEffect) *a_pdwEffect = (DROPEFFECT_COPY&*a_pdwEffect) ? DROPEFFECT_COPY : ((DROPEFFECT_MOVE&*a_pdwEffect) ? DROPEFFECT_MOVE : DROPEFFECT_LINK);
	//			return S_OK;
	//		}
	//	}
	//}
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
		//if (M_Doc())
		//{
		//	pDesignerCore->NewWindowPath(bstr, NULL);
		//}
		//else
		//{
		//	CComPtr<IInputManager> pInMgr;
		//	RWCoCreateInstance(pInMgr, __uuidof(InputManager));
		//	CComPtr<IDocument> pDoc;
		//	pInMgr->DocumentCreate(CStorageFilter(bstr), NULL, &pDoc);
		//	SetNewDocument(pDoc);
		//}
		if (a_pdwEffect) *a_pdwEffect = (DROPEFFECT_COPY&*a_pdwEffect) ? DROPEFFECT_COPY : ((DROPEFFECT_MOVE&*a_pdwEffect) ? DROPEFFECT_MOVE : DROPEFFECT_LINK);
		return S_OK;
	}
	if (a_pdwEffect) *a_pdwEffect = DROPEFFECT_NONE;
	return S_OK;
}

//LRESULT CTabbedMainFrame::OnToolsOptions(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
//{
//	static bool s_bInit = false;
//	if (!s_bInit)
//	{
//		INITCOMMONCONTROLSEX tICCE = {sizeof(tICCE), ICC_USEREX_CLASSES};
//		InitCommonControlsEx(&tICCE);
//		s_bInit = true;
//	}
//
//	CGlobalOptionsDlg cDlg(m_tLocaleID, GetIcon(FALSE));
//	if (cDlg.DoModal())
//	{
//		// TODO: consider observing config changes
//		UpdateUndoMode();
//		UpdateCaption();
//		CConfigValue cLCID;
//		{
//			CComPtr<IGlobalConfigManager> pGlobalConfigMgr;
//			RWCoCreateInstance(pGlobalConfigMgr, __uuidof(GlobalConfigManager));
//			CComPtr<IConfig> pFrameCfg;
//			pGlobalConfigMgr->Config(CLSID_GlobalConfigMainFrame, &pFrameCfg);
//			pFrameCfg->ItemValueGet(CComBSTR(CFGID_LANGUAGECODE), &cLCID);
//		}
//		if (cLCID.TypeGet() == ECVTInteger && cLCID.operator LONG() != m_tLocaleID)
//		{
//			m_tLocaleID = cLCID.operator LONG();
//			m_pThread->SetLocaleID(cLCID.operator LONG());
//			RECT rcClient;
//			::GetWindowRect(m_hWndClient, &rcClient);
//			ScreenToClient(&rcClient);
//			if (m_pDVWnd)
//				m_pDVWnd->OnDeactivate(FALSE);
//			DestroyView();
//			if (m_pCurrentLayoutCfg)
//			{
//				CreateView(m_pCurrentLayoutCfg, rcClient);
//			}
//			else
//			{
//				CComObject<CDesignerViewWndStart>* pViewWnd = NULL;
//				CComObject<CDesignerViewWndStart>::CreateInstance(&pViewWnd);
//				CComPtr<IDesignerView> pTmp = pViewWnd;
//
//				RECT rc;
//				GetClientRect(&rc);
//				CConfigValue cStartPage;
//				{
//					CConfigLock cLock(m_pThread);
//					m_pThread->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_STARTPAGE), &cStartPage);
//					pViewWnd->Init(m_pThread->M_Config(), this, CObserverImpl<CTabbedMainFrame, IStatusBarObserver, BYTE>::ObserverGet(), m_hWnd, &rc, m_tLocaleID, m_pThread->M_StartPageCount(), m_pThread->M_StartPages(), cStartPage, m_tStartPage, m_pThread->M_StorageManager(), m_pAppName);
//					m_tStartPage = GUID_NULL;
//				}
//
//				m_wndMenuBar.SetCloseButtonID(0);
//				UpdateRootMenu();
//
//				m_pDVWnd.Attach(pTmp.Detach());
//
//				m_pDVWnd->Handle(reinterpret_cast<RWHWND*>(&m_hWndClient));
//				::SetFocus(m_hWndClient);
//			}
//			UpdateLayout();
//			UpdateCaption();
//		}
//	}
//
//	return 0;
//}

int CTabbedMainFrame::AppMessageBox(LPCTSTR a_pszText, UINT a_nFlags) const
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

//void CTabbedMainFrame::UpdateLayout(BOOL bResizeBars)
//{
//	Win32LangEx::CLangFrameWindowImpl<CTabbedMainFrame>::UpdateLayout(bResizeBars);
//	if (m_wndStatusBar.m_hWnd && bResizeBars)
//	{
//		// get pane positions
//		int* pPanesPos = NULL;
//		int nPanes = m_wndStatusBar.GetParts(0, NULL);
//		ATLTRY(pPanesPos = (int*)_alloca(nPanes * sizeof(int)));
//		if(pPanesPos == NULL)
//			return;
//		m_wndStatusBar.GetParts(nPanes, pPanesPos);
//		// calculate offset
//		RECT rcClient;
//		m_wndStatusBar.GetClientRect(&rcClient);
//		int cxOff = rcClient.right - (pPanesPos[nPanes - 1] + ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE));
//		for(int i = 0; i < nPanes; i++)
//			pPanesPos[i] += cxOff;
//		// set pane postions
//		m_wndStatusBar.SetParts(nPanes, pPanesPos);
//	}
//}

LRESULT CTabbedMainFrame::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	//m_CmdBar.SendMessage(a_uMsg, a_wParam, a_lParam);
	//m_wndMenuBar.SendMessage(a_uMsg, a_wParam, a_lParam);
	//UpdateLayout();
	//if (m_hWndClient)
	//	::SendMessage(m_hWndClient, a_uMsg, a_wParam, a_lParam);
	return 0;
}

LRESULT CTabbedMainFrame::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		HH_POPUP hhp;
		hhp.cbStruct = sizeof(hhp);
		RECT rcItem;
		UINT uID = IDS_CONTEXTHELP_NONE;
		//if (pHelpInfo->hItemHandle == m_wndMenuBar || pHelpInfo->hItemHandle == m_CmdBar)
		//{
		//	uID = IDS_CONTEXTHELP_MENUBAR;
		//	m_wndMenuBar.GetWindowRect(&rcItem);
		//	hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		//	hhp.pt.y = rcItem.bottom;
		//}
		//else if (pHelpInfo->hItemHandle == m_wndStatusBar)
		//{
		//	uID = IDS_CONTEXTHELP_STATUSBAR;
		//	m_wndStatusBar.GetWindowRect(&rcItem);
		//	hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		//	hhp.pt.y = rcItem.top-100;
		//}
		//else
		//{
		//	::GetWindowRect((HWND)pHelpInfo->hItemHandle, &rcItem);
		//	hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		//	hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
		//}
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

//void CTabbedMainFrame::HandleOperationResult(HRESULT a_hRes)
//{
//	if (FAILED(a_hRes) && a_hRes != E_RW_CANCELLEDBYUSER)
//	{
//		CComBSTR bstr;
//		if (m_pErrorMessage)
//			m_pErrorMessage->GetLocalized(m_tLocaleID, &bstr);
//		CComBSTR bstrCaption;
//		m_pAppName->GetLocalized(m_tLocaleID, &bstrCaption);
//		if (bstrCaption == NULL) bstrCaption = L"Error";
//		if (bstr != NULL && bstr[0])
//		{
//			::MessageBox(m_hWnd, CW2T(bstr), CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
//		}
//		else
//		{
//			TCHAR szTemplate[256] = _T("");
//			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_GENERICERROR, szTemplate, itemsof(szTemplate), LANGIDFROMLCID(m_tLocaleID));
//			TCHAR szMsg[256];
//			_stprintf(szMsg, szTemplate, a_hRes);
//			::MessageBox(m_hWnd, szMsg, CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
//		}
//	}
//}
*/