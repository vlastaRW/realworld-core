// DocumentTypeWildcards.cpp : Implementation of CDocumentTypeWildcards

#include "stdafx.h"
#include "DocumentTypeWildcards.h"


// CDocumentTypeWildcards

STDMETHODIMP CDocumentTypeWildcards::UniqueIDGet(BSTR* a_pbstrUniqueID)
{
	return FilterGet(a_pbstrUniqueID);
}

STDMETHODIMP CDocumentTypeWildcards::FilterNameGet(ILocalizedString** a_ppFilterName)
{
	try
	{
		*a_ppFilterName = NULL;
		(*a_ppFilterName = m_pFilterName)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilterName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::TypeNameGet(BSTR UNREF(a_bstrExt), ILocalizedString** a_ppTypeName)
{
	try
	{
		*a_ppTypeName = NULL;
		if (m_pTypeName == NULL)
			return S_FALSE;

		(*a_ppTypeName = m_pTypeName)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppTypeName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::IconGet(BSTR UNREF(a_bstrExt), ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = 0;
		if (m_hIconModule == NULL && m_nIconID == 0 && m_fnGetIcon == NULL)
			return S_FALSE;

		if (m_fnGetIcon)
			*a_phIcon = m_fnGetIcon(a_nSize);
		else
			*a_phIcon = (HICON)::LoadImage(m_hIconModule, MAKEINTRESOURCE(m_nIconID), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
		return NULL == *a_phIcon ? S_FALSE : S_OK;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::IconPathGet(BSTR UNREF(a_bstrExt), BSTR* a_pbstrIconPath)
{
	try
	{
		*a_pbstrIconPath = NULL;
		if (m_bstrIconPath == NULL)
			return S_FALSE;
		return m_bstrIconPath.CopyTo(a_pbstrIconPath);
	}
	catch (...)
	{
		return a_pbstrIconPath == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::DefaultExtensionGet(BSTR* a_pbstrExtension)
{
	try
	{
		*a_pbstrExtension = NULL;
		if (m_pSupportedExtensions == NULL)
			return S_FALSE;
		return m_pSupportedExtensions->Get(0, a_pbstrExtension);
	}
	catch (...)
	{
		return a_pbstrExtension == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::SupportedExtensionsGet(IEnumStrings** a_ppSupportedExtensions)
{
	try
	{
		*a_ppSupportedExtensions = NULL;
		if (m_pSupportedExtensions == NULL)
			return S_FALSE;
		return m_pSupportedExtensions.CopyTo(a_ppSupportedExtensions);
	}
	catch (...)
	{
		return a_ppSupportedExtensions == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::MatchFilename(BSTR a_bstrFilename)
{
	try
	{
		if (m_bstrFilter == NULL)
			return S_OK;

		return wildcmp(m_bstrFilter, a_bstrFilename) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::Init(ILocalizedString* a_pFilterName, BSTR a_bstrFilter)
{
	try
	{
		m_pFilterName = a_pFilterName;
		m_bstrFilter = a_bstrFilter;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::InitEx(ILocalizedString* a_pFormatName, ILocalizedString* a_pTypeName, ULONG a_nSupportedExtensions, BSTR const* a_aSupportedExtensions, BSTR a_bstrIconPath, HMODULE a_hIconModule, UINT a_nIconID, BSTR a_bstrFilter)
{
	try
	{
		m_pFilterName = a_pFormatName;
		m_pTypeName = a_pTypeName;
		if (a_nSupportedExtensions != 0 && a_aSupportedExtensions != NULL)
		{
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			pTmp->InsertMultiple(a_nSupportedExtensions, a_aSupportedExtensions);
			m_pSupportedExtensions.Attach(pTmp.Detach());
		}
		m_bstrIconPath = a_bstrIconPath;
		m_hIconModule = a_hIconModule;
		m_nIconID = a_nIconID;
		m_bstrFilter = a_bstrFilter;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::InitEx2(ILocalizedString* a_pFormatName, ILocalizedString* a_pTypeName, ULONG a_nSupportedExtensions, BSTR const* a_aSupportedExtensions, BSTR a_bstrIconPath, HICON (a_fnGetIcon)(ULONG), BSTR a_bstrFilter)
{
	try
	{
		m_pFilterName = a_pFormatName;
		m_pTypeName = a_pTypeName;
		if (a_nSupportedExtensions != 0 && a_aSupportedExtensions != NULL)
		{
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			pTmp->InsertMultiple(a_nSupportedExtensions, a_aSupportedExtensions);
			m_pSupportedExtensions.Attach(pTmp.Detach());
		}
		m_bstrIconPath = a_bstrIconPath;
		m_fnGetIcon = a_fnGetIcon;
		m_bstrFilter = a_bstrFilter;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeWildcards::FilterGet(BSTR* a_pbstrFilter)
{
	try
	{
		*a_pbstrFilter = NULL;
		return m_bstrFilter.CopyTo(a_pbstrFilter);
	}
	catch (...)
	{
		return a_pbstrFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

int CDocumentTypeWildcards::wildcmp1(wchar_t const* wild, wchar_t const* string)
{
	wchar_t const* cp;
	wchar_t const* mp;

	while ((*string) && (*wild != L'*'))
	{
		if ((*wild != *string) && (*wild != L'?') && 
			(*wild < 'A' || *wild > 'Z' || (*wild - 'A' + 'a') != *string) &&
			(*string < 'A' || *string > 'Z' && (*string - 'A' + 'a') != *wild))
		{
			return 0;
		}
		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == L'*')
		{
			++wild;
			if (*wild == L';' || !*wild)
			{
				return 1;
			}
			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == L'?') ||
				(*wild >= 'A' && *wild <= 'Z' && (*wild - 'A' + 'a') == *string) ||
				(*string >= 'A' && *string <= 'Z' && (*string - 'A' + 'a') == *wild))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == L'*')
	{
		wild++;
	}
	return *wild == L';' || !*wild;
}

int CDocumentTypeWildcards::wildcmp(wchar_t const* wild, wchar_t const* string)
{
	while (true)
	{
		if (wildcmp1(wild, string))
			return 1;
		while (L'\0' != *wild && L';' != *wild) ++wild;
		if (L'\0' == *wild)
			return 0;
		++wild;
	}
}

