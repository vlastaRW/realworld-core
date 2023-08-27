
#pragma once

template<typename TSharedStateManager>
class ATL_NO_VTABLE CSharedStateUndo :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentUndoStep
{
public:
	static void SaveState(IDocumentUndo* a_pUndo, TSharedStateManager* a_pSSM, BSTR a_bstrSyncID, ISharedState* a_pState = NULL)
	{
		try
		{
			if (a_pSSM == NULL || a_bstrSyncID == NULL)
				return;
			CComPtr<ISharedState> pState(a_pState);
			if (pState == NULL)
				a_pSSM->StateGet(a_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			//if (pState == NULL)
			//	return;
			CComObject<CSharedStateUndo<TSharedStateManager> >* p = NULL;
			CComObject<CSharedStateUndo<TSharedStateManager> >::CreateInstance(&p);
			CComPtr<IDocumentUndoStep> pStep = p;
			p->Init(a_pUndo, a_pSSM, a_bstrSyncID, pState);
			a_pUndo->StepAddExternal(pStep);
		}
		catch (...)
		{
		}
	}
	static void SaveState(IDocument* a_pDoc, TSharedStateManager* a_pSSM, BSTR a_bstrSyncID, ISharedState* a_pState = NULL)
	{
		CComQIPtr<IDocumentUndo> pUndo(a_pDoc);
		if (pUndo) SaveState(pUndo, a_pSSM, a_bstrSyncID, a_pState);
	}
	void Init(IDocumentUndo* a_pUndo, TSharedStateManager* a_pSSM, BSTR a_bstrSyncID, ISharedState* a_pState)
	{
		m_pUndo = a_pUndo;
		m_pSSM = a_pSSM;
		m_bstrSyncID = a_bstrSyncID;
		m_pState = a_pState;
	}

BEGIN_COM_MAP(CSharedStateUndo)
	COM_INTERFACE_ENTRY(IDocumentUndoStep)
END_COM_MAP()

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		if (m_pUndo)
			CSharedStateUndo<TSharedStateManager>::SaveState(m_pUndo, m_pSSM, m_bstrSyncID);
		return m_pSSM->StateSet(m_bstrSyncID, m_pState);
	}
	STDMETHOD(MemorySize)(ULONGLONG *a_pnSize)
	{
		// TODO: implement correctly (~ using serializatio to text???)
		*a_pnSize = 256; // some value
		return S_OK;
	}
	STDMETHOD(Merge)(IDocumentUndoStep *a_pNextStep, DWORD a_dwTimeDelta)
	{
		return E_NOTIMPL; // TODO: implement and merge close states
	}

private:
	CComPtr<IDocumentUndo> m_pUndo;
	CComPtr<TSharedStateManager> m_pSSM;
	CComBSTR m_bstrSyncID;
	CComPtr<ISharedState> m_pState;
};

