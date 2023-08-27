
#pragma once

namespace PortablePath
{
	inline void Full2Portable(BSTR* a_pbstrPath, OLECHAR const* a_pszRootFolder)
	{
		if (*a_pbstrPath == NULL)
			return;

		OLECHAR szTmp[MAX_PATH] = L"";
		ULONG nLongLen = GetLongPathNameW(*a_pbstrPath, szTmp, itemsof(szTmp));
		ULONG nFolderLen = wcslen(a_pszRootFolder);
		if (wcsncmp(szTmp, a_pszRootFolder, nFolderLen) == 0)
		{
			if (SysStringLen(*a_pbstrPath) <= nLongLen-nFolderLen-1)
			{
				wcscpy(*a_pbstrPath, szTmp+nFolderLen+1);
			}
			else
			{
				BSTR bstr = SysAllocString(szTmp+nFolderLen+1);
				SysFreeString(*a_pbstrPath);
				*a_pbstrPath = bstr;
			}
			return;
		}
		nFolderLen = 2;
		if (a_pszRootFolder[0] != L'\\' && a_pszRootFolder[1] == L':' &&
			wcsncmp(szTmp, a_pszRootFolder, nFolderLen) == 0)
		{
			if (SysStringLen(*a_pbstrPath) <= nLongLen-nFolderLen)
			{
				wcscpy(*a_pbstrPath, szTmp+nFolderLen);
			}
			else
			{
				BSTR bstr = SysAllocString(szTmp+nFolderLen);
				SysFreeString(*a_pbstrPath);
				*a_pbstrPath = bstr;
			}
			return;
		}
	}

	inline void Full2Portable(BSTR* a_pbstrPath, IApplicationInfo* a_pAI = NULL)
	{
		if (*a_pbstrPath == NULL)
			return;

		CComPtr<IApplicationInfo> pAI;
		if (a_pAI == NULL)
		{
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			a_pAI = pAI;
		}
		if (a_pAI == NULL || a_pAI->Portable() != S_OK)
			return;
		CComBSTR bstrRoot;
		a_pAI->AppRootFolder(&bstrRoot);

		if (bstrRoot.m_str)
			Full2Portable(a_pbstrPath, bstrRoot.m_str);
	}

	inline void Portable2Full(BSTR* a_pbstrPath, OLECHAR const* a_pszRootFolder)
	{
		if (*a_pbstrPath == NULL ||
			((*a_pbstrPath)[0] == L'\\' && (*a_pbstrPath)[1] == L'\\') ||
			((*a_pbstrPath)[0] && (*a_pbstrPath)[1] == L':') ||
			wcsstr(*a_pbstrPath, L"://"))
			return;
		if (*a_pszRootFolder == L'\0' || a_pszRootFolder[1] != L':')
			return; // invalid root
		if ((*a_pbstrPath)[0] == L'\\')
		{
			// just drive letter
			BSTR bstr = SysAllocStringLen(NULL, SysStringLen(*a_pbstrPath)+2);
			bstr[0] = a_pszRootFolder[0];
			bstr[1] = a_pszRootFolder[1];
			wcscpy(bstr+2, *a_pbstrPath);
			SysFreeString(*a_pbstrPath);
			*a_pbstrPath = bstr;
		}
		else
		{
			// relative to root folder
			BSTR bstr = SysAllocStringLen(NULL, SysStringLen(*a_pbstrPath)+wcslen(a_pszRootFolder)+1);
			wcscpy(bstr, a_pszRootFolder);
			wcscat(bstr, L"\\");
			wcscat(bstr, *a_pbstrPath);
			SysFreeString(*a_pbstrPath);
			*a_pbstrPath = bstr;
		}
	}

	inline void Portable2Full(BSTR* a_pbstrPath, IApplicationInfo* a_pAI = NULL)
	{
		if (*a_pbstrPath == NULL ||
			((*a_pbstrPath)[0] == L'\\' && (*a_pbstrPath)[1] == L'\\') ||
			((*a_pbstrPath)[0] && (*a_pbstrPath)[1] == L':') ||
			wcsstr(*a_pbstrPath, L"://"))
			return;

		CComPtr<IApplicationInfo> pAI;
		if (a_pAI == NULL)
		{
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			a_pAI = pAI;
		}
		if (a_pAI == NULL)
			return;
		CComBSTR bstrRoot;
		a_pAI->AppRootFolder(&bstrRoot);

		if (bstrRoot.m_str)
			Portable2Full(a_pbstrPath, bstrRoot.m_str);
	}

}
