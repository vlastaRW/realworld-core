// DocumentBase.cpp : Implementation of CDocumentBase

#include "stdafx.h"
#include "DocumentBase.h"

#include <MultiLanguageString.h>


// CDocumentBase class

STDMETHODIMP CDocumentBase::BuilderID(CLSID* a_pguidBuilder)
{
	try
	{
		ObjectLock cLock(this);
		if (m_iRoot == m_cData.end())
		{
			*a_pguidBuilder = GUID_NULL;
			return S_FALSE;
		}

		return m_iRoot->second.second->BuilderID(a_pguidBuilder);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentBase::QueryFeatureInterface(REFIID a_iid, void** a_ppFeatureInterface)
{
	try
	{
		// TODO: ? lock
		// TODO: adapters
		if (m_iRoot == m_cData.end())
		{
			return E_NOINTERFACE;
		}

		return m_iRoot->second.second->QueryInterface(a_iid, a_ppFeatureInterface);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentBase::LocationGet(IStorageFilter** a_ppLocation)
{
	try
	{
		*a_ppLocation = NULL;

		CDocumentReadLock cLock(this);
		if (m_pLocationOverride)
		{
			(*a_ppLocation = m_pLocationOverride)->AddRef();
		}
		else if (m_pLocation)
		{
			(*a_ppLocation = m_pLocation)->AddRef();
		}

		return (*a_ppLocation) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

#include <DocumentName.h>

STDMETHODIMP CDocumentBase::RealRefreshEncoder()
{
	if (m_iRoot == m_cData.end())
		return E_FAIL;
	GUID tID = GUID_NULL;
	CComPtr<IConfig> pCfg;
	bool bNative = false;
	if (!IsEqualGUID(m_tEncoderID, GUID_NULL))
	{
		CComPtr<IDocumentEncoder> pMyEnc;
		RWCoCreateInstance(pMyEnc, m_tEncoderID);

		CComObjectStackEx<CEncoderCompatibilityChecker> cChecker;
		if (cChecker.Init(this, pMyEnc))
		{
			m_iRoot->second.second->Aspects(&cChecker);
			bNative = cChecker.Compatible();
		}
	}
	if (!bNative)
		FindBestEncoder(this, &tID, &pCfg);
	else
		tID = m_tEncoderID;
	if (IsEqualGUID(GUID_NULL, m_tEncoderIDOverride) ? !IsEqualGUID(tID, m_tEncoderID) : !IsEqualGUID(tID, m_tEncoderIDOverride))
	{
		if (!IsEqualGUID(tID, m_tEncoderID))
		{
			m_tEncoderIDOverride = tID;
			m_pEncoderCfgOverride = pCfg;
			if (m_pLocation)
			{
				CComPtr<IDocumentEncoder> pEnc;
				RWCoCreateInstance(pEnc, tID);
				CComPtr<IDocumentType> pDT;
				if (pEnc) pEnc->DocumentType(&pDT);
				CComBSTR bstrExt;
				if (pDT) pDT->DefaultExtensionGet(&bstrExt);
				if (bstrExt.Length())
				{
					m_pLocationOverride = NULL;
					CDocumentName::ChangeExtension(m_pLocation, bstrExt, &m_pLocationOverride);
				}
			}
		}
		else
		{
			m_tEncoderIDOverride = GUID_NULL;
			m_pEncoderCfgOverride = NULL;
			m_pLocationOverride = NULL;
		}
		Fire_Notify(EDCLocation|EDCFeatures);
	}
	return S_OK;
}

STDMETHODIMP CDocumentBase::DocumentCopy(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
{
	try
	{
		if (m_iRoot != m_cData.end())
		{
			CopyName(this, a_bstrPrefix, a_pBase);
			return m_iRoot->second.second->DataCopy(a_bstrPrefix, a_pBase, a_tPreviewEffectID, a_pPreviewEffect);
		}
		return S_FALSE; // document is empty
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::QuickInfo(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo)
{
	try
	{
		if (m_iRoot != m_cData.end())
		{
			return m_iRoot->second.second->QuickInfo(a_nInfoIndex, a_ppInfo);
		}
		return S_FALSE; // document is empty
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::IsDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

STDMETHODIMP CDocumentBase::ClearDirty()
{
	if (m_bDirty)
	{
		m_bDirty = false;
		Fire_Notify(EDCDirtyness);
	}
	return S_OK;
}

STDMETHODIMP CDocumentBase::LocationSet(IStorageFilter* a_pLocation)
{
	try
	{
		CComPtr<IStorageFilter> pOldLoc;
		{
			ObjectLock cLock(this);
			if (m_pLocation != a_pLocation)
			{
				pOldLoc.Attach(m_pLocation.Detach());
				m_pLocation = a_pLocation;
			}
			m_pLocationOverride = NULL;
		}
		CComPtr<IDocumentData> pRoot;
		{
			CDocumentReadLock cLock(this);
			if (m_iRoot != m_cData.end())
				pRoot = m_iRoot->second.second;
		}
		if (pRoot) pRoot->LocationChanged(pOldLoc);
		Fire_Notify(EDCLocation);
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentBase::EncoderAspects(IEnumEncoderAspects* a_pEnumAspects)
{
	try
	{
		CDocumentReadLock cLock(this);
		if (m_iRoot != m_cData.end())
			return m_iRoot->second.second->Aspects(a_pEnumAspects);
		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

// undo handling

STDMETHODIMP CDocumentBase::UndoModeSet(EUndoMode a_eMode)
{
	try
	{
		CDocumentWriteLock cLock(this);
		m_eMode = m_eDefaultMode == EUMDefault ? EUMDisabled  : (a_eMode == EUMDefault ? m_eDefaultMode : a_eMode);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::UndoModeGet(EUndoMode* a_pMode)
{
	try
	{
		*a_pMode = m_eMode;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::StepCount(ULONG* a_pnUndoSteps, ULONG* a_pnRedoSteps)
{
	try
	{
		CDocumentReadLock cLock(this);
		if (a_pnUndoSteps)
			*a_pnUndoSteps = m_cUndoSteps.size();
		if (a_pnRedoSteps)
			*a_pnRedoSteps = m_cRedoSteps.size();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::StepName(BOOL a_bRedo, ULONG a_nStep, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		CDocumentReadLock cLock(this);
		CUndoSteps& cSteps = a_bRedo ? m_cRedoSteps : m_cUndoSteps;
		if (a_nStep >= cSteps.size())
			return E_RW_INDEXOUTOFRANGE;
		*a_ppName = cSteps.at(a_nStep).pName;
		if (*a_ppName) (*a_ppName)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentBase::StepIcon(BOOL UNREF(a_bRedo), ULONG UNREF(a_nStep), ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon))
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocumentBase::Undo(ULONG a_nSteps)
{
	HRESULT hRes = S_OK;
	try
	{
		while (a_nSteps != 0)
		{
			bool bClearDirty = !m_bDirty;
			{
				CDocumentWriteLock cLock(this);
				if (m_nUndoLocks)
					return E_FAIL; // cannot undo while creating undo step
				if (m_cUndoSteps.size() < a_nSteps)
					return E_FAIL;
				m_eUndoState = EUSUndoing;
				m_cUndoSteps.front().pStep->Execute();
				HandleUndo(bClearDirty, false, m_cUndoSteps.front().pName);
				if (m_cUndoSteps.front().pStep) m_cUndoSteps.front().pStep->Release();
				if (m_cUndoSteps.front().pName) m_cUndoSteps.front().pName->Release();
				bClearDirty = m_cUndoSteps.front().bClearDirty;
				m_cUndoSteps.pop_front();
			}
			if (bClearDirty)
			{
				m_bDirty = false;
				Fire_Notify(EDCDirtyness);
			}
			a_nSteps--;
		}
	}
	catch (...)
	{
		hRes = E_UNEXPECTED;
	}
	m_eUndoState = EUSNormal;
	return hRes;
}

STDMETHODIMP CDocumentBase::Redo(ULONG a_nSteps)
{
	HRESULT hRes = S_OK;
	try
	{
		while (a_nSteps != 0)
		{
			bool bClearDirty = !m_bDirty;
			{
				CDocumentWriteLock cLock(this);
				if (m_nUndoLocks)
					return E_FAIL; // cannot redo while creating undo step
				if (m_cRedoSteps.size() < a_nSteps)
					return E_FAIL;
				m_eUndoState = EUSRedoing;
				m_cRedoSteps.front().pStep->Execute();
				HandleUndo(bClearDirty, false, m_cRedoSteps.front().pName);
				if (m_cRedoSteps.front().pStep) m_cRedoSteps.front().pStep->Release();
				if (m_cRedoSteps.front().pName) m_cRedoSteps.front().pName->Release();
				bClearDirty = m_cRedoSteps.front().bClearDirty;
				m_cRedoSteps.pop_front();
			}
			if (bClearDirty)
			{
				m_bDirty = false;
				Fire_Notify(EDCDirtyness);
			}
			a_nSteps--;
		}
	}
	catch (...)
	{
		hRes = E_UNEXPECTED;
	}
	m_eUndoState = EUSNormal;
	return hRes;
}

STDMETHODIMP CDocumentBase::StepStart(ILocalizedString* a_pName)
{
	try
	{
		CDocumentWriteLock cLock(this);
		++m_nUndoLocks;
		if (m_pUndoName == NULL || m_nUndoLocks == 1)
			m_pUndoName = a_pName;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::StepEnd()
{
	try
	{
		CDocumentWriteLock cLock(this);
		if (m_nUndoLocks == 0)
		{
			ATLASSERT(FALSE);
			return E_FAIL; // really bad
		}
		--m_nUndoLocks;
		//if (m_nUndoLocks == 0)
		//	m_pUndoName = NULL;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::StepAddExternal(IDocumentUndoStep* a_pStep)
{
	try
	{
		CDocumentWriteLock cLock(this);
		if (m_eMode != EUMDisabled)
			return UndoStep(a_pStep);
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <StringParsing.h>
struct lessGUID { bool operator()(GUID const& a_1, GUID const& a_2) const { return memcmp(&a_1, &a_2, sizeof GUID)<0; }};

void CDocumentBase::FindBestEncoder(IDocument* a_pDoc, GUID* a_pID, IConfig** a_ppCfg)
{
	CComPtr<IInputManager> pIM;
	RWCoCreateInstance(pIM, __uuidof(InputManager));
	CComBSTR bstrFav(L"[favorite]");
	static float const fWeight = 5.0f;
	pIM->FindBestEncoderEx(a_pDoc, 1, &(bstrFav.m_str), &fWeight, a_pID, a_ppCfg);
}


STDMETHODIMP CDocumentBase::CUndoGroup::Execute()
{
	try
	{
		for (CSteps::reverse_iterator i = m_cSteps.rbegin(); i != m_cSteps.rend(); i++)
		{
			HRESULT hRes = (*i)->Execute();
			if (FAILED(hRes))
				return hRes; // really bad
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentBase::CUndoGroup::MemorySize(ULONGLONG* a_pnSize)
{
	try
	{
		*a_pnSize = m_nMemorySize;
		return S_OK;
	}
	catch (...)
	{
		return a_pnSize ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentBase::CUndoGroup::Merge(IDocumentUndoStep* a_pNextStep, DWORD a_dwTimeDelta)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocumentBase::CUndoGroup::MergeOrAdd(IDocumentUndoStep* a_pNextStep)
{
	try
	{
		ULONGLONG nStepSize = 0;
		a_pNextStep->MemorySize(&nStepSize);
		if (!m_cSteps.empty())
		{
			HRESULT hRes = (*m_cSteps.rbegin())->Merge(a_pNextStep, 0);
			if (SUCCEEDED(hRes))
			{
				if (hRes == S_OK)
					m_nMemorySize += nStepSize;
				return S_OK;
			}
		}
		m_cSteps.push_back(a_pNextStep);
		a_pNextStep->AddRef();
		m_nMemorySize += nStepSize;
		return S_OK;
	}
	catch (...)
	{
		return a_pNextStep ? E_UNEXPECTED : E_POINTER;
	}
}

void CDocumentBase::CopyName(IDocument* a_pSource, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	if (a_pBase == NULL || (a_bstrPrefix && a_bstrPrefix[0]))
		return;
	CComPtr<IStorageFilter> pLoc;
	a_pBase->LocationGet(&pLoc);
	if (pLoc)
		return;
	OLECHAR szBuffer[MAX_PATH] = L"";
	CDocumentName::GetDocName(a_pSource, szBuffer, MAX_PATH);
	if (*szBuffer == L'\0')
		return;
	CComObject<CDocumentName>* pNew = NULL;
	CComObject<CDocumentName>::CreateInstance(&pNew);
	pLoc = pNew;
	pNew->Init(szBuffer);
	a_pBase->LocationSet(pLoc);
}
