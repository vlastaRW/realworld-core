
#pragma once

#include <math.h>
#include <InPlaceCalc.h>

class ATL_NO_VTABLE CConfigGUISliderDlg :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CChildWindowImpl<CConfigGUISliderDlg, IChildWindow>,
	public Win32LangEx::CLangDialogImpl<CConfigGUISliderDlg>,
	public CObserverImpl<CConfigGUISliderDlg, IConfigObserver, IUnknown*>,
	public CDialogResize<CConfigGUISliderDlg>
{
public:
	CConfigGUISliderDlg() : m_bEnableUpdates(true)
	{
	}
	~CConfigGUISliderDlg()
	{
		if (m_pConfig)
			m_pConfig->ObserverDel(ObserverGet(), 0);
	}
	void Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, float a_fMin, float a_fMax, LONG a_eScale, BSTR a_bstrID)
	{
		m_tLocaleID = a_tLocaleID;
		m_pConfig = a_pConfig;
		if (m_pConfig)
			m_pConfig->ObserverIns(ObserverGet(), 0);
		m_fMin = a_fMin;
		m_fMax = a_fMax;
		m_eScale = a_eScale;
		m_bstrParName = a_bstrID;

		Win32LangEx::CLangDialogImpl<CConfigGUISliderDlg>::Create(a_hParent);
		if (!IsWindow()) throw E_FAIL;

		MoveWindow(a_prcPositon);
		SetWindowLong(GWL_ID, a_nCtlID);
		ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
	}

	enum { IDD = IDD_CONFIGGUI_SLIDER };

	BEGIN_COM_MAP(CConfigGUISliderDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
	END_COM_MAP()

	BEGIN_MSG_MAP(CConfigGUISliderDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISliderDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_1DRT_EDIT, EN_CHANGE, OnEditChanged)
		COMMAND_HANDLER(IDC_1DRT_EDIT, EN_KILLFOCUS, OnEditKillFocus)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISliderDlg)
		DLGRESIZE_CONTROL(IDC_1DRT_PARAMETER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_1DRT_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_1DRT_EDIT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	void OwnerNotify(TCookie, IUnknown*)
	{
		try
		{
			if (m_pfnToSlider && m_pfnFromSlider && m_wndSlider.m_hWnd)
			{
				CConfigValue cValue;
				m_pConfig->ItemValueGet(m_bstrParName, &cValue);
				int nNew = 0;
				m_pfnToSlider(cValue, m_fMin, m_fMax, &nNew);
				if (nNew != m_wndSlider.GetPos())
				{
					m_wndSlider.SetPos(nNew);
					TCHAR szTmp[32];
					m_pfnFromSlider(nNew, m_fMin, m_fMax, cValue, szTmp);
					m_bEnableUpdates = false;
					m_wndEdit.SetWindowText(szTmp);
					m_bEnableUpdates = true;
				}
			}
		}
		catch (...)
		{
		}
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_wndSlider = GetDlgItem(IDC_1DRT_SLIDER);
		m_wndEdit = GetDlgItem(IDC_1DRT_EDIT);

		CConfigValue cValue;
		m_pConfig->ItemValueGet(m_bstrParName, &cValue);
		if (cValue.TypeGet() == ECVTInteger)
		{
			LONG nLower = m_fMin+0.5f;
			LONG nUpper = m_fMax+0.5f;
			if (nLower == nUpper)
			{
				CComPtr<IConfigItemRange> pItemInfo;
				m_pConfig->ItemGetUIInfo(m_bstrParName, __uuidof(IConfigItemRange), reinterpret_cast<void**>(&pItemInfo));
				if (pItemInfo)
				{
					CConfigValue cFrom, cTo, cStep;
					pItemInfo->RangePropsGet(&cFrom, &cTo, &cStep);
					if (cFrom.TypeGet() == ECVTInteger && cTo.TypeGet() == ECVTInteger)
					{
						nLower = cFrom;
						nUpper = cTo;
					}
				}
			}
			if (nLower == nUpper)
			{
				nLower = cValue.operator LONG() - 10;
				nUpper = cValue.operator LONG() + 10;
			}
			m_fMin = nLower;
			m_fMax = nUpper;
			m_wndSlider.SetRange(nLower, nUpper);
			m_pfnFromSlider = IntegerFromSlider;
			m_pfnToSlider = IntegerToSlider;
			m_pfnFromEdit = IntegerFromEdit;
		}
		else if (cValue.TypeGet() == ECVTFloat)
		{
			if (m_fMin == m_fMax)
			{
				CComPtr<IConfigItemRange> pItemInfo;
				m_pConfig->ItemGetUIInfo(m_bstrParName, __uuidof(IConfigItemRange), reinterpret_cast<void**>(&pItemInfo));
				if (pItemInfo)
				{
					CConfigValue cFrom, cTo, cStep;
					pItemInfo->RangePropsGet(&cFrom, &cTo, &cStep);
					if (cFrom.TypeGet() == ECVTFloat && cTo.TypeGet() == ECVTFloat)
					{
						m_fMin = cFrom;
						m_fMax = cTo;
					}
				}
			}
			if (m_fMin == m_fMax)
			{
				m_fMin = cValue.operator float() - 10.0f;
				m_fMax = cValue.operator float() + 10.0f;
			}
			m_wndSlider.SetRange(0, 100);
			if (m_eScale == CFGVAL_PS_LINEAR)
			{
				m_pfnFromSlider = FloatFromSlider;
				m_pfnToSlider = FloatToSlider;
			}
			else
			{
				if (m_fMin < 0.0001f) m_fMin = 0.1f;
				if (m_fMax < m_fMin) m_fMax = m_fMin*100.0f;
				m_pfnFromSlider = LogFloatFromSlider;
				m_pfnToSlider = LogFloatToSlider;
			}
			m_pfnFromEdit = FloatFromEdit;
		}
		else
		{
			m_pfnFromEdit = DummyFromEdit;
			m_pfnToSlider = DummyToSlider;
			m_pfnFromSlider = DummyFromSlider;
			m_wndEdit.EnableWindow(FALSE);
			m_wndSlider.EnableWindow(FALSE);
		}

		int nPos = 0;
		m_pfnToSlider(cValue, m_fMin, m_fMax, &nPos);
		m_wndSlider.SetPos(nPos);
		TCHAR szTmp[32];
		m_pfnFromSlider(nPos, m_fMin, m_fMax, cValue, szTmp);
		m_bEnableUpdates = false;
		m_wndEdit.SetWindowText(szTmp);
		m_bEnableUpdates = true;

		CComPtr<IConfigItem> pItemInfo;
		m_pConfig->ItemGetUIInfo(m_bstrParName, __uuidof(IConfigItem), reinterpret_cast<void**>(&pItemInfo));
		if (pItemInfo)
		{
			CComPtr<ILocalizedString> pName;
			pItemInfo->NameGet(&pName, &m_pParDesc);
			CComBSTR bstr;
			if (pName) pName->GetLocalized(m_tLocaleID, &bstr);
			SetDlgItemText(IDC_1DRT_PARAMETER, COLE2CT(bstr));
		}

		CComPtr<IGlobalConfigManager> pMgr;
		if (m_pParDesc) RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		if (pMgr)
		{
			CComPtr<IConfig> pCfg;
			// hacks: copied CLSID and CFGVAL
			static CLSID const tID = {0x2e85563c, 0x4ff0, 0x4820, {0xa8, 0xba, 0x1b, 0x47, 0x63, 0xab, 0xcc, 0x1c}}; // CLSID_GlobalConfigMainFrame
			pMgr->Config(tID, &pCfg);
			CConfigValue cVal;
			if (pCfg) pCfg->ItemValueGet(CComBSTR(L"CtxHelpTips"), &cVal);
			if (cVal.TypeGet() == ECVTBool && cVal.operator bool())
			{
				CComBSTR bstr;
				m_pParDesc->GetLocalized(m_tLocaleID, &bstr);
				if (bstr != NULL && bstr[0] != L'\0')
				{
					m_wndToolTip.Create(m_hWnd);
					HDC hDC = ::GetDC(m_hWnd);
					int nWidth = 420 * GetDeviceCaps(hDC, LOGPIXELSX) / 96;
					::ReleaseDC(m_hWnd, hDC);
					m_wndToolTip.SetMaxTipWidth(nWidth);
					COLE2T strDesc(bstr);
					TOOLINFO tTI;
					ZeroMemory(&tTI, sizeof tTI);
					tTI.cbSize = TTTOOLINFO_V1_SIZE;
					tTI.hwnd = m_hWnd;
					tTI.uId = reinterpret_cast<UINT_PTR>(m_wndSlider.m_hWnd);
					tTI.uFlags = TTF_PARSELINKS|TTF_SUBCLASS|TTF_IDISHWND;
					tTI.lpszText = strDesc;
					m_wndToolTip.AddTool(&tTI);
					tTI.uId = reinterpret_cast<UINT_PTR>(m_wndEdit.m_hWnd);
					m_wndToolTip.AddTool(&tTI);
				}
			}
		}

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pParDesc)
		{
			if (static_cast<UINT>(pHelpInfo->iCtrlId) == IDC_1DRT_SLIDER ||
				static_cast<UINT>(pHelpInfo->iCtrlId) == IDC_1DRT_EDIT ||
				static_cast<UINT>(pHelpInfo->iCtrlId) == IDC_1DRT_PARAMETER)
			{
				CComBSTR bstr;
				m_pParDesc->GetLocalized(m_tLocaleID, &bstr);
				COLE2CT str(bstr);
				RECT rcItem;
				::GetWindowRect(static_cast<HWND>(pHelpInfo->hItemHandle), &rcItem);
				HH_POPUP hhp;
				hhp.cbStruct = sizeof(hhp);
				hhp.hinst = _pModule->get_m_hInst();
				hhp.idString = 0;
				hhp.pszText = str;
				hhp.pt.x = rcItem.right;
				hhp.pt.y = rcItem.bottom;
				hhp.clrForeground = 0xffffffff;
				hhp.clrBackground = 0xffffffff;
				hhp.rcMargins.left = -1;
				hhp.rcMargins.top = -1;
				hhp.rcMargins.right = -1;
				hhp.rcMargins.bottom = -1;
				hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
				HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
				return 0;
			}
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnHScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CConfigValue cVal;
		m_pConfig->ItemValueGet(m_bstrParName, &cVal);
		TCHAR szTmp[32];
		m_pfnFromSlider(m_wndSlider.GetPos(), m_fMin, m_fMax, cVal, szTmp);
		m_pConfig->ItemValuesSet(1, &(m_bstrParName.m_str), cVal);
		m_bEnableUpdates = false;
		m_wndEdit.SetWindowText(szTmp);
		m_bEnableUpdates = true;
		return 0;
	}
	LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		GUI2Data();
		return 0;
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		KillTimer(71);
		if (m_wndToolTip.IsWindow())
			m_wndToolTip.DestroyWindow();
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnEditChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_bEnableUpdates)
			SetTimer(71, 750);
		return 0;
	}
	LRESULT OnEditKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		GUI2Data();
		return 0;
	}
	void GUI2Data()
	{
		if (m_bEnableUpdates && m_hWnd)
		{
			m_bEnableUpdates = false;
			KillTimer(71);
			CConfigValue cVal;
			m_pConfig->ItemValueGet(m_bstrParName, &cVal);
			TCHAR szTmp[32] = _T("");
			GetDlgItemText(IDC_1DRT_EDIT, szTmp, itemsof(szTmp));
			szTmp[itemsof(szTmp)-1] = _T('\0');
			m_pfnFromEdit(szTmp, cVal);
			int nPos = 0;
			m_pfnToSlider(cVal, m_fMin, m_fMax, &nPos);
			m_wndSlider.SetPos(nPos);
			m_pConfig->ItemValuesSet(1, &(m_bstrParName.m_str), cVal);
			m_bEnableUpdates = true;
		}
	}

private:
	typedef void (FromSlider)(int a_nSlider, float a_fMin, float a_fMax, CConfigValue& a_cVal, TCHAR* a_psz);
	typedef void (ToSlider)(CConfigValue const& a_cVal, float a_fMin, float a_fMax, int* a_pnSlider);
	typedef void (FromEdit)(TCHAR* a_psz, CConfigValue& a_cVal);

	static void IntegerFromSlider(int a_nSlider, float a_fMin, float a_fMax, CConfigValue& a_cVal, TCHAR* a_psz)
	{
		a_cVal = LONG(a_nSlider);
		_stprintf(a_psz, _T("%i"), a_nSlider);
	}
	static void FloatFromSlider(int a_nSlider, float a_fMin, float a_fMax, CConfigValue& a_cVal, TCHAR* a_psz)
	{
		float const f = (a_nSlider*a_fMax + (100-a_nSlider)*a_fMin)/100.0f;
		a_cVal = f;
		_stprintf(a_psz, _T("%g"), f);
	}
	static void LogFloatFromSlider(int a_nSlider, float a_fMin, float a_fMax, CConfigValue& a_cVal, TCHAR* a_psz)
	{
		float const f = a_fMin * powf(a_fMax/a_fMin, a_nSlider/100.0f);
		a_cVal = f;
		_stprintf(a_psz, _T("%g"), f);
	}

	static void IntegerToSlider(CConfigValue const& a_cVal, float a_fMin, float a_fMax, int* a_pnSlider)
	{
		*a_pnSlider = a_cVal.operator LONG();
		if (*a_pnSlider < a_fMin) *a_pnSlider = a_fMin;
		else if (*a_pnSlider > a_fMax) *a_pnSlider = a_fMax;
	}
	static void FloatToSlider(CConfigValue const& a_cVal, float a_fMin, float a_fMax, int* a_pnSlider)
	{
		*a_pnSlider = (a_cVal.operator float()-a_fMin)*100.0f/(a_fMax-a_fMin) + 0.5f;
		if (*a_pnSlider < 0) *a_pnSlider = 0;
		else if (*a_pnSlider > 100) *a_pnSlider = 100;
	}
	static void LogFloatToSlider(CConfigValue const& a_cVal, float a_fMin, float a_fMax, int* a_pnSlider)
	{
		*a_pnSlider = logf(a_cVal.operator float()/a_fMin)/logf(a_fMax/a_fMin)*100.0f + 0.5f;
		if (*a_pnSlider < 0) *a_pnSlider = 0;
		else if (*a_pnSlider > 100) *a_pnSlider = 100;
	}

	static void IntegerFromEdit(TCHAR* a_psz, CConfigValue& a_cVal)
	{
		LPCTSTR p = a_psz;
		double d = CInPlaceCalc::EvalExpression(a_psz, &p);
		if (p == a_psz || *p)
			return; // probably parsing error
		a_cVal = LONG(floor(d+0.5));
	}
	static void FloatFromEdit(TCHAR* a_psz, CConfigValue& a_cVal)
	{
		LPCTSTR p = a_psz;
		double d = CInPlaceCalc::EvalExpression(a_psz, &p);
		if (p == a_psz || *p)
			return; // probably parsing error
		a_cVal = float(d);
	}

	static void DummyFromEdit(TCHAR* a_psz, CConfigValue& a_cVal)
	{
	}
	static void DummyToSlider(CConfigValue const& a_cVal, float a_fMin, float a_fMax, int* a_pnSlider)
	{
		*a_pnSlider = 0;
	}
	static void DummyFromSlider(int a_nSlider, float a_fMin, float a_fMax, CConfigValue& a_cVal, TCHAR* a_psz)
	{
		*a_psz = _T('\0');
	}

private:
	CComPtr<IConfig> m_pConfig;
	CTrackBarCtrl m_wndSlider;
	CEdit m_wndEdit;
	CComPtr<ILocalizedString> m_pParDesc;
	CComBSTR m_bstrParName;
	bool m_bEnableUpdates;

	FromSlider* m_pfnFromSlider;
	FromEdit* m_pfnFromEdit;
	ToSlider* m_pfnToSlider;
	float m_fMin;
	float m_fMax;
	LONG m_eScale;

	CToolTipCtrl m_wndToolTip;
};

class ATL_NO_VTABLE CSimplifiedConfig :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfig,
	public IConfigCustomGUI
{
public:
	void Init(IConfig* a_pConfig, BSTR a_bstrID, float a_fMin, float a_fMax, LONG a_eScale, GUID const& a_tConfigID)
	{
		m_pOrig = a_pConfig;
		m_bstrID = a_bstrID;
		m_fMin = a_fMin;
		m_fMax = a_fMax;
		m_eScale = a_eScale;
		m_tConfigID = a_tConfigID;
	}

BEGIN_COM_MAP(CSimplifiedConfig)
	COM_INTERFACE_ENTRY(IConfig)
	COM_INTERFACE_ENTRY(IConfigCustomGUI)
END_COM_MAP()

	// IConfig methods
public:
	STDMETHOD(ItemIDsEnum)(IEnumStrings** a_ppIDs)
	{
		try
		{
			*a_ppIDs = NULL;
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			pTmp->Insert(m_bstrID);
			*a_ppIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppIDs ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ItemValueGet)(BSTR a_bstrID, TConfigValue* a_ptValue)
	{
		return m_pOrig->ItemValueGet(a_bstrID, a_ptValue);
	}
	STDMETHOD(ItemValuesSet)(ULONG a_nCount, BSTR* a_aIDs, TConfigValue const* a_atValues)
	{
		return m_pOrig->ItemValuesSet(a_nCount, a_aIDs, a_atValues);
	}
	STDMETHOD(ItemGetUIInfo)(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
	{
		return m_pOrig->ItemGetUIInfo(a_bstrID, a_iidInfo, a_ppItemInfo);
	}
	STDMETHOD(SubConfigGet)(BSTR a_bstrID, IConfig** a_ppSubConfig)
	{
		return m_pOrig->SubConfigGet(a_bstrID, a_ppSubConfig);
	}
	STDMETHOD(DuplicateCreate)(IConfig **a_ppCopiedConfig)
	{
		try
		{
			*a_ppCopiedConfig = NULL;
			CComObject<CSimplifiedConfig>* pCopy = NULL;
			CComObject<CSimplifiedConfig>::CreateInstance(&pCopy);
			CComPtr<IConfig> pTmp = pCopy;
			CComPtr<IConfig> pOrigCopy;
			m_pOrig->DuplicateCreate(&pOrigCopy);
			pCopy->Init(pOrigCopy, m_bstrID, m_fMin, m_fMax, m_eScale, m_tConfigID);
			*a_ppCopiedConfig = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppCopiedConfig ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(CopyFrom)(IConfig *a_pSource, BSTR a_bstrIDPrefix)
	{
		try
		{
			if (a_bstrIDPrefix)
			{
				if (wcsncpy(a_bstrIDPrefix, m_bstrID, wcslen(a_bstrIDPrefix)))
					return S_FALSE;
			}
			CConfigValue cVal;
			if (SUCCEEDED(a_pSource->ItemValueGet(m_bstrID, &cVal)))
				return m_pOrig->ItemValuesSet(1, &(m_bstrID.m_str), cVal);
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ObserverIns)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pOrig->ObserverIns(a_pObserver, a_tCookie);
	}
	STDMETHOD(ObserverDel)(IConfigObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pOrig->ObserverDel(a_pObserver, a_tCookie);
	}

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid)
	{
		*a_pguid = m_tConfigID;
		return S_OK;
	}
	STDMETHOD(RequiresMargins)()
	{
		return S_OK;
	}
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
	{
		try
		{
			SIZE tSize = {100, 100};
			Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD_CONFIGGUI_SLIDER, &tSize, a_tLocaleID);
			*a_nSizeX = tSize.cx;
			*a_nSizeY = tSize.cy;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
	{
		try
		{
			CComObject<CConfigGUISliderDlg>* pWnd = NULL;
			CComObject<CConfigGUISliderDlg>::CreateInstance(&pWnd);
			CComPtr<IChildWindow> pTmp = pWnd;

			pWnd->Create(a_hParent, a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_bParentBorder, a_pConfig, m_fMin, m_fMax, m_eScale, m_bstrID);

			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

private:
	CComPtr<IConfig> m_pOrig;
	CComBSTR m_bstrID;
	float m_fMin;
	float m_fMax;
	LONG m_eScale;
	GUID m_tConfigID;
};

