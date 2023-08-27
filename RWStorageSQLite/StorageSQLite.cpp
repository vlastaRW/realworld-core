// StorageSQLite.cpp : Implementation of CStorageSQLite

#include "stdafx.h"
#include "StorageSQLite.h"
#include <zlib.h>


// CStorageSQLite

bool CStorageSQLite::Init(OLECHAR const* a_bstrFilter)
{
	// examples:
	//   minimal:		tags://filename.ext
	//   with database:	tags://<C:\database.rwd>\filename.ext
	//   with keywords:	tags://<C:\database.rwd>\tag1\tag2\filename.ext
	//   with revision:	tags://<C:\database.rwd>\#Cvcvcvcvcvcvcvcvcvcvcvcv\filename.ext
	//   relative:		..blue\green\filename.ext (not applies to this scenario)
	if (a_bstrFilter == NULL || 0 != wcsncmp(a_bstrFilter, L"tags://", 7))
		return false;
	OLECHAR const* pszName = a_bstrFilter+7;
	OLECHAR const* pszL = wcschr(pszName, L'<');
	OLECHAR const* pszG = wcschr(pszName, L'>');
	if (pszL && pszG && pszL < pszG)
	{
		if (wcschr(pszL+1, L'<') || wcschr(pszG+1, L'>'))
			return false; // invalid path - both < and > must be used exactly once if at all
		m_szDatabase.assign(pszL+1, pszG);
		pszName = pszG+1;
	}
	else if (pszL || pszG)
		return false; // invalid path - both < and > must be used exactly once if at all
	if (m_szDatabase.empty())
	{
		// use default database path
		GetDefaultDatabase(m_szDatabase);
	}
	while (*pszName == L'\\' || *pszName == L'/')
		++pszName;

	m_szGUID[0] = L'\0';
	m_bUIDValid = false;
	m_nUID = -1LL;

	// parse tags, GUID and filename
	while (true)
	{
		OLECHAR const* pszSep = pszName;
		while (*pszSep && *pszSep != L'\\' && *pszSep != L'/')
			++pszSep;
		if (pszSep > pszName)
		{
			if (*pszName == L'#' && pszSep-pszName == 23)
			{
				// GUID
				wcsncpy(m_szGUID, pszName+1, 22);
				m_szGUID[22] = L'\0';
			}
			else if (*pszSep == L'\0')
			{
				// filename
				m_szFileName.assign(pszName, pszSep);
			}
			else
			{
				// tag
				size_t n = m_cTags.size();
				m_cTags.resize(n+1);
				m_cTags[n].assign(pszName, pszSep);
			}
		}
		if (*pszSep == L'\0')
			break;
		pszName = pszSep+1;
	}
	std::sort(m_cTags.begin(), m_cTags.end());
	return true;
}

STDMETHODIMP CStorageSQLite::ToText(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter)
{
	try
	{
		*a_pbstrFilter = NULL;
		if (a_pRoot)
		{
			CComBSTR bstrRoot;
			a_pRoot->ToText(NULL, &bstrRoot);
			if (bstrRoot && wcsncmp(bstrRoot, L"tags://", 7) == 0)
			{
				CComObject<CStorageSQLite>* pOther = NULL;
				CComObject<CStorageSQLite>::CreateInstance(&pOther);
				CComPtr<IStorageFilter> pTmp = pOther;
				CStorageSQLite* pAccess = pOther;
				pAccess->Init(bstrRoot);
				if (pAccess->m_szDatabase == m_szDatabase)
				{
					CSQLiteWrapper cDb(m_szDatabase.c_str());
					if (cDb.ResCode() == SQLITE_OK)
					{
						bool bWithUID = m_szGUID[0];
						ValidateUID(cDb);
						if (bWithUID && m_nUID > 0)
						{
							CComPtr<IStorageFilter> p2;
							pOther->SubFilterGet(CComBSTR(m_szFileName.c_str()), &p2);
							if (p2)
							{
								static_cast<CStorageSQLite*>(p2.p)->ValidateUID(cDb);
								bWithUID = static_cast<CStorageSQLite*>(p2.p)->m_nUID != m_nUID;
							}
						}
						if (bWithUID)
						{
							CComBSTR bstr(L"#");
							bstr += m_szGUID;
							bstr += L"/";
							bstr += m_szFileName.c_str();
							*a_pbstrFilter = bstr.Detach();
						}
						else
						{
							CComBSTR bstr(m_szFileName.c_str());
							*a_pbstrFilter = bstr.Detach();
						}
						return S_OK;
					}
				}
			}
		}
		std::wstring strDef;
		GetDefaultDatabase(strDef);
		CComBSTR bstr(L"tags://");
		if (m_szDatabase != strDef)
		{
			bstr += "<";
			bstr += m_szDatabase.c_str();
			bstr += L">/";
		}
		for (CTags::const_iterator i = m_cTags.begin(); i != m_cTags.end(); ++i)
		{
			bstr += i->c_str();
			bstr += L"/";
		}
		if (m_szGUID[0])
		{
			bstr += L"#";
			bstr += m_szGUID;
			bstr += L"/";
		}
		if (!m_szFileName.empty())
			bstr += m_szFileName.c_str();
		*a_pbstrFilter = bstr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrFilter ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageSQLite::SubFilterGet(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;
		if (a_bstrRelativeLocation == NULL)
			return E_RW_INVALIDPARAM;

		if (wcschr(a_bstrRelativeLocation, L'<') || wcschr(a_bstrRelativeLocation, L'>'))
			return E_FAIL; // invalid characters

		OLECHAR szGUID[23] = L"\0";
		std::wstring strFileName;
		CTags cTags;

		// parse tags, GUID and filename
		OLECHAR const* pszName = a_bstrRelativeLocation;
		while (true)
		{
			OLECHAR const* pszSep = pszName;
			while (*pszSep && *pszSep != L'\\' && *pszSep != L'/')
				++pszSep;
			if (pszSep > pszName)
			{
				if (*pszName == L'#' && pszSep-pszName == 23)
				{
					// GUID
					wcsncpy(m_szGUID, pszName+1, 22);
					szGUID[22] = L'\0';
				}
				else if (*pszSep == L'\0')
				{
					// filename
					strFileName.assign(pszName, pszSep);
				}
				else
				{
					// tag
					size_t n = cTags.size();
					cTags.resize(n+1);
					cTags[n].assign(pszName, pszSep);
				}
			}
			if (*pszSep == L'\0')
				break;
			pszName = pszSep+1;
		}

		CComObject<CStorageSQLite>* pLoc = NULL;
		CComObject<CStorageSQLite>::CreateInstance(&pLoc);
		CComPtr<IStorageFilter> pTmp = pLoc;

		if (szGUID[0] == NULL) // having GUID is easy->just use what was given
		{
			CSQLiteWrapper cDb(m_szDatabase.c_str());
			if (cDb.ResCode() == SQLITE_OK)
			{
				ValidateUID(cDb);
			}
			if (m_nUID > 0)
			{
				// get tags for this uid and use them for the new locator
				cTags.clear(); // use my tags instead of the given ones
				wchar_t szQuery[256];
				swprintf(szQuery, L"SELECT name FROM tags WHERE fid=%lli", m_nUID);
				CSQLiteStatement cStmt(cDb, szQuery);
				while (sqlite3_step(cStmt) == SQLITE_ROW)
				{
					wchar_t const* pszTag = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
					if (pszTag)
						cTags.push_back(std::wstring(pszTag));
				}
			}
		}
		pLoc->Init(m_szDatabase.c_str(), strFileName.c_str(), szGUID, cTags);
		*a_ppFilter = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilter ? E_UNEXPECTED : E_POINTER;
	}
}

//struct SStream { ULONG nLength; CAutoVectorPtr<BYTE> pData; bool bFound; };
//int AcceptStream(void* p, int n, char** data, char** names)
//{
//	SStream* pDst = reinterpret_cast<SStream*>(p);
//	ATLASSERT(n == 1);
//	ATLASSERT(data[0][0] == 'X' && data[0][1] == '\'');
//	char* pSrc = data[0]+2;
//	pDst->nLength = 0;
//	while (*pSrc != '\'' && *pSrc) { ++pSrc; ++pDst->nLength; }
//	if ((pDst->nLength&1) || *pSrc != '\'')
//		return 1; // invalid format;
//	pDst->bFound = true;
//	pDst->nLength >>= 1;
//	if (pDst->nLength)
//	{
//		if (!pDst->pData.Allocate(pDst->nLength))
//			return 1;
//	}
//	pSrc = data[0]+2;
//	for (ULONG i = 0; i < pDst->nLength; ++i)
//	{
//		BYTE const b =
//			((pSrc[0] <= '9' ? (pSrc[0]-'0') : (pSrc[0] <= 'F' ? (pSrc[0]-'A'+10) : (pSrc[0]-'a'+10)))<<4) |
//			(pSrc[0] <= '9' ? (pSrc[0]-'0') : (pSrc[0] <= 'F' ? (pSrc[0]-'A'+10) : (pSrc[0]-'a'+10)));
//		pSrc += 2;
//		pDst->pData[i] = b;
//	}
//	return 0;
//}

void CStorageSQLite::ValidateUID(sqlite3* a_pDb)
{
	wchar_t szQuery[256];
	if (!m_bUIDValid)
	{
		std::vector<LONGLONG> cIDs;
		{
			if (m_szGUID[0])
				swprintf(szQuery, L"SELECT id FROM files WHERE uid='%s' ORDER BY modified DESC", m_szGUID);
			else
				wcscpy(szQuery, L"SELECT id FROM files WHERE name=? ORDER BY modified DESC");
			CSQLiteStatement cStmt(a_pDb, szQuery);
			if (!m_szGUID[0])
				sqlite3_bind_text16(cStmt, 1, m_szFileName.c_str(), -1, SQLITE_STATIC);
			while (sqlite3_step(cStmt) == SQLITE_ROW)
				cIDs.push_back(sqlite3_column_int64(cStmt, 0));
		}
		if (cIDs.empty())
		{
			m_nUID = -1LL;
		}
		else if (cIDs.size() == 1 || m_cTags.empty())
		{
			// just one result or no tags specified -> take the first file
			m_nUID = cIDs[0];
		}
		else
		{
			// find the most relevant file
			int iBestRating = -100000;
			for (std::vector<LONGLONG>::const_iterator i = cIDs.begin(); i != cIDs.end(); ++i)
			{
				int iRating = 0;
				swprintf(szQuery, L"SELECT name FROM tags WHERE fid=%lli", *i);
				CSQLiteStatement cStmt(a_pDb, szQuery);
				std::vector<LONGLONG> cIDs;
				while (sqlite3_step(cStmt) == SQLITE_ROW)
				{
					--iRating;
					wchar_t const* pszTag = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
					for (CTags::const_iterator j = m_cTags.begin(); j != m_cTags.end(); ++j)
					{
						if (wcscmp(pszTag, j->c_str()) == 0)
						{
							iRating += 21;
							break;
						}
					}
				}
				if (iRating > iBestRating)
				{
					iBestRating = iRating;
					m_nUID = *i;
				}
			}
		}
		m_bUIDValid = true;
	}
}

STDMETHODIMP CStorageSQLite::SrcOpen(IDataSrcDirect** a_ppSrc)
{
	try
	{
		if (a_ppSrc) *a_ppSrc = NULL;

		CSQLiteWrapper cDb(m_szDatabase.c_str());
		if (cDb.ResCode() != SQLITE_OK)
			return E_FAIL;

		ValidateUID(cDb);
		if (m_nUID < 0)
			return E_RW_ITEMNOTFOUND;

		ULONG nLength = 0;
		CAutoVectorPtr<BYTE> pData;
		bool bFound = false;

		wchar_t szQuery[256];
		swprintf(szQuery, L"SELECT data,size FROM files WHERE id=%lli", m_nUID);

		{
			CSQLiteStatement cStmt(cDb, szQuery);
			if (sqlite3_step(cStmt) == SQLITE_ROW)
			{
				ULONG nSize = sqlite3_column_int(cStmt, 1);
				BYTE const* pSrc = reinterpret_cast<BYTE const*>(sqlite3_column_blob(cStmt, 0));
				nLength = sqlite3_column_bytes(cStmt, 0);
				if (nLength)
				{
					if (a_ppSrc == NULL)
						return S_OK;
					bFound = true;
					if (pData.Allocate(nSize))
					{
						if (nSize > nLength)
						{
							// unzip
							uncompress(pData, &nSize, pSrc, nLength);
							nLength = nSize;
						}
						else
						{
							// copy
							CopyMemory(pData.m_p, pSrc, nLength);
						}
					}
				}
			}
		}

		if (!bFound)
			return E_RW_ITEMNOTFOUND;
		CComObject<CSrcDirect>* p = NULL;
		CComObject<CSrcDirect>::CreateInstance(&p);
		CComPtr<IDataSrcDirect> pTmp = p;
		p->Init(nLength, pData);
		pData.Detach();
		*a_ppSrc = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSrc ? E_UNEXPECTED : E_POINTER;
	}
}

void escape_srtring(std::wstring const& a_in, std::wstring& a_out)
{
	size_t tOff = 0;
	size_t tCnt = 0;
	while (std::wstring::npos != (tOff = a_in.find(L'\'', tOff)))
	{
		++tOff;
		++tCnt;
	}
	if (tOff == 0)
		a_out = a_in;
	a_out.resize(a_in.length()+tCnt);
	tCnt = 0;
	for (tOff = 0; tOff < a_in.length(); ++tOff, ++tCnt)
	{
		if (a_in[tOff] == L'\'')
			a_out[tCnt++] = L'\\';
		a_out[tCnt] = a_in[tOff];
	}
}

STDMETHODIMP CStorageSQLite::DstOpen(IDataDstStream** a_ppDst)
{
	try
	{
		*a_ppDst = NULL;

		CSQLiteWrapper cDb(m_szDatabase.c_str());
		if (cDb.ResCode() != SQLITE_OK)
			return E_FAIL;

		ValidateUID(cDb);
		if (m_nUID > 0)
		{
			if (!m_szFileName.empty())
			{
				// rename the file
				wchar_t szQuery[MAX_PATH+100];
				swprintf(szQuery, L"UPDATE files SET name=? WHERE id=%lli", m_nUID);
				CSQLiteStatement cStmt(cDb, szQuery);
				sqlite3_bind_text16(cStmt, 1, m_szFileName.c_str(), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
				if (sqlite3_step(cStmt) != SQLITE_DONE)
					return E_FAIL;
			}
		}
		else
		{
			// create new file
			if (m_szGUID[0] == L'\0')
			{
				// guid not specified
				GUID tGUID;
				CoCreateGuid(&tGUID);
				BYTE const* p = reinterpret_cast<BYTE const*>(&tGUID);
				Base64Encode(p, p+16, m_szGUID);
				m_szGUID[22] = L'\0';
			}
			//std::wstring str;
			//escape_srtring(m_szFileName, str);
			FILETIME tFT;
			GetSystemTimeAsFileTime(&tFT);
			{
				wchar_t szQuery[MAX_PATH+100];
				swprintf(szQuery, L"INSERT INTO files(uid,created,name) VALUES('%s',%lli,?)", m_szGUID, *reinterpret_cast<ULONGLONG*>(&tFT));
				CSQLiteStatement cStmt(cDb, szQuery);
				sqlite3_bind_text16(cStmt, 1, m_szFileName.c_str(), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
				if (SQLITE_DONE != sqlite3_step(cStmt))
					return E_FAIL;
			}
			m_nUID = sqlite3_last_insert_rowid(cDb);
		}

		typedef std::set<std::wstring> CTagMap;
		CTagMap cNew;
		CTagMap cOld;
		std::copy(m_cTags.begin(), m_cTags.end(), std::inserter(cNew, cNew.end()));
		{
			char szQuery[256];
			sprintf(szQuery, "SELECT name,id FROM tags WHERE fid=%lli", m_nUID);
			CSQLiteStatement cStmt(cDb, szQuery);
			while (sqlite3_step(cStmt) == SQLITE_ROW)
			{
				wchar_t const* psz = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
				if (psz)
					cOld.insert(std::wstring(psz));
			}
		}
		CTags cToAdd;
		std::set_difference(cNew.begin(), cNew.end(), cOld.begin(), cOld.end(), std::back_inserter(cToAdd));
		CTags cToDel;
		std::set_difference(cOld.begin(), cOld.end(), cNew.begin(), cNew.end(), std::back_inserter(cToDel));

		// delete old tags (TODO: optimize using ids fetched before)
		for (CTags::const_iterator iT = cToDel.begin(); iT != cToDel.end(); ++iT)
		{
			char szQuery[256];
			sprintf(szQuery, "DELETE FROM tags WHERE fid=%lli AND name=?", m_nUID);
			CSQLiteStatement cStmt(cDb, szQuery);
			sqlite3_bind_text16(cStmt, 1, iT->c_str(), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
			if (SQLITE_DONE != sqlite3_step(cStmt))
				return E_FAIL;
		}

		// add new tags
		for (CTags::const_iterator iT = cToAdd.begin(); iT != cToAdd.end(); ++iT)
		{
			char szQuery[256];
			sprintf(szQuery, "INSERT INTO tags(fid,name) VALUES(%lli,?)", m_nUID);
			CSQLiteStatement cStmt(cDb, szQuery);
			sqlite3_bind_text16(cStmt, 1, iT->c_str(), -1, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
			if (SQLITE_DONE != sqlite3_step(cStmt))
				return E_FAIL;
		}

		CComObject<CDstStream>* p = NULL;
		CComObject<CDstStream>::CreateInstance(&p);
		CComPtr<IDataDstStream> pTmp = p;
		p->Init(this);

		*a_ppDst = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDst ? E_UNEXPECTED : E_POINTER;
	}
}

HRESULT CStorageSQLite::WriteData(BYTE const* a_pData, ULONG a_nData)
{
	try
	{
		CSQLiteWrapper cDb(m_szDatabase.c_str());
		if (cDb.ResCode() != SQLITE_OK)
			return E_FAIL;

		ValidateUID(cDb);
		if (m_nUID < 0)
			return E_RW_ITEMNOTFOUND;

		ULONG nRealLen = a_nData;
		ULONG nBuf = compressBound(a_nData);
		CAutoVectorPtr<BYTE> cBuf(new BYTE[nBuf]);
		int result = compress(cBuf, &nBuf, a_pData, a_nData);
		if (nBuf < a_nData-(a_nData>>3))
		{
			a_nData = nBuf;
			a_pData = cBuf;
		}
		else
		{
			cBuf.Free();
		}

		FILETIME tFT;
		GetSystemTimeAsFileTime(&tFT);

		HRESULT hRes = S_OK;

		{
			char szQuery[256];
			sprintf(szQuery, "UPDATE files SET data=?,size=%i,modified=%lli WHERE id=%lli", nRealLen, *reinterpret_cast<ULONGLONG*>(&tFT), m_nUID);
			CSQLiteStatement cStmt(cDb, szQuery);
			int resCode = sqlite3_bind_blob(cStmt, 1, a_pData, a_nData, SQLITE_STATIC/*SQLITE_TRANSIENT*/);
			if (resCode != SQLITE_OK) hRes = E_FAIL;
			resCode = sqlite3_step(cStmt);
			if (resCode != SQLITE_DONE) hRes = E_FAIL;
		}

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageSQLite::GetTime(EStorageTimeType a_eType, ULONGLONG* a_pTime)
{
	try
	{
		*a_pTime = 0;

		CSQLiteWrapper cDb(m_szDatabase.c_str());
		if (cDb.ResCode() != SQLITE_OK)
			return E_FAIL;

		ValidateUID(cDb);
		if (m_nUID < 0)
			return E_RW_ITEMNOTFOUND;

		{
			char szQuery[128];
			sprintf(szQuery, "SELECT %s FROM files WHERE id=%lli", a_eType == ESTTCreation ? "created" : (a_eType == ESTTModification ? "modified" : "accessed"), m_nUID);
			CSQLiteStatement cStmt(cDb, szQuery);
			if (sqlite3_step(cStmt) == SQLITE_ROW)
			{
				*a_pTime = sqlite3_column_int64(cStmt, 0);
				return S_OK;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_pTime ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageSQLite::SetTime(EStorageTimeType a_eType, ULONGLONG a_nTime)
{
	try
	{
		CSQLiteWrapper cDb(m_szDatabase.c_str());
		if (cDb.ResCode() != SQLITE_OK)
			return E_FAIL;

		ValidateUID(cDb);
		if (m_nUID < 0)
			return E_RW_ITEMNOTFOUND;

		{
			char szQuery[128];
			sprintf(szQuery, "UPDATE files SET %s=%lli WHERE id=%lli", a_eType == ESTTCreation ? "created" : (a_eType == ESTTModification ? "modified" : "accessed"), a_nTime, m_nUID);
			CSQLiteStatement cStmt(cDb, szQuery);
			return sqlite3_step(cStmt) == SQLITE_DONE ? S_OK : E_FAIL;
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP CStorageSQLite::CSrcDirect::SizeGet(ULONG* a_pnSize)
{
	try
	{
		*a_pnSize = m_nData;
		return S_OK;
	}
	catch (...)
	{
		return a_pnSize ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageSQLite::CSrcDirect::SrcLock(ULONG a_nOffset, ULONG a_nSize, BYTE const** a_ppBuffer)
{
	try
	{
		*a_ppBuffer = NULL;
		if ((a_nOffset+a_nSize) > m_nData)
			return E_RW_INVALIDPARAM;
		*a_ppBuffer = m_pData.m_p+a_nOffset;
		return S_OK;
	}
	catch (...)
	{
		return a_ppBuffer ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageSQLite::CSrcDirect::SrcUnlock(ULONG UNREF(a_nSize), BYTE const* UNREF(a_pBuffer))
{
	return S_OK;
}

STDMETHODIMP CStorageSQLite::CDstStream::SizeGet(ULONG* a_pnSize)
{
	try
	{
		*a_pnSize = m_nWritten;
		return S_OK;
	}
	catch (...)
	{
		return a_pnSize ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageSQLite::CDstStream::Write(ULONG a_nSize, BYTE const* a_pBuffer)
{
	try
	{
		EnsureSize(m_nPos+a_nSize);
		CopyMemory(m_pData.m_p+m_nPos, a_pBuffer, a_nSize);
		m_nPos += a_nSize;
		if (m_nWritten < m_nPos)
			m_nWritten = m_nPos;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageSQLite::CDstStream::Seek(ULONG a_nSize)
{
	try
	{
		EnsureSize(a_nSize);
		if (m_nWritten < a_nSize)
		{
			ZeroMemory(m_pData.m_p+m_nWritten, a_nSize-m_nWritten);
			m_nWritten = a_nSize;
		}
		m_nPos = a_nSize;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageSQLite::CDstStream::Close()
{
	try
	{
		if (m_pOwner == NULL)
			return S_FALSE; // nothing written or already closed
		HRESULT hRes = m_pOwner->WriteData(m_pData, m_nWritten);
		m_pOwner->Release();
		m_pOwner = NULL;
		m_pData.Free();
		m_nAlloc = m_nPos = m_nWritten = 0;
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CStorageSQLite::CDstStream::EnsureSize(ULONG a_nSize)
{
	if (a_nSize <= m_nAlloc)
		return;
	ULONG const nNewSize = max(a_nSize, m_nAlloc+(m_nAlloc>>1)+1);
	CAutoVectorPtr<BYTE> p(new BYTE[nNewSize]);
	CopyMemory(p.m_p, m_pData.m_p, m_nWritten);
	m_nAlloc = nNewSize;
	std::swap(m_pData.m_p, p.m_p);
}

void CStorageSQLite::GetDefaultDatabase(std::wstring& a_strBuffer)
{
	CComPtr<IApplicationInfo> pAI;
	RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
	CComBSTR bstrFolder;
	pAI->AppDataFolder(&bstrFolder);
	a_strBuffer = bstrFolder;
	a_strBuffer.append(L"\\default.rwdb");
}
