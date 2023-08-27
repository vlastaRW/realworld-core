
#pragma once

template<class T, class IObserver, class TParameter>
class CObserverImpl
{
public:
	CObserverImpl() : m_pInternObserver(NULL)
	{
		CComObject<CInternObserver>::CreateInstance(&m_pInternObserver);
		m_pInternObserver->AddRefAndLink(static_cast<T*>(this));
	}
	~CObserverImpl()
	{
		ATLVERIFY(m_pInternObserver->ReleaseAndUnlink() == 0);
	}

	IObserver* ObserverGet() const
	{
		return m_pInternObserver;
	}

private:
	class CInternObserver : public CComObjectRootEx<CComMultiThreadModel>, public IObserver
	{
	public:
		CInternObserver() : m_pOwner(NULL)
		{
		}

	BEGIN_COM_MAP(CInternObserver)
		COM_INTERFACE_ENTRY(IObserver)
	END_COM_MAP()

		ULONG AddRefAndLink(T* a_pOwner)
		{
			m_pOwner = a_pOwner;
			return AddRef();
		}
		ULONG ReleaseAndUnlink()
		{
			m_pOwner = NULL;
			return Release();
		}
		STDMETHOD(Notify)(TCookie a_tCookie, TParameter a_tParameter)
		{
			if (m_pOwner)
				m_pOwner->OwnerNotify(a_tCookie, a_tParameter); // base must implement OwnerNotify to receive events
			return S_OK;
		}
	private:
		T* m_pOwner;
	};

private:
	CComObject<CInternObserver>* m_pInternObserver;
};
