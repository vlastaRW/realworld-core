
#pragma once

template<class T>
class CSyncedDataListener
{
public:
	virtual void Notify(const T& a_tData) = 0;
};

template<class T>
class CSyncedData
{
	CSyncedData(const CSyncedData<T>&);
	CSyncedData<T>& operator =(const CSyncedData<T>&);

public:
	CSyncedData()
	{
	}

	operator const T&() const
	{
		return m_t;
	}
	CSyncedData<T>& operator =(const T& a_tOrig)
	{
		ValueSet(a_tOrig);
		return *this;
	}

	const T& ValueGet() const
	{
		return m_t;
	}
	void ValueSet(const T& a_tNew)
	{
		m_t = a_tNew;
		set<CSyncedDataListener<T>*>::const_iterator i;
		for (i = m_aListeners.begin(); i != m_aListeners.end(); i++)
		{
			(*i)->Notify(m_t);
		}
	}
	void ListenerAdd(CSyncedDataListener<T>* a_pListener)
	{
		m_aListeners.insert(a_pListener);
	}
	void ListenerRemove(CSyncedDataListener<T>* a_pListener)
	{
		m_aListeners.erase(a_pListener);
	}

private:
	T m_t;
	set<CSyncedDataListener<T>*> m_aListeners;
};