// ColorPicker.cpp : Implementation of CColorPicker

#include "stdafx.h"
#include "ColorPicker.h"
#include <XPGUI.h>
#define COLORPICKER_NOGRADIENT
#include <RWConceptSharedState.h>
#include <RWConceptDesignerView.h>
#include <WTL_ColorPicker.h>
#include <IconRenderer.h>


// CColorPicker

STDMETHODIMP CColorPicker::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (a_bBeforeAccel && m_hWnd && a_pMsg->hwnd)
	{
		if (m_wndARGB.m_hWnd == a_pMsg->hwnd ||
			m_aEdits[0].first.m_hWnd == a_pMsg->hwnd || m_aEdits[1].first.m_hWnd == a_pMsg->hwnd ||
			m_aEdits[2].first.m_hWnd == a_pMsg->hwnd || m_aEdits[3].first.m_hWnd == a_pMsg->hwnd)
		{
			if (IsDialogMessage(const_cast<LPMSG>(a_pMsg)))
				return S_OK;
			TranslateMessage(a_pMsg);
			DispatchMessage(a_pMsg);
			return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CColorPicker::ColorGet(TColor* a_pColor)
{
	try
	{
		*a_pColor = m_tColor;
		return S_OK;
	}
	catch (...)
	{
		return a_pColor ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CColorPicker::ColorSet(TColor const* a_pColor)
{
	try
	{
		if (a_pColor->fR == m_tColor.fR && a_pColor->fG == m_tColor.fG && a_pColor->fB == m_tColor.fB && a_pColor->fA == m_tColor.fA)
			return S_FALSE;
		m_tColor = *a_pColor;
		if (m_hWnd)
		{
			m_bEnableUpdates = false;
			Data2GUI(EUSCOM);
			m_bEnableUpdates = true;
		}
		return S_OK;
	}
	catch (...)
	{
		return a_pColor ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CColorPicker::Create(RWHWND a_hParent, RECT const* a_pPosition, BOOL a_bVisible, LCID a_tLocaleID, ILocalizedString* a_pName, BYTE a_bImportant, BSTR a_bstrContext, BSTR a_bstrLayout)
{
	if (m_hWnd)
		return E_FAIL;

	CComPtr<IGlobalConfigManager> pMgr;
	RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
	static GUID const tColorGCID = {0x2CBE06C7, 0x4847, 0x4766, {0xAA, 0x01, 0x22, 0x6A, 0xF5, 0x2D, 0x54, 0x88}}; // CLSID_DesignerViewFactoryColorSwatch
	pMgr->Config(tColorGCID, &m_pColorsCfg);
	//pMgr->Config(CLSID_DesignerViewFactoryColorSwatch, &m_pColorsCfg);

	if (a_bstrContext && *a_bstrContext)
	{
		m_bstrContext = L"[";
		m_bstrContext += a_bstrContext;
		m_bstrContext += L"]";
	}

	m_pName = a_pName;

	m_pGT.Attach(new CGammaTables());
	m_wndAlpha.InitGamma(m_pGT);
	m_wndWheel.InitGamma(m_pGT);
	Win32LangEx::CLangIndirectDialogImpl<CColorPicker>::Create(a_hParent);
	if (m_hWnd == NULL)
		return E_FAIL;
	if (a_pPosition)
		SetWindowPos(NULL, a_pPosition->left, a_pPosition->top, a_pPosition->right-a_pPosition->left, a_pPosition->bottom-a_pPosition->top, SWP_NOZORDER);
	if (a_bVisible)
		ShowWindow(SW_SHOW);
	return S_OK;
}

STDMETHODIMP CColorPicker::Layout(BSTR* a_pbstrLayout)
{
	if (a_pbstrLayout == NULL)
		return E_POINTER;
	*a_pbstrLayout = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP CColorPicker::OptimumSize(SIZE* a_pSize)
{
	if (m_hWnd)
	{
		a_pSize->cy = m_bExpanded ? m_nExpandedSize : m_nCollapsedSize;
	}
	else
	{
		GetDialogSize(a_pSize, m_tLocaleID);
	}
	return S_OK;
}

LRESULT CColorPicker::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	AddRef();

	m_rcGaps.left = m_rcGaps.top = 4;
	m_rcGaps.right = m_rcGaps.bottom = 7;
	MapDialogRect(&m_rcGaps);

	m_wndToolBar = GetDlgItem(IDC_COMMANDS);
	m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	m_nIconSize = XPGUI::GetSmallIconSize()*3/4;
	m_cImageList.Create(m_nIconSize, m_nIconSize, XPGUI::GetImageListColorFlags(), 4, 0);
	IROutlinedFill material(pSI->GetMaterial(ESMInterior, true), pSI->GetMaterial(ESMContrast), 1.333f/40.0f, 1.333f*0.225f);
	{
		static IRPolyPoint const pts[] =
		{
			{0, 24}, {256, 24}, {164, 116}, {256, 116}, {128, 244}, {0, 116}, {92, 116},
		};
		static IRGridItem const grid[] = { {0, 24}, {0, 116} };
		static IRCanvas const canvas = {0, 0, 256, 256, 0, itemsof(grid), NULL, grid};
		CIconRendererReceiver cRenderer(m_nIconSize);
		cRenderer(&canvas, itemsof(pts), pts, &material, IRTarget(0.8f));
		HICON hIc = cRenderer.get();
		m_cImageList.AddIcon(hIc);
		DestroyIcon(hIc);
	}
	{
		static IRPolyPoint const pts[] =
		{
			{0, 232}, {256, 232}, {164, 140}, {256, 140}, {128, 12}, {0, 140}, {92, 140},
		};
		static IRGridItem const grid[] = { {0, 140}, {0, 232} };
		static IRCanvas const canvas = {0, 0, 256, 256, 0, itemsof(grid), NULL, grid};
		CIconRendererReceiver cRenderer(m_nIconSize);
		cRenderer(&canvas, itemsof(pts), pts, &material, IRTarget(0.8f));
		HICON hIc = cRenderer.get();
		m_cImageList.AddIcon(hIc);
		DestroyIcon(hIc);
	}
	{
		static IRPathPoint const top[] =
		{
			{178, 18, 18.1904, -30.3169, -25, 41.6665},
			{238, 18, 16.6665, 16.6667, -16.6665, -16.6667},
			{238, 78, -41.6665, 25, 30.3171, -18.1902},
			{158, 98, -33.3335, -33.3335, 33.3335, 33.3335},
		};
		static IRPathPoint const bot[] =
		{
			{0, 256, 0, 0, 0, 0},
			{40, 176, 29.4626, -29.4629, -26.3523, 26.3525},
			{140, 76, 7, -19, -11.7852, 11.7852},
			{180, 116, -11.7852, 11.7849, 20, -8},
			{80, 216, -26.3523, 26.3525, 29.4626, -29.4629},
		};
		static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
		CIconRendererReceiver cRenderer(m_nIconSize);
		cRenderer(&canvas, itemsof(bot), bot, &material);
		cRenderer(&canvas, itemsof(top), top, &material);
		HICON hIc = cRenderer.get();
		m_cImageList.AddIcon(hIc);
		DestroyIcon(hIc);
	}
	HBITMAP hBmp = CreateColorBitmap();
	m_cImageList.Add(hBmp);
	DeleteObject(hBmp);

	CComBSTR bstrExpand;
	CMultiLanguageString::GetLocalized(L"[0409]More options[0405]Více možností", m_tLocaleID, &bstrExpand);
	CComBSTR bstrCollapse;
	CMultiLanguageString::GetLocalized(L"[0409]Less options[0405]Méně možností", m_tLocaleID, &bstrCollapse);
	CComBSTR bstrDropper;
	CMultiLanguageString::GetLocalized(L"[0409]From screen pixel[0405]Podle bodu obrazovky", m_tLocaleID, &bstrDropper);
	m_wndToolBar.SetImageList(m_cImageList);
	TBBUTTON aButtons[] =
	{
		{2, ID_DROPPER, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrDropper.m_str)},
		{1, ID_COLLAPSE, TBSTATE_ENABLED|TBSTATE_HIDDEN, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrCollapse.m_str)},
		{0, ID_EXPAND, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrExpand.m_str)},
	};
	m_wndToolBar.AddButtons(3, aButtons);
	if (CTheme::IsThemingSupported() && IsAppThemed())
	{
		int nButtonSize = m_nIconSize*1.625f + 0.5f;
		m_wndToolBar.SetButtonSize(nButtonSize, nButtonSize);
	}
	RECT rcTB;
	m_wndToolBar.GetWindowRect(&rcTB);
	ScreenToClient(&rcTB);
	RECT rcItem;
	m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcItem);
	m_nCollapsedSize = rcItem.bottom;
	RECT rcWindow;
	GetClientRect(&rcWindow);
	m_wndToolBar.SetWindowPos(NULL, rcTB.right-rcItem.right, 0, rcItem.right, m_nCollapsedSize, SWP_NOZORDER);
	m_tToolbarOffsets.cx = rcWindow.right-rcTB.right+rcItem.right;
	m_tToolbarOffsets.cy = 0;

	m_wndActive = GetDlgItem(IDC_ACTIVE);
	m_wndActive.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndActive.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_wndActive.SetImageList(m_cImageList);
	CComBSTR bstrTip;
	if (m_pName) m_pName->GetLocalized(m_tLocaleID, &bstrTip);
	if (bstrTip.m_str == NULL || bstrTip.m_str[0] == L'\0')
		CMultiLanguageString::GetLocalized(L"[0409]Current color[0405]Aktuální barva", m_tLocaleID, &bstrTip);
	TBBUTTON tActive = {3, ID_ACTIVE, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrTip.m_str)};
	m_wndActive.AddButtons(1, &tActive);
	if (CTheme::IsThemingSupported() && IsAppThemed())
	{
		int nButtonSize = m_nIconSize*1.625f + 0.5f;
		m_wndActive.SetButtonSize(nButtonSize, nButtonSize);
	}
	RECT rcActive;
	m_wndActive.GetItemRect(0, &rcActive);
	m_wndActive.SetWindowPos(NULL, 0, 0, rcActive.right, m_nCollapsedSize, SWP_NOZORDER);

	RECT rcHistory = {rcActive.right, 0, rcTB.right-rcItem.right-m_rcGaps.left, m_nCollapsedSize};
	m_wndHistory.Init(m_hWnd, &rcHistory, m_tLocaleID, IDC_HISTORY);
	m_tHistoryOffsets.cx = rcWindow.right-(rcHistory.right-rcHistory.left);
	m_tHistoryOffsets.cy = m_nCollapsedSize;

	m_bEnableUpdates = false;
	Data2GUI(EUSUnknown);
	m_bEnableUpdates = true;

	if (m_pColorsCfg && m_bstrContext.m_str)
	{
		CConfigValue cState;
		m_pColorsCfg->ItemValueGet(CComBSTR(L"Expanded"), &cState);
		if (wcsstr(cState, m_bstrContext))
		{
			BOOL b;
			OnExpand(0, 0, 0, b);
		}
	}
	return TRUE;
}

LRESULT CColorPicker::OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	BOOL bVisible = IsWindowVisible();
	if (bVisible)
		SetRedraw(FALSE);

	m_wndToolBar.SetWindowPos(NULL, GET_X_LPARAM(a_lParam)-m_tToolbarOffsets.cx, m_tToolbarOffsets.cy, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	m_wndHistory.SetWindowPos(NULL, 0, 0, GET_X_LPARAM(a_lParam)-m_tHistoryOffsets.cx, m_tHistoryOffsets.cy, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	if (m_wndWheel.m_hWnd)
	{
		RECT rc = {0, m_nCollapsedSize+m_rcGaps.top, GET_X_LPARAM(a_lParam)-m_rcEditSize.right-m_rcGaps.right, m_rcEditSize.bottom*4+m_rcGaps.top*5+m_nCollapsedSize*2};
		RECT rcWheel;
		RECT rcAlpha;
		bool bVertical = true;
		SplitArea(rc, &rcWheel, &rcAlpha, &bVertical);
		m_wndWheel.SetWindowPos(NULL, 0, 0, rcWheel.right-rcWheel.left, rcWheel.bottom-rcWheel.top, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		m_wndAlpha.SetWindowPos(NULL, &rcAlpha, SWP_NOZORDER|SWP_NOACTIVATE);
		if (bVertical) m_wndAlpha.SetVertical(); else m_wndAlpha.SetHorizontal();

		for (int i = 0; i < 4; ++i)
		{
			m_aEdits[i].first.SetWindowPos(NULL, GET_X_LPARAM(a_lParam)-m_rcEditSize.right, m_nCollapsedSize+m_rcGaps.top+(m_rcEditSize.bottom+m_rcGaps.top)*i, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			m_aEdits[i].second.SetWindowPos(NULL, GET_X_LPARAM(a_lParam)-m_rcEditSize.left, m_nCollapsedSize+m_rcGaps.top+(m_rcEditSize.bottom+m_rcGaps.top)*i, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		}

		m_wndARGB.SetWindowPos(NULL, GET_X_LPARAM(a_lParam)-m_rcEditSize.right, m_nCollapsedSize+m_rcGaps.top+(m_rcEditSize.bottom+m_rcGaps.top)*4, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}

	if (bVisible)
	{
		SetRedraw(TRUE);

		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	return 0;
}

COLORREF MixColors(COLORREF a_c1, COLORREF a_c2, float a_f1)
{
	float const fR1 = CGammaTables::FromSRGB(GetRValue(a_c1));
	float const fG1 = CGammaTables::FromSRGB(GetGValue(a_c1));
	float const fB1 = CGammaTables::FromSRGB(GetBValue(a_c1));
	float const fR2 = CGammaTables::FromSRGB(GetRValue(a_c2));
	float const fG2 = CGammaTables::FromSRGB(GetGValue(a_c2));
	float const fB2 = CGammaTables::FromSRGB(GetBValue(a_c2));
	int const nR = CGammaTables::ToSRGB(fR1*a_f1 + fR2*(1.0f-a_f1));
	int const nG = CGammaTables::ToSRGB(fG1*a_f1 + fG2*(1.0f-a_f1));
	int const nB = CGammaTables::ToSRGB(fB1*a_f1 + fB2*(1.0f-a_f1));
	return RGB(nR, nG, nB);
}

LRESULT CColorPicker::OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
{
	if (m_wndARGB.m_hWnd == NULL)
		return 0;

	COLORREF clrWnd = GetSysColor(COLOR_WINDOW);
	COLORREF clrLight = GetSysColor(COLOR_3DLIGHT);
	COLORREF clrDark = GetSysColor(COLOR_3DSHADOW);
	if (m_hBrushR) m_hBrushR.DeleteObject();
	m_hBrushR.CreateSolidBrush(m_clrBrushR = MixColors(clrWnd, RGB(255, 0, 0), 0.9f));
	if (m_hBrushG) m_hBrushG.DeleteObject();
	m_hBrushG.CreateSolidBrush(m_clrBrushG = MixColors(clrWnd, RGB(0, 255, 0), 0.9f));
	if (m_hBrushB) m_hBrushB.DeleteObject();
	m_hBrushB.CreateSolidBrush(m_clrBrushB = MixColors(clrWnd, RGB(0, 0, 255), 0.9f));

	HDC hDC = GetDC();

	if (m_hBrushA) m_hBrushA.DeleteObject();
	BITMAPINFO tBMI;
	ZeroMemory(&tBMI, sizeof tBMI);
	tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
	tBMI.bmiHeader.biWidth = tBMI.bmiHeader.biHeight = ((GetDeviceCaps(hDC, LOGPIXELSX)+12)/24)<<1;
	tBMI.bmiHeader.biPlanes = 1;
	tBMI.bmiHeader.biBitCount = 32;
	tBMI.bmiHeader.biCompression = BI_RGB;
	DWORD* pData = NULL;
	HBITMAP hBmp = CreateDIBSection(hDC, &tBMI, DIB_RGB_COLORS, reinterpret_cast<void**>(&pData), NULL, 0);
	DWORD dwClr1 = 0xff000000|MixColors(clrWnd, clrLight, 0.75f);
	DWORD dwClr2 = 0xff000000|MixColors(clrWnd, clrDark, 0.75f);
	for (LONG y = 0; y < tBMI.bmiHeader.biHeight; ++y)
	{
		for (LONG x = 0; x < tBMI.bmiHeader.biWidth; ++x, ++pData)
		{
			*pData = ((x/(tBMI.bmiHeader.biWidth>>1))^(y/(tBMI.bmiHeader.biHeight>>1)))&1 ? dwClr1 : dwClr2;
		}
	}
	m_hBrushA.CreatePatternBrush(hBmp);
	DeleteObject(hBmp);

	if (m_hBrushHex) m_hBrushHex.DeleteObject();
	RECT rcEdit = {0, 0, 100, 10};
	m_wndARGB.GetClientRect(&rcEdit);
	ZeroMemory(&tBMI, sizeof tBMI);
	tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
	tBMI.bmiHeader.biWidth = rcEdit.right;
	tBMI.bmiHeader.biHeight = 1;//((GetDeviceCaps(hDC, LOGPIXELSX)+12)/24)<<1;
	tBMI.bmiHeader.biPlanes = 1;
	tBMI.bmiHeader.biBitCount = 32;
	tBMI.bmiHeader.biCompression = BI_RGB;
	pData = NULL;
	hBmp = CreateDIBSection(hDC, &tBMI, DIB_RGB_COLORS, reinterpret_cast<void**>(&pData), NULL, 0);
	for (LONG y = 0; y < tBMI.bmiHeader.biHeight; ++y)
	{
		for (LONG x = 0; x < tBMI.bmiHeader.biWidth; ++x, ++pData)
		{
			if (x < m_tHexCharPos.first-6*m_tHexCharPos.second || x >= m_tHexCharPos.first)
				*pData = clrWnd;
			else if (x < m_tHexCharPos.first-4*m_tHexCharPos.second)
				*pData = m_clrBrushB;
			else if (x < m_tHexCharPos.first-2*m_tHexCharPos.second)
				*pData = m_clrBrushG;
			else
				*pData = m_clrBrushR;
			//*pData = ((x/(tBMI.bmiHeader.biWidth>>1))^(y/(tBMI.bmiHeader.biHeight>>1)))&1 ? dwClr1 : dwClr2;
		}
	}
	m_hBrushHex.CreatePatternBrush(hBmp);
	DeleteObject(hBmp);

	ReleaseDC(hDC);

	return 0;
}

LRESULT CColorPicker::OnCtlColorEdit(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	CDCHandle cDC = reinterpret_cast<HDC>(a_wParam);
	HWND hWnd = reinterpret_cast<HWND>(a_lParam);
	if (hWnd == m_aEdits[0].first)
	{
		cDC.SetBkColor(m_clrBrushR);
		return reinterpret_cast<LPARAM>(m_hBrushR.m_hBrush);
	}
	else if (hWnd == m_aEdits[1].first)
	{
		cDC.SetBkColor(m_clrBrushG);
		return reinterpret_cast<LPARAM>(m_hBrushG.m_hBrush);
	}
	else if (hWnd == m_aEdits[2].first)
	{
		cDC.SetBkColor(m_clrBrushB);
		return reinterpret_cast<LPARAM>(m_hBrushB.m_hBrush);
	}
	else if (hWnd == m_aEdits[3].first)
	{
		RECT r;
		m_aEdits[3].first.GetWindowRect(&r);
		//cDC.SetBrushOrg(r.left, r.top);
		cDC.SetBkMode(TRANSPARENT);
		return reinterpret_cast<LPARAM>(m_hBrushA.m_hBrush);
	}
	else if (hWnd == m_wndARGB)
	{
		//RECT r;
		//m_wndARGB.GetWindowRect(&r);
		//ScreenToClient(&r);
		//cDC.SetBrushOrg(10, 0);//-r.left, -r.top);
		cDC.SetBkMode(TRANSPARENT);
		return reinterpret_cast<LPARAM>(m_hBrushHex.m_hBrush);
	}
	else
	{
		return 0;
	}
}

LRESULT CColorPicker::OnARGBChar(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if ((a_wParam >= L'0' && a_wParam <= L'9') ||
		(a_wParam >= L'a' && a_wParam <= L'f') ||
		(a_wParam >= L'A' && a_wParam <= L'F') || a_wParam == VK_BACK ||
		::GetAsyncKeyState(VK_SHIFT)<0 || ::GetAsyncKeyState(VK_CONTROL)<0)
	{
		a_bHandled = FALSE;
		m_wndARGB.HideBalloonTip();
		return 0;
	}
	if (a_wParam == L'#')
	{
		return 0;
	}

	EDITBALLOONTIP tEBT;
	tEBT.cbStruct = sizeof tEBT;
	tEBT.pszTitle = _T("Invalid color format");
	tEBT.pszText = _T("Only 0-9 or A-F characters are allowed.");
	tEBT.ttiIcon = TTI_WARNING_LARGE;
	m_wndARGB.ShowBalloonTip(&tEBT);
	//CDCHandle cDC(reinterpret_cast<HDC>(a_wParam));
	//RECT rc = {0, 0, 0, 0};
	//m_wndARGB.GetClientRect(&rc);
	//cDC.FillRect(&rc, m_hBrushR);
	return 1;
}

LRESULT CColorPicker::OnARGBChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (!m_bEnableUpdates)
		return 0;

	TCHAR szTmp[11] = _T("");
	m_wndARGB.GetWindowText(szTmp, 10);
	szTmp[10] = _T('\0');
	size_t nLen = _tcslen(szTmp);
	DWORD dw = 0;
	if ((nLen != 3 && nLen != 6 && nLen != 8) || 1 != _stscanf(szTmp, _T("%x"), &dw))
		return 0;
	if (nLen == 3)
	{
		m_tColor.fR = CGammaTables::FromSRGB(((dw>>8)&0xf)*17);
		m_tColor.fG = CGammaTables::FromSRGB(((dw>>4)&0xf)*17);
		m_tColor.fB = CGammaTables::FromSRGB((dw&0xf)*17);
	}
	else
	{
		m_tColor.fR = CGammaTables::FromSRGB((dw>>16)&0xff);
		m_tColor.fG = CGammaTables::FromSRGB((dw>>8)&0xff);
		m_tColor.fB = CGammaTables::FromSRGB(dw&0xff);
	}
	if (nLen == 8)
		m_tColor.fA = ((dw>>24)&0xff)/255.0f;
	else
		m_tColor.fA = 1.0f;
	m_bEnableUpdates = false;
	Data2GUI(EUSHexEdit);
	Fire_Notify(ECPCColor);
	m_bEnableUpdates = true;
	return 0;
}

LRESULT CColorPicker::OnHistoryChanged(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	if (m_bEnableUpdates)
	{
		CColorHistory::NMCOLORHISTORY const* pNM = reinterpret_cast<CColorHistory::NMCOLORHISTORY const*>(a_pNMHeader);
		m_tColor = pNM->clr;
		m_bEnableUpdates = false;
		Data2GUI(EUSHistory);
		Fire_Notify(ECPCColor);
		m_bEnableUpdates = true;
	}
	return 0;
}

LRESULT CColorPicker::OnARGBKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_bEnableUpdates = false;
	Data2GUI(EUSUnknown);
	m_bEnableUpdates = true;
	ControlLostFocus();
	return 0;
}

LRESULT CColorPicker::OnDropper(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	COLORREF clr = 0;
	m_bEnableUpdates = false;
	if (CPixelColorPicker::PickColor(&clr))
	{
		m_tColor.fR = CGammaTables::FromSRGB(GetRValue(clr));
		m_tColor.fG = CGammaTables::FromSRGB(GetGValue(clr));
		m_tColor.fB = CGammaTables::FromSRGB(GetBValue(clr));
		m_tColor.fA = 1.0f;
		Data2GUI(EUSDropper);
		Fire_Notify(ECPCColor);
		m_wndHistory.AddActiveColorToHistory();
	}
	m_bEnableUpdates = true;
	return 0;
}

void CColorPicker::SplitArea(RECT const& a_rc, RECT* a_pWheel, RECT* a_pAlpha, bool* a_pVertical)
{
	*a_pWheel = a_rc;
	*a_pAlpha = a_rc;
	if (a_rc.right-a_rc.left > a_rc.bottom-a_rc.top)
	{
		*a_pVertical = true;
		a_pWheel->right = a_rc.right-m_rcGaps.left-m_rcGaps.right-m_rcGaps.left;
		a_pAlpha->left = a_rc.right-m_rcGaps.left-m_rcGaps.right;
	}
	else
	{
		*a_pVertical = false;
		a_pWheel->bottom = a_rc.bottom-m_rcGaps.left-m_rcGaps.right-m_rcGaps.left;
		a_pAlpha->top = a_rc.bottom-m_rcGaps.left-m_rcGaps.right;
	}
}

LRESULT CColorPicker::OnExpand(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_wndToolBar.HideButton(ID_EXPAND);
	m_wndToolBar.HideButton(ID_COLLAPSE, FALSE);
	m_bExpanded = true;
	if (m_wndWheel.m_hWnd == NULL)
	{
		m_bEnableUpdates = false;

		UpdateColorConfig();

		RECT rcWnd = {0, 0, 0, 0};
		GetClientRect(&rcWnd);
		m_rcEditSize.left = m_rcEditSize.top = m_rcEditSize.bottom = 12; m_rcEditSize.right = 37;
		MapDialogRect(&m_rcEditSize);

		RECT rc = {0, m_nCollapsedSize+m_rcGaps.top, rcWnd.right-m_rcEditSize.right-m_rcGaps.right, m_rcEditSize.bottom*4+m_rcGaps.top*5+m_nCollapsedSize*2};
		RECT rcWheel;
		RECT rcAlpha;
		bool bVertical = true;
		SplitArea(rc, &rcWheel, &rcAlpha, &bVertical);
		m_wndWheel.Create(m_hWnd, &rcWheel, NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_WHEEL);
		m_wndAlpha.Create(m_hWnd, &rcAlpha, NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_ALPHA);
		if (bVertical) m_wndAlpha.SetVertical(); else m_wndAlpha.SetHorizontal();
		m_wndWheel.SetColor(m_tColor);
		m_wndAlpha.SetColor(m_tColor);

		HFONT hFont = GetFont();
		for (int i = 0; i < 4; ++i)
		{
			TCHAR szVal[32] = _T("");
			FormatFloatEdit((&m_tColor.fR)[i], szVal);
			RECT rc = {rcWnd.right-m_rcEditSize.right, m_nCollapsedSize+m_rcGaps.top+(m_rcEditSize.bottom+m_rcGaps.top)*i, rcWnd.right, m_nCollapsedSize+m_rcGaps.top+(m_rcEditSize.bottom+m_rcGaps.top)*i+m_rcEditSize.bottom};
			m_aEdits[i].first.Create(m_hWnd, &rc, szVal, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_TABSTOP|ES_RIGHT, WS_EX_CLIENTEDGE, IDC_EDIT_BASE+i);
			m_aEdits[i].first.SetFont(hFont);
			rc.left = rcWnd.right-m_rcEditSize.left;
			m_aEdits[i].second.Create(m_hWnd, &rc, NULL, WS_CHILD|WS_VISIBLE|UDS_ALIGNRIGHT|UDS_AUTOBUDDY|UDS_NOTHOUSANDS|UDS_ARROWKEYS, 0, IDC_SPIN_BASE+i);
		}
		RECT rcTmp = {0, 0, 0, 0};
		m_aEdits[0].second.GetWindowRect(&rcTmp);
		m_rcEditSize.left = rcTmp.right-rcTmp.left;

		RECT rcARGB = {rcWnd.right-m_rcEditSize.right, m_nCollapsedSize+m_rcGaps.top+(m_rcEditSize.bottom+m_rcGaps.top)*4, rcWnd.right, m_nCollapsedSize+m_rcGaps.top+(m_rcEditSize.bottom+m_rcGaps.top)*4+m_nCollapsedSize};
		m_nExpandedSize = rcARGB.bottom;
		TCHAR szHex[32];
		FormatHexEdit(szHex);
		int nHex = _tcslen(szHex);
		m_wndARGB.Create(m_hWnd, &rcARGB, szHex, WS_VISIBLE|WS_TABSTOP|WS_CHILD|ES_AUTOHSCROLL|ES_RIGHT|ES_UPPERCASE, WS_EX_CLIENTEDGE, IDC_AARRGGBB);
		HDC hDC = GetDC();
		m_cARGBFont.CreateFont((13*GetDeviceCaps(hDC, LOGPIXELSY)+48)/96, 0, 0, 0, FW_BOLD, FALSE, FALSE, 
			FALSE,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Courier New"));
		ReleaseDC(hDC);
		m_wndARGB.SetFont(m_cARGBFont);
		POINT tPt0 = m_wndARGB.PosFromChar(nHex-2);
		POINT tPt1 = m_wndARGB.PosFromChar(nHex-1);
		m_tHexCharPos.second = tPt1.x-tPt0.x;
		m_tHexCharPos.first = tPt1.x+m_tHexCharPos.second;

		BOOL b;
		OnSettingChange(0, 0, 0, b);

		CContextHelpDlg<CColorPicker>::OnInitDialog(0, 0, 0, b);

		m_bEnableUpdates = true;
	}
	else
	{
		m_wndWheel.ShowWindow(SW_SHOW);
		m_wndAlpha.ShowWindow(SW_SHOW);
		for (int i = 0; i < 4; ++i)
		{
			m_aEdits[i].first.ShowWindow(SW_SHOW);
			m_aEdits[i].second.ShowWindow(SW_SHOW);
		}
		m_wndARGB.ShowWindow(SW_SHOW);
	}
	Fire_Notify(ECPCLayout);
	if (a_wID, m_pColorsCfg && m_bstrContext.m_str)
	{
		CConfigValue cState;
		CComBSTR bstrExpanded(L"Expanded");
		m_pColorsCfg->ItemValueGet(bstrExpanded, &cState);
		LPOLESTR p = wcsstr(cState, m_bstrContext);
		BSTR b = cState;
		if (p == NULL)
		{
			CComBSTR bstr(cState.operator BSTR());
			bstr += m_bstrContext;
			m_pColorsCfg->ItemValuesSet(1, &(bstrExpanded.m_str), CConfigValue(bstr));
		}
	}
	return 0;
}

LRESULT CColorPicker::OnCollapse(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	GotoDlgCtrl(m_wndActive);

	m_wndToolBar.HideButton(ID_COLLAPSE);
	m_wndToolBar.HideButton(ID_EXPAND, FALSE);
	m_bExpanded = false;
	if (m_wndWheel.m_hWnd)
	{
		m_wndWheel.ShowWindow(SW_HIDE);
		m_wndAlpha.ShowWindow(SW_HIDE);
		for (int i = 0; i < 4; ++i)
		{
			m_aEdits[i].first.ShowWindow(SW_HIDE);
			m_aEdits[i].second.ShowWindow(SW_HIDE);
		}
		m_wndARGB.ShowWindow(SW_HIDE);
	}
	Fire_Notify(ECPCLayout);
	if (m_pColorsCfg && m_bstrContext.m_str)
	{
		CConfigValue cState;
		CComBSTR bstrExpanded(L"Expanded");
		m_pColorsCfg->ItemValueGet(bstrExpanded, &cState);
		LPOLESTR p = wcsstr(cState, m_bstrContext);
		BSTR b = cState;
		if (p)
		{
			wcscpy(p, p+m_bstrContext.Length());
			m_pColorsCfg->ItemValuesSet(1, &(bstrExpanded.m_str), CConfigValue((OLECHAR*)b));
		}
	}
	return 0;
}

LRESULT CColorPicker::OnColorWheelChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	TColor tCol = m_wndWheel.GetColor();
	if (fabsf(tCol.fR-m_tColor.fR) > 1e-4f || fabsf(tCol.fG-m_tColor.fG) > 1e-4f || fabsf(tCol.fB-m_tColor.fB) > 1e-4f)
	{
		m_tColor.fR = tCol.fR;
		m_tColor.fG = tCol.fG;
		m_tColor.fB = tCol.fB;
		m_bEnableUpdates = false;
		Data2GUI(EUSWheel);
		Fire_Notify(ECPCColor);
		m_bEnableUpdates = true;
	}
	return 0;
}

LRESULT CColorPicker::OnColorAlphaChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	float fNewA = m_wndAlpha.GetColor().fA;
	if (fNewA != m_tColor.fA)
	{
		m_tColor.fA = fNewA;
		m_bEnableUpdates = false;
		Data2GUI(EUSAlpha);
		Fire_Notify(ECPCColor);
		m_bEnableUpdates = true;
	}
	return 0;
}

void CColorPicker::Data2GUI(CColorPicker::EUpdateSource a_eSource)
{
	if (a_eSource != EUSHexEdit && m_wndARGB)
	{
		TCHAR szTmp[32];
		FormatHexEdit(szTmp);
		m_wndARGB.SetWindowText(szTmp);
	}
	if (a_eSource != EUSFloatEdit && m_aEdits[0].first.m_hWnd)
	{
		for (int i = 0; i < 4; ++i)
		{
			TCHAR szTmp[32] = _T("");
			FormatFloatEdit((&m_tColor.fR)[i], szTmp);
			m_aEdits[i].first.SetWindowText(szTmp);
		}
	}
	if (a_eSource != EUSWheel && a_eSource != EUSAlpha && m_wndWheel.m_hWnd)
	{
		m_wndWheel.SetColor(m_tColor);
	}
	if (a_eSource != EUSAlpha && m_wndAlpha.m_hWnd)
	{
		m_wndAlpha.SetColor(m_tColor);
	}
	if (a_eSource != EUSHistory)
	{
		m_wndHistory.SetActiveColor(m_tColor);
	}
	HBITMAP hBmp = CreateColorBitmap();
	m_cImageList.Replace(3, hBmp, NULL);
	DeleteObject(hBmp);
	m_wndActive.Invalidate(FALSE);
}

void CColorPicker::FormatFloatEdit(float a_f, LPTSTR a_psz)
{
	float const fMul1 = m_fFactor*powf(10, m_nDecimals);
	float const fMul2 = powf(10, -m_nDecimals);
	_stprintf(a_psz, _T("%g"), float(int(a_f*fMul1+0.5f)*fMul2));
}

void CColorPicker::FormatHexEdit(LPTSTR a_psz)
{
	TCHAR* psz = a_psz;
	if (m_tColor.fA < 1.0f)
	{
		int n = m_tColor.fA*255.0+0.5f;
		if (n > 255) n = 255; else if (n < 0) n = 0;
		_stprintf(psz, _T("%02X"), n);
		psz += 2;
	}
	for (int i = 0; i < 3; ++i)
	{
		float f = (&m_tColor.fR)[i];
		//if (f < 0.0f) f = 0.0f; else if (f > 1.0f) f = 1.0f;
		int n = CGammaTables::ToSRGB(f);
		_stprintf(psz, _T("%02X"), n);
		psz += 2;
	}
	*psz = _T('\0');
}

HBITMAP CColorPicker::CreateColorBitmap()
{
	BITMAPINFO tBMI;
	ZeroMemory(&tBMI, sizeof tBMI);
	tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
	tBMI.bmiHeader.biWidth = m_nIconSize;
	tBMI.bmiHeader.biHeight = m_nIconSize;
	tBMI.bmiHeader.biPlanes = 1;
	tBMI.bmiHeader.biBitCount = 32;
	tBMI.bmiHeader.biCompression = BI_RGB;
	CAutoVectorPtr<DWORD> pData(new DWORD[m_nIconSize*m_nIconSize]);
	DWORD dwL = GetSysColor(COLOR_3DLIGHT);
	float fLR = CGammaTables::FromSRGB(GetRValue(dwL));
	float fLG = CGammaTables::FromSRGB(GetGValue(dwL));
	float fLB = CGammaTables::FromSRGB(GetBValue(dwL));
	DWORD dwS = GetSysColor(COLOR_3DSHADOW);
	float fSR = CGammaTables::FromSRGB(GetRValue(dwS));
	float fSG = CGammaTables::FromSRGB(GetGValue(dwS));
	float fSB = CGammaTables::FromSRGB(GetBValue(dwS));
	dwL = 0xff000000|RGB(
		CGammaTables::ToSRGB(fLB*(1.0f-m_tColor.fA)+m_tColor.fB*m_tColor.fA),
		CGammaTables::ToSRGB(fLG*(1.0f-m_tColor.fA)+m_tColor.fG*m_tColor.fA),
		CGammaTables::ToSRGB(fLR*(1.0f-m_tColor.fA)+m_tColor.fR*m_tColor.fA));
	dwS = 0xff000000|RGB(
		CGammaTables::ToSRGB(fSB*(1.0f-m_tColor.fA)+m_tColor.fB*m_tColor.fA),
		CGammaTables::ToSRGB(fSG*(1.0f-m_tColor.fA)+m_tColor.fG*m_tColor.fA),
		CGammaTables::ToSRGB(fSR*(1.0f-m_tColor.fA)+m_tColor.fR*m_tColor.fA));
	DWORD dwM = 0xff000000|RGB(
		CGammaTables::ToSRGB((fSB+fLB)*0.5f*(1.0f-m_tColor.fA)+m_tColor.fB*m_tColor.fA),
		CGammaTables::ToSRGB((fSG+fLG)*0.5f*(1.0f-m_tColor.fA)+m_tColor.fG*m_tColor.fA),
		CGammaTables::ToSRGB((fSR+fLR)*0.5f*(1.0f-m_tColor.fA)+m_tColor.fR*m_tColor.fA));
	DWORD* pD = pData;
	int y;
	for (y = 0; y < m_nIconSize>>1; ++y)
	{
		int x;
		for (x = 0; x < m_nIconSize>>1; ++x, ++pD)
			*pD = dwS;
		if (m_nIconSize&1)
		{
			*pD = dwM;
			++pD;
			++x;
		}
		for (; x < m_nIconSize; ++x, ++pD)
			*pD = dwL;
	}
	if (m_nIconSize&1)
	{
		for (int x = 0; x < m_nIconSize; ++x, ++pD)
			*pD = dwM;
		++y;
	}
	for (; y < m_nIconSize; ++y)
	{
		int x;
		for (x = 0; x < m_nIconSize>>1; ++x, ++pD)
			*pD = dwL;
		if (m_nIconSize&1)
		{
			*pD = dwM;
			++pD;
			++x;
		}
		for (; x < m_nIconSize; ++x, ++pD)
			*pD = dwS;
	}
	HDC hDC = GetDC();
	HBITMAP hBmp = CreateDIBitmap(hDC, &tBMI.bmiHeader, CBM_INIT, pData.m_p, &tBMI, DIB_RGB_COLORS);
	ReleaseDC(hDC);
	return hBmp;
}

class CSimpleRNG
{
public:
	void RandomInit(unsigned int seed)
	{
		int i;
		unsigned int s = seed;
		// make random numbers and put them into the buffer
		for (i=0; i<5; i++)
		{
			s = s * 29943829 - 1;
			x[i] = s * (1./(65536.*65536.));
		}
		// randomize some more
		for (i=0; i<19; i++)
			Random();
	}
	int IRandom(int min, int max)       // get integer random number in desired interval
	{
		int iinterval = max - min + 1;
		if (iinterval <= 0) return 0x80000000; // error
		int i = int(iinterval * Random());     // truncate
		if (i >= iinterval) i = iinterval-1;
		return min + i;
	}
	double Random()                     // get floating point random number
	{
		long double c;
		c = (long double)2111111111.0 * x[3] +
			1492.0 * (x[3] = x[2]) +
			1776.0 * (x[2] = x[1]) +
			5115.0 * (x[1] = x[0]) +
			x[4];
		x[4] = floorl(c);
		x[0] = c - x[4];
		x[4] = x[4] * (1./(65536.*65536.));
		return x[0];
	}

private:
	double x[5];                         // history buffer
};

LRESULT CColorPicker::OnActiveDropDown(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
{
	m_wndPopup.RemoveAllItems();
	CComBSTR bstrRandom;
	CMultiLanguageString::GetLocalized(L"[0409]Random color[0405]Náhodná barva", m_tLocaleID, &bstrRandom);
	m_wndPopup.SetTextItem(5, bstrRandom);
	m_wndPopup.SetSimilarColorsItem(10, CColorPopup::SColor(m_tColor.fR, m_tColor.fG, m_tColor.fB, m_tColor.fA));
	m_wndPopup.SetGapItem(12);
	m_wndPopup.SetDefaultColorsItem(15);
	CComBSTR bstrMore;
	CMultiLanguageString::GetLocalized(L"[0409]More colors...[0405]Další barvy...", m_tLocaleID, &bstrMore);
	m_wndPopup.SetTextItem(20, bstrMore);
	RECT rc = {0, 0, 0, 0};
	m_wndActive.GetItemRect(0, &rc);
	m_wndActive.ClientToScreen(&rc);
	int iEntry = -1;
	CColorPopup::SColor sNew;
	m_bEnableUpdates = false;
	if (m_wndPopup.ShowPopup(m_hWnd, rc, CColorPopup::SColor(m_tColor.fR, m_tColor.fG, m_tColor.fB, m_tColor.fA), &iEntry, &sNew))
	{
		if (iEntry == 10 || iEntry == 15)
		{
			m_tColor.fR = sNew.fR;
			m_tColor.fG = sNew.fG;
			m_tColor.fB = sNew.fB;
			m_tColor.fA = sNew.fA;

			Data2GUI(EUSPopup);
			Fire_Notify(ECPCColor);
		}
		else if (iEntry == 20)
		{
			if (ColorWindowModal(&m_tColor, m_tLocaleID, m_hWnd, true))
			{
				Data2GUI(EUSPopup);
				Fire_Notify(ECPCColor);
				m_wndHistory.AddActiveColorToHistory();
			}
		}
		else if (iEntry == 5)
		{
			static CSimpleRNG s_cRNG;
			static bool s_bInitialize = true;
			if (s_bInitialize)
			{
				// not thread safe, but unimportant
				s_cRNG.RandomInit(GetTickCount());
				s_cRNG.Random();
				s_cRNG.Random();
				s_cRNG.Random();
				s_bInitialize = false;
			}

			m_tColor.fR = s_cRNG.IRandom(0, 1000)/1000.0f;
			m_tColor.fG = s_cRNG.IRandom(0, 1000)/1000.0f;
			m_tColor.fB = s_cRNG.IRandom(0, 1000)/1000.0f;
			m_tColor.fA = 1.0f;

			Data2GUI(EUSPopup);
			Fire_Notify(ECPCColor);
		}
	}
	m_bEnableUpdates = true;
	return TBDDRET_DEFAULT;
}

LRESULT CColorPicker::OnEditChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (!m_bEnableUpdates)
		return 0;
	int t_nChannel = a_wID-IDC_EDIT_BASE;
	if (t_nChannel < 0 || t_nChannel >= 4)
	{
		a_bHandled = false;
		return 0;
	}

	UpdateColorConfig();

	TCHAR szTmp[32] = _T("0");
	m_aEdits[t_nChannel].first.GetWindowText(szTmp, itemsof(szTmp));
	szTmp[itemsof(szTmp)-1] = _T('\0');
	float f = 0.0f;
	if (1 == _stscanf(szTmp, _T("%f"), &f))
	{
		if (m_fFactor == 1.0f && f > 1.0f && _tcschr(szTmp, _T('.')) == NULL)
		{
			if (f <= 255 && f == int(f))
				f = f/255.0f; // assume number was entered using "old" format
		}
		f /= m_fFactor;
		if (fabsf(f-(&m_tColor.fR)[t_nChannel]) >= powf(10, -m_nDecimals)/m_fFactor)
		{
			(&m_tColor.fR)[t_nChannel] = f;
			m_bEnableUpdates = false;
			Data2GUI(EUSFloatEdit);
			Fire_Notify(ECPCColor);
			m_bEnableUpdates = true;
		}
	}
	return 0;
}

LRESULT CColorPicker::OnEditKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (!m_bEnableUpdates)
		return 0;
	int t_nChannel = a_wID-IDC_EDIT_BASE;
	if (t_nChannel < 0 || t_nChannel >= 4)
	{
		a_bHandled = false;
		return 0;
	}

	UpdateColorConfig();
	float const fMul1 = m_fFactor*powf(10, m_nDecimals);
	float const fMul2 = powf(10, -m_nDecimals);

	TCHAR szTmp[32];
	_stprintf(szTmp, _T("%g"), float(int((&m_tColor.fR)[t_nChannel]*fMul1+0.5f)*fMul2));
	m_bEnableUpdates = false;
	m_aEdits[t_nChannel].first.SetWindowText(szTmp);
	m_bEnableUpdates = true;
	ControlLostFocus();
	return 0;
}

LRESULT CColorPicker::OnEditSetFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	//m_pFocusWindow = m_wndEdit+t_nChannel;
	return 0;
}

LRESULT CColorPicker::OnUpDownChange(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
{
	if (!m_bEnableUpdates)
		return 0;
	int t_nChannel = a_idCtrl-IDC_SPIN_BASE;
	if (t_nChannel < 0 || t_nChannel >= 4)
	{
		a_bHandled = false;
		return 0;
	}

	LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

	UpdateColorConfig();
	float const fMul1 = m_fFactor*powf(10, m_nDecimals);
	float const fMul2 = powf(10, -m_nDecimals);

	if (pNMUD->iDelta > 0)
	{
		if ((&m_tColor.fR)[t_nChannel] > 0.0f)
		{
			int n = (&m_tColor.fR)[t_nChannel]*100.0f-0.5f;
			if (n < 0) n = 0;
			(&m_tColor.fR)[t_nChannel] = n/100.0f;
			m_bEnableUpdates = false;
			TCHAR szTmp[32];
			_stprintf(szTmp, _T("%g"), float(int((&m_tColor.fR)[t_nChannel]*fMul1+0.5f)*fMul2));
			m_aEdits[t_nChannel].first.SetWindowText(szTmp);
			Data2GUI(EUSFloatEdit);
			Fire_Notify(ECPCColor);
			m_bEnableUpdates = true;
		}
	}
	else
	{
		if ((&m_tColor.fR)[t_nChannel] < 1.0f)
		{
			int n = (&m_tColor.fR)[t_nChannel]*100.0f+1.5f;
			if (n > 100) n = 100;
			(&m_tColor.fR)[t_nChannel] = n/100.0f;
			m_bEnableUpdates = false;
			TCHAR szTmp[32];
			_stprintf(szTmp, _T("%g"), float(int((&m_tColor.fR)[t_nChannel]*fMul1+0.5f)*fMul2));
			m_aEdits[t_nChannel].first.SetWindowText(szTmp);
			Data2GUI(EUSFloatEdit);
			Fire_Notify(ECPCColor);
			m_bEnableUpdates = true;
		}
	}

	return 0;
}

LRESULT CColorPicker::OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	HWND hFocus = GetFocus();
	for (int i = 0; i < 4; i++)
	{
		if (hFocus == m_aEdits[i].first.m_hWnd)
		{
			m_aEdits[(i+1)&3].first.SetSelAll();
			m_aEdits[(i+1)&3].first.SetFocus();
			break;
		}
	}
	return 0;
}

void CColorPicker::ControlLostFocus()
{
	if (!m_bEnableUpdates)
		return;

	HWND hFocus = GetFocus();
	if (hFocus && hFocus != m_hWnd && hFocus != m_wndActive && hFocus != m_wndAlpha && hFocus != m_wndPopup &&
		hFocus != m_wndARGB && hFocus != m_wndHistory && hFocus != m_wndToolBar && hFocus != m_wndWheel &&
		hFocus != m_aEdits[0].first && hFocus != m_aEdits[0].second &&
		hFocus != m_aEdits[1].first && hFocus != m_aEdits[1].second &&
		hFocus != m_aEdits[2].first && hFocus != m_aEdits[2].second &&
		hFocus != m_aEdits[3].first && hFocus != m_aEdits[3].second)
	{
		m_wndHistory.AddActiveColorToHistory();
	}
}

LRESULT CColorPicker::OnCtrlKillFocus(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	ControlLostFocus();
	return 0;
}

