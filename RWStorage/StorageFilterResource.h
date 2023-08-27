// StorageFilterResource.h : Declaration of the CStorageFilterResource

#pragma once
#include "RWStorage.h"


// CStorageFilterResource

class ATL_NO_VTABLE CStorageFilterResource : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStorageFilter
{
public:
	CStorageFilterResource() : m_pSrcDirect(NULL)
	{
	}
	~CStorageFilterResource()
	{
		ATLASSERT(m_pSrcDirect == NULL);
	}


BEGIN_COM_MAP(CStorageFilterResource)
	COM_INTERFACE_ENTRY(IStorageFilter)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	bool Init(LPCTSTR a_pszFilter);
	void SrcFinished();

	// IStorageFilter methods
public:
	STDMETHOD(ToText)(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter);
	STDMETHOD(SubFilterGet)(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter);
	STDMETHOD(SrcOpen)(IDataSrcDirect** a_ppSrc);
	STDMETHOD(DstOpen)(IDataDstStream** a_ppDst);

private:
	class CSrcDirect :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDataSrcDirect
	{
	public:
		CSrcDirect() :
			m_pOwner(NULL), m_hModule(NULL), m_pData(NULL), m_nDataSize(0)
		{
		}
		~CSrcDirect()
		{
			if (m_hModule)
			{
				FreeLibrary(m_hModule);
			}
			if (m_pOwner)
			{
				m_pOwner->SrcFinished();
				m_pOwner->Release();
			}
		}

	BEGIN_COM_MAP(CSrcDirect)
		COM_INTERFACE_ENTRY(IDataSrcDirect)
	END_COM_MAP()

		HRESULT Init(CStorageFilterResource* a_pOwner, LPCTSTR a_pszLocation);

		// IDataSrcDirect methods
	public:
		STDMETHOD(SizeGet)(ULONG* a_pnSize);
		STDMETHOD(SrcLock)(ULONG a_nOffset, ULONG a_nSize, BYTE const** a_ppBuffer);
		STDMETHOD(SrcUnlock)(ULONG a_nSize, BYTE const* a_pBuffer);

	private:
		CStorageFilterResource* m_pOwner;
		HMODULE m_hModule;
		BYTE *m_pData;
		ULONG m_nDataSize;
	};

private:
	tstring m_strFilter;
	CComObject<CSrcDirect>* m_pSrcDirect;
};

