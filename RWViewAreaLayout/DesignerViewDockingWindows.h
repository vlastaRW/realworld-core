// DesignerViewDockingWindows.h : Declaration of the CDesignerViewDockingWindows

#pragma once
#include "resource.h"       // main symbols
#include "RWViewAreaLayout.h"
#include <Win32LangEx.h>
#include <DWAutoHide.h>
#include <DockingFrame.h>
#include "DockableWindow.h"


// CDesignerViewDockingWindows

class ATL_NO_VTABLE CDesignerViewDockingWindows : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public dockwins::CDockingFrameImplBase<CDesignerViewDockingWindows, Win32LangEx::CLangFrameWindowImpl<CDesignerViewDockingWindows, CWindow, dockwins::CDockingFrameTraits>, dockwins::CDockingFrameTraits>,
	public CDesignerViewWndImpl<CDesignerViewDockingWindows, IDesignerView>,
	public IDesignerViewDockingWindows
{
public:
	CDesignerViewDockingWindows() : m_pLastActive(NULL)
	{
	}
	~CDesignerViewDockingWindows()
	{
		for (CWindows::iterator i = m_cWindows.begin(); i != m_cWindows.begin(); i++)
		{
			delete *i;
		}
	}
	HRESULT Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pSubSpec, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, IDocument* a_pDoc, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID);


BEGIN_COM_MAP(CDesignerViewDockingWindows)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewDockingWindows)
END_COM_MAP()

	typedef dockwins::CDockingFrameImplBase<CDesignerViewDockingWindows, Win32LangEx::CLangFrameWindowImpl<CDesignerViewDockingWindows, CWindow, dockwins::CDockingFrameTraits>, dockwins::CDockingFrameTraits> baseFrame;
BEGIN_MSG_MAP(CDesignerViewText)
//	COMMAND_HANDLER(IDC_TEXT_TEXT, EN_CHANGE, OnTextChanged)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	CHAIN_MSG_MAP(baseFrame)
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)();
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges);
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces);
	STDMETHOD(OptimumSize)(SIZE* /*a_pSize*/) {return E_NOTIMPL;}

	// IDesignerViewDockingWindows methods
public:
	STDMETHOD(ItemCount)(ULONG* a_pnCount);
	STDMETHOD(NameGet)(ULONG a_nIndex, ILocalizedString** a_ppName);
	STDMETHOD(IconIDGet)(ULONG a_nIndex, GUID* a_ptIconID);
	STDMETHOD(IconGet)(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(IsVisible)(ULONG a_nIndex);
	STDMETHOD(SetVisible)(ULONG a_nIndex, BOOL a_bVisible);

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
//	LRESULT OnTextChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);

private:
	typedef std::vector<CDockableWindow*> CWindows;

private:
	CComPtr<IConfig> m_pConfig;
	CComPtr<IDesignerView> m_pMainWnd;
	IDesignerView* m_pLastActive;
	CWindows m_cWindows;
};

