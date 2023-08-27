
#include "stdafx.h"
#include "ConfigGUISplitter.h"

#include "ConfigIDsSplitter.h"


STDMETHODIMP CConfigGUISplitter::UIDGet(GUID* a_pguid)
{
	try
	{
		*a_pguid = CLSID_DesignerViewFactorySplitter;
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CConfigGUISplitter::RequiresMargins()
{
	return S_OK;
}

STDMETHODIMP CConfigGUISplitter::MinSizeGet(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
{
	try
	{
		SIZE tSize = {100, 100};
		Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), CConfigWnd::IDD, &tSize, a_tLocaleID);
		*a_nSizeX = tSize.cx;
		*a_nSizeY = tSize.cy;
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CConfigGUISplitter::WindowCreate(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
{
	try
	{
		CComObject<CConfigWnd>* pWnd = NULL;
		CComObject<CConfigWnd>::CreateInstance(&pWnd);
		CComPtr<IChildWindow> pTmp = pWnd;

		pWnd->Create(reinterpret_cast<HWND>(a_hParent), a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_pConfig, a_eMode);

		*a_ppWindow = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

void CConfigGUISplitter::CConfigWnd::Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, IConfig* a_pConfig, EConfigPanelMode a_eMode)
{
	m_tLocaleID = a_tLocaleID;
	m_pConfig = a_pConfig;
	m_eMode = a_eMode;

	Win32LangEx::CLangDialogImpl<CConfigWnd>::Create(a_hParent);
	if (!IsWindow()) throw E_FAIL;

	MoveWindow(a_prcPositon);
	SetWindowLong(GWL_ID, a_nCtlID);
	ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
}

LRESULT CConfigGUISplitter::CConfigWnd::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	m_wndSplitDirection = GetDlgItem(IDC_CGSPLITTER_SPLITDIRECTION);

	m_wndVerRelative = GetDlgItem(IDC_CGSPLITTER_VERRELATIVE);
	m_wndVerAdjustable = GetDlgItem(IDC_CGSPLITTER_VERADJUSTABLE);
	m_wndVerEdit = GetDlgItem(IDC_CGSPLITTER_VERABSOLUTEVAL);
	m_wndVerStatic = GetDlgItem(IDC_CGSPLITTER_VERPIXELS);
	m_wndVerLine = GetDlgItem(IDC_CGSPLITTER_VERGROUP);
	m_wndVerLabel = GetDlgItem(IDC_CGSPLITTER_VERLABEL);
	m_wndVerFromBack = GetDlgItem(IDC_CGSPLITTER_VERABSOLUTEBACK);
	m_wndVerSlider = GetDlgItem(IDC_CGSPLITTER_VERRELATIVEVAL);

	m_wndHorRelative = GetDlgItem(IDC_CGSPLITTER_HORRELATIVE);
	m_wndHorAdjustable = GetDlgItem(IDC_CGSPLITTER_HORADJUSTABLE);
	m_wndHorEdit = GetDlgItem(IDC_CGSPLITTER_HORABSOLUTEVAL);
	m_wndHorStatic = GetDlgItem(IDC_CGSPLITTER_HORPIXELS);
	m_wndHorLine = GetDlgItem(IDC_CGSPLITTER_HORGROUP);
	m_wndHorLabel = GetDlgItem(IDC_CGSPLITTER_HORLABEL);
	m_wndHorFromBack = GetDlgItem(IDC_CGSPLITTER_HORABSOLUTEBACK);
	m_wndHorSlider = GetDlgItem(IDC_CGSPLITTER_HORRELATIVEVAL);

	m_wndViewTypeLT = GetDlgItem(IDC_CGSPLITTER_LT);
	m_wndViewTypeLB = GetDlgItem(IDC_CGSPLITTER_LB);
	m_wndViewTypeRT = GetDlgItem(IDC_CGSPLITTER_RT);
	m_wndViewTypeRB = GetDlgItem(IDC_CGSPLITTER_RB);

	m_wndSubViews = GetDlgItem(IDC_CGSPLITTER_SUBVIEWS);

	try
	{
		RECT rcCfg;
		HWND hCfg = GetDlgItem(IDC_CGSPLITTER_LT);
		::GetWindowRect(hCfg, &rcCfg);
		ScreenToClient(&rcCfg);
		rcCfg.top = rcCfg.bottom;
		rcCfg.bottom += 20;
		CComPtr<IConfig> pSubCfg;
		m_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWLT), &pSubCfg);
		RWCoCreateInstance(m_pConfigWndLT, __uuidof(AutoConfigWnd));
		m_pConfigWndLT->Create(m_hWnd, &rcCfg, 0, m_tLocaleID, TRUE, ECWBMMarginAndOutline);
		m_pConfigWndLT->ConfigSet(pSubCfg, m_eMode);

		hCfg = GetDlgItem(IDC_CGSPLITTER_LB);
		::GetWindowRect(hCfg, &rcCfg);
		ScreenToClient(&rcCfg);
		rcCfg.top = rcCfg.bottom;
		rcCfg.bottom += 20;
		pSubCfg = NULL;
		m_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWLB), &pSubCfg);
		RWCoCreateInstance(m_pConfigWndLB, __uuidof(AutoConfigWnd));
		m_pConfigWndLB->Create(m_hWnd, &rcCfg, 0, m_tLocaleID, FALSE, ECWBMMarginAndOutline);
		m_pConfigWndLB->ConfigSet(pSubCfg, m_eMode);

		hCfg = GetDlgItem(IDC_CGSPLITTER_RT);
		::GetWindowRect(hCfg, &rcCfg);
		ScreenToClient(&rcCfg);
		rcCfg.top = rcCfg.bottom;
		rcCfg.bottom += 20;
		pSubCfg = NULL;
		m_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWRT), &pSubCfg);
		RWCoCreateInstance(m_pConfigWndRT, __uuidof(AutoConfigWnd));
		m_pConfigWndRT->Create(m_hWnd, &rcCfg, 0, m_tLocaleID, FALSE, ECWBMMarginAndOutline);
		m_pConfigWndRT->ConfigSet(pSubCfg, m_eMode);

		hCfg = GetDlgItem(IDC_CGSPLITTER_RB);
		::GetWindowRect(hCfg, &rcCfg);
		ScreenToClient(&rcCfg);
		rcCfg.top = rcCfg.bottom;
		rcCfg.bottom += 20;
		pSubCfg = NULL;
		m_pConfig->SubConfigGet(CComBSTR(CFGID_SUBVIEWRB), &pSubCfg);
		RWCoCreateInstance(m_pConfigWndRB, __uuidof(AutoConfigWnd));
		m_pConfigWndRB->Create(m_hWnd, &rcCfg, 0, m_tLocaleID, TRUE, ECWBMMarginAndOutline);
		m_pConfigWndRB->ConfigSet(pSubCfg, m_eMode);

		ValuesToGUI();
	}
	catch (...)
	{
	}

	DlgResize_Init(false, false, 0);

	return TRUE;
}

LRESULT CConfigGUISplitter::CConfigWnd::OnSplitDirectionChanged(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (!m_bSetting)
	{
		try
		{
			CComBSTR cCFGID_SPLITTYPE(CFGID_SPLITTYPE);
			CComPtr<IConfigItemOptions> pItem;
			m_pConfig->ItemGetUIInfo(cCFGID_SPLITTYPE, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
			CComPtr<IEnumConfigItemOptions> pOptions;
			pItem->OptionsEnum(&pOptions);
			ULONG nIndex = static_cast<ULONG>(m_wndSplitDirection.GetItemData(m_wndSplitDirection.GetCurSel()));
			CConfigValue cVal;
			pOptions->Get(nIndex, &cVal);
			m_pConfig->ItemValuesSet(1, &(cCFGID_SPLITTYPE.m_str), cVal);

			ValuesToGUI();
		}
		catch (...)
		{
		}
	}

	return 0;
}

LRESULT CConfigGUISplitter::CConfigWnd::OnVerTypeClicked(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (!m_bSetting)
	{
		try
		{
			LONG eSplitType = m_wndVerRelative.GetCheck() == BST_CHECKED ?
				(m_wndVerAdjustable.GetCheck() == BST_CHECKED ? EDTRelativeAdjustable : EDTRelativeFixed) :
				(m_wndVerAdjustable.GetCheck() == BST_CHECKED ? 
					(m_wndVerFromBack.GetCheck() == BST_CHECKED ? EDTAbsoluteRBAdjustable : EDTAbsoluteLTAdjustable) :
					(m_wndVerFromBack.GetCheck() == BST_CHECKED ? EDTAbsoluteRBFixed : EDTAbsoluteLTFixed));
			CConfigValue cSplitType(eSplitType);
			CComBSTR cCFGID_VERDIVTYPE(CFGID_VERDIVTYPE);
			m_pConfig->ItemValuesSet(1, &(cCFGID_VERDIVTYPE.m_str), cSplitType);

			ValuesToGUI();
		}
		catch (...)
		{
		}
	}

	return 0;
}

LRESULT CConfigGUISplitter::CConfigWnd::OnHorTypeClicked(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (!m_bSetting)
	{
		try
		{
			LONG eSplitType = m_wndHorRelative.GetCheck() == BST_CHECKED ?
				(m_wndHorAdjustable.GetCheck() == BST_CHECKED ? EDTRelativeAdjustable : EDTRelativeFixed) :
				(m_wndHorAdjustable.GetCheck() == BST_CHECKED ? 
					(m_wndHorFromBack.GetCheck() == BST_CHECKED ? EDTAbsoluteRBAdjustable : EDTAbsoluteLTAdjustable) :
					(m_wndHorFromBack.GetCheck() == BST_CHECKED ? EDTAbsoluteRBFixed : EDTAbsoluteLTFixed));
			CConfigValue cSplitType(eSplitType);
			CComBSTR cCFGID_HORDIVTYPE(CFGID_HORDIVTYPE);
			m_pConfig->ItemValuesSet(1, &(cCFGID_HORDIVTYPE.m_str), cSplitType);

			ValuesToGUI();
		}
		catch (...)
		{
		}
	}

	return 0;
}

LRESULT CConfigGUISplitter::CConfigWnd::OnVerAbsoluteChanged(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (!m_bSetting)
	{
		try
		{
			TCHAR szTmp[64] = _T("");
			m_wndVerEdit.GetWindowText(szTmp, static_cast<int>(itemsof(szTmp)));
			TCHAR* pEnd = szTmp;
			float fVal = _tcstod(szTmp, &pEnd);
			if (fVal != 0.0f || *pEnd == _T('\0'))
			{
				CConfigValue cValue(fVal);
				CComBSTR cCFGID_VERABSOLUTE(CFGID_VERABSOLUTE);
				m_pConfig->ItemValuesSet(1, &(cCFGID_VERABSOLUTE.m_str), cValue);
			}
		}
		catch (...)
		{
		}
	}

	return 0;
}

LRESULT CConfigGUISplitter::CConfigWnd::OnHorAbsoluteChanged(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	if (!m_bSetting)
	{
		try
		{
			TCHAR szTmp[64] = _T("");
			m_wndHorEdit.GetWindowText(szTmp, static_cast<int>(itemsof(szTmp)));
			TCHAR* pEnd = szTmp;
			float fVal = _tcstod(szTmp, &pEnd);
			if (fVal != 0.0f || *pEnd == _T('\0'))
			{
				CConfigValue cValue(fVal);
				CComBSTR cCFGID_HORABSOLUTE(CFGID_HORABSOLUTE);
				m_pConfig->ItemValuesSet(1, &(cCFGID_HORABSOLUTE.m_str), cValue);
			}
		}
		catch (...)
		{
		}
	}

	return 0;
}

LRESULT CConfigGUISplitter::CConfigWnd::OnVerRelativeChanged(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	if (!m_bSetting)
	{
		try
		{
			CConfigValue cValue(0.1f*static_cast<float>(m_wndVerSlider.GetPos()));
			CComBSTR cCFGID_VERRELATIVE(CFGID_VERRELATIVE);
			m_pConfig->ItemValuesSet(1, &(cCFGID_VERRELATIVE.m_str), cValue);
		}
		catch (...)
		{
		}
	}

	return 0;
}

LRESULT CConfigGUISplitter::CConfigWnd::OnHorRelativeChanged(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	if (!m_bSetting)
	{
		try
		{
			CConfigValue cValue(0.1f*static_cast<float>(m_wndHorSlider.GetPos()));
			CComBSTR cCFGID_HORRELATIVE(CFGID_HORRELATIVE);
			m_pConfig->ItemValuesSet(1, &(cCFGID_HORRELATIVE.m_str), cValue);
		}
		catch (...)
		{
		}
	}

	return 0;
}


bool Value2GUICombo(IConfig* a_pConfig, CComboBox& a_wnd, LPCOLESTR a_pszID, LCID a_tLocaleID)
{
	a_wnd.ResetContent();
	CComBSTR bstrID(a_pszID);
	CConfigValue cValue;
	a_pConfig->ItemValueGet(bstrID, &cValue);
	CComPtr<IConfigItemOptions> pItem;
	a_pConfig->ItemGetUIInfo(bstrID, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
	CComPtr<IEnumConfigItemOptions> pOptions;
	pItem->OptionsEnum(&pOptions);
	ULONG nSize = 0;
	pOptions->Size(&nSize);
	if (nSize == 0)
	{
		a_wnd.EnableWindow(FALSE);
		return false;
	}

	USES_CONVERSION;
	ULONG i;
	CConfigValue cOption;
	for (i = 0; SUCCEEDED(pOptions->Get(i, &cOption)); i++)
	{
		CComPtr<ILocalizedString> pName;
		pItem->ValueGetName(cOption, &pName);
		CComBSTR bstrName;
		if (pName) pName->GetLocalized(a_tLocaleID, &bstrName);
		int iPos = a_wnd.AddString(OLE2CT(bstrName));
		a_wnd.SetItemData(iPos, i);
		if (cValue == cOption)
		{
			a_wnd.SetCurSel(iPos);
		}
	}
	a_wnd.EnableWindow(TRUE);
	return true;
}

void SetDirectionValues(IConfig* a_pConfig, CButton& a_wndRelative, CButton& a_wndAdjustable, CButton& a_wndFromBack, CEdit& a_wndAbsoluteVal, CStatic& a_wndText, CTrackBarCtrl& a_wndRelativeVal, CWindow& a_wndLine, CWindow& a_wndLabel, LPCOLESTR a_pszTypeID, LPCOLESTR a_pszAbsoluteID, LPCOLESTR a_pszRelativeID, bool a_bEnabled)
{
	int nShow = a_bEnabled ? SW_SHOW : SW_HIDE;
	CComBSTR bstrTypeID(a_pszTypeID);
	CConfigValue cTypeValue;
	a_pConfig->ItemValueGet(bstrTypeID, &cTypeValue);
	LONG nTypeValue = cTypeValue;
	a_wndRelative.ShowWindow(nShow);
	a_wndRelative.SetCheck(nTypeValue == EDTRelativeFixed || nTypeValue == EDTRelativeAdjustable ? BST_CHECKED : BST_UNCHECKED);
	a_wndRelative.EnableWindow(a_bEnabled);
	a_wndAdjustable.ShowWindow(nShow);
	a_wndAdjustable.SetCheck(nTypeValue == EDTRelativeAdjustable || nTypeValue == EDTAbsoluteLTAdjustable || nTypeValue == EDTAbsoluteRBAdjustable ? BST_CHECKED : BST_UNCHECKED);
	a_wndAdjustable.EnableWindow(a_bEnabled);
	a_wndFromBack.ShowWindow(nShow);
	a_wndFromBack.SetCheck(nTypeValue == EDTAbsoluteRBFixed || nTypeValue == EDTAbsoluteRBAdjustable ? BST_CHECKED : BST_UNCHECKED);
	a_wndFromBack.EnableWindow(a_bEnabled && nTypeValue != EDTRelativeFixed && nTypeValue != EDTRelativeAdjustable);
	if (nTypeValue == EDTRelativeFixed || nTypeValue == EDTRelativeAdjustable)
	{
		CComBSTR bstrRelativeID(a_pszRelativeID);
		CConfigValue cRelativeValue;
		a_pConfig->ItemValueGet(bstrRelativeID, &cRelativeValue);
		a_wndRelativeVal.SetRange(0, 1000);
		a_wndRelativeVal.SetPos(cRelativeValue.operator float()*10.0f);
		a_wndFromBack.ShowWindow(SW_HIDE);
		a_wndAbsoluteVal.ShowWindow(SW_HIDE);
		a_wndText.ShowWindow(SW_HIDE);
		a_wndRelativeVal.ShowWindow(nShow);
	}
	else
	{
		CComBSTR bstrAbsoluteID(a_pszAbsoluteID);
		CConfigValue cAbsoluteValue;
		a_pConfig->ItemValueGet(bstrAbsoluteID, &cAbsoluteValue);
		TCHAR szTmp[32] = _T("");
		_stprintf(szTmp, _T("%g"), cAbsoluteValue.operator float());
		a_wndAbsoluteVal.SetWindowText(szTmp);
		a_wndFromBack.ShowWindow(nShow);
		a_wndAbsoluteVal.ShowWindow(nShow);
		a_wndText.ShowWindow(nShow);
		a_wndRelativeVal.ShowWindow(SW_HIDE);
	}
	a_wndAbsoluteVal.EnableWindow(a_bEnabled);
	a_wndText.EnableWindow(a_bEnabled);
	a_wndRelativeVal.EnableWindow(a_bEnabled);
	a_wndLabel.ShowWindow(nShow);
	a_wndLine.ShowWindow(nShow);
}

void CConfigGUISplitter::CConfigWnd::ValuesToGUI()
{
	m_bSetting = true;
	CConfigValue cSplitType;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_SPLITTYPE), &cSplitType);
	Value2GUICombo(m_pConfig, m_wndSplitDirection, CFGID_SPLITTYPE, m_tLocaleID);
	SetDirectionValues(m_pConfig, m_wndVerRelative, m_wndVerAdjustable, m_wndVerFromBack, m_wndVerEdit, m_wndVerStatic, m_wndVerSlider, m_wndHorLine, m_wndVerLabel, CFGID_VERDIVTYPE, CFGID_VERABSOLUTE, CFGID_VERRELATIVE, cSplitType.operator LONG() != ESTHorizontal);
	SetDirectionValues(m_pConfig, m_wndHorRelative, m_wndHorAdjustable, m_wndHorFromBack, m_wndHorEdit, m_wndHorStatic, m_wndHorSlider, m_wndVerLine, m_wndHorLabel, CFGID_HORDIVTYPE, CFGID_HORABSOLUTE, CFGID_HORRELATIVE, cSplitType.operator LONG() != ESTVertical);

	Value2GUICombo(m_pConfig, m_wndViewTypeLT, CFGID_SUBVIEWLT, m_tLocaleID);
	Value2GUICombo(m_pConfig, m_wndViewTypeLB, CFGID_SUBVIEWLB, m_tLocaleID);
	Value2GUICombo(m_pConfig, m_wndViewTypeRT, CFGID_SUBVIEWRT, m_tLocaleID);
	Value2GUICombo(m_pConfig, m_wndViewTypeRB, CFGID_SUBVIEWRB, m_tLocaleID);
	m_bSetting = false;
	if (m_eSplitType != cSplitType.operator LONG())
	{
		m_eSplitType = cSplitType;
		UpdateSubViewPositions();
		m_wndViewTypeLB.ShowWindow(m_eSplitType == ESTBoth ? SW_SHOW : SW_HIDE);
		m_wndViewTypeRT.ShowWindow(m_eSplitType == ESTBoth ? SW_SHOW : SW_HIDE);
		com_cast<IChildWindow>(m_pConfigWndLB)->Show(m_eSplitType == ESTBoth);
		com_cast<IChildWindow>(m_pConfigWndRT)->Show(m_eSplitType == ESTBoth);
	}
}

void CConfigGUISplitter::CConfigWnd::UpdateSubViewPositions()
{
	RECT rcSizes = {0, 4, 4, 12};
	MapDialogRect(&rcSizes);
	RECT rcSubViews;
	m_wndSubViews.GetWindowRect(&rcSubViews);
	ScreenToClient(&rcSubViews);
	if (m_eSplitType == ESTVertical)
	{
		rcSubViews.left = 0;
	}
	else if (m_eSplitType == ESTHorizontal)
	{
		rcSubViews.top = 0;
	}

	RECT rcTmp;
	switch (m_eSplitType)
	{
	case ESTBoth:
		rcTmp.left = rcSubViews.left;
		rcTmp.top = rcSubViews.top;
		rcTmp.right = rcSubViews.left+((rcSubViews.right-rcSubViews.left-rcSizes.right)>>1);
		rcTmp.bottom = rcSubViews.top + rcSizes.bottom;
		m_wndViewTypeLT.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.top+((rcSubViews.bottom-rcSubViews.top-rcSizes.top)>>1);
		com_cast<IChildWindow>(m_pConfigWndLT)->Move(&rcTmp);

		rcTmp.left = rcSubViews.left;
		rcTmp.top = rcSubViews.top+((rcSubViews.bottom-rcSubViews.top+rcSizes.top)>>1);
		rcTmp.right = rcSubViews.left+((rcSubViews.right-rcSubViews.left-rcSizes.right)>>1);
		rcTmp.bottom = rcTmp.top + rcSizes.bottom;
		m_wndViewTypeLB.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.bottom;
		com_cast<IChildWindow>(m_pConfigWndLB)->Move(&rcTmp);

		rcTmp.left = rcSubViews.left+((rcSubViews.right-rcSubViews.left+rcSizes.right)>>1);
		rcTmp.top = rcSubViews.top;
		rcTmp.right = rcSubViews.right;
		rcTmp.bottom = rcSubViews.top + rcSizes.bottom;
		m_wndViewTypeRT.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.top+((rcSubViews.bottom-rcSubViews.top-rcSizes.top)>>1);
		com_cast<IChildWindow>(m_pConfigWndRT)->Move(&rcTmp);

		rcTmp.left = rcSubViews.left+((rcSubViews.right-rcSubViews.left+rcSizes.right)>>1);
		rcTmp.top = rcSubViews.top+((rcSubViews.bottom-rcSubViews.top+rcSizes.top)>>1);
		rcTmp.right = rcSubViews.right;
		rcTmp.bottom = rcTmp.top + rcSizes.bottom;
		m_wndViewTypeRB.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.bottom;
		com_cast<IChildWindow>(m_pConfigWndRB)->Move(&rcTmp);
		break;

	case ESTVertical:
		rcTmp.left = rcSubViews.left;
		rcTmp.top = rcSubViews.top;
		rcTmp.right = rcSubViews.left+((rcSubViews.right-rcSubViews.left-rcSizes.right)>>1);
		rcTmp.bottom = rcSubViews.top + rcSizes.bottom;
		m_wndViewTypeLT.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.bottom;
		com_cast<IChildWindow>(m_pConfigWndLT)->Move(&rcTmp);

		rcTmp.left = rcSubViews.left+((rcSubViews.right-rcSubViews.left+rcSizes.right)>>1);
		rcTmp.right = rcSubViews.right;
		rcTmp.top = rcSubViews.top;
		rcTmp.bottom = rcSubViews.top + rcSizes.bottom;
		m_wndViewTypeRB.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.bottom;
		com_cast<IChildWindow>(m_pConfigWndRB)->Move(&rcTmp);
		break;

	case ESTHorizontal:
		rcTmp.left = rcSubViews.left;
		rcTmp.top = rcSubViews.top;
		rcTmp.right = rcSubViews.right;
		rcTmp.bottom = rcSubViews.top + rcSizes.bottom;
		m_wndViewTypeLT.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.top+((rcSubViews.bottom-rcSubViews.top-rcSizes.top)>>1);
		com_cast<IChildWindow>(m_pConfigWndLT)->Move(&rcTmp);

		rcTmp.top = rcSubViews.top+((rcSubViews.bottom-rcSubViews.top+rcSizes.top)>>1);
		rcTmp.bottom = rcTmp.top + rcSizes.bottom;
		m_wndViewTypeRB.SetWindowPos(NULL, &rcTmp, SWP_NOZORDER | SWP_NOACTIVATE);
		rcTmp.top = rcSizes.top + rcTmp.bottom;
		rcTmp.bottom = rcSubViews.bottom;
		com_cast<IChildWindow>(m_pConfigWndRB)->Move(&rcTmp);
		break;
	}
}

