
#ifndef _COMMON_H_
#define _COMMON_H_

#define CHECKPOINTER(ptr) if ((ptr) == NULL) return E_POINTER

#define UNREF(arg)

template<class T>
size_t itemsof(const T& a_t)
{
	return sizeof(a_t)/sizeof(a_t[0]);
}

//#define implicit_cast(TIn& a_tIn)

#pragma warning(disable:4201)
#pragma warning(disable:4152)
#pragma warning(disable:4996)
#pragma warning(error:4715)
#pragma warning(error:4018)

template<class TOut, class TIn>
TOut implicit_cast(TIn& a_tIn)
{
	return a_tIn;
}

class CThrowHResult
{
public:
	explicit CThrowHResult(HRESULT a_hRes = S_OK) : m_hRes(a_hRes)
	{
		if (FAILED(a_hRes))
			throw a_hRes;
	}

	operator HRESULT ()
	{
		return m_hRes;
	}

	HRESULT operator =(HRESULT a_hRes)
	{
		m_hRes = a_hRes;

		if (FAILED(a_hRes))
		{
			throw a_hRes;
		}

		return a_hRes;
	}

private:
	HRESULT m_hRes;
};


// COM class creation override

#ifdef RWCOM_ROOTEXE

void InitializePlugIns(LPCTSTR* a_apszSearchMasks);
void ReleasePlugIns();
extern "C" __declspec(dllexport) HRESULT __stdcall RWCoCreateInstance(REFCLSID a_tClass, LPUNKNOWN a_pUnkOuter, DWORD a_dwClsContext, REFIID a_tInterface, void** a_ppInstance);

template<class T>
inline HRESULT RWCoCreateInstance(CComPtr<T>& a_cComPtr, REFCLSID a_tClsID)
{
	return RWCoCreateInstance(a_tClsID, NULL, CLSCTX_ALL, __uuidof(T), reinterpret_cast<void**>(&a_cComPtr));
}

#else

typedef HRESULT (STDAPICALLTYPE fnCoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);

extern __declspec(selectany) fnCoCreateInstance* s_pfnCoCreateInstance = NULL;

struct RWCOM
{
	static HRESULT CreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
	{
		if (s_pfnCoCreateInstance == NULL)
		{
			HMODULE hMod = GetModuleHandle(NULL);
#ifdef WIN64
			s_pfnCoCreateInstance = (fnCoCreateInstance*) GetProcAddress(hMod, "RWCoCreateInstance");
#else
			s_pfnCoCreateInstance = (fnCoCreateInstance*) GetProcAddress(hMod, "_RWCoCreateInstance@20");
#endif
			if (s_pfnCoCreateInstance == NULL)
			{
				ATLASSERT(0);
				// cannot find RWCoCreateInstance function in the main executable, fallback to ordinary COM
				hMod = GetModuleHandle(_T("OLE32.DLL"));
				s_pfnCoCreateInstance = (fnCoCreateInstance*) GetProcAddress(hMod, "CoCreateInstance");
			}
		}
		return (*s_pfnCoCreateInstance)(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}
};

//#define CoCreateInstance RWCoCreateInstance

inline HRESULT RWCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
	return RWCOM::CreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

template<class T>
inline HRESULT RWCoCreateInstance(CComPtr<T>& a_cComPtr, REFCLSID a_tClsID)
{
	return RWCOM::CreateInstance(a_tClsID, NULL, CLSCTX_ALL, __uuidof(T), reinterpret_cast<void**>(&a_cComPtr));
}

#endif

#endif//_COMMON_H_
