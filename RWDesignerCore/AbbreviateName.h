
#pragma once

inline size_t AfxGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
	// always capture the complete file name including extension (if present)
	LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

	// lpszTitle can be NULL which just returns the number of bytes
	if (lpszTitle == NULL)
		return _tcslen(lpszTemp)+1;

	// otherwise copy it into the buffer provided
	_tcsncpy(lpszTitle, lpszTemp, nMax);
	return 0;
}

inline void AbbreviateName(LPTSTR a_pszCanon, int a_cchMax, BOOL a_bAtLeastName)
{
	if (NULL == _tcschr(a_pszCanon, _T('\\')))
	{
		return;
	}

	size_t cchFullPath, cchFileName, cchVolName;
	const TCHAR* lpszCur;
	const TCHAR* lpszBase;
	const TCHAR* lpszFileName;

	lpszBase = a_pszCanon;
	cchFullPath = _tcslen(a_pszCanon);

	cchFileName = AfxGetFileName(a_pszCanon, NULL, 0) - 1;
	lpszFileName = lpszBase + (cchFullPath-cchFileName);

	// If a_cchMax is more than enough to hold the full path name, we're done.
	// This is probably a pretty common case, so we'll put it first.
	if (size_t(a_cchMax) >= cchFullPath)
		return;

	// If a_cchMax isn't enough to hold at least the basename, we're done
	if (size_t(a_cchMax) < cchFileName)
	{
		_tcscpy(a_pszCanon, (a_bAtLeastName) ? lpszFileName : _T(""));
		return;
	}

	// Calculate the length of the volume name.  Normally, this is two characters
	// (e.g., "C:", "D:", etc.), but for a UNC name, it could be more (e.g.,
	// "\\server\share").
	//
	// If a_cchMax isn't enough to hold at least <volume_name>\...\<base_name>, the
	// result is the base filename.

	lpszCur = lpszBase + 2;                 // Skip "C:" or leading "\\"

	if (lpszBase[0] == '\\' && lpszBase[1] == '\\') // UNC pathname
	{
		// First skip to the '\' between the server name and the share name,
		while (*lpszCur != '\\')
		{
			lpszCur = _tcsinc(lpszCur);
			ATLASSERT(*lpszCur != '\0');
		}
	}
	// check for protocols
	if (_tcsncmp(a_pszCanon, _T("res://"), 6) == 0 || _tcsncmp(a_pszCanon, _T("ftp://"), 6) == 0)
	{
		lpszCur = lpszBase + 6;
	}
	else if (_tcsncmp(a_pszCanon, _T("http://"), 7) == 0)
	{
		lpszCur = lpszBase + 7;
	}
	else if (_tcsncmp(a_pszCanon, _T("tags://"), 7) == 0)
	{
		lpszCur = lpszBase + 7;
	}
	else
	{
		// if a UNC get the share name, if a drive get at least one directory
		ATLASSERT(*lpszCur == '\\');
		// make sure there is another directory, not just c:\filename.ext
		if (cchFullPath - cchFileName > 3)
		{
			lpszCur = _tcsinc(lpszCur);
			while (*lpszCur != '\\')
			{
				lpszCur = _tcsinc(lpszCur);
				ATLASSERT(*lpszCur != '\0');
			}
		}
		ATLASSERT(*lpszCur == '\\');
	}

	cchVolName = int(lpszCur - lpszBase);
	if (size_t(a_cchMax) < cchVolName + 5 + cchFileName)
	{
		_tcscpy(a_pszCanon, lpszFileName);
		return;
	}

	// Now loop through the remaining directory components until something
	// of the form <volume_name>\...\<one_or_more_dirs>\<base_name> fits.
	//
	// Assert that the whole filename doesn't fit -- this should have been
	// handled earlier.

	ATLASSERT(cchVolName + lstrlen(lpszCur) > size_t(a_cchMax));
	while (cchVolName + 4 + lstrlen(lpszCur) > size_t(a_cchMax))
	{
		do
		{
			lpszCur = _tcsinc(lpszCur);
			ATLASSERT(*lpszCur != '\0');
		}
		while (*lpszCur != '\\' && *lpszCur != '/');
	}

	// Form the resultant string and we're done.
	a_pszCanon[cchVolName] = '\0';
	_tcscat(a_pszCanon, _T("\\..."));
	_tcscat(a_pszCanon, lpszCur);
}

