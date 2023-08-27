
#pragma once

#include "RWInput.h"
#include <SubjectImpl.h>

struct IDocAndBase : public IDocument, public IDocumentBase {};

// CDocumentBase

class ATL_NO_VTABLE CDocumentBase : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentBase, &CLSID_DocumentBase>,
	public CSubjectImpl<CBlockOperationsImpl<CDocumentBase, IDocAndBase>, IDocumentObserver, ULONG>,
	public IDocumentUndo
{
	typedef CReadLock<IDocument> CDocumentReadLock;
	typedef CWriteLock<IDocument> CDocumentWriteLock;

public:
	CDocumentBase() : m_bDirty(false), m_nUndoLocks(0), m_eUndoState(EUSNormal),
		m_eMode(EUMDisabled), m_eDefaultMode(EUMDefault), m_nMaxUndo(5000000),
		m_bClearDirty(false), m_tEncoderID(GUID_NULL),
		m_tEncoderIDOverride(GUID_NULL), m_bUpdateQuickInfo(false)
	{
		m_iRoot = m_cData.end();
	}
	~CDocumentBase()
	{
		m_eMode = EUMDisabled;
		DataBlockSet(NULL, NULL);
		if (m_pUndoProxy)
			m_pUndoProxy->Init(NULL);
	}

DECLARE_NO_REGISTRY()

	static HRESULT WINAPI QIUndo(void* a_pThis, REFIID UNREF(a_iid), void** a_ppv, DWORD_PTR UNREF(a_dw))
	{
		*a_ppv = NULL;
		CDocumentBase* pThis = reinterpret_cast<CDocumentBase*>(a_pThis);
		if (pThis->m_eDefaultMode == EUMDefault)
			return E_NOINTERFACE;
		if (pThis->m_pUndoProxy == NULL)
		{
			ObjectLock cLock(pThis);
			if (pThis->m_pUndoProxy == NULL)
			{
				CComObject<CDocumentUndoProxy>::CreateInstance(&pThis->m_pUndoProxy.p);
				pThis->m_pUndoProxy.p->AddRef();
				pThis->m_pUndoProxy->Init(pThis);
			}
		}
		*a_ppv = reinterpret_cast<void*>(static_cast<IDocumentUndo*>(pThis->m_pUndoProxy.p));
		pThis->m_pUndoProxy.p->AddRef();
		return S_OK;
	}

BEGIN_COM_MAP(CDocumentBase)
	COM_INTERFACE_ENTRY(IDocument)
	COM_INTERFACE_ENTRY2(IBlockOperations, IDocument)
	COM_INTERFACE_ENTRY(IDocumentBase)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IDocumentUndo), 0, QIUndo)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IBlockOperations methods
public:
	void WriteFinished()
	{
		bool bDirty = m_bDirty;
		m_bRefreshEncoder = false;
		if (m_iRoot != m_cData.end())
			m_iRoot->second.second->WriteFinished();
		if (HandleUndoWithMerge(!bDirty))
		{
			Fire_Notify(EDCUndoLevel);
		}
		if (m_bUpdateQuickInfo)
			Fire_Notify(EDCQuickInfo);
		m_bUpdateQuickInfo = false;
		if (m_bRefreshEncoder)
			RealRefreshEncoder();
	}

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* a_pguidBuilder);
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface);
	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation);
	STDMETHOD(LocationSet)(IStorageFilter* a_pLocation);
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects);
	STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect);
	STDMETHOD(QuickInfo)(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo);
	STDMETHOD(IsDirty)();
	STDMETHOD(ClearDirty)();

	// IDocumentUndo methods
public:
	STDMETHOD(UndoModeSet)(EUndoMode a_eMode);
	STDMETHOD(UndoModeGet)(EUndoMode* a_pMode);
	STDMETHOD(StepCount)(ULONG* a_pnUndoSteps, ULONG* a_pnRedoSteps);
	STDMETHOD(StepName)(BOOL a_bRedo, ULONG a_nStep, ILocalizedString** a_ppName);
	STDMETHOD(StepIcon)(BOOL a_bRedo, ULONG a_nStep, ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(Undo)(ULONG a_nSteps);
	STDMETHOD(Redo)(ULONG a_nSteps);
	STDMETHOD(StepStart)(ILocalizedString* a_pName);
	STDMETHOD(StepEnd)();
	STDMETHOD(StepAddExternal)(IDocumentUndoStep* a_pStep);

	// IDocumentBase methods (indirect)
public:
	STDMETHOD(DataBlockIDs)(IEnumStrings** a_ppIDs)
	{
		try
		{
			*a_ppIDs = NULL;
			CDocumentReadLock cLock(this);
			CComPtr<IEnumStringsInit> pTmp;
			for (CDocumentData::const_iterator i = m_cData.begin(); i != m_cData.end(); ++i)
				pTmp->Insert(i->first);
			*a_ppIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppIDs ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(DataBlockGet)(BSTR a_bstrID, REFIID a_iid, void** a_ppFeatureInterface)
	{
		try
		{
			*a_ppFeatureInterface = NULL;
			CDocumentReadLock cLock(this);
			CComBSTR bstrID(a_bstrID);
			if (a_bstrID == NULL)
				bstrID = L"";
			CDocumentData::const_iterator i = m_cData.find(bstrID);
			if (i == m_cData.end())
				return E_RW_ITEMNOTFOUND;
			return i->second.second->QueryInterface(a_iid, a_ppFeatureInterface);
		}
		catch (...)
		{
			return a_ppFeatureInterface ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(DataBlockSet)(BSTR a_bstrID, IDocumentData* a_pBlock)
	{
		try
		{
			CDocumentWriteLock cLock(this);
			CComBSTR bstrID(a_bstrID);
			if (a_bstrID == NULL)
				bstrID = L"";
			if (a_pBlock == NULL)
			{
				// remove block
				CDocumentData::iterator i = m_cData.find(bstrID);
				if (i != m_cData.end())
				{
					i->second.second->RemovingBlock();
					i->second.first->Init(NULL);
					m_cData.erase(i);
					if (bstrID[0] == L'\0')
					{
						m_eMode = EUMDisabled;
						m_eDefaultMode = EUMDefault;
						m_iRoot = m_cData.end(); // document is invalid
					}
				}
				return S_OK;
			}

			CDocumentData::iterator i = m_cData.find(bstrID);
			if (i == m_cData.end())
			{
				// add block
				std::pair<CComPtr<CComObject<CDocumentBaseProxy> >, CComPtr<IDocumentData> >& sBlock = m_cData[bstrID];
				CComObject<CDocumentBaseProxy>::CreateInstance(&sBlock.first.p);
				sBlock.first.p->AddRef();
				sBlock.first.p->Init(this);
				sBlock.second = a_pBlock;
				a_pBlock->Init(sBlock.first, bstrID);
				if (bstrID[0] == L'\0')
				{
					m_iRoot = m_cData.find(bstrID);
					EUndoMode eUM = EUMDefault;
					a_pBlock->DefaultUndoMode(&eUM);
					m_eDefaultMode = eUM;
					if (m_eDefaultMode == EUMDefault)
						m_eMode = EUMDisabled;
				}
				return S_OK;
			}
			// replace block
			i->second.second->RemovingBlock();
			i->second.first->Init(NULL);
			i->second.first.Release();
			i->second.second.Release();
			CComObject<CDocumentBaseProxy>::CreateInstance(&i->second.first.p);
			i->second.first.p->AddRef();
			i->second.first.p->Init(this);
			i->second.second = a_pBlock;
			if (bstrID[0] == L'\0')
			{
				EUndoMode eUM = EUMDefault;
				a_pBlock->DefaultUndoMode(&eUM);
				m_eDefaultMode = eUM;
				if (m_eDefaultMode == EUMDefault)
					m_eMode = EUMDisabled;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(DataBlockDoc)(BSTR a_bstrID, IDocument** a_ppSubDocument)
	{
		try
		{
			*a_ppSubDocument = NULL;
			CDocumentReadLock cLock(this);
			CComBSTR bstrID(a_bstrID);
			if (a_bstrID == NULL)
				bstrID = L"";
			CDocumentData::const_iterator i = m_cData.find(bstrID);
			if (i == m_cData.end())
				return E_RW_ITEMNOTFOUND;
			CComObject<CSubDocument>* p = NULL;
			CComObject<CSubDocument>::CreateInstance(&p);
			CComPtr<IDocument> pSubDoc = p;
			p->Init(this, bstrID);
			*a_ppSubDocument = pSubDoc.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubDocument ? E_UNEXPECTED : E_POINTER;
		}
	}
	//STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)
	//STDMETHOD(LocationSet)(IStorageFilter* a_pLocation)
	STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig)
	{
		try
		{
			if (IsEqualGUID(m_tEncoderID, GUID_NULL) && IsEqualGUID(m_tEncoderIDOverride, GUID_NULL))
			{
				if (m_iRoot != m_cData.end())
				{
					FindBestEncoder(this, &m_tEncoderIDOverride, &m_pEncoderCfgOverride);
				}
			}
			if (!IsEqualGUID(m_tEncoderIDOverride, GUID_NULL))
			{
				*a_pEncoderID = m_tEncoderIDOverride;
				*a_ppConfig = NULL;
				if (m_pEncoderCfgOverride)
					m_pEncoderCfgOverride->DuplicateCreate(a_ppConfig);
				return S_OK;
			}
			*a_pEncoderID = m_tEncoderID;
			*a_ppConfig = NULL;
			if (m_pEncoderCfg)
				m_pEncoderCfg->DuplicateCreate(a_ppConfig);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig)
	{
		m_tEncoderID = a_tEncoderID;
		m_pEncoderCfg = a_pConfig;
		m_tEncoderIDOverride = GUID_NULL;
		m_pEncoderCfgOverride = NULL;
		m_pLocationOverride = NULL;
		return S_OK;
	}
	STDMETHOD(UndoEnabled)()
	{
		return m_eMode != EUMDisabled ? S_OK : S_FALSE;
	}
	STDMETHOD(UndoStep)(IDocumentUndoStep* a_pStep)
	{
		try
		{
			ATLASSERT(m_eMode != EUMDisabled);
			if (m_pUndoGroup == NULL)
			{
				CComObject<CUndoGroup>::CreateInstance(&m_pUndoGroup.p);
				m_pUndoGroup.p->AddRef();
			}
			m_pUndoGroup->MergeOrAdd(a_pStep);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(UndoStepCancel)()
	{
		try
		{
			ATLASSERT(m_eMode != EUMDisabled);
			if (m_pUndoGroup && S_OK == m_pUndoGroup->RemoveStep())
			{
				m_pUndoGroup = NULL;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetDirty)()
	{
		if (!m_bDirty)
		{
			m_bDirty = true;
			Fire_Notify(EDCDirtyness);
		}
		return S_OK;
	}
	STDMETHOD(RefreshEncoder)()
	{
		m_bRefreshEncoder = true;
		return S_OK;
	}
	STDMETHOD(RealRefreshEncoder)();
	STDMETHOD(UpdateQuickInfo)()
	{
		m_bUpdateQuickInfo = true;
		return S_OK;
	}

public:
	void ResetClearDirtyFlags()
	{
		for (CUndoSteps::iterator i = m_cUndoSteps.begin(); i != m_cUndoSteps.end(); ++i)
			i->bClearDirty = false;
		for (CUndoSteps::iterator i = m_cRedoSteps.begin(); i != m_cRedoSteps.end(); ++i)
			i->bClearDirty = false;
	}
	bool HandleUndoWithMerge(bool a_bClearDirty, bool a_bDeleteSteps = true, ILocalizedString* a_pName = NULL)
	{
		if (m_nUndoLocks)
		{
			m_bClearDirty |= a_bClearDirty;
			return false;
		}
		a_bClearDirty |= m_bClearDirty;
		m_bClearDirty = false;
		if (m_pUndoGroup == NULL)
			return false;

		switch (m_eUndoState)
		{
		case EUSNormal:
			{
				CComPtr<IDocumentUndoStep> pSingleStep;
				pSingleStep.Attach(m_pUndoGroup->GetSingleOrGroup());
				m_pUndoGroup = NULL;
				DWORD dwTick = GetTickCount();
				if (!m_cUndoSteps.empty() && SUCCEEDED(m_cUndoSteps.front().pStep->Merge(pSingleStep, dwTick == m_dwLastStep ? 1 : (dwTick-m_dwLastStep))))
				{
					m_pUndoName = NULL;
				}
				else
				{
					SUndoStep sStep = {pSingleStep.p, m_pUndoName.p, a_bClearDirty};
					if (a_bClearDirty) ResetClearDirtyFlags();
					m_cUndoSteps.push_front(sStep);
					pSingleStep.Detach();
					m_pUndoName.Detach();
				}
				m_dwLastStep = dwTick;
			}
			break;
		case EUSUndoing:
			{
				SUndoStep sStep = {m_pUndoGroup.p, a_pName, a_bClearDirty};
				if (a_bClearDirty) ResetClearDirtyFlags();
				m_cRedoSteps.push_front(sStep);
				m_pUndoGroup.Detach();
				if (a_pName) a_pName->AddRef();
			}
			break;
		case EUSRedoing:
			{
				SUndoStep sStep = {m_pUndoGroup.p, a_pName, a_bClearDirty};
				if (a_bClearDirty) ResetClearDirtyFlags();
				m_cUndoSteps.push_front(sStep);
				m_pUndoGroup.Detach();
				if (a_pName) a_pName->AddRef();
			}
			break;
		}

		if (a_bDeleteSteps)
			DeleteSteps();
		return true;
	}
	bool HandleUndo(bool a_bClearDirty, bool a_bDeleteSteps = true, ILocalizedString* a_pName = NULL)
	{
		if (m_nUndoLocks)
		{
			m_bClearDirty |= a_bClearDirty;
			return false;
		}
		a_bClearDirty |= m_bClearDirty;
		m_bClearDirty = false;
		if (m_pUndoGroup == NULL)
			return false;

		switch (m_eUndoState)
		{
		case EUSNormal:
			m_dwLastStep = GetTickCount();
			{
				SUndoStep sStep = {m_pUndoGroup.p, m_pUndoName.p, a_bClearDirty};
				if (a_bClearDirty) ResetClearDirtyFlags();
				m_cUndoSteps.push_front(sStep);
				m_pUndoGroup.Detach();
				m_pUndoName.Detach();
			}
			break;
		case EUSUndoing:
			{
				SUndoStep sStep = {m_pUndoGroup.p, a_pName, a_bClearDirty};
				if (a_bClearDirty) ResetClearDirtyFlags();
				m_cRedoSteps.push_front(sStep);
				m_pUndoGroup.Detach();
				if (a_pName) a_pName->AddRef();
			}
			break;
		case EUSRedoing:
			{
				SUndoStep sStep = {m_pUndoGroup.p, a_pName, a_bClearDirty};
				if (a_bClearDirty) ResetClearDirtyFlags();
				m_cUndoSteps.push_front(sStep);
				m_pUndoGroup.Detach();
				if (a_pName) a_pName->AddRef();
			}
			break;
		}

		if (a_bDeleteSteps)
			DeleteSteps();
		return true;
	}
	void DeleteSteps()
	{
		// remove all redo steps (they are now invalid)
		while (!m_cRedoSteps.empty())
		{
			if (m_cRedoSteps.back().pStep) m_cRedoSteps.back().pStep->Release();
			if (m_cRedoSteps.back().pName) m_cRedoSteps.back().pName->Release();
			m_cRedoSteps.pop_back();
		}
		// remove undo steps if neccessary
		switch (m_eMode)
		{
		case EUMDisabled:
			while (!m_cUndoSteps.empty())
			{
				if (m_cUndoSteps.back().pStep) m_cUndoSteps.back().pStep->Release();
				if (m_cUndoSteps.back().pName) m_cUndoSteps.back().pName->Release();
				m_cUndoSteps.pop_back();
			}
			break;
		case EUMSingleStep:
			while (m_cUndoSteps.size() > 1)
			{
				if (m_cUndoSteps.back().pStep) m_cUndoSteps.back().pStep->Release();
				if (m_cUndoSteps.back().pName) m_cUndoSteps.back().pName->Release();
				m_cUndoSteps.pop_back();
			}
			break;
		case EUMMemoryLimited:
			{
				ULONGLONG nMaxUndo = m_nMaxUndo; // 5MB by default for undo
				m_iRoot->second.second->MaximumUndoSize(&nMaxUndo);
				if (nMaxUndo > m_nMaxUndo)
					m_nMaxUndo = nMaxUndo;
				else
					nMaxUndo = m_nMaxUndo;

				size_t nSteps = 0;
				ULONGLONG nMemory = 0;
				for (CUndoSteps::const_iterator i = m_cUndoSteps.begin(); i != m_cUndoSteps.end(); ++i)
				{
					ULONGLONG n = 0;
					i->pStep->MemorySize(&n);
					nMemory += n;
					++nSteps;
					if (nMemory >= nMaxUndo)
						break;
				}
				while (m_cUndoSteps.size() > nSteps)
				{
					if (m_cUndoSteps.back().pStep) m_cUndoSteps.back().pStep->Release();
					if (m_cUndoSteps.back().pName) m_cUndoSteps.back().pName->Release();
					m_cUndoSteps.pop_back();
				}
			}
			break;
		}
	}

	void ClearUndo()
	{
		while (!m_cRedoSteps.empty())
		{
			if (m_cRedoSteps.back().pStep) m_cRedoSteps.back().pStep->Release();
			if (m_cRedoSteps.back().pName) m_cRedoSteps.back().pName->Release();
			m_cRedoSteps.pop_back();
		}
		while (!m_cUndoSteps.empty())
		{
			if (m_cUndoSteps.back().pStep) m_cUndoSteps.back().pStep->Release();
			if (m_cUndoSteps.back().pName) m_cUndoSteps.back().pName->Release();
			m_cUndoSteps.pop_back();
		}
		m_pUndoGroup = NULL;
	}

	static void CopyName(IDocument* a_pSource, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

private:
	struct SUndoStep
	{
		IDocumentUndoStep* pStep;
		ILocalizedString* pName;
		bool bClearDirty;
	};
	class CUndoSteps : public std::deque<SUndoStep>
	{
	public:
		~CUndoSteps()
		{
			while (size() != 0)
			{
				if (front().pStep) front().pStep->Release();
				if (front().pName) front().pName->Release();
				pop_front();
			}
		}
		void RemoveClearDirtyFlag()
		{
			for (iterator i = begin(); i != end(); ++i)
				i->bClearDirty = false;
		}
	};
	class CUndoGroup :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentUndoStep
	{
	public:
		// TODO: locks ? (should already be locked in document)
		CUndoGroup() : m_nMemorySize(0)
		{
		}
		~CUndoGroup()
		{
			for (CSteps::iterator i = m_cSteps.begin(); i != m_cSteps.end(); i++)
				(*i)->Release();
		}

	BEGIN_COM_MAP(CUndoGroup)
		COM_INTERFACE_ENTRY(IDocumentUndoStep)
	END_COM_MAP()

		// IDocumentUndoStep methods
	public:
		STDMETHOD(Execute)();
		STDMETHOD(MemorySize)(ULONGLONG* a_pnSize);
		STDMETHOD(Merge)(IDocumentUndoStep* a_pNextStep, DWORD a_dwTimeDelta);
		STDMETHOD(MergeOrAdd)(IDocumentUndoStep* a_pNextStep);

		IDocumentUndoStep* GetSingleOrGroup()
		{
			if (m_cSteps.size() == 1)
			{
				IDocumentUndoStep* p = m_cSteps[0];
				m_cSteps.clear();
				return p;
			}
			AddRef();
			return this;
		}

		HRESULT RemoveStep()
		{
			if (m_cSteps.empty()) return E_FAIL;
			m_cSteps.resize(m_cSteps.size()-1);
			return m_cSteps.empty() ? S_OK : S_FALSE;
		}

	private:
		typedef std::vector<IDocumentUndoStep*> CSteps;

	private:
		CSteps m_cSteps;
		ULONGLONG m_nMemorySize;
	};
	enum EUndoState
	{
		EUSNormal = 0,
		EUSUndoing,
		EUSRedoing
	};
	class ATL_NO_VTABLE CDocumentBaseProxy : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentBase
	{
	public:
		CDocumentBaseProxy() : m_pOwner(NULL)
		{
		}
		void Init(IDocumentBase* a_pOwner)
		{
			m_pOwner = a_pOwner;
		}

		static HRESULT WINAPI QISDUndo(void* a_pThis, REFIID UNREF(a_iid), void** a_ppv, DWORD_PTR UNREF(a_dw))
		{
			*a_ppv = NULL;
			CDocumentBaseProxy* pThis = reinterpret_cast<CDocumentBaseProxy*>(a_pThis);
			if (pThis->m_pOwner == NULL)
				return E_NOINTERFACE;
			return pThis->m_pOwner->QueryInterface(__uuidof(IDocumentUndo), a_ppv);
		}

	BEGIN_COM_MAP(CDocumentBaseProxy)
		COM_INTERFACE_ENTRY(IDocumentBase)
		COM_INTERFACE_ENTRY(IBlockOperations)
		COM_INTERFACE_ENTRY_FUNC(__uuidof(IDocumentUndo), 0, QISDUndo)
	END_COM_MAP()

		// IBlockOperations methods
	public:
		STDMETHOD(WriteLock)()	{ return m_pOwner ? m_pOwner->WriteLock() : E_UNEXPECTED; }
		STDMETHOD(WriteUnlock)(){ return m_pOwner ? m_pOwner->WriteUnlock() : E_UNEXPECTED; }
		STDMETHOD(ReadLock)()	{ return m_pOwner ? m_pOwner->ReadLock() : E_UNEXPECTED; }
		STDMETHOD(ReadUnlock)()	{ return m_pOwner ? m_pOwner->ReadUnlock() : E_UNEXPECTED; }

		// IDocumentBase methods
	public:
		STDMETHOD(DataBlockIDs)(IEnumStrings** a_ppIDs)						{ return m_pOwner ? m_pOwner->DataBlockIDs(a_ppIDs) : E_UNEXPECTED; }
		STDMETHOD(DataBlockGet)(BSTR a_bstrID, REFIID a_iid, void** a_ppFeatureInterface)	{ return m_pOwner ? m_pOwner->DataBlockGet(a_bstrID, a_iid, a_ppFeatureInterface) : E_UNEXPECTED; }
		STDMETHOD(DataBlockSet)(BSTR a_bstrID, IDocumentData* a_pBlock)		{ return m_pOwner ? m_pOwner->DataBlockSet(a_bstrID, a_pBlock) : E_UNEXPECTED; }
		STDMETHOD(DataBlockDoc)(BSTR a_bstrID, IDocument** a_ppSubDocument)	{ return m_pOwner ? m_pOwner->DataBlockDoc(a_bstrID, a_ppSubDocument) : E_UNEXPECTED; }
		STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)				{ return m_pOwner ? m_pOwner->LocationGet(a_ppLocation) : E_UNEXPECTED; }
		STDMETHOD(LocationSet)(IStorageFilter* a_pLocation)					{ return m_pOwner ? m_pOwner->LocationSet(a_pLocation) : E_UNEXPECTED; }
		STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig)	{ return m_pOwner ? m_pOwner->EncoderGet(a_pEncoderID, a_ppConfig) : E_UNEXPECTED; }
		STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig)	{ return m_pOwner ? m_pOwner->EncoderSet(a_tEncoderID, a_pConfig) : E_UNEXPECTED; }
		STDMETHOD(UndoEnabled)()											{ return m_pOwner ? m_pOwner->UndoEnabled() : S_FALSE; }
		STDMETHOD(UndoStep)(IDocumentUndoStep* a_pStep)						{ return m_pOwner ? m_pOwner->UndoStep(a_pStep) : E_UNEXPECTED; }
		STDMETHOD(SetDirty)()												{ return m_pOwner ? m_pOwner->SetDirty() : E_UNEXPECTED; }
		STDMETHOD(RefreshEncoder)()											{ return m_pOwner ? m_pOwner->RefreshEncoder() : E_UNEXPECTED; }
		STDMETHOD(UpdateQuickInfo)()										{ return m_pOwner ? m_pOwner->UpdateQuickInfo() : E_UNEXPECTED; }
		STDMETHOD(UndoStepCancel)()											{ return m_pOwner ? m_pOwner->UndoStepCancel() : E_UNEXPECTED; }


	private:
		IDocumentBase* m_pOwner;
	};

	class ATL_NO_VTABLE CDocumentUndoProxy : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentUndo
	{
	public:
		CDocumentUndoProxy() : m_pOwner(NULL)
		{
		}
		void Init(IDocumentUndo* a_pOwner)
		{
			m_pOwner = a_pOwner;
		}

	BEGIN_COM_MAP(CDocumentUndoProxy)
		COM_INTERFACE_ENTRY(IDocumentUndo)
	END_COM_MAP()

		// IDocumentUndo methods
	public:
		STDMETHOD(UndoModeSet)(EUndoMode a_eMode)										{ return m_pOwner ? m_pOwner->UndoModeSet(a_eMode) : E_UNEXPECTED; }
		STDMETHOD(UndoModeGet)(EUndoMode* a_pMode)										{ return m_pOwner ? m_pOwner->UndoModeGet(a_pMode) : E_UNEXPECTED; }
		STDMETHOD(StepCount)(ULONG* a_pnUndoSteps, ULONG* a_pnRedoSteps)				{ return m_pOwner ? m_pOwner->StepCount(a_pnUndoSteps, a_pnRedoSteps) : E_UNEXPECTED; }
		STDMETHOD(StepName)(BOOL a_bRedo, ULONG a_nStep, ILocalizedString** a_ppName)	{ return m_pOwner ? m_pOwner->StepName(a_bRedo, a_nStep, a_ppName) : E_UNEXPECTED; }
		STDMETHOD(StepIcon)(BOOL a_bRedo, ULONG a_nStep, ULONG a_nSize, HICON* a_phIcon){ return m_pOwner ? m_pOwner->StepIcon(a_bRedo, a_nStep, a_nSize, a_phIcon) : E_UNEXPECTED; }
		STDMETHOD(Undo)(ULONG a_nSteps)													{ return m_pOwner ? m_pOwner->Undo(a_nSteps) : E_UNEXPECTED; }
		STDMETHOD(Redo)(ULONG a_nSteps)													{ return m_pOwner ? m_pOwner->Redo(a_nSteps) : E_UNEXPECTED; }
		STDMETHOD(StepStart)(ILocalizedString* a_pName)									{ return m_pOwner ? m_pOwner->StepStart(a_pName) : E_UNEXPECTED; }
		STDMETHOD(StepEnd)()															{ return m_pOwner ? m_pOwner->StepEnd() : E_UNEXPECTED; }
		STDMETHOD(StepAddExternal)(IDocumentUndoStep* a_pStep)							{ return m_pOwner ? m_pOwner->StepAddExternal(a_pStep) : E_UNEXPECTED; }

	private:
		IDocumentUndo* m_pOwner;
	};

	class ATL_NO_VTABLE CSubDocument : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocument
	{
	public:
		CSubDocument() : m_pDoc(NULL)
		{
		}
		~CSubDocument()
		{
			if (m_pDoc)
				m_pDoc->Release();
		}
		void Init(CDocumentBase* a_pDoc, BSTR a_bstrID)
		{
			(m_pDoc = a_pDoc)->AddRef();
			m_bstrID = a_bstrID;
		}

		static HRESULT WINAPI QISDUndo(void* a_pThis, REFIID UNREF(a_iid), void** a_ppv, DWORD_PTR UNREF(a_dw))
		{
			*a_ppv = NULL;
			CSubDocument* pThis = reinterpret_cast<CSubDocument*>(a_pThis);
			if (pThis->m_pDoc == NULL)
				return E_NOINTERFACE;
			return pThis->m_pDoc->QueryInterface(__uuidof(IDocumentUndo), a_ppv);
		}

	BEGIN_COM_MAP(CSubDocument)
		COM_INTERFACE_ENTRY(IDocument)
		COM_INTERFACE_ENTRY(IBlockOperations)
		COM_INTERFACE_ENTRY_FUNC(__uuidof(IDocumentUndo), 0, QISDUndo)
		//COM_INTERFACE_ENTRY(IDocumentBase)
	END_COM_MAP()

		// IBlockOperations methods
	public:
		STDMETHOD(WriteLock)()	{ return static_cast<IDocument*>(m_pDoc)->WriteLock(); }
		STDMETHOD(WriteUnlock)(){ return static_cast<IDocument*>(m_pDoc)->WriteUnlock(); }
		STDMETHOD(ReadLock)()	{ return static_cast<IDocument*>(m_pDoc)->ReadLock(); }
		STDMETHOD(ReadUnlock)()	{ return static_cast<IDocument*>(m_pDoc)->ReadUnlock(); }

		// IDocumentBase methods
	public:
		//STDMETHOD(DataBlockIDs)(IEnumStrings** a_ppIDs)
		//{
		//	try
		//	{
		//		*a_ppIDs = NULL;
		//		CComPtr<IEnumStrings> pAll;
		//		m_pDoc->DataBlockIDs(&pAll);
		//		if (pAll == NULL)
		//			return S_FALSE;
		//		ULONG nAll = 0;
		//		pAll->Size(&nAll);
		//		CComPtr<IEnumStringsInit> pMy;
		//		RWCoCreateInstance(pMy, __uuidof(EnumStrings));
		//		for (ULONG i = 0; i < nAll; ++i)
		//		{
		//			CComBSTR bstr;
		//			pAll->Get(i, &bstr);
		//			if (bstr.m_str && wcsncmp(bstr, m_bstrID, m_bstrID.Length()) == 0)
		//				pMy->Insert(bstr);
		//		}
		//		*a_ppIDs = pMy.Detach();
		//		return S_OK;
		//	}
		//	catch (...)
		//	{
		//		return E_UNEXPECTED;
		//	}
		//}
		//STDMETHOD(DataBlockGet)(BSTR a_bstrID, REFIID a_iid, void** a_ppFeatureInterface)
		//{
		//	if (a_bstrID && wcsncmp(a_bstrID, m_bstrID, m_bstrID.Length()) == 0)
		//		return m_pDoc->DataBlockGet(a_bstrID, a_iid, a_ppFeatureInterface);
		//	return E_RW_ITEMNOTFOUND;
		//}
		//STDMETHOD(DataBlockSet)(BSTR a_bstrID, IDocumentData* a_pBlock)
		//{
		//	if (a_bstrID && wcsncmp(a_bstrID, m_bstrID, m_bstrID.Length()) == 0)
		//		return m_pDoc->DataBlockSet(a_bstrID, a_pBlock);
		//	return E_RW_ITEMNOTFOUND;
		//}
		//STDMETHOD(DataBlockDoc)(BSTR a_bstrID, IDocument** a_ppSubDocument)
		//{
		//	if (a_bstrID && wcsncmp(a_bstrID, m_bstrID, m_bstrID.Length()) == 0)
		//		return m_pDoc->DataBlockDoc(a_bstrID, a_ppSubDocument);
		//	return E_RW_ITEMNOTFOUND;
		//}
		STDMETHOD(EncoderGet)(CLSID* UNREF(a_pEncoderID), IConfig** UNREF(a_ppConfig))	{ return E_NOTIMPL; }
		STDMETHOD(EncoderSet)(REFCLSID UNREF(a_tEncoderID), IConfig* UNREF(a_pConfig))	{ return E_NOTIMPL; }
		STDMETHOD(UndoEnabled)()
		{
			return m_pDoc->UndoEnabled();
		}
		STDMETHOD(UndoStep)(IDocumentUndoStep* a_pStep)
		{
			return m_pDoc->UndoStep(a_pStep);
		}
		STDMETHOD(SetDirty)()
		{
			return m_pDoc->SetDirty();
		}
		STDMETHOD(UndoStepCancel)()
		{
			return m_pDoc->UndoStepCancel();
		}

		// IDocument methods
	public:
		STDMETHOD(BuilderID)(CLSID* a_pguidBuilder)
		{
			IDocumentData* p = m_pDoc->GetData(m_bstrID);
			return p ? p->BuilderID(a_pguidBuilder) : E_FAIL;
		}
		STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects)	
		{
			IDocumentData* p = m_pDoc->GetData(m_bstrID);
			return p ? p->Aspects(a_pEnumAspects) : E_FAIL;
		}
		STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
		{
			HRESULT hRes = m_pDoc->DataBlockGet(m_bstrID, a_iid, a_ppFeatureInterface);
			if (FAILED(hRes) && hRes != E_NOINTERFACE)
				return hRes;
			void* pOverride = *a_ppFeatureInterface;
			IDocumentData* pRoot = m_pDoc->GetData(CComBSTR(L""));
			if (pRoot == NULL)
				return hRes;
			HRESULT hResOver = pRoot->ComponentFeatureOverride(m_bstrID, a_iid, a_ppFeatureInterface);
			return pOverride != *a_ppFeatureInterface ? hResOver : hRes;
		}
		STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)
		{
			IDocumentData* pRoot = m_pDoc->GetData(CComBSTR(L""));
			if (pRoot == NULL)
				return E_UNEXPECTED;
			return pRoot->ComponentLocationGet(m_bstrID, m_pDoc->M_Location(), a_ppLocation);
		}
		STDMETHOD(LocationSet)(IStorageFilter* UNREF(a_pLocation))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
		{
			try
			{
				IDocumentData* p = m_pDoc->GetData(m_bstrID);
				CopyName(this, a_bstrPrefix, a_pBase);
				if (p) return p->DataCopy(a_bstrPrefix, a_pBase, a_tPreviewEffectID, a_pPreviewEffect);
				return S_FALSE;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(QuickInfo)(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo)
		{
			try
			{
				IDocumentData* p = m_pDoc->GetData(m_bstrID);
				if (p) return p->QuickInfo(a_nInfoIndex, a_ppInfo);
				return E_RW_ITEMNOTFOUND;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(IsDirty)()
		{
			return m_pDoc->IsDirty();
		}
		STDMETHOD(ClearDirty)()
		{
			return E_NOTIMPL;//m_pDoc->SetDirty(a_bDirty);
		}
		//STDMETHOD(SaveOptionsGet)(IConfig** a_ppSaveOptions, IEnumUnknowns** a_ppFormatFilters, IStorageFilterWindowListener** a_ppWindowListener)
		//{
		//	return E_NOTIMPL;
		//}
		//STDMETHOD(Save)(IConfig* a_pSaveOptions, IStorageFilter* a_pSaveCopyAsDestination)
		//{
		//	return E_NOTIMPL;
		//}
		STDMETHOD(ObserverIns)(IDocumentObserver* a_pObserver, TCookie a_tCookie) { return m_pDoc->ObserverIns(a_pObserver, a_tCookie); }
		STDMETHOD(ObserverDel)(IDocumentObserver* a_pObserver, TCookie a_tCookie) { return m_pDoc->ObserverDel(a_pObserver, a_tCookie); }

	private:
		CDocumentBase* m_pDoc;
		CComBSTR m_bstrID;
	};

	IDocumentData* GetData(CComBSTR const& a_bstrID)
	{
		CDocumentData::const_iterator i = m_cData.find(a_bstrID);
		if (i == m_cData.end())
			return NULL;
		return i->second.second;
	}
	IStorageFilter* M_Location() { return m_pLocation; }
	static void FindBestEncoder(IDocument* a_pDoc, GUID* a_pID, IConfig** a_ppCfg);

	class CEncoderCompatibilityChecker :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IEnumEncoderAspects
	{
	public:
		bool Init(IDocument* a_pDoc, IDocumentEncoder* a_pEnc)
		{
			if (a_pEnc == NULL || S_OK != a_pEnc->CanSerialize(a_pDoc, &m_bstrAsp))
				return false;
			m_bFound = false;
			return true;
		}
		bool Compatible() const { return m_bFound; }

		BEGIN_COM_MAP(CEncoderCompatibilityChecker)
			COM_INTERFACE_ENTRY(IEnumEncoderAspects)
		END_COM_MAP()

		// IEnumEncoderAspects methods
	public:
		STDMETHOD(Range)(ULONG* UNREF(a_pBegin), ULONG* UNREF(a_nCount)) { return S_OK; }
		STDMETHOD(Consume)(ULONG a_nBegin, ULONG a_nCount, BSTR const* a_abstrID, float const* a_afWeight)
		{
			if (wcsstr(m_bstrAsp, *a_abstrID) != NULL)
				m_bFound = true;
			return S_FALSE;
		}

	private:
		CComBSTR m_bstrAsp;
		bool m_bFound;
	};

private:
	typedef std::map<CComBSTR, std::pair<CComPtr<CComObject<CDocumentBaseProxy> >, CComPtr<IDocumentData> > > CDocumentData;
	typedef std::map<CComBSTR, std::pair<GUID, CComPtr<IConfig> > > CEncoderMap;

private:
	// data blocks
	CDocumentData m_cData;
	CDocumentData::const_iterator m_iRoot;

	// undo
	CComPtr<CComObject<CDocumentUndoProxy> > m_pUndoProxy;
	CUndoSteps m_cUndoSteps;
	CUndoSteps m_cRedoSteps;
	CComPtr<CComObject<CUndoGroup> > m_pUndoGroup;
	CComPtr<ILocalizedString> m_pUndoName;
	bool m_bClearDirty;
	ULONG m_nUndoLocks;
	EUndoMode m_eMode;
	EUndoMode m_eDefaultMode;
	EUndoState m_eUndoState;
	DWORD m_dwLastStep;
	ULONGLONG m_nMaxUndo;

	// location and document type
	bool m_bRefreshEncoder;
	CComPtr<IStorageFilter> m_pLocation;
	CLSID m_tEncoderID;
	CComPtr<IConfig> m_pEncoderCfg;

	CComPtr<IStorageFilter> m_pLocationOverride;
	CLSID m_tEncoderIDOverride;
	CComPtr<IConfig> m_pEncoderCfgOverride;

	CEncoderMap m_cBestEncoders;

	// dirty state
	bool m_bDirty;
	bool m_bUpdateQuickInfo;
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentBase), CDocumentBase)
