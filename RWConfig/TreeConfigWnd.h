// TreeConfigWnd.h : Declaration of the CTreeConfigWnd

#pragma once
#include "resource.h"       // main symbols
#include "RWConfig.h"
#include <ObserverImpl.h>
#include <ContextHelpDlg.h>
#include "AutoConfigWnd.h"
#include "ConfigMessages.h"


// CTreeConfigWnd

class ATL_NO_VTABLE CTreeConfigWnd : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTreeConfigWnd, &CLSID_TreeConfigWnd>,
	public CChildWindowImpl<CTreeConfigWnd, IConfigWnd>,
	public CObserverImpl<CTreeConfigWnd, IConfigObserver, IUnknown*>,
	public Win32LangEx::CLangDialogImpl<CTreeConfigWnd>,
	public CContextHelpDlg<CTreeConfigWnd>
{
public:
	CTreeConfigWnd() : m_bUpdating(false), m_hResizeCursor(NULL), m_hWndNextViewer(NULL), m_pConfigWnd(NULL), m_eMode(ECPMFull)
	{
	}
	~CTreeConfigWnd()
	{
		m_cImageList.Destroy();
	}

	enum
	{
		IDD = IDD_TREECONFIG,
		GAP = 4,
		ID_TREE_ROOT = 150,
		ID_TREE_BACK,
		ID_TREE_FORWARD,
		ID_TREE_COPY,
		ID_TREE_PASTE,
	};

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CTreeConfigWnd)
	COM_INTERFACE_ENTRY(IChildWindow)
	COM_INTERFACE_ENTRY(IConfigWnd)
END_COM_MAP()

BEGIN_MSG_MAP(CTreeConfigWnd)
	CHAIN_MSG_MAP(CContextHelpDlg<CTreeConfigWnd>)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	COMMAND_ID_HANDLER(ID_TREE_ROOT, OnCommandRoot)
	COMMAND_ID_HANDLER(ID_TREE_BACK, OnCommandBack)
	COMMAND_ID_HANDLER(ID_TREE_FORWARD, OnCommandForward)
	COMMAND_ID_HANDLER(ID_TREE_COPY, OnCommandCopy)
	COMMAND_ID_HANDLER(ID_TREE_PASTE, OnCommandPaste)
	NOTIFY_HANDLER(IDC_TCF_TREE, TVN_SELCHANGED, OnSelectionChanged)
	MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCBChain)
	MESSAGE_HANDLER(WM_DRAWCLIPBOARD, OnDrawClipBoard)
	MESSAGE_HANDLER(WM_SUPPORTSCONFIGSELECT, OnSupportsConfigSelect)
	MESSAGE_HANDLER(WM_CONFIGSELECT, OnConfigSelect)
END_MSG_MAP()

BEGIN_CTXHELP_MAP(CTreeConfigWnd)
	CTXHELP_CONTROL_RESOURCE(IDC_TCF_TREE, IDS_HELP_TCF_TREE)
END_CTXHELP_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pConfig != NULL)
		{
			m_pConfig->ObserverDel(ObserverGet(), 0);
		}
	}

	virtual void OnFinalMessage(HWND a_hWnd);
	void OwnerNotify(TCookie a_tCookie, IUnknown* a_pIDs);

	// IConfigWnd methods
public:
	STDMETHOD(ConfigSet)(IConfig* a_pConfig, EConfigPanelMode a_eMode);
	STDMETHOD(TopWindowSet)(BOOL a_bIsTopWindow, DWORD a_clrBackground);
	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode);
	STDMETHOD(OptimumSize)(SIZE *a_pSize) { return E_NOTIMPL; }
	STDMETHOD(ChangeLanguage)(LCID UNREF(a_tLocaleID)) { return E_NOTIMPL; }

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSelectionChanged(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnCommandRoot(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCommandBack(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCommandForward(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCommandCopy(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCommandPaste(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnChangeCBChain(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDrawClipBoard(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSupportsConfigSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnConfigSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

private:
	typedef std::stack<CComPtr<IConfig> > CHistory;

private:
	void ResizeSubWindows(int a_nSizeX, int a_nSizeY);
	bool RefreshItemIDs();
	void ClearTree();
	static void InsertSubItems(IConfig* a_pConfig, CTreeViewCtrl& a_wndTree, IEnumStrings* a_pItemIDs, int& a_nNextItem, HTREEITEM a_hParent, LPCOLESTR a_pszPrefix, LCID a_tLocaleID);
	static void GetItemText(IConfig* a_pConfig, size_t nPrefix, BSTR a_bstrID, LCID a_tLocaleID, std::tstring& a_strText);
	bool SelectTreeItem(HTREEITEM a_hRoot, INT_PTR a_xItemData);

private:
	CComPtr<IConfig> m_pConfig;
	EConfigPanelMode m_eMode;
	CComObject<CAutoConfigWnd>* m_pConfigWnd;
	CComPtr<IUnknown> m_pConfigWndRef;
	CTreeViewCtrl m_wndTree;
	CToolBarCtrl m_wndToolbar;
	CComBSTR m_bstrSelected;
	HTREEITEM m_hRootItem;
	CImageList m_cImageList;
	HCURSOR m_hResizeCursor;
	int m_nResizeDelta;
	int m_nTBHeight;

	static float s_fInitialTreeSize;
	float m_fTreeSize;
	bool m_bUpdating;

	CHistory m_cBack;
	CHistory m_cForward;
	CComPtr<IConfig> m_pCurCfg;

	UINT m_nClipboardFormat;

	HWND m_hWndNextViewer; // clipboard
};

OBJECT_ENTRY_AUTO(__uuidof(TreeConfigWnd), CTreeConfigWnd)
