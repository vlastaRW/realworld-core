
#include <ConfigCustomGUIImpl.h>

struct SLangLess { bool operator()(WORD a_w1, WORD a_w2) const {return PRIMARYLANGID(a_w1) < PRIMARYLANGID(a_w2) || (PRIMARYLANGID(a_w1) == PRIMARYLANGID(a_w2) && SUBLANGID(a_w1) < SUBLANGID(a_w2));} };
typedef std::set<WORD, SLangLess> CLGIDs;
static CLGIDs g_cLGIDs; // TODO: remove global variable

BOOL CALLBACK EnumAppLangs(LPTSTR lpLocaleString)
{
	DWORD i;
	_stscanf(lpLocaleString, _T("%x"), &i);
	g_cLGIDs.insert(i);
	return TRUE;
}

static OLECHAR const CFGID_TRANSLATIONMODE[] = L"Translating";
static OLECHAR const CFGID_HIGHLIGHTUNTRANSLATED[] = L"Highlight";
static OLECHAR const CFGID_TRANSLATIONFILTER[] = L"Filter";
static OLECHAR const CFGID_HIDETRANSLATED[] = L"HideTr";

class ATL_NO_VTABLE CConfigGUITranslatorDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUITranslatorDlg>,
	public CDialogResize<CConfigGUITranslatorDlg>
{
public:
	CConfigGUITranslatorDlg() : m_wndTranslation(this, 1),
		m_nTimestamp(0), m_nStandard(0), m_nCustom(0), m_nMissing(0),
		m_wListLang(0), m_nSelectedString(-1), m_bBuiltIn(FALSE)
	{
	}
	~CConfigGUITranslatorDlg()
	{
		m_cLanguageIcons.Destroy();
	}
	enum
	{
		IDC_CFGID_LANGUAGECODE = 100,
		IDC_LANGUAGEINFO,
		IDC_DOWNLOADLANGUAGE,
		IDC_TRANSLATORMODE,
		IDC_SEPLINE,
		IDC_HIGHLIGHTMISSING,
		IDC_TRANSLATIONHELP,
		IDC_FILTERLABEL,
		IDC_FILTERWORD,
		IDC_FILTERTRANSLATED,
		IDC_STRINGLIST,
		IDC_ORIGINALLABEL,
		IDC_ORIGINALSTRING,
		IDC_TRANSLATEDLABEL,
		IDC_TRANSLATEDSTRING,
	};

	BEGIN_DIALOG_EX(0, 0, 219, 171, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Language:[0405]Jazyk:"), IDC_STATIC, 0, 2, 58, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CFGID_LANGUAGECODE, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 60, 0, 159, 145, 0)
		CONTROL_LTEXT(_T(""), IDC_LANGUAGEINFO, 0, 16, 165, 16, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Download[0405]Stáhnout"), IDC_DOWNLOADLANGUAGE, 169, 17, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Translator mode[0405]Překladový režim"), IDC_TRANSLATORMODE, 0, 36, 80, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CONTROL(_T(""), IDC_SEPLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 80, 41, 139, 1, 0)
		CONTROL_CHECKBOX(_T("[0409]Highlight missing strings[0405]Zvýraznit chybějící výrazy"), IDC_HIGHLIGHTMISSING, 0, 50, 120, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_RTEXT(_T("[0409]Learn about translating.[0405]Zjistěte více o překladech."), IDC_TRANSLATIONHELP, 130, 50, 89, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Filter:[0405]Filtr:"), IDC_FILTERLABEL, 0, 66, 30, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_FILTERWORD, 40, 64, 60, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CHECKBOX(_T("[0409]Hide translated strings[0405]Schovat přeložené výrazy"), IDC_FILTERTRANSLATED, 104, 65, 100, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CONTROL(_T(""), IDC_STRINGLIST, WC_LISTVIEW, /*LVS_LIST | */LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_ALIGNLEFT | LVS_NOCOLUMNHEADER | WS_BORDER | WS_TABSTOP | WS_VISIBLE, 0, 80, 100, 90, 0)
		CONTROL_LTEXT(_T("[0409]Original string:[0405]Původní výraz"), IDC_ORIGINALLABEL, 104, 80, 100, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ORIGINALSTRING, 104, 90, 115, 33, ES_MULTILINE | ES_READONLY | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Translated string:[0405]Přeložený výraz"), IDC_TRANSLATEDLABEL, 104, 127, 100, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_TRANSLATEDSTRING, 104, 137, 115, 33, ES_MULTILINE | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUITranslatorDlg)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUITranslatorDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUITranslatorDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CFGID_LANGUAGECODE, CBN_SELCHANGE, OnLanguageChange)
		COMMAND_HANDLER(IDC_TRANSLATEDSTRING, EN_CHANGE, OnTranslationChange)
		COMMAND_HANDLER(IDC_DOWNLOADLANGUAGE, BN_CLICKED, OnDownloadClicked)
		NOTIFY_HANDLER(IDC_STRINGLIST, LVN_ITEMCHANGED, OnStringListItemChanged)
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUITranslatorDlg)
		DLGRESIZE_CONTROL(IDC_CFGID_LANGUAGECODE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_LANGUAGEINFO, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_DOWNLOADLANGUAGE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SEPLINE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TRANSLATIONHELP, DLSZ_MOVE_X)

		DLGRESIZE_CONTROL(IDC_STRINGLIST, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_ORIGINALSTRING, DLSZ_SIZE_X|DLSZ_DIVSIZE_Y(2))
		DLGRESIZE_CONTROL(IDC_TRANSLATEDLABEL, DLSZ_DIVMOVE_Y(2))
		DLGRESIZE_CONTROL(IDC_TRANSLATEDSTRING, DLSZ_SIZE_X|DLSZ_DIVSIZE_Y(2)|DLSZ_DIVMOVE_Y(2))
		//DLGRESIZE_CONTROL(IDC_FILTERWORD, DLSZ_DIVSIZE_X(2))
		//DLGRESIZE_CONTROL(IDC_FILTERTRANSLATED, DLSZ_DIVMOVE_X(2))
		//DLGRESIZE_CONTROL(IDC_STRINGLIST, DLSZ_DIVSIZE_X(2)|DLSZ_SIZE_Y)
		//DLGRESIZE_CONTROL(IDC_ORIGINALLABEL, DLSZ_DIVMOVE_X(2))
		//DLGRESIZE_CONTROL(IDC_ORIGINALSTRING, DLSZ_DIVSIZE_X(2)|DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_Y(2))
		//DLGRESIZE_CONTROL(IDC_TRANSLATEDLABEL, DLSZ_DIVMOVE_X(2)|DLSZ_DIVMOVE_Y(2))
		//DLGRESIZE_CONTROL(IDC_TRANSLATEDSTRING, DLSZ_DIVSIZE_X(2)|DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_Y(2)|DLSZ_DIVMOVE_Y(2))
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUITranslatorDlg)
		CONFIGITEM_CONTEXTHELP(IDC_CFGID_LANGUAGECODE, CFGID_LANGUAGECODE)
		CONFIGITEM_CHECKBOX(IDC_TRANSLATORMODE, CFGID_TRANSLATIONMODE)
		CONFIGITEM_CHECKBOX(IDC_HIGHLIGHTMISSING, CFGID_HIGHLIGHTUNTRANSLATED)
		CONFIGITEM_EDITBOX(IDC_FILTERWORD, CFGID_TRANSLATIONFILTER)
		CONFIGITEM_CHECKBOX(IDC_FILTERTRANSLATED, CFGID_HIDETRANSLATED)
		//CONFIGITEM_CONTEXTHELP_IDS(IDC_APPOPT_UNDOMODE_GRP, IDS_CFGID_UNDOMODE_DESC)
		//CONFIGITEM_RADIO(IDC_APPOPT_UNDODEFAULT, CFGID_UNDOMODE, EUMDefault)
	END_CONFIGITEM_MAP()


	void ExtraInitDialog()
	{
		RWCoCreateInstance(m_pTranslator, __uuidof(Translator));

		m_wndList = GetDlgItem(IDC_STRINGLIST);
		m_wndList.AddColumn(_T(""), 0);
		m_wndList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		m_wndTranslation.SubclassWindow(GetDlgItem(IDC_TRANSLATEDSTRING));

		if (g_cLGIDs.empty())
		{
			EnumSystemLocales(EnumAppLangs, LCID_SUPPORTED);
			//EnumSystemLanguageGroups(EnumLanguageGroupsProc, LGRPID_SUPPORTED, reinterpret_cast<LONG_PTR>(this));
		}

		m_wndLanguages = GetDlgItem(IDC_CFGID_LANGUAGECODE);

		m_cLanguageIcons.Create(16, 16, XPGUI::GetImageListColorFlags(), 4, 4);
		m_wndLanguages.SetImageList(m_cLanguageIcons);

		COMBOBOXEXITEM tItem;
		ZeroMemory(&tItem, sizeof tItem);
		TCHAR szBuffer[280] = _T("");
		tItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
		tItem.iItem = 0;
		tItem.pszText = szBuffer;

		CLGIDs::const_iterator i;
		int iIcon = 1;
		int nSelect = 0;
		for (i = g_cLGIDs.begin(); i != g_cLGIDs.end(); i++)
		{
			tItem.lParam = *i;
			HICON hIcon = NULL;
			m_pTranslator->LangIcon(*i, &hIcon);
			if (hIcon)
			{
				tItem.iImage = tItem.iSelectedImage = m_cLanguageIcons.AddIcon(hIcon);
				DestroyIcon(hIcon);
			}
			else
			{
				tItem.iImage = tItem.iSelectedImage = I_IMAGENONE;
			}
			if (*i == 0x2c09)
			{
				_tcscpy(szBuffer, _T("English (Easter Egg Island) - [2c09]"));
			}
			else if (XPGUI::IsWin7())
			{
				TCHAR szNDN[128] = _T("");
				GetLocaleInfo(*i, 0x00000073/*LOCALE_SNATIVEDISPLAYNAME*/, szNDN, itemsof(szNDN));
				_sntprintf(szBuffer, itemsof(szBuffer), _T("%s\u200e - [%04x]"), szNDN, static_cast<DWORD>(*i));
			}
			else
			{
				TCHAR szLang[128] = _T("");
				TCHAR szCtry[128] = _T("");
				GetLocaleInfo(*i, LOCALE_SNATIVELANGNAME, szLang, itemsof(szLang));
				GetLocaleInfo(*i, LOCALE_SNATIVECTRYNAME, szCtry, itemsof(szCtry));
				_sntprintf(szBuffer, itemsof(szBuffer), XPGUI::IsVista() ? _T("%s \u200e(%s\u200e) - [%04x]") : _T("%s (%s) - [%04x]"), szLang, szCtry, static_cast<DWORD>(*i));
			}
			m_wndLanguages.InsertItem(&tItem);
			tItem.iItem++;
		}
	}
	void ExtraConfigNotify()
	{
		CConfigValue cVal;
		M_Config()->ItemValueGet(CComBSTR(CFGID_LANGUAGECODE), &cVal);
		WORD wSelection = cVal.operator LONG();
		int nItems = m_wndLanguages.GetCount();
		for (int i = 0; i < nItems; ++i)
		{
			if (m_wndLanguages.GetItemData(i) == wSelection)
			{
				m_wndLanguages.SetCurSel(i);
				break;
			}
		}
		M_Config()->ItemValueGet(CComBSTR(CFGID_TRANSLATIONMODE), &cVal);
		BOOL bEnable = cVal.operator bool();
		static UINT const aIDs[] = { IDC_FILTERLABEL, IDC_STRINGLIST, IDC_ORIGINALLABEL, IDC_ORIGINALSTRING, IDC_TRANSLATEDLABEL, IDC_TRANSLATEDSTRING, IDC_TRANSLATIONHELP };
		for (UINT const* p = aIDs; p != aIDs+itemsof(aIDs); ++p)
			GetDlgItem(*p).EnableWindow(bEnable);
		if (m_wListLang != wSelection)
		{
			m_wListLang = wSelection;
			m_bBuiltIn = FALSE;
			m_nTimestamp = m_nStandard = m_nCustom = m_nMissing = 0;
			m_pTranslator->LangInfo(m_wListLang, &m_bBuiltIn, &m_nTimestamp, &m_nStandard, &m_nCustom, &m_nMissing);
			UpdateLanguageInfo();
			InitStringCache(m_wListLang);
			m_cFilter = CConfigValue();
			m_cHideTranslated = CConfigValue();
		}
		InitStringList();
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_wndHelpLink.SubclassWindow(GetDlgItem(IDC_TRANSLATIONHELP));
		m_wndHelpLink.SetHyperLink(_T("http://www.rw-designer.com/application-translation"));

		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = CDialogResize<CConfigGUITranslatorDlg>::OnSize(a_uMsg, a_wParam, a_lParam, a_bHandled);
		RECT rc;
		m_wndList.GetClientRect(&rc);
		LVCOLUMN tCol;
		tCol.mask = LVCF_WIDTH;
		tCol.cx = rc.right-GetSystemMetrics(SM_CXVSCROLL);
		m_wndList.SetColumn(0, &tCol);
		return lRes;
	}

	LRESULT OnLanguageChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		m_nSelectedString = -1;
		CConfigValue cLanguage(static_cast<LONG>(m_wndLanguages.GetItemData(m_wndLanguages.GetCurSel())));
		CComBSTR bstr(CFGID_LANGUAGECODE);
		M_Config()->ItemValuesSet(1, &(bstr.m_str), cLanguage);
		return 0;
	}

	LRESULT OnStringListItemChanged(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		int nCount = m_wndList.GetItemCount();
		int nSel = m_wndList.GetSelectedIndex();
		if (nSel == -1)
		{
			m_nSelectedString = -1;
			SetDlgItemText(IDC_ORIGINALSTRING, _T(""));
			SetDlgItemText(IDC_TRANSLATEDSTRING, _T(""));
		}
		else
		{
			int nNewSel = m_cIndex[m_wndList.GetItemData(nSel)];
			if (m_nSelectedString != nNewSel)
			{
				m_nSelectedString = -1;
				SItem const& sItem = m_cItems[nNewSel];
				SetDlgItemText(IDC_ORIGINALSTRING, COLE2CT(sItem.bstrOrig));
				SetDlgItemText(IDC_TRANSLATEDSTRING, COLE2CT(sItem.bstrTran));
				m_nSelectedString = nNewSel;
			}
		}

		return 0;
	}

	LRESULT OnTranslationChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_nSelectedString != -1)
		{
			SItem& sItem = m_cItems[m_nSelectedString];
			CComBSTR bstrOld;
			bstrOld.Attach(sItem.bstrTran.Detach());
			GetDlgItemText(IDC_TRANSLATEDSTRING, sItem.bstrTran.m_str);
			if (bstrOld != sItem.bstrTran)
			{
				m_pTranslator->StringSet(m_wListLang, sItem.bstrOrig, sItem.bstrTran, TRUE);
				if (bstrOld.Length() == 0)
				{
					++m_nCustom;
					--m_nMissing;
				}
				else
				{
					m_pTranslator->LangInfo(m_wListLang, &m_bBuiltIn, &m_nTimestamp, &m_nStandard, &m_nCustom, &m_nMissing);
				}
				UpdateLanguageInfo();
			}
		}
		return 0;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		try
		{
			if (a_wParam == VK_RETURN && (GetKeyState(VK_CONTROL)&0x8000) == 0)
			{
				int nSel = m_wndList.GetSelectedIndex();
				if (-1 != nSel)
				{
					int nCount = m_wndList.GetItemCount();
					if (nCount > 1)
					{
						m_wndList.SelectItem((nSel+1)%nCount);
					}
				}
			}
		}
		catch (...)
		{
		}
		return 0;
	}
	LRESULT OnChar(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		try
		{
			if (a_wParam == 'L'-'A'+1 /*&& (GetKeyState(VK_CONTROL)&0x8000) == 0x8000 */&& m_nSelectedString != -1)
			{
				m_wndTranslation.SetWindowText(COLE2CT(m_cItems[m_nSelectedString].bstrOrig));
				m_wndTranslation.SetSelAll();
				BOOL b;
				OnTranslationChange(0, 0, m_wndTranslation, b);
				a_bHandled = TRUE;
			}
		}
		catch (...)
		{
		}
		return 0;
	}

	LRESULT OnDownloadClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		OLECHAR const* psz;
		HRESULT hRes = m_pTranslator->Synchronize(m_wListLang, IsDlgButtonChecked(IDC_TRANSLATORMODE) == BST_CHECKED);
		UINT nMBIcon = MB_ICONINFORMATION;
		switch (hRes)
		{
		case S_OK: psz = L"[0409]Operation succeeded.[0405]Operace proběhla úspěšně."; break;
		case S_FALSE: psz = L"[0409]Your translation tables are up-to-date.[0405]Vaše překladová tabulky jsou aktuální."; break;
		case E_RW_ITEMNOTFOUND: psz = L"[0409]Translation tables for this language are not available.[0405]Překladové tabulky pro tento jazyk nejsou k dispozici."; break;
		default: nMBIcon = MB_ICONERROR; psz = L"[0409]Operation failed. Please try again later.[0405]Operace selhala. Zkuste to prosím později."; break;
		}
		m_pTranslator->LangInfo(m_wListLang, &m_bBuiltIn, &m_nTimestamp, &m_nStandard, &m_nCustom, &m_nMissing);
		if (SUCCEEDED(hRes) && m_nCustom != 0)
		{
			nMBIcon = MB_ICONWARNING;
			psz = L"[0409]Some expressions were not uploaded. Please log-in on the Online page or request language moderator priviledges to be able to modify already translated expressions.[0405]Některé výrazy nebylo možné odeslat. Prosím přihlaště se na Online stránce nebo požádejte o práva jazykového moderátora pro možnost měnit už přeložené výrazy.";
		}
		CComBSTR bstrMsg;
		CMultiLanguageString::GetLocalized(psz, m_tLocaleID, &bstrMsg, CComQIPtr<ITranslator>(m_pTranslator));
		CComBSTR bstrCap;
		CMultiLanguageString::GetLocalized(L"[0409]Language synchronization[0405]Synchronizace jazyka", m_tLocaleID, &bstrCap, CComQIPtr<ITranslator>(m_pTranslator));
		MessageBox(COLE2CT(bstrMsg.m_str), COLE2CT(bstrCap.m_str), MB_OK|nMBIcon);
		UpdateLanguageInfo();
		InitStringCache(m_wListLang);
		InitStringList(true);
		return 0;
	}

	void UpdateLanguageInfo()
	{
		if (m_bBuiltIn)
		{
			CComBSTR bstrLoc;
			CMultiLanguageString::GetLocalized(L"[0409]Selected language is built into the application and supported by all default components.[0405]Zvolený jazyk je zabudován do aplikace a podporován všemi výchozími komponentami.", m_tLocaleID, &bstrLoc, CComQIPtr<ITranslator>(m_pTranslator));
			SetDlgItemText(IDC_LANGUAGEINFO, COLE2CT(bstrLoc.m_str));
			GetDlgItem(IDC_DOWNLOADLANGUAGE).EnableWindow(FALSE);
		}
		else
		{
			LPCOLESTR pszBtn = NULL;
			if (m_nStandard+m_nCustom)
			{
				pszBtn = m_nCustom ? L"[0409]Upload[0405]Odeslat" : L"[0409]Update[0405]Obnovit";
				CComBSTR bstrLoc;
				CMultiLanguageString::GetLocalized(m_nCustom ?
					L"[0409]Translation table contains %i items. There are also %i items translated by you. Press Upload to make them available for others.[0405]Překladová tabulka obsahuje %i záznamů. Existuje %i Vámi přeložených záznamů. Stiskněte Odeslat pro jejich zpřístupnění ostatním." :
					L"[0409]Translation table contains %i items. Press Update to check if more are available.[0405]Překladová tabulka obsahuje %i záznamů. Stiskněte Obnovit pro aktualizaci.",
					m_tLocaleID, &bstrLoc, CComQIPtr<ITranslator>(m_pTranslator));
				wchar_t szFin[256] = L"";
				_snwprintf(szFin, itemsof(szFin), bstrLoc.m_str, m_nStandard, m_nCustom);
				SetDlgItemText(IDC_LANGUAGEINFO, COLE2CT(szFin));
			}
			else
			{
				pszBtn = L"[0409]Download[0405]Stáhnout";
				CComBSTR bstrLoc;
				CMultiLanguageString::GetLocalized(L"[0409]Translation table is not available; try downloading it or help by translating yourself.[0405]Překladová tabulka není k dispozici. Zkuste ji stáhnout nebo se zapojte do překládání.", m_tLocaleID, &bstrLoc, CComQIPtr<ITranslator>(m_pTranslator));
				SetDlgItemText(IDC_LANGUAGEINFO, COLE2CT(bstrLoc.m_str));
			}
			CWindow wndButton = GetDlgItem(IDC_DOWNLOADLANGUAGE);
			wndButton.EnableWindow(TRUE);
			CComBSTR bstrBtn;
			CMultiLanguageString::GetLocalized(pszBtn, m_tLocaleID, &bstrBtn, CComQIPtr<ITranslator>(m_pTranslator));
			wndButton.SetWindowText(COLE2CT(bstrBtn));
		}
	}

	void InitStringCache(WORD a_wLangID)
	{
		m_cItems.clear();
		CComPtr<IEnumStrings> pStrs;
		m_pTranslator->StringsEnum(a_wLangID, &pStrs);
		ULONG nStrs = 0;
		if (pStrs) pStrs->Size(&nStrs);
		m_cItems.resize(nStrs);
		for (ULONG i = 0; i < nStrs; ++i)
		{
			SItem& sItem = m_cItems[i];
			pStrs->Get(i, &sItem.bstrOrig);
			m_pTranslator->StringInfo(a_wLangID, sItem.bstrOrig, &sItem.bstrTran, &sItem.bCustom, &sItem.nPriority);
		}
		// sort by priority
		m_cIndex.resize(nStrs);
		for (ULONG i = 0; i < nStrs; ++i)
			m_cIndex[i] = i;
		std::stable_sort(m_cIndex.begin(), m_cIndex.end(), CPrioSort(m_cItems));
		m_nSelectedString = -1;
	}
	void InitStringList(bool a_bForceUpdate = false)
	{
		CConfigValue cHideTranslated;
		CConfigValue cFilter;
		M_Config()->ItemValueGet(CComBSTR(CFGID_HIDETRANSLATED), &cHideTranslated);
		M_Config()->ItemValueGet(CComBSTR(CFGID_TRANSLATIONFILTER), &cFilter);
		if (!a_bForceUpdate && m_cHideTranslated == cHideTranslated && m_cFilter == cFilter)
			return; // up-to-date
		m_cHideTranslated = cHideTranslated;
		m_cFilter = cFilter;
		bool bHideTranslated = m_cHideTranslated;
		BSTR bstrFilter = m_cFilter;
		if (bstrFilter && *bstrFilter == L'\0') bstrFilter = NULL;
		int j = 0;
		LONG nSelectedString = m_nSelectedString;
		m_wndList.DeleteAllItems();
		bool bSelect = false;
		bool bSelected = false;
		for (CIndex::const_iterator i = m_cIndex.begin(); i != m_cIndex.end(); ++i)
		{
			if (*i == nSelectedString)
				bSelect = true;
			SItem const& sItem = m_cItems[*i];
			if (bHideTranslated && sItem.bstrTran && *sItem.bstrTran)
				continue;
			if (bstrFilter && wcsstr(sItem.bstrOrig.m_str, bstrFilter) == NULL &&
				(sItem.bstrTran == NULL || wcsstr(sItem.bstrTran.m_str, bstrFilter) == NULL))
				continue;
			int nItem = m_wndList.AddItem(j++, 0, COLE2CT(sItem.bstrOrig.m_str));
			m_wndList.SetItemData(nItem, i-m_cIndex.begin());
			if (bSelect && !bSelected)
			{
				m_wndList.SelectItem(nItem);
				bSelected = true;
			}
		}
		if (!bSelected)
		{
			if (j)
			{
				m_wndList.SelectItem(bSelect ? j-1 : 0);
			}
			else
			{
				m_nSelectedString = -1;
				SetDlgItemText(IDC_ORIGINALSTRING, _T(""));
				SetDlgItemText(IDC_TRANSLATEDSTRING, _T(""));
			}
		}
	}

private:
	struct SItem
	{
		CComBSTR bstrOrig;
		CComBSTR bstrTran;
		ULONG nPriority;
		BOOL bCustom;
	};
	typedef std::vector<SItem> CItems;
	typedef std::vector<ULONG> CIndex;
	struct CPrioSort
	{
		CPrioSort(CItems const& a_cItems) : m_cItems(a_cItems) {}

		bool operator()(ULONG a_1, ULONG a_2) const
		{
			return m_cItems[a_1].nPriority > m_cItems[a_2].nPriority;
		}
		CItems const& m_cItems;
	};

private:
	CComboBoxEx m_wndLanguages;
	CImageList m_cLanguageIcons;
	CHyperLink m_wndHelpLink;
	CListViewCtrl m_wndList;
	CContainedWindowT<CEdit> m_wndTranslation;
	CComPtr<ITranslatorManager> m_pTranslator;
	CItems m_cItems;
	CIndex m_cIndex;
	CConfigValue m_cHideTranslated;
	CConfigValue m_cFilter;
	LONG m_nSelectedString;

	WORD m_wListLang;
	BOOL m_bBuiltIn;
	ULONG m_nTimestamp;
	ULONG m_nStandard;
	ULONG m_nCustom;
	ULONG m_nMissing;
};
