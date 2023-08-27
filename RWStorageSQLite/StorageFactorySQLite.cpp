// StorageFactorySQLite.cpp : Implementation of CStorageFactorySQLite

#include "stdafx.h"
#include "StorageFactorySQLite.h"

#include "StorageSQLite.h"
#include "StorageWindowSQLite.h"
#include "StorageBrowserSQLite.h"

#include <SharedStringTable.h>
#include <IconRenderer.h>


// CStorageFactorySQLite

STDMETHODIMP CStorageFactorySQLite::NameGet(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_SQLITEFILTERNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

void GetLayersTag(IStockIcons* pSI, CIconRendererReceiver& cRenderer, IRTarget const* target)
{
	static IRPathPoint const outline[] =
	{
		{32, 0, 0, 0, -17.2313, 0},
		{80, 0, 10, 0, 0, 0},
		{108, 10, 0, 0, -7.39325, -7.39325},
		{247, 149, 12, 12, 0, 0},
		{247, 193, 0, 0, 12, -12},
		{193, 247, -12, 12, 0, 0},
		{149, 247, 0, 0, 12, 12},
		{10, 108, -7.39325, -7.39325, 0, 0},
		{0, 80, 0, 0, 0, 10},
		{0, 32, 0, -17.2313, 0, 0},
	};
	static IRPathPoint const hole[] =
	{
		{50, 50, 10.6613, -10.6613, -10.6613, 10.6613},
		{89, 50, 10.6613, 10.6613, -10.6613, -10.6613},
		{89, 89, -10.6613, 10.6613, 10.6613, -10.6613},
		{50, 89, -10.6613, -10.6613, 10.6613, 10.6613},
	};
	static IRPath const label[] = { {itemsof(outline), outline}, {itemsof(hole), hole} };
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	IRFill labelFillMat(0xffebd467);
	IROutlinedFill labelMat(&labelFillMat, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(label), label, &labelMat, target);
}

STDMETHODIMP CStorageFactorySQLite::IconGet(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		GetLayersTag(pSI, cRenderer, IRTarget(0.92f));
		*a_phIcon = cRenderer.get();
		return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFactorySQLite::SupportsGUI(DWORD UNREF(a_dwFlags))
{
	return S_OK;
}

STDMETHODIMP CStorageFactorySQLite::FilterCreate(BSTR a_bstrFilter, DWORD a_dwFlags, IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;

		if (a_bstrFilter == NULL || _wcsnicmp(a_bstrFilter, L"tags://", 7) != 0)
			return E_FAIL; // TODO: error code

		CComObject<CStorageSQLite>* pObj = NULL;
		CComObject<CStorageSQLite>::CreateInstance(&pObj);
		CComPtr<IStorageFilter> pFilter = pObj;
		if (!pObj->Init(a_bstrFilter))
			return E_FAIL; // TODO: error code

		*a_ppFilter = pFilter.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFactorySQLite::WindowCreate(BSTR a_bstrInitial, DWORD a_dwFlags, RWHWND a_hParent, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID, IStorageFilterWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;

		CComPtr<IStorageFilterWindow> pTmp;
		HRESULT hRes = E_FAIL;

		if (a_dwFlags & EFTFileBrowser)
		{
			CComObject<CStorageBrowserSQLite>* pWnd = NULL;
			CComObject<CStorageBrowserSQLite>::CreateInstance(&pWnd);
			pTmp = pWnd;
			hRes = pWnd->Init(a_bstrInitial, a_dwFlags, a_pFormatFilters, a_pContextConfig, a_pCallback, a_pListener, a_hParent, a_tLocaleID);
			if (FAILED(hRes)) return hRes;
		}
		else
		{
			CComObject<CStorageWindowSQLite>* pWnd = NULL;
			CComObject<CStorageWindowSQLite>::CreateInstance(&pWnd);
			pTmp = pWnd;
			hRes = pWnd->Init(a_bstrInitial, a_dwFlags, a_pFormatFilters, a_pContextConfig, a_pCallback, a_pListener, a_hParent, a_tLocaleID);
			if (FAILED(hRes)) return hRes;
		}

		*a_ppWindow = pTmp.Detach();
		return hRes;
	}
	catch (...)
	{
		return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFactorySQLite::ContextConfigGetDefault(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_SQL_LASTDATABASE), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SQL_LASTTAGS), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SQL_LASTFILTER), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SQL_LASTSORTSPEC), NULL, NULL, CConfigValue(L"5D3D"), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SQL_USETHUMBNAILS), NULL, NULL, CConfigValue(true), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SQL_USEGROUPS), NULL, NULL, CConfigValue(true), NULL, 0, NULL);

		pCfg->Finalize(NULL);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFactorySQLite::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		CComPtr<IStorageFilterFactory> pFct;
		RWCoCreateInstance(pFct, __uuidof(StorageFilterFactoryFileSystem));
		{
			CComPtr<IConfig> pStCfg;
			pFct->ContextConfigGetDefault(&pStCfg);
			CComQIPtr<ISubConfig> pSubCfg(pStCfg);
			pCfg->ItemInsSimple(CComBSTR(CFGID_STOREDATABASE), NULL, NULL, CConfigValue(true), pSubCfg, 0, NULL);
		}
		{
			CComPtr<IConfig> pStCfg;
			pFct->ContextConfigGetDefault(&pStCfg);
			CComQIPtr<ISubConfig> pSubCfg(pStCfg);
			pCfg->ItemInsSimple(CComBSTR(CFGID_STOREFILE), NULL, NULL, CConfigValue(true), pSubCfg, 0, NULL);
		}

		pCfg->Finalize(NULL);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFactorySQLite::MergeDefaults(ULONG a_nData, BYTE const* a_pData, ULONG a_nVersion, BSTR a_bstrDatabase)
{
	try
	{
		wchar_t const* pszDataBase = a_bstrDatabase;
		std::wstring db;
		if (pszDataBase == NULL || *pszDataBase == L'\0')
		{
			CStorageSQLite::GetDefaultDatabase(db);
			pszDataBase = db.c_str();
		}
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pszDataBase))
		{
			// no database -> simply write the data
			HANDLE h = CreateFile(pszDataBase, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (h != INVALID_HANDLE_VALUE)
			{
				DWORD dw;
				WriteFile(h, a_pData, a_nData, &dw, NULL);
				CloseHandle(h);
				return S_OK;
			}
			return E_FAIL;
		}
		CSQLiteWrapper cDb(pszDataBase);
		if (cDb.ResCode() != SQLITE_OK)
			return E_FAIL;

		bool insertVer = true;
		// check if the current database is up to date
		{
			CSQLiteStatement cVer(cDb, "SELECT val FROM meta WHERE key='initver'");
			if (sqlite3_step(cVer) == SQLITE_ROW)
			{
				wchar_t const* pszVer = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cVer, 0));
				if (pszVer)
				{
					if (_wtoi(pszVer) >= LONG(a_nVersion))
						return S_FALSE;
				}
				insertVer = false;
			}
		}

		TCHAR szTemp[MAX_PATH];
		{
			TCHAR szTempDir[MAX_PATH];
			GetTempPath(MAX_PATH-13, szTempDir);
			GetTempFileName(szTempDir, _T("rwsqin"), 0, szTemp);
			HANDLE h = CreateFile(szTemp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (h == INVALID_HANDLE_VALUE)
				return E_FAIL;
			DWORD dw;
			WriteFile(h, a_pData, a_nData, &dw, NULL);
			CloseHandle(h);
		}
		{
			CSQLiteWrapper cImp(szTemp);
			if (cImp.ResCode() != SQLITE_OK)
			{
				DeleteFile(szTemp);
				return E_FAIL;
			}
			sqlite3_exec(cDb, "BEGIN EXCLUSIVE", NULL, NULL, NULL);
			try
			{
				CSQLiteStatement cFImp(cImp, "SELECT uid,name,created,modified,accessed,data,size,note,id FROM files");
				while (sqlite3_step(cFImp) == SQLITE_ROW)
				{
					ULONGLONG nUID = 0;
					bool bSkip = false;
					char const* pszUID = reinterpret_cast<char const*>(sqlite3_column_text(cFImp, 0));
					{
						char szTmp[128] = "";
						sprintf(szTmp, "SELECT id,modified FROM files WHERE uid='%s'", pszUID);
						CSQLiteStatement cFCur(cDb, szTmp);
						if (sqlite3_step(cFCur) == SQLITE_ROW)
						{
							// conflict
							if (sqlite3_column_int64(cFCur, 1) < sqlite3_column_int64(cFImp, 3))
								nUID = sqlite3_column_int64(cFCur, 0);
							else
								bSkip = true;
						}
					}
					if (bSkip)
						continue;
					// update file
					ULONGLONG nCreated = sqlite3_column_int64(cFImp, 2);
					ULONGLONG nModified = sqlite3_column_int64(cFImp, 3);
					ULONGLONG nAccessed = sqlite3_column_int64(cFImp, 4);
					ULONG nSize = sqlite3_column_int(cFImp, 6);
					char szTmp[256];
					if (nUID)
					{
						// remove old tags (really?)
						sprintf(szTmp, "DELETE FROM tags WHERE fid=%lli", nUID);
						sqlite3_exec(cDb, szTmp, NULL, NULL, NULL);
						sprintf(szTmp, "UPDATE files SET uid='%s',name=?,created=%lli,modified=%lli,accessed=%lli,data=?,size=%i,note=? WHERE id=%lli", pszUID, nCreated, nModified, nAccessed, nSize, nUID);
					}
					else
					{
						sprintf(szTmp, "INSERT INTO files(uid,name,created,modified,accessed,data,size,note) VALUES('%s',?,%lli,%lli,%lli,?,%i,?)", pszUID, nCreated, nModified, nAccessed, nSize);
					}
					CSQLiteStatement cFUpd(cDb, szTmp);
					sqlite3_bind_text(cFUpd, 1, reinterpret_cast<char const*>(sqlite3_column_text(cFImp, 1)), -1, SQLITE_STATIC);
					void const* pData = sqlite3_column_blob(cFImp, 5);
					sqlite3_bind_blob(cFUpd, 2, pData, sqlite3_column_bytes(cFImp, 5), SQLITE_STATIC);
					sqlite3_bind_text(cFUpd, 3, reinterpret_cast<char const*>(sqlite3_column_text(cFImp, 7)), -1, SQLITE_STATIC);
					sqlite3_step(cFUpd);
					if (nUID == 0)
						nUID = sqlite3_last_insert_rowid(cDb);
					// add tags
					sprintf(szTmp, "SELECT name FROM tags WHERE fid=%lli", sqlite3_column_int64(cFImp, 8));
					CSQLiteStatement cTImp(cImp, szTmp);
					while (sqlite3_step(cTImp) == SQLITE_ROW)
					{
						sprintf(szTmp, "INSERT INTO tags(fid,name) VALUES(%lli,?)", nUID);
						CSQLiteStatement cTUpd(cDb, szTmp);
						sqlite3_bind_text(cTUpd, 1, reinterpret_cast<char const*>(sqlite3_column_text(cTImp, 0)), -1, SQLITE_STATIC);
						sqlite3_step(cTUpd);
					}
				}
				{
					char szTmp[128] = "";
					sprintf(szTmp, insertVer ? "INSERT INTO meta(key,val) VALUES('initver','%i')" : "UPDATE meta SET val='%i' WHERE key='initver'", a_nVersion);
					sqlite3_exec(cDb, szTmp, NULL, NULL, NULL);
				}
			}
			catch (...)
			{
			}
			sqlite3_exec(cDb, "COMMIT", NULL, NULL, NULL);
		}
		DeleteFile(szTemp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFactorySQLite::EnumFiles(BSTR a_bstrNameFilter, ULONG a_nTags, BSTR* a_pTags, IEnum2Strings* a_pFiles, BSTR a_bstrDatabase)
{
	if (a_nTags > 1)
		return E_NOTIMPL;

	try
	{
		wchar_t const* pszDataBase = a_bstrDatabase;
		std::wstring db;
		if (pszDataBase == NULL || *pszDataBase == L'\0')
		{
			CStorageSQLite::GetDefaultDatabase(db);
			pszDataBase = db.c_str();
		}
		CSQLiteWrapper cDb(pszDataBase);
		if (cDb.ResCode() != SQLITE_OK)
			return E_FAIL;

		if (a_nTags == 1)
		{
			if (a_bstrNameFilter == NULL)
			{
				CSQLiteStatement cStmt(cDb, "SELECT files.name,files.uid FROM tags LEFT JOIN files ON tags.fid=files.id WHERE tags.name LIKE ?");
				sqlite3_bind_text16(cStmt, 1, a_pTags[0], -1, SQLITE_STATIC);

				ULONG i = 0;
				while (sqlite3_step(cStmt) == SQLITE_ROW)
				{
					wchar_t const* pszName = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
					int n = wcslen(pszName);
					wchar_t const* pszID = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 1));
					CAutoVectorPtr<wchar_t> tmp(new wchar_t[n+100]);
					swprintf(tmp, L"tags://#%s/%s", pszID, pszName);
					if (S_OK != a_pFiles->Consume(i++, 1, &(tmp.m_p)))
						return S_FALSE;
				}
				return S_OK;
			}
			else
			{
				return E_NOTIMPL;
			}
		}
		else
		{
			// simple filter
			if (a_bstrNameFilter == NULL)
				return E_RW_INVALIDPARAM; // either tag or name filter should be used
			CSQLiteStatement cStmt(cDb, "SELECT name,uid FROM files WHERE name LIKE ?");
			sqlite3_bind_text16(cStmt, 1, a_bstrNameFilter, -1, SQLITE_STATIC);

			ULONG n = 0;
			if (sqlite3_step(cStmt) == SQLITE_ROW)
			{
				wchar_t const* pszName = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 0));
				int n = wcslen(pszName);
				wchar_t const* pszID = reinterpret_cast<wchar_t const*>(sqlite3_column_text16(cStmt, 1));
				CAutoVectorPtr<wchar_t> tmp(new wchar_t[n+100]);
				swprintf(tmp, L"tags://#%s/%s", pszID, pszName);
				if (S_OK != a_pFiles->Consume(n++, 1, &(tmp.m_p)))
					return S_FALSE;
			}
			return S_OK;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
