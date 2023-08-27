#pragma once

#ifdef __cplusplus

class CPlugInEnumerator
{
public:
	// cozy interface
	template<typename TCallback>
	static void EnumCategoryCLSIDs(REFCATID catID, TCallback callback)
	{
		EnumCategoryCLSIDs(catID, &callback, CForwarder<TCallback>::callback);
	}

	struct lessCATID { bool operator()(CATID const& x, CATID const& y) const {return memcmp(&x, &y, sizeof x)<0;}};
	template<class IPlugIn>
	static void GetCategoryPlugInMap(REFCATID catID, std::map<CATID, CComPtr<IPlugIn>, lessCATID>& plugins)
	{
		EnumCategoryCLSIDs(catID, &plugins, CMapBuilder<IPlugIn>::callback);
	}

	// raw interface
	typedef void (__stdcall fnEnumCategoryCLSIDsCallback) (void* a_pContext, ULONG a_nClasses, CLSID const* a_aClasses);
	typedef void (STDAPICALLTYPE fnEnumCategoryCLSIDs)(REFCATID a_tCatID, void* a_pContext, fnEnumCategoryCLSIDsCallback* a_pfnCallback);
	typedef ULONG (STDAPICALLTYPE fnGetCategoryTimestamp)(REFCATID a_tCatID);

	static void EnumCategoryCLSIDs(REFCATID a_tCatID, void* a_pContext, fnEnumCategoryCLSIDsCallback* a_pfnCallback)
	{
		static fnEnumCategoryCLSIDs* s_pfnEnumCategoryCLSIDs = NULL;
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

	static ULONG GetCategoryTimestamp(REFCATID a_tCatID)
	{
		static fnGetCategoryTimestamp* s_pfnGetCategoryTimestamp = NULL;
		if (s_pfnGetCategoryTimestamp == NULL)
		{
			HMODULE hMod = GetModuleHandle(NULL);
#ifdef WIN64
			s_pfnGetCategoryTimestamp = (fnGetCategoryTimestamp*) GetProcAddress(hMod, "RWGetCategoryTimestamp");
#else
			s_pfnGetCategoryTimestamp = (fnGetCategoryTimestamp*) GetProcAddress(hMod, "_RWGetCategoryTimestamp@4");
#endif
			if (s_pfnGetCategoryTimestamp == NULL)
				s_pfnGetCategoryTimestamp = &DummyGetCategoryTimestamp;
		}
		return (*s_pfnGetCategoryTimestamp)(a_tCatID);
	}

private:
	static void STDAPICALLTYPE DummyEnumCategoryCLSIDs(REFCATID, void*, fnEnumCategoryCLSIDsCallback*) {}
	static ULONG STDAPICALLTYPE DummyGetCategoryTimestamp(REFCATID) { return 0; }

	template<typename TCallback>
	struct CForwarder
	{
		static void __stdcall callback(void* a_pContext, ULONG a_nClasses, CLSID const* a_aClasses)
		{
			(*reinterpret_cast<TCallback*>(a_pContext))(a_aClasses, a_aClasses+a_nClasses);
		}
	};
	template<class IPlugIn>
	struct CMapBuilder
	{
		static void __stdcall callback(void* a_pContext, ULONG a_nClasses, CLSID const* a_aClasses)
		{
			try
			{
				std::map<CATID, CComPtr<IPlugIn>, lessCATID>& m = *reinterpret_cast<std::map<CATID, CComPtr<IPlugIn>, lessCATID>*>(a_pContext);
				for (CLSID const* const end = a_aClasses+a_nClasses; a_aClasses != end; ++a_aClasses)
					RWCoCreateInstance(m[*a_aClasses], *a_aClasses);
			}
			catch (...)
			{
			}
		}
	};
};

// not multi-thread safe - user must use external locks
template<CATID const* catid, class IPlugIn>
struct CPlugInCache
{
	typedef std::map<CATID, CComPtr<IPlugIn>, CPlugInEnumerator::lessCATID> map_type;

	CPlugInCache() : initialized(false) {}

	map_type const& Map()
	{
		ULONG const newstamp = CPlugInEnumerator::GetCategoryTimestamp(*catid);
		if (!initialized || newstamp != timestamp)
		{
			map_type newmap;
			CPlugInEnumerator::GetCategoryPlugInMap(*catid, newmap);
			std::swap(newmap, plugins);
			timestamp = newstamp;
			initialized = true;
		}
		return plugins;
	}

private:
	map_type plugins;
	ULONG timestamp;
	bool initialized;
};

#endif//__cplusplus

