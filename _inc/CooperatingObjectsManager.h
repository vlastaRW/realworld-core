
#pragma once

#include <PLugInCache.h>

template<class IManager, class IObject, const CATID* t_ptCATID, const GUID* t_ptDefault>
class CCooperatingObjectsManagerImpl : public IManager
{
	// ILateConfigCreator methods
public:
	STDMETHOD(CreateConfig)(const TConfigValue* a_ptControllerID, IConfig** a_ppConfig)
	{
		return CreateConfigEx(this, a_ptControllerID, a_ppConfig);
	}
	// IManager methods
public:
	STDMETHOD(CreateConfigEx)(IManager* a_pOverrideForItem, const TConfigValue* a_ptControllerID, IConfig** a_ppConfig)
	{
		try
		{
			*a_ppConfig = NULL;
			if (a_ptControllerID->eTypeID == ECVTGUID)
			{
				CComPtr<IObject> p;
				RWCoCreateInstance(p, a_ptControllerID->guidVal);
				if (p)
					return p->ConfigCreate(a_pOverrideForItem, a_ppConfig);
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return (a_ptControllerID == NULL || a_ppConfig == NULL) ? E_POINTER : E_UNEXPECTED;
		}
	}

	struct counter
	{
		ULONG* cnt;
		counter(ULONG* cnt) : cnt(cnt) {}
		void operator()(CLSID const* begin, CLSID const* const end) const { *cnt += end-begin; }
	};
	STDMETHOD(ItemGetCount)(ULONG* a_pnCount)
	{
		try
		{
			*a_pnCount = 0;
			CPlugInEnumerator::EnumCategoryCLSIDs(*t_ptCATID, counter(a_pnCount));
			return S_OK;
		}
		catch (...)
		{
			return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	STDMETHOD(ItemIDGetDefault)(TConfigValue* a_ptDefaultOpID)
	{
		try
		{
			a_ptDefaultOpID->eTypeID = ECVTGUID;
			a_ptDefaultOpID->guidVal = *t_ptDefault;
			return S_OK;
		}
		catch (...)
		{
			return a_ptDefaultOpID == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	struct itemidget
	{
		ULONG a_nIndex;
		HRESULT& hRes;
		TConfigValue* a_ptOperationID;
		IManager* a_pOverrideForItem;
		ILocalizedString** a_ppName;
		itemidget(ULONG a_nIndex, HRESULT& hRes, TConfigValue* a_ptOperationID, IManager* a_pOverrideForItem, ILocalizedString** a_ppName) :
		a_nIndex(a_nIndex), hRes(hRes), a_ptOperationID(a_ptOperationID), a_pOverrideForItem(a_pOverrideForItem), a_ppName(a_ppName) {}
		void operator()(CLSID const* begin, CLSID const* const end) const
		{
			if (begin+a_nIndex >= end)
				return;
			CComPtr<IObject> p;
			RWCoCreateInstance(p, begin[a_nIndex]);
			if (p)
			{
				p->NameGet(a_pOverrideForItem, a_ppName);
				if (*a_ppName)
				{
					a_ptOperationID->eTypeID = ECVTGUID;
					a_ptOperationID->guidVal = begin[a_nIndex];
					hRes = S_OK;
					return;
				}
			}
			hRes = E_FAIL;
		}
	};
	STDMETHOD(ItemIDGet)(IManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			a_ptOperationID->eTypeID = ECVTEmpty;

			HRESULT hRes = E_RW_ITEMNOTFOUND;
			CPlugInEnumerator::EnumCategoryCLSIDs(*t_ptCATID, itemidget(a_nIndex, hRes, a_ptOperationID, a_pOverrideForItem ? a_pOverrideForItem : this, a_ppName));
			return hRes;
		}
		catch (...)
		{
			return (a_ppName == NULL || a_ptOperationID == NULL) ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ItemIDGet)(ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
	{
		return ItemIDGet(NULL, a_nIndex, a_ptOperationID, a_ppName);
	}

	STDMETHOD(InsertIntoConfigAs)(IManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, const TConfigOptionCondition* a_aItemConditions)
	{
		try
		{
			CComPtr<ISubConfigSwitchLate> pSubCfg;
			RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitchLate));
			HRESULT hRes;
			if (a_pOverrideForItem == NULL || a_pOverrideForItem == static_cast<IManager*>(this))
			{
				hRes = pSubCfg->Init(this);
			}
			else
			{
				CComObject<CHelpLateConfigCreator>* p = NULL;
				CComObject<CHelpLateConfigCreator>::CreateInstance(&p);
				CComPtr<ILateConfigCreator> pTmp = p;
				p->Init(a_pOverrideForItem, this);
				hRes = pSubCfg->Init(pTmp);
			}
			if (FAILED(hRes)) return hRes;
			CComPtr<IConfigItemCustomOptions> pCustOpts;
			CComObject<CCustomOptions>* pCO = NULL;
			CComObject<CCustomOptions>::CreateInstance(&pCO);
			pCO->Init(a_pOverrideForItem ? a_pOverrideForItem : this);
			pCustOpts = pCO;
			return a_pConfig->ItemIns1ofNWithCustomOptions(a_bstrID, a_pItemName, a_pItemDesc, CConfigValue(*t_ptDefault), pCustOpts, pSubCfg, a_nItemConditions, a_aItemConditions);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(InsertIntoConfigAs)(IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, const TConfigOptionCondition* a_aItemConditions)
	{
		return InsertIntoConfigAs(NULL, a_pConfig, a_bstrID, a_pItemName, a_pItemDesc, a_nItemConditions, a_aItemConditions);
	}

private:
	class ATL_NO_VTABLE CHelpLateConfigCreator :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ILateConfigCreator
	{
	public:
		void Init(IManager* a_pOverride, CCooperatingObjectsManagerImpl<IManager, IObject, t_ptCATID, t_ptDefault>* a_pParent)
		{
			m_pOverride = a_pOverride;
			m_pParent = a_pParent;
		}

	BEGIN_COM_MAP(CHelpLateConfigCreator)
		COM_INTERFACE_ENTRY(ILateConfigCreator)
	END_COM_MAP()

		// ILateConfigCreator methods
	public:
		STDMETHOD(CreateConfig)(const TConfigValue* a_ptControllerID, IConfig** a_ppConfig)
		{
			return m_pParent->CreateConfigEx(m_pOverride, a_ptControllerID, a_ppConfig);
		}

	private:
		CComPtr<IManager> m_pOverride;
		CComPtr<CCooperatingObjectsManagerImpl<IManager, IObject, t_ptCATID, t_ptDefault> > m_pParent;
	};

	class ATL_NO_VTABLE CCustomOptions :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IConfigItemCustomOptions
	{
	public:
		void Init(IManager* a_pOverride)
		{
			m_pOverride = a_pOverride;
		}

	BEGIN_COM_MAP(CCustomOptions)
		COM_INTERFACE_ENTRY(IConfigItemCustomOptions)
		COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
	END_COM_MAP()

		// IConfigItemCustomOptions methods
	public:
		STDMETHOD(GetValueName)(TConfigValue const* a_pValue, ILocalizedString** a_ppName)
		{
			CComPtr<IObject> p;
			if (a_pValue && a_pValue->eTypeID == ECVTGUID)
				RWCoCreateInstance(p, a_pValue->guidVal);
			if (p)
				return p->NameGet(m_pOverride, a_ppName);
			return E_FAIL;
		}

		// IEnumConfigItemOptions methods
	public:
		struct size
		{
			ULONG* a_pnSize;
			size(ULONG* a_pnSize) : a_pnSize(a_pnSize) {}
			void operator()(CLSID const* begin, CLSID const* const end) const { *a_pnSize = end-begin; }
		};
		STDMETHOD(Size)(ULONG* a_pnSize)
		{
			CPlugInEnumerator::EnumCategoryCLSIDs(*t_ptCATID, size(a_pnSize));
			return S_OK;
		}
		struct copy
		{
			ULONG a_nIndexFirst;
			ULONG a_nCount;
			TConfigValue* a_atItems;
			ULONG& copied;
			copy(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems, ULONG& copied) :
			a_nIndexFirst(a_nIndexFirst), a_nCount(a_nCount), a_atItems(a_atItems), copied(copied) {}
			void operator()(CLSID const* begin, CLSID const* end)
			{
				begin += a_nIndexFirst;
				if (begin + a_nCount < end) end = begin + a_nCount;
				while (begin < end)
				{
					a_atItems->eTypeID = ECVTGUID;
					a_atItems->guidVal = *begin;
					++begin;
					++a_atItems;
					++copied;
				}
			}
		};
		STDMETHOD(Get)(ULONG a_nIndex, TConfigValue* a_ptItem)
		{
			ULONG copied = 0;
			CPlugInEnumerator::EnumCategoryCLSIDs(*t_ptCATID, copy(a_nIndex, 1, a_ptItem, copied));
			return copied == 1 ? S_OK : E_RW_INDEXOUTOFRANGE;
		}
		STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems)
		{
			ULONG copied = 0;
			CPlugInEnumerator::EnumCategoryCLSIDs(*t_ptCATID, copy(a_nIndexFirst, a_nCount, a_atItems, copied));
			return copied == a_nCount ? S_OK : E_RW_INDEXOUTOFRANGE;
		}

	private:
		CComPtr<IManager> m_pOverride;
	};
};
