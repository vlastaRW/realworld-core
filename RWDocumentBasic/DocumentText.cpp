// DocumentText.cpp : Implementation of CDocumentText

#include "stdafx.h"
#include "DocumentText.h"

#include <DocumentUndoImpl.h>


class CUndoStepReplace : public CDocumentUndoStep
{
public:
	CUndoStepReplace() : m_nNewCount(0), m_aLines(NULL)
	{
	}
	~CUndoStepReplace()
	{
		for (ULONG i = 0; i < m_nNewCount; ++i)
			SysFreeString(m_aLines[i]);
		delete[] m_aLines;
	};

	void Init(IDocumentText* a_pDoc, ULONG a_nIndex, ULONG a_nOldCount, ULONG a_nNewCount, BSTR* a_pLines)
	{
		m_pDoc = a_pDoc;
		m_nIndex = a_nIndex;
		m_nOldCount = a_nOldCount;
		m_nNewCount = a_nNewCount;
		m_aLines = a_pLines;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->LinesReplace(m_nIndex, m_nOldCount, m_nNewCount, m_aLines);
	}

private:
	CComPtr<IDocumentText> m_pDoc;
	ULONG m_nIndex;
	ULONG m_nOldCount;
	ULONG m_nNewCount;
	BSTR* m_aLines;
};

typedef CUndoStepImpl<CUndoStepReplace> CUndoReplace;



// CDocumentText

STDMETHODIMP CDocumentText::TextGet(BSTR* a_pbstrText)
{
	try
	{
		*a_pbstrText = NULL;

		CDocumentReadLock cLock(this);

		size_t nLen = 0;
		for (CLines::const_iterator i = m_cLines.begin(); i != m_cLines.end(); i++)
		{
			nLen += i->length() + 1; // 1 for endline character
		}
		if (nLen != 0)
			nLen--; // SysAllocStringLen allocates the terminating L'\0'

		BSTR bstr = SysAllocStringLen(NULL, static_cast<ULONG>(nLen));
		if (bstr == NULL)
			return E_FAIL;

		nLen = 0;
		for (CLines::const_iterator i = m_cLines.begin(); i != m_cLines.end(); i++)
		{
			memcpy(bstr+nLen, i->c_str(), i->length()*sizeof(wchar_t));
			nLen += i->length();
			bstr[nLen] = L'\n';
			nLen++;
		}
		bstr[nLen ? nLen-1 : 0] = L'\0';

		*a_pbstrText = bstr;
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrText == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentText::TextSet(BSTR a_bstrText)
{
	try
	{
		CDocumentWriteLock cLock(this);

		if (a_bstrText == NULL)
			return LinesReplace(0, m_cLines.size(), 0, NULL);

		CLines cLines;
		LPCWSTR pszLine = a_bstrText;
		LPCWSTR psz = a_bstrText;
		while (*psz)
		{
			if (L'\n' == *psz || L'\r' == *psz)
			{
				cLines.push_back(wstring(pszLine, psz));
				pszLine = psz = psz+(psz[1] == (*psz^L'\n'^L'\r') ? 2 : 1);
			}
			else
				++psz;
		}
		if (pszLine != psz)
		{
			cLines.push_back(wstring(pszLine, psz));
		}
		if (UndoEnabled() == S_OK)
		{
			CLines::const_iterator iOB = m_cLines.begin();
			CLines::const_iterator iNB = cLines.begin();
			while (iOB != m_cLines.end() && iNB != cLines.end() && *iOB == *iNB) { ++iOB; ++iNB; }
			ULONG nFirst = iOB-m_cLines.begin();
			ULONG nLast = 0;

			if (nFirst != m_cLines.size())
			{
				CLines::const_iterator iOE = m_cLines.begin()+(m_cLines.size()-1);
				CLines::const_iterator iNE = cLines.begin()+(cLines.size()-1);
				while (iOE != iOB && iNE != iNB && *iOE == *iNE) { --iOE; --iNE; }
				nLast = m_cLines.begin()+(m_cLines.size()-1)-iOE;
			}

			ULONG nOldCount = m_cLines.size()-nLast-nFirst;
			CAutoVectorPtr<BSTR> aLines;
			if (nOldCount)
			{
				aLines.Allocate(nOldCount);
				for (ULONG i = 0; i < nOldCount; ++i)
					aLines[i] = SysAllocString(m_cLines[nFirst+i].c_str());
			}
			CUndoReplace::Add(M_Base(), this, nFirst, nOldCount+cLines.size()-m_cLines.size(), nOldCount, aLines.m_p);
			aLines.Detach();
		}
		std::swap(m_cLines, cLines);
		m_bChanged = true;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentText::LinesGetCount(ULONG* a_pnLineCount)
{
	try
	{
		CDocumentReadLock cLock(this);

		*a_pnLineCount = static_cast<ULONG>(m_cLines.size());
		return S_OK;
	}
	catch (...)
	{
		return a_pnLineCount == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentText::LineGet(ULONG a_nIndex, BSTR* a_pbstrLine)
{
	try
	{
		*a_pbstrLine = NULL;

		CDocumentReadLock cLock(this);

		if (a_nIndex >= m_cLines.size())
			return E_RW_ITEMNOTFOUND;

		*a_pbstrLine = CComBSTR(m_cLines[a_nIndex].c_str()).Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrLine == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentText::LinesReplace(ULONG a_nIndex, ULONG a_nOldCount, ULONG a_nNewCount, BSTR* a_pbstrLines)
{
	try
	{
		CDocumentWriteLock cLock(this);

		if (a_nIndex+a_nOldCount > m_cLines.size())
			return E_RW_INDEXOUTOFRANGE;

		if (UndoEnabled() == S_OK)
		{
			CAutoVectorPtr<BSTR> aLines;
			if (a_nOldCount)
			{
				aLines.Allocate(a_nOldCount);
				for (ULONG i = 0; i < a_nOldCount; ++i)
					aLines[i] = SysAllocString(m_cLines[a_nIndex+i].c_str());
			}
			CUndoReplace::Add(M_Base(), this, a_nIndex, a_nNewCount, a_nOldCount, aLines.m_p);
			aLines.Detach();
		}
		if (a_nOldCount)
			m_cLines.erase(m_cLines.begin()+a_nIndex, m_cLines.begin()+a_nIndex+a_nOldCount);
		for (ULONG i = 0; i < a_nNewCount; ++i)
		{
			if (a_nIndex+i == m_cLines.size())
				m_cLines.push_back(a_pbstrLines[i] ? a_pbstrLines[i] : L"");
			else
				m_cLines.insert(m_cLines.begin()+a_nIndex+i, a_pbstrLines[i] ? a_pbstrLines[i] : L"");
		}

		m_bChanged = true;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentText::LineSet(ULONG a_nIndex, BSTR a_bstrLine)
{
	return LinesReplace(a_nIndex, 1, 1, &a_bstrLine);
}

STDMETHODIMP CDocumentText::LineIns(ULONG a_nIndex, BSTR a_bstrLine)
{
	return LinesReplace(a_nIndex, 0, 1, &a_bstrLine);
}

STDMETHODIMP CDocumentText::LineDel(ULONG a_nIndex)
{
	return LinesReplace(a_nIndex, 1, 0, NULL);
}

