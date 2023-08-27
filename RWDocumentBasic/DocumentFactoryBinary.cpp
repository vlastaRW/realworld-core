// DocumentFactoryBinary.cpp : Implementation of CDocumentFactoryBinary

#include "stdafx.h"
#include "DocumentFactoryBinary.h"
#include <MultiLanguageString.h>


// CDocumentFactoryBinary

HRESULT CDocumentFactoryBinary::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), IDocumentFactoryBinary* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	try
	{
		if (a_pEncoderID) *a_pEncoderID = __uuidof(DocumentFactoryBinary);
		return a_pBuilder->Init(a_nLen, a_pData, a_bstrPrefix, a_pBase);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <ConfigCustomGUIImpl.h>

STDMETHODIMP CDocumentFactoryBinary::DefaultConfig(IConfig** a_ppDefCfg)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocumentFactoryBinary::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	CComPtr<IDocumentBinary> pBin;
	if (a_pDoc) a_pDoc->QueryFeatureInterface(__uuidof(IDocumentBinary), reinterpret_cast<void**>(&pBin));
	//if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_);
	return pBin ? S_OK : S_FALSE;
}

STDMETHODIMP CDocumentFactoryBinary::Serialize(IDocument* a_pDoc, IConfig* UNREF(a_pCfg), IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	try
	{
		CComPtr<IDocumentBinary> pBin;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentBinary), reinterpret_cast<void**>(&pBin));

		CReadLock<IDocument> cLock(a_pDoc);

		ULONG nSize = 0;
		pBin->Size(&nSize);
		BYTE bChunk[16384];
		for (ULONG i = 0; i < nSize; i+=sizeof(bChunk))
		{
			ULONG nChunk = min(sizeof(bChunk), nSize-i);
			if (FAILED(pBin->Data(i, nChunk, bChunk)) || FAILED(a_pDst->Write(nChunk, bChunk)))
				return E_FAIL;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryBinary::Init(ULONG a_nLength, BYTE const* a_pData, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComObject<CDocumentBinary>* pDoc = NULL;
		CComObject<CDocumentBinary>::CreateInstance(&pDoc);
		CComPtr<IDocumentData> pTmp = pDoc;
		pDoc->Init(a_nLength, a_pData);
		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
