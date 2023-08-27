// StorageSQLite.h : Declaration of the CStorageSQLite

#pragma once
#include "resource.h"       // main symbols
#include "RWStorageSQLite.h"


#include "sqlite3.h"
struct CSQLiteWrapper
{
	CSQLiteWrapper(wchar_t const* a_pszFile) : pDb(NULL), resCode(SQLITE_ERROR)
	{
		Open(a_pszFile);
	}
	~CSQLiteWrapper()
	{
		if (pDb) sqlite3_close(pDb);
	}
	void Open(wchar_t const* a_pszFile)
	{
		if (pDb)
		{
			sqlite3_close(pDb);
			pDb = NULL;
		}
		resCode = SQLITE_ERROR;
		if (a_pszFile)
		{
			resCode = sqlite3_open16(a_pszFile, &pDb);
			sqlite3_busy_timeout(pDb, 10000);
			if (resCode == SQLITE_OK)
			{
				// create database (if not exists)
				resCode = sqlite3_exec(pDb, "\
CREATE TABLE IF NOT EXISTS files\
(\
	id INTEGER PRIMARY KEY AUTOINCREMENT,\
	uid TEXT UNIQUE,\
	name TEXT,\
	created INTEGER,\
	modified INTEGER,\
	accessed INTEGER,\
	data BLOB,\
	size INTEGER,\
	note TEXT\
);\
CREATE INDEX IF NOT EXISTS files_i1 on files(name);\
CREATE INDEX IF NOT EXISTS files_i2 on files(uid,name);\
CREATE TABLE IF NOT EXISTS tags\
(\
	id INTEGER PRIMARY KEY AUTOINCREMENT,\
	name TEXT,\
	fid INTEGER\
);\
CREATE INDEX IF NOT EXISTS tags_i1 on tags(name);\
CREATE INDEX IF NOT EXISTS tags_i2 on tags(fid);\
CREATE TABLE IF NOT EXISTS meta\
(\
	id INTEGER PRIMARY KEY AUTOINCREMENT,\
	key TEXT,\
	val TEXT\
);\
",
NULL, NULL, NULL);

//CREATE TABLE IF NOT EXISTS map\
//(\
//	id INTEGER PRIMARY KEY AUTOINCREMENT,\
//	tid INTEGER,\
//	fid INTEGER\
//);
			}
		}
	}
	void Swap(CSQLiteWrapper& a_rhs)
	{
		std::swap(resCode, a_rhs.resCode);
		std::swap(pDb, a_rhs.pDb);
	}
	int ResCode() const { return resCode; }
	operator sqlite3*() const { return pDb; }

private:
	int resCode;
	sqlite3* pDb;
};

class CSQLiteStatement
{
public:
	CSQLiteStatement() : pStmt(NULL), resCode(SQLITE_ERROR)
	{
	}
	CSQLiteStatement(sqlite3* a_pDb, char const* a_pszQuery) : pStmt(NULL), resCode(SQLITE_ERROR)
	{
		char const* pTail = NULL;
		resCode = sqlite3_prepare(a_pDb, a_pszQuery, -1, &pStmt, &pTail);
	}
	CSQLiteStatement(sqlite3* a_pDb, wchar_t const* a_pszQuery) : pStmt(NULL), resCode(SQLITE_ERROR)
	{
		void const* pTail = NULL;
		resCode = sqlite3_prepare16(a_pDb, a_pszQuery, -1, &pStmt, &pTail);
	}
	~CSQLiteStatement()
	{
		if (pStmt)
			sqlite3_finalize(pStmt);
	}

	int ResCode() const { return resCode; }
	operator sqlite3_stmt*() const { return pStmt; }

private:
	int resCode;
	sqlite3_stmt* pStmt;
};

template<typename TIterator, class TOutChar>
inline void Base64Encode(TIterator a_begin, TIterator a_end, TOutChar* a_pOut)
{
	static char const s_tBase64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
    size_t i;
    TOutChar c;
    size_t len = a_end-a_begin;

    for (i = 0; i < len; ++i)
    {
        c = (*(a_begin+i) >> 2) & 0x3f;
        *(a_pOut++) = s_tBase64Encode[c];
        c = (*(a_begin+i) << 4) & 0x3f;
        if (++i < len)
            c |= (*(a_begin+i) >> 4) & 0x0f;

        *(a_pOut++) = s_tBase64Encode[c];
        if (i < len)
        {
            c = (*(a_begin+i) << 2) & 0x3f;
            if (++i < len)
                c |= (*(a_begin+i) >> 6) & 0x03;

            *(a_pOut++) = s_tBase64Encode[c];
        }
        else
        {
            ++i;
			return; // this is a special version without fill chars
            //*(a_pOut++) = '='; // fillchar
        }

        if (i < len)
        {
            c = *(a_begin+i) & 0x3f;
            *(a_pOut++) = s_tBase64Encode[c];
        }
        else
        {
			return; // this is a special version without fill chars
            //*(a_pOut++) = '='; // fillchar
        }
    }
}


// CStorageSQLite

class ATL_NO_VTABLE CStorageSQLite :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStorageFilter,
	public IStorageLocatorAttrs
{
public:
	typedef std::vector<std::wstring> CTags;

	CStorageSQLite()
	{
		m_nUID = -1LL;
	}
	bool Init(OLECHAR const* a_bstrFilter);
	void Init(OLECHAR const* a_pszDatabase, OLECHAR const* a_pszFileName, OLECHAR const* a_pszGUID, CTags const& a_cTags)
	{
		m_szDatabase = a_pszDatabase;
		m_szFileName = a_pszFileName;
		wcsncpy(m_szGUID, a_pszGUID, 23);
		m_cTags = a_cTags;
		m_bUIDValid = false;
		m_nUID = -1LL;
		std::sort(m_cTags.begin(), m_cTags.end());
	}
	LONGLONG M_UID()
	{
		CSQLiteWrapper cDb(m_szDatabase.c_str());
		ValidateUID(cDb);
		return m_nUID;
	}

	static void GetDefaultDatabase(std::wstring& a_strBuffer);


BEGIN_COM_MAP(CStorageSQLite)
	COM_INTERFACE_ENTRY(IStorageFilter)
	COM_INTERFACE_ENTRY(IStorageLocatorAttrs)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IStorageFilter methods
public:
	STDMETHOD(ToText)(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter);
	STDMETHOD(SubFilterGet)(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter);
	STDMETHOD(SrcOpen)(IDataSrcDirect** a_ppSrc);
	STDMETHOD(DstOpen)(IDataDstStream** a_ppDst);

	// helpers
	HRESULT WriteData(BYTE const* a_pData, ULONG a_nData);

	// IStorageLocatorAttrs methods
public:
	STDMETHOD(GetTime)(EStorageTimeType a_eType, ULONGLONG* a_pTime);
	STDMETHOD(SetTime)(EStorageTimeType a_eType, ULONGLONG a_nTime);

private:
	class CSrcDirect :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDataSrcDirect
	{
	public:
		CSrcDirect() : m_nData(0)
		{
		}
		void Init(ULONG a_nData, BYTE* a_pData)
		{
			m_nData = a_nData;
			m_pData.Attach(a_pData);
		}

	BEGIN_COM_MAP(CSrcDirect)
		COM_INTERFACE_ENTRY(IDataSrcDirect)
	END_COM_MAP()

		// IDataSrcDirect methods
	public:
		STDMETHOD(SizeGet)(ULONG* a_pnSize);
		STDMETHOD(SrcLock)(ULONG a_nOffset, ULONG a_nSize, BYTE const** a_ppBuffer);
		STDMETHOD(SrcUnlock)(ULONG a_nSize, BYTE const* a_pBuffer);

	private:
		ULONG m_nData;
		CAutoVectorPtr<BYTE> m_pData;
	};

	class CDstStream :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDataDstStream
	{
	public:
		CDstStream() : m_nPos(0), m_nWritten(0), m_nAlloc(0), m_pOwner(NULL)
		{
		}
		~CDstStream()
		{
			Close();
		}

	BEGIN_COM_MAP(CDstStream)
		COM_INTERFACE_ENTRY(IDataDstStream)
	END_COM_MAP()

		void Init(CStorageSQLite* a_pOwner)
		{
			(m_pOwner = a_pOwner)->AddRef();
		}

		// IDataDstStream methods
	public:
		STDMETHOD(SizeGet)(ULONG* a_pnSize);
		STDMETHOD(Write)(ULONG a_nSize, BYTE const* a_pBuffer);
		STDMETHOD(Seek)(ULONG a_nSize);
		STDMETHOD(Close)();

	private:
		void EnsureSize(ULONG a_nSize);

	private:
		CStorageSQLite* m_pOwner;
		ULONG m_nPos;
		CAutoVectorPtr<BYTE> m_pData;
		ULONG m_nWritten;
		ULONG m_nAlloc;
	};

private:
	void ValidateUID(sqlite3* a_pDb);

private:
	std::wstring m_szDatabase;
	std::wstring m_szFileName;
	OLECHAR m_szGUID[23];
	CTags m_cTags;
	bool m_bUIDValid;
	LONGLONG m_nUID;
};

extern __declspec(selectany) OLECHAR const CFGID_STOREFILE[] = L"File";
extern __declspec(selectany) OLECHAR const CFGID_STOREDATABASE[] = L"Database";