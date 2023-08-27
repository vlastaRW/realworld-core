// DesignerViewWndSubDocumentFrame.h : Declaration of the CDesignerViewWndSubDocumentFrame

#pragma once
#include "resource.h"       // main symbols
#include "RWViewStructure.h"
#include <ObserverImpl.h>


// CDesignerViewWndSubDocumentFrame

class ATL_NO_VTABLE CDesignerViewWndSubDocumentFrame : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerView,
	public CObserverImpl<CDesignerViewWndSubDocumentFrame, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewWndSubDocumentFrame, IStructuredObserver, TStructuredChanges>
{
public:
	CDesignerViewWndSubDocumentFrame()
	{
	}

	void Init(IViewManager* a_pViewMgr, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, ISubDocumentsMgr* a_pDocMgr);

BEGIN_COM_MAP(CDesignerViewWndSubDocumentFrame)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pFrame)
			m_pFrame->ObserverDel(CObserverImpl<CDesignerViewWndSubDocumentFrame, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		ClearSubCtxs(0);
		m_aSubCtxs.clear();
	}

	// observer handlers
public:
	void OwnerNotify(TCookie a_tCookie, TSharedStateChange a_tCategory);
	void OwnerNotify(TCookie a_tCookie, TStructuredChanges a_tChanges);

	// IChildWindow methods
public:
	STDMETHOD(Handle)(RWHWND *a_pHandle);
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);
	STDMETHOD(Show)(BOOL a_bShow);
	STDMETHOD(Move)(RECT const* a_prcPosition);
	STDMETHOD(Destroy)();
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)();
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges);
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces);
	STDMETHOD(OptimumSize)(SIZE* a_pSize);
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges);

private:
	void SetSubView(IDocument* a_pDoc);
	bool UpdateSubCtxs(ULONG a_nFrom);
	void ClearSubCtxs(ULONG a_nFrom);

private:
	struct SSubCtx
	{
		CComPtr<ISubDocumentsMgr> pDocMgr;
		CComBSTR bstrSyncID;
		CComBSTR bstrSyncIDRaw;
		CComPtr<IComparable> pItem;
		CComPtr<IDocument> pSubDoc;
	};
	typedef std::vector<SSubCtx> CSubCtxs;

private:
	CConfigValue m_cSubViewID;
	CComPtr<IConfig> m_pSubViewCfg;
	CComPtr<ISharedStateManager> m_pFrame;
	CComPtr<IStatusBarObserver> m_pStatusBar;
	LCID m_tLocaleID;
	EDesignerViewWndStyle m_nStyle;
	CComPtr<IViewManager> m_pViewMgr;

	CSubCtxs m_aSubCtxs;
	CComPtr<IDesignerView> m_pSubWnd;
	CStatic m_wndNothing;
	CWindow m_wndParent;

	std::wstring m_strEmptyMsg;
};

