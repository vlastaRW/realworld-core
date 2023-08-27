
#pragma once

#include "RWViewAreaLayout.h"
#include <DWAutoHide.h>
#include <ExtDockingWindow.h>
#include <DockingBox.h>
#include <TabDockingBox.h>


class CDockableWindow :
	public dockwins::CBoxedDockingWindowImpl<CDockableWindow, CWindow, dockwins::COutlookLikeBoxedDockingWindowTraits >
{
	typedef dockwins::CBoxedDockingWindowImpl<CDockableWindow, CWindow, dockwins::COutlookLikeBoxedDockingWindowTraits > baseClass;
public:
	struct SInit
	{
		CComPtr<IViewManager> pViewMgr;
		CConfigValue cViewID;
		CComPtr<IConfig> pViewCfg;
		CComPtr<ISharedStateManager> pStateMgr;
		CComPtr<IStatusBarObserver> pStatusBar;
		CComPtr<IDocument> pDoc;
		LCID tLocaleID;
	};
	HWND Create(SInit const& a_sInit, HWND hDockingFrameWnd, RECT& rcPos, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, UINT nID = 0)
	{
		return baseClass::Create(hDockingFrameWnd, rcPos, szWindowName, dwStyle, dwExStyle, nID, const_cast<SInit*>(&a_sInit));
	}

	DECLARE_WND_CLASS(_T("DockableWindow"))

	BEGIN_MSG_MAP(CDockableWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	// handlers
public:
	LRESULT OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		CREATESTRUCT const* pCreateStruct = reinterpret_cast<CREATESTRUCT const*>(a_lParam);
		SInit const& sInit = *reinterpret_cast<SInit const*>(pCreateStruct->lpCreateParams);
		RECT rcInit = {0, 0, pCreateStruct->cx, pCreateStruct->cy};
		sInit.pViewMgr->CreateWnd(sInit.pViewMgr, sInit.cViewID, sInit.pViewCfg, sInit.pStateMgr, sInit.pStatusBar, sInit.pDoc, m_hWnd, &rcInit, EDVWSNoBorder, sInit.tLocaleID, &m_pClient);
		return 0;
	}
	LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (m_pClient != NULL)
		{
            RECT rcWhole = {0, 0, LOWORD(a_lParam), HIWORD(a_lParam)};
			m_pClient->Move(&rcWhole);
		}
		return 0;
	}

	void OnDocked(HDOCKBAR hBar, bool bHorizontal)
	{
		baseClass::OnDocked(hBar, bHorizontal);
	}
	void OnUndocked(HDOCKBAR hBar)
	{
		baseClass::OnUndocked(hBar);
	}

private:
	HWND Create(HWND, RECT&, LPCTSTR, DWORD, DWORD, UINT, LPVOID); // disable standard Create

private:
	CComPtr<IDesignerView> m_pClient;
};
