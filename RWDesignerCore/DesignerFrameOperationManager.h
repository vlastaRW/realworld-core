
#pragma once

#include <SharedStringTable.h>


extern __declspec(selectany) GUID const DesignerFrameOperationManagerID = {0x7bc1a5ca, 0x1e, 0x41d5, {0xa8, 0x55, 0x84, 0x78, 0xc, 0x34, 0x1a, 0x36}};

class ATL_NO_VTABLE CDesignerFrameOperationManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationManager
{
public:
	CDesignerFrameOperationManager()
	{
	}
	void Init(IOperationManager* a_pManager, IDocumentControl* a_pFrame = NULL)
	{
		if (a_pManager == NULL)
			throw E_POINTER;
		m_pManager = a_pManager;
		m_pFrame = a_pFrame;
	}

BEGIN_COM_MAP(CDesignerFrameOperationManager)
	COM_INTERFACE_ENTRY(IOperationManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// ILateConfigCreator methods
public:
	STDMETHOD(CreateConfig)(TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		return CreateConfigEx(this, a_ptControllerID, a_ppConfig);
	}

	// IOperationManager methods
public:
	STDMETHOD(CreateConfigEx)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		try
		{
			*a_ppConfig = NULL;
			if (IsEqualGUID(DesignerFrameOperationManagerID, a_ptControllerID->guidVal))
			{
				CComPtr<IConfigWithDependencies> pTmp;
				RWCoCreateInstance(pTmp, __uuidof(ConfigWithDependencies));
				pTmp->Finalize(NULL);
				*a_ppConfig = pTmp.Detach();
				return S_OK;
			}
			return m_pManager->CreateConfigEx(a_pOverrideForItem, a_ptControllerID, a_ppConfig);
		}
		catch (...)
		{
			return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	STDMETHOD(ItemGetCount)(ULONG* a_pnCount)
	{
		try
		{
			ULONG nCount = 0;
			HRESULT hRes = m_pManager->ItemGetCount(&nCount);
			*a_pnCount = nCount+1;
			return hRes;
		}
		catch (...)
		{
			return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ItemIDGetDefault)(TConfigValue* a_ptDefaultOpID)
	{
		return m_pManager->ItemIDGetDefault(a_ptDefaultOpID);
	}
	STDMETHOD(ItemIDGet)(IOperationManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
	{
		try
		{
			if (a_nIndex == 0)
			{
				*a_ppName = NULL;
				a_ptOperationID->eTypeID = ECVTGUID;
				a_ptOperationID->guidVal = DesignerFrameOperationManagerID;
				*a_ppName = _SharedStringTable.GetString(IDS_OP_OPENINSAMEWINDOW);
				return S_OK;
			}
			else
			{
				return m_pManager->ItemIDGet(a_pOverrideForItem ? a_pOverrideForItem : this, a_nIndex-1, a_ptOperationID, a_ppName);
			}
		}
		catch (...)
		{
			return a_ppName == NULL || a_ptOperationID == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(InsertIntoConfigAs)(IOperationManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, TConfigOptionCondition const* a_aItemConditions)
	{
		try
		{
			CComPtr<ISubConfigSwitchLate> pSubCfg;
			RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitchLate));
			HRESULT hRes = pSubCfg->Init(a_pOverrideForItem ? a_pOverrideForItem : this);
			if (FAILED(hRes)) return hRes;
			CConfigValue cDefault;
			hRes = m_pManager->ItemIDGetDefault(&cDefault);
			if (FAILED(hRes)) return hRes;
			hRes = a_pConfig->ItemIns1ofN(a_bstrID, a_pItemName, a_pItemDesc, cDefault, pSubCfg);
			if (FAILED(hRes)) return hRes;

			ULONG nCount = 0;
			hRes = ItemGetCount(&nCount);
			if (FAILED(hRes))
				return hRes;

			for (ULONG i = 0; i != nCount; ++i)
			{
				CComPtr<ILocalizedString> pStr;
				CConfigValue cVal;
				ItemIDGet(a_pOverrideForItem, i, &cVal, &pStr);
				a_pConfig->ItemOptionAdd(a_bstrID, cVal, pStr, a_nItemConditions, a_aItemConditions);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			if (IsEqualGUID(DesignerFrameOperationManagerID, a_ptOperationID->guidVal))
			{
				return m_pFrame != NULL ? S_OK : S_FALSE;
			}
			return m_pManager->CanActivate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			if (IsEqualGUID(DesignerFrameOperationManagerID, a_ptOperationID->guidVal))
			{
				if (m_pFrame == NULL)
					return E_FAIL;
				m_pFrame->SetNewDocument(a_pDocument);
				return S_OK;
			}
			return m_pManager->Activate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates, a_hParent, a_tLocaleID);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Visit)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor)
	{
		return m_pManager->Visit(a_pOverrideForItem ? a_pOverrideForItem : this, a_ptOperationID, a_pConfig, a_pVisitor);
	}

private:
	CComPtr<IOperationManager> m_pManager;
	IDocumentControl* m_pFrame;
};
