
#pragma once

enum EPathAnalyzerResult
{
	EPARExistingFile = 0,
	EPARExistingFolder,
	EPARNewFileInExistingFolder,
	EPARNonExistingPath,
	EPARCustomFilter,
	EPARParsingError,
};

inline EPathAnalyzerResult AnalyzeAbsolutePath(IShellFolder* a_pDesktopFolder, std::tstring& a_strLocator, LPCTSTR a_pszDefaultExtension)
{
	// is it an existing file?
	HANDLE h = CreateFile(a_strLocator.c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE)
	{
		CloseHandle(h);
		return EPARExistingFile;
	}
	// is it an existing folder?
	CPIDL cFolder;
	ULONG nEaten = 0;
	ULONG nAttribs = SFGAO_FOLDER;
	if (SUCCEEDED(a_pDesktopFolder->ParseDisplayName(NULL, NULL, const_cast<LPWSTR>(static_cast<LPCWSTR>(CT2CW(a_strLocator.c_str()))), &nEaten, &cFolder, &nAttribs)))
	{
		return EPARExistingFolder;
	}
	// is it a new file in an existing folder?
	size_t nNamePos = a_strLocator.find_last_of(_T('\\'));
	if (nNamePos != std::tstring::npos && nNamePos >= 2)
	{
		std::tstring strFolder(a_strLocator.substr(0, nNamePos));
		if (strFolder.length() == 2 && strFolder[1] == _T(':'))
			strFolder += _T('\\'); // "C:" -> "C:\"
		if (SUCCEEDED(a_pDesktopFolder->ParseDisplayName(NULL, NULL, const_cast<LPWSTR>(static_cast<LPCWSTR>(CT2CW(strFolder.c_str()))), &nEaten, &cFolder, &nAttribs)))
		{
			size_t nDotPos = a_strLocator.find_last_of(_T('.'));
			if (a_pszDefaultExtension == NULL || a_pszDefaultExtension[0] == _T('\0') ||
				(nDotPos != std::tstring::npos && nDotPos > nNamePos))
				return EPARNewFileInExistingFolder;
			a_strLocator += _T('.');
			a_strLocator += a_pszDefaultExtension;
			HANDLE h = CreateFile(a_strLocator.c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h != INVALID_HANDLE_VALUE)
			{
				CloseHandle(h);
				return EPARExistingFile;
			}
			return EPARNewFileInExistingFolder;
		}
	}
	return EPARNonExistingPath;
}

inline EPathAnalyzerResult AnalyzePath(IMalloc* a_pShellAlloc, IShellFolder* a_pDesktopFolder, CPIDL const& a_cCurrentFolder, std::tstring& a_strLocator, LPCTSTR a_pszDefaultExtension)
{
	// characters not allowed in name /\:"*?<>|

	if (a_strLocator.empty())
		return EPARParsingError;

	if (a_strLocator.length() >= 2)
	{
		if (a_strLocator[0] == _T('\"') && a_strLocator[a_strLocator.length()-1] == _T('\"'))
		{
			// if a_strLocator is in "", never append default extension
			a_strLocator = a_strLocator.substr(1, a_strLocator.length()-2);
			a_pszDefaultExtension = NULL;
		}
	}

	if (a_strLocator.find_first_of(_T("<>|\"")) != std::tstring::npos)
		return EPARParsingError;

	if (a_strLocator.find_first_of(_T("?*")) != std::tstring::npos)
	{
		return a_strLocator.find_first_of(_T("/\:\"<>|")) != std::tstring::npos ? EPARParsingError : EPARCustomFilter;
	}

	std::replace(a_strLocator.begin(), a_strLocator.end(), _T('/'), _T('\\'));

	if (a_strLocator.length() >= 2 && ((a_strLocator[1] == _T(':') &&
		((a_strLocator[0] >= _T('A') && a_strLocator[0] <= _T('Z')) ||
		 (a_strLocator[0] >= _T('a') && a_strLocator[0] <= _T('z')))) ||
		(a_strLocator[0] == _T('\\') && a_strLocator[1] == _T('\\'))))
	{
		// looks like absolute path
		return AnalyzeAbsolutePath(a_pDesktopFolder, a_strLocator, a_pszDefaultExtension);
	}
	else if (a_strLocator.length() >= 2 && a_strLocator[0] == _T('\\') && a_strLocator[1] != _T('\\'))
	{
		// partially absolute path (missing drive spec.
		CComPtr<IShellFolder> pFolder;
		STRRET str;
		if (a_cCurrentFolder.GetIDCount())
		{
			a_pDesktopFolder->GetDisplayNameOf(a_cCurrentFolder, SHGDN_FORPARSING, &str);
		}
		else
		{
			CPIDL cFolder;
			SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &cFolder);
			a_pDesktopFolder->GetDisplayNameOf(cFolder, SHGDN_FORPARSING, &str);
		}
		LPTSTR pFld = MyStrRetToStr(a_pShellAlloc, NULL, str);
		if (pFld == NULL)
			return EPARParsingError;
		std::tstring strTmp(pFld);
		delete[] pFld;
		if (strTmp.length() < 2 || strTmp[1] != _T(':')) 
			return EPARParsingError;
		strTmp.resize(2);
		strTmp += a_strLocator;
		a_strLocator = strTmp;
		return AnalyzeAbsolutePath(a_pDesktopFolder, a_strLocator, a_pszDefaultExtension);
	}
	else
	{
		// maybe it is relative path
		CComPtr<IShellFolder> pFolder;
		STRRET str;
		if (a_cCurrentFolder.GetIDCount())
		{
			a_pDesktopFolder->GetDisplayNameOf(a_cCurrentFolder, SHGDN_FORPARSING, &str);
		}
		else
		{
			CPIDL cFolder;
			SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &cFolder);
			a_pDesktopFolder->GetDisplayNameOf(cFolder, SHGDN_FORPARSING, &str);
		}
		LPTSTR pFld = MyStrRetToStr(a_pShellAlloc, NULL, str);
		if (pFld == NULL)
			return EPARParsingError;
		std::tstring strTmp(pFld);
		delete[] pFld;
		if (strTmp[strTmp.length()-1] != _T('\\'))
			strTmp += _T('\\');
		strTmp += a_strLocator;
		a_strLocator = strTmp;
		return AnalyzeAbsolutePath(a_pDesktopFolder, a_strLocator, a_pszDefaultExtension);
	}

	return EPARParsingError;
}
