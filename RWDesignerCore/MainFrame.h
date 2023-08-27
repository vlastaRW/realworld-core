// MainFrame.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"
#include "RWDesignerCore.h"
#include "RWProcessing.h"
#include <ObserverImpl.h>
#include <Win32LangEx.h>
#include "ThreadCommand.h" // TODO: remove
#include "DesignerViewWndStart.h" // TODO: remove
#include "ClipboardViewer.h"
#include "DesignerFrameOperationManager.h"
#include "DesignerFrameMenuCommandsManager.h"
#include "DesignerFrameViewManager.h"
#include "DesignerViewStatusBar.h"
#include "MenuContainer.h"
#include "RecentFiles.h"
#include <ContextMenuWithIcons.h>


class CWindowThread;

class ATL_NO_VTABLE CMainFrame :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangFrameWindowImpl<CMainFrame>,
	public CContextMenuWithIcons<CMainFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public CClipboardViewer<CMainFrame>,
	public ISharedStateManager,
	public IOperationContext,
	public CObserverImpl<CMainFrame, IDocumentObserver, ULONG>,
	public CObserverImpl<CMainFrame, IStatusBarObserver, BYTE>,
	public CObserverImpl<CMainFrame, IConfigObserver, IUnknown*>,
	public IDocumentControl,
	public IDesignerViewStatusBar,
	public IDropTarget,
	public IMenuTextProvider
{
public:
	CMainFrame();
	~CMainFrame();
	HRESULT Init(CThreadCommand* a_pOrder, CWindowThread* a_pThread);
	IDocument* M_Doc() const {return m_pDoc;}
	IDocumentUndo* M_DocUndo() const {return m_pDocUndo;}
	CComBSTR const& M_ActiveLayout() const {return m_strActiveLayout;}
	bool IsStatusBarVisible() const {return m_bStatusBarVisible;}
	void ShowStatusBar(bool a_bShow);
	void SwitchLayout(LPCOLESTR a_pszLayoutName, bool a_bChangeOrder = false);

	BEGIN_COM_MAP(CMainFrame)
		COM_INTERFACE_ENTRY(ISharedStateManager)
		COM_INTERFACE_ENTRY(IOperationContext)
		COM_INTERFACE_ENTRY(IDropTarget)
	END_COM_MAP()

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	enum
	{
		ID_CLOSEDOCUMENT = 27147,
		ID_LAYOUT_MENU1 = 27148,
		WM_SENDSTATECHANGES = WM_APP+438,
		WM_RW_LANGUAGECHANGE = WM_APP+348,
	};

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_RANGE_HANDLER(ID_LAYOUT_MENU1, (ID_LAYOUT_MENU1+2049), OnLayoutOperation) // TODO: remove the 1000 limit (and constant)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		CHAIN_MSG_MAP(Win32LangEx::CLangFrameWindowImpl<CMainFrame>)
		CHAIN_MSG_MAP(CClipboardViewer<CMainFrame>)
		NOTIFY_HANDLER(ATL_IDW_STATUS_BAR, NM_CLICK, OnStatusBarClick<ESBCTLeftSingle>)
		NOTIFY_HANDLER(ATL_IDW_STATUS_BAR, NM_DBLCLK, OnStatusBarClick<ESBCTLeftDouble>)
		NOTIFY_HANDLER(ATL_IDW_STATUS_BAR, NM_RCLICK, OnStatusBarClick<ESBCTRightSingle>)
		NOTIFY_HANDLER(ATL_IDW_STATUS_BAR, NM_RDBLCLK, OnStatusBarClick<ESBCTRightDouble>)
		NOTIFY_HANDLER(ATL_IDW_COMMAND_BAR+1, CMenuContainer::MCN_LAYOUT_SWITCHED, OnLayoutChanged)
		COMMAND_HANDLER(ID_CLOSEDOCUMENT, BN_CLICKED, OnFileClose)
		CHAIN_MSG_MAP(CContextMenuWithIcons<CMainFrame>)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
		MESSAGE_HANDLER(WM_SENDSTATECHANGES, OnSendStateChanges)
		MESSAGE_HANDLER(WM_RW_DEACTIVATE, OnDeactivateView)
		MESSAGE_HANDLER(WM_RW_LANGUAGECHANGE, OnLanguageChange)
	END_MSG_MAP()

	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnInitMenuPopup(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnFileExit(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnFileClose(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnViewStatusBar(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnProfileConfigureCurrent(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnProfileManage(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnLayoutOperation(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled, bool a_bAtMousePos = true);
	LRESULT OnFileSave(bool a_bForceInteractive);
	LRESULT SaveFile(IDocument* a_pDoc, IConfig* a_pSaveCfg, LCID a_tLocaleID, IStorageFilter* a_pDst = NULL);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnFileMRU(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnDropFiles(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnToolsOptions(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	template<EStatusBarClickType t_eClickType>
	LRESULT OnStatusBarClick(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
	{
		NMMOUSE* p = reinterpret_cast<NMMOUSE*>(a_pNMHdr);
		size_t nToolIndex = p->dwItemSpec-1;
		if (nToolIndex < m_cStatusBarItems.size() && m_cStatusBarItems[nToolIndex].nPane >= 0)
		{
			RECT rc;
			m_wndStatusBar.GetRect(p->dwItemSpec, &rc);
			m_wndStatusBar.ClientToScreen(&rc);
			POINT pt = p->pt;
			m_wndStatusBar.ClientToScreen(&pt);
			m_cStatusBarTools[m_cStatusBarItems[nToolIndex].nPane].first->OnClick(m_hWnd, m_tLocaleID, &rc, pt, t_eClickType);
		}
		return TRUE;
	}
	LRESULT OnLayoutChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnMenuSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSendStateChanges(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDeactivateView(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pDVWnd)
			m_pDVWnd->OnDeactivate(a_wParam);
		return 0;
	}

	void UpdateLayout(BOOL bResizeBars = TRUE);

private:
	enum
	{
		ETBToolBar = ATL_IDW_BAND_FIRST + 1,
		ETBMenu = ATL_IDW_BAND_FIRST + 2,
		ETBOperationsBar = ATL_IDW_BAND_FIRST + 3
	};

public:
	static HRESULT InitConfig(IConfigWithDependencies* a_pMainCfg, IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager, IStorageManager* a_pStorageManager);
	static IConfig* CreateLayoutConfig(IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager);
	static ISubConfig* CreateLayoutsConfig(IViewManager* a_pViewManager, IOperationManager* a_pOperationManager, IMenuCommandsManager* a_pMenuCommandsManager, IDesignerFrameIcons* a_pIconsManager);

	CWindowThread* GetThread() const { return m_pThread; }

	// ISharedStateManager methods
public:
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState);
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState);
	STDMETHOD(ObserverIns)(ISharedStateObserver* a_pObserver, TCookie a_tCookie);
	STDMETHOD(ObserverDel)(ISharedStateObserver* a_pObserver, TCookie a_tCookie);

	// IOperationContext methods
public:
	STDMETHOD(IsCancelled)() { return S_FALSE; }
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		if (a_pItemIndex) *a_pItemIndex = 0;
		if (a_pItemsRemaining) *a_pItemsRemaining = 0;
		if (a_pStepIndex) *a_pStepIndex = 0;
		if (a_pStepsRemaining) *a_pStepsRemaining = 0;
		return S_OK;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		m_pErrorMessage = a_pMessage;
		return S_OK;
	}

	// IDropTarget methods
public:
	STDMETHOD(DragEnter)(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragOver)(DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);

	HRESULT DragImpl(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect);

	// IDocumentControl methods
public:
	void SetNewDocument(IDocument* a_pNewDoc);

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar);

	// internal observer
public:
	void OwnerNotify(TCookie, ULONG a_nChangeFlags);
	void OwnerNotify(TCookie, BYTE dummy);
	void OwnerNotify(TCookie, IUnknown*)
	{
		if (!m_bLangChangePosted)
		{
			m_bLangChangePosted = true;
			PostMessage(WM_RW_LANGUAGECHANGE);
		}

	}
	LRESULT OnLanguageChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	LPCTSTR GetMenuItemText(UINT a_nID)
	{
		return CContextMenuWithIcons<CMainFrame>::GetMenuItemText(a_nID);
	}

private:
	template<class T>
	struct lessBinary
	{
		bool operator()(const T& a_1, const T& a_2) const
		{
			return memcmp(&a_1, &a_2, sizeof(T)) < 0;
		}
	};

	typedef map<GUID, int, lessBinary<GUID> > CFrameIcons;

	typedef vector<pair<CComPtr<IDesignerFrameTools>, ULONG> > CExternTools;
	typedef vector<pair<CComPtr<IStatusBarPane>, ULONG> > CStatusBarTools;
	typedef std::map<UINT, CComPtr<IDocumentMenuCommand> > CMenuCommands;
	typedef std::map<HMENU, CComPtr<IEnumUnknowns> > CSubMenus;
	typedef std::map<HMENU, ULONG> CTopMenus;
	typedef std::queue<UINT> CMenuIDs;
	typedef std::vector<std::pair<ISharedStateObserver*, TCookie> > CObservers;
	typedef std::set<CComBSTR> CStateIDs;

private:
	CComPtr<IEnumUnknownsInit> GetBuilders();
	bool CreateView(IConfig* a_pViewProfile, const RECT& a_rcView);
	bool DestroyView();
	static IConfig* FindBestLayout(IDocument* a_pDoc, IViewManager* a_pMgr, IConfig* a_pMainConfig, CComBSTR& a_strName, std::vector<std::pair<CComBSTR, CComBSTR> >& a_cCompatible);
	void UpdateCaption();
	void InitToolBarsAndMenu(int dpi);
	void UpdateRootMenu();
	bool RefreshDocument();
	bool AskAndCloseDocument();
	void UpdateUndoMode();
	int AppMessageBox(LPCTSTR a_pszText, UINT a_nFlags) const;
	void SaveLastLayout() const;
	void UpdateStatusBar();
	void UpdateMenu(CMenuHandle& a_cMenu, IEnumUnknowns* a_pCmds);
	UINT AllocateMenuID();
	void ReleaseMenuID(UINT a_nID);
	bool ExecuteAccelerator(IEnumUnknowns* a_pCommands, WORD a_wKeyCode, WORD a_fVirtFlags);
	void GetMainMenu(IEnumUnknowns** a_pCmds);
	void HandleOperationResult(HRESULT a_hRes);
	static void FilesFromDrop(IDataObject* a_pDataObj, CComPtr<IEnumStringsInit>& a_pFileList);
	IDropTargetHelper* GetDropHelper()
	{
		if (!m_bDragHelperCreated)
		{
			m_bDragHelperCreated = true;
			m_pDragHelper.CoCreateInstance(CLSID_DragDropHelper);
		}
		return m_pDragHelper;
	}

private:
	CStatusBarCtrl m_wndStatusBar;
	CStatusBarTools m_cStatusBarTools;
	CComPtr<CComObject<CDesignerViewStatusBar> > m_pDesignerViewStatusBar;
	CStatusBarItems m_cStatusBarItems;
	size_t m_nStatusBarTools;
	HICON m_hIconSmall;
	HICON m_hIconLarge;
	CComPtr<ILocalizedString> m_pAppName;

	// shared data
	CWindowThread* m_pThread;

	// document / start view options
	CComPtr<IInputManager> m_pIM;
	CLSID m_tStartPage;
	CComPtr<IDocument> m_pDoc;
	CComQIPtr<IDocumentUndo> m_pDocUndo;
	CViewStates m_cViewStates;
	CStateIDs m_cChangedStateIDs;
	CObservers m_aObservers;

	// extern tools
	CExternTools m_cExternTools;

	// configuration
	CComPtr<IConfig> m_pCurrentLayoutCfg;
	CComBSTR m_strActiveLayout;
	CComPtr<IConfig> m_pLanguageCfg;
	bool m_bLangChangePosted;

	// cached window related items
	CMenuContainer m_wndMenuBar;
	CCommandBarCtrl m_CmdBar;
	bool m_bStatusBarVisible;
	CComPtr<IDesignerView> m_pDVWnd;
	DWORD m_dwLastOnIdle;

	CFrameIcons m_cFrameIcons;
	CComObject<CDesignerFrameOperationManager>* m_pOperMgr;
	CComObject<CDesignerFrameMenuCommandsManager>* m_pCmdsMgr;
	CComObject<CDesignerFrameViewManager>* m_pViewMgr;

	// main menu
	CMenuCommands m_cMenuCommands;
	CSubMenus m_cSubMenus;
	CTopMenus m_cTopMenus;
	CMenuIDs m_cFreeIDs;
	UINT m_uFreeID;
	bool m_bShowCommandDesc;
	CComBSTR m_bstrCommandDesc;
	bool m_bChangeCommandDesc;
	CImageList m_cMenuImages;
	CFrameIcons m_cIconMap;
	CComPtr<IConfigWithDependencies> m_pDefaultMenuCfg;
	CComPtr<ILocalizedString> m_pErrorMessage;

	// drag and drop
	CComPtr<IDataObject> m_pDragData;
	CComPtr<IEnumStringsInit> m_pDragFiles;
	CComPtr<ILocalizedString> m_pDragFeedback;
	CToolTipCtrl m_wndTips;
	TTTOOLINFO m_tToolTip;
	std::tstring m_strLastTip;
	CComPtr<IDropTargetHelper> m_pDragHelper;
	bool m_bDragHelperCreated;
};
