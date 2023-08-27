// Translator.cpp : Implementation of CTranslator

#include "stdafx.h"
#include "Translator.h"
#include <MultiLanguageString.h>
#include <XPGUI.h>
#include <wininet.h>

#pragma comment(lib, "wininet")

inline void AddUTFString(char const* a_pString, std::vector<char>& a_cOut)
{
	while (*a_pString)
	{
		a_cOut.push_back(*a_pString);
		++a_pString;
	}
}

inline void AddChar(OLECHAR a_cChar, std::vector<char>& a_cOut)
{
	if (a_cChar & 0xff80)
	{
		if (a_cChar & 0xf800)
		{
			// 0x0800-0xffff - 3 bytes
			a_cOut.push_back(static_cast<char const>(0xe0 | ((a_cChar>>12)&0x0f)));
			a_cOut.push_back(static_cast<char const>(0x80 | ((a_cChar>>6)&0x3f)));
			a_cOut.push_back(static_cast<char const>(0x80 | (a_cChar&0x3f)));
		}
		else
		{
			// 0x0080-0x07ff - 2 bytes
			a_cOut.push_back(static_cast<char const>(0xc0 | ((a_cChar>>6)&0x1f)));
			a_cOut.push_back(static_cast<char const>(0x80 | (a_cChar&0x3f)));
		}
	}
	else
	{
		// 0x0000-0x007f - 1 byte
		a_cOut.push_back(static_cast<char const>(a_cChar));
	}
}

inline void AddEscapedString(OLECHAR const* a_pString, std::vector<char>& a_cOut)
{
	while (*a_pString)
	{
		switch (*a_pString)
		{
		case L'\r': AddUTFString("\\r", a_cOut); break;
		case L'\n': AddUTFString("\\n", a_cOut); break;
		case L'\t': AddUTFString("\\t", a_cOut); break;
		case L'\"': AddUTFString("\\\"", a_cOut); break;
		case L'\\': AddUTFString("\\\\", a_cOut); break;
		default:
			AddChar(*a_pString, a_cOut);
		}
		++a_pString;
	}
}



// CTranslator

STDMETHODIMP CTranslator::Translate(BSTR a_bstrEng, LCID a_tLocaleID, BSTR* a_pTranslated)
{
	try
	{
		*a_pTranslated = NULL;
		ULONG const nLen = SysStringLen(a_bstrEng);
		if (nLen < 2 || nLen > 10000)
			return E_FAIL; // empty or single character or extremely long strings are not translated
		if (LANGIDFROMLCID(a_tLocaleID) == 0x2c09)
		{
			CComBSTR bstr(a_bstrEng);
			TranslateInPlace(bstr.Length()+1, bstr.m_str, a_tLocaleID);
			*a_pTranslated = bstr.Detach();
			return S_OK;
		}
		ObjectLock cLock(this);
		STable& cTbl = GetTable(LANGIDFROMLCID(a_tLocaleID));
		//CComBSTR bstrTmp;
		//if (a_bstrEng[nLen-1] == L':')
		//{
		//	bstrTmp.Attach(SysAllocStringLen(a_bstrEng, nLen-1));
		//	a_bstrEng = bstrTmp;
		//}
		if (m_bMonitor)
		{
			STranslation& sTr = cTbl.cTable[a_bstrEng];
			if (sTr.bstrTranslation.m_str == NULL)
			{
				++sTr.nUses;
				cTbl.bChanged = true;
				if (m_bHilight)
				{
					CComBSTR bstr(L"[*]");
					bstr += a_bstrEng;
					*a_pTranslated = bstr.Detach();
					return S_OK;
				}
				return E_RW_ITEMNOTFOUND;
			}
			return sTr.bstrTranslation.CopyTo(a_pTranslated);
		}
		else
		{
			CTable::iterator iTr = cTbl.cTable.find(a_bstrEng);
			if (iTr == cTbl.cTable.end() || iTr->second.bstrTranslation.m_str == NULL)
				return E_RW_ITEMNOTFOUND;
			return iTr->second.bstrTranslation.CopyTo(a_pTranslated);
		}
	}
	catch (...)
	{
		return a_pTranslated ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CTranslator::TranslateInPlace(ULONG a_nBufLen, OLECHAR* a_pszBuffer, LCID a_tLocaleID)
{
	try
	{
		a_pszBuffer[a_nBufLen-1] = L'\0';
		if (a_pszBuffer == NULL || a_pszBuffer[0] == L'\0' || a_pszBuffer[1] == L'\0')
			return S_FALSE; // empty or single character string are not translated
		if (LANGIDFROMLCID(a_tLocaleID) == 0x2c09)
		{
			for (OLECHAR* pEnd = a_pszBuffer+a_nBufLen; a_pszBuffer < pEnd && *a_pszBuffer; ++a_pszBuffer)
			{
				switch (*a_pszBuffer)
				{
				case L'a':
				case L'A': *a_pszBuffer = L'4'; break;
				case L'b':
				case L'B': *a_pszBuffer = L'8'; break;
				case L'C': *a_pszBuffer = L'c'; break;
				case L'D': *a_pszBuffer = L'd'; break;
				case L'e':
				case L'E': *a_pszBuffer = L'3'; break;
				case L'F': *a_pszBuffer = L'f'; break;
				case L'G': *a_pszBuffer = L'g'; break;
				case L'H': *a_pszBuffer = L'h'; break;
				//case L'i':
				//case L'I': *a_pszBuffer = L'|'; break;
				case L'I': *a_pszBuffer = L'i'; break;
				case L'J': *a_pszBuffer = L'j'; break;
				case L'K': *a_pszBuffer = L'k'; break;
				case L'l':
				case L'L': *a_pszBuffer = L'1'; break;
				case L'M': *a_pszBuffer = L'm'; break;
				case L'N': *a_pszBuffer = L'n'; break;
				case L'o':
				case L'O': *a_pszBuffer = L'0'; break;
				case L'P': *a_pszBuffer = L'p'; break;
				case L'Q': *a_pszBuffer = L'q'; break;
				case L'R': *a_pszBuffer = L'r'; break;
				case L's':
				case L'S': *a_pszBuffer = L'5'; break;
				case L't':
				case L'T': *a_pszBuffer = L'7'; break;
				case L'U': *a_pszBuffer = L'u'; break;
				case L'V': *a_pszBuffer = L'v'; break;
				case L'W': *a_pszBuffer = L'w'; break;
				case L'X': *a_pszBuffer = L'x'; break;
				case L'Y': *a_pszBuffer = L'y'; break;
				case L'z':
				case L'Z': *a_pszBuffer = L'2'; break;
				case L'%': ++a_pszBuffer; break;
				}
			}
			return S_OK;
		}
		ObjectLock cLock(this);
		STable& cTbl = GetTable(LANGIDFROMLCID(a_tLocaleID));
		if (m_bMonitor)
		{
			STranslation& sTr = cTbl.cTable[a_pszBuffer];
			if (sTr.bstrTranslation.m_str == NULL)
			{
				++sTr.nUses;
				cTbl.bChanged = true;
				if (m_bHilight)
				{
					memmove(a_pszBuffer+3, a_pszBuffer, (a_nBufLen-6)*sizeof*a_pszBuffer);
					memcpy(a_pszBuffer, L"[*]", 3*sizeof*a_pszBuffer);
					return S_OK;
				}
				return E_RW_ITEMNOTFOUND;
			}
			wcsncpy(a_pszBuffer, sTr.bstrTranslation.m_str, a_nBufLen-1);
			return S_OK;
		}
		else
		{
			CTable::iterator iTr = cTbl.cTable.find(a_pszBuffer);
			if (iTr == cTbl.cTable.end() || iTr->second.bstrTranslation.m_str == NULL)
				return E_RW_ITEMNOTFOUND;
			wcsncpy(a_pszBuffer, iTr->second.bstrTranslation.m_str, a_nBufLen-1);
			return S_OK;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CTranslator::Initialize(IApplicationInfo* a_pInfo)
{
	try
	{
		CComBSTR bstrFolder;
		a_pInfo->AppDataFolder(&bstrFolder);
		ULONG nLen = bstrFolder.Length();
		if (nLen+3 < itemsof(m_szAppPath))
		{
			wcscpy(m_szAppPath+1, bstrFolder.m_str+1);
			m_szAppPath[nLen] = L'\\';
			m_szAppPath[nLen+1] = L'\0';
			m_szAppPath[0] = bstrFolder.m_str[0]; // trigger of validity
			ObjectLock cLock(this);
			m_cLanguages.clear();
		}
		a_pInfo->Identifier(&m_bstrAppID);
		a_pInfo->Version(&m_nVersion);
		m_pInfo = a_pInfo;

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CTranslator::Finalize(BOOL a_bAllowPOFileUpdates)
{
	try
	{
		// TODO: stop eventual updating thread
		if (a_bAllowPOFileUpdates == 0)
			return S_OK;
		for (CLanguages::const_iterator iL = m_cLanguages.begin(); iL != m_cLanguages.end(); ++iL)
		{
			if (iL->second.bChanged)
			{
				std::vector<char> cOut;
				SerializePOFile(iL->second, cOut);
				TCHAR szPath[MAX_PATH];
				GetPathToPOFile(iL->first, szPath, MAX_PATH);
				TCHAR szPath2[MAX_PATH+4];
				_stprintf(szPath2, _T("%s.tmp"), szPath);
				HANDLE hFile = CreateFile(szPath2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					DWORD nWritten = 0;
					WriteFile(hFile, &(cOut[0]), cOut.size(), &nWritten, NULL);
					CloseHandle(hFile);
					if (nWritten == cOut.size())
					{
						DeleteFile(szPath);
						MoveFile(szPath2, szPath);
					}
					else
					{
						DeleteFile(szPath2);
					}
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

STDMETHODIMP CTranslator::LangIcon(WORD a_wLangID, HICON* a_pLangIcon)
{
	try
	{
		*a_pLangIcon = NULL;
		if (a_wLangID == MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))
		{
			*a_pLangIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_LANG0409), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			return S_OK;
		}
		else if (a_wLangID == MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT))
		{
			*a_pLangIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_LANG0405), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			return S_OK;
		}
		ObjectLock cLock(this);
		STable& sTbl = GetTable(a_wLangID);
		if (!sTbl.IconValid())
			return E_RW_ITEMNOTFOUND;
		CAutoVectorPtr<BYTE> pIconRes(new BYTE[sizeof(BITMAPINFOHEADER)+16*16*4+2*16]);
		BITMAPINFOHEADER* pBIH = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
		pBIH->biSize = sizeof*pBIH;
		pBIH->biWidth = 16;
		pBIH->biHeight = 32;
		pBIH->biPlanes = 1;
		pBIH->biBitCount = 32;
		pBIH->biCompression = BI_RGB;
		pBIH->biSizeImage = 16*16*4+2*16;
		pBIH->biXPelsPerMeter = 0x8000;
		pBIH->biYPelsPerMeter = 0x8000;
		pBIH->biClrUsed = 0;
		pBIH->biClrImportant = 0;
		DWORD* pXOR = reinterpret_cast<DWORD*>(pBIH+1);
		for (ULONG y = 0; y < 16; ++y)
			for (ULONG x = 0; x < 16; ++x)
				pXOR[x+((15-y)<<4)] = sTbl.aIcon[x+(y<<4)] ? sTbl.aIcPal[sTbl.aIcon[x+(y<<4)]-1] : 0;
		BYTE* pAND = reinterpret_cast<BYTE*>(pXOR+16*16);
		// create mask
		for (ULONG y = 0; y < 16; ++y)
		{
			BYTE* pA = pAND+2*y;
			DWORD* pC = pXOR+16*y;
			for (ULONG x = 0; x < 16; ++x, ++pC)
			{
				BYTE* p = pA+(x>>3);
				if (*pC&0xff000000)
					*p &= ~(0x80 >> (x&7));
				else
					*p |= 0x80 >> (x&7);
			}
		}

		*a_pLangIcon = CreateIconFromResourceEx(pIconRes, sizeof(BITMAPINFOHEADER)+16*16*4+2*16, TRUE, 0x00030000, 16, 16, LR_DEFAULTCOLOR);
		return *a_pLangIcon ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return a_pLangIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CTranslator::LangInfo(WORD a_wLangID, BOOL* a_pBuiltIn, ULONG* a_pTimestamp, ULONG* a_pStandard, ULONG* a_pCustom, ULONG* a_pMissing)
{
	if (a_wLangID == MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT) || a_wLangID == MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT) || a_wLangID == 0x2c09)
	{
		if (a_pBuiltIn)
			*a_pBuiltIn = TRUE;
		if (a_pTimestamp)
			*a_pTimestamp = 0;
		if (a_pStandard)
			*a_pStandard = 0;
		if (a_pCustom)
			*a_pCustom = 0;
		if (a_pMissing)
			*a_pMissing = 0;
		return S_OK;
	}
	if (a_pBuiltIn)
		*a_pBuiltIn = FALSE;
	ObjectLock cLock(this);
	STable& sTbl = GetTable(a_wLangID);
	if (a_pTimestamp)
		*a_pTimestamp = sTbl.nTimestamp;
	if (a_pStandard || a_pCustom || a_pMissing)
	{
		ULONG nStandard = 0;
		ULONG nCustom = 0;
		ULONG nMissing = 0;
		for (CTable::const_iterator i = sTbl.cTable.begin(); i != sTbl.cTable.end(); ++i)
			if (i->second.bstrTranslation.m_str)
			{
				ULONG nCust = i->second.bCustom;
				nStandard += 1-nCust;
				nCustom += nCust;
			}
			else
			{
				++nMissing;
			}
		if (a_pStandard)
			*a_pStandard = nStandard;
		if (a_pCustom)
			*a_pCustom = nCustom;
		if (a_pMissing)
			*a_pMissing = nMissing;
	}
	return S_OK;
}

STDMETHODIMP CTranslator::StringsEnum(WORD a_wLangID, IEnumStrings** a_ppStrings)
{
	try
	{
		*a_ppStrings = NULL;
		ObjectLock cLock(this);
		STable& sTbl = GetTable(a_wLangID);
		CComPtr<IEnumStringsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
		for (CTable::const_iterator i = sTbl.cTable.begin(); i != sTbl.cTable.end(); ++i)
		{
			if (FAILED(pTmp->Insert(i->first)))
				return E_FAIL;
		}
		*a_ppStrings = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppStrings ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CTranslator::StringInfo(WORD a_wLangID, BSTR a_bstrOrig, BSTR* a_pbstrTransl, BOOL* a_pCustom, ULONG* a_pPriority)
{
	try
	{
		if (a_pbstrTransl) *a_pbstrTransl = NULL;
		if (a_pCustom) *a_pCustom = FALSE;
		if (a_pPriority) *a_pPriority = 0;
		ObjectLock cLock(this);
		STable& sTbl = GetTable(a_wLangID);
		CTable::const_iterator i = sTbl.cTable.find(a_bstrOrig);
		if (i == sTbl.cTable.end())
			return E_RW_ITEMNOTFOUND;
		if (a_pCustom) *a_pCustom = i->second.bCustom;
		if (a_pPriority) *a_pPriority = i->second.nUses;
		if (a_pbstrTransl) *a_pbstrTransl = i->second.bstrTranslation.Copy();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CTranslator::StringSet(WORD a_wLangID, BSTR a_bstrOrig, BSTR a_bstrTransl, BOOL a_bCustom)
{
	try
	{
		ObjectLock cLock(this);
		STable& sTbl = GetTable(a_wLangID);
		if (a_bstrTransl && *a_bstrTransl)
		{
			STranslation& sTr = sTbl.cTable[a_bstrOrig];
			sTr.bCustom = a_bCustom;
			sTr.bstrTranslation = a_bstrTransl;
		}
		else
		{
			// delete
			sTbl.cTable.erase(a_bstrOrig);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static void GenerateFileHeader(LPCSTR a_pszBoundary, LPCSTR a_pszFieldName, LPCSTR a_pszPathName, LPCSTR a_pszContentType, std::string& a_strOut)
{
	char szTmp[1024] = "";
	sprintf(szTmp, "--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: %s\r\n\r\n", a_pszBoundary, a_pszFieldName, a_pszPathName, a_pszContentType);
	a_strOut = szTmp;
}
static void GenerateFieldHeader(LPCSTR a_pszBoundary, LPCSTR a_pszFieldName, LPCSTR a_pszFieldData, std::string& a_strOut)
{
	char szTmp[1024] = "";
	sprintf(szTmp, "--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", a_pszBoundary, a_pszFieldName, a_pszFieldData);
	a_strOut = szTmp;
}

STDMETHODIMP CTranslator::Synchronize(WORD a_wLangID, BYTE a_bUntranslatedStrings)
{
	try
	{
		ObjectLock cLock(this);
		STable& sTbl = GetTable(a_wLangID);
		std::vector<char> cOut;

		OLECHAR m_szWebServer[32];
		OLECHAR m_szWebRoot[64];
		CComBSTR m_bstrEmail;
		CComBSTR m_bstrPassHash;
		CComBSTR bstrServer;
		m_pInfo->Account(&bstrServer, &m_bstrEmail, &m_bstrPassHash);
		if (bstrServer.m_str && _wcsnicmp(bstrServer.m_str, L"http://", 7) == 0)
		{
			LPWSTR p = wcschr(bstrServer.m_str+7, L'/');
			wcsncpy(m_szWebRoot, p, itemsof(m_szWebRoot));
			m_szWebRoot[itemsof(m_szWebRoot)-32] = L'\0';
			int nLen = wcslen(m_szWebRoot);
			if (nLen && m_szWebRoot[nLen-1] == L'/')
				--nLen;
			wcscpy(m_szWebRoot+nLen, L"/translation-sync.php");
			*p = L'\0';
			wcsncpy(m_szWebServer, bstrServer.m_str+7, itemsof(m_szWebServer));
			m_szWebServer[itemsof(m_szWebServer)-1] = L'\0';
		}

		//static char const szMarker[] = {0xef, 0xbb, 0xbf};
		//AddUTFString(szMarker, cOut);

		for (CTable::const_iterator i = sTbl.cTable.begin(); i != sTbl.cTable.end(); ++i)
		{
			if (i->second.bstrTranslation.Length() && i->second.bCustom)
			{
				AddUTFString("O ", cOut);
				AddEscapedString(i->first, cOut);
				AddUTFString("\nT ", cOut);
				AddEscapedString(i->second.bstrTranslation, cOut);
				AddUTFString("\n", cOut);
			}
			else if (m_bMonitor && i->second.bstrTranslation.Length() == 0)
			{
				AddUTFString("O ", cOut);
				AddEscapedString(i->first, cOut);
				AddUTFString("\n", cOut);
			}
		}

		HINTERNET hSession = InternetOpen(_T("RealWorld - translation table sync"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hSession == NULL)
			return E_FAIL;

		HINTERNET hConnect = InternetConnect(hSession, COLE2CT(m_szWebServer), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
		if (!hConnect)
		{
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		HINTERNET hRequest = HttpOpenRequest(hConnect, _T("POST"), COLE2CT(m_szWebRoot), NULL, NULL, NULL,
								INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
								INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | 
								INTERNET_FLAG_KEEP_CONNECTION |
								INTERNET_FLAG_NO_AUTH |
								INTERNET_FLAG_NO_AUTO_REDIRECT |
								INTERNET_FLAG_NO_COOKIES |
								INTERNET_FLAG_NO_UI |
								INTERNET_FLAG_RELOAD, NULL);
		if (!hRequest)
		{
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		char szMultipartBoundary[40];
		sprintf(szMultipartBoundary, "---------------------------%04X%04X%04X", rand() & 0xffff, rand() & 0xffff, rand() & 0xffff);
		char szContentTypeHeader[84];
		sprintf(szContentTypeHeader, "Content-Type: multipart/form-data; boundary=%s", szMultipartBoundary);
		char szBodyTrailer[46];
		sprintf(szBodyTrailer, "--%s--\r\n", szMultipartBoundary);

		BOOL bResult = HttpAddRequestHeaders(hRequest, CA2T(szContentTypeHeader), -1, HTTP_ADDREQ_FLAG_ADD);
		if (!bResult)
		{
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		std::string strLangIDField;
		char szTmp[256];
		sprintf(szTmp, "%04x", a_wLangID);
		GenerateFieldHeader(szMultipartBoundary, "langid", szTmp, strLangIDField);

		std::string strTimestampField;
		sprintf(szTmp, "%i", sTbl.nTimestamp);
		GenerateFieldHeader(szMultipartBoundary, "timestamp", szTmp, strTimestampField);

		std::string strAppIDField;
		GenerateFieldHeader(szMultipartBoundary, "appid", CW2A(m_bstrAppID.m_str, CP_UTF8), strAppIDField);

		std::string strVersionField;
		sprintf(szTmp, "%i.%i.%i", (m_nVersion>>16)&0xffff, (m_nVersion>>8)&0xff, m_nVersion&0xff);
		GenerateFieldHeader(szMultipartBoundary, "version", szTmp, strVersionField);

		std::string strEmailField;
		std::string strPasswordField;
		if (m_bstrEmail.Length() && m_bstrPassHash.Length())
		{
			GenerateFieldHeader(szMultipartBoundary, "email", CW2A(m_bstrEmail.m_str, CP_UTF8), strEmailField);
			GenerateFieldHeader(szMultipartBoundary, "passwd", CW2A(m_bstrPassHash.m_str, CP_UTF8), strPasswordField);
		}

		std::string strTranslatorField;
		if (m_bMonitor || a_bUntranslatedStrings)
			GenerateFieldHeader(szMultipartBoundary, "translator", "1", strTranslatorField);

		std::string strTableHeader;
		GenerateFileHeader(szMultipartBoundary, "table", "tablefile", "application/octet-stream", strTableHeader);

		DWORD contentLength =
			strLangIDField.length() +
			strTimestampField.length() +
			strAppIDField.length() +
			strVersionField.length() +
			strEmailField.length() +
			strPasswordField.length() +
			strTranslatorField.length() +
			(cOut.empty() ? 0 : strTableHeader.length()+cOut.size()+2) +
			strlen(szBodyTrailer);

		INTERNET_BUFFERS buffersIn;
		memset(&buffersIn, 0, sizeof(INTERNET_BUFFERS));
		buffersIn.dwStructSize = sizeof(INTERNET_BUFFERS);
		buffersIn.dwBufferTotal = contentLength;

		bResult = HttpSendRequestEx(hRequest, &buffersIn, NULL, HSR_INITIATE, NULL);
		if (!bResult)
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		DWORD dwBytesWritten;

		if (strLangIDField.length() &&
			!InternetWriteFile(hRequest, strLangIDField.c_str(), strLangIDField.length(), &dwBytesWritten))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		if (strTimestampField.length() &&
			!InternetWriteFile(hRequest, strTimestampField.c_str(), strTimestampField.length(), &dwBytesWritten))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		if (strAppIDField.length() &&
			!InternetWriteFile(hRequest, strAppIDField.c_str(), strAppIDField.length(), &dwBytesWritten))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		if (strVersionField.length() &&
			!InternetWriteFile(hRequest, strVersionField.c_str(), strVersionField.length(), &dwBytesWritten))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		if (strEmailField.length() &&
			!InternetWriteFile(hRequest, strEmailField.c_str(), strEmailField.length(), &dwBytesWritten))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		if (strPasswordField.length() &&
			!InternetWriteFile(hRequest, strPasswordField.c_str(), strPasswordField.length(), &dwBytesWritten))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		if (strTranslatorField.length() &&
			!InternetWriteFile(hRequest, strTranslatorField.c_str(), strTranslatorField.length(), &dwBytesWritten))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		if (!cOut.empty())
		{
			if (!InternetWriteFile(hRequest, strTableHeader.c_str(), strTableHeader.length(), &dwBytesWritten) ||
				!InternetWriteFile(hRequest, &(cOut[0]), cOut.size(), &dwBytesWritten) ||
				!InternetWriteFile(hRequest, "\r\n", 2, &dwBytesWritten))
			{
				InternetCloseHandle(hRequest);
				InternetCloseHandle(hConnect);
				InternetCloseHandle(hSession);
				return E_FAIL;
			}
		}

		if (!InternetWriteFile(hRequest, szBodyTrailer, strlen(szBodyTrailer), &dwBytesWritten) ||
			!HttpEndRequest(hRequest, NULL, 0, NULL))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		DWORD dwStatusCodeSize = sizeof(DWORD);
		DWORD dwStatusCode;
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeSize, NULL);
		if (dwStatusCode != 200)
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hSession);
			return E_FAIL;
		}

		//DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
		//DWORD dwInfoBufferLength = 10;
		//BYTE *pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
		//while (!HttpQueryInfo(hRequest, dwInfoLevel, pInfoBuffer, &dwInfoBufferLength, NULL))
		//{
		//	DWORD dwError = GetLastError();
		//	if (dwError == ERROR_INSUFFICIENT_BUFFER)
		//	{
		//		free(pInfoBuffer);
		//		pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
		//	}
		//	else
		//	{
		//		TRACE("HttpQueryInfo failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		//		return HRESULT_FROM_WIN32(GetLastError());
		//	}
		//}

		//pInfoBuffer[dwInfoBufferLength] = '\0';
		//TRACE("%s", pInfoBuffer);
		//free(pInfoBuffer);

		CAutoVectorPtr<char> m_szResponse;
		DWORD m_nResponseLength = 0;
		DWORD dwBytesAvailable = 0;
		while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
		{
			char* p = new char[m_nResponseLength+dwBytesAvailable+1];
			CopyMemory(p, m_szResponse.m_p, m_nResponseLength);
			m_szResponse.Free();
			m_szResponse.Attach(p);
			p += m_nResponseLength;
			p[dwBytesAvailable] = '\0';
			m_nResponseLength += dwBytesAvailable;

			DWORD dwBytesRead;
			BOOL bResult = InternetReadFile(hRequest, p, dwBytesAvailable, &dwBytesRead);
			if (!bResult)
			{
				InternetCloseHandle(hRequest);
				InternetCloseHandle(hConnect);
				InternetCloseHandle(hSession);
				return E_FAIL;
			}

			if (dwBytesRead == 0)
			{
				break;	// End of File.
			}
		}

		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);

		STable sNewTbl;
		sNewTbl.nTimestamp = 0;
		sNewTbl.bChanged = false;
		ZeroMemory(sNewTbl.aIcPal, sizeof sNewTbl.aIcPal);
		ZeroMemory(sNewTbl.aIcon, sizeof sNewTbl.aIcon);
		ParsePOFile(m_szResponse.m_p, m_nResponseLength, sNewTbl);
		if (sNewTbl.IconValid())
		{
			CopyMemory(sTbl.aIcon, sNewTbl.aIcon, sizeof sTbl.aIcon);
			CopyMemory(sTbl.aIcPal, sNewTbl.aIcPal, sizeof sTbl.aIcPal);
			sTbl.bChanged = true;
		}
		for (CTable::const_iterator iN = sNewTbl.cTable.begin(); iN != sNewTbl.cTable.end(); ++iN)
		{
			STranslation& sTr = sTbl.cTable[iN->first];
			if (sTr.bstrTranslation == NULL || sTr.bstrTranslation[0] == L'\0')
			{
				if (iN->second.bstrTranslation && iN->second.bstrTranslation[0])
				{
					sTr.bstrTranslation = iN->second.bstrTranslation;
					sTr.bCustom = iN->second.bCustom;
					sTr.nUses = iN->second.nUses;
				}
				else if (sTr.nUses == 0)
				{
					sTr.nUses = 1;
				}
			}
			else
			{
				if (iN->second.bstrTranslation && iN->second.bstrTranslation[0])
				{
					if (sTr.bstrTranslation == iN->second.bstrTranslation)
					{
						sTr.bCustom = false;
						sTr.nUses = 0;
					}
					else if (!sTr.bCustom)
					{
						sTr.bstrTranslation = iN->second.bstrTranslation;
						sTr.nUses = 0;
					}
				}
			}
			sTbl.bChanged = true;
		}
		if (sNewTbl.nTimestamp == 0)
			return E_RW_ITEMNOTFOUND;
		if (sNewTbl.nTimestamp == sTbl.nTimestamp)
			return S_FALSE;
		sTbl.nTimestamp = sNewTbl.nTimestamp;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP CTranslator::Interactive(BYTE* a_pPriority)
{
	if (a_pPriority) *a_pPriority = 50; 
	return S_OK;
}

STDMETHODIMP CTranslator::Name(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Language[0405]Jazyk", this);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CTranslator::Description(ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		*a_ppDesc = new CMultiLanguageString(L"[0409]Select the preferred language. English will be used for strings unavailable in the selected language.[0405]Zvolte si preferovaný jazyk. Anglické texty budou použity v případě nedostupnosti překladu.", this);
		return S_OK;
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

#include "ConfigGUITranslator.h"

STDMETHODIMP CTranslator::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		(*a_ppConfig = this)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

HRESULT CTranslator::CreateConfig()
{
	RWCoCreateInstance(m_pConfig, __uuidof(ConfigWithDependencies));
	if (m_pConfig == NULL)
		return E_FAIL;
//	or use GetUserDefaultUILanguage() ?
	LCID tLCID = GetThreadLocale();
	LONG nLoc = (LANGIDFROMLCID(tLCID) == MAKELANGID(LANG_SLOVAK, SUBLANG_DEFAULT) || LANGIDFROMLCID(tLCID) == MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT)) ?
		MAKELCID(MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT), SORT_DEFAULT) : MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT);
	m_bstrCFGID = CFGID_LANGUAGECODE;
	m_pConfig->ItemInsSimple(m_bstrCFGID, CMultiLanguageString::GetAuto(L"[0409]Language[0405]Jazyk"), CMultiLanguageString::GetAuto(L"[0409]Languages marked with flags are installed. Select a language and click the Download button to attempt to download translation table.[0405]Jazyky označené vlajkou jsou instalovány. Vyberte jazyk a stiskněte tlačítko Stáhnout pro pokus o stažení překladových tabulek."), CConfigValue(nLoc), NULL, 0, NULL);

	CComBSTR cCFGID_TRANSLATIONMODE(CFGID_TRANSLATIONMODE);
	m_pConfig->ItemInsSimple(cCFGID_TRANSLATIONMODE, CMultiLanguageString::GetAuto(L"[0409]Translation mode[0405]Překladový režim"), CMultiLanguageString::GetAuto(L"[0409]If enabled, the application will gather information about untranslated strings and allow their translation.[0405]Je-li povoleno, aplikace bude shromažďovat informace o nepřeložených výrazech a umožní jejich překlad."), CConfigValue(false), NULL, 0, NULL);
	TConfigOptionCondition tCond;
	tCond.bstrID = cCFGID_TRANSLATIONMODE;
	tCond.eConditionType = ECOCEqual;
	tCond.tValue = CConfigValue(true);
	m_pConfig->ItemInsSimple(CComBSTR(CFGID_HIGHLIGHTUNTRANSLATED), CMultiLanguageString::GetAuto(L"[0409]Highlight untranslated[0405]Zvýraznit nepřeložené"), CMultiLanguageString::GetAuto(L"[0409]If enabled, untranslated strings will be preceded with [*] in the application window.[0405]Je-li povoleno, budou nepřeložené výrazy označeny v okně aplikace pomocí [*]."), CConfigValue(false), NULL, 1, &tCond);
	m_pConfig->ItemInsSimple(CComBSTR(CFGID_TRANSLATIONFILTER), CMultiLanguageString::GetAuto(L"[0409]Filter[0405]Filtr"), CMultiLanguageString::GetAuto(L"[0409]Only show strings containning the entered character sequence.[0405]Budou zobrazeny pouze výrazy obsahující zadanou sekvenci znaků."), CConfigValue(L""), NULL, 1, &tCond);
	m_pConfig->ItemInsSimple(CComBSTR(CFGID_HIDETRANSLATED), CMultiLanguageString::GetAuto(L"[0409]Hide translated[0405]Schovat přeložené"), CMultiLanguageString::GetAuto(L"[0409]Remove already translated strings from the list.[0405]Odstranit ž přeložené výrazy ze seznamu."), CConfigValue(true), NULL, 1, &tCond);

	CConfigCustomGUI<&CLSID_Translator, CConfigGUITranslatorDlg>::FinalizeConfig(m_pConfig);

	return S_OK;
}

void CTranslator::GetPathToPOFile(WORD a_wLangID, TCHAR* a_pszBuffer, ULONG a_nLength)
{
	if (m_szAppPath[0])
	{
		COLE2CT str(m_szAppPath);
		_sntprintf(a_pszBuffer, a_nLength, _T("%s%04x.po"), (LPCTSTR)str, a_wLangID);
		return;
	}
	GetModuleFileName(NULL, a_pszBuffer, a_nLength-8);
	a_pszBuffer[a_nLength-8] = _T('\\');
	_stprintf(_tcsrchr(a_pszBuffer, _T('\\'))+1, _T("%04x.po"), a_wLangID);
}

CTranslator::STable& CTranslator::GetTable(WORD a_wLangID)
{
	CLanguages::iterator iL = m_cLanguages.find(a_wLangID);
	if (iL != m_cLanguages.end())
		return iL->second;
	STable& sTbl = m_cLanguages[a_wLangID];
	sTbl.nTimestamp = 0;
	sTbl.bChanged = false;
	ZeroMemory(sTbl.aIcPal, sizeof sTbl.aIcPal);
	ZeroMemory(sTbl.aIcon, sizeof sTbl.aIcon);

	TCHAR szPath[MAX_PATH];
	GetPathToPOFile(a_wLangID, szPath, MAX_PATH);
	HANDLE hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		ULONG nFile = GetFileSize(hFile, NULL);
		if (nFile)
		{
			CAutoVectorPtr<BYTE> pFile(new BYTE[nFile]);
			DWORD dwRead = 0;
			ReadFile(hFile, pFile.m_p, nFile, &dwRead, NULL);
			ParsePOFile(reinterpret_cast<char const*>(pFile.m_p), dwRead, sTbl);
		}
		CloseHandle(hFile);
	}

	return sTbl;
}

LPWSTR ConvertFromUTF8(LPCSTR p, LPCSTR const pEnd, LPWSTR a_pDst)
{
	// utf8 to wchar_t
	size_t nInMultibyte = 0;
	wchar_t cMultibyte = L'\0';
	for (; *p && p != pEnd; ++p)
	{
		if (nInMultibyte == 0)
		{
			// start of new character)
			if (0x80 & *p)
			{
				// multibyte
				if ((0xe0 & *p) == 0xc0)
				{
					// 2-byte character
					nInMultibyte = 1;
					cMultibyte = 0x1f & *p;
				}
				else if ((0xf0 & *p) == 0xe0)
				{
					// 3-byte character
					nInMultibyte = 2;
					cMultibyte = 0x0f & *p;
				}
				else if ((0xf8 & *p) == 0xf0)
				{
					// 4-byte character
					nInMultibyte = 3;
					cMultibyte = 0x07 & *p;
				}
				else if ((0xfc & *p) == 0xf8)
				{
					// 5-byte character
					nInMultibyte = 4;
					cMultibyte = 0x03 & *p;
				}
				else if ((0xfe & *p) == 0xfc)
				{
					// 6-byte character
					nInMultibyte = 5;
					cMultibyte = 0x01 & *p;
				}
				else
					break; // invalid utf8 stream
			}
			else
			{
				// ANSI character
				*(a_pDst++) = *p;
			}
		}
		else
		{
			if ((0xc0 & *p) != 0x80)
				break; // invalid utf8 stream
			cMultibyte = (cMultibyte<<6) | (0x3f & *p);
			nInMultibyte--;
			if (nInMultibyte == 0)
			{
				*(a_pDst++) = cMultibyte;
			}
		}
	}
	return a_pDst;
}

void ParseString(LPCWSTR a_psz, LPCWSTR a_pszEnd, CComBSTR& a_bstrString)
{
	a_bstrString.Empty();
	while (a_psz != a_pszEnd && *a_psz != L'\"') ++a_psz;
	if (a_psz == a_pszEnd)
		return;
	LPCWSTR pszBeg = ++a_psz;
	ULONG nLen = 0;
	bool bSlash = false;
	while (a_psz != a_pszEnd)
	{
		if (bSlash)
		{
			++a_psz;
			++nLen;
			bSlash = false;
			continue;
		}
		if (*a_psz == L'\\')
		{
			bSlash = true;
			++a_psz;
			continue;
		}
		if (*a_psz == L'\"')
		{
			break;
		}
		++a_psz;
		++nLen;
	}
	if (nLen == 0)
		return;
	a_bstrString.AssignBSTR(::SysAllocStringLen(NULL, nLen));
	OLECHAR* pDst = a_bstrString.m_str;
	while (nLen)
	{
		if (*pszBeg == L'\\')
		{
			++pszBeg;
			switch (*pszBeg)
			{
			case L'n': *pDst = L'\n'; break;
			case L'r': *pDst = L'\r'; break;
			case L't': *pDst = L'\t'; break;
			case L'0': *pDst = L'\0'; break;
			default: *pDst = *pszBeg;
			}
		}
		else
		{
			*pDst = *pszBeg;
		}
		++pszBeg;
		--nLen;
		++pDst;
	}
}

void CTranslator::ParsePOFile(char const* a_pData, ULONG a_nLen, STable& a_cTbl)
{
	if (a_nLen >= 3 && a_pData[0] == char(0xef) && a_pData[1] == char(0xbb) && a_pData[2] == char(0xbf))
	{
		a_pData += 3;
		a_nLen -= 3;
	}
	if (a_nLen == 0)
		return;
	CAutoVectorPtr<wchar_t> szData(new wchar_t[a_nLen+1]);
	wchar_t* const pEnd = ConvertFromUTF8(a_pData, a_pData+a_nLen, szData);
	*pEnd = L'\0';
	wchar_t* p = szData;

	CComBSTR bstrID;
	bool bCustom = false;
	LONG nUses = 0;

	while (p != pEnd)
	{
		while (p != pEnd && (*p == L' ' || *p == L'\t' || *p == L'\n' || *p == L'\r'))
			++p;
		wchar_t* pLineEnd = p;
		while (pLineEnd != pEnd && *pLineEnd != L'\n' && *pLineEnd != L'\r')
			++pLineEnd;
		if (*p == L'#')
		{
			if (p[1] == L'^' && (p[2] == L' ' || p[2] == L'\t'))
			{
				nUses = 0;
				swscanf(p+3, L"%i", &nUses);
			}
			else if (p[1] == L'!')
			{
				bCustom = true;
			}
			else if (p[1] == L'i' && p[2] == L'c')
			{
				if (p[3] == L'p' && (p[4] == L' ' || p[4] == L'\t'))
				{
					int n = swscanf(p+4, L"%x %x %x %x %x %x %x %x %x %x", a_cTbl.aIcPal, a_cTbl.aIcPal+1, a_cTbl.aIcPal+2, a_cTbl.aIcPal+3, a_cTbl.aIcPal+4, a_cTbl.aIcPal+5, a_cTbl.aIcPal+6, a_cTbl.aIcPal+7, a_cTbl.aIcPal+8, a_cTbl.aIcPal+9);
					for (int i = 0; i < n; ++i)
						if ((a_cTbl.aIcPal[i]&0xff000000) == 0)
							a_cTbl.aIcPal[i] |= 0xff000000;
				}
				else if (((p[3] >= L'0' && p[3] <= L'9') || (p[3] >= L'a' && p[3] <= L'f')) && (p[4] == L' ' || p[4] == L'\t'))
				{
					int n = p[3] >= L'a' ? p[3]-L'a'+10 : p[3]-L'0';
					BYTE* pD = a_cTbl.aIcon+16*n;
					wchar_t const* pS = p+4;
					while (*pS == L' ' || *pS == L'\t') ++pS;
					for (int i = 0; i < 16 && (*pS == L'.' || (*pS >= L'0' && *pS <= L'9')); ++pS, ++i)
					{
						if (*pS != L'.')
							pD[i] = 1+*pS-L'0';
					}
				}
			}
			else if (p[1] == L's' && p[2] == L't' && p[3] == L'p' && (p[4] == L' ' || p[4] == L'\t'))
			{
				swscanf(p+5, L"%u", &a_cTbl.nTimestamp);
			}
		}
		else if (pLineEnd-p > 6)
		{
			if (wcsncmp(p, L"msgid", 5) == 0)
			{
				ParseString(p+5, pLineEnd, bstrID);
				if (nUses)
				{
					STranslation sVal;
					sVal.nUses = nUses;
					sVal.bCustom = false;
					a_cTbl.cTable[bstrID] = sVal;
				}
				nUses = 0;
			}
			else if (wcsncmp(p, L"msgstr", 6) == 0)
			{
				if (bstrID.Length())
				{
					STranslation sVal;
					sVal.nUses = 0;
					sVal.bCustom = bCustom;
					ParseString(p+6, pLineEnd, sVal.bstrTranslation);
					a_cTbl.cTable[bstrID] = sVal;
					bstrID.Empty();
					bCustom = false;
				}
			}
		}
		p = pLineEnd;
	}
}

void CTranslator::SerializePOFile(STable const& a_cTbl, std::vector<char>& a_cOut)
{
	static char const szMarker[] = {0xef, 0xbb, 0xbf};
	AddUTFString(szMarker, a_cOut);
	char szBuffer[256];

	// save timestamp
	sprintf(szBuffer, "#stp %u\r\n", a_cTbl.nTimestamp);
	AddUTFString(szBuffer, a_cOut);

	// save icon
	bool bExtraEOL = false;
	int n = 0;
	for (int i = 0; i < 10 && a_cTbl.aIcPal[i]; ++i)
	{
		if (i == 0)
		{
			szBuffer[n++] = '#';
			szBuffer[n++] = 'i';
			szBuffer[n++] = 'c';
			szBuffer[n++] = 'p';
			szBuffer[n++] = ' ';
		}
		else
		{
			szBuffer[n++] = ' ';
		}
		sprintf(szBuffer+n, "%08x", a_cTbl.aIcPal[i]);
		n += 8;
	}
	if (n)
	{
		szBuffer[n++] = '\r';
		szBuffer[n++] = '\n';
		szBuffer[n++] = '\0';
		AddUTFString(szBuffer, a_cOut);
		bExtraEOL = true;
	}
	szBuffer[0] = '#';
	szBuffer[1] = 'i';
	szBuffer[2] = 'c';
	szBuffer[4] = ' ';
	szBuffer[21] = '\r';
	szBuffer[22] = '\n';
	szBuffer[23] = '\0';
	for (int y = 0; y < 16; ++y)
	{
		szBuffer[3] = y < 10 ? '0'+y : 'a'+y-10;
		ULONG nSum = 0;
		for (int x = 0; x < 16; ++x)
		{
			BYTE b = a_cTbl.aIcon[(y<<4)+x];
			nSum += b;
			szBuffer[5+x] = b ? '0'+b-1 : '.';
		}
		if (nSum)
		{
			AddUTFString(szBuffer, a_cOut);
			bExtraEOL = true;
		}
	}
	if (bExtraEOL)
		AddUTFString(szBuffer+21, a_cOut);

	// save strings
	for (CTable::const_iterator i = a_cTbl.cTable.begin(); i != a_cTbl.cTable.end(); ++i)
	{
		if (i->second.bstrTranslation.Length())
		{
			if (i->second.bCustom)
				AddUTFString("#!\r\n", a_cOut);
			AddUTFString("msgid \"", a_cOut);
			AddEscapedString(i->first, a_cOut);
			AddUTFString("\"\r\nmsgstr \"", a_cOut);
			AddEscapedString(i->second.bstrTranslation, a_cOut);
			AddUTFString("\"\r\n\r\n", a_cOut);
		}
		else if (i->first)
		{
			char szCount[64];
			sprintf(szCount, "#^ %i\r\nmsgid \"", i->second.nUses);
			AddUTFString(szCount, a_cOut);
			AddEscapedString(i->first, a_cOut);
			AddUTFString("\"\r\n\r\n", a_cOut);
		}
	}
}

void CTranslator::SyncCachedValues()
{
	CConfigValue cVal;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSLATIONMODE), &cVal);
	m_bMonitor = cVal;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_HIGHLIGHTUNTRANSLATED), &cVal);
	m_bHilight = cVal;
}

bool CTranslator::STable::IconValid() const
{
	for (size_t i = 0; i < itemsof(aIcPal); ++i)
		if (aIcPal[i])
			return true;
	for (size_t i = 0; i < itemsof(aIcon); ++i)
		if (aIcon[i])
			return true;
	return false;
}

