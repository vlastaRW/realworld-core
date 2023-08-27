#pragma once

template<class T, class IEnumXxxx, class IEnumXxxxInit, class TItem>
class ATL_NO_VTABLE CEnumXxxxImpl :
	public IEnumXxxxInit
{
public:
	CEnumXxxxImpl()
	{
	}

	// IEnumXxxx methods
public:
	STDMETHOD(Size)(ULONG *a_pnSize)
	{
		CHECKPOINTER(a_pnSize);

		*a_pnSize = static_cast<ULONG>(m_aItems.size());

		return S_OK;
	}
	STDMETHOD(Get)(ULONG a_nIndex, TItem* a_pxItem)
	{
		CHECKPOINTER(a_pxItem);

		if (a_nIndex >= m_aItems.size())
			return E_RW_INDEXOUTOFRANGE;

		static_cast<T*>(this)->Lock();
		*a_pxItem = m_aItems[a_nIndex];
		static_cast<T*>(this)->Unlock();

		return S_OK;
	}
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TItem* a_axItems)
	{
		CHECKPOINTER(a_axItems);

		if ((a_nIndexFirst+a_nCount) > m_aItems.size())
			return E_RW_INDEXOUTOFRANGE;

		vector<TItem>::const_iterator i;

		static_cast<T*>(this)->Lock();
		for (i = m_aItems.begin() + a_nIndexFirst; a_nCount; a_nCount--, i++, a_axItems++)
		{
			*a_axItems = *i;
		}
		static_cast<T*>(this)->Unlock();
	    
		return S_OK;
	}

	// IEnumXxxxInit Methods
public:
	STDMETHOD(Insert)(TItem a_xItem)
	{
		HRESULT hRet = S_OK;
		static_cast<T*>(this)->Lock();
		try
		{
			m_aItems.push_back(a_xItem);
		}
		catch (...)
		{
			hRet = E_FAIL; // TODO: better return code
		}
		static_cast<T*>(this)->Unlock();

		return hRet;
	}
	STDMETHOD(InsertMultiple)(ULONG a_nCount, TItem const* a_axItems)
	{
		CHECKPOINTER(a_axItems);

		HRESULT hRet = S_OK;
		static_cast<T*>(this)->Lock();
		try
		{
			ULONG i;
			for (i = 0; i < a_nCount; i++)
			{
				m_aItems.push_back(a_axItems[i]);
			}
		}
		catch (...)
		{
			hRet = E_FAIL; // TODO: better return code
		}
		static_cast<T*>(this)->Unlock();

		return hRet;
	}
	STDMETHOD(InsertFromEnum)(IEnumXxxx* a_pSource)
	{
		CHECKPOINTER(a_pSource);

		HRESULT hRet = S_OK;
		static_cast<T*>(this)->Lock();
		try
		{
			ULONG i;
			TItem tItem;
			for (i = 0; SUCCEEDED(a_pSource->Get(i, &tItem)); i++)
			{
				m_aItems.push_back(tItem);
			}
		}
		catch (...)
		{
			hRet = E_FAIL; // TODO: better return code
		}
		static_cast<T*>(this)->Unlock();

		return hRet;
	}

private:
	vector<TItem> m_aItems;
};
