// DesignerViewTab.h : Declaration of the CDesignerViewTab

#pragma once
#include "resource.h"       // main symbols
#include "RWViewAreaLayout.h"
#include <atlgdix.h>
#include <CustomTabCtrl.h>
#include <DotNetTabCtrl.h>
#include <vector>
#include <ContextMenuWithIcons.h>


// CDesignerViewTab

class ATL_NO_VTABLE CDesignerViewTab : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewTab>,
	public CThemeImpl<CDesignerViewTab>,
	public CChildWindowImpl<CDesignerViewTab, IDesignerView>,
	public IDesignerViewTabControl,
	public IDragAndDropHandler,
	public CContextMenuWithIcons<CDesignerViewTab>
{
public:
	CDesignerViewTab() : m_iActiveView(-1), m_dwTabFlags(0), m_bTabActive(false), m_bInvisibleTab(false),
		m_iWaitItem(-1), m_tLocaleID(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT)),
		m_tabHeight(26), m_buttonWidth(22), m_nIconSize(16)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	void Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pViewManager, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID);

	enum { DRAGTIMERID = 13 };

BEGIN_MSG_MAP(CDesignerViewTab)
	NOTIFY_HANDLER(IDC_TAB, CTCN_SELCHANGE, OnTabChange)
	NOTIFY_HANDLER(IDC_TAB, CTCN_CLOSE, OnTabClose)
	NOTIFY_HANDLER(IDC_TAB, NM_RCLICK, OnTabRClick)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewTab>)
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	MESSAGE_HANDLER(WM_RW_GOTFOCUS, OnRWGotFocus)
	MESSAGE_HANDLER(WM_RW_DEACTIVATE, OnRWForward)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewTab>)
	COMMAND_HANDLER(IDC_BUTTON, BN_CLICKED, OnReactivate)
    MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
END_MSG_MAP()

BEGIN_COM_MAP(CDesignerViewTab)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewTabControl)
	COM_INTERFACE_ENTRY(IDragAndDropHandler)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		m_cIcons.Destroy();
	}

	// handlers
public:
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnTabChange(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled);
	LRESULT OnTabClose(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled);
	LRESULT OnTabRClick(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnReactivate(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		ActiveIndexSet(0);
		return 0;
	}
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

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

	// IDesignerViewTabControl methods
public:
	STDMETHOD(TabID)(BSTR* a_pbstrID);
	STDMETHOD(ActiveIndexGet)(ULONG* a_pnIndex);
	STDMETHOD(ActiveIndexSet)(ULONG a_nIndex);
	STDMETHOD(ItemCount)(ULONG* a_pnCount);
	STDMETHOD(NameGet)(ULONG a_nIndex, ILocalizedString** a_ppName);
	STDMETHOD(IconIDGet)(ULONG a_nIndex, GUID* a_ptIconID);
	STDMETHOD(IconGet)(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon);

	// IDragAndDropHandler methods
public:
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* UNREF(a_pDataObj), IEnumStrings* UNREF(a_pFileNames), DWORD UNREF(a_grfKeyState), POINT UNREF(a_pt)) { return E_FAIL; }

private:
	enum { IDC_TAB = 100, IDC_BUTTON = 99 };

	void SplitArea(RECT const& a_rcClient, RECT* a_prcTab, RECT* a_prcWindow, CWindow& a_wndTab);

	class CColoredTabCtrl : public CDotNetTabCtrlImpl<CColoredTabCtrl, CCustomTabItem>
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

			int nRed = GetRValue(lpNMCustomDraw->clrBtnFace);
			int nGreen = GetGValue(lpNMCustomDraw->clrBtnFace);
			int nBlue = GetBValue(lpNMCustomDraw->clrBtnFace);

			int nMax = (nRed > nGreen) ? ((nRed > nBlue) ? nRed : nBlue) : ((nGreen > nBlue) ? nGreen : nBlue);
			const BYTE nMagicBackgroundOffset = (nMax > (0xFF - 35)) ? (BYTE)(0xFF - nMax) : (BYTE)35;
			if(nMax == 0)
			{
				nRed = (BYTE)(nRed + nMagicBackgroundOffset);
				nGreen = (BYTE)(nGreen + nMagicBackgroundOffset);
				nBlue = (BYTE)(nBlue + nMagicBackgroundOffset);
			}
			else
			{
				nRed = (BYTE)(nRed + (nMagicBackgroundOffset*(nRed/(double)nMax) + 0.5));
				nGreen = (BYTE)(nGreen + (nMagicBackgroundOffset*(nGreen/(double)nMax) + 0.5));
				nBlue = (BYTE)(nBlue + (nMagicBackgroundOffset*(nBlue/(double)nMax) + 0.5));
			}

			//m_hbrBackground.CreateSolidBrush(RGB(nRed, nGreen, nBlue));

			lpNMCustomDraw->clrBtnFace = 
			lpNMCustomDraw->clrSelectedTab = RGB(nRed, nGreen, nBlue);//::GetSysColor(COLOR_WINDOW);
			lpNMCustomDraw->clrBtnHighlight = ::GetSysColor(COLOR_3DSHADOW);
			lpNMCustomDraw->hBrushBackground = m_hBrush;
		}
	private:
		HBRUSH m_hBrush;
	};

	typedef std::map<LONG, LONG> CIIMap;

private:
	CComPtr<IConfig> m_pConfig;
	CComPtr<ISharedStateManager> m_pFrame;
	CComPtr<IStatusBarObserver> m_pStatusBar;
	CComPtr<IViewManager> m_pViewManager;
	CComPtr<IDocument> m_pDocument;
	CColoredTabCtrl m_wndTab;
	int m_tabHeight;
	int m_buttonWidth;
	int m_nIconSize;
	int m_iActiveView;
	std::vector<CComPtr<IDesignerView> > m_vpWnd; // Windows under the Tab control
	CIIMap m_cCfgToTab;
	CIIMap m_cTabToCfg;
	LCID m_tLocaleID;
	DWORD m_dwTabFlags;
	bool m_bTabActive;
	CImageList m_cIcons;
	bool m_bInvisibleTab;
	CBrush m_hClientBG;
	COLORREF m_clrClientBG;

	CImageList m_cImageList; // context menu

	int m_iWaitItem;
	DWORD m_dwWaitStart;
	POINT m_ptWaitPos;

	CButton m_wndReactivate;
};
