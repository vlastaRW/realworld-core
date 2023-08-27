// SharedStateColor.h : Declaration of the CSharedStateColor

#pragma once
#include "resource.h"       // main symbols
#include "RWConceptSharedState.h"


// CSharedStateColor

class ATL_NO_VTABLE CSharedStateColor :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateColor, &CLSID_SharedStateColor>,
	public ISharedStateColor
{
public:
	CSharedStateColor() :
		m_fR(0.0f), m_fG(0.0f), m_fB(0.0f),
		m_fH(0.0f), m_fL(0.0f), m_fS(0.0f),
		m_fA(0.0f), m_bAlphaEnabled(1)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateColor)
	COM_INTERFACE_ENTRY(ISharedStateColor)
	COM_INTERFACE_ENTRY(ISharedState)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		*a_pCLSID = CLSID_SharedStateColor;
		return S_OK;
	}
	STDMETHOD(ToText)(BSTR* a_pbstrText)
	{
		try
		{
			*a_pbstrText = NULL;
			wchar_t szTmp[128];
			swprintf(szTmp, L"%g,%g,%g,%g,%g,%g,%g,%i", m_fR, m_fG, m_fB, m_fH, m_fL, m_fS, m_fA, int(m_bAlphaEnabled));
			*a_pbstrText = SysAllocString(szTmp);
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(FromText)(BSTR a_bstrText)
	{
		if (a_bstrText == NULL)
			return E_RW_INVALIDPARAM;
		int i = 1;
		swscanf(a_bstrText, L"%g,%g,%g,%g,%g,%g,%g", &m_fR, &m_fG, &m_fB, &m_fH, &m_fL, &m_fS, &m_fA, &i);
		m_bAlphaEnabled = i;
		return S_OK;
	}

	// ISharedStateColor methods
public:
	STDMETHOD(RGBAGet)(float* a_pRGBA);
	STDMETHOD(HLSAGet)(float* a_pHLSA);
	STDMETHOD(RGBASet)(float const* a_pRGBA, ISharedStateColor* a_pOldState);
	STDMETHOD(HLSASet)(float const* a_pHLSA, ISharedStateColor* a_pOldState);
	STDMETHOD(RGBHLSAGet)(float* a_pRGBHLSA);
	STDMETHOD(RGBHLSASet)(float const* a_pRGBHLSA, ISharedStateColor* a_pOldState);
	STDMETHOD(AlphaEnabled)();
	STDMETHOD(AlphaSet)(BYTE a_bEnable);

private:
	float m_fR;
	float m_fG;
	float m_fB;
	float m_fH;
	float m_fL;
	float m_fS;
	float m_fA;
	BYTE m_bAlphaEnabled;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateColor), CSharedStateColor)
