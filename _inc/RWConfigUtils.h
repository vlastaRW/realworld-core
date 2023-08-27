#pragma once

#ifdef __cplusplus

} // pause: extern "C"{

inline void ConfigValueInit(TConfigValue& a_tValue)
{
	a_tValue.eTypeID = ECVTEmpty;
}

inline void ConfigValueClear(TConfigValue& a_tValue)
{
	if (a_tValue.eTypeID == ECVTString)
	{
		SysFreeString(a_tValue.bstrVal);
	}
	a_tValue.eTypeID = ECVTEmpty;
}

inline TConfigValue ConfigValueCopy(const TConfigValue& a_tValue)
{
	TConfigValue tTemp;
	tTemp = a_tValue;
	if (tTemp.eTypeID == ECVTString)
	{
		tTemp.bstrVal = SysAllocString(a_tValue.bstrVal);
	}
	return tTemp;
}

inline HRESULT CopyConfigValues(IConfig* a_pDst, IConfig* a_pSrc)
{
	if (a_pDst == NULL || a_pSrc == NULL)
		return E_POINTER;

	HRESULT hRes = a_pDst->CopyFrom(a_pSrc, NULL);
	if (hRes != E_NOTIMPL)
		return hRes;

	CComPtr<IEnumStrings> pES;
	a_pSrc->ItemIDsEnum(&pES);
	ULONG nItems;
	pES->Size(&nItems);
	CAutoVectorPtr<BSTR> aIDs(new BSTR[nItems]);
	pES->GetMultiple(0, nItems, aIDs);
	CAutoVectorPtr<TConfigValue> aValues(new TConfigValue[nItems]);
	ULONG i;
	for (i = 0; i < nItems; i++)
	{
		ConfigValueInit(aValues[i]);
		a_pSrc->ItemValueGet(aIDs[i], aValues+i);
	}

	HRESULT hr = a_pDst->ItemValuesSet(nItems, aIDs, aValues);

	for (i = 0; i < nItems; i++)
	{
		SysFreeString(aIDs[i]);
		ConfigValueClear(aValues[i]);
	}

	return hr;
}

inline HRESULT CopySelectedConfigValues(IConfig* a_pDst, IConfig* a_pSrc, LPCOLESTR a_pszIDPrefix)
{
	if (a_pDst == NULL || a_pSrc == NULL)
		return E_POINTER;

	HRESULT hRes = a_pDst->CopyFrom(a_pSrc, CComBSTR(a_pszIDPrefix));
	if (hRes != E_NOTIMPL)
		return hRes;

	size_t nPrefixLen = wcslen(a_pszIDPrefix);

	CComPtr<IEnumStrings> pES;
	a_pSrc->ItemIDsEnum(&pES);
	ULONG nItems;
	pES->Size(&nItems);
	CAutoVectorPtr<BSTR> aIDs(new BSTR[nItems]);
	CAutoVectorPtr<BSTR> aIDs2(new BSTR[nItems]);
	pES->GetMultiple(0, nItems, aIDs);
	BSTR* pDst = aIDs2;
	for (ULONG i = 0; i < nItems; ++i)
	{
		if (aIDs[i] != NULL && wcsncmp(aIDs[i], a_pszIDPrefix, nPrefixLen) == 0)
		{
			*pDst = aIDs[i];
			++pDst;
		}
	}
	size_t nItems2 = pDst-aIDs2;
	HRESULT hr = S_OK;
	if (nItems2 != 0)
	{
		CAutoVectorPtr<TConfigValue> aValues(new TConfigValue[nItems2]);
		size_t i;
		for (i = 0; i < nItems2; ++i)
		{
			ConfigValueInit(aValues[i]);
			a_pSrc->ItemValueGet(aIDs2[i], aValues+i);
		}

		ATLASSERT(nItems2 <= 0xffffffff); // 64bit limitation
		hr = a_pDst->ItemValuesSet(static_cast<ULONG>(nItems2), aIDs2, aValues);

		for (i = 0; i < nItems2; i++)
		{
			ConfigValueClear(aValues[i]);
		}
	}

	for (ULONG i = 0; i < nItems; i++)
	{
		SysFreeString(aIDs[i]);
	}

	return hr;
}

class CConfigValue
{
public:
	class CCastException
	{
	};

	// constructors
	CConfigValue()
	{
		m_tValue.eTypeID = ECVTEmpty;
	}
	CConfigValue(bool a_bVal)
	{
		m_tValue.eTypeID = ECVTBool;
		m_tValue.bVal = a_bVal;
	}
	CConfigValue(LONG a_iVal)
	{
		m_tValue.eTypeID = ECVTInteger;
		m_tValue.iVal = a_iVal;
	}
	CConfigValue(float a_fVal)
	{
		m_tValue.eTypeID = ECVTFloat;
		m_tValue.fVal = a_fVal;
	}
	CConfigValue(float a_fVal1, float a_fVal2)
	{
		m_tValue.eTypeID = ECVTVector2;
		m_tValue.vecVal[0] = a_fVal1;
		m_tValue.vecVal[1] = a_fVal2;
	}
	CConfigValue(float a_fVal1, float a_fVal2, float a_fVal3, bool a_bIsColor = false)
	{
		m_tValue.eTypeID = a_bIsColor ? ECVTFloatColor : ECVTVector3;
		m_tValue.vecVal[0] = a_fVal1;
		m_tValue.vecVal[1] = a_fVal2;
		m_tValue.vecVal[2] = a_fVal3;
	}
	CConfigValue(float a_fVal1, float a_fVal2, float a_fVal3, float a_fVal4)
	{
		m_tValue.eTypeID = ECVTVector4;
		m_tValue.vecVal[0] = a_fVal1;
		m_tValue.vecVal[1] = a_fVal2;
		m_tValue.vecVal[2] = a_fVal3;
		m_tValue.vecVal[3] = a_fVal4;
	}
	CConfigValue(EConfigValueType a_eType, float const* a_pVecOrCol)
	{
		switch (a_eType)
		{
		case ECVTVector4:
			m_tValue.vecVal[3] = a_pVecOrCol[3];
		case ECVTVector3:
		case ECVTFloatColor:
			m_tValue.vecVal[2] = a_pVecOrCol[2];
		case ECVTVector2:
			m_tValue.vecVal[1] = a_pVecOrCol[1];
			m_tValue.vecVal[0] = a_pVecOrCol[0];
			break;
		default:
			throw new CCastException;
		}
		m_tValue.eTypeID = a_eType;
	}
	CConfigValue(GUID a_guidVal)
	{
		m_tValue.eTypeID = ECVTGUID;
		m_tValue.guidVal = a_guidVal;
	}
	CConfigValue(LPCSTR a_pszVal)
	{
		m_tValue.eTypeID = ECVTString;
		USES_CONVERSION;
		m_tValue.bstrVal = SysAllocString(A2OLE(a_pszVal));
	}
	CConfigValue(LPCWSTR a_pszVal)
	{
		m_tValue.eTypeID = ECVTString;
		m_tValue.bstrVal = SysAllocString(a_pszVal);
	}
	CConfigValue(const BSTR& a_bstrVal)
	{
		m_tValue.eTypeID = ECVTString;
		m_tValue.bstrVal = SysAllocString(a_bstrVal);
	}
	CConfigValue(const CConfigValue& a_cOrig)
	{
		SetContent(a_cOrig.m_tValue);
	}
	CConfigValue(const TConfigValue& a_cOrig)
	{
		SetContent(a_cOrig);
	}
	// destructor
	~CConfigValue()
	{
		DeleteContent();
	}

	// assignment
	const CConfigValue& operator =(const CConfigValue& a_cOrig)
	{
		DeleteContent();
		SetContent(a_cOrig.m_tValue);
		return *this;
	}
	const CConfigValue& operator =(const TConfigValue& a_tOrig)
	{
		DeleteContent();
		SetContent(a_tOrig);
		return *this;
	}

	// comparison
	bool operator ==(const TConfigValue& a_tOther) const
	{
		if (m_tValue.eTypeID != a_tOther.eTypeID)
			return false;
		switch (m_tValue.eTypeID)
		{
		case ECVTInteger:	return m_tValue.iVal == a_tOther.iVal;
		case ECVTFloat:		return m_tValue.fVal == a_tOther.fVal;
		case ECVTBool:		return m_tValue.bVal == a_tOther.bVal;
		case ECVTString:	return m_tValue.bstrVal ? (a_tOther.bstrVal ? wcscmp(m_tValue.bstrVal, a_tOther.bstrVal) == 0 : false) : (a_tOther.bstrVal ? false : true);
		case ECVTGUID:		return InlineIsEqualGUID(m_tValue.guidVal, a_tOther.guidVal) != 0;
		case ECVTVector4:	if (m_tValue.vecVal[3] != a_tOther.vecVal[3]) return false;
		case ECVTFloatColor:
		case ECVTVector3:	if (m_tValue.vecVal[2] != a_tOther.vecVal[2]) return false;
		case ECVTVector2:	return m_tValue.vecVal[1] == a_tOther.vecVal[1] && m_tValue.vecVal[0] == a_tOther.vecVal[0];
		default:			return true;
		}
	}
	bool operator ==(const CConfigValue& a_cOther) const
	{
		return operator ==(a_cOther.m_tValue);
	}
	bool operator !=(const TConfigValue& a_tOther) const
	{
		return !operator==(a_tOther);
	}
	bool operator !=(const CConfigValue& a_cOther) const
	{
		return !operator==(a_cOther.m_tValue);
	}
	bool operator <(const TConfigValue& a_tOther) const
	{
		if (m_tValue.eTypeID != a_tOther.eTypeID)
			return false;
		switch (m_tValue.eTypeID)
		{
		case ECVTInteger:	return m_tValue.iVal < a_tOther.iVal;
		case ECVTFloat:		return m_tValue.fVal < a_tOther.fVal;
		case ECVTBool:		return m_tValue.bVal < a_tOther.bVal;
		case ECVTString:	return wcscmp(m_tValue.bstrVal, a_tOther.bstrVal) < 0;
		case ECVTGUID:
			{
				const DWORD* p1 = reinterpret_cast<const DWORD*>(&m_tValue.guidVal);
				const DWORD* p2 = reinterpret_cast<const DWORD*>(&a_tOther.guidVal);
				int i;
				for (i = 0; i < 4; i++)
				{
					if (p1[i] < p2[i]) return true;
					if (p1[i] > p2[i]) return false;
				}
				return false;
			}
		case ECVTVector4:	if (m_tValue.vecVal[3] < a_tOther.vecVal[3]) return true; else if (m_tValue.vecVal[3] > a_tOther.vecVal[3]) return false;
		case ECVTFloatColor:
		case ECVTVector3:	if (m_tValue.vecVal[2] < a_tOther.vecVal[2]) return true; else if (m_tValue.vecVal[2] > a_tOther.vecVal[2]) return false;
		case ECVTVector2:	return m_tValue.vecVal[1] < a_tOther.vecVal[1] || (m_tValue.vecVal[1] == a_tOther.vecVal[1] && m_tValue.vecVal[0] < a_tOther.vecVal[0]);
		default:			return false;
		}
	}
	bool operator <(const CConfigValue& a_cOther) const
	{
		return operator<(a_cOther.m_tValue);
	}
	// data exchange
	operator const TConfigValue&() const
	{
		return m_tValue;
	}
	operator const TConfigValue*() const
	{
		return &m_tValue;
	}
	TConfigValue* operator&()
	{
		DeleteContent();
		return &m_tValue;
	}
	TConfigValue Detach()
	{
		TConfigValue tTemp = m_tValue;
		m_tValue.eTypeID = ECVTEmpty;
		return tTemp;
	}
	void Swap(CConfigValue& a_rhs)
	{
		TConfigValue t = a_rhs.m_tValue;
		a_rhs.m_tValue = m_tValue;
		m_tValue = t;
	}
	EConfigValueType TypeGet() const
	{
		return m_tValue.eTypeID;
	}
	operator LONG() const
	{
		ATLASSERT(m_tValue.eTypeID == ECVTInteger);
		if (m_tValue.eTypeID != ECVTInteger) throw new CCastException;
		return m_tValue.iVal;
	}
	operator float() const
	{
		ATLASSERT(m_tValue.eTypeID == ECVTFloat);
		if (m_tValue.eTypeID != ECVTFloat) throw new CCastException;
		return m_tValue.fVal;
	}
	operator bool() const
	{
		ATLASSERT(m_tValue.eTypeID == ECVTBool);
		if (m_tValue.eTypeID != ECVTBool) throw new CCastException;
		return m_tValue.bVal != 0;
	}
	operator BSTR() const
	{
		ATLASSERT(m_tValue.eTypeID == ECVTString);
		if (m_tValue.eTypeID != ECVTString) throw new CCastException;
		return m_tValue.bstrVal;
	}
	operator const GUID&() const
	{
		ATLASSERT(m_tValue.eTypeID == ECVTGUID);
		if (m_tValue.eTypeID != ECVTGUID) throw new CCastException;
		return m_tValue.guidVal;
	}
	float operator[](int a_nIndex) const
	{
		ATLASSERT(m_tValue.eTypeID == ECVTVector2 || m_tValue.eTypeID == ECVTVector3 || m_tValue.eTypeID == ECVTVector4 || m_tValue.eTypeID == ECVTFloatColor);
		ATLASSERT(a_nIndex<4);
		if (m_tValue.eTypeID != ECVTVector2 && m_tValue.eTypeID != ECVTVector3 && m_tValue.eTypeID != ECVTVector4 && m_tValue.eTypeID != ECVTFloatColor) throw new CCastException;
		return m_tValue.vecVal[a_nIndex];
	}
	operator float const*() const
	{
		ATLASSERT(m_tValue.eTypeID == ECVTVector2 || m_tValue.eTypeID == ECVTVector3 || m_tValue.eTypeID == ECVTVector4 || m_tValue.eTypeID == ECVTFloatColor);
		if (m_tValue.eTypeID != ECVTVector2 && m_tValue.eTypeID != ECVTVector3 && m_tValue.eTypeID != ECVTVector4 && m_tValue.eTypeID != ECVTFloatColor) throw new CCastException;
		return m_tValue.vecVal;
	}

private:
	void DeleteContent()
	{
		if (m_tValue.eTypeID == ECVTString)
		{
			SysFreeString(m_tValue.bstrVal);
		}
		m_tValue.eTypeID = ECVTEmpty;
	}
	void SetContent(const TConfigValue& a_cNewValue)
	{
		m_tValue = a_cNewValue;
		if (m_tValue.eTypeID == ECVTString)
		{
			m_tValue.bstrVal = SysAllocString(a_cNewValue.bstrVal);
		}
	}

	TConfigValue m_tValue;
};

class CConfigDescriptorImpl : public IConfigDescriptor
{
	//STDMETHOD(Tags)(BSTR* /*a_pbstrTags*/) { return E_NOTIMPL; }

	STDMETHOD(Name)(IUnknown* /*a_pContext*/, IConfig* /*a_pConfig*/, ILocalizedString** /*a_ppName*/) { return E_NOTIMPL; }
	STDMETHOD(Description)(IUnknown* /*a_pContext*/, IConfig* /*a_pConfig*/, ILocalizedString** /*a_ppDesc*/) { return E_NOTIMPL; }
	STDMETHOD(PreviewIconID)(IUnknown* /*a_pContext*/, IConfig* /*a_pConfig*/, GUID* /*a_pIconID*/) { return E_NOTIMPL; }
	STDMETHOD(PreviewIcon)(IUnknown* /*a_pContext*/, IConfig* /*a_pConfig*/, ULONG /*a_nSize*/, HICON* /*a_phIcon*/) { return E_NOTIMPL; }

	STDMETHOD(Command)(BSTR* /*a_pbstrCmd*/) { return E_NOTIMPL; }
	STDMETHOD(Serialize)(IConfig* /*a_pConfig*/, BSTR* /*a_pbstrCmd*/) { return E_NOTIMPL; }
	STDMETHOD(Parse)(BSTR /*a_bstrCmd*/, IConfig* /*a_pConfig*/) { return E_NOTIMPL; }
};

extern "C"{ // continue: extern "C"{

#endif//__cplusplus
