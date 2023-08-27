
#pragma once

template<class T>
class CDocumentUndoOwnedImpl : public IDocumentUndo
{
	// T must implement IDocumentUndo* MainUndo();

	// IDocumentUndo methods
public:
	STDMETHOD(UndoModeSet)(EUndoMode a_eMode)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->UndoModeSet(a_eMode) : E_NOTIMPL;
	}
	STDMETHOD(UndoModeGet)(EUndoMode* a_pMode)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->UndoModeGet(a_pMode) : E_NOTIMPL;
	}
	STDMETHOD(StepCount)(ULONG* a_pnUndoSteps, ULONG* a_pnRedoSteps)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->StepCount(a_pnUndoSteps, a_pnRedoSteps) : E_NOTIMPL;
	}
	STDMETHOD(StepName)(BOOL a_bRedo, ULONG a_nStep, ILocalizedString** a_ppName)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->StepName(a_bRedo, a_nStep, a_ppName) : E_NOTIMPL;
	}
	STDMETHOD(StepIcon)(BOOL a_bRedo, ULONG a_nStep, ULONG a_nSize, HICON* a_phIcon)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->StepIcon(a_bRedo, a_nStep, a_nSize, a_phIcon) : E_NOTIMPL;
	}
	STDMETHOD(Undo)(ULONG a_nSteps)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->Undo(a_nSteps) : E_NOTIMPL;
	}
	STDMETHOD(Redo)(ULONG a_nSteps)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->Redo(a_nSteps) : E_NOTIMPL;
	}
	STDMETHOD(StepStart)(ILocalizedString* a_pName)
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->StepStart(a_pName) : E_NOTIMPL;
	}
	STDMETHOD(StepEnd)()
	{
		IDocumentUndo* p = static_cast<T*>(this)->MainUndo();
		return p ? p->StepEnd() : E_NOTIMPL;
	}
};

