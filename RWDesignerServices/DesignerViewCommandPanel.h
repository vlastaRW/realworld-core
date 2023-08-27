
#pragma once


class ATL_NO_VTABLE CDesignerViewCommandPanel : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewCommandPanel>,
	public CThemeImpl<CDesignerViewCommandPanel>,
	public CDesignerViewWndImpl<CDesignerViewCommandPanel, IDesignerView>
{
public:
	CDesignerViewCommandPanel()
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}

	DECLARE_WND_CLASS_EX(_T("CommandPanelClass"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	void Init(IViewManager* a_pManager, IDocument* a_pDocument, ISharedStateManager* a_pSharing, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		m_pStates = a_pSharing;
		//m_pStatusBar = a_pStatusBar;
		m_tLocaleID = a_tLocaleID;
		m_pConfig = a_pConfig;
		a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&m_pManager));
		if (m_pManager == NULL)
		{
			RWCoCreateInstance(m_pManager, __uuidof(MenuCommandsManager));
		}
		m_pDocument = a_pDocument;

		// create self
		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("CommandPanel"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE : 0) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
	}

BEGIN_COM_MAP(CDesignerViewCommandPanel)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewCommandPanel)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewCommandPanel>)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
//	NOTIFY_HANDLER(ID_COLORAREAHS, CColorAreaHS::CAN_COLOR_CHANGED, OnColorAreaHSChanged)
//	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// message handlers
protected:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
//	LRESULT OnColorAreaLAChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
//	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			return E_NOTIMPL;
		}
		catch (...)
		{
			return a_pSize ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(OnIdle)()
	{
		try
		{
			CComBSTR bstr(CFGID_CMDPANEL_COMMANDS);
			CConfigValue cID;
			m_pConfig->ItemValueGet(bstr, &cID);
			CComPtr<IConfig> pCfg;
			m_pConfig->SubConfigGet(bstr, &pCfg);
			CComPtr<IEnumUnknowns> pCmds;
			CComObject<COperationContextFromStateManager>* p = NULL;
			CComObject<COperationContextFromStateManager>::CreateInstance(&p);
			CComPtr<IOperationContext> pCtx = p;
			p->Init(m_pStates);
			m_pManager->CommandsEnum(m_pManager, cID, pCfg, pCtx, this, m_pDocument, &pCmds);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
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
	CComPtr<IConfig> m_pConfig;
	CComPtr<IMenuCommandsManager> m_pManager;
	CComPtr<ISharedStateManager> m_pStates;
	CComPtr<IDocument> m_pDocument;
	LCID m_tLocaleID;
	CFont m_cFont;
	int m_nRowHeight;
	int m_nMargins;
};

