
#pragma once

class ATL_NO_VTABLE CBatchOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
	CBatchOperationContext() : m_nIndex(0), m_nRemaining(0)
	{
	}
	void Step(ULONG a_nIndex, ULONG a_nRemaining, ULONG a_nDropPath, std::tstring const& output)
	{
		m_pMessage = NULL;
		m_nIndex = a_nIndex;
		m_nRemaining = a_nRemaining;
		m_nDropPath = a_nDropPath;
		m_output = output;
	}
	void EndJob()
	{
		m_cStates.clear();
		m_pMessage = NULL;
	}
	ILocalizedString* M_ErrorMessage()
	{
		return m_pMessage;
	}


BEGIN_COM_MAP(CBatchOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		if (a_bstrCategoryName && wcscmp(a_bstrCategoryName, STATE_DROPROOT) == 0)
		{
			CComPtr<ISharedState> pState;
			RWCoCreateInstance(pState, __uuidof(SharedStateString));
			OLECHAR sz[32];
			swprintf(sz, L"%i", m_nDropPath);
			pState->FromText(CComBSTR(sz));
			return pState->QueryInterface(a_iid, a_ppState);
		}
		if (a_bstrCategoryName && wcscmp(a_bstrCategoryName, L"OutputFolder") == 0)
		{
			CComPtr<ISharedState> state;
			RWCoCreateInstance(state, __uuidof(SharedStateString));

			TCHAR szOut[MAX_PATH+4] = _T("");
			if (m_output.length() < 3 || m_output.length() > MAX_PATH)
			{
				GetTempPath(MAX_PATH, szOut);
				_tcscat(szOut, _T("RWBatch\\"));
			}
			else
			{
				_tcscpy(szOut, m_output.c_str());
				if (szOut[m_output.length()-1] != _T('\\') && szOut[m_output.length()-1] != _T('/'))
				{
					szOut[m_output.length()] = _T('\\');
					szOut[m_output.length()+1] = _T('\0');
				}
			}
			bool slash = false;
			TCHAR* pD = szOut+1;
			for (TCHAR* pS = szOut+1; *pS; ++pS)
			{
				*pD = *pS == _T('/') ? _T('\\') : *pS;
				if (*pS == _T('\\'))
				{
					if (!slash)
					{
						++pD;
						slash = true;
					}
				}
				else
				{
					++pD;
					slash = false;
				}
			}
			*pD = _T('\0');
			state->FromText(CComBSTR(szOut));
			*a_ppState = state.Detach();
			return S_OK;
		}
		CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
		if (i == m_cStates.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->QueryInterface(a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (a_pState)
			m_cStates[a_bstrCategoryName] = a_pState;
		else
			m_cStates.erase(a_bstrCategoryName);
		return S_OK;
	}
	STDMETHOD(IsCancelled)()
	{
		return S_FALSE;
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		if (a_pItemIndex) *a_pItemIndex = m_nIndex;
		if (a_pItemsRemaining) *a_pItemsRemaining = m_nRemaining;
		if (a_pStepIndex) *a_pStepIndex = 0;
		if (a_pStepsRemaining) *a_pStepsRemaining = 0;
		return S_OK;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		m_pMessage = a_pMessage;
		return S_OK;
	}

private:
	typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

private:
	ULONG m_nIndex;
	ULONG m_nRemaining;
	ULONG m_nDropPath;
	CStates m_cStates;
	CComPtr<ILocalizedString> m_pMessage;
	std::tstring m_output;
};

