// DesignerViewChooseBestLayoutSubDoc.h : Declaration of the CDesignerViewChooseBestLayoutSubDoc

#pragma once
#include "resource.h"       // main symbols
#include "RWViewStructure.h"


// CDesignerViewChooseBestLayoutSubDoc

class ATL_NO_VTABLE CDesignerViewChooseBestLayoutSubDoc : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerView,
	public CObserverImpl<CDesignerViewChooseBestLayoutSubDoc, ISharedStateObserver, TSharedStateChange>
{
public:
	CDesignerViewChooseBestLayoutSubDoc()
	{
	}
	~CDesignerViewChooseBestLayoutSubDoc()
	{
		if (m_pFrame) m_pFrame->ObserverDel(ObserverGet(), 0);
	}

	void Init(IViewManager* a_pViewMgr, ISharedStateManager* a_pFrame, BSTR a_bstrSyncID, IStatusBarObserver* a_pStatusBar, LONG nLayouts, IConfig* pLayouts, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, ISubDocumentsMgr* pSDM)
	{
		m_pViewMgr = a_pViewMgr;
		m_pFrame = a_pFrame;
		pSDM->StatePrefix(&m_bstrSyncID);
		m_bstrSyncID += a_bstrSyncID;
		m_pFrame->ObserverIns(ObserverGet(), 0);
		m_pStatusBar = a_pStatusBar;
		m_nLayouts = nLayouts;
		m_pLayouts = pLayouts;
		m_hParent = a_hParent;
		m_tLocaleID = a_tLocaleID;
		m_nStyle = a_nStyle;
		m_pDoc = a_pDoc;
		m_pSDM = pSDM;

		CComPtr<ISharedState> pState;
		a_pFrame->StateGet(a_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IDocument> pDoc;
		GetSubDoc(pState, &pDoc);

		LONG nBest = GetBestLayout(pDoc, m_nLayouts, m_pLayouts, m_pViewMgr);
		if (nBest >= 0 && CreateView(nBest, *a_rcWindow))
		{
			m_nCurrent = nBest;
		}
		else
		{
			m_wndNothing.Create(m_hParent, const_cast<RECT*>(a_rcWindow), NULL, WS_VISIBLE|WS_CHILD|SS_GRAYRECT);
			m_nCurrent = -1;
		}
	}

BEGIN_COM_MAP(CDesignerViewChooseBestLayoutSubDoc)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()


	void OwnerNotify(TCookie a_tCookie, TSharedStateChange a_tCategory)
	{
		if (m_bstrSyncID != a_tCategory.bstrName)
			return;
		CComPtr<IDocument> pDoc;
		GetSubDoc(a_tCategory.pState, &pDoc);
		LONG nBest = GetBestLayout(pDoc, m_nLayouts, m_pLayouts, m_pViewMgr);
		if (nBest != m_nCurrent)
		{
			bool bIsVisible = (::GetWindowLong(m_hParent, GWL_STYLE)&WS_VISIBLE) == WS_VISIBLE;

			bool bPrevVisible = true;
			// destroy window
			RECT rc;
			if (m_pSubWnd)
			{
				HWND hWnd = NULL;
				m_pSubWnd->Handle(&hWnd);
				::GetWindowRect(hWnd, &rc);
				bPrevVisible = (::GetWindowLong(hWnd, GWL_STYLE)&WS_VISIBLE) == WS_VISIBLE;
				m_pSubWnd->OnDeactivate(FALSE);
				m_pSubWnd->Destroy();
				m_pSubWnd = NULL;
			}
			else
			{
				m_wndNothing.GetWindowRect(&rc);
				bPrevVisible = (m_wndNothing.GetStyle()&WS_VISIBLE) == WS_VISIBLE;
				m_wndNothing.DestroyWindow();
			}
			::ScreenToClient(m_hParent, (LPPOINT)&rc.left);
			::ScreenToClient(m_hParent, (LPPOINT)&rc.right);

			if (nBest >= 0 && CreateView(nBest, rc))
			{
				m_nCurrent = nBest;
				if (!bPrevVisible)
					m_pSubWnd->Show(FALSE);
			}
			else
			{
				m_wndNothing.Create(m_hParent, rc, NULL, bPrevVisible ? WS_VISIBLE|WS_CHILD|SS_GRAYRECT : WS_CHILD|SS_GRAYRECT);
				m_nCurrent = -1;
			}

			if (bIsVisible)
			{
				//m_wndParent.SetRedraw(TRUE);
				::RedrawWindow(m_hParent, &rc, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
		}
	}

	// IChildWindow methods
public:
	STDMETHOD(Handle)(RWHWND *a_pHandle) { return m_pSubWnd ? m_pSubWnd->Handle(a_pHandle) : E_FAIL; }
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam) { return m_pSubWnd ? m_pSubWnd->SendMessage(a_uMsg, a_wParam, a_lParam) : E_FAIL; }
	STDMETHOD(Show)(BOOL a_bShow) { return m_pSubWnd ? m_pSubWnd->Show(a_bShow) : E_FAIL; }
	STDMETHOD(Move)(RECT const* a_prcPosition) { return m_pSubWnd ? m_pSubWnd->Move(a_prcPosition) : E_FAIL; }
	STDMETHOD(Destroy)() { return m_pSubWnd ? m_pSubWnd->Destroy() : E_FAIL; }
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel) { return m_pSubWnd ? m_pSubWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE; }

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)() { return m_pSubWnd ? m_pSubWnd->OnIdle() : S_FALSE; }
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges) { return m_pSubWnd ? m_pSubWnd->OnDeactivate(a_bCancelChanges) : S_FALSE; }
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces) { return m_pSubWnd ? m_pSubWnd->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces) : S_OK; }
	STDMETHOD(OptimumSize)(SIZE* a_pSize){ return m_pSubWnd ? m_pSubWnd->OptimumSize(a_pSize) : E_NOTIMPL; }
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges) { return m_pSubWnd ? m_pSubWnd->DeactivateAll(a_bCancelChanges) : S_OK; }

private:
	bool CreateView(LONG nView, RECT rcWindow)
	{
		if (nView < 0 || nView >= m_nLayouts)
			return false;

		OLECHAR szNameID[64];
		_snwprintf(szNameID, itemsof(szNameID), L"%08x\\%s", nView, CFGID_BEST_SUBVIEW);
		CComBSTR bstrID(szNameID);

		CConfigValue cSubViewID;
		m_pLayouts->ItemValueGet(bstrID, &cSubViewID);
		CComPtr<IConfig> pSubViewCfg;
		m_pLayouts->SubConfigGet(bstrID, &pSubViewCfg);

		m_pViewMgr->CreateWnd(m_pViewMgr, cSubViewID, pSubViewCfg, m_pFrame, m_pStatusBar, m_pDoc, m_hParent, &rcWindow, m_nStyle, m_tLocaleID, &m_pSubWnd);
		return m_pSubWnd != NULL;
	}
	void GetSubDoc(ISharedState* pState, IDocument** ppDoc)
	{
		CComPtr<IEnumUnknowns> pItems;
		m_pSDM->StateUnpack(pState, &pItems);
		CComPtr<IComparable> pItem;
		if (pItems) pItems->Get(0, &pItem);
		CComQIPtr<ISubDocumentID> pSDID(pItem);
		if (pSDID) pSDID->SubDocumentGet(ppDoc);
	}


private:
	CComPtr<ISharedStateManager> m_pFrame;
	CComBSTR m_bstrSyncID;
	CComPtr<IStatusBarObserver> m_pStatusBar;
	LONG m_nLayouts;
	CComPtr<IConfig> m_pLayouts;
	RWHWND m_hParent;
	LCID m_tLocaleID;
	EDesignerViewWndStyle m_nStyle;
	CComPtr<IViewManager> m_pViewMgr;
	CComPtr<IDocument> m_pDoc;
	CComPtr<ISubDocumentsMgr> m_pSDM;

	CComPtr<IDesignerView> m_pSubWnd;
	LONG m_nCurrent;
	CStatic m_wndNothing;
};

