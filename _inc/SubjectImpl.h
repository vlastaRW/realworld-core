
#pragma once
#include<vector>

template<class ISubject, class IObserver, class TParameter>
class CSubjectImpl : public ISubject
{
public:
#ifdef _DEBUG
	CSubjectImpl() : m_iActNotifying(static_cast<size_t>(-1)), m_iMaxNotifying(0)
	{
	}
#endif

	~CSubjectImpl()
	{
		ATLASSERT(m_aObservers.empty());
		std::vector<TObserver>::iterator i;
		for (i = m_aObservers.begin(); i != m_aObservers.end(); i++)
		{
			i->pObserver->Release();
		}
	}

	STDMETHOD(ObserverIns)(IObserver* a_pObserver, TCookie a_tCookie)
	{
		CHECKPOINTER(a_pObserver);

		CComCritSecLock<CComAutoCriticalSection> cLock(m_cCS);
		m_aObservers.push_back(TObserver(a_pObserver, a_tCookie));

		return S_OK;
	}
	STDMETHOD(ObserverDel)(IObserver* a_pObserver, TCookie a_tCookie)
	{
		CComCritSecLock<CComAutoCriticalSection> cLock(m_cCS);
		std::vector<TObserver>::iterator i;
		for (i = m_aObservers.begin(); i != m_aObservers.end(); i++)
		{
			// comparison via IUnknown omnited
			if (i->pObserver == a_pObserver && i->tCookie == a_tCookie)
			{
				if (static_cast<int>(m_iActNotifying) >= static_cast<int>(i-m_aObservers.begin())) // force signed comparison
				{
					m_iActNotifying--;
				}
				m_aObservers.erase(i);
				a_pObserver->Release();
				m_iMaxNotifying--;
				return S_OK;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}

	void Fire_Notify(TParameter a_tParameter)
	{
		CComCritSecLock<CComAutoCriticalSection> cLock(m_cCS);
		// prevent iterator invalidation during Notify calls
		ATLASSERT(0 > (int)m_iActNotifying);// Fire_Notify is not reentrant (the behavior is affected while in execution)
		m_iMaxNotifying = m_aObservers.size();
		for (m_iActNotifying = 0; m_iActNotifying < m_iMaxNotifying; m_iActNotifying++)
		{
			m_aObservers[m_iActNotifying].pObserver->Notify(m_aObservers[m_iActNotifying].tCookie, a_tParameter);
		}
#ifdef _DEBUG
		m_iActNotifying = static_cast<size_t>(-1);
#endif
	}
	size_t ObserverGetCount() const
	{
		CComCritSecLock<CComAutoCriticalSection> cLock(m_cCS);
		return m_aObservers.size();
	}

private:
	struct TObserver
	{
		TObserver(IObserver* a_pObserver, TCookie a_tCookie) :
			pObserver(a_pObserver), tCookie(a_tCookie) {pObserver->AddRef();}
		IObserver* pObserver;
		TCookie tCookie;
	};
	std::vector<TObserver> m_aObservers;
	size_t m_iActNotifying;
	size_t m_iMaxNotifying;
	CComAutoCriticalSection mutable m_cCS;
};

