
#pragma once

#ifdef __cplusplus

} // pause: extern "C"{

/// Implements ParentsEnum and ItemFeatureGet using ItemsGet, TBase must inherit from IStructuredRoot
/// !! does not work correctly if structure is not tree (same parent added multiple times)
template<class T, class TBase>
class CStructuredRootImpl : public TBase
{
public:
	STDMETHOD(ParentsEnum)(IComparable* a_pItem, IEnumUnknowns** a_ppPredecessors)
	{
		try
		{
			*a_ppPredecessors = NULL;

			if (a_pItem == NULL)
				return S_OK;

			CReadLock<T> cLock(static_cast<T*>(this));

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
			enum_parents ep(pItems);
			FindItem(NULL, a_pItem, ep);
			*a_ppPredecessors = pItems.Detach();
			return ep;
		}
		catch (...)
		{
			return a_ppPredecessors == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ItemFeatureGet)(IComparable* a_pItem, REFIID a_iid, void** a_ppFeatureInterface)
	{
		try
		{
			*a_ppFeatureInterface = NULL;
			if (a_pItem == NULL)
				return E_RW_ITEMNOTFOUND;

			CReadLock<T> cLock(static_cast<T*>(this));

			query_feature qf(a_pItem, a_iid, a_ppFeatureInterface);
			FindItem(NULL, a_pItem, qf);
			return qf;
		}
		catch (...)
		{
			return a_ppFeatureInterface == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(StatePrefix)(BSTR* a_pbstrPrefix)
	{
		try
		{
			*a_pbstrPrefix = NULL;
			*a_pbstrPrefix = CComBSTR(static_cast<T*>(this)->M_DataID()).Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrPrefix ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	class enum_parents
	{
	public:
		enum_parents(IEnumUnknownsInit* a_pItems) : m_pItems(a_pItems), hRes(E_RW_ITEMNOTFOUND)
		{
		}
		bool operator()(IComparable* a_pRoot)
		{
			m_pItems->Insert(a_pRoot);
			hRes = S_OK;
			return false;
		}
		operator HRESULT() const
		{
			return hRes;
		}
	private:
		CComPtr<IEnumUnknownsInit> m_pItems;
		HRESULT hRes;
	};

	class query_feature
	{
	public:
		query_feature(IComparable* a_pItem, REFIID a_iid, void** a_ppFeatureInterface) :
			m_pItem(a_pItem), m_iid(a_iid), m_ppFeatureInterface(a_ppFeatureInterface), hRes(E_RW_ITEMNOTFOUND)
		{
		}
		bool operator()(IComparable*)
		{
			hRes = m_pItem->QueryInterface(m_iid, m_ppFeatureInterface);
			return true;
		}
		operator HRESULT() const
		{
			return hRes;
		}

	private:
		IComparable* m_pItem;
		REFIID m_iid;
		void** m_ppFeatureInterface;
		HRESULT hRes;
	};

	template<class t_functor>
	bool FindItem(IComparable* a_pRoot, IComparable* a_pItem, t_functor& a_fnc)
	{
		CComPtr<IEnumUnknowns> pItems;
		this->ItemsEnum(a_pRoot, &pItems);
		if (pItems == NULL)
			return false;
		CComPtr<IComparable> pItem;
		for (ULONG i = 0; SUCCEEDED(pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem))); ++i, pItem = NULL)
		{
			if (pItem->Compare(a_pItem) == S_OK)
			{
				if (a_fnc(a_pRoot))
					return true;
			}
			if (FindItem(pItem, a_pItem, a_fnc))
				return true;
		}
		return false;
	}
};

extern "C"{ // continue: extern "C"{

#endif//__cplusplus