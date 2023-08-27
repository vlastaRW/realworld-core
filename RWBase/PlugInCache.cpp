// PlugInCache.cpp : Implementation of CPlugInCache

#include "stdafx.h"
#include "PlugInCache.h"

// CPlugInCache

typedef void (__stdcall fnEnumCategoryCLSIDsCallback) (void* a_pContext, ULONG a_nClasses, CLSID const* a_aClasses);
typedef void (STDAPICALLTYPE fnEnumCategoryCLSIDs)(REFCATID a_tCatID, void* a_pContext, fnEnumCategoryCLSIDsCallback* a_pfnCallback);

extern __declspec(selectany) fnEnumCategoryCLSIDs* s_pfnEnumCategoryCLSIDs = NULL;

void STDAPICALLTYPE DummyEnumCategoryCLSIDs(REFCATID, void*, fnEnumCategoryCLSIDsCallback*) {}
void EnumCategoryCLSIDs(REFCATID a_tCatID, void* a_pContext, fnEnumCategoryCLSIDsCallback* a_pfnCallback)
{
	if (s_pfnEnumCategoryCLSIDs == NULL)
	{
		HMODULE hMod = GetModuleHandle(NULL);
#ifdef WIN64
		s_pfnEnumCategoryCLSIDs = (fnEnumCategoryCLSIDs*) GetProcAddress(hMod, "RWEnumCategoryCLSIDs");
#else
		s_pfnEnumCategoryCLSIDs = (fnEnumCategoryCLSIDs*) GetProcAddress(hMod, "_RWEnumCategoryCLSIDs@12");
#endif
		if (s_pfnEnumCategoryCLSIDs == NULL)
			s_pfnEnumCategoryCLSIDs = &DummyEnumCategoryCLSIDs;
	}
	return (*s_pfnEnumCategoryCLSIDs)(a_tCatID, a_pContext, a_pfnCallback);
}

void __stdcall EnumIDs(void* a_pContext, ULONG a_nClasses, CLSID const* a_aClasses)
{
	reinterpret_cast<IEnumGUIDsInit*>(a_pContext)->InsertMultiple(a_nClasses, a_aClasses);
}

struct SContext
{
	IID iid;
	CComPtr<IEnumGUIDsInit> pGUIDs;
	CComPtr<IEnumUnknownsInit> pObjs;
};

void __stdcall EnumObjects(void* a_pContext, ULONG a_nClasses, CLSID const* a_aClasses)
{
	SContext* p = reinterpret_cast<SContext*>(a_pContext);
	for (ULONG i = 0; i < a_nClasses; ++i)
	{
		CComPtr<IUnknown> pObj;
		RWCoCreateInstance(a_aClasses[i], NULL, CLSCTX_ALL, p->iid, reinterpret_cast<void**>(&pObj));
		if (pObj)
		{
			p->pObjs->Insert(pObj);
			if (p->pGUIDs) p->pGUIDs->Insert(a_aClasses[i]);
		}
	}
}

STDMETHODIMP CPlugInCache::CLSIDsEnum(REFGUID a_guidCategory, DWORD UNREF(a_nMaxAge), IEnumGUIDs** a_ppCLSIDs)
{
	try
	{
		*a_ppCLSIDs = NULL;
		CComPtr<IEnumGUIDsInit> pGUIDs;
		RWCoCreateInstance(pGUIDs, __uuidof(EnumGUIDs));
		EnumCategoryCLSIDs(a_guidCategory, pGUIDs.p, EnumIDs);
		*a_ppCLSIDs = pGUIDs.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppCLSIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CPlugInCache::InterfacesEnum(REFGUID a_guidCategory, REFIID a_iidRequiredInterface, DWORD UNREF(a_nMaxAge), IEnumUnknowns** a_ppObjects, IEnumGUIDs** a_ppCLSIDs)
{
	try
	{
		*a_ppObjects = NULL;
		if (a_ppCLSIDs) *a_ppCLSIDs = NULL;
		SContext sCtx;
		sCtx.iid = a_iidRequiredInterface;
		RWCoCreateInstance(sCtx.pObjs, __uuidof(EnumUnknowns));
		if (a_ppCLSIDs)
			RWCoCreateInstance(sCtx.pGUIDs, __uuidof(EnumGUIDs));
		EnumCategoryCLSIDs(a_guidCategory, &sCtx, EnumObjects);
		if (a_ppCLSIDs) *a_ppCLSIDs = sCtx.pGUIDs.Detach();
		*a_ppObjects = sCtx.pObjs.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppObjects ? E_UNEXPECTED : E_POINTER;
	}
}

