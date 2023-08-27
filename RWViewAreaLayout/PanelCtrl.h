
#pragma once

#include <ContextMenuWithIcons.h>

class CDesignerViewRolldown;

class CPanelCtrl :
	public CWindowImpl<CPanelCtrl>,
	public CThemeImpl<CPanelCtrl>,
	public CContextMenuWithIcons<CPanelCtrl>
{
public:
	CPanelCtrl() : m_bOpen(true), m_bHot(false), m_bResizing(false), m_bAdjustable(false), m_nSizeY(0), m_hIcon(NULL), m_fScale(1.0f)
	{
		SetThemeClassList(L"ExplorerBar");
	}
	~CPanelCtrl()
	{
		if (m_hIcon)
			DestroyIcon(m_hIcon);
		m_cImageList.Destroy();
	}

	void Init(CDesignerViewRolldown* a_pParent, LCID a_tLocaleID, IConfig* a_pItemCfg, BSTR a_bstrName, int a_nLeft, int a_nRight, int a_nTop, float a_fScale);

	DECLARE_WND_CLASS_EX(_T("RWPanelCtrl"), CS_VREDRAW|CS_HREDRAW, COLOR_WINDOW);

BEGIN_MSG_MAP(CPanelCtrl)
	MESSAGE_HANDLER(WM_NCPAINT, OnNCPaint)
	MESSAGE_HANDLER(WM_NCHITTEST, OnNCHitTest)
	CHAIN_MSG_MAP(CThemeImpl<CPanelCtrl>)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_NCCALCSIZE, OnNCCalcSize)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_RW_GOTFOCUS, OnRWGotFocus)
	MESSAGE_HANDLER(WM_RW_DEACTIVATE, OnRWForward)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CPanelCtrl>)
END_MSG_MAP()

	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetFocus();
		return 0;
	}
	LRESULT OnNCCalcSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnNCPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnNCHitTest(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	HRESULT PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return m_bOpen && m_pView ? m_pView->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
	}
	HRESULT OnIdle()
	{
		return m_pView ? m_pView->OnIdle() : S_FALSE;
	}
	HRESULT OnDeactivate(BOOL a_bCancelChanges)
	{
		return m_pView ? m_pView->OnDeactivate(a_bCancelChanges) : S_FALSE;
	}
	HRESULT QueryInterfaces(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
	{
		return m_pView && (a_eFilter == EQIFAll || m_bOpen) ? m_pView->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces) : S_FALSE;
	}
	void SetFocus()
	{
		RWHWND h = NULL;
		if (m_bOpen && m_pView && SUCCEEDED(m_pView->Handle(&h)) && h)
			::SetFocus(h);
	}
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		RWHWND h = NULL;
		if (m_pView && SUCCEEDED(m_pView->Handle(&h)) && h)
			::SendMessage(h, a_uMsg, a_wParam, a_lParam);
		return 0;
	}

	bool IsOpen() { return m_bOpen; }
	bool UpdateAutoSize();

private:
	void CreateView();
	bool HeaderHitTest(LPARAM a_lParam, bool a_bClient = true);
	void RedrawHeader();

private:
	CDesignerViewRolldown* m_pParent;
	CComPtr<IDesignerView> m_pView;
	CComPtr<IConfig> m_pItemCfg;
	int m_nSizeY;
	bool m_bOpen;
	bool m_bHot;
	bool m_bAdjustable;
	bool m_bResizing;
	int m_nResizeOffset;
	HCURSOR m_hHand;
	HCURSOR m_hArrow;
	HCURSOR m_hSizeNS;
	HICON m_hIcon;
	float m_fScale;
	CImageList m_cImageList;
	LCID m_tLocaleID;
};
