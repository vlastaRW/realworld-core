
#pragma once

namespace ATL
{

template <class Base>
class CComObjectWeakGlobal;

/////////////////////////////////////////////////////////////////////////////////////////////
// Weak Singleton Class Factory (max. 1 instance exists)

template <class T>
class CComClassFactoryWeakSingleton :
	public CComClassFactory
{
public:
	CComClassFactoryWeakSingleton() : m_pObj(NULL)
	{
	}
	~CComClassFactoryWeakSingleton()
	{
	}

	// IClassFactory
public:
	STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj);
	// LockServer inherited

	void SetInstancePtr(CComObjectWeakGlobal<T>* a_pNewPtr)
	{
		m_pObj = a_pNewPtr;
	}
	CComObjectWeakGlobal<T>* InstancePtr() const
	{
		return m_pObj;
	}

private:
	CComObjectWeakGlobal<T>* m_pObj;
};


template <class Base>
class CComObjectWeakGlobal : public Base
{
public:
	typedef Base _BaseClass;
	CComObjectWeakGlobal(CComClassFactoryWeakSingleton<Base>* a_pClassFactory) :
		m_pClassFactory(a_pClassFactory)
	{
		ATLASSERT(m_pClassFactory); // must not be NULL
		_pAtlModule->Lock();
		// (maybe addrefing/releasing the classfactory work be better?)
	}
	// Set refcount to -(LONG_MAX/2) to protect destruction and 
	// also catch mismatched Release in debug builds
	~CComObjectWeakGlobal()
	{
		m_dwRef = -(LONG_MAX/2);
		FinalRelease();
#ifdef _ATL_DEBUG_INTERFACES
		_AtlDebugInterfacesModule.DeleteNonAddRefThunk(_GetRawUnknown());
#endif
		_pAtlModule->Unlock();
	}
	//If InternalAddRef or InternalRelease is undefined then your class
	//doesn't derive from CComObjectRoot
	STDMETHOD_(ULONG, AddRef)() throw() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)() throw()
	{
		m_pClassFactory->Lock();
		ULONG l = InternalRelease();
		if (l == 0)
			m_pClassFactory->SetInstancePtr(NULL);
		m_pClassFactory->Unlock();

		if (l == 0)
			delete this;
		return l;
	}
	//if _InternalQueryInterface is undefined then you forgot BEGIN_COM_MAP
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) throw()
	{return _InternalQueryInterface(iid, ppvObject);}
	template <class Q>
	HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp) throw()
	{
		return QueryInterface(__uuidof(Q), (void**)pp);
	}

	static HRESULT WINAPI CreateInstance(CComClassFactoryWeakSingleton<Base>* a_pClassFactory, REFIID a_iid, void** a_ppInstance) throw()
	{
		ATLASSERT(a_ppInstance);
		ATLASSERT(a_pClassFactory);

		if (a_ppInstance == NULL)
			return E_POINTER;
		*a_ppInstance = NULL;
		if (a_pClassFactory == NULL)
			return E_UNEXPECTED;

		a_pClassFactory->Lock();

		HRESULT hRes = E_OUTOFMEMORY;
		if (a_pClassFactory->InstancePtr())
		{
			hRes = a_pClassFactory->InstancePtr()->QueryInterface(a_iid, a_ppInstance);
		}
		else
		{
			CComObjectWeakGlobal<Base>* p = NULL;
			ATLTRY(p = new CComObjectWeakGlobal<Base>(a_pClassFactory))
			if (p != NULL)
			{
				p->InternalFinalConstructAddRef();
				hRes = p->_AtlInitialConstruct();
				if (SUCCEEDED(hRes))
					hRes = p->FinalConstruct();
				if (SUCCEEDED(hRes))
					hRes = p->_AtlFinalConstruct();
				p->InternalFinalConstructRelease();
				if (SUCCEEDED(hRes))
				{
					hRes = p->QueryInterface(a_iid, a_ppInstance);
				}
				if (hRes != S_OK)
				{
					delete p;
				}
				else
				{
					a_pClassFactory->SetInstancePtr(p);
				}
			}
		}

		a_pClassFactory->Unlock();

		return hRes;
	}

private:
	CComClassFactoryWeakSingleton<Base>* m_pClassFactory;
};

template <class T>
inline STDMETHODIMP CComClassFactoryWeakSingleton<T>::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj)
{
	HRESULT hRes = E_POINTER;
	if (ppvObj != NULL)
	{
		*ppvObj = NULL;
		// aggregation is not supported in WeakSingletons
		ATLASSERT(pUnkOuter == NULL);
		if (pUnkOuter != NULL)
		{
			hRes = CLASS_E_NOAGGREGATION;
		}
		else
		{
			Lock();
			hRes = CComObjectWeakGlobal<T>::CreateInstance(this, riid, ppvObj);
			Unlock();
		}
	}
	return hRes;
}

};

#define DECLARE_CLASSFACTORY_WEAKSINGLETON(obj) DECLARE_CLASSFACTORY_EX(ATL::CComClassFactoryWeakSingleton<obj>)

