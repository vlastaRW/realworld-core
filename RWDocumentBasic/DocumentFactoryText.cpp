// DocumentFactoryText.cpp : Implementation of CDocumentFactoryText

#include "stdafx.h"
#include "DocumentFactoryText.h"

#include "uchd/nscore.h"
#include "uchd/nsUniversalDetector.h"

#define CP_UCS16BE 65008
#define CP_UCS16LE 65009

class CCharsetDetector : nsUniversalDetector
{
public:
	CCharsetDetector() : nsUniversalDetector(NS_FILTER_ALL), nEncoding(0)
	{
		m_cEncodings["UTF-8"] = CP_UTF8;
		m_cEncodings["Big5"] = 950;
		m_cEncodings["EUC-JP"] = 51932;//20932;
		m_cEncodings["EUC-KR"] = 51949;
		//m_cEncodings["x-euc-tw"]
		m_cEncodings["gb18030"] = 54936;
		m_cEncodings["windows-1250"] = 1250;
		m_cEncodings["windows-1251"] = 1251;
		m_cEncodings["windows-1252"] = 1252;
		m_cEncodings["windows-1253"] = 1253;
		m_cEncodings["windows-1255"] = 1255;
		m_cEncodings["Shift_JIS"] = 932;
		m_cEncodings["ISO-8859-8"] = 28598;
		m_cEncodings["ISO-8859-7"] = 28597;
		m_cEncodings["ISO-8859-5"] = 28595;
		m_cEncodings["ISO-8859-2"] = 28592;
		m_cEncodings["UTF-16BE"] = CP_UCS16BE;
		m_cEncodings["UTF-16LE"] = CP_UCS16LE;
		m_cEncodings["KOI8-R"] = 20866;
		m_cEncodings["x-mac-cyrillic"] = 10007;
		m_cEncodings["IBM866"] = /*20*/866;
		m_cEncodings["IBM855"] = 855;
		//m_cEncodings["TIS-620"] = ;
	}

	int Detect(BYTE const* a_pData, ULONG a_nLen)
	{
		nEncoding = 0;
		if (NS_OK == HandleData(reinterpret_cast<char const*>(a_pData), a_nLen))
		{
			DataEnd();
		}
		return nEncoding;
	}

	void Report(char const* a_pszEncoding)
	{
		std::map<char const*, UINT, lessStrcmp>::const_iterator i = m_cEncodings.find(a_pszEncoding);
		if (i != m_cEncodings.end())
			nEncoding = i->second;
	}

private:
	struct lessStrcmp
	{
		bool operator()(char const* a_p1, char const* a_p2) const
		{
			return (a_p1 == NULL && a_p2) ||
				(a_p1 && a_p2 && strcmp(a_p1, a_p2) < 0);
		}
	};

private:
	int nEncoding;
	std::map<char const*, UINT, lessStrcmp> m_cEncodings;
};

// CDocumentFactoryText

static OLECHAR const CFGID_MARKER[] = L"Marker";
static OLECHAR const CFGID_ENCODING[] = L"Encoding";
static OLECHAR const CFGVAL_UTF8[] = L"UTF8";
static OLECHAR const CFGVAL_UCS16[] = L"UCS16";

HRESULT CDocumentFactoryText::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), IDocumentFactoryText* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	try
	{
		CComObject<CDocumentText>* pDoc = NULL;
		CComObject<CDocumentText>::CreateInstance(&pDoc);
		CComPtr<IDocumentData> pTmp = pDoc;

		if (a_nLen)
		{
			CCharsetDetector cDetector;
			UINT nEncoding = cDetector.Detect(a_pData, a_nLen);
			BYTE const* pB = a_pData;
			BYTE const* pE = a_pData+a_nLen;
			wchar_t const* pWB = NULL;
			wchar_t const* pWE = NULL;
			CAutoVectorPtr<wchar_t> pBuffer;
			bool bMarker = true;
			bool bUTF8 = true;
			if (nEncoding == CP_UCS16LE)
			{
				bMarker = a_nLen >= 2 && pB[0] == 0xff && pB[1] == 0xfe;
				bUTF8 = false;
				pWB = reinterpret_cast<wchar_t const*>(bMarker ? pB+2 : pB);
				pWE = reinterpret_cast<wchar_t const*>(pE);
			}
			else if (nEncoding == CP_UCS16BE)
			{
				bMarker = a_nLen >= 2 && pB[0] == 0xfe && pB[1] == 0xff;
				bUTF8 = false;
				if (a_nLen >= 4)
				{
					pBuffer.Allocate((pE-(bMarker ? pB+2 : pB))>>1);
					wchar_t* pp = pBuffer;
					for (BYTE const* p = bMarker ? pB+2 : pB; p < pE; p += 2, ++pp)
						*pp = (wchar_t(p[0])<<8)|p[1];
					pWB = pBuffer;
					pWE = pp;
				}
			}
			else if (nEncoding == CP_UTF8)
			{
				bMarker = a_nLen >= 3 && pB[0] == 0xef && pB[1] == 0xbb && pB[2] == 0xbf;
				int nSize = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(bMarker ? pB+3 : pB), pE-(bMarker ? pB+3 : pB), NULL, 0);
				if (nSize)
				{
					pBuffer.Allocate(nSize);
					MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(bMarker ? pB+3 : pB), pE-(bMarker ? pB+3 : pB), pBuffer.m_p, nSize);
					pWB = pBuffer;
					pWE = pWB+nSize;
				}
			}
			else if (nEncoding != 0)
			{
				int nSize = MultiByteToWideChar(nEncoding, 0, reinterpret_cast<char const*>(pB), pE-pB, NULL, 0);
				if (nSize)
				{
					pBuffer.Allocate(nSize);
					MultiByteToWideChar(nEncoding, 0, reinterpret_cast<char const*>(pB), pE-pB, pBuffer.m_p, nSize);
					pWB = pBuffer;
					pWE = pWB+nSize;
				}
			}
			else
			{
				// unknown encoding, try actual codepage
				int nSize = MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<char const*>(pB), pE-pB, NULL, 0);
				if (nSize)
				{
					pBuffer.Allocate(nSize);
					MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<char const*>(pB), pE-pB, pBuffer.m_p, nSize);
					pWB = pBuffer;
					pWE = pWB+nSize;
				}
				//// unknown encoding, assume plain ascii
				//pBuffer.Allocate(pE-pB);
				//wchar_t* pp = pBuffer;
				//for (BYTE const* p = pB; p < pE; ++p, ++pp)
				//	*pp = *p;
				//pWB = pBuffer;
				//pWE = pp;
			}
			if (a_pEncoderID)
				*a_pEncoderID = __uuidof(DocumentFactoryText);
			if (a_ppEncoderCfg)
			{
				CComPtr<IConfig> pConfig;
				DefaultConfig(&pConfig);
				if (!bMarker)
				{
					CComBSTR cCFGID_MARKER(CFGID_MARKER);
					pConfig->ItemValuesSet(1, &(cCFGID_MARKER.m_str), CConfigValue(false));
				}
				if (!bUTF8)
				{
					CComBSTR cCFGID_ENCODING(CFGID_ENCODING);
					pConfig->ItemValuesSet(1, &(cCFGID_ENCODING.m_str), CConfigValue(CFGVAL_UCS16));
				}
				*a_ppEncoderCfg = pConfig.Detach();
			}
			return a_pBuilder->InitEx(pWB, pWE-pWB, a_bstrPrefix, a_pBase);
		}
		if (a_pEncoderID)
			*a_pEncoderID = __uuidof(DocumentFactoryText);
		if (a_ppEncoderCfg)
			DefaultConfig(a_ppEncoderCfg);
		return a_pBuilder->InitEx(L"", 0, a_bstrPrefix, a_pBase); // empty file...
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIEncoderTextDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIEncoderTextDlg>,
	public CDialogResize<CConfigGUIEncoderTextDlg>
{
public:
	enum { IDC_CGTEXT_LABEL = 100, IDC_CGTEXT_ENCODING, IDC_CGTEXT_MARKER };

	BEGIN_DIALOG_EX(0, 0, 227, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Encoding:[0405]Kódování:"), IDC_CGTEXT_LABEL, 0, 2, 58, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGTEXT_ENCODING, 60, 0, 80, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
		CONTROL_CHECKBOX(_T("[0409]Unicode marker[0405]Značka unicode"), IDC_CGTEXT_MARKER, 147, 1, 80, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIEncoderTextDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEncoderTextDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEncoderTextDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEncoderTextDlg)
		DLGRESIZE_CONTROL(IDC_CGTEXT_LABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGTEXT_ENCODING, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGTEXT_MARKER, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEncoderTextDlg)
		CONFIGITEM_COMBOBOX(IDC_CGTEXT_ENCODING, CFGID_ENCODING)
		CONFIGITEM_CHECKBOX(IDC_CGTEXT_MARKER, CFGID_MARKER)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

STDMETHODIMP CDocumentFactoryText::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		CComBSTR cCFGID_ENCODING(CFGID_ENCODING);
		pCfg->ItemIns1ofN(cCFGID_ENCODING, CMultiLanguageString::GetAuto(L"[0409]Character encoding[0405]Kódování znaků"), CMultiLanguageString::GetAuto(L"[0409]Method used to encode international characters.[0405]Metoda použitá pro zapisování mezinárodních znaků."), CConfigValue(CFGVAL_UTF8), NULL);
		pCfg->ItemOptionAdd(cCFGID_ENCODING, CConfigValue(CFGVAL_UTF8), NULL, 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_ENCODING, CConfigValue(CFGVAL_UCS16), NULL, 0, NULL);
		CComBSTR cCFGID_MARKER(CFGID_MARKER);
		pCfg->ItemInsSimple(cCFGID_MARKER, CMultiLanguageString::GetAuto(L"[0409]Unicode marker[0405]Značka unicode"), CMultiLanguageString::GetAuto(L"[0409]The unicode marker is special invisible character that identifies a unicode text file.[0405]Značka unicode je speciální neviditelný znak, který identifikuje textové soubory unicode."), CConfigValue(true), NULL, 0, NULL);
		CConfigCustomGUI<&CLSID_DocumentFactoryText, CConfigGUIEncoderTextDlg>::FinalizeConfig(pCfg);
		*a_ppDefCfg = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryText::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	CComPtr<IDocumentText> pText;
	if (a_pDoc) a_pDoc->QueryFeatureInterface(__uuidof(IDocumentText), reinterpret_cast<void**>(&pText));
	if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_TEXT);
	return pText ? S_OK : S_FALSE;
}

STDMETHODIMP CDocumentFactoryText::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	try
	{
		CComPtr<IDocumentText> pText;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentText), reinterpret_cast<void**>(&pText));

		bool bMarker = true;
		bool bUTF8 = true;
		if (a_pCfg)
		{
			CConfigValue cVal;
			a_pCfg->ItemValueGet(CComBSTR(CFGID_MARKER), &cVal);
			if (cVal.TypeGet() == ECVTBool)
				bMarker = cVal;
			a_pCfg->ItemValueGet(CComBSTR(CFGID_ENCODING), &cVal);
			if (cVal.TypeGet() == ECVTString)
				bUTF8 = wcscmp(cVal, CFGVAL_UTF8) == 0;
		}

		CReadLock<IDocument> cLock(a_pDoc);

		ULONG nLines = 0;
		pText->LinesGetCount(&nLines);
		if (bUTF8)
		{
			if (bMarker)
			{
				static BYTE const aMarker8[] = {0xef, 0xbb, 0xbf};
				if (FAILED(a_pDst->Write(sizeof aMarker8, aMarker8)))
					return E_FAIL;
			}
			for (ULONG i = 0; i < nLines; ++i)
			{
				CComBSTR bstr;
				if (FAILED(pText->LineGet(i, &bstr)))
					return E_FAIL;
				ULONG const nLine = bstr.Length();
				if (nLine)
				{
					int nLen = WideCharToMultiByte(CP_UTF8, 0, bstr.m_str, nLine, NULL, 0, NULL, NULL);
					if (nLen)
					{
						CAutoVectorPtr<char> cBuffer(new char[nLen]);
						WideCharToMultiByte(CP_UTF8, 0, bstr.m_str, nLine, cBuffer, nLen, NULL, NULL);
						if (FAILED(a_pDst->Write(static_cast<ULONG>(nLen), reinterpret_cast<BYTE const*>(cBuffer.m_p))))
							return E_FAIL;
					}
				}
				if (i+1 != nLines)
				{
					static char const aNewLine8[] = {0xd, 0xa};
					if (FAILED(a_pDst->Write(sizeof aNewLine8, reinterpret_cast<BYTE const*>(aNewLine8))))
						return E_FAIL;
				}
			}
		}
		else
		{
			if (bMarker)
			{
				static BYTE const aMarker[] = {0xff, 0xfe};
				if (FAILED(a_pDst->Write(sizeof aMarker, aMarker)))
					return E_FAIL;
			}
			for (ULONG i = 0; i < nLines; ++i)
			{
				CComBSTR bstr;
				if (FAILED(pText->LineGet(i, &bstr)))
					return E_FAIL;
				ULONG const nLine = bstr.Length();
				if (nLine)
				{
					if (FAILED(a_pDst->Write(static_cast<ULONG>(nLine*sizeof(WCHAR)), reinterpret_cast<BYTE const*>(bstr.m_str))))
						return E_FAIL;
				}
				if (i+1 != nLines)
				{
					static wchar_t const aNewLine[] = {0xd, 0xa};
					if (FAILED(a_pDst->Write(sizeof aNewLine, reinterpret_cast<BYTE const*>(aNewLine))))
						return E_FAIL;
				}
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryText::InitEx(OLECHAR const* a_pText, ULONG a_nLength, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComObject<CDocumentText>* pDoc = NULL;
		CComObject<CDocumentText>::CreateInstance(&pDoc);
		CComPtr<IDocumentData> pTmp = pDoc;
		pDoc->InitLines(a_pText, a_pText+a_nLength);
		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


// CDocumentFactoryText::IDocumentCreator

STDMETHODIMP CDocumentFactoryText::Category(ILocalizedString** a_ppCategory)
{
	try
	{
		*a_ppCategory = 0;
		return E_NOTIMPL;
	}
	catch (...)
	{
		return a_ppCategory == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryText::State(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText)
{
	try
	{
		if (a_pEnableDocName)
			*a_pEnableDocName = TRUE;
		if (a_ppButtonText)
		{
			*a_ppButtonText = NULL;
			*a_ppButtonText = new CMultiLanguageString(L"[0409]Create[0405]Vytvořit");
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>

class ATL_NO_VTABLE CConfigGUINewTextDoc : 
	public CCustomConfigResourcelessWndImpl<CConfigGUINewTextDoc>
{
public:
	BEGIN_DIALOG_EX(0, 0, 156, 20, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_TITLE = 100,
		IDC_DOCUMENTATION,
		IDC_ENCODING,
		IDC_MARKER
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]New plain text[0405]Nový prostý text"), IDC_TITLE, 0, 0, 156, 20, WS_VISIBLE|SS_ENDELLIPSIS, 0)
		CONTROL_LTEXT(_T("[0409]Plain text does not contain any formatting. It can be used to store notes, code fragments, web pages, XML data, etc.[0405]Prostý text neobsahuje žádné formátování. Lze ho použit pro zapisování poznámek, zdrojových kódů, webových stránek, XML dat a podobně."), IDC_STATIC, 0, 24, 156, 32, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]More information[0405]Více informací"), IDC_DOCUMENTATION, 0, 60, 75, 8, WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_CONTROL(_T(""), IDC_SEPLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 80, 41, 139, 1, 0)
		CONTROL_LTEXT(_T("[0409]Encoding:[0405]Kódování:"), IDC_STATIC, 0, 74, 58, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_ENCODING, 60, 72, 96, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
		CONTROL_CHECKBOX(_T("[0409]Unicode marker[0405]Značka unicode"), IDC_MARKER, 0, 88, 100, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUINewTextDoc)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUINewTextDoc>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUINewTextDoc)
		CONFIGITEM_COMBOBOX(IDC_ENCODING, CFGID_ENCODING)
		CONFIGITEM_CHECKBOX(IDC_MARKER, CFGID_MARKER)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfUnderline = 1;
		lf.lfHeight *= 2;
		GetDlgItem(IDC_TITLE).SetFont(m_font.CreateFontIndirect(&lf));

		m_wndDocumentation.SubclassWindow(GetDlgItem(IDC_DOCUMENTATION));
		m_wndDocumentation.SetHyperLink(_T("http://www.rw-designer.com/text-document"));
		return 1;  // Let the system set the focus
	}

private:
	CFont m_font;
	CHyperLink m_wndDocumentation;
};

// {6EA96590-662A-4755-8DAF-547F41638E9F}
extern const GUID s_tNewTextGUIID = {0x6ea96590, 0x662a, 0x4755, {0x8d, 0xaf, 0x54, 0x7f, 0x41, 0x63, 0x8e, 0x9f}};
STDMETHODIMP CDocumentFactoryText::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_ENCODING(CFGID_ENCODING);
		pCfg->ItemIns1ofN(cCFGID_ENCODING, CMultiLanguageString::GetAuto(L"[0409]Character encoding[0405]Kódování znaků"), CMultiLanguageString::GetAuto(L"[0409]Method used to encode international characters.[0405]Metoda použitá pro zapisování mezinárodních znaků."), CConfigValue(CFGVAL_UTF8), NULL);
		pCfg->ItemOptionAdd(cCFGID_ENCODING, CConfigValue(CFGVAL_UTF8), NULL, 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_ENCODING, CConfigValue(CFGVAL_UCS16), NULL, 0, NULL);
		CComBSTR cCFGID_MARKER(CFGID_MARKER);
		pCfg->ItemInsSimple(cCFGID_MARKER, CMultiLanguageString::GetAuto(L"[0409]Unicode marker[0405]Značka unicode"), CMultiLanguageString::GetAuto(L"[0409]The unicode marker is special invisible character that identifies a unicode text file.[0405]Značka unicode je speciální neviditelný znak, který identifikuje textové soubory unicode."), CConfigValue(true), NULL, 0, NULL);

		CConfigCustomGUI<&s_tNewTextGUIID, CConfigGUINewTextDoc>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryText::Activate(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryText* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComObject<CDocumentText>* pDoc = NULL;
		CComObject<CDocumentText>::CreateInstance(&pDoc);
		CComPtr<IDocumentData> pTmp = pDoc;
		if (pDoc == NULL)
			return E_FAIL;

		//if (a_pBase && a_bstrPrefix == NULL)
		//{
		//	CComPtr<IConfig> pConfig;
		//	DefaultConfig(&pConfig);
		//	CopyConfigValues(pConfig, a_pConfig);
		//	a_pBase->EncoderSet(CLSID_DocumentFactoryText, pConfig);
		//}
		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

STDMETHODIMP CDocumentFactoryText::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);

		static IRPolyPoint const paper[] = { {50, 8}, {206, 8}, {206, 248}, {50, 248} };
		static IRPathPoint const pin1[] =
		{
			{0, 24, 10, -9, 10, 9},
			{26, 0, 10, 0, -21, 0},
			{44, 14, 0, 0, -2, -7},
			{212, 14, 2, -7, 0, 0},
			{230, 0, 21, 0, -10, 0},
			{256, 24, -10, 9, -10, -9},
			{230, 48, -10, 0, 21, 0},
			{212, 34, 0, 0, 2, 7},
			{44, 34, -2, 7, 0, 0},
			{26, 48, -21, 0, 10, 0},
		};
		static IRPathPoint const pin2[] =
		{
			{0, 232, 10, -9, 10, 9},
			{26, 208, 10, 0, -21, 0},
			{44, 222, 0, 0, -2, -7},
			{212, 222, 2, -7, 0, 0},
			{230, 208, 21, 0, -10, 0},
			{256, 232, -10, 9, -10, -9},
			{230, 256, -10, 0, 21, 0},
			{212, 242, 0, 0, 2, 7},
			{44, 242, -2, 7, 0, 0},
			{26, 256, -21, 0, 10, 0},
		};
		static IRPolyPoint const line1[] =
		{
			{68.5, 45.5},
			{84.5, 50.5},
			{88.5, 40.5},
			{98.5, 48.5},
			{109.5, 48.5},
			{116.5, 38.5},
			{122.5, 46.5},
			{130.5, 39.5},
			{139.5, 47.5},
			{145.5, 41.5},
			{155.5, 49.5},
			{163.5, 40.5},
			{170.5, 49.5},
			{182.5, 46.5},
		};
		static IRPolyPoint const line2[] =
		{
			{69.5, 78.5},
			{75.5, 71.5},
			{85.5, 71.5},
			{94.5, 80.5},
			{101.5, 71.5},
			{109.5, 79.5},
			{115.5, 73.5},
			{125.5, 77.5},
			{131.5, 71.5},
			{142.5, 80.5},
			{149.5, 70.5},
			{161.5, 70.5},
			{169.5, 78.5},
			{180.5, 73.5},
		};
		static IRPolyPoint const line3[] =
		{
			{70.5, 101.5},
			{77.5, 108.5},
			{85.5, 105.5},
			{90.5, 99.5},
			{96.5, 109.5},
			{108.5, 109.5},
			{114.5, 101.5},
			{119.5, 109.5},
			{132.5, 101.5},
			{137.5, 109.5},
		};
		static IRPolyPoint const shadow1[] =
		{
			{52, 240}, {204, 240}, {204, 246}, {52, 246},
		};
		static IRPolyPoint const shadow2[] =
		{
			{52, 10}, {204, 10}, {204, 16}, {52, 16},
		};
		static IRPathPoint const shadow3[] =
		{
			{6, 27, 4, 8, 5, 2},
			{24, 46, 13, -1, -11.9647, 0.920358},
			{40, 35, 1.08465, -4.33861, -1, 4},
			{52, 31, 0, 0, 0, 0},
			{52, 26, 0, 0, 0, 0},
			{38, 28, -3, 3, 3, -3},
			{23, 38, -6, 0, 8, 0},
		};
		static IRFill shadowMat(0x5f000000);
		static IROutlinedFill pinMat(pSI->GetMaterial(ESMScheme2Color2, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		static IROutlinedFill paperMat(pSI->GetMaterial(ESMInterior, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		static IRStroke lineMat(pSI->GetSRGBColor(ESMContrast), 5.0f/256.0f);
		static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
		cRenderer(&canvas, itemsof(pin1), pin1, &pinMat);
		cRenderer(&canvas, itemsof(pin2), pin2, &pinMat);
		cRenderer(&canvas, itemsof(paper), paper, &paperMat);
		cRenderer(&canvas, itemsof(shadow1), shadow1, &shadowMat);
		cRenderer(&canvas, itemsof(shadow2), shadow2, &shadowMat);
		cRenderer(&canvas, itemsof(shadow3), shadow3, &shadowMat);
		cRenderer(&canvas, itemsof(line1), line1, &lineMat);
		cRenderer(&canvas, itemsof(line2), line2, &lineMat);
		cRenderer(&canvas, itemsof(line3), line3, &lineMat);

		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryText::GetThumbnail(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo, HRESULT(fnRescaleImage)(ULONG a_nSrcSizeX, ULONG a_nSrcSizeY, DWORD const* a_pSrcData, bool a_bSrcAlpha, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds))
{
	try
	{
		CComPtr<IDocumentText> pDocText;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentText), reinterpret_cast<void**>(&pDocText));
		if (pDocText == NULL)
			return E_RW_UNKNOWNINPUTFORMAT;

		ULONG nLines = 0;
		pDocText->LinesGetCount(&nLines);
		CComBSTR bstrText;
		pDocText->TextGet(&bstrText);

		CDC cDCMain;
		cDCMain.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		CDC cDC;
		cDC.CreateCompatibleDC(cDCMain);
		//CFont cFont;
		//cFont.CreateFont(
		//	-m_cData.fSize,
		//	0,
		//	0,
		//	0,
		//	m_cData.bBold ? FW_BOLD : FW_NORMAL,
		//	m_cData.bItalic,
		//	0,
		//	0,
		//	DEFAULT_CHARSET,
		//	OUT_DEFAULT_PRECIS,
		//	CLIP_DEFAULT_PRECIS,
		//	m_eRasterizationMode == ERMSmooth ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY,
		//	DEFAULT_PITCH|FF_DONTCARE,
		//	CW2CT(m_cData.lfFaceName)
		//	);
		// cFont.Attach(cDC.SelectFont(cFont.Detach()));
		COLE2T str(bstrText.m_str);
		//RECT rc = {0, 0, 2048, 0};
		//cDC.DrawText(str, -1, &rc, DT_LEFT|DT_TOP|DT_CALCRECT);
		//TEXTMETRIC tm;
		//cDC.GetTextMetrics(&tm);
		//rc.right += tm.tmHeight; // try to compensate for bold italic fonts
		//if (rc.right == 0 || rc.bottom == 0)
		//{
		//	m_cTextBitmap.Free();
		//	m_nTextSizeX = rc.right;
		//	m_nTextSizeY = rc.bottom;
		//	m_bCacheValid = true;
		//	return;
		//}

		ULONG const nSizeY = a_nSizeY > 256 ? a_nSizeY : 256;
		ULONG const nSizeX = a_nSizeX * nSizeY / a_nSizeY;

		CBitmap cBmp;
		cBmp.CreateCompatibleBitmap(cDCMain, nSizeX, nSizeY);
		cBmp.Attach(cDC.SelectBitmap(cBmp.Detach()));
		cDC.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		cDC.SetBkColor(GetSysColor(COLOR_WINDOW));
		RECT rc = { 0, 0, nSizeX, nSizeY };
		cDC.FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));
		cDC.DrawText(str, -1, &rc, DT_LEFT | DT_TOP);

		//cFont.Attach(cDC.SelectFont(cFont.Detach()));

		CAutoVectorPtr<BYTE> pData(new BYTE[rc.right * rc.bottom << 2]);
		BITMAPINFO tBMPInfo;
		ZeroMemory(&tBMPInfo, sizeof(tBMPInfo));
		tBMPInfo.bmiHeader.biSize = sizeof(tBMPInfo.bmiHeader);
		tBMPInfo.bmiHeader.biWidth = rc.right;
		tBMPInfo.bmiHeader.biHeight = -rc.bottom;
		tBMPInfo.bmiHeader.biCompression = BI_RGB;
		tBMPInfo.bmiHeader.biPlanes = 1;
		tBMPInfo.bmiHeader.biBitCount = 32;
		cBmp.Attach(cDC.SelectBitmap(cBmp.Detach()));
		cBmp.GetDIBits(cDC, 0, rc.bottom, pData.m_p, &tBMPInfo, DIB_RGB_COLORS);
		//for (LONG y = 0; y < rc.bottom; ++y)
		//{
		//	BYTE const* p = pSrc;
		//	for (LONG x = 0; x < rc.right; ++x)
		//	{
		//		*pDst = *p;
		//		p += 3;
		//		++pDst;
		//	}
		//	pSrc += nLinearSize;
		//}
		HRESULT hRes = fnRescaleImage(rc.right, rc.bottom, reinterpret_cast<DWORD const*>(pData.m_p), false, a_nSizeX, a_nSizeY, a_pBGRAData, a_prcBounds);
		if (a_pbstrInfo)
		{
			wchar_t sz[128] = L"";
			CComBSTR bstrTempl;
			CMultiLanguageString::GetLocalized(L"[0409]Line count: %i[0405]Počet řádek: %i", a_tLocaleID, &bstrTempl);
			swprintf(sz, bstrTempl, nLines);
			*a_pbstrInfo = SysAllocString(sz);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
