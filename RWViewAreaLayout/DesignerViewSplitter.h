// DesignerViewSplitter.h : Declaration of the CDesignerViewSplitter

#pragma once
#include "resource.h"       // main symbols
#include "RWViewAreaLayout.h"

#include "ConfigIDsSplitter.h"


// CDesignerViewSplitter

class ATL_NO_VTABLE CDesignerViewSplitter : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewSplitter>,
	public CChildWindowImpl<CDesignerViewSplitter, IDesignerView>
{
public:
	CDesignerViewSplitter() :
		m_eSplitType(ESTHorizontal), m_eActualDragType(ESHTNothing),
		m_hHorizontal(NULL), m_hVertical(NULL), m_hBoth(NULL), m_pLastActive(NULL)
	{
		ZeroMemory(&m_rcLast, sizeof m_rcLast);
		ZeroMemory(m_rcWnds, sizeof m_rcWnds);
	}

	enum {SPLITTER_SIZE = 4, SEPARATOR_SIZE = 2, BORDER_SIZE = 1};

	DECLARE_WND_CLASS_EX(_T("DesignerViewSplitterWndClass"), CS_OWNDC|CS_VREDRAW|CS_HREDRAW, COLOR_BTNFACE);

BEGIN_MSG_MAP(CDesignerViewSplitter)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	MESSAGE_HANDLER(WM_RW_GOTFOCUS, OnRWGotFocus)
	MESSAGE_HANDLER(WM_RW_DEACTIVATE, OnRWForward)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnRWForward)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnRWForward)
END_MSG_MAP()


BEGIN_COM_MAP(CDesignerViewSplitter)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// handlers
public:
	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);

	// internal methods
public:
	HRESULT Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pSubSpec, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, bool a_bBorders, IDocument* a_pDoc, LCID a_tLocaleID);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);
	STDMETHOD(Move)(RECT const* a_prcPosition);

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

private:
	template<bool t_bVertical>
	struct TDivisionInfo
	{
		TDivisionInfo() {}
		void Init(IConfig* a_pCfg, LPCOLESTR a_pszID, bool a_bBorders, bool a_bAdjustable, float a_fRelative)
		{
			m_pCfg = a_pCfg;
			m_bstrID = a_pszID;
			eType = a_bAdjustable ? EDTRelativeAdjustable : EDTRelativeFixed;
			m_bBorders = a_bBorders;
			fRelative = a_fRelative;
		}
		void Init(IConfig* a_pCfg, LPCOLESTR a_pszID, bool a_bBorders, bool a_bAdjustable, bool a_bTopLeft, float a_fAbsolute, float a_fScale)
		{
			m_pCfg = a_pCfg;
			m_bstrID = a_pszID;
			eType = a_bAdjustable ? (a_bTopLeft ? EDTAbsoluteLTAdjustable : EDTAbsoluteRBAdjustable) : (a_bTopLeft ? EDTAbsoluteLTFixed : EDTAbsoluteRBFixed);
			m_bBorders = a_bBorders;
			fAbsolute = a_fAbsolute;
			fScale = a_fScale;
		}
		std::pair<int,int> GetSizes(int a_nWhole, IDesignerView* a_pWndLT, IDesignerView* a_pWndLB, IDesignerView* a_pWndRT, IDesignerView* a_pWndRB) const
		{
			IDesignerView* const pWindow1 = t_bVertical ? a_pWndLB : a_pWndRT;
			IDesignerView* const pWindow2 = t_bVertical ? a_pWndRT : a_pWndLB;

			int const nSplitter = (eType&EDTAdjustableMask) ? SPLITTER_SIZE + (m_bBorders ? 0 : (BORDER_SIZE<<1)) : SEPARATOR_SIZE;

			if (eType <= EDTRelativeAdjustable)
			{
				if (a_nWhole <= nSplitter)
					return std::make_pair(0, 0);
				int i1 = static_cast<int>((a_nWhole-nSplitter)*fRelative/100.0f + 0.5f);
				return std::make_pair(i1, a_nWhole-nSplitter-i1);
			}

			if (eType == EDTAbsoluteLTFixed || eType == EDTAbsoluteLTAdjustable)
			{
				//int n = -1;
				int n = static_cast<int>(fAbsolute*fScale+0.5f);
				SIZE tSize = {t_bVertical ? n : -1, t_bVertical ? -1 : n};
				if (a_pWndLT && SUCCEEDED(a_pWndLT->OptimumSize(&tSize)))
					n = t_bVertical ? tSize.cx : tSize.cy;
				if (pWindow1 && SUCCEEDED(pWindow1->OptimumSize(&tSize)))
					n = t_bVertical ? tSize.cx : tSize.cy;
				if (n == 0)
					return std::make_pair(0, a_nWhole);
				if (n > 0 /*&& fAbsolute == 0.0f*/)
					return std::make_pair(n, a_nWhole-nSplitter-n);
			}
			else
			{
				//int n = -1;
				int n = /*a_nWhole-nSplitter-*/static_cast<int>(fAbsolute*fScale+0.5f);
				SIZE tSize = {t_bVertical ? n : -1, t_bVertical ? -1 : n};
				if (a_pWndRB && SUCCEEDED(a_pWndRB->OptimumSize(&tSize)))
					n = t_bVertical ? tSize.cx : tSize.cy;
				if (pWindow2 && SUCCEEDED(pWindow2->OptimumSize(&tSize)))
					n = t_bVertical ? tSize.cx : tSize.cy;
				if (n == 0)
					return std::make_pair(a_nWhole, 0);
				if (n > 0 /*&& fAbsolute == 0.0f*/)
					return std::make_pair(a_nWhole-nSplitter-n, n);
			}
			if (fAbsolute == 0.0f && !IsAdjustable())
			{
				int const n = (a_nWhole-nSplitter)>>1;
				return std::make_pair(n, a_nWhole-nSplitter-n);
			}

			int const n = eType <= EDTAbsoluteLTAdjustable ? static_cast<int>(fAbsolute*fScale+0.5f) : a_nWhole-nSplitter-static_cast<int>(fAbsolute*fScale+0.5f);
			return std::make_pair(n, a_nWhole-nSplitter-n);
		}
		bool SetFirst(int a_nWhole, int a_nFirst)
		{
			a_nWhole -= (eType&EDTAdjustableMask) ? SPLITTER_SIZE + (m_bBorders ? 0 : (BORDER_SIZE<<1)) : SEPARATOR_SIZE;
			if (eType <= EDTRelativeAdjustable)
			{
				fRelative = a_nFirst*100.0f/a_nWhole;
				if (m_pCfg)
				{
					CConfigValue cVal(fRelative);
					return SetValue(cVal);
				}
			}
			else if (eType <= EDTAbsoluteLTAdjustable)
			{
				fAbsolute = a_nFirst/fScale;
				if (m_pCfg)
				{
					CConfigValue cVal(fAbsolute);
					return SetValue(cVal);
				}
			}
			else
			{
				fAbsolute = (a_nWhole-a_nFirst)/fScale;
				if (m_pCfg)
				{
					CConfigValue cVal(fAbsolute);
					return SetValue(cVal);
				}
			}
			return false;
		}
		bool IsAdjustable() const
		{
			return eType == EDTRelativeAdjustable || eType == EDTAbsoluteLTAdjustable || eType == EDTAbsoluteRBAdjustable;
		}
		bool IsRelative() const
		{
			return eType == EDTRelativeAdjustable || eType == EDTRelativeFixed;
		}
		bool IsTemporaryLocked(IDesignerView* a_pWndLT, IDesignerView* a_pWndLB, IDesignerView* a_pWndRT, IDesignerView* a_pWndRB) const
		{
			ULONG i = 0;
			ULONG n = 0;
			CComQIPtr<IDesignerViewTabControl> pTab0(a_pWndLT);
			if (pTab0 && SUCCEEDED(pTab0->ActiveIndexGet(&i)) && SUCCEEDED(pTab0->ItemCount(&n)) && i >= n)
				return true;
			CComQIPtr<IDesignerViewTabControl> pTab3(a_pWndRB);
			if (pTab3 && SUCCEEDED(pTab3->ActiveIndexGet(&i)) && SUCCEEDED(pTab3->ItemCount(&n)) && i >= n)
				return true;
			CComQIPtr<IDesignerViewTabControl> pTab1(a_pWndLB);
			if (pTab1 && SUCCEEDED(pTab1->ActiveIndexGet(&i)) && SUCCEEDED(pTab1->ItemCount(&n)) && i >= n)
				return true;
			CComQIPtr<IDesignerViewTabControl> pTab2(a_pWndRT);
			if (pTab2 && SUCCEEDED(pTab2->ActiveIndexGet(&i)) && SUCCEEDED(pTab2->ItemCount(&n)) && i >= n)
				return true;
			return false;
		}

	private:
		bool SetValue(CConfigValue& a_cNew)
		{
			CConfigValue cOld;
			m_pCfg->ItemValueGet(m_bstrID, &cOld);
			if (cOld != a_cNew)
			{
				m_pCfg->ItemValuesSet(1, &(m_bstrID.m_str), a_cNew);
				return true;
			}
			return false;
		}
	private:
		LONG eType;
		bool m_bBorders;
		float fRelative;
		float fAbsolute;
		float fScale;
		CComPtr<IConfig> m_pCfg;
		CComBSTR m_bstrID;
	};

private:
	enum ESplitHitTest
	{
		ESHTNothing = 0,
		ESHTVertical = 1,
		ESHTHorizontal = 2,
		ESHTBoth = 3,
	};
	void GetSplittedSizes(const RECT& a_rcWhole, RECT* a_pLT, RECT* a_pLB, RECT* a_pRT, RECT* a_pRB) const;
	ESplitHitTest SplitHitTest(POINT a_tPos) const;
	static void MoveSubWindow(CComPtr<IDesignerView>& a_pWnd, const RECT& a_rcNewPos);

private:
	RECT m_rcLast;
	CComPtr<IDesignerView> m_pWndLT; // left/top
	CComPtr<IDesignerView> m_pWndLB; // left/bottom
	CComPtr<IDesignerView> m_pWndRT; // right/top
	CComPtr<IDesignerView> m_pWndRB; // right/bottom
	RECT m_rcWnds[4];
	IDesignerView* m_pLastActive;

	LONG m_eSplitType;
	TDivisionInfo<false> m_tHorizontalInfo;
	TDivisionInfo<true> m_tVerticalInfo;

	HCURSOR m_hHorizontal;
	HCURSOR m_hVertical;
	HCURSOR m_hBoth;
	POINT m_tStartMousePos;
	POINT m_tStartSize;
	POINT m_tLastMousePos;
	ESplitHitTest m_eActualDragType;
	bool m_bBorders;
};

