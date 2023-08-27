
#pragma once

#include <ConfigCustomGUIImpl.h>


template <class T, class TBase = CCustomConfigWndImpl<T> >
class ATL_NO_VTABLE CCustomConfigWndMultiLang :
	public TBase
{
public:
	CCustomConfigWndMultiLang(wchar_t const* a_pszID) : m_nIDs(1), m_pIDs(m_aIDs) { m_aIDs[0] = a_pszID; }
	CCustomConfigWndMultiLang(wchar_t const* a_pszID1, wchar_t const* a_pszID2) : m_nIDs(2), m_pIDs(m_aIDs) { m_aIDs[0] = a_pszID1; m_aIDs[1] = a_pszID2; }
	CCustomConfigWndMultiLang(ULONG a_nIDs, wchar_t const* a_pIDs) : m_nIDs(a_nIDs), m_pIDs(a_pIDs) {}

	bool ValueToText(WCHAR const* id, TConfigValue const& val, TCHAR* text, ULONG len)
	{
		for (ULONG j = 0; j < m_nIDs; ++j)
		{
			if (0 == wcscmp(id, m_pIDs[j]) && val.eTypeID == ECVTString)
			{
				LANGID const lang = LANGIDFROMLCID(static_cast<T*>(this)->m_tLocaleID);
				CMultiLangChunks chunks;
				ParseMLString(val.bstrVal, lang, chunks);
				CMultiLangChunks::const_iterator i = chunks.find(lang);
				if (i == chunks.end()) i = chunks.find(0x409);
				if (i == chunks.end()) i = chunks.find(0);
				if (i == chunks.end()) i = chunks.begin();
				if (i == chunks.end())
				{
					*text = _T('\0');
				}
				else
				{
					ULONG l = min(len-1, ULONG(i->second.second-i->second.first));
					wcsncpy(text, i->second.first, l);
					text[l] = _T('\0');
				}
				return true;
			}
		}
		return false;
	}
	bool TextToValue(WCHAR const* id, TCHAR const* text, CConfigValue const& old, CConfigValue& val)
	{
		for (ULONG j = 0; j < m_nIDs; ++j)
		{
			if (0 == wcscmp(id, m_pIDs[j]) && old.TypeGet() == ECVTString)
			{
				LANGID const lang = LANGIDFROMLCID(static_cast<T*>(this)->m_tLocaleID);
				BSTR str = NULL;
				MergeMLString(old, text, lang, &str);
				TConfigValue* p = &val;
				p->eTypeID = ECVTString;
				p->bstrVal = str;
				return true;
			}
		}
		return false;
	}

private:
	typedef std::map<LANGID, std::pair<wchar_t const*, wchar_t const*> > CMultiLangChunks;
	static inline WORD ParseDigit(wchar_t c)
	{
		if (c >= L'A' && c <= L'F')
			return c-(L'A'+10);
		if (c >= L'a' && c <= L'f')
			return c-(L'a'+10);
		return c-L'0';
	}
	static void ParseMLString(wchar_t const* psz, LANGID langid, CMultiLangChunks& chunks)
	{
		wchar_t const* start = psz;
		while (*psz)
		{
			if (*psz == L'[' &&
				((psz[1] >= L'0' && psz[1] <= L'9') || (psz[1] >= L'A' && psz[1] <= L'F') || (psz[1] >= L'a' && psz[1] <= L'f')) &&
				((psz[2] >= L'0' && psz[2] <= L'9') || (psz[2] >= L'A' && psz[2] <= L'F') || (psz[2] >= L'a' && psz[2] <= L'f')) &&
				((psz[3] >= L'0' && psz[3] <= L'9') || (psz[3] >= L'A' && psz[3] <= L'F') || (psz[3] >= L'a' && psz[3] <= L'f')) &&
				((psz[4] >= L'0' && psz[4] <= L'9') || (psz[4] >= L'A' && psz[4] <= L'F') || (psz[4] >= L'a' && psz[4] <= L'f')) &&
				psz[5] == L']')
			{
				if (start != psz)
					chunks[langid] = std::pair<wchar_t const*, wchar_t const*>(start, psz);
				langid = (ParseDigit(psz[1])<<12)|(ParseDigit(psz[2])<<8)|(ParseDigit(psz[3])<<4)|ParseDigit(psz[4]);
				psz += 6;
				start = psz;
			}
			else
			{
				++psz;
			}
		}
		if (start != psz)
			chunks[langid] = std::pair<wchar_t const*, wchar_t const*>(start, psz);
	}
	static void FormatMLString(wchar_t*& p, CMultiLangChunks::const_iterator i)
	{
		_swprintf(p, L"[%04x]", i->first);
		std::copy(i->second.first, i->second.second, p+6);
		p += 6+(i->second.second-i->second.first);
	}
	static void MergeMLString(wchar_t const* a_pszCurrent, wchar_t const* a_pszNew, LANGID a_langID, BSTR* a_pMerged)
	{
		CMultiLangChunks cur;
		ParseMLString(a_pszCurrent, a_langID, cur);
		CMultiLangChunks upd;
		ParseMLString(a_pszNew, a_langID, upd);
		for (CMultiLangChunks::const_iterator i = upd.begin(); i != upd.end(); ++i)
			cur[i->first] = i->second;
		size_t total = 0;
		for (CMultiLangChunks::const_iterator i = cur.begin(); i != cur.end(); ++i)
			total += 6+(i->second.second-i->second.first);
		*a_pMerged = SysAllocStringLen(NULL, total);
		wchar_t* p = *a_pMerged;
		CMultiLangChunks::const_iterator eng = cur.find(0x409);
		if (eng != cur.end())
		{
			FormatMLString(p, eng);
			cur.erase(eng);
		}
		for (CMultiLangChunks::const_iterator i = cur.begin(); i != cur.end(); ++i)
		{
			FormatMLString(p, i);
		}
		*p = L'\0';
	}

private:
	ULONG m_nIDs;
	wchar_t const** m_pIDs;
	wchar_t const* m_aIDs[2];
};
