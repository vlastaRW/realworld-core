// DesignerViewText.cpp : Implementation of CDesignerViewText

#include "stdafx.h"
#include "DesignerViewText.h"
#include <MultiLanguageString.h>


// CDesignerViewText

void CDesignerViewText::Init(ISharedStateManager* a_pStateMgr, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID, IScintillaFactory* a_pScintilla)
{
	a_pScintilla->RegisterClasses();
	m_pScintilla = a_pScintilla;

	m_nStyle = a_nStyle;
	// not multithread safe, but it does not matter
	static INITCOMMONCONTROLSEX tICCE = {sizeof(tICCE), ICC_STANDARD_CLASSES};
	static BOOL b = InitCommonControlsEx(&tICCE);

	CComPtr<IDocumentText> pTextDoc;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentText), reinterpret_cast<void**>(&pTextDoc));
	pTextDoc->ObserverIns(ObserverGet(), 0);
	m_pTextDoc = pTextDoc;

	m_tLocaleID = a_tLocaleID;

	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("Basic_Text"), WS_CHILDWINDOW|WS_CLIPSIBLINGS) == NULL)
	{
		// creation failed
		throw E_FAIL; // TODO: error code
	}

	ShowWindow(SW_SHOW);
}

void CDesignerViewText::OwnerNotify(TCookie a_tCookie, LONG a_nLineIndex)
{
	if (m_wndClient.m_hWnd == NULL || m_bModifying)
		return;
	CComBSTR bstr;
	m_pTextDoc->TextGet(&bstr);
	COLE2CT sz(bstr);
	m_bModifying = true;
	m_wndClient.SetWindowText(sz);
	m_bModifying = false;
}

LRESULT CDesignerViewText::OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	CREATESTRUCT const* pCreateStruct = reinterpret_cast<CREATESTRUCT const*>(a_lParam);

	RECT rc;
	GetClientRect(&rc);
	m_cFont.CreateFont(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, _T("Lucida Console"));
	m_wndClient = ::CreateWindow(_T("Scintilla"), NULL, WS_CHILD|WS_TABSTOP|WS_VISIBLE|((m_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_BORDER : 0), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, m_hWnd, (HMENU)IDC_TEXT_TEXT, _pModule->get_m_hInst(), NULL);
	m_wndClient.SendMessage(SCI_SETUNDOCOLLECTION, false);
	m_wndClient.SendMessage(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
	m_wndClient.SendMessage(SCI_SETTABWIDTH, 4, 0);
	m_wndClient.SendMessage(SCI_SETSTYLEBITS, 7/*m_wndClient.SendMessage(SCI_GETSTYLEBITSNEEDED))*/);
	m_wndClient.SendMessage(SCI_SETLEXER, SCLEX_HTML, 0);
	m_wndClient.SendMessage(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Courier New");
	HDC hDC = GetDC();
	m_wndClient.SendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, (8*GetDeviceCaps(hDC, LOGPIXELSX)+48)/96);
	ReleaseDC(hDC);
	m_wndClient.SendMessage(SCI_STYLESETBACK, SCE_H_DEFAULT, RGB(255, 255, 255));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_H_DEFAULT, RGB(20, 20, 30));
	m_wndClient.SendMessage(SCI_STYLECLEARALL, 0, 0);
	//m_wndClient.SendMessage(SCI_SETKEYWORDS, 0, (LPARAM)"break for new true with continue function null typeof else if return var false in this while Infinity NaN Undefined");
	//m_wndClient.SendMessage(SCI_SETKEYWORDS, 1, (LPARAM)"abstract default implements static void boolean do import super byte double instanceof switch case extends int synchronized catch final interface throw char finally long throws class float native transient const goto package try");
	//m_wndClient.SendMessage(SCI_SETKEYWORDS, 3, (LPARAM)pszKW);
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_H_TAG, RGB(50, 50, 200));
	//m_wndClient.SendMessage(SCI_STYLESETBOLD, SCE_H_TAG, true);
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_H_TAGUNKNOWN, RGB(50, 50, 200));
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_H_ATTRIBUTE, RGB(160, 50, 50));
	//m_wndClient.SendMessage(SCI_STYLESETBOLD, SCE_H_ATTRIBUTE, true);
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_H_ATTRIBUTEUNKNOWN, RGB(160, 50, 50));
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_H_DOUBLESTRING, RGB(120, 60, 60));
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_H_SINGLESTRING, RGB(120, 60, 60));
	//m_wndClient.SendMessage(SCI_STYLESETBOLD, SCE_H_NUMBER, true);
	struct SStyleInfo
	{
		int nStyle;
		COLORREF clrFG;
		COLORREF clrBG;
		bool bBold;
		bool bItalic;
		bool bEOLFilled;
	};
	static SStyleInfo const s_aStyles[] =
	{
		//{0, 0x000000, 0xffffff, false, false},
		{SCE_H_TAG, 0x800000, 0xffffff, true, false, false},
		{SCE_H_TAGUNKNOWN, 0x0000ff, 0xffffff, false, false, false},
		{SCE_H_ATTRIBUTE, 0x808000, 0xffffff, true, false, false},
		{SCE_H_ATTRIBUTEUNKNOWN, 0x0000ff, 0xffffff, false, false, false},
		{SCE_H_NUMBER, 0x000000, 0xffffff, true, false, false},
		{SCE_H_DOUBLESTRING, 0x2020a0, 0xffffff, false, false, false},
		{SCE_H_SINGLESTRING, 0x2020a0, 0xffffff, false, false, false},
		{SCE_H_OTHER, 0x800080, 0xffffff, false, false, false},
		{SCE_H_COMMENT, 0x008080, 0xffffff, false, false, false},
		{SCE_H_ENTITY, 0x800080, 0xffffff, false, false, false},
		{SCE_H_TAGEND, 0x800000, 0xffffff, false, false, false},
		{SCE_H_XMLSTART, 0xff0000, 0xffffff, false, false, false},
		{SCE_H_XMLEND, 0xff0000, 0xffffff, false, false, false},
		{SCE_H_SCRIPT, 0x800000, 0xffffff, false, false, false},
		{SCE_HPHP_COMPLEX_VARIABLE, 0x007f00, 0xf8f8ff, false, true, false},
		{SCE_HPHP_DEFAULT, 0x330000, 0xf8f8ff, false, false, true},
		{SCE_HPHP_HSTRING, 0x007f00, 0xf8f8ff, false, false, false},
		{SCE_HPHP_SIMPLESTRING, 0x009f00, 0xf8f8ff, false, false, false},
		{SCE_HPHP_WORD, 0x7f007f, 0xf8f8ff, false, true, false},
		{SCE_HPHP_NUMBER, 0x0099cc, 0xf8f8ff, false, false, false},
		{SCE_HPHP_VARIABLE, 0x7f0000, 0xf8f8ff, false, true, false},
		{SCE_HPHP_COMMENT, 0x999999, 0xf8f8ff, false, false, false},
		{SCE_HPHP_COMMENTLINE, 0x999999, 0xf8f8ff, false, false, false},
		{SCE_HPHP_HSTRING_VARIABLE, 0x007f00, 0xf8f8ff, false, true, false},
		{SCE_HPHP_OPERATOR, 0x000000, 0xf8f8ff, false, false, false},
	};
	for (SStyleInfo const* p = s_aStyles; p != s_aStyles+sizeof(s_aStyles)/sizeof(s_aStyles[0]); ++p)
	{
		if (p->clrFG != 0x000000) m_wndClient.SendMessage(SCI_STYLESETFORE, p->nStyle, p->clrFG);
		if (p->clrBG != 0xffffff) m_wndClient.SendMessage(SCI_STYLESETBACK, p->nStyle, p->clrBG);
		if (p->bBold) m_wndClient.SendMessage(SCI_STYLESETBOLD, p->nStyle, true);
		if (p->bItalic) m_wndClient.SendMessage(SCI_STYLESETITALIC, p->nStyle, true);
		if (p->bEOLFilled) m_wndClient.SendMessage(SCI_STYLESETEOLFILLED, p->nStyle, true);
	}
//#define  0
//#define SCE_H_ASP 15
//#define SCE_H_ASPAT 16
//#define SCE_H_CDATA 17
//#define SCE_H_QUESTION 18
//#define SCE_H_VALUE 19
//#define SCE_H_XCCOMMENT 20
//#define SCE_H_SGML_DEFAULT 21
//#define SCE_H_SGML_COMMAND 22
//#define SCE_H_SGML_1ST_PARAM 23
//#define SCE_H_SGML_DOUBLESTRING 24
//#define SCE_H_SGML_SIMPLESTRING 25
//#define SCE_H_SGML_ERROR 26
//#define SCE_H_SGML_SPECIAL 27
//#define SCE_H_SGML_ENTITY 28
//#define SCE_H_SGML_COMMENT 29
//#define SCE_H_SGML_1ST_PARAM_COMMENT 30
//#define SCE_H_SGML_BLOCK_DEFAULT 31
	/*
	//m_wndClient.SendMessage(SCI_STYLESETBOLD, SCE_C_WORD, true);
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_STRING, RGB(120, 60, 60));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_CHARACTER, RGB(120, 60, 60));
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_UUID, RGB(0,0,255));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, RGB(50, 50, 200));
	m_wndClient.SendMessage(SCI_STYLESETBOLD, SCE_C_PREPROCESSOR, true);
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_OPERATOR, RGB(255,0,64));
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_IDENTIFIER, RGB(20, 20, 30));
	m_wndClient.SendMessage(SCI_STYLESETBOLD, SCE_C_IDENTIFIER, true);
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_STRINGEOL, RGB(200,20,0));
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_VERBATIM, RGB(200,20,0));
	//m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_REGEX, RGB(200,20,0));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_COMMENTLINEDOC, RGB(50, 160, 50));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_WORD2, RGB(50, 50, 200));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_COMMENTDOCKEYWORD, RGB(50, 160, 50));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_COMMENTDOCKEYWORDERROR, RGB(50, 160, 50));
	m_wndClient.SendMessage(SCI_STYLESETFORE, SCE_C_GLOBALCLASS, RGB(50, 50, 200));
	m_wndClient.SendMessage(SCI_STYLESETBOLD, SCE_C_GLOBALCLASS, true);
	*/
	m_wndClient.SendMessage(SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT);

	OwnerNotify(0, -1);

	return 0;
}

LRESULT CDesignerViewText::OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_wndClient.SetFocus();
	return 0;
}

LRESULT CDesignerViewText::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	RECT rc = {0, 0, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	m_wndClient.MoveWindow(&rc);
	return 0;
}

LRESULT CDesignerViewText::OnTextModified(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	if (m_bModifying)
		return 0;

	SCNotification* a_pSCNot = reinterpret_cast<SCNotification*>(a_pNMHdr);
	CComBSTR bstr;
	m_wndClient.GetWindowText(&bstr);
	m_bModifying = true;
	m_pTextDoc->TextSet(bstr);
	m_bModifying = false;
	return 0;
}

STDMETHODIMP CDesignerViewText::ObjectName(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Text[0405]Text");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewText::Check(EDesignerViewClipboardAction a_eAction)
{
	try
	{
		switch (a_eAction)
		{
		case EDVCACut:				return m_wndClient.SendMessage(SCI_GETSELECTIONSTART) != m_wndClient.SendMessage(SCI_GETSELECTIONEND) ? S_OK : S_FALSE;
		case EDVCACopy:				return S_OK;
		case EDVCAPaste:			return IsClipboardFormatAvailable(CF_TEXT) ? S_OK : S_FALSE;
		case EDVCASelectAll:		return S_OK;
		case EDVCAInvertSelection:	return S_FALSE;
		}
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewText::Exec(EDesignerViewClipboardAction a_eAction)
{
	try
	{
		switch (a_eAction)
		{
		case EDVCACut:				m_wndClient.SendMessage(SCI_CUT); return S_OK;
		case EDVCACopy:				m_wndClient.SendMessage(SCI_COPYALLOWLINE); return S_OK;
		case EDVCAPaste:			m_wndClient.SendMessage(SCI_PASTE); return S_OK;
		case EDVCASelectAll:		m_wndClient.SendMessage(SCI_SELECTALL); return S_OK;
		}
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

