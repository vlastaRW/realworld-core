// DesignerViewSplitter.cpp : Implementation of CDesignerViewSplitter

#include "stdafx.h"
#include "DesignerViewSplitter.h"

#include "ConfigIDsSplitter.h"


// CDesignerViewSplitter

HRESULT CDesignerViewSplitter::Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IViewManager *a_pSubSpec, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, bool a_bBorders, IDocument* a_pDoc, LCID a_tLocaleID)
{
	m_bBorders = a_bBorders;
	m_rcLast = *a_rcWindow;
	if (m_rcLast.left == 0x80000000 && m_rcLast.right == 0 &&
		m_rcLast.top == 0x80000000 && m_rcLast.bottom == 0)
		ZeroMemory(&m_rcLast, sizeof m_rcLast);

	// create self
	if (Create(a_hParent, const_cast<LPRECT>(&m_rcLast), _T("Splitter View"), 0, WS_EX_CONTROLPARENT) == NULL)
	{
		// creation failed
		return E_FAIL; // TODO: error code
	}

	try
	{
		// read config and set members
		CConfigValue cValue;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SPLITTYPE), &cValue);
		m_eSplitType = cValue;

		HDC hdc = GetDC();
		float fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0;
		ReleaseDC(hdc);

		a_pConfig->ItemValueGet(CComBSTR(CFGID_VERDIVTYPE), &cValue);
		if (cValue.operator LONG() == EDTRelativeAdjustable || cValue.operator LONG() == EDTRelativeFixed)
		{
			CConfigValue cValueRel;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_VERRELATIVE), &cValueRel);
			m_tVerticalInfo.Init(a_pConfig, CFGID_VERRELATIVE, m_bBorders, cValue.operator LONG() == EDTRelativeAdjustable, cValueRel);
		}
		else
		{
			CConfigValue cValueAbs;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_VERABSOLUTE), &cValueAbs);
			m_tVerticalInfo.Init(a_pConfig, CFGID_VERABSOLUTE, m_bBorders, cValue.operator LONG() == EDTAbsoluteLTAdjustable || cValue.operator LONG() == EDTAbsoluteRBAdjustable, cValue.operator LONG() == EDTAbsoluteLTFixed || cValue.operator LONG() == EDTAbsoluteLTAdjustable, cValueAbs, fScale);
		}

		a_pConfig->ItemValueGet(CComBSTR(CFGID_HORDIVTYPE), &cValue);
		if (cValue.operator LONG() == EDTRelativeAdjustable || cValue.operator LONG() == EDTRelativeFixed)
		{
			CConfigValue cValueRel;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_HORRELATIVE), &cValueRel);
			m_tHorizontalInfo.Init(a_pConfig, CFGID_HORRELATIVE, m_bBorders, cValue.operator LONG() == EDTRelativeAdjustable, cValueRel);
		}
		else
		{
			CConfigValue cValueAbs;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_HORABSOLUTE), &cValueAbs);
			m_tHorizontalInfo.Init(a_pConfig, CFGID_HORABSOLUTE, m_bBorders, cValue.operator LONG() == EDTAbsoluteLTAdjustable || cValue.operator LONG() == EDTAbsoluteRBAdjustable, cValue.operator LONG() == EDTAbsoluteLTFixed || cValue.operator LONG() == EDTAbsoluteLTAdjustable, cValueAbs, fScale);
		}

		// divide the region
		GetSplittedSizes(m_rcLast, m_rcWnds, m_rcWnds+1, m_rcWnds+2, m_rcWnds+3);

		CComPtr<IConfig> pSubCfg;

		pSubCfg = NULL;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWLT), &cValue);
		a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWLT), &pSubCfg);
		a_pSubSpec->CreateWnd(a_pSubSpec, cValue, pSubCfg, a_pFrame, a_pStatusBar, a_pDoc, m_hWnd, m_rcWnds, m_bBorders ? EDVWSBorder : EDVWSNoBorder, a_tLocaleID, &m_pWndLT);
		m_pLastActive = m_pWndLT;

		if (m_eSplitType == ESTBoth)
		{
			pSubCfg = NULL;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWRT), &cValue);
			a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWRT), &pSubCfg);
			a_pSubSpec->CreateWnd(a_pSubSpec, cValue, pSubCfg, a_pFrame, a_pStatusBar, a_pDoc, m_hWnd, m_rcWnds+2, m_bBorders ? EDVWSBorder : EDVWSNoBorder, a_tLocaleID, &m_pWndRT);

			pSubCfg = NULL;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWLB), &cValue);
			a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWLB), &pSubCfg);
			a_pSubSpec->CreateWnd(a_pSubSpec, cValue, pSubCfg, a_pFrame, a_pStatusBar, a_pDoc, m_hWnd, m_rcWnds+1, m_bBorders ? EDVWSBorder : EDVWSNoBorder, a_tLocaleID, &m_pWndLB);
		}

		pSubCfg = NULL;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SUBVIEWRB), &cValue);
		a_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWRB), &pSubCfg);
		a_pSubSpec->CreateWnd(a_pSubSpec, cValue, pSubCfg, a_pFrame, a_pStatusBar, a_pDoc, m_hWnd, m_rcWnds+3, m_bBorders ? EDVWSBorder : EDVWSNoBorder, a_tLocaleID, &m_pWndRB);
	}
	catch (...)
	{
	}

	return S_OK;
}

STDMETHODIMP CDesignerViewSplitter::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (m_pLastActive)
	{
		if (S_OK == m_pLastActive->PreTranslateMessage(a_pMsg, a_bBeforeAccel))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CDesignerViewSplitter::OnIdle()
{
	if (m_pWndLT != NULL) m_pWndLT->OnIdle();
	if (m_pWndLB != NULL) m_pWndLB->OnIdle();
	if (m_pWndRT != NULL) m_pWndRT->OnIdle();
	if (m_pWndRB != NULL) m_pWndRB->OnIdle();
	RECT rc = {0, 0, m_rcLast.right-m_rcLast.left, m_rcLast.bottom-m_rcLast.top};
	RECT rc2[4];
	ZeroMemory(rc2, sizeof rc2);
	GetSplittedSizes(rc, rc2, rc2+1, rc2+2, rc2+3);
	if (memcmp(m_rcWnds, rc2, sizeof(m_rcWnds)))
	{
		bool bIsVisible = IsWindowVisible();
		if (bIsVisible)
		{
			SetRedraw(FALSE);
		}
		MoveSubWindow(m_pWndLT, m_rcWnds[0] = rc2[0]);
		MoveSubWindow(m_pWndLB, m_rcWnds[1] = rc2[1]);
		MoveSubWindow(m_pWndRT, m_rcWnds[2] = rc2[2]);
		MoveSubWindow(m_pWndRB, m_rcWnds[3] = rc2[3]);
		if (bIsVisible)
		{
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
	return S_OK;
}

STDMETHODIMP CDesignerViewSplitter::OnDeactivate(BOOL a_bCancelChanges)
{
	if (m_pWndLT != NULL) m_pWndLT->OnDeactivate(a_bCancelChanges);
	if (m_pWndLB != NULL) m_pWndLB->OnDeactivate(a_bCancelChanges);
	if (m_pWndRT != NULL) m_pWndRT->OnDeactivate(a_bCancelChanges);
	if (m_pWndRB != NULL) m_pWndRB->OnDeactivate(a_bCancelChanges);
	return S_OK;
}

STDMETHODIMP CDesignerViewSplitter::QueryInterfaces(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
{
	try
	{
		if (m_pLastActive)
			m_pLastActive->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
		if (a_eFilter != EQIFActive)
		{
			if (m_pWndLT && m_pLastActive != m_pWndLT) m_pWndLT->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
			if (m_pWndLB && m_pLastActive != m_pWndLB) m_pWndLB->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
			if (m_pWndRT && m_pLastActive != m_pWndRT) m_pWndRT->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
			if (m_pWndRB && m_pLastActive != m_pWndRB) m_pWndRB->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewSplitter::OptimumSize(SIZE* a_pSize)
{
	try
	{
		if ((m_eSplitType != ESTHorizontal && (m_tVerticalInfo.IsRelative() || m_tVerticalInfo.IsAdjustable())) &&
			(m_eSplitType != ESTVertical && (m_tHorizontalInfo.IsRelative() || m_tHorizontalInfo.IsAdjustable())))
			return E_NOTIMPL;
		RECT rc = {0, 0, a_pSize->cx, a_pSize->cy};
		RECT rcLT;
		RECT rcRT;
		RECT rcLB;
		RECT rcRB;
		GetSplittedSizes(rc, &rcLT, &rcLB, &rcRT, &rcRB);
		//a_pSize->cx = a_pSize->cy = 0;
		SIZE szLT = {rcLT.right-rcLT.left, rcLT.bottom-rcLT.top};
		SIZE szRT = {rcRT.right-rcRT.left, rcRT.bottom-rcRT.top};
		SIZE szLB = {rcLB.right-rcLB.left, rcLB.bottom-rcLB.top};
		SIZE szRB = {rcRB.right-rcRB.left, rcRB.bottom-rcRB.top};
		bool bLT = false;
		bool bRT = false;
		bool bLB = false;
		bool bRB = false;
		if (m_pWndLT) bLT = SUCCEEDED(m_pWndLT->OptimumSize(&szLT));
		if (m_pWndRT) bRT = SUCCEEDED(m_pWndRT->OptimumSize(&szRT));
		if (m_pWndLB) bLB = SUCCEEDED(m_pWndLB->OptimumSize(&szLB));
		if (m_pWndRB) bRB = SUCCEEDED(m_pWndRB->OptimumSize(&szRB));
		if (bRT && szLT.cy < szRT.cy) szLT.cy = szRT.cy;
		if (bLB && szRB.cy < szLB.cy) szRB.cy = szLB.cy;
		if (bLB && szLT.cx < szLB.cx) szLT.cx = szLB.cx;
		if (bRT && szRB.cx < szRT.cx) szRB.cx = szRT.cx;
		int const nSplitter = (m_eSplitType&EDTAdjustableMask) ? (SPLITTER_SIZE + (m_bBorders ? 0 : (BORDER_SIZE<<1))) : SEPARATOR_SIZE;
		switch (m_eSplitType)
		{
		case ESTBoth:
			if (szLT.cx && szRB.cx)
				a_pSize->cx = szLT.cx + szRB.cx + nSplitter;
			if (szLT.cy && szRB.cy)
				a_pSize->cy = szLT.cy + szRB.cy + nSplitter;
			break;
		case ESTVertical:
			a_pSize->cy = max(szLT.cy, szRB.cy);
			if (bLT && bRB)
				a_pSize->cx = szLT.cx + szRB.cx + nSplitter;
			break;
		case ESTHorizontal:
			a_pSize->cx = max(szLT.cx, szRB.cx);
			if (bLT && bRB)
				a_pSize->cy = szLT.cy + szRB.cy + nSplitter;
			break;
		}
		return a_pSize->cx > 0 || a_pSize->cy > 0 ? S_OK : E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDesignerViewSplitter::GetSplittedSizes(const RECT& a_rcWhole, RECT* a_pLT, RECT* a_pLB, RECT* a_pRT, RECT* a_pRB) const
{
	*a_pLT = *a_pLB = *a_pRT = *a_pRB = a_rcWhole;

	if (m_eSplitType == ESTBoth || m_eSplitType == ESTVertical)
	{
		// perform vertical split
		std::pair<int, int> tSizes = m_tVerticalInfo.GetSizes(a_rcWhole.right - a_rcWhole.left, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB);
		a_pLT->right = a_pLT->left + tSizes.first;
		a_pLB->right = a_pLB->left + tSizes.first;
		a_pRT->left = a_pRT->right - tSizes.second;
		a_pRB->left = a_pRB->right - tSizes.second;
	}

	if (m_eSplitType == ESTBoth || m_eSplitType == ESTHorizontal)
	{
		// perform vertical split
		std::pair<int, int> tSizes = m_tHorizontalInfo.GetSizes(a_rcWhole.bottom - a_rcWhole.top, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB);
		a_pLT->bottom = a_pLT->top + tSizes.first;
		a_pRT->bottom = a_pRT->top + tSizes.first;
		a_pLB->top = a_pLB->bottom - tSizes.second;
		a_pRB->top = a_pRB->bottom - tSizes.second;
	}
}

CDesignerViewSplitter::ESplitHitTest CDesignerViewSplitter::SplitHitTest(POINT a_tPos) const
{
	RECT rc;
	GetClientRect(&rc);
	int eRet = ESHTNothing;
	if ((m_eSplitType == ESTBoth || m_eSplitType == ESTHorizontal) && m_tHorizontalInfo.IsAdjustable() && !m_tHorizontalInfo.IsTemporaryLocked(m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB))
	{
		std::pair<int, int> tSizes = m_tHorizontalInfo.GetSizes(rc.bottom, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB);
		if (a_tPos.y >= tSizes.first && a_tPos.y < rc.bottom-tSizes.second)
			eRet |= ESHTHorizontal;
	}
	if ((m_eSplitType == ESTBoth || m_eSplitType == ESTVertical) && m_tVerticalInfo.IsAdjustable() && !m_tVerticalInfo.IsTemporaryLocked(m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB))
	{
		std::pair<int, int> tSizes = m_tVerticalInfo.GetSizes(rc.right, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB);
		if (a_tPos.x >= tSizes.first && a_tPos.x < rc.right-tSizes.second)
			eRet |= ESHTVertical;
	}
	return static_cast<ESplitHitTest>(eRet);
}

void CDesignerViewSplitter::MoveSubWindow(CComPtr<IDesignerView>& a_pWnd, const RECT& a_rcNewPos)
{
	if (a_pWnd != NULL)
	{
		a_pWnd->Move(&a_rcNewPos);
	}
}

LRESULT CDesignerViewSplitter::OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	return 1;
}

LRESULT CDesignerViewSplitter::OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	PAINTSTRUCT tPS;
	BeginPaint(&tPS);
	HDC hDC = tPS.hdc;
	RECT rc;
	GetClientRect(&rc);
	HBRUSH hBgBr = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORDLG, (WPARAM)tPS.hdc);
	if (hBgBr == NULL) hBgBr = (HBRUSH)(COLOR_3DFACE+1);
	if (m_bBorders)
	{
		FillRect(hDC, &rc, hBgBr);
		EndPaint(&tPS);
		return 0;
	}
	static int const SPCOL_LIGHT = COLOR_3DHILIGHT;
	static int const SPCOL_DARK = COLOR_3DSHADOW;
	if (m_eSplitType == ESTHorizontal)
	{
		if (m_rcWnds[0].bottom < m_rcWnds[3].top)
		{
			if (m_tHorizontalInfo.IsAdjustable())
			{
				rc.top = m_rcWnds[0].bottom;
				rc.bottom = rc.top+BORDER_SIZE;
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_LIGHT+1));
				rc.top = rc.bottom;
				rc.bottom = m_rcWnds[3].top-BORDER_SIZE;
				FillRect(hDC, &rc, hBgBr);
				rc.top = rc.bottom;
				rc.bottom += BORDER_SIZE;
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_DARK+1));
			}
			else if (rc.right-rc.left < SPLITTER_SIZE)
			{
				FillRect(hDC, &rc, hBgBr);
			}
			else
			{
				rc.top = m_rcWnds[0].bottom;
				rc.bottom = rc.top+(SEPARATOR_SIZE>>1);
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_DARK+1));
				rc.top = rc.bottom;
				rc.bottom = m_rcWnds[3].top;
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_LIGHT+1));

				//rc.top = m_rcWnds[0].bottom;
				//RECT rc2 = {rc.left, rc.top, rc.left+(SPLITTER_SIZE>>1), rc.top+SPLITTER_SIZE+BORDER_SIZE+BORDER_SIZE};
				//FillRect(hDC, &rc2, hBgBr);
				//rc.left += rc2.right;
				//rc2.right = rc.right; rc2.left = rc.right-(SPLITTER_SIZE>>1);
				//FillRect(hDC, &rc2, hBgBr);
				//rc.right = rc2.left;
				//rc.bottom = rc.top+(SPLITTER_SIZE>>1);
				//FillRect(hDC, &rc, hBgBr);
				//rc.top = rc.bottom;
				//rc.bottom = rc.top+BORDER_SIZE;
				//--rc.right;
				//FillRect(hDC, &rc, (HBRUSH)(SPCOL_DARK+1));
				//SetPixel(hDC, rc.right, rc.top, GetSysColor(SPCOL_LIGHT));
				//++rc.right;
				//rc.top = rc.bottom;
				//rc.bottom += BORDER_SIZE;
				//FillRect(hDC, &rc, (HBRUSH)(SPCOL_LIGHT+1));
				//rc.top = rc.bottom;
				//rc.bottom += SPLITTER_SIZE-(SPLITTER_SIZE>>1);
				//FillRect(hDC, &rc, hBgBr);
			}
		}
	}
	else if (m_eSplitType == ESTVertical)
	{
		if (m_rcWnds[0].right < m_rcWnds[3].left)
		{
			if (m_tVerticalInfo.IsAdjustable())
			{
				rc.left = m_rcWnds[0].right;
				rc.right = rc.left+BORDER_SIZE;
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_LIGHT+1));
				rc.left = rc.right;
				rc.right = m_rcWnds[3].left-BORDER_SIZE;
				FillRect(hDC, &rc, hBgBr);
				rc.left = rc.right;
				rc.right += BORDER_SIZE;
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_DARK+1));
			}
			else if (rc.bottom-rc.top < SPLITTER_SIZE)
			{
				FillRect(hDC, &rc, hBgBr);
			}
			else
			{
				rc.left = m_rcWnds[0].right;
				rc.right = rc.left+(SEPARATOR_SIZE>>1);
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_DARK+1));
				rc.left = rc.right;
				rc.right = m_rcWnds[3].left;
				FillRect(hDC, &rc, (HBRUSH)(SPCOL_LIGHT+1));

				//rc.left = m_rcWnds[0].right;
				//RECT rc2 = {rc.left, rc.top, rc.left+SPLITTER_SIZE+BORDER_SIZE+BORDER_SIZE, rc.top+(SPLITTER_SIZE>>1)};
				//FillRect(hDC, &rc2, hBgBr);
				//rc.top += rc2.bottom;
				//rc2.bottom = rc.bottom; rc2.top = rc.bottom-(SPLITTER_SIZE>>1);
				//FillRect(hDC, &rc2, hBgBr);
				//rc.bottom = rc2.top;
				//rc.right = rc.left+(SPLITTER_SIZE>>1);
				//FillRect(hDC, &rc, hBgBr);
				//rc.left = rc.right;
				//rc.right = rc.left+BORDER_SIZE;
				//--rc.bottom;
				//FillRect(hDC, &rc, (HBRUSH)(SPCOL_DARK+1));
				//SetPixel(hDC, rc.left, rc.bottom, GetSysColor(SPCOL_LIGHT));
				//++rc.bottom;
				//rc.left = rc.right;
				//rc.right += BORDER_SIZE;
				//FillRect(hDC, &rc, (HBRUSH)(SPCOL_LIGHT+1));
				//rc.left = rc.right;
				//rc.right += SPLITTER_SIZE-(SPLITTER_SIZE>>1);
				//FillRect(hDC, &rc, hBgBr);
			}
		}
	}
	else
	{
		std::pair<int, int> tSizesH = m_tHorizontalInfo.GetSizes(rc.bottom, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB);
		std::pair<int, int> tSizesV = m_tVerticalInfo.GetSizes(rc.right, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB);
		int nY = tSizesH.first;
		int nX = tSizesV.first;
		RECT rc1 = {nX, rc.top, nX+BORDER_SIZE, nY+BORDER_SIZE};
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_LIGHT+1));
		rc1.left = rc1.right;
		rc1.right += SPLITTER_SIZE;
		FillRect(hDC, &rc1, hBgBr);
		rc1.left = rc1.right;
		rc1.right += BORDER_SIZE;
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_DARK+1));

		rc1.left = rc.left;
		rc1.top = nY;
		rc1.right = nX;
		rc1.bottom = rc1.top+BORDER_SIZE;
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_LIGHT+1));
		rc1.left = nX+(BORDER_SIZE<<1)+SPLITTER_SIZE;
		rc1.right = rc.right;
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_LIGHT+1));
		rc1.top = rc1.bottom;
		rc1.bottom += SPLITTER_SIZE;
		rc1.left = rc.left;
		FillRect(hDC, &rc1, hBgBr);
		rc1.top = rc1.bottom;
		rc1.right = nX;
		rc1.bottom += BORDER_SIZE;
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_DARK+1));
		rc1.left = nX+(BORDER_SIZE<<1)+SPLITTER_SIZE;
		rc1.right = rc.right;
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_DARK+1));

		rc1.left = nX;
		rc1.top = nY+BORDER_SIZE+SPLITTER_SIZE;
		rc1.right = nX+BORDER_SIZE;
		rc1.bottom = rc.bottom;
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_LIGHT+1));
		rc1.left = rc1.right;
		rc1.right += SPLITTER_SIZE;
		FillRect(hDC, &rc1, hBgBr);
		rc1.left = rc1.right;
		rc1.right += BORDER_SIZE;
		FillRect(hDC, &rc1, (HBRUSH)(SPCOL_DARK+1));
	}
	EndPaint(&tPS);
	return 0;
}

STDMETHODIMP CDesignerViewSplitter::Move(RECT const* a_prcPosition)
{
	if (m_hWnd == NULL)
		return S_FALSE;
	if (a_prcPosition->left == m_rcLast.left && a_prcPosition->right == m_rcLast.right &&
		a_prcPosition->top == m_rcLast.top && a_prcPosition->bottom == m_rcLast.bottom)
		return S_OK; // no change;
	RECT rc2 = {0, 0, a_prcPosition->right-a_prcPosition->left, a_prcPosition->bottom-a_prcPosition->top};
	GetSplittedSizes(rc2, m_rcWnds, m_rcWnds+1, m_rcWnds+2, m_rcWnds+3);
	m_rcLast = *a_prcPosition;

	bool bIsVisible = IsWindowVisible();
	if (bIsVisible)
	{
		SetRedraw(FALSE);
	}
	MoveSubWindow(m_pWndLT, m_rcWnds[0]);
	MoveSubWindow(m_pWndLB, m_rcWnds[1]);
	MoveSubWindow(m_pWndRT, m_rcWnds[2]);
	MoveSubWindow(m_pWndRB, m_rcWnds[3]);
	MoveWindow(a_prcPosition);
	if (bIsVisible)
	{
		SetRedraw(TRUE);
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	return S_OK;
}

LRESULT CDesignerViewSplitter::OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (LOWORD(a_lParam) != m_rcLast.right-m_rcLast.left || HIWORD(a_lParam) != m_rcLast.bottom-m_rcLast.top)
	{
		RECT rcSelf = {0, 0, LOWORD(a_lParam), HIWORD(a_lParam)};
		ClientToScreen(&rcSelf);
		GetParent().ScreenToClient(&rcSelf);
		Move(&rcSelf);
	}
	return 0;
}

LRESULT CDesignerViewSplitter::OnSetCursor(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	if ((!m_tHorizontalInfo.IsAdjustable() && !m_tVerticalInfo.IsAdjustable()) ||
		reinterpret_cast<HWND>(a_wParam) != m_hWnd)
	{
		a_bHandled = FALSE;
		return FALSE;
	}

	POINT tPos;
	GetCursorPos(&tPos);
	ScreenToClient(&tPos);
	switch (SplitHitTest(tPos))
	{
	case ESHTBoth:
		if (m_hBoth == NULL)
		{
			m_hBoth = LoadCursor(NULL, IDC_SIZEALL);
		}
		SetCursor(m_hBoth);
		return TRUE;
	case ESHTHorizontal:
		if (m_hHorizontal == NULL)
		{
			m_hHorizontal = LoadCursor(NULL, IDC_SIZENS);
		}
		SetCursor(m_hHorizontal);
		return TRUE;
	case ESHTVertical:
		if (m_hVertical == NULL)
		{
			m_hVertical = LoadCursor(NULL, IDC_SIZEWE);
		}
		SetCursor(m_hVertical);
		return TRUE;
	default:
		a_bHandled = FALSE;
		return FALSE;
	}
}

LRESULT CDesignerViewSplitter::OnLButtonDown(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	SetCapture();

	m_tStartMousePos.x = GET_X_LPARAM(a_lParam);
	m_tStartMousePos.y = GET_Y_LPARAM(a_lParam);
	m_tLastMousePos = m_tStartMousePos;
	m_eActualDragType = SplitHitTest(m_tStartMousePos);
	RECT rcWhole;
	GetClientRect(&rcWhole);
	m_tStartSize.x = m_tHorizontalInfo.GetSizes(rcWhole.bottom, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB).first;
	m_tStartSize.y = m_tVerticalInfo.GetSizes(rcWhole.right, m_pWndLT, m_pWndLB, m_pWndRT, m_pWndRB).first;

	return 0;
}

LRESULT CDesignerViewSplitter::OnLButtonUp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	ReleaseCapture();

	m_eActualDragType = ESHTNothing;

	return 0;
}

LRESULT CDesignerViewSplitter::OnMouseMove(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (m_eActualDragType != ESHTNothing)
	{
		RECT rcWhole;
		GetClientRect(&rcWhole);

		bool bChanged = false;
		POINT tMousePos = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		if (m_eActualDragType & ESHTHorizontal)
		{
			bChanged = m_tHorizontalInfo.SetFirst(rcWhole.bottom, m_tStartSize.x + tMousePos.y - m_tStartMousePos.y) || bChanged;
		}
		if (m_eActualDragType & ESHTVertical)
		{
			bChanged = m_tVerticalInfo.SetFirst(rcWhole.right, m_tStartSize.y + tMousePos.x - m_tStartMousePos.x) || bChanged;
		}

		if (bChanged || m_tLastMousePos.x != tMousePos.x || m_tLastMousePos.y != tMousePos.y)
		{
			GetSplittedSizes(rcWhole, m_rcWnds, m_rcWnds+1, m_rcWnds+2, m_rcWnds+3);

			SetRedraw(FALSE);
			MoveSubWindow(m_pWndLT, m_rcWnds[0]);
			MoveSubWindow(m_pWndLB, m_rcWnds[1]);
			MoveSubWindow(m_pWndRT, m_rcWnds[2]);
			MoveSubWindow(m_pWndRB, m_rcWnds[3]);
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

			m_tLastMousePos = tMousePos;
		}
	}

	return 0;
}

LRESULT CDesignerViewSplitter::OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	RWHWND hWnd = NULL;
	if (m_pLastActive && SUCCEEDED(m_pLastActive->Handle(&hWnd)) && hWnd != NULL)
	{
		::SetFocus(reinterpret_cast<HWND>(hWnd));
	}

	return 0;
}

LRESULT CDesignerViewSplitter::OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	LRESULT lRet = DefWindowProc(a_uMsg, a_wParam, a_lParam);

	if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
	{
		DWORD dwPos = ::GetMessagePos();
		POINT pt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
		ScreenToClient(&pt);

		struct {IDesignerView* pWnd; RECT* prcPane;} aViews[] =
		{
			{m_pWndLT, m_rcWnds},
			{m_pWndRB, m_rcWnds+3},
			{m_pWndRT, m_rcWnds+2},
			{m_pWndLB, m_rcWnds+1}
		};
		int i;
		for (i = 0; i < (sizeof(aViews)/sizeof(aViews[0])); i++)
		{
			RWHWND hWnd = NULL;
			if (aViews[i].pWnd && ::PtInRect(aViews[i].prcPane, pt) &&
				SUCCEEDED(aViews[i].pWnd->Handle(&hWnd)) && hWnd != NULL)
			{
				m_pLastActive = aViews[i].pWnd;
				//::SetFocus(reinterpret_cast<HWND>(hWnd));
				break;
			}
		}
	}

	return lRet;
}

LRESULT CDesignerViewSplitter::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	RWHWND h;
	h = NULL; if (m_pWndLT != NULL) m_pWndLT->Handle(&h); if (h) ::SendMessage(h, a_uMsg, a_wParam, a_lParam);
	h = NULL; if (m_pWndLB != NULL) m_pWndLB->Handle(&h); if (h) ::SendMessage(h, a_uMsg, a_wParam, a_lParam);
	h = NULL; if (m_pWndRT != NULL) m_pWndRT->Handle(&h); if (h) ::SendMessage(h, a_uMsg, a_wParam, a_lParam);
	h = NULL; if (m_pWndRB != NULL) m_pWndRB->Handle(&h); if (h) ::SendMessage(h, a_uMsg, a_wParam, a_lParam);
	return 0;
}

LRESULT CDesignerViewSplitter::OnRWGotFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	IDesignerView* p = reinterpret_cast<IDesignerView*>(a_lParam);
	//if (p && (m_pWndLT == p || m_pWndLB == p || m_pWndRT == p || m_pWndRB == p))
	//{
	//	m_pLastActive = p;
	//	GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
	//}
	HWND h = reinterpret_cast<HWND>(a_wParam);
	if (h && p)
	{
		HWND hLT = NULL, hLB = NULL, hRT = NULL, hRB = NULL;
		if (m_pWndLT && (p == m_pWndLT || (SUCCEEDED(m_pWndLT->Handle(&hLT)) && h == hLT)))
		{
			m_pLastActive = m_pWndLT;
			GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
			return 0;
		}
		if (m_pWndLB && (p == m_pWndLB || ( SUCCEEDED(m_pWndLB->Handle(&hLB)) && h == hLB)))
		{
			m_pLastActive = m_pWndLB;
			GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
			return 0;
		}
		if (m_pWndRT && (p == m_pWndRT || ( SUCCEEDED(m_pWndRT->Handle(&hRT)) && h == hRT)))
		{
			m_pLastActive = m_pWndRT;
			GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
			return 0;
		}
		if (m_pWndRB && (p == m_pWndRB || ( SUCCEEDED(m_pWndRB->Handle(&hRB)) && h == hRB)))
		{
			m_pLastActive = m_pWndRB;
			GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
			return 0;
		}
	}
	GetParent().SendMessage(WM_RW_GOTFOCUS);
	return 0;
}

LRESULT CDesignerViewSplitter::OnRWForward(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
}

