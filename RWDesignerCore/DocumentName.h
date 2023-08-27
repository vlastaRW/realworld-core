
#pragma once
#include "RWDesignerCore.h"


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

	void Init(wstring const& a_str)
	{
		m_strName = a_str;
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
		*a_ppSrc = NULL;
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
		*a_pName = SysAllocString(m_strName.c_str());
		return S_OK;
	}

private:
	wstring m_strName;
};

