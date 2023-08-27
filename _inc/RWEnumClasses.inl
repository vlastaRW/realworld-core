
typedef HRESULT (__stdcall fnAddClassCallback) (void* a_pContext, REFCLSID a_tClsID, ULONG a_nCategories, CATID const* a_aCategoryIDs);
static int const MAX_CATEGORIES = 16; // ...to avoid unncecessary memory allocations (16 categories per class is more than enough)
STDAPI DllEnumClasses(void* a_pContext, fnAddClassCallback* a_pfnAddClass)
{
	if (a_pfnAddClass == NULL)
		return E_POINTER;

	CATID aCatIDs[MAX_CATEGORIES];

	for (_ATL_OBJMAP_ENTRY_EX** ppEntry = _AtlComModule.m_ppAutoObjMapFirst; ppEntry < _AtlComModule.m_ppAutoObjMapLast; ppEntry++)
	{
		if (*ppEntry != NULL)
		{
			_ATL_OBJMAP_ENTRY_EX* pEntry = *ppEntry;
			_ATL_CATMAP_ENTRY const* aCategories = (*pEntry->pfnGetCategoryMap)();
			ULONG nCats = 0;
			if (aCategories) for (; nCats < MAX_CATEGORIES && aCategories->pcatid && aCategories->iType != _ATL_CATMAP_ENTRY_END; ++aCategories)
			{
				if (aCategories->iType == _ATL_CATMAP_ENTRY_IMPLEMENTED)
				{
					aCatIDs[nCats++] = *aCategories->pcatid;
				}
			}
			HRESULT hRes = (*a_pfnAddClass)(a_pContext, *pEntry->pclsid, nCats, aCatIDs);
			if (FAILED(hRes))
				return hRes;
		}
	}

	return S_OK;
}

