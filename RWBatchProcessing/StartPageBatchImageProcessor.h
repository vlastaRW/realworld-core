// StartPageBatchImageProcessor.h : Declaration of the CStartPageFactoryBatchImageProcessor

#pragma once
#include "RWBatchProcessing.h"
#include <Win32LangEx.h>
#include <ObserverImpl.h>
#include <ContextMenuWithIcons.h>
#include <StartViewWnd.h>
#include "BatchOperationManager.h"
#include <atlgdix.h>
#include <CustomTabCtrl.h>
#include <DotNetTabCtrl.h>
#include <IconRenderer.h>


#ifndef WM_GETISHELLBROWSER
    #define WM_GETISHELLBROWSER (WM_USER+7)
#endif

static const OLECHAR CFGID_ROOT[] = L"BatchOpsPhoto";
static const OLECHAR CFGID_CURRENT[] = L"CurrentOp";
static const OLECHAR CFGID_CURRENTSUB[] = L"SubOp";
static const OLECHAR CFGID_OPERATIONS[] = L"Operations";
static const OLECHAR CFGID_ICONID[] = L"IconID";
static const OLECHAR CFGID_NAME[] = L"Name";
static const OLECHAR CFGID_DESCRIPTION[] = L"Description";
static const OLECHAR CFGID_PREVNAME[] = L"PrevName";
static const OLECHAR CFGID_OPERATION[] = L"Operation";
static const OLECHAR CFGID_FACTORY[] = L"Builder";
static const OLECHAR CFGID_CUSTOMFILTER[] = L"Filter";
static const OLECHAR CFGID_OUTPUT[] = L"Output";
static const OLECHAR CFGID_STG_DROPLET[] = L"StgDroplet";
static const OLECHAR CFGID_STG_BROWSER[] = L"StgBrowser";
static const OLECHAR CFGID_SPLITTER[] = L"Splitter";


// CStartPageBatchImageProcessor

class ATL_NO_VTABLE CStartPageBatchImageProcessor : 
	public CStartViewPageImpl<CStartPageBatchImageProcessor>,
	public Win32LangEx::CLangIndirectDialogImpl<CStartPageBatchImageProcessor>,
	public CThemeImpl<CStartPageBatchImageProcessor>,
	public CContextMenuWithIcons<CStartPageBatchImageProcessor>,
	public CObserverImpl<CStartPageBatchImageProcessor, IConfigObserver, IUnknown*>,
	public IDragAndDropHandler,
	public IShellBrowser,
	public ICommDlgBrowser,
	public IStorageFilterWindowCallback
{
public:
	CStartPageBatchImageProcessor() : m_hWorkThread(NULL), m_tOpBuilder(GUID_NULL),
		m_hWakeWorker(CreateEvent(NULL, FALSE, FALSE, NULL)), m_bStopWorker(false),
		m_bRunning(false), m_bHidden(true), m_bRequestPosted(false),
		m_bEnableUpdates(true), m_splitPos(0.5f), m_hVertical(NULL), m_jobId(0),
		m_activeOpId(GUID_NULL), m_activeIconId(GUID_NULL), m_activeBuilder(GUID_NULL)
	{
		m_tViewSettings.ViewMode = FVM_ICON;
		m_tViewSettings.fFlags = FWF_SHOWSELALWAYS|FWF_NOWEBVIEW|FWF_NOCLIENTEDGE;
		SetThemeClassList(L"ComboBox");
	}
	~CStartPageBatchImageProcessor()
	{
		m_cToolbarImages.Destroy();
		m_cLargeToolbarImages.Destroy();
		m_cOperationImages.Destroy();
		m_cLargeOperationImages.Destroy();
		if (m_hWorkThread)
		{
			m_bStopWorker = true;
			SetEvent(m_hWakeWorker);
			AtlWaitWithMessageLoop(m_hWorkThread);
			CloseHandle(m_hWorkThread);
		}
		if (m_hWakeWorker)
			CloseHandle(m_hWakeWorker);
	}
	void WindowCreate(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig, IOperationManager* a_pOpMgr);
	virtual void OnFinalMessage(HWND a_hWnd);

	static HRESULT Init1OpConfig(IOperationManager* a_pOpMgr, IConfig** a_pOut);
	static HRESULT InitConfig(IConfigWithDependencies* a_pConfig, IOperationManager* a_pOpMgr);

	enum
	{
		WM_UPDATESTATUS = WM_APP+4528,
		WM_ADDLOGMESSAGE,
		WM_CONFIGCHANGED,
		ID_BP_SELECT = 400,
		ID_BP_START,
		ID_BP_STOP,
		ID_BP_OPTIONS,
		ID_BP_HELP,
		IDC_BP_STARTSTOP = 200,
		IDC_BP_LOG,
		IDC_BP_OUTPUT,
		IDC_BP_HEADER1,
		IDC_BP_HEADER2,
		IDC_BP_HEADER3,
		IDC_BP_BROWSE,
		IDC_BP_PROGRESS,
		IDC_BP_LINE1,
		IDC_BP_LINE2,
		IDC_BP_LINE3,
		IDC_BP_LINE4,
		IDC_BP_LINE5,
	};

	BEGIN_DIALOG_EX(0, 0, 360, 275, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_BP_STARTSTOP, TOOLBARCLASSNAME, TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT | CCS_NODIVIDER | CCS_NOMOVEY | CCS_NOPARENTALIGN | CCS_NORESIZE | WS_VISIBLE, 7, 7, 346, 30, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_PROGRESS, PROGRESS_CLASS, PBS_SMOOTH | WS_VISIBLE, 14, 42, 332, 10, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_HEADER1, WC_STATIC, SS_GRAYRECT | WS_VISIBLE, 7, 60, 171, 11, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_BROWSE, WC_STATIC, SS_WHITERECT, 7, 71, 171, 204, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_OUTPUT, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 182, 71, 178, 145, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_LOG, WC_LISTVIEW, LVS_REPORT | LVS_ALIGNLEFT | LVS_NOCOLUMNHEADER | WS_TABSTOP | WS_VISIBLE, 182, 230, 178, 45, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_LINE1, WC_STATIC, SS_OWNERDRAW | WS_VISIBLE, 6, 71, 1, 204, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_LINE2, WC_STATIC, SS_OWNERDRAW | WS_VISIBLE, 6, 71, 1, 204, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_LINE3, WC_STATIC, SS_OWNERDRAW | WS_VISIBLE, 6, 71, 1, 204, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_LINE4, WC_STATIC, SS_OWNERDRAW | WS_VISIBLE, 6, 71, 1, 204, 0)
		CONTROL_CONTROL(_T(""), IDC_BP_LINE5, WC_STATIC, SS_OWNERDRAW | WS_VISIBLE, 6, 71, 1, 204, 0)
	END_CONTROLS_MAP()

BEGIN_COM_MAP(CStartPageBatchImageProcessor)
	COM_INTERFACE_ENTRY(IStartViewPage)
	COM_INTERFACE_ENTRY(IChildWindow)
	COM_INTERFACE_ENTRY(IDragAndDropHandler)
	COM_INTERFACE_ENTRY(IOleWindow)
	COM_INTERFACE_ENTRY(IShellBrowser)
	COM_INTERFACE_ENTRY_IID(IID_ICommDlgBrowser, ICommDlgBrowser)
	COM_INTERFACE_ENTRY(IStorageFilterWindowCallback)
END_COM_MAP()

BEGIN_MSG_MAP(CStartPageBatchImageProcessor)
	NOTIFY_HANDLER(IDC_BP_STARTSTOP, NM_CUSTOMDRAW, OnDrawOperation)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	NOTIFY_HANDLER(IDC_BP_STARTSTOP, TBN_DROPDOWN, OnDropdownOptions)
	COMMAND_HANDLER(ID_BP_HELP, BN_CLICKED, OnClickedHelp)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP(CThemeImpl<CStartPageBatchImageProcessor>)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_UPDATESTATUS, OnUpdateStatus)
	MESSAGE_HANDLER(WM_ADDLOGMESSAGE, OnAddLogMessage)
	MESSAGE_HANDLER(WM_CONFIGCHANGED, OnConfigChanged)
	MESSAGE_HANDLER(WM_GETISHELLBROWSER, OnGetIShellBrowser)
	//NOTIFY_HANDLER(IDC_BP_HEADER1, CTCN_SELCHANGE, OnTabSelchange)
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	COMMAND_HANDLER(ID_BP_START, BN_CLICKED, OnClickedStart)
	COMMAND_HANDLER(ID_BP_STOP, BN_CLICKED, OnClickedStop)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CStartPageBatchImageProcessor>)
	REFLECT_NOTIFICATIONS()
END_MSG_MAP()


	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (a_bBeforeAccel && m_hWnd != NULL)
		{
			if (m_pBrowseWnd)
			{
				if (S_OK == m_pBrowseWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel))
					return S_OK;
			}
			if (m_pActiveShellView)
			{
				HWND hMessageWnd = a_pMsg->hwnd;
				HWND hShellWnd = NULL;
				m_pActiveShellView->GetWindow(&hShellWnd);
				while (hMessageWnd != NULL)
				{
					if (hMessageWnd == hShellWnd && S_OK == m_pActiveShellView->TranslateAccelerator(const_cast<LPMSG>(a_pMsg)))
						return S_OK;
					else if (hMessageWnd == m_hWnd)
						break;
					hMessageWnd = ::GetParent(hMessageWnd);
				}
			}
		}
		return S_FALSE;
	}

	// IDragAndDropHandler methods
public:
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt);

	// IOleWindow methods
public:
	STDMETHOD(GetWindow)(HWND* a_phWnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL a_fEnterMode);

	// IShellBrowser methods
public:
	STDMETHOD(InsertMenusSB)(HMENU a_hMenuShared, LPOLEMENUGROUPWIDTHS a_pMenuWidths);
	STDMETHOD(SetMenuSB)(HMENU a_hMenuShared, HOLEMENU a_hOleMenuReserved, HWND a_hWndActiveObject);
	STDMETHOD(RemoveMenusSB)(HMENU a_hMenuShared);
	STDMETHOD(SetStatusTextSB)(LPCOLESTR a_pszStatusText);
	STDMETHOD(EnableModelessSB)(BOOL a_fEnable);
	STDMETHOD(BrowseObject)(LPCITEMIDLIST a_pIDL, UINT a_wFlags);
	STDMETHOD(GetViewStateStream)(DWORD a_dwGrfMode, LPSTREAM *a_ppStrm);
	STDMETHOD(OnViewWindowActive)(IShellView* a_pShellView);
	STDMETHOD(SetToolbarItems)(LPTBBUTTON a_pButtons, UINT a_nButtons, UINT a_uFlags);
	STDMETHOD(TranslateAcceleratorSB)(LPMSG a_pMsg, WORD a_wID);
	STDMETHOD(QueryActiveShellView)(IShellView** a_ppShellView);
	STDMETHOD(GetControlWindow)(UINT a_uID, HWND* a_phWnd);
	STDMETHOD(SendControlMsg)(UINT a_uID, UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, LRESULT* a_pResult);

	// ICommDlgBrowser methods
public:
	STDMETHOD(OnDefaultCommand)(IShellView* a_pShellView) { return E_NOTIMPL; }
	STDMETHOD(OnStateChange)(IShellView* a_pShellView, ULONG a_uChange) { return S_OK; }
	STDMETHOD(IncludeObject)(IShellView* a_pShellView, LPCITEMIDLIST a_pIDL) { return S_OK; }

	// handlers
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDropdownOptions(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnDrawOperation(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnClickedHelp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnClickedStart(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnClickedStop(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnUpdateStatus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAddLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnConfigChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetIShellBrowser(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	static void GetCogwheelPoints(IRPolyPoint* aInner, IRPolyPoint* aOuter);
	static HICON GetCogwheelIcon(ULONG size);
	static void GetRWBatchOpDocType(CComPtr<IDocumentTypeWildcards>& pDocType);

	bool OnOperationCreate();
	bool OnOperationDuplicate();
	bool OnOperationDelete();
	bool OnOperationImport();
	bool OnOperationExport();
	bool OnOperationConfigure();

	// IStorageFilterWindowCallback methods
public:
	STDMETHOD(ForwardOK)();
	STDMETHOD(ForwardCancel)() { return S_FALSE; }
	STDMETHOD(DefaultCommand)(ILocalizedString** a_ppName, ILocalizedString** a_ppDesc, GUID* a_pIconID);
	STDMETHOD(DefaultCommandIcon)(ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon)) { return E_NOTIMPL; }
	STDMETHOD(SelectionChanged)()
	{
		UpdateStatusText();
		return S_OK;
	}

	// IStartViewWnd methods
public:
	STDMETHOD(Activate)();
	STDMETHOD(Deactivate)();
	STDMETHOD(ClickedDefault)();

	void OwnerNotify(TCookie, IUnknown*);

public:
	struct SSubOp
	{
		ULONG hash;
		CComPtr<IConfig> config;
		CComPtr<ILocalizedString> name;
	};
	typedef std::vector<SSubOp> CSubOps;

	static void GetSubOps(IConfig* cfg, CSubOps& subOps);

private:
	struct SPathInfo
	{
		std::tstring strPath;
		int nRootLength;
		GUID tOperationID;
		CComPtr<IConfig> pOperationCfg;
		GUID tBuilderID;
		int jobId;
		std::tstring strOutput;
	};
	typedef std::deque<SPathInfo> CPaths;
	struct less_guid { bool operator()(GUID const& a_1, GUID const& a_2) const {return memcmp(&a_1, &a_2, sizeof(GUID)) < 0;} };
	typedef std::map<GUID, int, less_guid> CIconMap;
	class CColoredTabCtrl : public CDotNetTabCtrlImpl<CColoredTabCtrl>
	{
	public:
		CColoredTabCtrl()
		{
			m_hBrush = CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
		}
		~CColoredTabCtrl()
		{
			DeleteObject(m_hBrush);
		}
		DECLARE_WND_CLASS_EX(_T("WTL_ColoredDotNetTabCtrl"), CS_DBLCLKS, COLOR_WINDOW)

		BEGIN_MSG_MAP(CColoredTabCtrl)
			MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
			MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSettingChange)
			CHAIN_MSG_MAP(CDotNetTabCtrlImpl<CColoredTabCtrl>)
		END_MSG_MAP()

		LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			bHandled = FALSE;
			DeleteObject(m_hBrush);
			m_hBrush = CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
			return 0;
		}

		void InitializeDrawStruct(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
		{
			CDotNetTabCtrlImpl<CColoredTabCtrl>::InitializeDrawStruct(lpNMCustomDraw);
			lpNMCustomDraw->clrBtnFace = 
			lpNMCustomDraw->clrSelectedTab = ::GetSysColor(COLOR_WINDOW);
			lpNMCustomDraw->clrBtnHighlight = ::GetSysColor(COLOR_3DSHADOW);
			lpNMCustomDraw->hBrushBackground = m_hBrush;
		}
	private:
		HBRUSH m_hBrush;
	};

	//class ATL_NO_VTABLE CStorageWindowCallback :
	//	public CComObjectRootEx<CComMultiThreadModel>,
	//	public IStorageFilterWindowCallback
	//{
	//public:
	//	void Init(HWND a_hWnd)
	//	{
	//	}

	//BEGIN_COM_MAP(CStorageWindowCallback)
	//	COM_INTERFACE_ENTRY(IStorageFilterWindowCallback)
	//END_COM_MAP()

	//	// IStorageFilterWindowCallback methods
	//public:
	//	STDMETHOD(ForwardOK)()
	//	{
	//	}
	//	STDMETHOD(ForwardCancel)()
	//	{
	//		return S_FALSE;
	//	}

	//private:
	//	HWND m_hWnd;
	//};

private:
	static unsigned __stdcall WorkerProc(void* a_pThis);
	void UpdateStatusText();
	static bool GetSubOps(BSTR opId, IConfig* cfg, CSubOps& subOps, IConfig* cloneBase, BSTR cloneId);
	static void AddOpInfo(IConfigDescriptor* opAss, IConfig* opCfg, CSubOps& subOps, IConfig* cloneBase, BSTR cloneId, TConfigValue const* opId);
	static void AddOpsFromHistory(IConfig* templ, IConfig* history, IConfigDescriptor* opAss, CSubOps& subOps, IConfig* cloneBase, BSTR cloneId, TConfigValue const* opId);
	static ULONG ComputeCfgHash(IConfig* cfg);
	void RepositionControls(SIZE const sz);
	void UpdateActiveOperationCache();

private:
	CToolBarCtrl m_wndStartStop;
	CProgressBarCtrl m_wndProgress;
	CColoredTabCtrl m_wndHeader1;
	CComPtr<IStorageFilterWindow> m_pBrowseWnd;
	CColoredTabCtrl m_wndHeader2;
	CColoredTabCtrl m_wndHeader3;
	CListViewCtrl m_wndTasks;
	CStatic m_wndLine1;
	CStatic m_wndLine2;
	CStatic m_wndLine3;
	CStatic m_wndLine4;
	CStatic m_wndLine5;
	// output folder
	HWND m_hActiveShellWnd;
	CComPtr<IShellView> m_pActiveShellView;
	CComPtr<IShellFolder> m_pLastFolder;
	GUID m_tOpBuilder; // works as a filter in the Source files window
	CComPtr<IMalloc> m_pShellAlloc;
	CComPtr<IShellFolder> m_pDesktopFolder;
	FOLDERSETTINGS m_tViewSettings;
	CComBSTR m_lastActiveOutput;

	// GDI resources
	CImageList m_cToolbarImages;
	CImageList m_cLargeToolbarImages;
	CImageList m_cOperationImages;
	CImageList m_cLargeOperationImages;
	CIconMap m_cOperationIconMap;
	CIconMap m_cLargeOperationIconMap;
	CFont m_cLargeFont;
	HCURSOR m_hVertical;

	// operational state
	bool m_bHidden;
	bool m_bEnableUpdates;
	CComPtr<IConfig> m_pConfig;
	HANDLE m_hWorkThread;
	unsigned m_uWorkThreadID;
	HANDLE m_hWakeWorker;
	bool m_bStopWorker;
	CPaths m_cPaths;
	bool m_bRunning;
	bool m_bRequestPosted;
	CComPtr<IOperationManager> m_pOpMgr;
	int m_jobId;

	// cached active operation properties
	GUID m_activeOpId;
	CComPtr<IConfig> m_activeOpCfg;
	GUID m_activeIconId;
	CComPtr<ILocalizedString> m_activeName;
	CComPtr<ILocalizedString> m_activeSubtitle;
	CComPtr<ILocalizedString> m_activeDesc;
	GUID m_activeBuilder; // works as a filter in the Source files window
	CComBSTR m_activeOutput;

	// init resizing constants
	SIZE m_gap7;
	SIZE m_gap4;
	SIZE m_toolbar;
	ULONG m_progressY;
	ULONG m_headerY;
	ULONG m_logY;
	float m_scale;
	float m_splitPos;

	// splitter dragging
	LONG m_lastMousePos;
	LONG m_dragOffset;
};
