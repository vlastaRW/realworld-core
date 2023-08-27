
#pragma once

struct CDocumentUndoStep :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentUndoStep
{
	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(MemorySize)(ULONGLONG* UNREF(a_pnSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Merge)(IDocumentUndoStep* UNREF(a_pNextStep), DWORD UNREF(a_dwTimeDelta))
	{
		return E_NOTIMPL;
	}

BEGIN_COM_MAP(CDocumentUndoStep)
	COM_INTERFACE_ENTRY(IDocumentUndoStep)
END_COM_MAP()
};

template<class T>
class CUndoStepImpl : public T
{
public:
	template<typename T1>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		Init(a_tPar1);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4, T5 a_tPar5)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4, a_tPar5);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4, T5 a_tPar5, T6 a_tPar6)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4, a_tPar5, a_tPar6);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4, T5 a_tPar5, T6 a_tPar6, T7 a_tPar7)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4, a_tPar5, a_tPar6, a_tPar7);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4, T5 a_tPar5, T6 a_tPar6, T7 a_tPar7, T8 a_tPar8)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4, a_tPar5, a_tPar6, a_tPar7, a_tPar8);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4, T5 a_tPar5, T6 a_tPar6, T7 a_tPar7, T8 a_tPar8, T9 a_tPar9)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4, a_tPar5, a_tPar6, a_tPar7, a_tPar8, a_tPar9);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4, T5 a_tPar5, T6 a_tPar6, T7 a_tPar7, T8 a_tPar8, T9 a_tPar9, T10 a_tPar10)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4, a_tPar5, a_tPar6, a_tPar7, a_tPar8, a_tPar9, a_tPar10);
		a_pBase->UndoStep(pDUS);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
	static void Add(IDocumentBase* a_pBase, T1 a_tPar1, T2 a_tPar2, T3 a_tPar3, T4 a_tPar4, T5 a_tPar5, T6 a_tPar6, T7 a_tPar7, T8 a_tPar8, T9 a_tPar9, T10 a_tPar10, T11 a_tPar11, T12 a_tPar12, T13 a_tPar13, T14 a_tPar14, T15 a_tPar15, T16 a_tPar16)
	{
		CComObject<CUndoStepImpl<T> >* p = NULL;
		CComObject<CUndoStepImpl<T> >::CreateInstance(&p);
		CComPtr<IDocumentUndoStep> pDUS = p;
		p->Init(a_tPar1, a_tPar2, a_tPar3, a_tPar4, a_tPar5, a_tPar6, a_tPar7, a_tPar8, a_tPar9, a_tPar10, a_tPar11, a_tPar12, a_tPar13, a_tPar14, a_tPar15, a_tPar16);
		a_pBase->UndoStep(pDUS);
	}
};
