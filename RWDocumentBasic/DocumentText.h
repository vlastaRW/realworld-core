// DocumentText.h : Declaration of the CDocumentText

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentBasic.h"
#include <SubjectImpl.h>


// CDocumentText

class ATL_NO_VTABLE CDocumentText : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDocumentDataImpl<CDocumentText, &CLSID_DocumentFactoryText, EUMMemoryLimited>,
	public CSubjectImpl<IDocumentText, ITextObserver, LONG>
{
public:
	CDocumentText() : m_bChanged(false)
	{
	}
	void InitLines(wchar_t const* a_begin, wchar_t const* a_end)
	{
		wchar_t const* pLine = a_begin;
		while (a_begin < a_end)
		{
			if (*a_begin == L'\r' || *a_begin == L'\n')
			{
				m_cLines.push_back(std::wstring(pLine, a_begin));
				if (a_begin+1 != a_end &&
					((a_begin[0] == L'\r' && a_begin[1] == L'\n') || (a_begin[0] == L'\n' && a_begin[1] == L'\r')))
					++a_begin;
				pLine = a_begin+1;
			}
			++a_begin;
		}
		m_cLines.push_back(std::wstring(pLine, a_end));
	}


BEGIN_COM_MAP(CDocumentText)
	COM_INTERFACE_ENTRY(IDocumentData)
	COM_INTERFACE_ENTRY(IDocumentText)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDocumentData mehods
public:
	STDMETHOD(WriteFinished)()
	{
		try
		{
			if (m_bChanged)
			{
				M_Base()->SetDirty();
				Fire_Notify(-1);
			}
			m_bChanged = false;
		}
		catch (...)
		{
		}
		return S_OK;
	}

	// IDocumentText methods
public:
	STDMETHOD(TextGet)(BSTR* a_pbstrText);
	STDMETHOD(TextSet)(BSTR a_bstrText);
	STDMETHOD(LinesGetCount)(ULONG* a_pnLineCount);
	STDMETHOD(LineGet)(ULONG a_nIndex, BSTR* a_pbstrLine);
	STDMETHOD(LinesReplace)(ULONG a_nIndex, ULONG a_nOldCount, ULONG a_nNewCount, BSTR* a_pbstrLines);
	STDMETHOD(LineSet)(ULONG a_nIndex, BSTR a_bstrLine);
	STDMETHOD(LineIns)(ULONG a_nIndex, BSTR a_bstrLine);
	STDMETHOD(LineDel)(ULONG a_nIndex);

private:
	typedef std::vector<std::wstring> CLines;

private:
	CLines m_cLines;
	bool m_bChanged;
};

