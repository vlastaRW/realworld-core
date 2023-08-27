// DocumentTypeComposed.cpp : Implementation of CDocumentTypeComposed

#include "stdafx.h"
#include "DocumentTypeComposed.h"
#include "DocumentTypeWildcards.h"
#include <MultiLanguageString.h>


// CDocumentTypeComposed

STDMETHODIMP CDocumentTypeComposed::UniqueIDGet(BSTR* a_pbstrUniqueID)
{
	try
	{
		*a_pbstrUniqueID = NULL;
		if (a_pbstrUniqueID == NULL)
			return S_FALSE;
		return m_bstrUniqueID.CopyTo(a_pbstrUniqueID);
	}
	catch (...)
	{
		return a_pbstrUniqueID == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::FilterNameGet(ILocalizedString** a_ppFilterName)
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

STDMETHODIMP CDocumentTypeComposed::TypeNameGet(BSTR a_bstrExt, ILocalizedString** a_ppTypeName)
{
	try
	{
		*a_ppTypeName = NULL;

		if (a_bstrExt && a_bstrExt[0])
		{
			for (CSubTypes::const_iterator i = m_cAllSubTypes.begin(); i != m_cAllSubTypes.end(); ++i)
			{
				CComPtr<IEnumStrings> pExts;
				(*i)->SupportedExtensionsGet(&pExts);
				ULONG nExts = 0;
				if (pExts) pExts->Size(&nExts);
				for (ULONG j = 0; j < nExts; ++j)
				{
					CComBSTR bstrExt;
					pExts->Get(j, &bstrExt);
					if (_wcsicmp(bstrExt, a_bstrExt) == 0)
					{
						HRESULT hRes = (*i)->TypeNameGet(a_bstrExt, a_ppTypeName);
						if (hRes == S_OK)
							return hRes;
						break;
					}
				}
			}
		}

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

STDMETHODIMP CDocumentTypeComposed::IconGet(BSTR a_bstrExt, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = 0;

		if (a_bstrExt && a_bstrExt[0])
		{
			for (CSubTypes::const_iterator i = m_cAllSubTypes.begin(); i != m_cAllSubTypes.end(); ++i)
			{
				CComPtr<IEnumStrings> pExts;
				(*i)->SupportedExtensionsGet(&pExts);
				ULONG nExts = 0;
				if (pExts) pExts->Size(&nExts);
				for (ULONG j = 0; j < nExts; ++j)
				{
					CComBSTR bstrExt;
					pExts->Get(j, &bstrExt);
					if (_wcsicmp(bstrExt, a_bstrExt) == 0)
					{
						HRESULT hRes = (*i)->IconGet(a_bstrExt, a_nSize, a_phIcon);
						if (hRes == S_OK)
							return hRes;
						break;
					}
				}
			}
		}

		if (m_hIconModule == NULL && m_nIconID == 0)
			return S_FALSE;

		*a_phIcon = (HICON)::LoadImage(m_hIconModule, MAKEINTRESOURCE(m_nIconID), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
		return NULL == *a_phIcon ? S_FALSE : S_OK;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::IconPathGet(BSTR a_bstrExt, BSTR* a_pbstrIconPath)
{
	try
	{
		*a_pbstrIconPath = NULL;

		if (a_bstrExt && a_bstrExt[0])
		{
			for (CSubTypes::const_iterator i = m_cAllSubTypes.begin(); i != m_cAllSubTypes.end(); ++i)
			{
				CComPtr<IEnumStrings> pExts;
				(*i)->SupportedExtensionsGet(&pExts);
				ULONG nExts = 0;
				if (pExts) pExts->Size(&nExts);
				for (ULONG j = 0; j < nExts; ++j)
				{
					CComBSTR bstrExt;
					pExts->Get(j, &bstrExt);
					if (_wcsicmp(bstrExt, a_bstrExt) == 0)
					{
						HRESULT hRes = (*i)->IconPathGet(a_bstrExt, a_pbstrIconPath);
						if (hRes == S_OK)
							return hRes;
						break;
					}
				}
			}
		}

		if (m_bstrIconPath == NULL)
			return S_FALSE;
		return m_bstrIconPath.CopyTo(a_pbstrIconPath);
	}
	catch (...)
	{
		return a_pbstrIconPath == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::DefaultExtensionGet(BSTR* a_pbstrExtension)
{
	try
	{
		*a_pbstrExtension = NULL;
		return S_FALSE;
	}
	catch (...)
	{
		return a_pbstrExtension == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::SupportedExtensionsGet(IEnumStrings** a_ppSupportedExtensions)
{
	try
	{
		*a_ppSupportedExtensions = NULL;

		if (m_pSupportedExtensions == NULL)
			return S_FALSE;
		return m_pSupportedExtensions->QueryInterface(a_ppSupportedExtensions);
	}
	catch (...)
	{
		return a_ppSupportedExtensions == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::MatchFilename(BSTR a_bstrFilename)
{
	try
	{
		HRESULT hRes = S_OK;
		if (m_bstrFilter != NULL)
		{
			if (CDocumentTypeWildcards::wildcmp(m_bstrFilter, a_bstrFilename))
				return S_OK;
			hRes = S_FALSE;
		}
		for (CSubTypes::const_iterator i = m_cSubTypes.begin(); i != m_cSubTypes.end(); ++i)
		{
			if ((*i)->MatchFilename(a_bstrFilename) == S_OK)
				return S_OK;
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::Init(BSTR a_bstrUniqueID, ILocalizedString* a_pFilterName)
{
	try
	{
		m_bstrUniqueID = a_bstrUniqueID;
		m_pFilterName = a_pFilterName;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::InitEx(BSTR a_bstrUniqueID, ILocalizedString* a_pFilterName, ILocalizedString* a_pTypeName, BSTR a_bstrIconPath, HMODULE a_hIconModule, UINT a_nIconID)
{
	try
	{
		m_bstrUniqueID = a_bstrUniqueID;
		m_pFilterName = a_pFilterName;
		m_pTypeName = a_pTypeName;
		m_bstrIconPath = a_bstrIconPath;
		m_hIconModule = a_hIconModule;
		m_nIconID = a_nIconID;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::InitAsAllFiles()
{
	try
	{
		m_bstrUniqueID = L"AllFiles";
		m_pFilterName.Attach(new CMultiLanguageString(L"[0409]All files[0405]Všechny soubory"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::InitAsAllSupportedFiles()
{
	try
	{
		m_bstrUniqueID = L"AllSupportedFiles";
		m_pFilterName.Attach(new CMultiLanguageString(L"[0409]All supported files[0405]Všechny podporované soubory"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::DocTypesAddFromList(ULONG a_nCount, IDocumentType** a_apTypes)
{
	try
	{
		for (ULONG i = 0; i < a_nCount; ++i)
		{
			if (a_apTypes[i] == NULL)
				continue; // ????

			m_cAllSubTypes.push_back(a_apTypes[i]);

			CComQIPtr<IDocumentTypeWildcards> pWCs(a_apTypes[i]);
			if (pWCs != NULL)
			{
				CComBSTR bstrFilter;
				pWCs->FilterGet(&bstrFilter);
				// TODO: better merging algorithm
				if (bstrFilter != NULL && bstrFilter.m_str[0] != L'\0')
				{
					if (m_bstrFilter != NULL)
					{
						m_bstrFilter += L";";
						m_bstrFilter += bstrFilter;
					}
					else
					{
						m_bstrFilter = bstrFilter;
					}
				}
				else
				{
					if (m_bstrFilter != NULL)
					{
						m_bstrFilter += L"*.*";
					}
					else
					{
						m_bstrFilter = L"*.*";
					}
				}
			}
			else
			{
				CComQIPtr<IDocumentTypeComposed> pComp(a_apTypes[i]);
				if (pComp != NULL)
				{
					CComPtr<IEnumUnknowns> pTypes;
					pComp->DocTypesGet(&pTypes);
					DocTypesAddFromEnum(pTypes);
				}
				else
				{
					m_cSubTypes.push_back(a_apTypes[i]);
				}
			}
			if (m_pSupportedExtensions == NULL)
				RWCoCreateInstance(m_pSupportedExtensions, __uuidof(EnumStrings));
			CComPtr<IEnumStrings> pSupExts;
			a_apTypes[i]->SupportedExtensionsGet(&pSupExts);
			if (pSupExts != NULL &&m_pSupportedExtensions != NULL)
				m_pSupportedExtensions->InsertFromEnum(pSupExts);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::DocTypesAddFromEnum(IEnumUnknowns* a_pTypes)
{
	try
	{
		ULONG nSize = 0;
		a_pTypes->Size(&nSize);
		if (nSize == NULL)
			return S_OK;
		HRESULT hRes = E_FAIL;
		IDocumentType** apTypes = reinterpret_cast<IDocumentType**>(_alloca(sizeof(IDocumentType*)*nSize));
		ZeroMemory(apTypes, sizeof(IDocumentType*)*nSize);
		a_pTypes->GetMultiple(0, nSize, __uuidof(IDocumentType), reinterpret_cast<void**>(apTypes));
		hRes = DocTypesAddFromList(nSize, apTypes);
		for (ULONG i = 0; i < nSize; ++i)
		{
			if (apTypes[i] != NULL)
				apTypes[i]->Release();
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTypeComposed::DocTypesGet(IEnumUnknowns** a_ppTypes)
{
	try
	{
		*a_ppTypes = NULL;
		CComPtr<IEnumUnknownsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumUnknowns));
		if (m_bstrFilter != NULL && m_bstrFilter.m_str[0] != L'\0')
		{
			CComObject<CDocumentTypeWildcards>* p = NULL;
			CComObject<CDocumentTypeWildcards>::CreateInstance(&p);
			CComPtr<IDocumentType> p2 = p;
			p->Init(NULL, m_bstrFilter);
			pTmp->Insert(p2);
		}
		for (CSubTypes::const_iterator i = m_cSubTypes.begin(); i != m_cSubTypes.end(); ++i)
		{
			pTmp->Insert(*i);
		}
		*a_ppTypes = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

