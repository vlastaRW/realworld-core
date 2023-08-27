
#pragma once

#include <ConfigCustomGUIIcons.h>
#include <ConfigCustomGUIML.h>

// copied to avoid dependency - TODO: separate scintilla control?
//#include <RWViewBasic.h>
MIDL_INTERFACE("EAC1611D-15C0-461B-B38E-E7E3FBE0E0CF")
IScintillaFactory : public IUnknown
{
public:
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RegisterClasses( void) = 0;
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UnregisterClasses( void) = 0;
};
class DECLSPEC_UUID("4B01B979-A6B1-4144-83B6-49571B46CEB6")
DesignerViewFactoryText;

#include <Platform.h>
#include <Scintilla.h>
#include <SciLexer.h>

#include <atlgdix.h>
#include <CustomTabCtrl.h>
#include <DotNetTabCtrl.h>


class ATL_NO_VTABLE CConfigGUIJScriptDlg :
	public CCustomConfigWndMultiLang<CConfigGUIJScriptDlg, CCustomConfigWndWithIcons<CConfigGUIJScriptDlg, CCustomConfigResourcelessWndImpl<CConfigGUIJScriptDlg>>>,
	public CDialogResize<CConfigGUIJScriptDlg>
{
public:
	CConfigGUIJScriptDlg() :
		CCustomConfigWndMultiLang<CConfigGUIJScriptDlg, CCustomConfigWndWithIcons<CConfigGUIJScriptDlg, CCustomConfigResourcelessWndImpl<CConfigGUIJScriptDlg>>>(CFGID_CFG_CAPTION, CFGID_CFG_HELPTOPIC)
	{
	}
	~CConfigGUIJScriptDlg()
	{
		if (m_pScintilla)
			m_pScintilla->UnregisterClasses();
	}

	enum
	{
		IDC_CGS_SCRIPT = 100,
		IDC_CGS_CAPTIONLABEL,
		IDC_CGS_CAPTION,
		IDC_CGS_ICONLABEL,
		IDC_CGS_ICON,
		IDC_CGS_PREVIEWLABEL,
		IDC_CGS_PREVIEW,
		IDC_CGS_VIEWCONFIG,
		IDC_CGS_HELPTOPICLABEL,
		IDC_CGS_HELPTOPIC,
		IDC_CGS_CFGSCRIPT,
		IDC_CGS_TAB = 400,
	};

	BEGIN_DIALOG_EX(0, 0, 194, 101, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_EDITTEXT(IDC_CGS_SCRIPT, 7, 18, 180, 75, WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_WANTRETURN, 0)
		CONTROL_LTEXT(_T("[0409]Dialog caption:[0405]Záhlaví dialogu:"), IDC_CGS_CAPTIONLABEL, 7, 20, 57, 8, 0, 0)
		CONTROL_EDITTEXT(IDC_CGS_CAPTION, 65, 18, 59, 12, WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Icon:[0405]Ikona:"), IDC_CGS_ICONLABEL, 131, 19, 24, 8, 0, 0)
		CONTROL_CONTROL(_T(""), IDC_CGS_ICON, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP, 157, 17, 29, 197, 0)
		CONTROL_LTEXT(_T("[0409]Preview:[0405]Náhled:"), IDC_CGS_PREVIEWLABEL, 7, 36, 50, 8, 0, 0)
		CONTROL_COMBOBOX(IDC_CGS_PREVIEW, 65, 34, 122, 163, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP, 0)
		CONTROL_CONTROL(_T(""), IDC_CGS_VIEWCONFIG, WC_STATIC, SS_GRAYRECT, 7, 50, 180, 28, 0)
		CONTROL_LTEXT(_T("[0409]Help text:[0405]Text nápovědy:"), IDC_CGS_HELPTOPICLABEL, 7, 84, 55, 8, 0, 0)
		CONTROL_EDITTEXT(IDC_CGS_HELPTOPIC, 65, 82, 121, 12, WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_EDITTEXT(IDC_CGS_CFGSCRIPT, 7, 18, 180, 75, WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_WANTRETURN, 0)
	END_CONTROLS_MAP()

		typedef CCustomConfigWndWithIcons<CConfigGUIJScriptDlg, CCustomConfigResourcelessWndImpl<CConfigGUIJScriptDlg>> baseClass;
	BEGIN_MSG_MAP(CConfigGUIJScriptDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIJScriptDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//COMMAND_HANDLER(IDC_CGS_TEST, BN_CLICKED, OnClickedTest)
		NOTIFY_HANDLER(IDC_CGS_TAB, CTCN_SELCHANGE, OnTcnSelchange)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIJScriptDlg)
		DLGRESIZE_CONTROL(IDC_CGS_TAB, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGS_SCRIPT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		//DLGRESIZE_CONTROL(IDC_CGS_TEST, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGS_CAPTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGS_ICONLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGS_ICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGS_PREVIEW, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGS_VIEWCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGS_HELPTOPICLABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGS_HELPTOPIC, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGS_CFGSCRIPT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIJScriptDlg)
		CONFIGITEM_EDITBOX(IDC_CGS_SCRIPT, CFGID_JS_SCRIPT)
		CONFIGITEM_EDITBOX(IDC_CGS_CFGSCRIPT, CFGID_JS_CFGSCRIPT)
		CONFIGITEM_EDITBOX(IDC_CGS_CAPTION, CFGID_CFG_CAPTION)
		CONFIGITEM_ICONCOMBO(IDC_CGS_ICON, CFGID_CFG_ICONID)
		CONFIGITEM_COMBOBOX(IDC_CGS_PREVIEW, CFGID_CFG_PREVIEW)
		CONFIGITEM_SUBCONFIG(IDC_CGS_VIEWCONFIG, CFGID_CFG_PREVIEW)
		CONFIGITEM_EDITBOX(IDC_CGS_HELPTOPIC, CFGID_CFG_HELPTOPIC)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		RECT rcWin;
		GetClientRect(&rcWin);
		RECT rcTab = {0, 0, 12, 12};
		MapDialogRect(&rcTab);
		int nNewTabAreaHeight = rcTab.bottom;
//IDC_CGS_SCRIPT
		CComBSTR bstrConfig;
		CMultiLanguageString::GetLocalized(L"[0409]Configuration[0405]Konfigurace", m_tLocaleID, &bstrConfig);
		CComBSTR bstrExec;
		CMultiLanguageString::GetLocalized(L"[0409]Execution[0405]Spuštění", m_tLocaleID, &bstrExec);
		CComBSTR bstrPreview;
		CMultiLanguageString::GetLocalized(L"[0409]Preview[0405]Náhled", m_tLocaleID, &bstrPreview);
		RECT rc = {0, 0, rcWin.right, nNewTabAreaHeight};
		m_wndTab.Create(m_hWnd, rc, _T("Tab"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE, 0, IDC_CGS_TAB);
		m_wndTab.InsertItem(0, bstrConfig);
		m_wndTab.InsertItem(1, bstrExec, -1, 0, true);
		m_wndTab.InsertItem(2, bstrPreview);

		RWCoCreateInstance(m_pScintilla, __uuidof(DesignerViewFactoryText));
		RWCoCreateInstance(m_pScripting, __uuidof(ScriptingInterfaceManager));
		if (m_pScintilla && m_pScripting && SUCCEEDED(m_pScintilla->RegisterClasses()))
		{
			ReplaceWithScintilla(GetDlgItem(IDC_CGS_SCRIPT));
			ReplaceWithScintilla(GetDlgItem(IDC_CGS_CFGSCRIPT));
		}
		else
		{
			m_pScintilla = NULL;
			m_pScripting = NULL;
		}

		BOOL b;
		CCustomConfigWndMultiLang<CConfigGUIJScriptDlg, CCustomConfigWndWithIcons<CConfigGUIJScriptDlg, CCustomConfigResourcelessWndImpl<CConfigGUIJScriptDlg>>>::OnInitDialog(WM_INITDIALOG, 0, 0, b);
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnClickedTest(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		try
		{
			CConfigValue cScriptText;
			M_Config()->ItemValueGet(CComBSTR(CFGID_JS_SCRIPT), &cScriptText);

			// {f414c260-6ac0-11cf-b6d1-00aa00bbbb58}
			static CLSID const CLSID_JScriptEngine = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};
			CComPtr<IActiveScript> pScripEngine;
			pScripEngine.CoCreateInstance(CLSID_JScriptEngine);
			CComQIPtr<IActiveScriptParse> pScriptParse(pScripEngine);
			if (pScripEngine == NULL || pScriptParse == NULL)
				return 0; // TODO: report error

			HRESULT hRes = S_OK;
			CComObject<CRWActiveScriptSite>* pSite = NULL;
			CComObject<CRWActiveScriptSite>::CreateInstance(&pSite);
			CComPtr<IActiveScriptSite> pSitePtr = pSite;
			CComPtr<IConfig> pCfg;
			M_Config()->SubConfigGet(CComBSTR(CFGID_JS_SCRIPT), &pCfg);
			pSite->Init(pScripEngine, NULL, NULL, pCfg, NULL, m_hWnd, m_tLocaleID);

			hRes = pScriptParse->InitNew();
			if (SUCCEEDED(hRes))
				hRes = pScriptParse->ParseScriptText(cScriptText, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE, NULL, NULL/*&tExceptInfo*/);

			pScripEngine->Close();
			if (FAILED(hRes))
			{
				CComBSTR bstrErr;
				bstrErr = L"Source: ";
				bstrErr += pSite->M_ErrorSource();
				bstrErr += L"\nDescription: ";
				bstrErr += pSite->M_ErrorDescription();
				bstrErr += L"\nLine: ";
				bstrErr += pSite->M_ErrorLineText();
				MessageBox(COLE2T(bstrErr), _T("Scripted Operation"), MB_OK|MB_ICONERROR);
				CEdit wnd = GetDlgItem(IDC_CGS_SCRIPT);
				wnd.SetFocus();
				int nPosition = 0;
				ULONG nLine = 0;
				LPCOLESTR p = cScriptText.operator BSTR();
				while (*p)
				{
					if (nLine < pSite->M_ErrorLine())
					{
						if (*p == L'\r' || *p == L'\n')
						{
							++nLine;
							if (p[1] == *p^(L'\r'^L'\n'))
								++p;
						}
					}
					else if (nLine == pSite->M_ErrorLine() && nPosition < pSite->M_ErrorColumn() && *p != L'\r' && *p != L'\n')
					{
						++nPosition;
					}
					else
					{
						break;
					}
					++p;
				}
				nPosition = p-cScriptText.operator BSTR();
				wnd.SetSel(nPosition, nPosition);
			}
			else
			{
				MessageBox(_T("No syntactic errors found."), _T("Scripted Operation"), MB_OK|MB_ICONINFORMATION);
				GetDlgItem(IDC_CGS_SCRIPT).SetFocus();
			}

			return 0;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
		return 0;
	}
	LRESULT OnTcnSelchange(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
	{
		int nCount = m_wndTab.GetItemCount();
		int nSel = m_wndTab.GetCurSel();

		if (nSel < 0 || nSel >= nCount)
			return 0;

		bool bIsVisible = (GetWindowLong(GWL_STYLE) & WS_VISIBLE) != 0;
		if (bIsVisible)
			SetRedraw(FALSE);

		static UINT aIDs[] =
		{
			IDC_CGS_CAPTIONLABEL, 2,
			IDC_CGS_CAPTION, 2,
			IDC_CGS_ICONLABEL, 2,
			IDC_CGS_ICON, 2,
			IDC_CGS_PREVIEWLABEL, 2,
			IDC_CGS_PREVIEW, 2,
			IDC_CGS_VIEWCONFIG, 2,
			IDC_CGS_HELPTOPICLABEL, 2,
			IDC_CGS_HELPTOPIC, 2,
			IDC_CGS_CFGSCRIPT, 0,
			IDC_CGS_SCRIPT, 1,
		};
		HWND hToFocus = NULL;
		for (UINT i = 0; i < itemsof(aIDs); i+=2)
		{
			CWindow wnd = GetDlgItem(aIDs[i]);
			wnd.ShowWindow(nSel == aIDs[i+1] ? SW_SHOW : SW_HIDE);
			if (hToFocus == NULL && nSel == aIDs[i+1])
				hToFocus = wnd;
		}

		if (bIsVisible)
		{
			GotoDlgCtrl(hToFocus);
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		return 0;
	}

private:
	void ReplaceWithScintilla(CWindow& a_wnd)
	{
		RECT rc;
		a_wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		HWND hScintilla = ::CreateWindow(_T("Scintilla"), NULL, WS_CHILD|WS_TABSTOP|WS_BORDER|(WS_VISIBLE&a_wnd.GetWindowLong(GWL_STYLE)), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, m_hWnd, (HMENU)a_wnd.GetWindowLong(GWLP_ID), _pModule->get_m_hInst(), NULL);
		if (hScintilla)
		{
			CComPtr<IEnumStringsInit> pPrimKW;
			RWCoCreateInstance(pPrimKW, __uuidof(EnumStrings));
			m_pScripting->GetKeywords(pPrimKW, pPrimKW);
			CComBSTR bstrKW(L"Math String Boolean Array Number Date RegExp Transformation");
			ULONG nKW = 0;
			pPrimKW->Size(&nKW);
			for (ULONG i = 0; i < nKW; ++i)
			{
				CComBSTR bstr;
				pPrimKW->Get(i, &bstr);
				if (bstr != NULL && bstr[0])
				{
					bstrKW += L" ";
					bstrKW += bstr;
				}
			}
			// TODO: convert to UTF8
			CW2A strKW(bstrKW);
			LPCSTR pszKW = strKW;

			::SendMessage(hScintilla, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
			::SendMessage(hScintilla, SCI_SETTABWIDTH, 4, 0);
			::SendMessage(hScintilla, SCI_SETLEXER, SCLEX_CPP, 0);
			::SendMessage(hScintilla, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Courier New");
			::SendMessage(hScintilla, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
			::SendMessage(hScintilla, SCI_STYLESETBACK, 0, RGB(255, 255, 255));
			::SendMessage(hScintilla, SCI_STYLESETFORE, 0, RGB(20, 20, 30));
			::SendMessage(hScintilla, SCI_STYLECLEARALL, 0, 0);
			::SendMessage(hScintilla, SCI_SETKEYWORDS, 0, (LPARAM)"break for new true with continue function null typeof else if return var false in this while Infinity NaN Undefined");
			::SendMessage(hScintilla, SCI_SETKEYWORDS, 1, (LPARAM)"abstract default implements static void boolean do import super byte double instanceof switch case extends int synchronized catch final interface throw char finally long throws class float native transient const goto package try");
			::SendMessage(hScintilla, SCI_SETKEYWORDS, 3, (LPARAM)pszKW);
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_COMMENT, RGB(50, 160, 50));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_COMMENTLINE, RGB(50, 160, 50));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_COMMENTDOC, RGB(50, 160, 50));
			::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_C_NUMBER, true);
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_WORD, RGB(50, 50, 200));
			//::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_C_WORD, true);
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_STRING, RGB(120, 60, 60));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_CHARACTER, RGB(120, 60, 60));
			//::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_UUID, RGB(0,0,255));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_PREPROCESSOR, RGB(50, 50, 200));
			::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_C_PREPROCESSOR, true);
			//::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_OPERATOR, RGB(255,0,64));
			//::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_IDENTIFIER, RGB(20, 20, 30));
			::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_C_IDENTIFIER, true);
			//::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_STRINGEOL, RGB(200,20,0));
			//::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_VERBATIM, RGB(200,20,0));
			//::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_REGEX, RGB(200,20,0));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_COMMENTLINEDOC, RGB(50, 160, 50));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_WORD2, RGB(50, 50, 200));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_COMMENTDOCKEYWORD, RGB(50, 160, 50));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_COMMENTDOCKEYWORDERROR, RGB(50, 160, 50));
			::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_C_GLOBALCLASS, RGB(50, 50, 200));
			::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_C_GLOBALCLASS, true);
			//::SendMessage(hScintilla, SCI_COLOURISE, 0, -1);
			::SetWindowPos(hScintilla, a_wnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
			a_wnd.DestroyWindow();
		}
		else
		{
			a_wnd.SetFont((HFONT)::GetStockObject(OEM_FIXED_FONT));
		}
	}

	template <class TItem = CCustomTabItem>
	class CContrastTabCtrl : public CDotNetTabCtrlImpl<CContrastTabCtrl<TItem>, TItem>
	{
	public:
		DECLARE_WND_CLASS_EX(_T("WTL_ContrastDotNetTabCtrl"), CS_DBLCLKS, COLOR_WINDOW)

		void InitializeDrawStruct(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
		{
			CDotNetTabCtrlImpl<CContrastTabCtrl, TItem>::InitializeDrawStruct(lpNMCustomDraw);
			lpNMCustomDraw->clrBtnHighlight = ::GetSysColor(COLOR_3DSHADOW);
		}
	};

private:
	CComPtr<IScintillaFactory> m_pScintilla;
	CComPtr<IScriptingInterfaceManager> m_pScripting;
	CContrastTabCtrl<> m_wndTab;
};

