// ConfigInMemory.cpp : Implementation of CConfigInMemory

#include "stdafx.h"
#include "ConfigInMemory.h"


// CConfigInMemory

STDMETHODIMP CConfigInMemory::ItemIDsEnum(IEnumStrings** a_ppIDs)
{
	try
	{
		*a_ppIDs = NULL;

		CComPtr<IEnumStringsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
		ItemIDsEnum(L"", pTmp);

		*a_ppIDs = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppIDs == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::ItemValueGet(BSTR a_bstrID, TConfigValue* a_ptValue)
{
	try
	{
		ConfigValueInit(*a_ptValue);

		ObjectLock cLock(this);
		CValues::const_iterator i = m_cValues.find(a_bstrID);
		if (i == m_cValues.end())
			return E_RW_ITEMNOTFOUND; // TODO: error code

		*a_ptValue = ConfigValueCopy(i->second);
		return S_OK;
	}
	catch (...)
	{
		return a_ptValue == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::ItemValuesSet(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues)
{
	try
	{
		ObjectLock cLock(this);
		bool bChange = false;
		for (; a_nCount > 0; --a_nCount, ++a_aIDs, ++a_atValues)
		{
			if (bChange || m_cValues[*a_aIDs] != *a_atValues)
			{
				m_cValues[*a_aIDs] = *a_atValues;
				bChange = true;
			}
		}
		if (bChange)
			Fire_Notify(NULL);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::ItemGetUIInfo(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
{
	try
	{
		*a_ppItemInfo = NULL;
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CConfigInMemory::SubConfigGet(BSTR a_bstrID, IConfig** a_ppSubConfig)
{
	try
	{
		*a_ppSubConfig = NULL;

		CComObject<CSubConfigInMemory>* p = NULL;
		CComObject<CSubConfigInMemory>::CreateInstance(&p);
		CComPtr<IConfig> pTmp = p;
		p->Init(this, a_bstrID);

		*a_ppSubConfig = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	try
	{
		*a_ppCopiedConfig = NULL;

		CComObject<CConfigInMemory>* p = NULL;
		CComObject<CConfigInMemory>::CreateInstance(&p);
		CComPtr<IConfig> pTmp = static_cast<IConfigInMemory*>(p);

		ObjectLock cLock(this);
		p->m_cValues = m_cValues;

		*a_ppCopiedConfig = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppCopiedConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::CopyFrom(IConfig* a_pSource, BSTR a_bstrIDPrefix)
{
	if (a_bstrIDPrefix != NULL && a_bstrIDPrefix[0])
		return E_NOTIMPL;
	try
	{
		ObjectLock cLock(this);
		//m_cValues.clear();
		CComPtr<IEnumStrings> pIDs;
		a_pSource->ItemIDsEnum(&pIDs);
		ULONG nIDs = 0;
		if (pIDs) pIDs->Size(&nIDs);
		CValues cValues;
		for (ULONG i = 0; i < nIDs; ++i)
		{
			CComBSTR bstr;
			pIDs->Get(i, &bstr);
			a_pSource->ItemValueGet(bstr, &cValues[std::wstring(bstr)]);
		}
		if (cValues != m_cValues)
		{
			std::swap(m_cValues, cValues);
			Fire_Notify(NULL);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static char const CONFIGNINMEMORYHEADER_1[] = "ConfigInMemory_1.00";
static char const RWCONFIG_HEADER[] = "\xef\xbb\xbf<rwconfig>\r\n";
static char const RWCONFIG_FOOTER[] = "</rwconfig>\r\n";

STDMETHODIMP CConfigInMemory::DataBlockSet(ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		if (a_nSize >= sizeof(CONFIGNINMEMORYHEADER_1) && 0 == memcmp(CONFIGNINMEMORYHEADER_1, a_pData, sizeof(CONFIGNINMEMORYHEADER_1)))
		{
			a_pData += sizeof(CONFIGNINMEMORYHEADER_1);
			a_nSize -= sizeof(CONFIGNINMEMORYHEADER_1);
		}
		else if (a_nSize > 20 && (0 == memcmp(RWCONFIG_HEADER, a_pData, 13) || 0 == memcmp(RWCONFIG_HEADER+3, a_pData, 10)))
		{
			return ParseTextConfig(a_pData, a_pData+a_nSize);
		}
		else
		{
			//return E_FAIL;
		}
		CValues cValues;
		DWORD const* pSrc = reinterpret_cast<DWORD const*>(a_pData);
		DWORD const* pSrcEnd = pSrc + (a_nSize >>= 2);
		while (pSrc < pSrcEnd)
		{
			EConfigValueType eType = *reinterpret_cast<EConfigValueType const*>(pSrc);
			++pSrc;
			std::wstring strID(reinterpret_cast<wchar_t const*>(pSrc));
			pSrc += (((strID.length()+1)*sizeof(wchar_t)+3)>>2);
			if (pSrc > pSrcEnd)
				return E_FAIL;

			CConfigValue cVal;
			ULONG nDataSize = 0;
			switch(eType)
			{
			case ECVTInteger:
				cVal = *reinterpret_cast<LONG const*>(pSrc);
				nDataSize = sizeof(LONG);
				break;
			case ECVTFloat:
				cVal = *reinterpret_cast<float const*>(pSrc);
				nDataSize = sizeof(float);
				break;
			case ECVTBool:
				cVal = *reinterpret_cast<bool const*>(pSrc);
				nDataSize = sizeof(bool);
				break;
			case ECVTString:
				cVal = reinterpret_cast<LPCWSTR>(pSrc);;
				nDataSize = sizeof(wchar_t)*(wcslen(cVal)+1);
				break;
			case ECVTGUID:
				cVal = *reinterpret_cast<GUID const*>(pSrc);
				nDataSize = sizeof(GUID);
				break;
			case ECVTVector4:
				cVal = CConfigValue(eType, reinterpret_cast<float const*>(pSrc));
				nDataSize = sizeof(float)*4;
				break;
			case ECVTFloatColor:
			case ECVTVector3:
				cVal = CConfigValue(eType, reinterpret_cast<float const*>(pSrc));
				nDataSize = sizeof(float)*3;
				break;
			case ECVTVector2:
				cVal = CConfigValue(eType, reinterpret_cast<float const*>(pSrc));
				nDataSize = sizeof(float)*2;
				break;
			default:
				break;
			}
			pSrc += (nDataSize+3)>>2;
			if (pSrc > pSrcEnd)
				return E_FAIL;
			cValues[strID] = cVal;
		}

		ObjectLock cLock(this);
		std::swap(cValues, m_cValues);
		Fire_Notify(NULL);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::DataBlockGetData(IReturnedData* buffer)
{
	try
	{
		ObjectLock cLock(this);

		buffer->Write(sizeof(CONFIGNINMEMORYHEADER_1), reinterpret_cast<BYTE const*>(CONFIGNINMEMORYHEADER_1));
		for (CValues::const_iterator i = m_cValues.begin(); i != m_cValues.end(); ++i)
		{
			ULONG const type = i->second.TypeGet();
			buffer->Write(sizeof(type), reinterpret_cast<BYTE const*>(&type));

			buffer->Write(i->first.length()*sizeof(wchar_t), reinterpret_cast<BYTE const*>(i->first.c_str()));
			static BYTE const term[3] = {0, 0, 0}; // obsolete in C++11 (can +1 the above)
			buffer->Write(2, term);

			ULONG nDataSize = 0;
			switch (i->second.TypeGet())
			{
			case ECVTInteger:
				buffer->Write(nDataSize = sizeof(LONG), reinterpret_cast<BYTE const*>(&i->second.operator TConfigValue const&().iVal));
				break;
			case ECVTFloat:
				buffer->Write(nDataSize = sizeof(float), reinterpret_cast<BYTE const*>(&i->second.operator TConfigValue const&().fVal));
				break;
			case ECVTBool:
				buffer->Write(nDataSize = sizeof(bool), reinterpret_cast<BYTE const*>(&i->second.operator TConfigValue const&().bVal));
				break;
			case ECVTString:
				buffer->Write(nDataSize = sizeof(wchar_t)*(wcslen(i->second)+1), reinterpret_cast<BYTE const*>(i->second.operator BSTR()));
				break;
			case ECVTGUID:
				buffer->Write(nDataSize = sizeof(GUID), reinterpret_cast<BYTE const*>(&i->second.operator const GUID&()));
				break;
			case ECVTVector4:
				buffer->Write(nDataSize = sizeof(float)*4, reinterpret_cast<BYTE const*>(i->second.operator TConfigValue const&().vecVal));
				break;
			case ECVTFloatColor:
			case ECVTVector3:
				buffer->Write(nDataSize = sizeof(float)*3, reinterpret_cast<BYTE const*>(i->second.operator TConfigValue const&().vecVal));
				break;
			case ECVTVector2:
				buffer->Write(nDataSize = sizeof(float)*2, reinterpret_cast<BYTE const*>(i->second.operator TConfigValue const&().vecVal));
				break;
			default:
				break;
			}
			if (nDataSize&3)
				buffer->Write(4-(nDataSize&3), term);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::DataBlockGet(ULONG a_nSize, BYTE* a_pBuffer)
{
	try
	{
		ObjectLock cLock(this);
		ULONG nSize = 0;
		DataBlockGetSize(&nSize);
		if (a_nSize != nSize)
			return E_FAIL;

		memcpy(a_pBuffer, CONFIGNINMEMORYHEADER_1, sizeof(CONFIGNINMEMORYHEADER_1));
		DWORD* pDst = reinterpret_cast<DWORD*>(a_pBuffer+sizeof(CONFIGNINMEMORYHEADER_1));
		for (CValues::const_iterator i = m_cValues.begin(); i != m_cValues.end(); ++i)
		{
			*pDst = i->second.TypeGet();
			++pDst;

			CopyMemory(pDst, i->first.c_str(), i->first.length()*sizeof(wchar_t));
			reinterpret_cast<wchar_t*>(pDst)[i->first.length()] = L'\0';
			pDst += ((i->first.length()+1)*sizeof(wchar_t)+3)>>2;

			ULONG nDataSize = 0;
			switch (i->second.TypeGet())
			{
			case ECVTInteger:
				*reinterpret_cast<LONG*>(pDst) = i->second.operator LONG();
				nDataSize = sizeof(LONG);
				break;
			case ECVTFloat:
				*reinterpret_cast<float*>(pDst) = i->second.operator float();
				nDataSize = sizeof(float);
				break;
			case ECVTBool:
				*reinterpret_cast<bool*>(pDst) = i->second.operator bool();
				nDataSize = sizeof(bool);
				break;
			case ECVTString:
				if (i->second.operator BSTR())
				{
					CopyMemory(pDst, i->second.operator BSTR(), (wcslen(i->second)+1)*sizeof(wchar_t));
					nDataSize = sizeof(wchar_t)*(wcslen(i->second)+1);
				}
				else
				{
					*reinterpret_cast<wchar_t*>(pDst) = L'\0';
					nDataSize = sizeof(wchar_t);
				}
				break;
			case ECVTGUID:
				*reinterpret_cast<GUID*>(pDst) = i->second.operator const GUID&();
				nDataSize = sizeof(GUID);
				break;
			case ECVTVector4:
				nDataSize = sizeof(float)*4;
				CopyMemory(pDst, i->second.operator TConfigValue const&().vecVal, nDataSize);
				break;
			case ECVTFloatColor:
			case ECVTVector3:
				nDataSize = sizeof(float)*3;
				CopyMemory(pDst, i->second.operator TConfigValue const&().vecVal, nDataSize);
				break;
			case ECVTVector2:
				nDataSize = sizeof(float)*2;
				CopyMemory(pDst, i->second.operator TConfigValue const&().vecVal, nDataSize);
				break;
			default:
				break;
			}
			pDst += (nDataSize+3)>>2;
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::DataBlockGetSize(ULONG* a_pnSize)
{
	try
	{
		ObjectLock cLock(this);
		ULONG nRequired = sizeof(CONFIGNINMEMORYHEADER_1);
		for (CValues::const_iterator i = m_cValues.begin(); i != m_cValues.end(); ++i)
		{
			ULONG nDataSize = 0;
			switch (i->second.TypeGet())
			{
			case ECVTInteger:
				nDataSize = sizeof(LONG);
				break;
			case ECVTFloat:
				nDataSize = sizeof(float);
				break;
			case ECVTBool:
				nDataSize = sizeof(bool);
				break;
			case ECVTString:
				nDataSize = i->second.operator BSTR() ? sizeof(wchar_t)*(wcslen(i->second)+1) : 1;
				break;
			case ECVTGUID:
				nDataSize = sizeof(GUID);
				break;
			case ECVTVector4:
				nDataSize = sizeof(float)*4;
				break;
			case ECVTFloatColor:
			case ECVTVector3:
				nDataSize = sizeof(float)*3;
				break;
			case ECVTVector2:
				nDataSize = sizeof(float)*2;
				break;
			default:
				break;
			}
			nRequired += 4 + (((i->first.length()+1)*sizeof(wchar_t)+3)&(~3)) + ((nDataSize+3)&(~3));
		}
		*a_pnSize = nRequired;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::DeleteItems(BSTR a_bstrPrefix)
{
	try
	{
		ObjectLock cLock(this);
		ULONG nLen = ::SysStringLen(a_bstrPrefix);
		if (nLen == 0)
		{
			m_cValues.clear();
			return S_OK;
		}

		for (CValues::iterator i = m_cValues.begin(); i != m_cValues.end(); )
		{
			if (wcsncmp(i->first.c_str(), a_bstrPrefix, nLen) == 0)
			{
				CValues::iterator j = i;
				++i;
				m_cValues.erase(j);
			}
			else
			{
				++i;
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

// <rwconfig>
// <i n="Layouts" t="int" v="">
// <i n="" type="" v="" />
// </i>
// </rwconfig>

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
			if (*a_pString < L' ')
			{
				char sz[8] = "\\x00";
				if ((*a_pString) & 15 < 10)
					sz[4] = '0'+((*a_pString) & 15);
				else
					sz[4] = 'a'+((*a_pString) & 15)-10;
				if ((*a_pString) >= 16)
					sz[3] = '1';
				AddUTFString(sz, a_cOut);
			}
			else
				AddChar(*a_pString, a_cOut);
		}
		++a_pString;
	}
}

#include <StringParsing.h>

void AddConfigValue(TConfigValue const& val, std::vector<char>& a_cOut)
{
	switch (val.eTypeID)
	{
	case ECVTInteger:
		{
			char sz[64];
			sprintf(sz, " i=\"%i\"", val.iVal);
			AddUTFString(sz, a_cOut);
		}
		break;
	case ECVTFloat:
		{
			char sz[64];
			sprintf(sz, " f=\"%g\"", val.fVal);
			AddUTFString(sz, a_cOut);
		}
		break;
	case ECVTBool:
		AddUTFString(val.bVal ? " b=\"1\"" : " b=\"0\"", a_cOut);
		break;
	case ECVTString:
		AddUTFString(" t=\"", a_cOut);
		AddEscapedString(val.bstrVal, a_cOut);
		a_cOut.push_back('\"');
		break;
	case ECVTGUID:
		{
			char sz[128];
			StringFromGUID(val.guidVal, sz);
			AddUTFString(" g=\"", a_cOut);
			AddUTFString(sz, a_cOut);
			a_cOut.push_back('\"');
		}
		break;
	case ECVTVector2:
		{
			char sz[192];
			sprintf(sz, " v2=\"%g,%g\"", val.vecVal[0], val.vecVal[1]);
			AddUTFString(sz, a_cOut);
		}
		break;
	case ECVTVector3:
		{
			char sz[192];
			sprintf(sz, " v3=\"%g,%g,%g\"", val.vecVal[0], val.vecVal[1], val.vecVal[2]);
			AddUTFString(sz, a_cOut);
		}
		break;
	case ECVTVector4:
		{
			char sz[192];
			sprintf(sz, " v4=\"%g,%g,%g,%g\"", val.vecVal[0], val.vecVal[1], val.vecVal[2], val.vecVal[3]);
			AddUTFString(sz, a_cOut);
		}
		break;
	case ECVTFloatColor:
		{
			char sz[192];
			sprintf(sz, " c=\"%g,%g,%g\"", val.vecVal[0], val.vecVal[1], val.vecVal[2]);
			AddUTFString(sz, a_cOut);
		}
		break;
	//case ECVTEmpty:
	default:
		break;
	}
}
void AddTabs(int n, std::vector<char>& dst)
{
	while (n--)
		dst.push_back(' ');
}

void CConfigInMemory::ItemToText(std::wstring const& prefix, CValues::const_iterator& i, CValues::const_iterator const e, int tabs, std::vector<char>& dst)
{
	AddTabs(tabs, dst);
	AddUTFString("<i n=\"", dst);
	AddEscapedString(i->first.substr(prefix.length()).c_str(), dst);
	dst.push_back('\"');
	AddConfigValue(i->second, dst);
	CValues::const_iterator j = i;
	++j;
	std::wstring p2 = i->first;
	p2.append(L"\\");
	if (j != e && p2.length() < j->first.length() && j->first.compare(0, p2.length(), p2) == 0)
	{
		// has subitem(s)
		AddUTFString(">\r\n", dst);
		do
		{
			ItemToText(p2, j, e, tabs+1, dst);
		}
		while (j != e && p2.length() < j->first.length() && j->first.compare(0, p2.length(), p2) == 0);
		AddTabs(tabs, dst);
		AddUTFString("</i>\r\n", dst);
	}
	else
	{
		AddUTFString(" />\r\n", dst);
	}
	i = j;
}

STDMETHODIMP CConfigInMemory::TextBlockGet(IReturnedData* buffer)
{
	try
	{
		if (buffer == NULL) return E_POINTER;
		ObjectLock lock(this);
		std::vector<char> buf;
		AddUTFString(RWCONFIG_HEADER, buf);
		CValues::const_iterator i = m_cValues.begin();
		CValues::const_iterator const e = m_cValues.end();
		std::wstring none;
		while (i != e)
		{
			ItemToText(none, i, e, 1, buf);
		}
		AddUTFString(RWCONFIG_FOOTER, buf);
		return buffer->Write(buf.size(), reinterpret_cast<BYTE const*>(&(buf[0])));
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static bool SkipWS(BYTE const*& b, BYTE const* const e)
{
	while (b < e && (*b == ' ' || *b == '\t' || *b == '\r' || *b == '\n')) ++b;
	return b < e;
}

static bool Match(BYTE const*& b, BYTE const* const e, char const* psz)
{
	while (b < e && *psz && *b == *psz) {++b; ++psz;}
	return *psz == '\0';
}

static bool ReadUTF8Char(BYTE const*& p, BYTE const* const e, wchar_t& val)
{
	val = L'\0';
	size_t nInMultibyte = 0;
	for (; p != e; ++p)
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
					val = 0x1f & *p;
				}
				else if ((0xf0 & *p) == 0xe0)
				{
					// 3-byte character
					nInMultibyte = 2;
					val = 0x0f & *p;
				}
				else if ((0xf8 & *p) == 0xf0)
				{
					// 4-byte character
					nInMultibyte = 3;
					val = 0x07 & *p;
				}
				else if ((0xfc & *p) == 0xf8)
				{
					// 5-byte character
					nInMultibyte = 4;
					val = 0x03 & *p;
				}
				else if ((0xfe & *p) == 0xfc)
				{
					// 6-byte character
					nInMultibyte = 5;
					val = 0x01 & *p;
				}
				else
					return false; // invalid utf8 stream
			}
			else
			{
				// ANSI character
				val = *p;
				++p;
				return true;
			}
		}
		else
		{
			if ((0xc0 & *p) != 0x80)
				break; // invalid utf8 stream
			val = (val<<6) | (0x3f & *p);
			nInMultibyte--;
			if (nInMultibyte == 0)
			{
				++p;
				return true;
			}
		}
	}
	return false;
}

static bool ParseValue(BYTE const*& b, BYTE const* const e, std::wstring& val)
{
	if (b == e) return false;

	if (*b != '\"') return false;
	++b;

	while (b < e)
	{
		if (*b == '\"')
		{
			++b;
			return true;
		}
		if (*b == '\\')
		{
			++b;
			if (b == e) return false;
			switch (*b)
			{
			case 'r':	val.push_back(L'\r'); ++b; break;
			case 'n':	val.push_back(L'\n'); ++b; break;
			case 't':	val.push_back(L'\t'); ++b; break;
			case '\"':	val.push_back(L'\"'); ++b; break;
			case '\\':	val.push_back(L'\\'); ++b; break;
			case 'x':
				++b;
				if ((b+2) >= e) return false;
				{
					wchar_t v = 0;
					if (b[0] == '1') v = 0x10;
					else if (b[0] != '0') return false;
					if (b[1] >= '0' && b[1] <= '9') v += b[1]-'0';
					else if (b[1] >= 'a' && b[1] <= 'f') v += b[1]-'a'+10;
					else if (b[1] >= 'A' && b[1] <= 'F') v += b[1]-'A'+10;
					else return false;
					b += 2;
					val.push_back(v);
				}
				break;
			default:
				return false;
			}
			continue;
		}
		wchar_t v = L'\0';
		if (!ReadUTF8Char(b, e, v))
			return false;
		val.push_back(v);
	}
	return false;
}

bool CConfigInMemory::ParseItem(BYTE const*& b, BYTE const* const e, wchar_t const* const prefix, int const pflen, CValues& vals)
{
	if (!SkipWS(b, e)) return false;

	{
		BYTE const* c = b;
		if (!Match(b, e, "<i"))
		{
			b = c;
			return false;
		}
	}

	if (!SkipWS(b, e)) return false;

	if (*b != 'n') return false;
	++b;

	if (!SkipWS(b, e)) return false;

	if (*b != '=') return false;
	++b;

	if (!SkipWS(b, e)) return false;

	std::wstring name;
	if (!ParseValue(b, e, name)) return false;

	if (!SkipWS(b, e)) return false;

	TConfigValue t;
	CComBSTR bstr;
	t.eTypeID = ECVTEmpty;
	switch (*b)
	{
	case 'i':
		t.eTypeID = ECVTInteger;
		break;
	case 'f':
		t.eTypeID = ECVTFloat;
		break;
	case 'b':
		t.eTypeID = ECVTBool;
		break;
	case 't':
		t.eTypeID = ECVTString;
		break;
	case 'g':
		t.eTypeID = ECVTGUID;
		break;
	case 'v':
		++b;
		if (b == e) return false;
		if (*b == '2')
			t.eTypeID = ECVTVector2;
		else if (*b == '3')
			t.eTypeID = ECVTVector3;
		else if (*b == '4')
			t.eTypeID = ECVTVector4;
		else
			return false;
		break;
	case 'c':
		t.eTypeID = ECVTFloatColor;
		break;
	case '/':
		break;
	case '>':
		break;
	default:
		return false;
	}
	if (t.eTypeID != ECVTEmpty)
	{
		++b;
		if (!SkipWS(b, e)) return false;
		if (*b != '=') return false;
		++b;
		if (!SkipWS(b, e)) return false;
		std::wstring val;
		if (!ParseValue(b, e, val)) return false;
		if (!SkipWS(b, e)) return false;
		switch (t.eTypeID)
		{
		case ECVTInteger:
			if (swscanf(val.c_str(), L"%i", &t.iVal) != 1) return false;
			break;
		case ECVTFloat:
			if (swscanf(val.c_str(), L"%f", &t.fVal) != 1) return false;
			break;
		case ECVTBool:
			if (val == L"1" || val == L"true")
				t.bVal = 1;
			else if (val == L"0" || val == L"false")
				t.bVal = 0;
			else
				return false;
			break;
		case ECVTString:
			bstr = val.c_str();
			t.bstrVal = bstr;
			break;
		case ECVTGUID:
			if (!GUIDFromString(val.c_str(), &t.guidVal)) return false;
			break;
		case ECVTVector2:
			if (swscanf(val.c_str(), L"%f,%f", t.vecVal, t.vecVal+1) != 2) return false;
			break;
		case ECVTVector3:
		case ECVTFloatColor:
			if (swscanf(val.c_str(), L"%f,%f,%f", t.vecVal, t.vecVal+1, t.vecVal+2) != 3) return false;
			break;
		case ECVTVector4:
			if (swscanf(val.c_str(), L"%f,%f,%f,%f", t.vecVal, t.vecVal+1, t.vecVal+2, t.vecVal+3) != 4) return false;
			break;
		}
	}
	std::wstring strID(prefix, prefix+pflen);
	strID.append(name);
	vals[strID] = t;
	if (*b == '>')
	{
		++b;
		strID.push_back(L'\\');
		while (ParseItem(b, e, strID.c_str(), strID.length(), vals)) ;
		if (!SkipWS(b, e)) return false;
		return Match(b, e, "</i>");
	}
	else if (*b == '/')
	{
		++b;
		if (b == e) return false;
		if (*b == '>')
		{
			++b;
			return true;
		}
		++b;
		return false;
	}

	return false;
}

HRESULT CConfigInMemory::ParseTextConfig(BYTE const* b, BYTE const* const e)
{
	if (b[0] == 0xef && b[1] == 0xbb && b[2] == 0xbf)
		b += 3;
	if (!SkipWS(b, e)) return E_FAIL;
	if (!Match(b, e, "<rwconfig>")) return E_FAIL;

	CValues vals;
	while (ParseItem(b, e, L"", 0, vals)) ;

	if (!SkipWS(b, e)) return E_FAIL;
	if (!Match(b, e, "</rwconfig>")) return E_FAIL;

	ObjectLock cLock(this);
	std::swap(vals, m_cValues);
	Fire_Notify(NULL);

	return S_OK;
}


void CConfigInMemory::ItemIDsEnum(LPCWSTR a_pszPrefix, IEnumStringsInit* a_pTmp)
{
	ObjectLock cLock(this);
	for (CValues::const_iterator i = m_cValues.begin(); i != m_cValues.end(); ++i)
	{
		LPCWSTR psz = i->first.c_str();
		ULONG ii = 0;
		while (psz[ii] == a_pszPrefix[ii] && a_pszPrefix[ii] != L'\0')
			++ii;

		if (a_pszPrefix[ii] == L'\0' && psz[ii] != L'\0')
			a_pTmp->Insert(CComBSTR(psz[ii] == L'\\' ? (psz+ii+1) : (psz+ii)));
	}
}


STDMETHODIMP CConfigInMemory::CSubConfigInMemory::ItemIDsEnum(IEnumStrings** a_ppIDs)
{
	try
	{
		*a_ppIDs = NULL;

		CComPtr<IEnumStringsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
		m_pParent->ItemIDsEnum(m_bstrPrefix, pTmp);

		*a_ppIDs = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppIDs == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::CSubConfigInMemory::ItemValueGet(BSTR a_bstrID, TConfigValue* a_ptValue)
{
	try
	{
		return m_pParent->ItemValueGet(CComBSTR((std::wstring(m_bstrPrefix)+L"\\"+a_bstrID).c_str()), a_ptValue);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::CSubConfigInMemory::ItemValuesSet(ULONG a_nCount, BSTR* a_aIDs, const TConfigValue* a_atValues)
{
	try
	{
		if (a_nCount == 0)
			return S_OK;

		std::vector<CComBSTR> aIDs;
		for (ULONG i = 0; i < a_nCount; ++i)
		{
			aIDs.push_back(CComBSTR((std::wstring(m_bstrPrefix)+L"\\"+a_aIDs[i]).c_str()));
		}
		CAutoVectorPtr<BSTR> aIDs2(new BSTR[a_nCount]);
		std::copy(aIDs.begin(), aIDs.end(), aIDs2.m_p);
		return m_pParent->ItemValuesSet(a_nCount, aIDs2, a_atValues);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::CSubConfigInMemory::ItemGetUIInfo(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
{
	try
	{
		*a_ppItemInfo = NULL;
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CConfigInMemory::CSubConfigInMemory::SubConfigGet(BSTR a_bstrID, IConfig** a_ppSubConfig)
{
	try
	{
		*a_ppSubConfig = NULL;

		CComObject<CSubConfigInMemory>* p = NULL;
		CComObject<CSubConfigInMemory>::CreateInstance(&p);
		CComPtr<IConfig> pTmp = p;
		p->Init(m_pParent, CComBSTR((std::wstring(m_bstrPrefix)+L"\\"+a_bstrID).c_str()));

		*a_ppSubConfig = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::CSubConfigInMemory::DuplicateCreate(IConfig** a_ppCopiedConfig)
{
	try
	{
		*a_ppCopiedConfig = NULL;

		CComPtr<IConfig> pParent;
		if (FAILED(m_pParent->DuplicateCreate(&pParent)))
			return E_FAIL;

		CComObject<CSubConfigInMemory>* p = NULL;
		CComObject<CSubConfigInMemory>::CreateInstance(&p);
		CComPtr<IConfig> pTmp = p;
		p->Init(static_cast<CConfigInMemory*>(static_cast<IConfigInMemory*>(pParent.p)), m_bstrPrefix);

		*a_ppCopiedConfig = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppCopiedConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CConfigInMemory::CSubConfigInMemory::ObserverIns(IConfigObserver* a_pObserver, TCookie a_tCookie)
{
	return E_NOTIMPL;
}

STDMETHODIMP CConfigInMemory::CSubConfigInMemory::ObserverDel(IConfigObserver* a_pObserver, TCookie a_tCookie)
{
	return E_NOTIMPL;
}

