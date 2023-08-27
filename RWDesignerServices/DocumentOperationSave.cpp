// DocumentOperationSave.cpp : Implementation of CDocumentOperationSave

#include "stdafx.h"
#include "DocumentOperationSave.h"

#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <DocumentName.h>


// CDocumentOperationSave

STDMETHODIMP CDocumentOperationSave::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Document - Save[0405]Dokument - uložit");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSave::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		return E_NOTIMPL;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSave::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSave::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IApplicationInfo> pAI;
		RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
		ULONG nDays = 0;
		ELicensingMode eMode = ELMDonate;
		pAI->LicensingMode(&eMode);
		if (eMode == ELMEnterSerial && S_OK != pAI->License(NULL, NULL, NULL, NULL, &nDays) && nDays > 30)
		{
			a_pStates->SetErrorMessage(CMultiLanguageString::GetAuto(L"[0409]Evaluation period expired.\n\nSaving functionality will be restored after valid license is entered.[0405]Zkušební doba vypršela.\n\nUkládání souborů bude umožněno po vložení platné licence."));
			return CLASS_E_NOTLICENSED;
		}

		if (m_pStorageMgr == NULL)
		{
			RWCoCreateInstance(m_pStorageMgr, __uuidof(StorageManager));
		}

		TCHAR szName[256] = _T("");
		CDocumentName::GetDocName(a_pDocument, szName, itemsof(szName));

		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IStorageFilterWindowListener> pWindowListener;
		CComPtr<IEnumUnknowns> pFlts;
		CComPtr<IConfig> pSaveCfg;
		pIM->SaveOptionsGet(a_pDocument, &pSaveCfg, &pFlts, &pWindowListener);

		CComPtr<IStorageFilter> pFilter;
		static const GUID tSaveContext = {0xc9567d17, 0x78bb, 0x428d, {0x86, 0x8d, 0x46, 0x63, 0xff, 0x6d, 0xb4, 0xc}};
		m_pStorageMgr->FilterCreateInteractivelyUID(CComBSTR(szName), EFTCreateNew, a_hParent, pFlts, pSaveCfg, tSaveContext, CMultiLanguageString::GetAuto(L"[0409]Save document[0405]Uložit dokument"), pWindowListener, a_tLocaleID, &pFilter);
		if (pFilter == NULL)
			return S_FALSE;
		return pIM->Save(a_pDocument, pSaveCfg, pFilter);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


