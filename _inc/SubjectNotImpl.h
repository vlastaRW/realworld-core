
#pragma once

template<class ISubject, class IObserver, class TParameter>
class CSubjectNotImpl : public ISubject
{
public:

	STDMETHOD(ObserverIns)(IObserver* /*a_pObserver*/, TCookie /*a_tCookie*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ObserverDel)(IObserver* /*a_pObserver*/, TCookie /*a_tCookie*/)
	{
		return S_FALSE;
	}

	void Fire_Notify(TParameter /*a_tParameter*/)
	{
	}
	size_t ObserverGetCount() const
	{
		return 0;
	}
};

