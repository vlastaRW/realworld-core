// DesignerViewSubDocumentFrame.cpp : Implementation of CDesignerViewSubDocumentFrame

#include "stdafx.h"
#include "DesignerViewSubDocumentFrame.h"

#include "ConfigIDsSubDocumentFrame.h"
#include <MultiLanguageString.h>


// CDesignerViewWndSubDocumentFrame

void CDesignerViewWndSubDocumentFrame::Init(IViewManager* a_pViewMgr, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, ISubDocumentsMgr* a_pDocMgr)
{
	m_pViewMgr = a_pViewMgr;
	m_nStyle = a_nStyle;
	m_tLocaleID = a_tLocaleID;
	m_wndParent = a_hParent;

	m_pStatusBar = a_pStatusBar;

	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_FRM_SELSYNCGROUP), &cVal);
	{
		// support multi-level extraction (IDs spearated by space or comma)
		wchar_t const* p = cVal;
		while (*p)
		{
			while (*p == L' ' || *p == L',')
				++p;
			wchar_t const* p1 = p;
			while (*p1 && *p1 != L' ' && *p1 != L',')
				++p1;
			if (p < p1)
			{
				m_aSubCtxs.resize(m_aSubCtxs.size()+1);
				SSubCtx& s = m_aSubCtxs[m_aSubCtxs.size()-1];
				s.bstrSyncIDRaw.Attach(::SysAllocStringLen(p, p1-p));
			}
			p = p1;
		}
	}

	a_pConfig->ItemValueGet(CComBSTR(CFGID_FRM_NOVIEWMSG), &cVal);
	{
		CComBSTR bstr;
		CMultiLanguageString::GetLocalized(cVal.operator BSTR(), m_tLocaleID, &bstr);
		m_strEmptyMsg = bstr;
	}

	a_pConfig->ItemValueGet(CComBSTR(CFGID_FRM_SUBVIEW), &m_cSubViewID);
	a_pConfig->SubConfigGet(CComBSTR(CFGID_FRM_SUBVIEW), &m_pSubViewCfg);

	a_pFrame->ObserverIns(CObserverImpl<CDesignerViewWndSubDocumentFrame, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	m_pFrame = a_pFrame;

	// create the window

	if (!m_aSubCtxs.empty())
	{
		SSubCtx& s = m_aSubCtxs[0];
		CComBSTR bstrPrefix;
		a_pDocMgr->StatePrefix(&s.bstrSyncID);
		s.bstrSyncID += s.bstrSyncIDRaw;
		s.pDocMgr = a_pDocMgr;
		s.pDocMgr->ObserverIns(CObserverImpl<CDesignerViewWndSubDocumentFrame, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
		UpdateSubCtxs(0);
	}

	m_wndNothing.Create(a_hParent, *const_cast<RECT*>(a_rcWindow), !m_strEmptyMsg.empty() ? m_strEmptyMsg.c_str() : NULL, m_strEmptyMsg.empty() ? WS_VISIBLE|WS_CHILD|SS_GRAYRECT : WS_VISIBLE|WS_CHILD|SS_CENTER|SS_CENTERIMAGE);
	m_wndNothing.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	if (!m_aSubCtxs.empty() && m_aSubCtxs.rbegin()->pSubDoc)
		SetSubView(m_aSubCtxs.rbegin()->pSubDoc);
}

void CDesignerViewWndSubDocumentFrame::ClearSubCtxs(ULONG a_nFrom)
{
	for (size_t i = m_aSubCtxs.size(); i > a_nFrom; --i)
	{
		SSubCtx& s = m_aSubCtxs[i-1];
		if (s.pDocMgr)
		{
			s.pDocMgr->ObserverDel(CObserverImpl<CDesignerViewWndSubDocumentFrame, IStructuredObserver, TStructuredChanges>::ObserverGet(), i-1);
			s.pDocMgr = NULL;
		}
		s.bstrSyncID.Empty();
		s.pItem = NULL;
		s.pSubDoc = NULL;
	}
}

bool CDesignerViewWndSubDocumentFrame::UpdateSubCtxs(ULONG a_nFrom)
{
	if (m_aSubCtxs.size() <= a_nFrom)
		return false;
	CSubCtxs::iterator i = m_aSubCtxs.begin()+a_nFrom;

	CComPtr<ISharedState> pState;
	m_pFrame->StateGet(i->bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
	CComPtr<IEnumUnknowns> pSelectedItems;
	i->pDocMgr->StateUnpack(pState, &pSelectedItems);
	CComPtr<IComparable> pItem;
	if (pSelectedItems != NULL)
		pSelectedItems->Get(0, &pItem);
	if (pItem == NULL)
	{
		if (i->pItem == NULL)
			return false; // no real change
		i->pItem = NULL;
		i->pSubDoc = NULL;
		ClearSubCtxs(a_nFrom+1);
		return true;
	}

	if (i->pItem && i->pItem->Compare(pItem) == S_OK)
		return false; // same item

	i->pItem = pItem;
	i->pSubDoc = NULL;
	ClearSubCtxs(a_nFrom+1);

	CComPtr<ISubDocumentID> pID;
	if (pItem)
		i->pDocMgr->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pID));
	if (pID)
		pID->SubDocumentGet(&i->pSubDoc);

	if (i->pSubDoc == NULL)
		return true; // failure

	CSubCtxs::iterator j = i;
	++j;
	if (j != m_aSubCtxs.end())
	{
		i->pSubDoc->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&(j->pDocMgr)));
		if (j->pDocMgr)
		{
			j->pDocMgr->ObserverIns(CObserverImpl<CDesignerViewWndSubDocumentFrame, IStructuredObserver, TStructuredChanges>::ObserverGet(), a_nFrom+1);
			j->pDocMgr->StatePrefix(&j->bstrSyncID);
			j->bstrSyncID += j->bstrSyncIDRaw;
			return UpdateSubCtxs(a_nFrom+1);
		}
	}

	return true;
}

void CDesignerViewWndSubDocumentFrame::SetSubView(IDocument* a_pDoc)
{
	try
	{
		bool bIsVisible = (m_wndParent.GetStyle()&WS_VISIBLE) == WS_VISIBLE;

		//if (bIsVisible)
		//{
		//	m_wndParent.SetRedraw(FALSE);
		//}

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
			if (a_pDoc == NULL)
				return;
			m_wndNothing.GetWindowRect(&rc);
			bPrevVisible = (m_wndNothing.GetStyle()&WS_VISIBLE) == WS_VISIBLE;
			m_wndNothing.DestroyWindow();
		}
		m_wndParent.ScreenToClient(&rc);

		if (a_pDoc != NULL)
		{
			m_pViewMgr->CreateWnd(m_pViewMgr, m_cSubViewID, m_pSubViewCfg, m_pFrame, m_pStatusBar, a_pDoc, reinterpret_cast<RWHWND>(m_wndParent.m_hWnd), &rc, m_nStyle, m_tLocaleID, &m_pSubWnd);
			if (!bPrevVisible && m_pSubWnd)
				m_pSubWnd->Show(FALSE);
		}

		if (m_pSubWnd == NULL)
		{
			m_wndNothing.Create(m_wndParent, rc, !m_strEmptyMsg.empty() ? m_strEmptyMsg.c_str() : NULL, bPrevVisible ? (m_strEmptyMsg.empty() ? WS_VISIBLE|WS_CHILD|SS_GRAYRECT : WS_VISIBLE|WS_CHILD|SS_CENTER|SS_CENTERIMAGE) : (m_strEmptyMsg.empty() ? WS_CHILD|SS_GRAYRECT : WS_CHILD|SS_CENTER|SS_CENTERIMAGE));
			m_wndNothing.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		}

		if (bIsVisible)
		{
			//m_wndParent.SetRedraw(TRUE);
			m_wndParent.RedrawWindow(&rc, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
	catch (...)
	{
	}
}

void CDesignerViewWndSubDocumentFrame::OwnerNotify(TCookie, TSharedStateChange a_tCategory)
{
	try
	{
		if (m_pSubWnd == NULL && m_wndNothing.m_hWnd == NULL)
			return; // TODO: ??? 

		size_t i = 0;
		while (i != m_aSubCtxs.size() && m_aSubCtxs[i].bstrSyncID != a_tCategory.bstrName)
			++i;
		if (i == m_aSubCtxs.size())
			return; // notification is not for our group

		if (UpdateSubCtxs(i))
			SetSubView(!m_aSubCtxs.empty() ? m_aSubCtxs.rbegin()->pSubDoc.p : NULL);
	}
	catch (...)
	{
	}
}

void CDesignerViewWndSubDocumentFrame::OwnerNotify(TCookie a_nLevel, TStructuredChanges a_tChanges)
{
	try
	{
		if (m_pSubWnd == NULL)
			return; // TODO: ??? 

		if (a_nLevel >= m_aSubCtxs.size())
			return; // weird

		SSubCtx& s = m_aSubCtxs[a_nLevel];
		// something has changed, check if selection is still valid
		CComPtr<IComparable> pItem;
		s.pDocMgr->ItemFeatureGet(s.pItem, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
		//CComPtr<IComparable> pItem;
		//m_pDocMgr->ItemFeatureGet(m_pSubWndID, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
		if (pItem)
			return;

		if (UpdateSubCtxs(a_nLevel))
			SetSubView(!m_aSubCtxs.empty() ? m_aSubCtxs.rbegin()->pSubDoc.p : NULL);

		//CComPtr<ISharedState> pState;
		//m_pFrame->StateGet(CComBSTR(m_strSelGrp.c_str()), __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		//CComPtr<IEnumUnknowns> pSelectedItems;
		//if (pState) m_pDocMgr->StateUnpack(pState, &pSelectedItems);
		//if (pSelectedItems) pSelectedItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));

		//if (pItem == NULL)
		//{
		//	// try to select first available subdocument
		//	CComPtr<IEnumUnknowns> pSubDocs;
		//	m_pDocMgr->ItemsEnum(NULL, &pSubDocs);
		//	if (pSubDocs != NULL)
		//		pSubDocs->Get(0, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pItem));
		//}

		//if (pItem)
		//{
		//	SetSubView(pItem);
		//}
		//else
		//{
		//	HWND hWnd = NULL;
		//	m_pSubWnd->Handle(&hWnd);
		//	RECT rc;
		//	::GetWindowRect(hWnd, &rc);
		//	bool bPrevVisible = (::GetWindowLong(hWnd, GWL_STYLE)&WS_VISIBLE) == WS_VISIBLE;
		//	m_wndParent.ScreenToClient(&rc);
		//	m_pSubWnd->OnDeactivate(FALSE);
		//	m_pSubWnd->Destroy();
		//	m_pSubWnd = NULL;
		//	m_wndNothing.Create(m_wndParent, rc, !m_strEmptyMsg.empty() ? m_strEmptyMsg.c_str() : NULL, bPrevVisible ? (m_strEmptyMsg.empty() ? WS_VISIBLE|WS_CHILD|SS_GRAYRECT : WS_VISIBLE|WS_CHILD|SS_CENTER|SS_CENTERIMAGE) : (m_strEmptyMsg.empty() ? WS_CHILD|SS_GRAYRECT : WS_CHILD|SS_CENTER|SS_CENTERIMAGE));
		//	m_wndNothing.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		//}
	}
	catch (...)
	{
	}
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::Handle(RWHWND *a_pHandle)
{
	try
	{
		if (m_pSubWnd)
			return m_pSubWnd->Handle(a_pHandle);
		*a_pHandle = m_wndNothing;
		return S_OK;
	}
	catch (...)
	{
		return a_pHandle == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::SendMessage(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
{
	try
	{
		if (m_pSubWnd)
			return m_pSubWnd->SendMessage(a_uMsg, a_wParam, a_lParam);
		m_wndNothing.SendMessage(a_uMsg, a_wParam, a_lParam);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::Show(BOOL a_bShow)
{
	try
	{
		if (m_pSubWnd)
			return m_pSubWnd->Show(a_bShow);
		m_wndNothing.ShowWindow(a_bShow ? SW_SHOW : SW_HIDE);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::Move(RECT const* a_prcPosition)
{
	try
	{
		if (m_pSubWnd)
			return m_pSubWnd->Move(a_prcPosition);
		m_wndNothing.MoveWindow(a_prcPosition);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::Destroy()
{
	try
	{
		if (m_pSubWnd)
		{
			HRESULT hRes = m_pSubWnd->Destroy();
			if (SUCCEEDED(hRes))
				m_pSubWnd = NULL;
			return hRes;
		}
		if (m_wndNothing.m_hWnd)
			m_wndNothing.DestroyWindow();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	return m_pSubWnd ? m_pSubWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::OnIdle()
{
	return m_pSubWnd ? m_pSubWnd->OnIdle() : S_OK;
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::OnDeactivate(BOOL a_bCancelChanges)
{
	return m_pSubWnd ? m_pSubWnd->OnDeactivate(a_bCancelChanges) : S_OK;
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::QueryInterfaces(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
{
	return m_pSubWnd ? m_pSubWnd->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces) : S_OK;
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::OptimumSize(SIZE* a_pSize)
{
	return m_pSubWnd ? m_pSubWnd->OptimumSize(a_pSize) : E_NOTIMPL;
}

STDMETHODIMP CDesignerViewWndSubDocumentFrame::DeactivateAll(BOOL a_bCancelChanges)
{
	//return m_pSubWnd ? m_pSubWnd->DeactivateAll(a_bCancelChanges) : E_NOTIMPL;
	m_wndParent.SendMessage(WM_RW_DEACTIVATE, a_bCancelChanges, 0);
	return S_OK;
}
