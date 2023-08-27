// DesignerViewToolbar.h : Declaration of the CDesignerViewToolbar

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <Win32LangEx.h>
#include <XPGUI.h>
#include "ConfigIDsToolbar.h"
#include <ContextMenuWithIcons.h>
#include <ObserverImpl.h>
#include <atltheme.h>
#include <RWProcessing.h>


// CDesignerViewToolbar

class ATL_NO_VTABLE CDesignerViewToolbar :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewToolbar>,
	public CThemeImpl<CDesignerViewToolbar>,
	public CChildWindowImpl<CDesignerViewToolbar, IDesignerView>,
	public CContextMenuWithIcons<CDesignerViewToolbar>,
	public CObserverImpl<CDesignerViewToolbar, ISharedStateObserver, TSharedStateChange>,
	public IDesignerViewStatusBar,
	public IDragAndDropHandler
{
public:
	CDesignerViewToolbar() : m_bShowCommandDesc(false), m_bShowButtonDesc(false), m_nButtonSize(0), m_nIconDelta(0)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CDesignerViewToolbar()
	{
		m_cImageList.Destroy();
	}

	DECLARE_WND_CLASS_EX(_T("ToolbarFrameWndClass"), 0, COLOR_WINDOW);

	void Init(IViewManager* a_pManager, IDocument* a_pDocument, ISharedStateManager* a_pSharing, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		m_pStateMgr = a_pSharing;
		CComObject<COperationContextFromStateManager>::CreateInstance(&m_pContext.p);
		m_pContext.p->AddRef();
		m_pContext->Init(a_pSharing);
		m_pStatusBar = a_pStatusBar;
		m_tLocaleID = a_tLocaleID;
		m_pConfig = a_pConfig;
		m_pViewManager = a_pManager;
		a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&m_pMenuCmds));
		if (m_pMenuCmds == NULL)
		{
			RWCoCreateInstance(m_pMenuCmds, __uuidof(MenuCommandsManager));
		}
		m_pDocument = a_pDocument;

		// create self
		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("ToolbarFrame"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT : WS_EX_CONTROLPARENT) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
		m_pStateMgr->ObserverIns(ObserverGet(), 0);
	}

	enum { ID_FIRST_BUTTON = 300 };

	BEGIN_COM_MAP(CDesignerViewToolbar)
		COM_INTERFACE_ENTRY(IDesignerView)
		COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
		COM_INTERFACE_ENTRY(IDragAndDropHandler)
	END_COM_MAP()

	BEGIN_MSG_MAP(CDesignerViewToolbar)
		CHAIN_MSG_MAP(CThemeImpl<CDesignerViewToolbar>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		COMMAND_RANGE_HANDLER(ID_FIRST_BUTTON, ID_FIRST_BUTTON+1000, OnButtonClicked)
		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnButtonDropDown)
		NOTIFY_CODE_HANDLER(TBN_HOTITEMCHANGE, OnHotItemChange)
		CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewToolbar>)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		NOTIFY_CODE_HANDLER(RBN_CHEVRONPUSHED, OnChevronPushed)
		NOTIFY_CODE_HANDLER(RBN_AUTOSIZE, OnReBarAutoSize)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnRightClick)
		//CHAIN_MSG_MAP(CFrameWindowImpl<CDesignerViewToolbar>)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_RW_GOTFOCUS, OnRWGotFocus)
		MESSAGE_HANDLER(WM_RW_DEACTIVATE, OnRWForward)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnRWForward)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnRWForward)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		//NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
	END_MSG_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	void OwnerNotify(TCookie, TSharedStateChange a_tState)
	{
		try
		{
			if (!m_wndReBar.IsWindow())
				return;

			CComQIPtr<ISharedStateToolbar> pTBState(a_tState.pState);
			if (pTBState == NULL)
				return;
			BOOL bVisible = pTBState->IsVisible() != S_FALSE;
			if (bVisible)
				m_bShowButtonDesc = false; // maybe too pesimistic

			if (m_pStateMgr && a_tState.bstrName != NULL && a_tState.bstrName[0])
				for (CToolbars::iterator i = m_cToolbars.begin(); i != m_cToolbars.end(); ++i)
				{
					if (i->bstrID == a_tState.bstrName)
					{
						REBARBANDINFO tRBI;
						ZeroMemory(&tRBI, sizeof(tRBI));
						tRBI.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
						tRBI.fMask = RBBIM_STYLE;
						m_wndReBar.GetBandInfo(i-m_cToolbars.begin(), &tRBI);
						if (((tRBI.fStyle & RBBS_HIDDEN) && !i->bVisible && bVisible) ||
							(!(tRBI.fStyle & RBBS_HIDDEN) && !bVisible))
						{
							tRBI.fStyle ^= RBBS_HIDDEN;
							m_wndReBar.SetBandInfo(i-m_cToolbars.begin(), &tRBI);
							UpdateLayout();
						}
						i->bVisible = bVisible;
						break;
					}
				}
		}
		catch (...)
		{
		}
	}

	void UpdateLayout(BOOL bResizeBars = TRUE);


	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);
	bool ExecuteAccelerator(IEnumUnknowns* a_pCommands, WORD a_wKeyCode, WORD a_fVirtFlags);

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)();
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges);
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces);
	STDMETHOD(OptimumSize)(SIZE* a_pSize);
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{
		GetParent().SendMessage(WM_RW_DEACTIVATE, a_bCancelChanges, 0);
		return S_OK;
	}

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		if (m_bShowCommandDesc)
			a_pStatusBar->SimpleModeSet(m_bstrCommandDesc);
		else if (m_bShowButtonDesc)
			a_pStatusBar->SimpleModeSet(m_bstrButtonDesc);
		return S_OK;
	}

	// IDragAndDropHandler methods
public:
	HRESULT CheckDropPoint(int a_nX, int a_nY, CComPtr<IConfig>& a_pCfg);

	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt);

private:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnButtonClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnButtonDropDown(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnHotItemChange(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnMenuSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnRightClick(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled);
	LRESULT OnChevronPushed(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled);
	//LRESULT OnCustomDraw(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam != SIZE_MINIMIZED)
		{
			UpdateLayout();
		}
		bHandled = FALSE;
		return 1;
	}
	LRESULT OnReBarAutoSize(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	{
		UpdateLayout(FALSE);
		return 0;
	}
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (m_pView != NULL)   // view will paint itself instead
			return 1;

		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		if (m_pView != NULL)
		{
			HWND h = NULL;
			m_pView->Handle(&h);
			if (h)
				::SetFocus(h);
		}

		bHandled = FALSE;
		return 1;
	}

private:
	struct SToolbar
	{
		CComBSTR bstrID;
		CToolBarCtrl wndToolBar;
		CConfigValue cCmdsID;
		CComPtr<IConfig> pCmdsCfg;
		CComPtr<IEnumUnknowns> pCommands;
		CComPtr<IChildWindow> pCustomWindow;
		bool bVisible;
	};
	typedef std::vector<SToolbar> CToolbars;
	struct less_guid { bool operator()(GUID const& a_1, GUID const& a_2) const {return memcmp(&a_1, &a_2, sizeof(GUID)) < 0;} };
	typedef std::map<GUID, int, less_guid> CIconMap;

	typedef std::vector<CComPtr<IDocumentMenuCommand> > CCommands;

	class ATL_NO_VTABLE COperationContextFromStateManager :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationContext
	{
	public:
		void Init(ISharedStateManager* a_pOrig)
		{
			m_pOrig = a_pOrig;
		}
		ILocalizedString* M_ErrorMessage()
		{
			return m_pMessage;
		}
		void ResetErrorMessage()
		{
			m_pMessage = NULL;
		}


	BEGIN_COM_MAP(COperationContextFromStateManager)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// ISharedStateManager methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			return m_pOrig->StateGet(a_bstrCategoryName, a_iid, a_ppState);
		}
		STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
		{
			return m_pOrig->StateSet(a_bstrCategoryName, a_pState);
		}
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
			m_pMessage = a_pMessage;
			return S_OK;
		}

	private:
		CComPtr<ISharedStateManager> m_pOrig;
		CComPtr<ILocalizedString> m_pMessage;
	};

private:
	void UpdateToolbar(CToolBarCtrl& a_wndToolbar, CComPtr<IChildWindow>& a_pCustomWindow, IEnumUnknowns* a_pCmds, int a_nFirstID, int a_nIDStep);
	void InsertMenuItems(IEnumUnknowns* a_pCmds, CCommands& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID);
	void HandleOperationResult(HRESULT a_hRes);
	HICON AdjustIcon(HICON orig) const;

private:
	CComPtr<ISharedStateManager> m_pStateMgr;
	CComPtr<CComObject<COperationContextFromStateManager> > m_pContext;
	CComPtr<IStatusBarObserver> m_pStatusBar;
	CComPtr<IMenuCommandsManager> m_pMenuCmds;
	CComPtr<IViewManager> m_pViewManager;
	CComPtr<IDocument> m_pDocument;
	CToolbars m_cToolbars;
	CComPtr<IConfig> m_pConfig;
	CComPtr<IDesignerView> m_pView;
	CReBarCtrl m_wndReBar;
	CImageList m_cImageList;
	CIconMap m_cIconMap;
	CCommands m_cCommands;
	bool m_bShowCommandDesc;
	CComBSTR m_bstrCommandDesc;
	bool m_bShowButtonDesc;
	CComBSTR m_bstrButtonDesc;
	LCID m_tLocaleID;
	int m_nButtonSize;
	int m_nIconDelta;
};

