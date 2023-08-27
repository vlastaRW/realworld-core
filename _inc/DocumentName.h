
#pragma once
#include <RWInput.h>


MIDL_INTERFACE("B640C9DA-1FF5-4D75-8FCF-EDA54614E495")
IDocumentName : public IUnknown
{
public:
	STDMETHOD(GetName)(BSTR* a_pName) = 0;
};

// CDocumentName

class ATL_NO_VTABLE CDocumentName : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStorageFilter,
	public IDocumentName
{
public:
	static bool IsValidFilter(IDocument* a_pDoc)
	{
		if (a_pDoc == NULL)
			return false;
		CComPtr<IStorageFilter> pFlt;
		a_pDoc->LocationGet(&pFlt);
		if (pFlt == NULL)
			return false;
		CComQIPtr<IDocumentName> pDN(pFlt);
		return pDN == NULL;
	}
	static void GetDocNameW(IStorageFilter* a_pSF, LPWSTR a_pszBuffer, size_t a_nBuffer)
	{
		if (a_pSF == NULL)
			return;
		CComQIPtr<IDocumentName> pDN(a_pSF);
		if (pDN != NULL)
		{
			CComBSTR bstrName;
			pDN->GetName(&bstrName);
			wcsncpy(a_pszBuffer, bstrName, a_nBuffer);
		}
		else
		{
			CComBSTR bstrFileName;
			a_pSF->ToText(NULL, &bstrFileName);
			if (bstrFileName == NULL)
			{
				a_pszBuffer = _T('\0');
			}
			else
			{
				LPCOLESTR pszTmp = bstrFileName;
				int nLen = wcslen(pszTmp);
				int i;
				for (i = nLen-1; i >= 0 && pszTmp[i] != L'\\' && pszTmp[i] != L'/'; i--) ;
				wcsncpy(a_pszBuffer, pszTmp+i+1, a_nBuffer);
			}
		}
	}

	static void GetDocName(IDocument* a_pDoc, LPTSTR a_pszBuffer, size_t a_nBuffer)
	{
		if (a_pDoc == NULL)
			return;
		CComPtr<IStorageFilter> pFlt;
		a_pDoc->LocationGet(&pFlt);
		if (pFlt == NULL)
			return;
		CComQIPtr<IDocumentName> pDN(pFlt);
		if (pDN != NULL)
		{
			CComBSTR bstrName;
			pDN->GetName(&bstrName);
			USES_CONVERSION;
			_tcsncpy(a_pszBuffer, OLE2CT(bstrName), a_nBuffer);
		}
		else
		{
			CComBSTR bstrFileName;
			pFlt->ToText(NULL, &bstrFileName);
			if (bstrFileName == NULL)
			{
				a_pszBuffer = _T('\0');
			}
			else
			{
				USES_CONVERSION;
				LPCOLESTR pszTmp = bstrFileName;
				int nLen = wcslen(pszTmp);
				int i;
				for (i = nLen-1; i >= 0 && pszTmp[i] != L'\\' && pszTmp[i] != L'/'; i--) ;
				_tcsncpy(a_pszBuffer, OLE2CT(pszTmp+i+1), a_nBuffer);
			}
		}
	}
	static bool ChangeExtension(IStorageFilter* a_pSource, LPCOLESTR a_pszExt, IStorageFilter** a_ppNameLoc)
	{
		try
		{
			*a_ppNameLoc = NULL;
			wchar_t szName[MAX_PATH+10] = L"";
			GetDocNameW(a_pSource, szName, itemsof(szName)-10);
			LPWSTR pszDot = szName;
			LPWSTR psz;
			for (psz = szName; *psz; psz++)
				if (*psz == L'.')
					pszDot = psz;
			if (pszDot == szName)
			{
				if (psz == szName)
				{
					// original has no name
					szName[0] = _T('_');
					pszDot++;
				}
				else
				{
					pszDot = psz;
				}
			}
			*pszDot = L'.';
			wcscpy(pszDot+1, a_pszExt);
			CComObject<CDocumentName>* pName = NULL;
			CComObject<CDocumentName>::CreateInstance(&pName);
			CComPtr<IStorageFilter> pTmp = pName;
			pName->Init(szName);
			*a_ppNameLoc = pTmp.Detach();
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
	template<class TDoc>
	static bool ChangeExtension(TDoc* a_pDoc, LPCOLESTR a_pszExt)
	{
		if (a_pDoc == NULL) return false;
		CComPtr<IStorageFilter> pOld;
		a_pDoc->LocationGet(&pOld);
		CComPtr<IStorageFilter> pNew;
		ChangeExtension(pOld, a_pszExt, &pNew);
		if (pNew == NULL) return false;
		return SUCCEEDED(a_pDoc->LocationSet(pNew));
	}
	static bool ChangeExtension(IDocument* a_pSrc, LPCOLESTR a_pszExt, IDocument* a_pDst)
	{
		CComPtr<IStorageFilter> pOld;
		a_pSrc->LocationGet(&pOld);
		CComPtr<IStorageFilter> pNew;
		ChangeExtension(pOld, a_pszExt, &pNew);
		if (pNew == NULL) return false;
		return SUCCEEDED(a_pDst->LocationSet(pNew));
	}

	void Init(wchar_t const* a_psz)
	{
		m_bstrName = a_psz;
	}

BEGIN_COM_MAP(CDocumentName)
	COM_INTERFACE_ENTRY(IStorageFilter)
	COM_INTERFACE_ENTRY(IDocumentName)
END_COM_MAP()


	// IStorageFilter
public:
	STDMETHOD(ToText)(IStorageFilter* UNREF(a_pRoot), BSTR* a_pbstrFilter)
	{
		*a_pbstrFilter = NULL;
		return E_NOTIMPL;
	}
	STDMETHOD(SubFilterGet)(BSTR UNREF(a_bstrRelativeLocation), IStorageFilter** a_ppFilter)
	{
		*a_ppFilter = NULL;
		return E_NOTIMPL;
	}
	STDMETHOD(SrcOpen)(IDataSrcDirect** a_ppSrc)
	{
		if (a_ppSrc) *a_ppSrc = NULL;
		return E_NOTIMPL;
	}
	STDMETHOD(DstOpen)(IDataDstStream** a_ppDst)
	{
		*a_ppDst = NULL;
		return E_NOTIMPL;
	}

	// IDocumentName methods
public:
	STDMETHOD(GetName)(BSTR* a_pName)
	{
		return m_bstrName.CopyTo(a_pName);
	}

private:
	CComBSTR m_bstrName;
};

