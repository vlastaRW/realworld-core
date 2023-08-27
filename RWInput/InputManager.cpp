// InputManager.cpp : Implementation of CInputManager

#include "stdafx.h"
#include "RWInput.h"
#include "InputManager.h"

#include <StringParsing.h>
#include "DocumentBase.h"
#include <MultiLanguageString.h>


// CInputManager

STDMETHODIMP CInputManager::DocumentTypesEnum(IEnumUnknowns** a_ppDocumentTypes)
{
	return DocumentTypesEnumEx(NULL, a_ppDocumentTypes);
}

STDMETHODIMP CInputManager::DocumentTypesEnumEx(IUnknown* a_pBuilderSpec, IEnumUnknowns** a_ppDocumentTypes)
{
	try
	{
		*a_ppDocumentTypes = NULL;

		CComQIPtr<IDocumentBuilder> pBuilder(a_pBuilderSpec);
		CComQIPtr<IEnumUnknowns> pBuilders(a_pBuilderSpec);
		ULONG nBuilders = 0;
		if (pBuilders) pBuilders->Size(&nBuilders);
		CAutoVectorPtr<CComPtr<IDocumentBuilder> > cBuilders;
		if (nBuilders)
		{
			cBuilders.Allocate(nBuilders);
			for (ULONG j = 0; j < nBuilders; ++j)
				pBuilders->Get(j, __uuidof(IDocumentBuilder), reinterpret_cast<void**>(&cBuilders[j]));
		}
		else
		{
			nBuilders = 1;
			cBuilders.Allocate(1);
			cBuilders[0] = pBuilder;
		}

		HRESULT hRes;
		CComPtr<IEnumUnknownsInit> pEUInit;
		if (FAILED(hRes = RWCoCreateInstance(pEUInit, __uuidof(EnumUnknowns))))
			return hRes;

		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumUnknowns> pDecoders;
		if (pPIC) pPIC->InterfacesEnum(CATID_DocumentDecoder, __uuidof(IDocumentDecoder), 0, &pDecoders, NULL);
		ULONG nDecoders = 0;
		if (pDecoders) pDecoders->Size(&nDecoders);
		for (ULONG i = 0; i < nDecoders; ++i)
		{
			CComPtr<IDocumentDecoder> pDD;
			pDecoders->Get(i, __uuidof(IDocumentDecoder), reinterpret_cast<void**>(&pDD));
			if (a_pBuilderSpec == NULL || S_OK == pDD->IsCompatible(nBuilders, &(cBuilders.m_p->p)))
			{
				CComPtr<IDocumentType> pType;
				pDD->DocumentType(&pType);
				pEUInit->Insert(pType.p);
			}
		}

		*a_ppDocumentTypes = pEUInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocumentTypes == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CInputManager::DocumentCreate(IStorageFilter* a_pSource, IBlockOperations* a_pOwner, IDocument** a_ppDocument)
{
	return DocumentCreateEx(NULL, a_pSource, a_pOwner, a_ppDocument);
}

STDMETHODIMP CInputManager::DocumentCreateEx(IUnknown* a_pBuilderSpec, IStorageFilter* a_pSource, IBlockOperations* a_pOwner, IDocument** a_ppDocument)
{
	try
	{
		*a_ppDocument = NULL;
		CComObject<CDocumentBase>* pBase = NULL;
		CComObject<CDocumentBase>::CreateInstance(&pBase);
		CComPtr<IDocument> pDoc = pBase;
		pBase->LocationSet(a_pSource);
		HRESULT hRes = DocumentCreateData(a_pBuilderSpec, a_pSource, NULL, pBase);
		if (FAILED(hRes))
			return hRes;
		*a_ppDocument = pDoc.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocument == NULL || a_pSource == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CInputManager::DocumentCreateData(IUnknown* a_pBuilderSpec, IStorageFilter* a_pSource, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComPtr<IDataSrcDirect> pSrc;
		HRESULT hRes = a_pSource->SrcOpen(&pSrc);
		if (pSrc == NULL)
			return SUCCEEDED(hRes) ? E_FAIL : hRes;
		ULONG nData = 0;
		pSrc->SizeGet(&nData);
		CDirectInputLock cData(pSrc, nData);
		GUID tEncID = GUID_NULL;
		CComPtr<IConfig> pEncCfg;
		hRes = DocumentCreateDataEx(a_pBuilderSpec, nData, cData, a_pSource, a_bstrPrefix, a_pBase, &tEncID, &pEncCfg, NULL);
		if (SUCCEEDED(hRes) && a_pBase && (a_bstrPrefix == NULL || *a_bstrPrefix == L'\0') && !IsEqualGUID(tEncID, GUID_NULL))
			a_pBase->EncoderSet(tEncID, pEncCfg);
		if (SUCCEEDED(hRes) && a_pBase && (a_bstrPrefix == NULL || *a_bstrPrefix == L'\0') && a_pSource)
			a_pBase->LocationSet(a_pSource);
		return hRes;
	}
	catch (...)
	{
		return a_pSource == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <DocumentName.h>

STDMETHODIMP CInputManager::DocumentCreateDataEx(IUnknown* a_pBuilderSpec, ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pSource, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl)
{
	try
	{
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));

		std::vector<CComPtr<IDocumentBuilder> > cBuilders;
		if (a_pBuilderSpec == NULL)
		{
			CComPtr<IEnumUnknowns> pBuilders;
			if (pPIC) pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
			ULONG nBuilders = 0;
			pBuilders->Size(&nBuilders);
			for (ULONG i = 0; i < nBuilders; ++i)
			{
				CComPtr<IDocumentBuilder> pDB;
				pBuilders->Get(i, __uuidof(IDocumentBuilder), reinterpret_cast<void**>(&pDB));
				ULONG nPriority = EDPAverage;
				pDB->Priority(&nPriority);
				size_t j = 0;
				for (; j < cBuilders.size(); ++j)
				{
					ULONG nPriority2 = EDPAverage;
					cBuilders[j]->Priority(&nPriority2);
					if (nPriority2 < nPriority)
					{
						cBuilders.insert(cBuilders.begin()+j, pDB);
						break;
					}
				}
				if (j == cBuilders.size())
					cBuilders.push_back(pDB);
			}
		}
		else
		{
			CComQIPtr<IDocumentBuilder> pDB(a_pBuilderSpec);
			if (pDB)
			{
				cBuilders.push_back(pDB);
			}
			else
			{
				CComQIPtr<IEnumUnknowns> pBuilders(a_pBuilderSpec);
				ULONG nBuilders = 0;
				if (pBuilders) pBuilders->Size(&nBuilders);
				for (ULONG i = 0; i < nBuilders; ++i)
				{
					CComPtr<IDocumentBuilder> pDB;
					pBuilders->Get(i, __uuidof(IDocumentBuilder), reinterpret_cast<void**>(&pDB));
					cBuilders.push_back(pDB);
				}
			}
		}

		CComPtr<IEnumUnknowns> pDecoders;
		if (pPIC) pPIC->InterfacesEnum(CATID_DocumentDecoder, __uuidof(IDocumentDecoder), 0, &pDecoders, NULL);
		ULONG nDecoders = 0;
		pDecoders->Size(&nDecoders);
		std::vector<CComPtr<IDocumentDecoder> > cDecoders;
		for (ULONG i = 0; i < nDecoders; ++i)
		{
			CComPtr<IDocumentDecoder> pDD;
			pDecoders->Get(i, __uuidof(IDocumentDecoder), reinterpret_cast<void**>(&pDD));
			ULONG nPriority = EDPAverage;
			pDD->Priority(&nPriority);
			bool bInserted = false;
			for (std::vector<CComPtr<IDocumentDecoder> >::const_iterator j = cDecoders.begin(); j != cDecoders.end(); ++j)
			{
				ULONG nPriority2 = EDPAverage;
				(*j)->Priority(&nPriority2);
				if (nPriority > nPriority2)
				{
					cDecoders.insert(j, pDD);
					bInserted = true;
					break;
				}
			}
			if (!bInserted)
				cDecoders.push_back(pDD);
		}

		// guess document type from extension -> speed up the process
		CComBSTR bstrInputName;
		if (a_pSource)
		{
			CComQIPtr<IDocumentName> pDN(a_pSource);
			if (pDN)
				pDN->GetName(&bstrInputName);
			else
				a_pSource->ToText(NULL, &bstrInputName);
		}
		std::vector<CComPtr<IDocumentDecoder> >::const_iterator iToSkip = cDecoders.end();

		CComQIPtr<IDocument> pMainDoc(a_pBase);
		bool bLocked = false;
		try
		{
			if (pMainDoc.p) bLocked = SUCCEEDED(pMainDoc->WriteLock());

			// TODO: optimize location setting and set default encoder and its configuration
			if (a_pBase && (a_bstrPrefix == NULL || *a_bstrPrefix == L'\0'))
			{
				a_pBase->LocationSet(a_pSource);
			}

			if (bstrInputName && bstrInputName[0])
			{
				for (std::vector<CComPtr<IDocumentDecoder> >::const_iterator j = cDecoders.begin(); j != cDecoders.end(); ++j)
				{
					CComPtr<IDocumentType> pDocType;
					if (SUCCEEDED((*j)->DocumentType(&pDocType)) && pDocType != NULL)
					{
						if (S_OK == pDocType->MatchFilename(bstrInputName))
						{
							// the most probable decoder was found
							if (a_pEncoderID) *a_pEncoderID = GUID_NULL;
							HRESULT hRes = (*j)->Parse(a_nLen, a_pData, a_pSource, cBuilders.size(), &(cBuilders[0].p), a_bstrPrefix, a_pBase, a_pEncoderID, a_ppEncoderCfg, a_pControl);
							if (FAILED(hRes))
								a_pBase->DataBlockSet(a_bstrPrefix, NULL); // clear possible remains
							else
							{
								if (bLocked) pMainDoc->WriteUnlock();
								return hRes;
							}
							iToSkip = j;
							break;
						}
					}
				}
			}

			for (std::vector<CComPtr<IDocumentDecoder> >::const_iterator j = cDecoders.begin(); j != cDecoders.end(); ++j)
			{
				if (j == iToSkip)
					continue;

				if (a_pEncoderID) *a_pEncoderID = GUID_NULL;
				HRESULT hRes = (*j)->Parse(a_nLen, a_pData, a_pSource, cBuilders.size(), &(cBuilders[0].p), a_bstrPrefix, a_pBase, a_pEncoderID, a_ppEncoderCfg, a_pControl);
				if (FAILED(hRes))
					a_pBase->DataBlockSet(a_bstrPrefix, NULL); // clear possible remains
				else
				{
					if (bLocked) pMainDoc->WriteUnlock();
					return hRes;
				}
			}
		}
		catch (...) {}
		if (bLocked) pMainDoc->WriteUnlock();
		return E_RW_UNKNOWNINPUTFORMAT;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

struct lessCLSID
{
	bool operator()(GUID const& a_1, GUID const& a_2) const
	{
		return a_1.Data1 < a_2.Data1 ||
			(a_1.Data1 == a_2.Data1 && (a_1.Data2 < a_2.Data2 ||
			(a_1.Data2 == a_2.Data2 && (a_1.Data3 < a_2.Data3 ||
			(a_1.Data3 == a_2.Data3 && memcmp(a_1.Data4, a_2.Data4, sizeof a_1.Data4)<0)))));
	}
};

STDMETHODIMP CInputManager::GetCompatibleBuilders(ULONG a_nCount, IID const* a_aiidRequired, IEnumUnknowns** a_ppBuilders)
{
	try
	{
		*a_ppBuilders = NULL;

		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumGUIDs> pGUIDs;
		if (pPIC) pPIC->CLSIDsEnum(CATID_DocumentBuilder, 0, &pGUIDs);
		ULONG nGUIDs = 0;
		pGUIDs->Size(&nGUIDs);
		std::map<GUID, CComPtr<IDocumentBuilder>, lessCLSID> cBuilders;
		for (ULONG i = 0; i < nGUIDs; ++i)
		{
			GUID tID = GUID_NULL;
			pGUIDs->Get(i, &tID);
			CComPtr<IDocumentBuilder> pDB;
			RWCoCreateInstance(pDB, tID);
			if (pDB == NULL)
				continue;
			if (S_OK == pDB->HasFeatures(a_nCount, a_aiidRequired))
				cBuilders[tID] = pDB;
		}

		CComPtr<IEnumUnknownsInit> pOut;
		RWCoCreateInstance(pOut, __uuidof(EnumUnknowns));
		for (std::map<GUID, CComPtr<IDocumentBuilder>, lessCLSID>::const_iterator i = cBuilders.begin(); i != cBuilders.end(); ++i)
			if (FAILED(pOut->Insert(i->second)))
				return E_FAIL;

		*a_ppBuilders = pOut.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppBuilders ? E_UNEXPECTED : E_POINTER;
	}
}

class ATL_NO_VTABLE CWindowListenerTypeToConfig :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStorageFilterWindowListener
{
public:
	friend class CInputManager;

BEGIN_COM_MAP(CWindowListenerTypeToConfig)
	COM_INTERFACE_ENTRY(IStorageFilterWindowListener)
END_COM_MAP()

	// IStorageFilterWindowListener methods
public:
	STDMETHOD(LocatorTextChanged)(BSTR a_bstrText)
	{
		return S_OK;
	}

	STDMETHOD(DocumentChanged)(IDocumentType *a_pType)
	{
		try
		{
			for (CFormats::const_iterator i = m_aFormats.begin(); i != m_aFormats.end(); ++i)
			{
				if (a_pType == i->second.p)
				{
					m_pConfig->ItemValuesSet(1, &(m_bstrFormatID.m_str), CConfigValue(i->first));
					return S_OK;
				}
			}
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(DefaultDocumentGet)(IDocumentType** a_ppType)
	{
		try
		{
			*a_ppType = NULL;
			if (m_pDefaultFormat == NULL)
				return S_FALSE;
			*a_ppType = m_pDefaultFormat;
			(*a_ppType)->AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppType == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

private:
	typedef std::vector<std::pair<GUID, CComPtr<IDocumentType> > > CFormats;

private:
	CFormats m_aFormats;
	CComPtr<IDocumentType> m_pDefaultFormat;
	CComPtr<IConfig> m_pConfig;
	CComBSTR m_bstrFormatID;
};

#include <Win32LangEx.h>
#include <ObserverImpl.h>

class ATL_NO_VTABLE CSaveConfigGUI :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigCustomGUI
{
public:

BEGIN_COM_MAP(CSaveConfigGUI)
	COM_INTERFACE_ENTRY(IConfigCustomGUI)
END_COM_MAP()

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid)
	{
		*a_pguid = CLSID_DocumentBase;
		return S_OK;
	}
	STDMETHOD(RequiresMargins)()
	{
		return S_FALSE;
	}
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
	{
		try
		{
			*a_nSizeX = *a_nSizeY = 0;
			if (a_pConfig == NULL)
				return S_OK;
			CComPtr<IConfig> pSubCfg;
			a_pConfig->SubConfigGet(CComBSTR(L"FFEncoder"), &pSubCfg);
			CComQIPtr<IConfigCustomGUI> pCfgGUI(pSubCfg);
			if (pCfgGUI == NULL)
				return S_OK;
			return pCfgGUI->MinSizeGet(pSubCfg, a_tLocaleID, a_eMode, a_nSizeX, a_nSizeY);
		}
		catch (...)
		{
			return E_NOTIMPL;
		}
	}
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
	{
		try
		{
			CComObject<CSaveCustomConfigWnd>* pWnd = NULL;
			CComObject<CSaveCustomConfigWnd>::CreateInstance(&pWnd);
			CComPtr<IChildWindow> pTmp = pWnd;

			pWnd->Create(a_hParent, a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_bParentBorder, a_pConfig);

			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CSaveCustomConfigWnd :
		public CComObjectRootEx<CComMultiThreadModel>,
		public CChildWindowImpl<CSaveCustomConfigWnd, IChildWindow>,
		public Win32LangEx::CLangIndirectDialogImpl<CSaveCustomConfigWnd>,
		public CObserverImpl<CSaveCustomConfigWnd, IConfigObserver, IUnknown*>
	{
	public:

	BEGIN_COM_MAP(CSaveCustomConfigWnd)
		COM_INTERFACE_ENTRY(IChildWindow)
	END_COM_MAP()

	BEGIN_DIALOG_EX(0, 0, 200, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CSaveCustomConfigWnd)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

		void OwnerNotify(TCookie, IUnknown*)
		{
		}
		void Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig)
		{
			m_tLocaleID = a_tLocaleID;
			m_pConfig = a_pConfig;

			Win32LangEx::CLangIndirectDialogImpl<CSaveCustomConfigWnd>::Create(a_hParent);
			if (!IsWindow()) throw E_FAIL;

			MoveWindow(a_prcPositon);
			SetWindowLong(GWL_ID, a_nCtlID);
			ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
		}

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			RECT rc;
			GetClientRect(&rc);
			CComPtr<IConfigWnd> pConfigWnd;
			RWCoCreateInstance(pConfigWnd, __uuidof(AutoConfigWnd));
			pConfigWnd->Create(m_hWnd, &rc, 100, m_tLocaleID, TRUE, ECWBMNothing);
			CComPtr<IConfig> pCfg;
			m_pConfig->SubConfigGet(CComBSTR(L"FFEncoder"), &pCfg);
			pConfigWnd->ConfigSet(pCfg, ECPMFull);

			return 1;
		}
		LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
		{
			HWND h = GetDlgItem(100);
			::SetWindowPos(h, NULL, 0, 0, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam), SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
			return 0;
		}

	private:
		CComPtr<IConfig> m_pConfig;
	};

};

STDMETHODIMP CInputManager::SaveOptionsGet(IDocument* a_pDocument, IConfig** a_ppSaveOptions, IEnumUnknowns** a_ppFormatFilters, IStorageFilterWindowListener** a_ppWindowListener)
{
	try
	{
		if (a_ppSaveOptions) *a_ppSaveOptions = NULL;
		if (a_ppFormatFilters) *a_ppFormatFilters = NULL;
		if (a_ppWindowListener) *a_ppWindowListener = NULL;

		CReadLock<IDocument> cLock(a_pDocument);

		GUID tEncoder = GUID_NULL;
		CComPtr<IConfig> pEncoder;
		a_pDocument->EncoderGet(&tEncoder, &pEncoder);

		if (IsEqualGUID(tEncoder, GUID_NULL))
		{
			pEncoder = NULL;
			CComBSTR bstrFav(L"[favorite]");
			float const fFav = 5.0f;
			FindBestEncoderEx(a_pDocument, 1, &(bstrFav.m_str), &fFav, &tEncoder, &pEncoder);
		}

		//if (m_iRoot == m_cData.end())
		//	return E_FAIL;
		//CComBSTR bstrAspectsMandatory;
		//CComBSTR bstrAspectsOptional;
		//m_iRoot->second.second->EncoderKeys(&bstrAspectsMandatory, &bstrAspectsOptional);

		std::vector<std::pair<GUID, CComPtr<IDocumentEncoder> > > cEncoders;
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumGUIDs> pEncGUIDs;
		CComPtr<IEnumUnknowns> pEncoders;
		pPIC->InterfacesEnum(CATID_DocumentEncoder, __uuidof(IDocumentEncoder), 0xffffffff, &pEncoders, &pEncGUIDs);
		ULONG nEncoders = 0;
		if (pEncoders) pEncoders->Size(&nEncoders);
		if (nEncoders == 0)
			return E_FAIL; // no compatible encoders

		bool bActFound = false;
		for (ULONG i = 0; i < nEncoders; ++i)
		{
			std::pair<GUID, CComPtr<IDocumentEncoder> > tEnc;
			tEnc.first = GUID_NULL;
			pEncGUIDs->Get(i, &tEnc.first);
			pEncoders->Get(i, __uuidof(IDocumentEncoder), reinterpret_cast<void**>(&tEnc.second));
			CComBSTR bstrAspects;
			if (!IsEqualGUID(tEnc.first, GUID_NULL) && tEnc.second && S_OK == tEnc.second->CanSerialize(a_pDocument, &bstrAspects))
			{
				cEncoders.push_back(tEnc);
				if (IsEqualGUID(tEnc.first, tEncoder))
					bActFound = true;
			}
		}
		CComPtr<IEnumUnknownsInit> pFFs;
		RWCoCreateInstance(pFFs, __uuidof(EnumUnknowns));
		//if (cEncoders.size() == 1)
		//{
		//	CComPtr<IDocumentType> pDocType;
		//	cEncoders[0].second->DocumentType(&pDocType);
		//	CComPtr<IConfig> pCfg;
		//	cEncoders[0].second->DefaultConfig(&pCfg);
		//	pFFs->Insert(pDocType);
		//	if (a_ppFormatFilters) *a_ppFormatFilters = pFFs.Detach();
		//	if (a_ppSaveOptions) *a_ppSaveOptions = pCfg.Detach();
		//	return S_OK;
		//}

		CComPtr<IConfigWithDependencies> pSO;
		RWCoCreateInstance(pSO, __uuidof(ConfigWithDependencies));
		CComPtr<ISubConfigSwitch> pSwitch;
		RWCoCreateInstance(pSwitch, __uuidof(SubConfigSwitch));
		CComBSTR cCFGID_FFSWITCH(L"FFEncoder");
		pSO->ItemIns1ofN(cCFGID_FFSWITCH, CMultiLanguageString::GetAuto(L"[0409]File format[0405]Formát souboru"), CMultiLanguageString::GetAuto(L"[0409]File format[0405]Formát souboru"), CConfigValue(cEncoders.empty() ? GUID_NULL : cEncoders[0].first), pSwitch);

		CComObject<CWindowListenerTypeToConfig>* pTTC = NULL;
		CComObject<CWindowListenerTypeToConfig>::CreateInstance(&pTTC);
		CComPtr<IStorageFilterWindowListener> pTTC2 = pTTC;
		pTTC->m_bstrFormatID = cCFGID_FFSWITCH;

		for (std::vector<std::pair<GUID, CComPtr<IDocumentEncoder> > >::const_iterator i = cEncoders.begin(); i != cEncoders.end(); ++i)
		{
			std::pair<GUID, CComPtr<IDocumentType> > tFmt;
			tFmt.first = i->first;
			i->second->DocumentType(&(tFmt.second));
			pTTC->m_aFormats.push_back(tFmt);
			if (bActFound && IsEqualGUID(tFmt.first, tEncoder))
			{
				pTTC->m_pDefaultFormat = tFmt.second;
			}
			CComPtr<IConfig> pCfg;
			i->second->DefaultConfig(&pCfg);
			pFFs->Insert(tFmt.second);
			if (pCfg) pSwitch->ItemInsert(CConfigValue(i->first), pCfg);
			CComPtr<ILocalizedString> pTypeName;
			tFmt.second->TypeNameGet(NULL, &pTypeName);
			pSO->ItemOptionAdd(cCFGID_FFSWITCH, CConfigValue(i->first), pTypeName, 0, NULL);
		}
		CComObject<CSaveConfigGUI>* pCG = NULL;
		CComObject<CSaveConfigGUI>::CreateInstance(&pCG);
		CComPtr<IConfigCustomGUI> pCustGUI = pCG;
		pSO->Finalize(pCustGUI);
		if (bActFound)
		{
			pSO->ItemValuesSet(1, &(cCFGID_FFSWITCH.m_str), CConfigValue(tEncoder));
			if (pEncoder)
			{
				CComPtr<IConfig> pSub;
				pSO->SubConfigGet(cCFGID_FFSWITCH, &pSub);
				if (pSub)
					CopyConfigValues(pSub, pEncoder);
			}
		}
		pTTC->m_pConfig = pSO;
		if (a_ppSaveOptions) *a_ppSaveOptions = pSO.Detach();
		if (a_ppFormatFilters) *a_ppFormatFilters = pFFs.Detach();
		if (a_ppWindowListener) *a_ppWindowListener = pTTC2.Detach();
		return S_OK;

		//return m_iRoot->second.second->SaveOptionsGet(a_ppSaveOptions, a_ppFormatFilters, a_ppWindowListener);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CInputManager::SaveEncoder(IConfig* a_pSaveOptions, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg)
{
	try
	{
		if (a_pEncoderID) *a_pEncoderID = GUID_NULL;
		if (a_ppEncoderCfg) *a_ppEncoderCfg = NULL;
		if (a_pSaveOptions == NULL)
			return E_RW_INVALIDPARAM;
		CComBSTR cCFGID_FFSWITCH(L"FFEncoder");
		CConfigValue cVal;
		a_pSaveOptions->ItemValueGet(cCFGID_FFSWITCH, &cVal);
		if (a_pEncoderID) *a_pEncoderID = cVal;
		if (a_ppEncoderCfg) a_pSaveOptions->SubConfigGet(cCFGID_FFSWITCH, a_ppEncoderCfg);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CInputManager::Save(IDocument* a_pDocument, IConfig* a_pSaveOptions, IStorageFilter* a_pSaveCopyAsDestination)
{
	try
	{
		HRESULT hRes = E_FAIL;

		//CReadLock<IDocument> cLock(a_pDocument);

		CComPtr<IConfig> pOptions(a_pSaveOptions);

		CComPtr<IConfig> pConfig;
		CLSID tEncoderID = GUID_NULL;
		if (pOptions == NULL)
		{
			a_pDocument->EncoderGet(&tEncoderID, &pConfig);
			if (IsEqualGUID(tEncoderID, GUID_NULL))
				SaveOptionsGet(a_pDocument, &pOptions, NULL, NULL);
		}
		if (IsEqualGUID(tEncoderID, GUID_NULL) && pOptions)
		{
			CComBSTR cCFGID_FFSWITCH(L"FFEncoder");
			CConfigValue cVal;
			pOptions->ItemValueGet(cCFGID_FFSWITCH, &cVal);
			tEncoderID = cVal;
			pOptions->SubConfigGet(cCFGID_FFSWITCH, &pConfig);
		}
		if (IsEqualGUID(tEncoderID, GUID_NULL))
			return E_FAIL;

		CComPtr<IDocumentEncoder> pEnc;
		RWCoCreateInstance(pEnc, tEncoderID);
		if (pEnc == NULL)
			return E_FAIL;

		CComPtr<IStorageFilter> pFlt;
		if (a_pSaveCopyAsDestination)
			pFlt = a_pSaveCopyAsDestination;
		else
			a_pDocument->LocationGet(&pFlt);
		CComPtr<IDataDstStream> pDst;
		if (pFlt)
			pFlt->DstOpen(&pDst);
		if (pDst == NULL)
			return E_RW_INVALIDPARAM;

		hRes = pEnc->Serialize(a_pDocument, pConfig, pDst, pFlt, NULL);

		pDst->Close();

		if (SUCCEEDED(hRes) && a_pSaveCopyAsDestination == NULL)
			a_pDocument->ClearDirty();

		return hRes;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

class CBestEncoderFinder :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumEncoderAspects
{
public:
	void Init(IDocument* a_pDoc)
	{
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumUnknowns> pEncoders;
		CComPtr<IEnumGUIDs> pGUIDs;
		pPIC->InterfacesEnum(CATID_DocumentEncoder, __uuidof(IDocumentEncoder), 0, &pEncoders, &pGUIDs);
		ULONG nEncoders = 0;
		pEncoders->Size(&nEncoders);
		for (ULONG i = 0; i < nEncoders; ++i)
		{
			SEncInfo tInfo;
			tInfo.fScore = 0.0f;
			tInfo.tID = GUID_NULL;
			pGUIDs->Get(i, &tInfo.tID);
			pEncoders->Get(i, &tInfo.pEnc);
			if (tInfo.pEnc && S_OK == tInfo.pEnc->CanSerialize(a_pDoc, &tInfo.bstrAsp))
				m_cEncoders.push_back(tInfo);
		}
	}
	bool Best(GUID* a_pID, IConfig** a_ppCfg) const
	{
		if (m_cEncoders.empty())
			return false;
		std::vector<SEncInfo>::const_iterator iBest = m_cEncoders.begin();
		for (std::vector<SEncInfo>::const_iterator i = iBest+1; i != m_cEncoders.end(); ++i)
			if (i->fScore > iBest->fScore)
				iBest = i;
		if (a_pID)
			*a_pID = iBest->tID;
		if (a_ppCfg)
			iBest->pEnc->DefaultConfig(a_ppCfg);
		return true;
	}

	BEGIN_COM_MAP(CBestEncoderFinder)
		COM_INTERFACE_ENTRY(IEnumEncoderAspects)
	END_COM_MAP()

	// IEnumEncoderAspects methods
public:
	STDMETHOD(Range)(ULONG* a_pBegin, ULONG* a_pCount)
	{
		return S_OK;
	}
	STDMETHOD(Consume)(ULONG a_nBegin, ULONG a_nCount, BSTR const* a_abstrID, float const* a_afWeight)
	{
		try
		{
			for (; a_nCount > 0; --a_nCount, ++a_abstrID, ++a_afWeight)
			{
				if (*a_abstrID == NULL)
					continue;
				if (SysStringLen(*a_abstrID) == 38 && (*a_abstrID)[0] == L'{' && (*a_abstrID)[37] == L'}')
				{
					GUID tID;
					if (GUIDFromString((*a_abstrID)+1, &tID))
					{
						for (std::vector<SEncInfo>::iterator i = m_cEncoders.begin(); i != m_cEncoders.end(); ++i)
							if (IsEqualGUID(i->tID, tID))
							{
								i->fScore += *a_afWeight;
								break;
							}
					}
					continue;
				}
				if (wcscmp(*a_abstrID, L"[favorite]") == 0)
				{
					CComPtr<IGlobalConfigManager> pGCM;
					RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
					CComPtr<IConfig> pCfg;
					if (pGCM) pGCM->Config(__uuidof(InputManager), &pCfg);
					CConfigValue cFavs;
					if (pCfg) pCfg->ItemValueGet(CComBSTR(L"Favorites"), &cFavs);
					if (cFavs.TypeGet() == ECVTString)
					{
						float f = 1.0f;
						LPCOLESTR psz = cFavs;
						while (*psz)
						{
							GUID t = GUID_NULL;
							if (!GUIDFromString(psz, &t))
								break;
							for (std::vector<SEncInfo>::iterator i = m_cEncoders.begin(); i != m_cEncoders.end(); ++i)
								if (IsEqualGUID(i->tID, t))
								{
									i->fScore += f**a_afWeight;
									break;
								}
							f *= 0.99f;
							psz += 36;
							while (*psz == L' ') ++psz;
						}
					}
					continue;
				}
				for (std::vector<SEncInfo>::iterator i = m_cEncoders.begin(); i != m_cEncoders.end(); ++i)
					if (wcsstr(i->bstrAsp, (*a_abstrID)) != NULL)
						i->fScore += *a_afWeight;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	struct SEncInfo
	{
		GUID tID;
		CComPtr<IDocumentEncoder> pEnc;
		CComBSTR bstrAsp;
		float fScore;
	};

private:
	GUID m_tBestID;
	CComPtr<IConfig> m_pBestCfg;
	std::vector<SEncInfo> m_cEncoders;
};

STDMETHODIMP CInputManager::FindBestEncoderEx(IDocument* a_pDocument, ULONG a_nExtra, BSTR const* a_abstrIDs, float const* a_afWeights, GUID* a_pEncID, IConfig** a_ppEncCfg)
{
	try
	{
		if (a_pEncID) *a_pEncID = GUID_NULL;
		if (a_ppEncCfg) *a_ppEncCfg = NULL;
		CComObjectStackEx<CBestEncoderFinder> cFinder;
		cFinder.Init(a_pDocument);
		a_pDocument->EncoderAspects(&cFinder);
		if (a_nExtra)
			cFinder.Consume(0, a_nExtra, a_abstrIDs, a_afWeights);
		a_pDocument->EncoderAspects(&cFinder);
		return cFinder.Best(a_pEncID, a_ppEncCfg) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CInputManager::Name(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Formats[0405]Formáty");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CInputManager::Description(ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		*a_ppDesc = new CMultiLanguageString(L"[0409]Selected file formats will be used for newly created documents.[0405]Vybrané souborové formáty budou použity pro nově vytvořené dokumenty.");
		return S_OK;
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CInputManager::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		InitConfig();
		(*a_ppConfig = m_pConfig)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

#include <ConfigCustomGUIImpl.h>

static OLECHAR const CFGID_FORMATS_FAVORITES[] = L"Favorites";

class ATL_NO_VTABLE CConfigGUIFormatsDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIFormatsDlg>,
	public CDialogResize<CConfigGUIFormatsDlg>
{
public:
	CConfigGUIFormatsDlg() : m_bUpdating(false)
	{
	}
	enum
	{
		IDC_SELECTED = 100,
		IDC_SELECTED_LABEL,
		IDC_OTHER,
		IDC_OTHER_LABEL,
		IDC_MOVE_UP,
		IDC_MOVE_DOWN,
		IDC_ADD,
		IDC_REMOVE,
	};

	BEGIN_DIALOG_EX(0, 0, 219, 101, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Favorite formats:[0405]Oblíbené formáty:"), IDC_SELECTED_LABEL, 0, 0, 77, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SELECTED, WC_LISTVIEW, /*LVS_LIST | */LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_ALIGNLEFT | LVS_NOCOLUMNHEADER | WS_BORDER | WS_TABSTOP | WS_VISIBLE, 0, 12, 77, 88, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Up[0405]Nahoru"), IDC_MOVE_UP, 84, 12, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_PUSHBUTTON(_T("[0000]<-"), IDC_ADD, 84, 30, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_PUSHBUTTON(_T("[0000]->"), IDC_REMOVE, 84, 48, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Down[0405]Dolů"), IDC_MOVE_DOWN, 84, 66, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Secondary formats:[0405]Vedlejší formáty:"), IDC_OTHER_LABEL, 141, 0, 77, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_OTHER, WC_LISTVIEW, /*LVS_LIST | */LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_ALIGNLEFT | LVS_NOCOLUMNHEADER | LVS_SORTASCENDING | WS_BORDER | WS_TABSTOP | WS_VISIBLE, 141, 12, 77, 88, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIFormatsDlg)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIFormatsDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIFormatsDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_MOVE_UP, BN_CLICKED, OnMoveUp)
		COMMAND_HANDLER(IDC_MOVE_DOWN, BN_CLICKED, OnMoveDown)
		COMMAND_HANDLER(IDC_ADD, BN_CLICKED, OnAdd)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, OnRemove)
		NOTIFY_HANDLER(IDC_SELECTED, LVN_ITEMCHANGED, OnSelChange)
		NOTIFY_HANDLER(IDC_OTHER, LVN_ITEMCHANGED, OnSelChange)
		NOTIFY_HANDLER(IDC_SELECTED, LVN_ITEMACTIVATE, OnSelActivate)
		NOTIFY_HANDLER(IDC_OTHER, LVN_ITEMACTIVATE, OnOthActivate)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIFormatsDlg)
		DLGRESIZE_CONTROL(IDC_SELECTED_LABEL, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_SELECTED, DLSZ_DIVSIZE_X(2)|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_MOVE_UP, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_ADD, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_REMOVE, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_MOVE_DOWN, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_OTHER_LABEL, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_OTHER, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2)|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIFormatsDlg)
		CONFIGITEM_CONTEXTHELP(IDC_SELECTED, CFGID_FORMATS_FAVORITES)
		//CONFIGITEM_CONTEXTHELP_IDS(IDC_APPOPT_UNDOMODE_GRP, IDS_CFGID_UNDOMODE_DESC)
		//CONFIGITEM_RADIO(IDC_APPOPT_UNDODEFAULT, CFGID_UNDOMODE, EUMDefault)
	END_CONFIGITEM_MAP()


	void ExtraInitDialog()
	{
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumGUIDs> pGUIDs;
		pPIC->CLSIDsEnum(CATID_DocumentEncoder, 0, &pGUIDs);
		ULONG n = 0;
		if (pGUIDs) pGUIDs->Size(&n);
		for (ULONG i = 0; i < n; ++i)
		{
			GUID t = GUID_NULL;
			if (FAILED(pGUIDs->Get(i, &t)))
				continue;
			CComPtr<IDocumentEncoder> p;
			RWCoCreateInstance(p, t);
			if (p)
				m_cEncoders[t] = p;
		}

		m_wndSelected = GetDlgItem(IDC_SELECTED);
		m_wndSelected.AddColumn(_T(""), 0);
		m_wndSelected.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		m_wndOther = GetDlgItem(IDC_OTHER);
		m_wndOther.AddColumn(_T(""), 0);
		m_wndOther.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		m_bstrLastFavs = L"dummy";
	}
	void ExtraConfigNotify()
	{
		CConfigValue cFavs;
		M_Config()->ItemValueGet(CComBSTR(CFGID_FORMATS_FAVORITES), &cFavs);
		if (cFavs.TypeGet() != ECVTString || m_bstrLastFavs == cFavs.operator BSTR())
			return;
		m_bUpdating = true;
		m_wndSelected.DeleteAllItems();
		m_wndOther.DeleteAllItems();
		std::vector<GUID> cNewSel;
		LPCOLESTR psz = cFavs;
		while (*psz)
		{
			GUID t = GUID_NULL;
			if (!GUIDFromString(psz, &t) || m_cEncoders.find(t) == m_cEncoders.end())
				break;
			cNewSel.push_back(t);
			psz += 36;
			while (*psz == L' ') ++psz;
		}
		for (std::vector<GUID>::const_iterator i = cNewSel.begin(); i != cNewSel.end(); ++i)
		{
			IDocumentEncoder* pEnc = m_cEncoders[*i];
			CComPtr<IDocumentType> pDocType;
			if (pEnc) pEnc->DocumentType(&pDocType);
			CComPtr<ILocalizedString> pTypeName;
			if (pDocType) pDocType->TypeNameGet(NULL, &pTypeName);
			CComBSTR bstrTypeName;
			if (pTypeName) pTypeName->GetLocalized(m_tLocaleID, &bstrTypeName);
			int iAdded = m_wndSelected.AddItem(i-cNewSel.begin(), 0, bstrTypeName);
			m_wndSelected.SetItemData(iAdded, reinterpret_cast<DWORD_PTR>(&m_cEncoders.find(*i)->first));
		}
		if (!cNewSel.empty())
			m_wndSelected.SelectItem(0);
		for (CEncoders::const_iterator i = m_cEncoders.begin(); i != m_cEncoders.end(); ++i)
		{
			bool bFound = false;
			for (std::vector<GUID>::const_iterator j = cNewSel.begin(); !bFound && j != cNewSel.end(); ++j)
				bFound = IsEqualGUID(*j, i->first);
			if (bFound)
				continue;
			IDocumentEncoder* pEnc = i->second;
			CComPtr<IDocumentType> pDocType;
			if (pEnc) pEnc->DocumentType(&pDocType);
			CComPtr<ILocalizedString> pTypeName;
			if (pDocType) pDocType->TypeNameGet(NULL, &pTypeName);
			CComBSTR bstrTypeName;
			if (pTypeName) pTypeName->GetLocalized(m_tLocaleID, &bstrTypeName);
			int iAdded = m_wndOther.AddItem(m_wndOther.GetItemCount(), 0, bstrTypeName);
			m_wndOther.SetItemData(iAdded, reinterpret_cast<DWORD_PTR>(&i->first));
		}
		if (m_cEncoders.size() > cNewSel.size())
			m_wndOther.SelectItem(0);
		UpdateButtonState();
		UpdateColumnWidths();
		m_bUpdating = false;
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = CDialogResize<CConfigGUIFormatsDlg>::OnSize(a_uMsg, a_wParam, a_lParam, a_bHandled);
		UpdateColumnWidths();
		return lRes;
	}

	LRESULT OnSelChange(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		if (!m_bUpdating)
			UpdateButtonState();
		return 0;
	}
	LRESULT OnSelActivate(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		OnRemove(0, 0, NULL, a_bHandled);
		return 0;
	}
	LRESULT OnOthActivate(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		OnAdd(0, 0, NULL, a_bHandled);
		return 0;
	}

	LRESULT OnMoveUp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int iSel = m_wndSelected.GetSelectedIndex();
		if (iSel > 0)
		{
			m_bUpdating = true;
			DWORD_PTR dw = m_wndSelected.GetItemData(iSel-1);
			CComBSTR bstr;
			m_wndSelected.GetItemText(iSel-1, 0, bstr.m_str);
			m_wndSelected.DeleteItem(iSel-1);
			int iAdded = m_wndSelected.AddItem(iSel, 0, bstr);
			m_wndSelected.SetItemData(iAdded, dw);
			UpdateButtonState();
			UpdateConfig();
			m_bUpdating = false;
		}
		return 0;
	}

	LRESULT OnMoveDown(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int iSel = m_wndSelected.GetSelectedIndex();
		if (iSel < m_wndSelected.GetItemCount()-1)
		{
			m_bUpdating = true;
			DWORD_PTR dw = m_wndSelected.GetItemData(iSel+1);
			CComBSTR bstr;
			m_wndSelected.GetItemText(iSel+1, 0, bstr.m_str);
			m_wndSelected.DeleteItem(iSel+1);
			int iAdded = m_wndSelected.AddItem(iSel, 0, bstr);
			m_wndSelected.SetItemData(iAdded, dw);
			UpdateButtonState();
			UpdateConfig();
			m_bUpdating = false;
		}
		return 0;
	}

	LRESULT OnAdd(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int iSel = m_wndOther.GetSelectedIndex();
		if (iSel >= 0)
		{
			m_bUpdating = true;
			DWORD_PTR dw = m_wndOther.GetItemData(iSel);
			CComBSTR bstr;
			m_wndOther.GetItemText(iSel, 0, bstr.m_str);
			m_wndOther.DeleteItem(iSel);
			int nCnt = m_wndOther.GetItemCount();
			if (nCnt)
				m_wndOther.SelectItem(iSel >= nCnt ? iSel-1 : iSel);
			int iAdded = m_wndSelected.AddItem(m_wndSelected.GetItemCount(), 0, bstr);
			m_wndSelected.SetItemData(iAdded, dw);
			m_wndSelected.SelectItem(iAdded);
			UpdateButtonState();
			UpdateConfig();
			UpdateColumnWidths();
			m_bUpdating = false;
		}
		return 0;
	}

	LRESULT OnRemove(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int iSel = m_wndSelected.GetSelectedIndex();
		if (iSel >= 0)
		{
			m_bUpdating = true;
			DWORD_PTR dw = m_wndSelected.GetItemData(iSel);
			CComBSTR bstr;
			m_wndSelected.GetItemText(iSel, 0, bstr.m_str);
			m_wndSelected.DeleteItem(iSel);
			int nCnt = m_wndSelected.GetItemCount();
			if (nCnt)
				m_wndSelected.SelectItem(iSel >= nCnt ? iSel-1 : iSel);
			int iAdded = m_wndOther.AddItem(m_wndOther.GetItemCount(), 0, bstr);
			m_wndOther.SetItemData(iAdded, dw);
			m_wndOther.SelectItem(iAdded);
			UpdateButtonState();
			UpdateConfig();
			UpdateColumnWidths();
			m_bUpdating = false;
		}
		return 0;
	}

private:
	struct lessGUID { bool operator()(GUID const& a_1, GUID const& a_2) const { return memcmp(&a_1, &a_2, sizeof GUID)<0; }};
	typedef std::map<GUID, CComPtr<IDocumentEncoder>, lessGUID> CEncoders;

private:
	void UpdateButtonState()
	{
		int iSelPos = m_wndSelected.GetSelectedIndex();
		int nSelCnt = m_wndSelected.GetItemCount();
		int iOthPos = m_wndOther.GetSelectedIndex();
		GetDlgItem(IDC_MOVE_UP).EnableWindow(iSelPos > 0);
		GetDlgItem(IDC_MOVE_DOWN).EnableWindow(iSelPos >=0 && iSelPos < nSelCnt-1);
		GetDlgItem(IDC_ADD).EnableWindow(iOthPos >= 0);
		GetDlgItem(IDC_REMOVE).EnableWindow(iSelPos >= 0);
	}
	void UpdateConfig()
	{
		int nCnt = m_wndSelected.GetItemCount();
		CComBSTR bstr(nCnt ? nCnt*36+(nCnt-1) : 0);
		for (int i = 0; i < nCnt; ++i)
		{
			GUID const* p = reinterpret_cast<GUID const*>(m_wndSelected.GetItemData(i));
			if (i)
				bstr[i*37-1] = L' ';
			StringFromGUID(*p, bstr.m_str+i*37);
		}
		m_bstrLastFavs = bstr;
		CComBSTR bstrID(CFGID_FORMATS_FAVORITES);
		M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(bstr));
	}
	void UpdateColumnWidths()
	{
		RECT rc;
		LVCOLUMN tCol;
		m_wndSelected.GetClientRect(&rc);
		tCol.mask = LVCF_WIDTH;
		tCol.cx = rc.right;
		m_wndSelected.SetColumn(0, &tCol);
		m_wndOther.GetClientRect(&rc);
		tCol.mask = LVCF_WIDTH;
		tCol.cx = rc.right;
		m_wndOther.SetColumn(0, &tCol);
	}

private:
	CEncoders m_cEncoders;
	CListViewCtrl m_wndSelected;
	CListViewCtrl m_wndOther;
	CComBSTR m_bstrLastFavs;
	bool m_bUpdating;
};


void CInputManager::InitConfig()
{
	if (m_pConfig)
		return;
	ObjectLock cLock(this);
	if (m_pConfig == NULL)
	{
		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));
		pCfgInit->ItemInsSimple(CComBSTR(L"Favorites"), CMultiLanguageString::GetAuto(L"[0409]Favorite formats[0405]Oblíbené formáty"), CMultiLanguageString::GetAuto(L"[0409]Selected formats will be used for newly created documents in the given order.[0405]Zvolené formáty budou nastaveny nově vytvořeným dokumentům ve zvoleném pořadí."), CConfigValue(L""), NULL, 0, NULL);
		CConfigCustomGUI<&CLSID_InputManager, CConfigGUIFormatsDlg>::FinalizeConfig(pCfgInit);
		m_pConfig.Attach(pCfgInit.Detach());
	}
}

