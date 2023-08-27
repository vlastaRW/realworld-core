// DocumentBinary.h : Declaration of the CDocumentBinary

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentBasic.h"
#include <SubjectImpl.h>


// CDocumentBinary

class ATL_NO_VTABLE CDocumentBinary : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDocumentDataImpl<CDocumentBinary, &CLSID_DocumentFactoryBinary>,
	public CSubjectImpl<IDocumentBinary, IBinaryObserver, TBinaryChange>
{
public:
	CDocumentBinary() : m_nBegSame(0xffffffff), m_nEndSame(0xffffffff)
	{
	}
	void Init(ULONG a_nSize, BYTE const* a_pData)
	{
		m_cData.reserve(a_nSize);
		std::copy(a_pData, a_pData+a_nSize, back_inserter(m_cData));
	}


BEGIN_COM_MAP(CDocumentBinary)
	COM_INTERFACE_ENTRY(IDocumentData)
	COM_INTERFACE_ENTRY(IDocumentBinary)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	STDMETHOD(WriteFinished)()
	{
		if (m_nBegSame != 0xffffffff && m_nEndSame != 0xffffffff)
		{
			TBinaryChange tChg = {m_nBegSame, m_nEndSame, m_cData.size()};
			CSubjectImpl<IDocumentBinary, IBinaryObserver, TBinaryChange>::Fire_Notify(tChg);
			m_nBegSame = 0xffffffff;
			m_nEndSame = 0xffffffff;
			M_Base()->SetDirty();
		}
		return S_OK;
	}

	// IDocumentData mehods
public:
	STDMETHOD(DataCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect);

	// IDocumentBinary methods
public:
	STDMETHOD(Size)(ULONG* a_pSize);
	STDMETHOD(Data)(ULONG a_nOffset, ULONG a_nSize, BYTE* a_pData);

	STDMETHOD(Insert)(ULONG a_nOffset, ULONG a_nSize, BYTE const* a_pData);
	STDMETHOD(Replace)(ULONG a_nOffset, ULONG a_nSize, BYTE const* a_pData);
	STDMETHOD(Delete)(ULONG a_nOffset, ULONG a_nSize);

private:
	void AddChange(ULONG a_nBegSame, ULONG a_nEndSame)
	{
		if (m_nBegSame > a_nBegSame)
			m_nBegSame = a_nBegSame;
		if (m_nEndSame > a_nEndSame)
			m_nEndSame = a_nEndSame;
	}

private:
	std::vector<BYTE> m_cData;
	ULONG m_nBegSame;
	ULONG m_nEndSame;
};

