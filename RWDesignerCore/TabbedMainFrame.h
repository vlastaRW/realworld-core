// MainFrame.h : interface of the CTabbedMainFrame class
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
#include "TabbedPage.h"


class CTabbedWindowThread;

class ATL_NO_VTABLE CTabbedMainFrame :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CTabbedMainFrame>,
	//public CContextMenuWithIcons<CTabbedMainFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public CClipboardViewer<CTabbedMainFrame>,
	public IDropTarget
{
public:
	CTabbedMainFrame();
	~CTabbedMainFrame();
	HRESULT Init(CThreadCommand* a_pOrder, CTabbedWindowThread* a_pThread);

	BEGIN_COM_MAP(CTabbedMainFrame)
		COM_INTERFACE_ENTRY(IDropTarget)
	END_COM_MAP()

	static ATL::CWndClassInfo& GetWndClassInfo()
	{
		static ATL::CWndClassInfo wc =
		{
			{ sizeof(WNDCLASSEX), 0, StartWindowProc,
			  0, 0, NULL, NULL, NULL, (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, _T("RWTabbedFrame"), NULL },
			NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
		};
		return wc;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	enum
	{
		//ID_CLOSEDOCUMENT = 27147,
		//ID_LAYOUT_MENU1 = 27148,
		//WM_SENDSTATECHANGES = WM_APP+438,
	};

	BEGIN_MSG_MAP(CTabbedMainFrame)
		MESSAGE_HANDLER(WM_NCHITTEST, OnNCHitTest)
		MESSAGE_HANDLER(WM_NCMOUSEMOVE, OnNCMouseMove)
		//MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		//MESSAGE_HANDLER(WM_CLOSE, OnClose)
		//COMMAND_RANGE_HANDLER(ID_LAYOUT_MENU1, (ID_LAYOUT_MENU1+2049), OnLayoutOperation) // TODO: remove the 1000 limit (and constant)
		//MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		//CHAIN_MSG_MAP(Win32LangEx::CLangFrameWindowImpl<CTabbedMainFrame>)
		CHAIN_MSG_MAP(CClipboardViewer<CTabbedMainFrame>)
		//COMMAND_HANDLER(ID_CLOSEDOCUMENT, BN_CLICKED, OnFileClose)
		//CHAIN_MSG_MAP(CContextMenuWithIcons<CTabbedMainFrame>)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
		//MESSAGE_HANDLER(WM_SENDSTATECHANGES, OnSendStateChanges)
		//MESSAGE_HANDLER(WM_RW_DEACTIVATE, OnDeactivateView)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		//MESSAGE_HANDLER(WM_NCCALCSIZE, OnNCCalcSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_NCCREATE, OnNCCreate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNCLButtonDown)
		MESSAGE_HANDLER(WM_NCMOUSELEAVE, OnNCMouseLeave)
	END_MSG_MAP()

	LRESULT OnNCCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnNCCalcSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnNCHitTest(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnNCMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnNCMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnNCLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//LRESULT OnClose(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnDropFiles(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

private:
	enum
	{
		ETBToolBar = ATL_IDW_BAND_FIRST + 1,
		ETBMenu = ATL_IDW_BAND_FIRST + 2,
		ETBOperationsBar = ATL_IDW_BAND_FIRST + 3
	};

public:
	CTabbedWindowThread* GetThread() const { return m_pThread; }

	// IDropTarget methods
public:
	STDMETHOD(DragEnter)(IDataObject* a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragOver)(DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject *a_pDataObj, DWORD a_grfKeyState, POINTL a_pt, DWORD *a_pdwEffect);

	HRESULT DragImpl(DWORD a_grfKeyState, POINTL a_pt, DWORD* a_pdwEffect);

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

private:
	bool CreateView(IConfig* a_pViewProfile, const RECT& a_rcView);
	bool DestroyView();
	void UpdateCaption();
	int AppMessageBox(LPCTSTR a_pszText, UINT a_nFlags) const;
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

	void UpdateDrawingConstants();

private:
	LCID m_tLocaleID;
	HICON m_hIconSmall;
	HICON m_hIconLarge;
	CComPtr<ILocalizedString> m_pAppName;

	// shared data
	CTabbedWindowThread* m_pThread;

	// document / start view options
	CLSID m_tStartPage;
	CComPtr<IDocument> m_pDoc;
	CComQIPtr<IDocumentUndo> m_pDocUndo;

	// extern tools
	CExternTools m_cExternTools;

	// configuration
	CComPtr<IConfig> m_pCurrentLayoutCfg;
	CComBSTR m_strActiveLayout;

	CFrameIcons m_cFrameIcons;

	// main menu
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

	// drawing constants
	bool m_bGlass;
	RECT m_rcBorder;
	LONG m_nCaptionHeight;
	LONG m_nTabPanelWidth;
	float m_fScale;
	LONG m_nGlobeSize;
	LONG m_nLargeSize;
	LONG m_nSmallSize;
	LONG m_nGapSize;
	LONG m_nThumbnailSizeX;
	LONG m_nThumbnailSizeY;

	// drawing state
	bool m_bTracking;
	bool m_bGlobeHot;
	bool m_bIntelliTipHot;
	bool m_bContextHelpHot;

	// client windows
	CImageList m_cQuickToolsImages;
	CToolBarCtrl m_wndQuickTools;
	CWindow m_wndClient;
	CThreadCommand* m_pOrder;
};
