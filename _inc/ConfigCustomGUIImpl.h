
#pragma once

#include <Win32LangEx.h>
#include <ObserverImpl.h>
#include <StringParsing.h>
#include <htmlhelp.h>
#include <InPlaceCalc.h>
#include <map>
#pragma comment(lib, "htmlhelp.lib")


enum ECustomConfigControlType
{
	ECCCTInvalid = 0,
	ECCCTContextHelp,
	ECCCTCheckbox,
	ECCCTCombobox,
	ECCCTEditbox,
	ECCCTSubConfig,
	ECCCTIconCombo,
	ECCCTSlider,
	ECCCTRadio,
	ECCCTVisibility,
};

struct SCustomConfigControlMap
{
	ECustomConfigControlType eControlType;
	UINT uIDC;
	LPCOLESTR pszCfgID;
	LONG nRadioOption;
	bool bHideWhenInaccessible;
};

#define BEGIN_CONFIGITEM_MAP(class) static SCustomConfigControlMap const* GetCustomConfigControlMap() { static SCustomConfigControlMap const sMap[] = {
#define END_CONFIGITEM_MAP() { ECCCTInvalid, 0} }; return sMap; }

#define CONFIGITEM_CONTEXTHELP(uIDC, pszCfgID) { ECCCTContextHelp, uIDC, pszCfgID, 0, false },
#define CONFIGITEM_CONTEXTHELP_IDS(uIDC, uIDS) { ECCCTContextHelp, uIDC, NULL, uIDS, false },
#define CONFIGITEM_CONTEXTHELP_CALLBACK(uIDC) { ECCCTContextHelp, uIDC, NULL, 0, false },
#define CONFIGITEM_CHECKBOX(uIDC, pszCfgID) { ECCCTCheckbox, uIDC, pszCfgID, 0, false },
#define CONFIGITEM_CHECKBOX_VISIBILITY(uIDC, pszCfgID) { ECCCTCheckbox, uIDC, pszCfgID, 0, true },
#define CONFIGITEM_CHECKBOX_FLAG(uIDC, pszCfgID, nValue) { ECCCTCheckbox, uIDC, pszCfgID, nValue, false },
#define CONFIGITEM_CHECKBOX_FLAG_VISIBILITY(uIDC, pszCfgID, nValue) { ECCCTCheckbox, uIDC, pszCfgID, nValue, true },
#define CONFIGITEM_COMBOBOX(uIDC, pszCfgID) { ECCCTCombobox, uIDC, pszCfgID, 0, false },
#define CONFIGITEM_COMBOBOX_VISIBILITY(uIDC, pszCfgID) { ECCCTCombobox, uIDC, pszCfgID, 0, true },
#define CONFIGITEM_EDITBOX(uIDC, pszCfgID) { ECCCTEditbox, uIDC, pszCfgID, 0, false },
#define CONFIGITEM_EDITBOX_VISIBILITY(uIDC, pszCfgID) { ECCCTEditbox, uIDC, pszCfgID, 0, true },
#define CONFIGITEM_SUBCONFIG(uIDC, pszCfgID) { ECCCTSubConfig, uIDC, pszCfgID, 0, false },
#define CONFIGITEM_SUBCONFIG_NOMARGINS(uIDC, pszCfgID) { ECCCTSubConfig, uIDC, pszCfgID, 1, false },
#define CONFIGITEM_ICONCOMBO(uIDC, pszCfgID) { ECCCTIconCombo, uIDC, pszCfgID, 0, false },
#define CONFIGITEM_SLIDER(uIDC, pszCfgID) { ECCCTSlider, uIDC, pszCfgID, 0, false },
#define CONFIGITEM_SLIDER_TRACKUPDATE(uIDC, pszCfgID) { ECCCTSlider, uIDC, pszCfgID, 1, false },
#define CONFIGITEM_RADIO(uIDC, pszCfgID, nValue) { ECCCTRadio, uIDC, pszCfgID, nValue, false },
#define CONFIGITEM_VISIBILITY(uIDC, pszCfgID) { ECCCTVisibility, uIDC, pszCfgID, 0, true },


template <class T, class TDialog = Win32LangEx::CLangDialogImpl<T> >
class ATL_NO_VTABLE CCustomConfigWndImpl :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CChildWindowImpl<T, IChildWindow>,
	public TDialog,
	public CObserverImpl<CCustomConfigWndImpl<T, TDialog>, IConfigObserver, IUnknown*>
{
public:
	typedef CCustomConfigWndImpl<T, TDialog> TThisClass;

	CCustomConfigWndImpl() : m_nDirtyEditIDC(0), m_bEnableEditUpdates(false), m_eMode(ECPMFull)
	{
	}

	void SetMode(EConfigPanelMode a_eMode) { m_eMode = a_eMode; }
	void Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode)
	{
		m_tLocaleID = a_tLocaleID;
		m_pConfig = a_pConfig;
		m_eMode = a_eMode;

		TDialog::Create(a_hParent);
		if (!IsWindow()) throw E_FAIL;

		MoveWindow(a_prcPositon);
		SetWindowLong(GWL_ID, a_nCtlID);
		ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
	}

	BEGIN_COM_MAP(TThisClass)
		COM_INTERFACE_ENTRY(IChildWindow)
	END_COM_MAP()

	BEGIN_MSG_MAP(TThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnBnClicked)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnCbnSelChange)
		COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnEnKillFocus)
		COMMAND_CODE_HANDLER(EN_CHANGE, OnEnChange)
		MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColor2Parent)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnColor2Parent)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnColor2Parent)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		NOTIFY_CODE_HANDLER(UDN_DELTAPOS, OnUpDownChange)
	END_MSG_MAP()

	enum {EDITTIMERID = 13};

	void ExtraConfigNotify() {}
	void ExtraInitDialog() {}
	void SubConfigWindowCreated(UINT nIDC, IConfigWnd* pConfigWnd) {}
	bool GetSliderRange(wchar_t const* a_pszName, TConfigValue* a_pFrom, TConfigValue* a_pTo, TConfigValue* a_pStep)
	{
		CComPtr<IConfigItemRange> pRange;
		m_pConfig->ItemGetUIInfo(CComBSTR(a_pszName), __uuidof(IConfigItemRange), reinterpret_cast<void**>(&pRange));
		return pRange ? SUCCEEDED(pRange->RangePropsGet(a_pFrom, a_pTo, a_pStep)) : false;
	}

	bool ValueToText(WCHAR const* id, TConfigValue const& val, TCHAR* text, ULONG len) { return false; }
	float ValueToFloat(WCHAR const* id, TConfigValue const& val) { return val.fVal; }
	void OwnerNotify(TCookie, IUnknown*)
	{
		try
		{
			if (!IsWindow())
				return;
			for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
			{
				if (pMap->pszCfgID == NULL)
					continue;
				CConfigValue cVal;
				m_pConfig->ItemValueGet(CComBSTR(pMap->pszCfgID), &cVal);
				switch (pMap->eControlType)
				{
				case ECCCTCheckbox:
					{
						CButton wnd(GetDlgItem(pMap->uIDC));
						if (pMap->nRadioOption)
						{
							if (cVal.TypeGet() != ECVTInteger)
							{
								if (pMap->bHideWhenInaccessible)
									wnd.ShowWindow(SW_HIDE);
								else
									wnd.EnableWindow(FALSE);
								break;
							}
							wnd.SetCheck(cVal.operator LONG()&pMap->nRadioOption ? BST_CHECKED : BST_UNCHECKED);
							CComPtr<IConfigItem> pItem;
							m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
							if (pItem == NULL || pItem->ValueIsValid(CConfigValue(pMap->nRadioOption)) == S_OK)
							{
								if (pMap->bHideWhenInaccessible)
									wnd.ShowWindow(SW_SHOW);
								else
									wnd.EnableWindow(TRUE);
							}
							else
							{
								if (pMap->bHideWhenInaccessible)
									wnd.ShowWindow(SW_HIDE);
								else
									wnd.EnableWindow(FALSE);
							}
						}
						else
						{
							if (cVal.TypeGet() != ECVTBool)
							{
								if (pMap->bHideWhenInaccessible)
									wnd.ShowWindow(SW_HIDE);
								else
									wnd.EnableWindow(FALSE);
								break;
							}
							wnd.SetCheck(cVal.operator bool() ? BST_CHECKED : BST_UNCHECKED);
							CComPtr<IConfigItem> pItem;
							m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
							if (pItem == NULL || (pItem->ValueIsValid(CConfigValue(true)) == S_OK && pItem->ValueIsValid(CConfigValue(false)) == S_OK))
							{
								if (pMap->bHideWhenInaccessible)
									wnd.ShowWindow(SW_SHOW);
								else
									wnd.EnableWindow(TRUE);
							}
							else
							{
								if (pMap->bHideWhenInaccessible)
									wnd.ShowWindow(SW_HIDE);
								else
									wnd.EnableWindow(FALSE);
							}
						}
					}
					break;
				case ECCCTCombobox:
					{
						CComboBox wnd(GetDlgItem(pMap->uIDC));
						CComPtr<IConfigItemOptions> pItem;
						m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
						CComPtr<IEnumConfigItemOptions> pOptions;
						ULONG nOptions = 0;
						if (pItem == NULL || FAILED(pItem->OptionsEnum(&pOptions)) || pOptions == NULL || FAILED(pOptions->Size(&nOptions)) || nOptions == 0)
						{
							wnd.EnableWindow(FALSE);
							if (pMap->bHideWhenInaccessible)
								wnd.ShowWindow(SW_HIDE);
							break;
						}

						wnd.EnableWindow(TRUE);
						if (pMap->bHideWhenInaccessible)
							wnd.ShowWindow(SW_SHOW);
						wnd.ResetContent();

						CConfigValue cOption;
						for (ULONG i = 0; SUCCEEDED(pOptions->Get(i, &cOption)); ++i)
						{
							int iCombo = -1;
							CComBSTR bstrText;
							CComPtr<ILocalizedString> pStr;
							if (SUCCEEDED(pItem->ValueGetName(cOption, &pStr)) &&
								SUCCEEDED(pStr->GetLocalized(m_tLocaleID, &bstrText)))
							{
								iCombo = wnd.AddString(COLE2CT(bstrText));
							}
							else
							{
								TCHAR szText[256];
								switch (cOption.TypeGet())
								{
								case ECVTInteger:
									_stprintf(szText, _T("%i"), (LONG)cOption);
									iCombo = wnd.AddString(szText);
									break;
								case ECVTFloat:
									_stprintf(szText, _T("%f"), (float)cOption);
									iCombo = wnd.AddString(szText);
									break;
								case ECVTString:
									iCombo = wnd.AddString(COLE2CT(cOption));
									break;
								case ECVTGUID:
									{
										GUID guid(cOption);
										_stprintf(szText, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
											guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
											guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
										iCombo = wnd.AddString(szText);
									}
									break;
								default:
									ATLASSERT(0);
									iCombo = wnd.AddString(_T("error"));
								}
							}
							wnd.SetItemData(iCombo, i);
							if (cOption == cVal)
								wnd.SetCurSel(iCombo);
							m_cComboOptions[pMap->uIDC] = pOptions;
						}
					}
					break;
				case ECCCTIconCombo:
					{
						CComboBoxEx wnd(GetDlgItem(pMap->uIDC));
						CComPtr<IConfigItemOptions> pItem;
						m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
						CComPtr<IEnumConfigItemOptions> pOptions;
						if (pItem == NULL || FAILED(pItem->OptionsEnum(&pOptions)) || pOptions == NULL)
						{
							wnd.EnableWindow(FALSE);
							break;
						}

						wnd.EnableWindow(TRUE);

						CConfigValue cOption;
						for (ULONG i = 0; SUCCEEDED(pOptions->Get(i, &cOption)); ++i)
						{
							if (cOption == cVal)
							{
								wnd.SetCurSel(i);
								break;
							}
						}
					}
					break;
				case ECCCTEditbox:
					{
						CEdit wnd(GetDlgItem(pMap->uIDC));
						BOOL bEnabled = TRUE;

						TCHAR szText[2048];
						if (static_cast<T*>(this)->ValueToText(pMap->pszCfgID, cVal, szText, 2048))
						{
							TextToEdit(wnd, szText);
						}
						else switch (cVal.TypeGet())
						{
						case ECVTInteger:
							//_stprintf(szText, _T("%i"), (LONG)cVal);
							//TextToEdit(wnd, szText);
							DoubleToEdit(wnd, (LONG)cVal, false);
							break;
						case ECVTFloat:
							//_stprintf(szText, _T("%g"), (float)cVal);
							//TextToEdit(wnd, szText);
							DoubleToEdit(wnd, static_cast<T*>(this)->ValueToFloat(pMap->pszCfgID, cVal), true);
							break;
						case ECVTString:
							TextToEdit(wnd, COLE2CT(cVal));
							break;
						case ECVTGUID:
							{
								GUID guid(cVal);
								_stprintf(szText, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
									guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
									guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
								TextToEdit(wnd, szText);
							}
							break;
						case ECVTVector2:
							_stprintf(szText, _T("%g, %g"), cVal[0], cVal[1]);
							TextToEdit(wnd, szText);
							break;
						case ECVTVector3:
						case ECVTFloatColor:
							_stprintf(szText, _T("%g, %g, %g"), cVal[0], cVal[1], cVal[2]);
							TextToEdit(wnd, szText);
							break;
						case ECVTVector4:
							_stprintf(szText, _T("%g, %g, %g, %g"), cVal[0], cVal[1], cVal[2], cVal[3]);
							TextToEdit(wnd, szText);
							break;
						default:
							ATLASSERT(0);
							bEnabled = FALSE;
							TextToEdit(wnd, _T(""));
						}
						CComPtr<IConfigItemSimple> pSimple;
						m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pSimple));
						if (pSimple != NULL && pSimple->IsEnabled() != S_OK)
							bEnabled = FALSE;
						wnd.EnableWindow(bEnabled);
						if (pMap->bHideWhenInaccessible)
							wnd.ShowWindow(bEnabled ? SW_SHOW : SW_HIDE);
					}
					break;
				case ECCCTSlider:
					{
						CTrackBarCtrl wnd(GetDlgItem(pMap->uIDC));
						BOOL bEnabled = TRUE;
						int nCurrent = 0;
						float fCurrent = 0.0f;
						switch (cVal.TypeGet())
						{
						case ECVTInteger:
							nCurrent = (LONG)cVal;
							break;
						case ECVTFloat:
							fCurrent = static_cast<T*>(this)->ValueToFloat(pMap->pszCfgID, cVal);
							break;
						default:
							ATLASSERT(0);
							bEnabled = FALSE;
						}

						CComPtr<IConfigItemSimple> pSimple;
						m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pSimple));
						CConfigValue tFrom, tTo, tStep;
						if (!static_cast<T*>(this)->GetSliderRange(pMap->pszCfgID, &tFrom, &tTo, &tStep) || pSimple == NULL || pSimple->IsEnabled() != S_OK)
							bEnabled = FALSE;
						wnd.EnableWindow(bEnabled);
						if (tFrom.TypeGet() != ECVTEmpty)
						{
							if (tFrom.TypeGet() == ECVTInteger && tTo.TypeGet() == ECVTInteger)
							{
								int nMin = tFrom.operator LONG();
								int nMax = tTo.operator LONG();
								wnd.GetRange(nMin, nMax);
								int nCur = wnd.GetPos();
								if (tFrom.operator LONG() != nMin || tTo.operator LONG() != nMax)
								{
									wnd.SetRange(tFrom.operator LONG(), tTo.operator LONG(), TRUE);
									wnd.SetPageSize((tTo.operator LONG()-tFrom.operator LONG())/10);
								}
								if (nCurrent != nCur)
									wnd.SetPos(nCurrent);
							}
							else if (tFrom.TypeGet() == ECVTFloat && tTo.TypeGet() == ECVTFloat)
							{
								int nMin = 0;
								int nMax = 100;
								wnd.GetRange(nMin, nMax);
								int nCur = wnd.GetPos();
								if (0 != nMin || 100 != nMax)
								{
									wnd.SetRange(0, 100, TRUE);
									wnd.SetPageSize(10);
								}
								nCurrent = (fCurrent-tFrom.operator float())*100.0f/(tTo.operator float()-tFrom.operator float())+0.5f;
								if (nCurrent != nCur)
									wnd.SetPos(nCurrent);
							}
						}
					}
					break;
				case ECCCTRadio:
					{
						CButton wnd(GetDlgItem(pMap->uIDC));
						BOOL bEnabled = TRUE;
						BOOL bChecked = FALSE;
						switch (cVal.TypeGet())
						{
						case ECVTInteger:
							bChecked = pMap->nRadioOption == (LONG)cVal;
							{
								CComPtr<IConfigItem> pItem;
								m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
								bEnabled = pItem == NULL || pItem->ValueIsValid(CConfigValue(pMap->nRadioOption)) == S_OK;
							}
							break;
						case ECVTBool:
							bChecked = bool(pMap->nRadioOption) == cVal.operator bool();
							{
								CComPtr<IConfigItem> pItem;
								m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
								bEnabled = pItem == NULL || pItem->ValueIsValid(CConfigValue(bool(pMap->nRadioOption))) == S_OK;
							}
							break;
						default:
							ATLASSERT(0);
							bEnabled = FALSE;
						}
						wnd.EnableWindow(bEnabled);
						wnd.SetCheck(bChecked ? BST_CHECKED : BST_UNCHECKED);
					}
					break;
				case ECCCTVisibility:
					{
						CComPtr<IConfigItemSimple> pSimple;
						m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pSimple));
						if (pSimple != NULL)
							GetDlgItem(pMap->uIDC).ShowWindow(pSimple->IsEnabled() == S_OK ? SW_SHOW : SW_HIDE);
					}
					break;
				}
			}
			static_cast<T*>(this)->ExtraConfigNotify();
		}
		catch (...)
		{
		}
	}

	IConfig* M_Config() const
	{
		return m_pConfig;
	}
	EConfigPanelMode M_Mode() const
	{
		return m_eMode;
	}

	int GetIconIndex(GUID const&)
	{
		return -1;
	}
	HIMAGELIST M_ImageList()
	{
		return NULL;
	}

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// TODO: ?? set control styles (no auto check, drop list combo, want return edit)

		static_cast<T*>(this)->ExtraInitDialog(); // create eventual child controls sooner than the first config change is sent

		bool bContxtTips = true;
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		if (pMgr)
		{
			CComPtr<IConfig> pCfg;
			// hacks: copied CLSID and CFGVAL
			static CLSID const tID = {0x2e85563c, 0x4ff0, 0x4820, {0xa8, 0xba, 0x1b, 0x47, 0x63, 0xab, 0xcc, 0x1c}}; // CLSID_GlobalConfigMainFrame
			pMgr->Config(tID, &pCfg);
			CConfigValue cVal;
			if (pCfg) pCfg->ItemValueGet(CComBSTR(L"CtxHelpTips"), &cVal);
			if (cVal.TypeGet() == ECVTBool) bContxtTips = cVal;
		}

		if (m_pConfig)
			m_pConfig->ObserverIns(ObserverGet(), 0);

		for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
		{
			if (pMap->eControlType == ECCCTSubConfig)
			{
				CWindow wnd(GetDlgItem(pMap->uIDC));
				RECT rc;
				wnd.GetWindowRect(&rc);
				ScreenToClient(&rc);
				CComPtr<IConfigWnd> pConfigWnd;
				RWCoCreateInstance(pConfigWnd, __uuidof(AutoConfigWnd));
				pConfigWnd->Create(m_hWnd, &rc, pMap->uIDC, m_tLocaleID, wnd.GetWindowLong(GWL_STYLE)&WS_VISIBLE, pMap->nRadioOption ? ECWBMNothing : ECWBMMarginAndOutline);
				CComPtr<IConfig> pCfg;
				m_pConfig->SubConfigGet(CComBSTR(pMap->pszCfgID), &pCfg);
				pConfigWnd->ConfigSet(pCfg, m_eMode);
				RWHWND h = NULL;
				pConfigWnd->Handle(&h);
				if (h)
				{
					::SetWindowPos(h, wnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
				}
				wnd.DestroyWindow();
				static_cast<T*>(this)->SubConfigWindowCreated(pMap->uIDC, pConfigWnd);
			}
			else if (pMap->eControlType == ECCCTIconCombo)
			{
				CComboBoxEx wndIcon(GetDlgItem(pMap->uIDC));
				wndIcon.SetImageList(static_cast<T*>(this)->M_ImageList());
				CComPtr<IConfigItemOptions> pItem;
				m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
				CComPtr<IEnumConfigItemOptions> pOptions;
				if (pItem != NULL && SUCCEEDED(pItem->OptionsEnum(&pOptions)) && pOptions != NULL)
				{
					CConfigValue cOption;
					for (ULONG i = 0; SUCCEEDED(pOptions->Get(i, &cOption)); ++i)
					{
						COMBOBOXEXITEM tItem;
						ZeroMemory(&tItem, sizeof tItem);
						tItem.mask = CBEIF_IMAGE|CBEIF_SELECTEDIMAGE|CBEIF_TEXT|CBEIF_LPARAM;
						tItem.iItem = i;
						tItem.pszText = _T("                                     ");
						tItem.iImage = tItem.iSelectedImage = static_cast<T*>(this)->GetIconIndex(cOption);
						tItem.lParam = i;
						wndIcon.InsertItem(&tItem);
					}
					m_cComboOptions[pMap->uIDC] = pOptions;
				}
			}

			if (!bContxtTips)
				continue;

			bool bDuplicate = false;
			for (SCustomConfigControlMap const* p = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap != p && !bDuplicate; ++p)
				bDuplicate = p->uIDC == pMap->uIDC;
			if (bDuplicate)
				continue;
			// tooltips
			//if (pMap->bNoToolTips)
			//	continue;
			if (pMap->eControlType == ECCCTVisibility)
				continue;
			HWND hCtl = GetDlgItem(pMap->uIDC);
			if (hCtl == NULL)
				continue;
			TCHAR szClass[128] = _T("");
			::GetClassName(hCtl, szClass, itemsof(szClass));
			if (_tcscmp(szClass, WC_COMBOBOXEX) == 0)
			{
				hCtl = reinterpret_cast<HWND>(::SendMessage(hCtl, CBEM_GETCOMBOCONTROL, 0, 0));
				if (hCtl == NULL)
					continue;
			}
			else if (_tcscmp(szClass, WC_COMBOBOX) == 0)
			{
				if ((::GetWindowLong(hCtl, GWL_STYLE)&0x3) == CBS_DROPDOWN)
				{
					HWND hEdit = ::GetWindow(hCtl, GW_CHILD);
					if (hEdit)
						hCtl = hEdit;
				}
			}
			//if (_tcscmp(szClass, TOOLBARCLASSNAME) == 0)
			//	continue; // no tooltip for toolbars (they have their own)
			CComBSTR bstrDesc;
			if (pMap->pszCfgID == NULL)
			{
				if (pMap->nRadioOption == 0)
					continue; // callback help does not work with tooltips
				OLECHAR szBuffer[1024] = L"";
				Win32LangEx::LoadStringW(_pModule->get_m_hInst(), pMap->nRadioOption, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
				if (*szBuffer == L'\0')
					continue; // strange - no help
				bstrDesc = szBuffer;
			}
			else
			{
				CComPtr<IConfigItem> pItem;
				m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
				if (pItem == NULL)
					continue;
				CComPtr<ILocalizedString> pDesc;
				pItem->NameGet(NULL, &pDesc);
				if (pDesc == NULL)
					continue;
				pDesc->GetLocalized(m_tLocaleID, &bstrDesc);
				if (bstrDesc == NULL || bstrDesc[0] == L'\0')
					continue;
			}
			AddToolTip(hCtl, bstrDesc);
		}
		OwnerNotify(0, NULL);
		m_bEnableEditUpdates = true;

		a_bHandled = FALSE;
		return 1;
	}
	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		KillTimer(EDITTIMERID);

		if (m_pConfig)
			m_pConfig->ObserverDel(ObserverGet(), 0);

		// read values from edit controls
		for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
		{
			if (pMap->eControlType == ECCCTEditbox)
			{
				CEdit wnd(GetDlgItem(pMap->uIDC));
				EditToConfig(wnd, pMap->pszCfgID);
			}
		}

		if (m_wndToolTip.IsWindow())
			m_wndToolTip.DestroyWindow();

		a_bHandled = FALSE;
		return 0;
	}
	void GetHelpString(UINT UNREF(iCtrlId), POINT UNREF(tMousePos), wchar_t* UNREF(pszBuffer), size_t UNREF(nBuffer))
	{
	}
	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		if (pHelpInfo->iContextType == HELPINFO_WINDOW && pHelpInfo->iCtrlId >= 0)
		{
			for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
			{
				if (pMap->uIDC == static_cast<UINT>(pHelpInfo->iCtrlId))
				{
					CComBSTR bstrDesc;
					if (pMap->pszCfgID == NULL)
					{
						OLECHAR szBuffer[1024] = L"";
						if (pMap->nRadioOption)
							Win32LangEx::LoadStringW(_pModule->get_m_hInst(), pMap->nRadioOption, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
						else
							static_cast<T*>(this)->GetHelpString(pHelpInfo->iCtrlId, pHelpInfo->MousePos, szBuffer, itemsof(szBuffer));
						if (*szBuffer == L'\0')
							continue; // strange - no help
						bstrDesc = szBuffer;
					}
					else
					{
						CComPtr<IConfigItem> pItem;
						m_pConfig->ItemGetUIInfo(CComBSTR(pMap->pszCfgID), __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
						if (pItem == NULL)
							return 0;
						CComPtr<ILocalizedString> pDesc;
						pItem->NameGet(NULL, &pDesc);
						if (pDesc == NULL)
							return 0;
						pDesc->GetLocalized(m_tLocaleID, &bstrDesc);
						if (bstrDesc == NULL)
							return 0;
					}
					COLE2T strDesc(bstrDesc.m_str);

					RECT rcItem;
					::GetWindowRect(static_cast<HWND>(pHelpInfo->hItemHandle), &rcItem);
					HH_POPUP hhp;
					hhp.cbStruct = sizeof(hhp);
					hhp.hinst = _pModule->get_m_hInst();
					hhp.idString = 0;
					hhp.pszText = strDesc;
					hhp.pt.x = rcItem.right;
					hhp.pt.y = rcItem.bottom;
					hhp.clrForeground = 0xffffffff;
					hhp.clrBackground = 0xffffffff;
					hhp.rcMargins.left = -1;
					hhp.rcMargins.top = -1;
					hhp.rcMargins.right = -1;
					hhp.rcMargins.bottom = -1;
					hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
					HtmlHelp(static_cast<T const*>(this)->m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD_PTR>(&hhp));
					return 0;
				}
			}
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnBnClicked(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
		{
			if (pMap->eControlType == ECCCTCheckbox && pMap->uIDC == a_wID)
			{
				CButton wnd(a_hWndCtl);
				if (pMap->nRadioOption)
				{
					CConfigValue cVal; // must not be auto-checkbox !!
					CComBSTR bstrID(pMap->pszCfgID);
					m_pConfig->ItemValueGet(bstrID, &cVal);
					if (cVal.TypeGet() == ECVTInteger)
					{
						if (wnd.GetCheck() == BST_UNCHECKED) // prev state (must not be auto-checkbox !!)
							cVal = LONG(cVal.operator LONG()|pMap->nRadioOption);
						else
							cVal = LONG(cVal.operator LONG()&~pMap->nRadioOption);
						m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cVal);
					}
					return 0;
				}
				else
				{
					CConfigValue cVal(wnd.GetCheck() == BST_UNCHECKED); // must not be auto-checkbox !!
					CComBSTR bstrID(pMap->pszCfgID);
					m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cVal);
					return 0;
				}
			}
			else if (pMap->eControlType == ECCCTRadio && pMap->uIDC == a_wID)
			{
				CComBSTR bstrID(pMap->pszCfgID); // must not be auto-radiobox !!
				CConfigValue cVal;
				m_pConfig->ItemValueGet(bstrID, &cVal);
				if (cVal.TypeGet() == ECVTInteger)
					cVal = pMap->nRadioOption;
				else if (cVal.TypeGet() == ECVTBool)
					cVal = bool(pMap->nRadioOption);
				m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cVal);
				return 0;
			}
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnCbnSelChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
		{
			if ((pMap->eControlType == ECCCTCombobox || pMap->eControlType == ECCCTIconCombo) && pMap->uIDC == a_wID)
			{
				CComboOptions::const_iterator i = m_cComboOptions.find(pMap->uIDC);
				if (i != m_cComboOptions.end())
				{
					CComboBox wnd(a_hWndCtl);
					CConfigValue cVal;
					i->second->Get(wnd.GetItemData(wnd.GetCurSel()), &cVal);
					if (cVal.TypeGet() != ECVTEmpty)
					{
						CComBSTR bstrID(pMap->pszCfgID);
						m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cVal);
					}
				}
				return 0; // assuming there is only one control associated with this cfg-item
			}
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnEnKillFocus(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
		{
			if (pMap->eControlType == ECCCTEditbox && pMap->uIDC == a_wID)
			{
				if (m_nDirtyEditIDC)
				{
					KillTimer(EDITTIMERID);
					m_nDirtyEditIDC = 0;
				}
				CEdit wnd(a_hWndCtl);
				EditToConfig(wnd, pMap->pszCfgID);
				return 0; // assuming there is only one control associated with this cfg-item
			}
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnEnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_bEnableEditUpdates)
		{
			for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
			{
				if (pMap->eControlType == ECCCTEditbox && pMap->uIDC == a_wID)
				{
					if (m_nDirtyEditIDC && m_nDirtyEditIDC != a_wID)
					{
						BOOL b;
						OnTimer(0, 0, 0, b);
					}
					m_nDirtyEditIDC = a_wID;
					SetTimer(EDITTIMERID, 750);
					return 0;
				}
			}
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bEnableEditUpdates || m_hWnd == NULL)
			return 0;

		m_bEnableEditUpdates = false;
		KillTimer(EDITTIMERID);
		if (m_nDirtyEditIDC)
		{
			CEdit wnd;
			LPCOLESTR pszID = NULL;
			for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
			{
				if (pMap->eControlType == ECCCTEditbox && pMap->uIDC == m_nDirtyEditIDC)
				{
					wnd = GetDlgItem(m_nDirtyEditIDC);
					pszID = pMap->pszCfgID;
					break;
				}
			}
			if (wnd.m_hWnd && pszID)
			{
				EditToConfig(wnd, pszID);
			}
		}
		m_bEnableEditUpdates = true;
		m_nDirtyEditIDC = 0;
		return 0;
	}
	LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);
		CEdit wndEdit((HWND)::SendMessage(a_pNMHDR->hwndFrom, UDM_GETBUDDY, 0, 0));
		if (wndEdit.m_hWnd && m_bEnableEditUpdates)
		{
			WORD wID = wndEdit.GetWindowLong(GWLP_ID);
			for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
			{
				if (pMap->eControlType == ECCCTEditbox && pMap->uIDC == wID)
				{
					CComBSTR bstrID(pMap->pszCfgID);
					CConfigValue cOldVal;
					m_pConfig->ItemValueGet(bstrID, &cOldVal);
					if (cOldVal.TypeGet() != ECVTInteger && cOldVal.TypeGet() != ECVTFloat)
						break;
					int nTextLen = wndEdit.GetWindowTextLength();
					if (nTextLen < 0)
						nTextLen = 0;
					LPTSTR psz = NULL;
					psz = new TCHAR[nTextLen+1];
					tmp deleter(psz);
					wndEdit.GetWindowText(psz, nTextLen+1);
					psz[nTextLen] = _T('\0');
					LPCTSTR p = psz;
					double const f = CInPlaceCalc::EvalExpression(psz, &p);
					if (*p == 0 || *p == _T(' '))
					{
						if (cOldVal.TypeGet() == ECVTInteger)
						{
							cOldVal = static_cast<LONG>(f+0.5);
						}
						else
						{
							cOldVal = static_cast<float>(f);
						}
					}
					double d = cOldVal.TypeGet() == ECVTInteger ? static_cast<double>(cOldVal.operator LONG()) : cOldVal.operator float();
					LONG dwMax = 0;
					LONG dwMin = 100;
					::SendMessage(a_pNMHDR->hwndFrom, UDM_GETRANGE32, reinterpret_cast<WPARAM>(&dwMin), reinterpret_cast<LPARAM>(&dwMax));
					if (dwMax > dwMin)
						d += pNMUD->iDelta;
					else
						d -= pNMUD->iDelta;
					CComPtr<IConfigItemRange> pRng;
					if (dwMax == 0 && dwMin == 100)
						m_pConfig->ItemGetUIInfo(bstrID, __uuidof(IConfigItemRange), reinterpret_cast<void**>(&pRng));
					if (pRng)
					{
						CConfigValue cFrom;
						CConfigValue cTo;
						CConfigValue cStep;
						pRng->RangePropsGet(&cFrom, &cTo, &cStep);
						if (cFrom.TypeGet() == ECVTInteger && cTo.TypeGet() == ECVTInteger)
						{
							if (d < cFrom.operator LONG())
								d = cFrom.operator LONG();
							else if (d > cTo.operator LONG())
								d = cTo.operator LONG();
						}
						else if (cFrom.TypeGet() == ECVTFloat && cTo.TypeGet() == ECVTFloat)
						{
							if (d < cFrom.operator float())
								d = cFrom.operator float();
							else if (d > cTo.operator float())
								d = cTo.operator float();
						}
					}
					else
					{
						if (dwMax < dwMin)
						{
							DWORD const d = dwMax;
							dwMax = dwMin;
							dwMin = d;
						}
						if (d < dwMin)
							d = dwMin;
						else if (d > dwMax)
							d = dwMax;
					}
					TCHAR szTmp[32] = _T("");
					_stprintf(szTmp, _T("%g"), d);
					wndEdit.SetWindowText(szTmp);
					return 0;
				}
			}
		}

		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnScroll(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		for (SCustomConfigControlMap const* pMap = static_cast<T*>(this)->GetCustomConfigControlMap(); pMap->eControlType != ECCCTInvalid; ++pMap)
		{
			if (pMap->eControlType == ECCCTSlider)
			{
				if (LOWORD(a_wParam) != SB_THUMBTRACK || pMap->nRadioOption)
				{
					CTrackBarCtrl wnd(GetDlgItem(pMap->uIDC));
					if (wnd == reinterpret_cast<HWND>(a_lParam))
					{
						CComBSTR bstrID(pMap->pszCfgID);
						CConfigValue cPrev;
						m_pConfig->ItemValueGet(bstrID, &cPrev);
						if (cPrev.TypeGet() == ECVTInteger)
						{
							CConfigValue cVal(LONG(wnd.GetPos()));
							m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cVal);
						}
						else if (cPrev.TypeGet() == ECVTFloat)
						{
							CConfigValue tFrom, tTo, tStep;
							if (static_cast<T*>(this)->GetSliderRange(bstrID, &tFrom, &tTo, &tStep))
							{
								CConfigValue cVal;
								static_cast<T*>(this)->FloatToValue(pMap->pszCfgID, tFrom.operator float()+float(wnd.GetPos()*0.01f*(tTo.operator float()-tFrom.operator float())), cVal);
								m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cVal);
							}
						}
						return 0;
					}
				}
			}
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnColor2Parent(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
	}

	bool TextToValue(WCHAR const* id, TCHAR const* text, CConfigValue const& old, CConfigValue& val) { return false; }
	void FloatToValue(WCHAR const* id, float f, CConfigValue& val) { val = f; }
	void EditToConfig(CEdit& a_wnd, LPCOLESTR a_pszCfgID)
	{
		try
		{
			int nTextLen = a_wnd.GetWindowTextLength();
			if (nTextLen < 0)
				return;
			LPTSTR psz = NULL;
			psz = new TCHAR[nTextLen+1];
			tmp deleter(psz);
			a_wnd.GetWindowText(psz, nTextLen+1);
			psz[nTextLen] = _T('\0');

			CComBSTR bstrID(a_pszCfgID);
			CConfigValue cOldVal;
			m_pConfig->ItemValueGet(bstrID, &cOldVal);

			CConfigValue cNewVal;
			if (!static_cast<T*>(this)->TextToValue(bstrID, psz, cOldVal, cNewVal))
			switch (cOldVal.TypeGet())
			{
			case ECVTString:
				cNewVal = psz;
				break;
			case ECVTInteger:
				{
					LPCTSTR p = psz;
					double const f = CInPlaceCalc::EvalExpression(psz, &p);
					LONG iTmp = f < 0 ? -static_cast<LONG>(-f+0.5) : static_cast<LONG>(f+0.5);
					if (*p == 0 || *p == _T(' '))
						cNewVal = iTmp;
				}
				break;
			case ECVTFloat:
				{
					LPCTSTR p = psz;
					float const f = CInPlaceCalc::EvalExpression(psz, &p);
					if (*p == 0 || *p == _T(' '))
						static_cast<T*>(this)->FloatToValue(bstrID, f, cNewVal);
				}
				break;
			case ECVTGUID:
				{
					ULONG nLen = _tcslen(psz);
					if (nLen >= 38 && *psz == _T('{'))
					{
						GUID t = GUID_NULL;
						if (GUIDFromString(psz+1, &t))
							cNewVal = t;
					}
					else if (nLen >= 36)
					{
						GUID t = GUID_NULL;
						if (GUIDFromString(psz, &t))
							cNewVal = t;
					}
				}
				break;
			case ECVTVector2:
				{
					float f1, f2;
					if (2 == _stscanf(psz, _T("%f, %f"), &f1, &f2))
						cNewVal = CConfigValue(f1, f2);
				}
				break;
			case ECVTVector3:
			case ECVTFloatColor:
				{
					float f1, f2, f3;
					if (3 == _stscanf(psz, _T("%f, %f, %f"), &f1, &f2, &f3))
						cNewVal = CConfigValue(f1, f2, f3, cOldVal.TypeGet() == ECVTFloatColor);
				}
				break;
			case ECVTVector4:
				{
					float f1, f2, f3, f4;
					if (4 == _stscanf(psz, _T("%f, %f, %f, %f"), &f1, &f2, &f3, &f4))
						cNewVal = CConfigValue(f1, f2, f3, f4);
				}
				break;
			}
			if (cNewVal.TypeGet() != ECVTEmpty && cNewVal != cOldVal)
			{
				m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cNewVal);
			}
		}
		catch (...)
		{
		}
	}
	void TextToEdit(CEdit& a_wnd, LPCTSTR a_psz)
	{
		//if (GetFocus() == a_wnd)
		//	return;
		int nOld = a_wnd.GetWindowTextLength();
		int nNew = _tcslen(a_psz);
		if (nNew == nOld)
		{
			if (nNew == 0)
				return;
			LPTSTR psz = new TCHAR[nOld+1];
			tmp deleter(psz);
			a_wnd.GetWindowText(psz, nOld+1);
			if (_tcscmp(psz, a_psz) == 0)
				return;
		}
		m_bEnableEditUpdates = false;
		a_wnd.SetWindowText(a_psz);
		a_wnd.SetSelAll();
		m_bEnableEditUpdates = true;
	}
	void DoubleToEdit(CEdit& a_wnd, double a_f, bool a_bFloat)
	{
		int nOld = a_wnd.GetWindowTextLength();
		if (nOld)
		{
			LPTSTR psz = new TCHAR[nOld+1];
			tmp deleter(psz);
			a_wnd.GetWindowText(psz, nOld+1);
			LPCTSTR pEnd = psz;
			double fOld = CInPlaceCalc::EvalExpression(psz, &pEnd);
			double fSmall = a_bFloat ? a_f >= 0.0 ? a_f/1e6 : -a_f/1e6 : 0.5;
			double fDiff = a_f >= fOld ? a_f-fOld : fOld-a_f;
			if (fDiff < fSmall)
				return;
		}
		m_bEnableEditUpdates = false;
		TCHAR sz[32];
		_stprintf(sz, _T("%g"), a_f);
		a_wnd.SetWindowText(sz);
		a_wnd.SetSelAll();
		m_bEnableEditUpdates = true;
	}

	void AddToolTip(HWND window, wchar_t const* desc)
	{
		if (m_wndToolTip.m_hWnd == NULL)
		{
			m_wndToolTip.Create(static_cast<T const*>(this)->m_hWnd);
			HDC hDC = ::GetDC(static_cast<T const*>(this)->m_hWnd);
			int nWidth = 420 * GetDeviceCaps(hDC, LOGPIXELSX) / 96;
			::ReleaseDC(static_cast<T const*>(this)->m_hWnd, hDC);
			m_wndToolTip.SetMaxTipWidth(nWidth);
		}
		COLE2T strDesc(desc);
		TOOLINFO tTI;
		ZeroMemory(&tTI, sizeof tTI);
		tTI.cbSize = TTTOOLINFO_V1_SIZE;
		tTI.hwnd = static_cast<T const*>(this)->m_hWnd;
		tTI.uId = reinterpret_cast<UINT_PTR>(window);
		tTI.uFlags = TTF_PARSELINKS|TTF_SUBCLASS|TTF_IDISHWND;
		tTI.lpszText = strDesc;
		m_wndToolTip.AddTool(&tTI);
	}

private:
	typedef std::map<UINT, CComPtr<IEnumConfigItemOptions> > CComboOptions;
	struct tmp
	{
		tmp(LPTSTR p) : m_p(p) {}
		~tmp() { delete[] m_p; }
		LPTSTR m_p;
	};

private:
	CComPtr<IConfig> m_pConfig;
	EConfigPanelMode m_eMode;
	CComboOptions m_cComboOptions;
	CToolTipCtrl m_wndToolTip;
	bool m_bEnableEditUpdates;
	UINT m_nDirtyEditIDC;
};

#ifdef __ATLDLGS_H__
template <class T>
class ATL_NO_VTABLE CCustomConfigResourcelessWndImpl : public CCustomConfigWndImpl<T, Win32LangEx::CLangIndirectDialogImpl<T> >
{
};
#endif

template<GUID const* t_pGUID, class TCustomConfigWnd, bool t_bRequiresMargins = true>
class ATL_NO_VTABLE CConfigCustomGUI :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigCustomGUI
{
public:
	typedef CConfigCustomGUI<t_pGUID, TCustomConfigWnd, t_bRequiresMargins> thisClass;

	static void FinalizeConfig(IConfigWithDependencies* a_pCfg)
	{
		CComObject<thisClass>* p = NULL;
		HRESULT hRes = CComObject<thisClass>::CreateInstance(&p);
		CComPtr<IConfigCustomGUI> pTmp = p;
		if (FAILED(hRes))
			throw hRes;
		hRes = a_pCfg->Finalize(pTmp);
		if (FAILED(hRes))
			throw hRes;
	}

	BEGIN_COM_MAP(thisClass)
		COM_INTERFACE_ENTRY(IConfigCustomGUI)
	END_COM_MAP()

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid)
	{
		try
		{
			*a_pguid = *t_pGUID;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(RequiresMargins)()
	{
		return t_bRequiresMargins ? S_OK : S_FALSE;
	}
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
	{
		try
		{
			CComObjectStackEx<TCustomConfigWnd> cWnd;
			cWnd.SetMode(a_eMode);
			SIZE tSize = {100, 100};
			cWnd.GetDialogSize(&tSize, LANGIDFROMLCID(a_tLocaleID));
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
			CComObject<TCustomConfigWnd>* pWnd = NULL;
			CComObject<TCustomConfigWnd>::CreateInstance(&pWnd);
			CComPtr<IChildWindow> pTmp = pWnd;

			pWnd->Create(a_hParent, a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_bParentBorder, a_pConfig, a_eMode);

			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
};

