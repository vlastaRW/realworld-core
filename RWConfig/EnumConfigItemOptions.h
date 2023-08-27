
#pragma once

class ATL_NO_VTABLE CEnumConfigItemOptions : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumConfigItemOptions
{
public:
	CEnumConfigItemOptions()
	{
	}


BEGIN_COM_MAP(CEnumConfigItemOptions)
	COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IEnumStrings
public:
	STDMETHOD(Size)(ULONG *a_pnSize)
	{
		CHECKPOINTER(a_pnSize);

		*a_pnSize = m_aItems.size();

		return S_OK;
	}

	STDMETHOD(Get)(ULONG a_nIndex, TConfigValue* a_ptItem)
	{
		CHECKPOINTER(a_ptItem);
		ConfigValueInit(*a_ptItem);

		if (a_nIndex >= m_aItems.size())
			return E_RW_INDEXOUTOFRANGE;

		*a_ptItem = ConfigValueCopy(m_aItems[a_nIndex]);

		return S_OK;
	}

	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems)
	{
		CHECKPOINTER(a_atItems);
		ULONG j;
		for (j = 0; j < a_nCount; j++)
		{
			ConfigValueInit(a_atItems[j]);
		}

		if ((a_nIndexFirst+a_nCount) > m_aItems.size())
			return E_RW_INDEXOUTOFRANGE;

		vector<CConfigValue>::const_iterator i;
		for (i = m_aItems.begin() + a_nIndexFirst; a_nCount; a_nCount--, i++, a_atItems++)
		{
			*a_atItems = ConfigValueCopy(*i);
		}

		return S_OK;
	}

	// internal init method
public:
	STDMETHOD(Insert)(const TConfigValue& a_tItem)
	{
		m_aItems.push_back(a_tItem);

		return S_OK;
	}

private:
	vector<CConfigValue> m_aItems;
};
