// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"

#include "RWStorage.h"
#include "SharedStringTable.h"


LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	RECT rc;
	GetDlgItem(IDC_EDIT1).GetWindowRect(&rc);

	LONG n = rc.bottom-rc.top+8;
	rc.top += n;
	rc.bottom += n;
	ScreenToClient(&rc);

	m_wndFolderBreadcrumbs.Create(m_hWnd, rc, _T("C:\\MyProjects\\RealWorld"), WS_VISIBLE|WS_TABSTOP|WS_CHILD, WS_EX_CLIENTEDGE, 333);
	m_wndFolderBreadcrumbs.SetFont(GetFont(), FALSE);

	return TRUE;
}

LRESULT CMainDlg::OnTestDlg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CComPtr<IStorageManager> pMgr;
	RWCoCreateInstance(pMgr, __uuidof(StorageManager));
	CComPtr<IStorageFilter> pFlt;
	HRESULT hRes = pMgr->FilterCreateInteractivelyUID(CComBSTR(L"C:\\MyProjects\\RealWorld\\empty.rwo"), EFTOpenExisting, m_hWnd, NULL, NULL, GUID_NULL, NULL, NULL, GetThreadLocale(), &pFlt);
	TCHAR szBuf[256];
	_stprintf(szBuf, _T("Result code: %08x"), hRes);
	MessageBox(szBuf, _T("FilterCreateInteractively"));
	if (pFlt == NULL)
	{
		MessageBox(_T("Critical: filter is NULL"), _T("FilterCreateInteractively"));
	}
	else
	{
		CComBSTR bstrFilter;
		pFlt->ToText(NULL, &bstrFilter);
		if (bstrFilter == NULL)
		{
			MessageBox(_T("Critical: ToText returned NULL"), _T("FilterCreateInteractively"));
		}
		else
		{
			USES_CONVERSION;
			_stprintf(szBuf, _T("Filter text: %s"), OLE2T(bstrFilter));
			MessageBox(szBuf, _T("FilterCreateInteractively"));
		}
	}

	return 0;
}

LRESULT CMainDlg::OnTestEmptyDlg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CComPtr<IEnumUnknownsInit> pFilters;
	RWCoCreateInstance(pFilters, __uuidof(EnumUnknowns));
	CComPtr<IDocumentTypeWildcards> pTextFiles;
	RWCoCreateInstance(pTextFiles, __uuidof(DocumentTypeWildcards));
	CComBSTR bstrTXT(L"txt");
	pTextFiles->InitEx(_SharedStringTable.GetStringAuto(IDS_TEXTFILES), NULL, 1, &(bstrTXT.m_str), NULL, NULL, 0, CComBSTR(L"*.txt"));
	pFilters->Insert(pTextFiles.p);
	CComPtr<IDocumentTypeWildcards> pPdfFiles;
	RWCoCreateInstance(pPdfFiles, __uuidof(DocumentTypeWildcards));
	CComBSTR bstrPDF(L"pdf");
	pPdfFiles->InitEx(_SharedStringTable.GetStringAuto(IDS_PDFFILES), NULL, 1, &(bstrPDF.m_str), NULL, NULL, 0, CComBSTR(L"*.pdf"));
	pFilters->Insert(pPdfFiles.p);

	CComPtr<IStorageManager> pMgr;
	RWCoCreateInstance(pMgr, __uuidof(StorageManager));
	CComPtr<IStorageFilter> pFlt;
	HRESULT hRes = pMgr->FilterCreateInteractivelyUID(CComBSTR(L""), EFTOpenExisting, m_hWnd, pFilters.p, NULL, GUID_NULL, NULL, NULL, GetThreadLocale(), &pFlt);
	TCHAR szBuf[256];
	_stprintf(szBuf, _T("Result code: %08x"), hRes);
	MessageBox(szBuf, _T("FilterCreateInteractively"));
	if (pFlt == NULL)
	{
		MessageBox(_T("Critical: filter is NULL"), _T("FilterCreateInteractively"));
	}
	else
	{
		CComBSTR bstrFilter;
		pFlt->ToText(NULL, &bstrFilter);
		if (bstrFilter == NULL)
		{
			MessageBox(_T("Critical: ToText returned NULL"), _T("FilterCreateInteractively"));
		}
		else
		{
			USES_CONVERSION;
			_stprintf(szBuf, _T("Filter text: %s"), OLE2T(bstrFilter));
			MessageBox(szBuf, _T("FilterCreateInteractively"));
		}
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedTestFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	USES_CONVERSION;

	CComPtr<IStorageManager> pMgr;
	RWCoCreateInstance(pMgr, __uuidof(StorageManager));

	CComPtr<IStorageFilter> pFlt;
	HRESULT hRes = pMgr->FilterCreate(CComBSTR(L"file://C:\\MyProjects\\RealWorld\\_testData\\Storage\\32bytes.txt"), 0, &pFlt);
	if (FAILED(hRes))
	{
		TCHAR szBuf[256];
		_stprintf(szBuf, _T("FAILED: %08x"), hRes);
		MessageBox(szBuf, _T("FilterCreate-FIleSystem"));
	}
	else
	{
		CComBSTR bstrNewText;
		pFlt->ToText(NULL, &bstrNewText);

		MessageBox(OLE2CT(bstrNewText), _T("FilterCreate-FIleSystem"));

		CComPtr<IDataSrcDirect> pSrc;
		if (FAILED(pFlt->SrcOpen(&pSrc)))
		{
			MessageBox(_T("FAILED"), _T("SrcOpen-FIleSystem"));
		}
		else
		{
			if (pSrc == NULL)
			{
				MessageBox(_T("CRITICAL: pSrc is NULL"), _T("SrcOpen-FIleSystem"));
			}
			else
			{
				ULONG nSize = 0;
				pSrc->SizeGet(&nSize);
				TCHAR szBuf[256];
				_stprintf(szBuf, _T("Size is: %i"), nSize);
				MessageBox(szBuf, _T("pSrc->SizeGet-FIleSystem"));
				if (nSize > 0)
				{
					BYTE* pBuffer = new BYTE[nSize+1];
					pBuffer[nSize] = '\0';
					BYTE const* pData;
					pSrc->SrcLock(0, nSize, &pData);
					CopyMemory(pBuffer, pData, nSize);
					pSrc->SrcUnlock(nSize, pData);
					MessageBox(A2CT(LPSTR(pBuffer)), _T("Data-FileSystem")); //A2T is ugly
					delete[] pBuffer;
				}
			}
		}
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedTestHttp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	USES_CONVERSION;

	CComPtr<IStorageManager> pMgr;
	RWCoCreateInstance(pMgr, __uuidof(StorageManager));

	CComPtr<IStorageFilter> pFlt;
	HRESULT hRes = pMgr->FilterCreate(CComBSTR(L"http://logout.sh.cvut.cz/~datel/32bytes.txt"), 0, &pFlt);
	if (FAILED(hRes))
	{
		TCHAR szBuf[256];
		_stprintf(szBuf, _T("FAILED: %08x"), hRes);
		MessageBox(szBuf, _T("FilterCreate-HTTP"));
	}
	else
	{
		CComBSTR bstrNewText;
		pFlt->ToText(NULL, &bstrNewText);

		MessageBox(OLE2CT(bstrNewText), _T("FilterCreate-HTTP"));

		CComPtr<IDataSrcDirect> pSrc;
		if (FAILED(pFlt->SrcOpen(&pSrc)))
		{
			MessageBox(_T("FAILED"), _T("SrcOpen-HTTP"));
		}
		else
		{
			if (pSrc == NULL)
			{
				MessageBox(_T("CRITICAL: pSrc is NULL"), _T("SrcOpen-HTTP"));
			}
			else
			{
				ULONG nSize = 0;
				pSrc->SizeGet(&nSize);
				TCHAR szBuf[256];
				_stprintf(szBuf, _T("Size is: %i"), nSize);
				MessageBox(szBuf, _T("pSrc->SizeGet-HTTP"));
				if (nSize > 0)
				{
					BYTE* pBuffer = new BYTE[nSize+1];
					pBuffer[nSize] = '\0';
					BYTE const* pData;
					pSrc->SrcLock(0, nSize, &pData);
					CopyMemory(pBuffer, pData, nSize);
					pSrc->SrcUnlock(nSize, pData);
					MessageBox(A2CT(LPSTR(pBuffer)), _T("Data-HTTP")); //A2T is ugly
					delete[] pBuffer;
				}
			}
		}
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedButton4(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	USES_CONVERSION;

	CComPtr<IStorageManager> pMgr;
	RWCoCreateInstance(pMgr, __uuidof(StorageManager));

	TCHAR szTmp[MAX_PATH] = _T("");
	GetDlgItemText(IDC_EDIT1, szTmp, MAX_PATH);
	CComPtr<IStorageFilter> pFlt;
	HRESULT hRes = pMgr->FilterCreate(CComBSTR(szTmp), 0, &pFlt);
	if (FAILED(hRes))
	{
		TCHAR szBuf[256];
		_stprintf(szBuf, _T("FAILED: %08x"), hRes);
		MessageBox(szBuf, _T("FilterCreate-custom"));
	}
	else
	{
		CComBSTR bstrNewText;
		pFlt->ToText(NULL, &bstrNewText);

		MessageBox(OLE2CT(bstrNewText), _T("FilterCreate-custom"));

		CComPtr<IDataSrcDirect> pSrc;
		if (FAILED(pFlt->SrcOpen(&pSrc)))
		{
			MessageBox(_T("FAILED"), _T("SrcOpen-custom"));
		}
		else
		{
			if (pSrc == NULL)
			{
				MessageBox(_T("CRITICAL: pSrc is NULL"), _T("SrcOpen-custom"));
			}
			else
			{
				ULONG nSize = 0;
				pSrc->SizeGet(&nSize);
				TCHAR szBuf[256];
				_stprintf(szBuf, _T("Size is: %i"), nSize);
				MessageBox(szBuf, _T("pSrc->SizeGet-custom"));
				if (nSize > 0)
				{
					BYTE* pBuffer = new BYTE[nSize+1];
					pBuffer[nSize] = '\0';
					BYTE const* pData;
					pSrc->SrcLock(0, nSize, &pData);
					CopyMemory(pBuffer, pData, nSize);
					pSrc->SrcUnlock(nSize, pData);
					MessageBox(A2CT(LPSTR(pBuffer)), _T("Data-custom")); //A2T is ugly
					delete[] pBuffer;
				}
			}
		}
	}

	return 0;
}

LRESULT CMainDlg::OnStandardOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR szName[MAX_PATH] = _T("C:\\abc.txt");
	OPENFILENAME tOFN;
	ZeroMemory(&tOFN, sizeof(tOFN));
	tOFN.lStructSize = sizeof(tOFN);
	tOFN.hwndOwner = m_hWnd;
	tOFN.lpstrFile = szName;
	tOFN.nMaxFile = MAX_PATH;

	GetOpenFileName(&tOFN);

	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}


