// StorageFilterFileSystem.h : Declaration of the CStorageFilterFileSystem

#pragma once
#include "RWStorage.h"


// CStorageFilterFileSystem

class ATL_NO_VTABLE CStorageFilterFileSystem : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStorageFilter,
	public IStorageLocatorAttrs,
	public IStorageFilterBrowsable
{
public:
	CStorageFilterFileSystem() : m_pSrcDirect(NULL), m_bWrite(false), m_pszLocation(NULL)
	{
	}
	~CStorageFilterFileSystem()
	{
		if (m_pszLocation) free(m_pszLocation);
		ATLASSERT(m_pSrcDirect == NULL);
		ATLASSERT(!m_bWrite);
	}


BEGIN_COM_MAP(CStorageFilterFileSystem)
	COM_INTERFACE_ENTRY(IStorageFilter)
	COM_INTERFACE_ENTRY(IStorageLocatorAttrs)
	COM_INTERFACE_ENTRY(IStorageFilterBrowsable)
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
	void DstFinished();

	// IStorageFilter methods
public:
	STDMETHOD(ToText)(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter);
	STDMETHOD(SubFilterGet)(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter);
	STDMETHOD(SrcOpen)(IDataSrcDirect** a_ppSrc);
	STDMETHOD(DstOpen)(IDataDstStream** a_ppDst);

	// IStorageLocatorAttrs methods
public:
	STDMETHOD(GetTime)(EStorageTimeType a_eType, ULONGLONG* a_pTime);
	STDMETHOD(SetTime)(EStorageTimeType a_eType, ULONGLONG a_nTime);

	// IStorageFilterBrowsable methods
public:
	STDMETHOD(OpenInFolder)();

private:
	class CSrcDirect :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDataSrcDirect
	{
	public:
		CSrcDirect() :
			m_pOwner(NULL), m_hFile(INVALID_HANDLE_VALUE), m_hFileMapping(NULL),
			m_pData(NULL), m_nDataSize(0)
		{
		}
		~CSrcDirect()
		{
			if (m_pData != NULL)
				UnmapViewOfFile(m_pData);
			if (m_hFileMapping != NULL)
				CloseHandle(m_hFileMapping);
			if (m_hFile != INVALID_HANDLE_VALUE)
				CloseHandle(m_hFile);
			if (m_pOwner)
			{
				m_pOwner->SrcFinished();
				m_pOwner->Release();
			}
		}

	BEGIN_COM_MAP(CSrcDirect)
		COM_INTERFACE_ENTRY(IDataSrcDirect)
	END_COM_MAP()

		HRESULT Init(CStorageFilterFileSystem* a_pOwner, LPCTSTR a_pszLocation);

		// IDataSrcDirect methods
	public:
		STDMETHOD(SizeGet)(ULONG* a_pnSize);
		STDMETHOD(SrcLock)(ULONG a_nOffset, ULONG a_nSize, BYTE const** a_ppBuffer);
		STDMETHOD(SrcUnlock)(ULONG a_nSize, BYTE const* a_pBuffer);

	private:
		CStorageFilterFileSystem* m_pOwner;
		HANDLE m_hFile;
		HANDLE m_hFileMapping;
		BYTE *m_pData;
		ULONG m_nDataSize;
	};

	class CDstStream :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDataDstStream
	{
	public:
		CDstStream() : m_hFile(INVALID_HANDLE_VALUE), m_pOwner(NULL), m_pszTemp(NULL), m_pszTarget(NULL)
		{
		}
		~CDstStream()
		{
			Close();
			if (m_pOwner)
			{
				m_pOwner->DstFinished();
				m_pOwner->Release();
			}
			delete[] m_pszTemp;
			delete[] m_pszTarget;
		}

	BEGIN_COM_MAP(CDstStream)
		COM_INTERFACE_ENTRY(IDataDstStream)
	END_COM_MAP()

		HRESULT Init(CStorageFilterFileSystem* a_pOwner, LPCTSTR a_pszLocation);

		// IDataDstStream methods
	public:
		STDMETHOD(SizeGet)(ULONG* a_pnSize);
		STDMETHOD(Write)(ULONG a_nSize, BYTE const* a_pBuffer);
		STDMETHOD(Seek)(ULONG a_nSize);
		STDMETHOD(Close)();

	private:
		CStorageFilterFileSystem* m_pOwner;
		HANDLE m_hFile;
		CComPtr<IFileOperation> m_pFO;
		LPTSTR m_pszTemp;
		LPTSTR m_pszTarget;
	};

private:
	CComObject<CSrcDirect>* m_pSrcDirect;
	bool m_bWrite;
	LPTSTR m_pszLocation;
};

