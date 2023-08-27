
#pragma once

#include <Win32LangEx.h>
#include <ContextHelpDlg.h>
#include <SubjectImpl.h>
#include <WTL_ColorPicker.h>
#include <WTL_ColorGradient.h>
#include <WTL_2DPosition.h>
#include <InPlaceCalc.h>
#include <StringParsing.h>

// CScriptedConfiguration

class ATL_NO_VTABLE CScriptedConfiguration : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedConfiguration, &IID_IScriptedConfiguration, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>,
	public CSubjectImpl<IConfig, IConfigObserver, IUnknown*>,
	public IConfigCustomGUI
{
	typedef IDispatchImpl<IScriptedConfiguration, &IID_IScriptedConfiguration, &LIBID_RWScriptingLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;

public:
	CScriptedConfiguration() : m_fSplit(-1.0f)
	{
	}
	void Init(IConfig* a_pConfig, IScriptingInterfaceManager* a_pScriptingMgr)
	{
		m_pScriptingMgr = a_pScriptingMgr;
		m_pConfig = a_pConfig;
		CoCreateGuid(&m_tConfigID);
	}

DECLARE_NOT_AGGREGATABLE(CScriptedConfiguration)

BEGIN_COM_MAP(CScriptedConfiguration)
	COM_INTERFACE_ENTRY(IScriptedConfiguration)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConfig)
	COM_INTERFACE_ENTRY(IConfigCustomGUI)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDispatch methods
public:
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
	{
		try
		{
			CConfigValue cVal;
			CComBSTR bstr(*rgszNames);
			if (SUCCEEDED(m_pConfig->ItemValueGet(bstr, &cVal)))
			{
				for (CDispIDs::const_iterator i = m_cDispIDs.begin(); i != m_cDispIDs.end(); ++i)
				{
					if (_wcsicmp(*i, bstr) == 0)
					{
						*rgDispId = 100+(i-m_cDispIDs.begin());
						return S_OK;
					}
				}
				m_cDispIDs.push_back(bstr);
				*rgDispId = 99+m_cDispIDs.size();
				return S_OK;
			}
			return CDispatchBase::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
	{
		try
		{
			if (dispIdMember >= 100 && dispIdMember < DISPID(100+m_cDispIDs.size()))
			{
				if (wFlags&DISPATCH_PROPERTYGET)
				{
					return GetValue(m_cDispIDs[dispIdMember-100], pVarResult);
				}
				else if (wFlags&(DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF))
				{
					return SetValue(m_cDispIDs[dispIdMember-100], pDispParams->rgvarg[0]);
				}
				return DISP_E_MEMBERNOTFOUND;

				//pVarResult->vt = VT_DISPATCH;
				//(pVarResult->pdispVal = m_cInterfaces[dispIdMember-100].second)->AddRef();
				//return S_OK;
			}
			return CDispatchBase::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IScriptedConfiguration methods
public:
	STDMETHOD(GetValue)(BSTR id, VARIANT* pVal)
	{
		try
		{
			pVal->vt = VT_EMPTY;
			CConfigValue cVal;
			m_pConfig->ItemValueGet(id, &cVal);
			switch (cVal.TypeGet())
			{
			case ECVTInteger:
				pVal->vt = VT_I4;
				pVal->lVal = cVal;
				break;
			case ECVTFloat:
				pVal->vt = VT_R4;
				pVal->fltVal = cVal;
				break;
			case ECVTBool:
				pVal->vt = VT_BOOL;
				pVal->boolVal = cVal.operator bool() ? VARIANT_TRUE : VARIANT_FALSE;
				break;
			case ECVTString:
				pVal->vt = VT_BSTR;
				pVal->bstrVal = cVal.Detach().bstrVal;
				break;
			case ECVTGUID:
				pVal->vt = VT_BSTR;
				{
					OLECHAR sz[40];
					StringFromGUID(cVal.operator const GUID &(), sz);
					pVal->bstrVal = CComBSTR(sz).Detach();
				}
				break;
			case ECVTVector2:
				pVal->vt = VT_DISPATCH;
				{
					CComPtr<IJScriptArrayInit> p = NULL;
					m_pScriptingMgr->CreateJScriptArray(&p);
					p->AddFloat(cVal[0]);
					p->AddFloat(cVal[1]);
					pVal->pdispVal = p.Detach();
				}
				break;
			case ECVTVector3:
			case ECVTFloatColor:
				pVal->vt = VT_DISPATCH;
				{
					CComPtr<IJScriptArrayInit> p = NULL;
					m_pScriptingMgr->CreateJScriptArray(&p);
					p->AddFloat(cVal[0]);
					p->AddFloat(cVal[1]);
					p->AddFloat(cVal[2]);
					pVal->pdispVal = p.Detach();
				}
				break;
			case ECVTVector4:
				pVal->vt = VT_DISPATCH;
				{
					CComPtr<IJScriptArrayInit> p = NULL;
					m_pScriptingMgr->CreateJScriptArray(&p);
					p->AddFloat(cVal[0]);
					p->AddFloat(cVal[1]);
					p->AddFloat(cVal[2]);
					p->AddFloat(cVal[3]);
					pVal->pdispVal = p.Detach();
				}
				break;
			default:
				return S_FALSE;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetValueOrDefault)(BSTR id, VARIANT defVal, VARIANT* pVal)
	{
		try
		{
			HRESULT hRes = GetValue(id, pVal);
			if (hRes == S_OK)
				return S_OK;
			return CComVariant(defVal).Detach(pVal);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetValue)(BSTR id, VARIANT val)
	{
		try
		{
			CConfigValue cVal;
			m_pConfig->ItemValueGet(id, &cVal);
			CComVariant cVar;
			switch (cVal.TypeGet())
			{
			case ECVTInteger:
				if (val.vt == VT_BSTR)
				{
					CComPtr<IConfigItemOptions> pOptions;
					m_pConfig->ItemGetUIInfo(id, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pOptions));
					CComPtr<IEnumConfigItemOptions> pOpts;
					if (pOptions) pOptions->OptionsEnum(&pOpts);
					ULONG nOpts = 0;
					if (pOpts) pOpts->Size(&nOpts);
					ULONG i;
					for (i = 0; i < nOpts; ++i)
					{
						CConfigValue cOpt;
						CComPtr<ILocalizedString> pName;
						CComBSTR bstrOpt;
						if (SUCCEEDED(pOpts->Get(i, &cOpt)) &&
							SUCCEEDED(pOptions->ValueGetName(cOpt, &pName)) && pName &&
							SUCCEEDED(pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT), &bstrOpt)) &&
							bstrOpt == val.bstrVal)
						{
							cVal = cOpt;
							break;
						}
					}
					if (i < nOpts)
						break;
				}
				if (FAILED(cVar.ChangeType(VT_I4, &val)))
					return S_FALSE;
				cVal = cVar.lVal;
				break;
			case ECVTFloat:
				if (FAILED(cVar.ChangeType(VT_R4, &val)))
					return S_FALSE;
				cVal = cVar.fltVal;
				break;
			case ECVTBool:
				if (FAILED(cVar.ChangeType(VT_BOOL, &val)))
					return S_FALSE;
				cVal = cVar.boolVal != VARIANT_FALSE;
				break;
			case ECVTString:
				if (FAILED(cVar.ChangeType(VT_BSTR, &val)))
					return S_FALSE;
				cVal = cVar.bstrVal;
				break;
			case ECVTGUID:
				//if (val.vt == VT_CLSID)
				//{
				//	cVal = val
				//	break;
				//}
				if (FAILED(cVar.ChangeType(VT_BSTR, &val)))
					return S_FALSE;
				if (cVar.bstrVal != NULL && wcslen(cVar.bstrVal) >= 36)
				{
					GUID g = GUID_NULL;
					if (GUIDFromString(cVar.bstrVal[0] == L'{' ? cVar.bstrVal+1 : cVar.bstrVal, &g))
					{
						cVal = g;
						break;
					}
				}
				return S_FALSE;
			case ECVTVector2:
				if (FAILED(cVar.ChangeType(VT_BSTR, &val)))
					return S_FALSE;
				if (cVar.bstrVal != NULL)
				{
					float f1 = 0.0f, f2 = 0.0f;
					if (2 == swscanf(cVar.bstrVal, L"%f,%f", &f1, &f2))
					{
						cVal = CConfigValue(f1, f2);
						break;
					}
				}
				return S_FALSE;
			case ECVTFloatColor:
				if (val.vt == VT_I4 || val.vt == VT_UI4)
				{
					cVal = CConfigValue((val.intVal&0xff)/255.0f, ((val.intVal>>8)&0xff)/255.0f, ((val.intVal>>16)&0xff)/255.0f, true);
					break;
				}
			case ECVTVector3:
				if (FAILED(cVar.ChangeType(VT_BSTR, &val)))
					return S_FALSE;
				if (cVar.bstrVal != NULL)
				{
					float f1 = 0.0f, f2 = 0.0f, f3 = 0.0f;
					if (2 == swscanf(cVar.bstrVal, L"%f,%f,%f", &f1, &f2, &f3))
					{
						cVal = CConfigValue(f1, f2, f3, cVal.TypeGet() == ECVTFloatColor);
						break;
					}
				}
				return S_FALSE;
			case ECVTVector4:
				if (FAILED(cVar.ChangeType(VT_BSTR, &val)))
					return S_FALSE;
				if (cVar.bstrVal != NULL)
				{
					float f1 = 0.0f, f2 = 0.0f, f3 = 0.0f, f4 = 0.0f;
					if (2 == swscanf(cVar.bstrVal, L"%f,%f,%f,%f", &f1, &f2, &f3, &f4))
					{
						cVal = CConfigValue(f1, f2, f3, f4);
						break;
					}
				}
				return S_FALSE;
			default:
				return S_FALSE;
			}
			m_pConfig->ItemValuesSet(1, &id, cVal);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(put_Columns)(LONG columns)
	{
		ATLASSERT(FALSE); // deprecated - always 1 column
		return S_OK;
	}
	STDMETHOD(put_SplitPreview)(VARIANT pos)
	{
		if (pos.vt == VT_BOOL)
			m_fSplit = pos.boolVal ? 1.0f : -1.0f;
		else if (pos.vt == VT_INT || pos.vt == VT_UINT || pos.vt == VT_I4 || pos.vt == VT_UI4)
			m_fSplit = pos.intVal;
		else if (pos.vt == VT_R4)
			m_fSplit = pos.fltVal;
		else if (pos.vt == VT_R8)
			m_fSplit = pos.dblVal;
		if (m_fSplit > 1.0f) m_fSplit = 1.0f; else if (m_fSplit < 0.0f) m_fSplit = -1.0f;
		return S_OK;
	}
	STDMETHOD(AddStaticText)(BSTR text)
	{
		try
		{
			SConfigItem sItem;
			sItem.eType = ECITStaticText;
			sItem.bstrName = text;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddEditBox)(BSTR id, BSTR name, BSTR description, VARIANT initialValue, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal;
			switch (initialValue.vt)
			{
			case VT_I2:
			case VT_I4:
			case VT_I1:
			case VT_UI1:
			case VT_UI2:
			case VT_UI4:
			case VT_I8:
			case VT_UI8:
			case VT_INT:
			case VT_UINT:
				{
					CComVariant cVar;
					if (FAILED(cVar.ChangeType(VT_I4, &initialValue)))
						return E_INVALIDARG;
					cVal = cVar.lVal;
				}
				break;
			case VT_R4:
			case VT_R8:
			case VT_DECIMAL:
				{
					CComVariant cVar;
					if (FAILED(cVar.ChangeType(VT_R4, &initialValue)))
						return E_INVALIDARG;
					cVal = cVar.fltVal;
				}
				break;
			case VT_BSTR:
				cVal = initialValue.bstrVal;
				break;
			default:
				return E_INVALIDARG;
			}
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITEditBox;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddSlider)(BSTR id, BSTR name, BSTR description, LONG minimum, LONG maximum, LONG initialValue, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal(initialValue);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITSlider;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			sItem.nMin = minimum;
			sItem.nMax = maximum;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddCheckBox)(BSTR id, BSTR name, BSTR description, VARIANT_BOOL initialValue, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal(initialValue != VARIANT_FALSE);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITCheckBox;
			sItem.bstrName = name;
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Add1ofNPicker)(BSTR id, BSTR name, BSTR description, IDispatch* options, LONG initialIndex, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal(initialIndex);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITComboBox;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			sItem.pOptions = options;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddColorButton)(BSTR id, BSTR name, BSTR description, LONG initialRGB, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal(initialRGB);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITColorPicker;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddFilePath)(BSTR id, BSTR name, BSTR description, BSTR filter, BSTR initialPath, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal(initialPath);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITFilePath;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			sItem.bstrFilter = filter;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddAlphaColorButton)(BSTR id, BSTR name, BSTR description, ULONG initialARGB, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal((LONG)initialARGB);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITAlphaColorPicker;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddColorGradient)(BSTR id, BSTR name, BSTR description, BSTR initialGradient, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal(initialGradient);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITColorGradient;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(RenderGradient)(BSTR cfg, ULONG total, ULONG first, ULONG count, VARIANT type, IDispatch** colors)
	{
		try
		{
			if (colors == NULL || cfg == NULL || total == 0 || first >= total || first+count > total) return E_INVALIDARG;

			if (type.vt != VT_EMPTY && type.vt != VT_ERROR)
			{
				if ((type.vt == VT_UINT || type.vt == VT_INT || type.vt == VT_I1 || type.vt == VT_UI1 || type.vt == VT_I4 || type.vt == VT_UI4) && type.bVal == 0)
					;
				else
					return E_INVALIDARG;
			}

			CGradientColorPicker::CGradient gradient;
			if (!TextToGradient(cfg, gradient)) return E_INVALIDARG;

			std::vector<CButtonColorPicker::SColor> clrs;
			clrs.resize(count);
			CGradientColorPicker::RenderGradient(gradient, total, first, first+count, clrs.begin());

			CComPtr<IJScriptArrayInit> jsarray;
			m_pScriptingMgr->CreateJScriptArray(&jsarray);
			for (ULONG i = 0; i < count; ++i)
				jsarray->AddNumber(clrs[i].ToRGBA());

			*colors = jsarray.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AddAlignment)(BSTR id, BSTR name, BSTR description, float initialX, float initialY, VARIANT forceInitial)
	{
		try
		{
			CConfigValue cVal(initialX, initialY);
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(id, &cPrev);
			if (cPrev.TypeGet() != cVal.TypeGet() || (forceInitial.vt == VT_BOOL && forceInitial.boolVal == VARIANT_TRUE))
			{
				if (FAILED(m_pConfig->ItemValuesSet(1, &id, cVal)))
					return E_FAIL;
			}
			SConfigItem sItem;
			sItem.bstrID = id;
			sItem.eType = ECITAlignment;
			sItem.bstrName = name;
			sItem.bstrName += L":";
			sItem.bstrDesc = description;
			sItem.cDefault = cVal;
			m_cConfigItems.push_back(sItem);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IConfig methods
public:
	STDMETHOD(ItemIDsEnum)(IEnumStrings** a_ppIDs)
	{
		try
		{
			*a_ppIDs = NULL;
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			for (CConfigItems::const_iterator i = m_cConfigItems.begin(); i != m_cConfigItems.end(); ++i)
				if (i->eType != ECITStaticText)
					pTmp->Insert(i->bstrID);
			*a_ppIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppIDs ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ItemValueGet)(BSTR a_bstrID, TConfigValue* a_ptValue)
	{
		return m_pConfig->ItemValueGet(a_bstrID, a_ptValue);
	}
	STDMETHOD(ItemValuesSet)(ULONG a_nCount, BSTR* a_aIDs, TConfigValue const* a_atValues)
	{
		HRESULT hRes = m_pConfig->ItemValuesSet(a_nCount, a_aIDs, a_atValues);
		Fire_Notify(NULL);
		return hRes;
	}

	class ATL_NO_VTABLE CScriptedConfigItem : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IConfigItem
	{
	public:
		BEGIN_COM_MAP(CScriptedConfigItem)
			COM_INTERFACE_ENTRY(IConfigItem)
		END_COM_MAP()

		void Init(BSTR a_bstrName, BSTR a_bstrDesc, TConfigValue const& a_tDefault)
		{
			m_bstrName = a_bstrName;
			m_bstrDesc = a_bstrDesc;
			m_cDefault = a_tDefault;
		}

		// specific IConfigItem methods
	public:
		STDMETHOD(NameGet)(ILocalizedString** a_ppName, ILocalizedString** a_ppHelpText)
		{
			if (a_ppName)
			{
				*a_ppName = NULL;
				*a_ppName = new CSimpleLocalizedString(m_bstrName.Copy());
			}
			if (a_ppHelpText)
			{
				*a_ppHelpText = NULL;
				*a_ppHelpText = new CSimpleLocalizedString(m_bstrDesc.Copy());
			}
			return S_OK;
		}
		STDMETHOD(ValueGetName)(const TConfigValue* a_ptValue, ILocalizedString** a_ppName)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(ValueIsValid)(const TConfigValue* a_ptValue)
		{
			if (a_ptValue == NULL)
				return S_FALSE;

			return a_ptValue->eTypeID == m_cDefault.TypeGet() ? S_OK : S_FALSE;
		}
		STDMETHOD(Default)(TConfigValue* a_ptValue)
		{
			a_ptValue->eTypeID = ECVTEmpty;
			*a_ptValue = ConfigValueCopy(m_cDefault);
			return S_OK;
		}

	private:
		CComBSTR m_bstrName;
		CComBSTR m_bstrDesc;
		CConfigValue m_cDefault;
	};
	STDMETHOD(ItemGetUIInfo)(BSTR a_bstrID, REFIID a_iidInfo, void** a_ppItemInfo)
	{
		try
		{
			*a_ppItemInfo = NULL;
			if (!IsEqualGUID(a_iidInfo, __uuidof(IConfigItem)))
				return E_NOINTERFACE;
			for (CConfigItems::const_iterator i = m_cConfigItems.begin(); i != m_cConfigItems.end(); ++i)
			{
				if (i->bstrID == a_bstrID)
				{
					CComObject<CScriptedConfigItem>* p = NULL;
					CComObject<CScriptedConfigItem>::CreateInstance(&p);
					CComPtr<IConfigItem> pTmp = p;
					p->Init(i->bstrName, i->bstrDesc, i->cDefault);
					*a_ppItemInfo = pTmp.Detach();
					return S_OK;
				}
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SubConfigGet)(BSTR a_bstrID, IConfig** a_ppSubConfig)
	{
		return m_pConfig->SubConfigGet(a_bstrID, a_ppSubConfig);
	}
	STDMETHOD(DuplicateCreate)(IConfig** a_ppCopiedConfig)
	{
		return m_pConfig->DuplicateCreate(a_ppCopiedConfig);
	}
	STDMETHOD(CopyFrom)(IConfig* a_pSource, BSTR a_bstrIDPrefix)
	{
		HRESULT hRes = m_pConfig->CopyFrom(a_pSource, a_bstrIDPrefix);
		Fire_Notify(NULL);
		return hRes;
	}

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid)
	{
		*a_pguid = m_tConfigID;
		return S_OK;
	}
	STDMETHOD(RequiresMargins)()
	{
		return S_OK;
	}
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
	{
		try
		{
			SIZE tSize = { 125, 24 };
			static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
			if (tVersion.dwMajorVersion == 0)
				GetVersionEx(&tVersion);
			NONCLIENTMETRICS ncm;
			LPCTSTR pszFontName = NULL;
			WORD wFontSize = 0;
			if (tVersion.dwMajorVersion >= 6)
			{
				ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
				SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
				pszFontName = ncm.lfMessageFont.lfFaceName;
				static int nLogPixelsY = 0;
				if (nLogPixelsY == 0)
				{
					HDC hDC = ::GetDC(NULL);
					nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
					::ReleaseDC(NULL, hDC);
				}
				wFontSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight < 0 ? -ncm.lfMessageFont.lfHeight : ncm.lfMessageFont.lfHeight, 72, nLogPixelsY);
			}
			else
			{
				wFontSize = 8;
				pszFontName = _T("MS Shell Dlg");
			}
			_DialogSizeHelper::ConvertDialogUnitsToPixels(pszFontName, wFontSize, &tSize);
			ULONG nCols = 1;//m_nColumns;
			if (nCols > m_cConfigItems.size())
				nCols = m_cConfigItems.size();
			if (nCols > 4)
				nCols = 4;
			if (nCols < 1)
			{
				static int const s_aCols[] = {1, 1, 2, 3, 2, 3, 3};
				nCols = m_cConfigItems.size() > 6 ? 4 : s_aCols[m_cConfigItems.size()];
			}
			LONG nRows = (m_cConfigItems.size()+nCols-1)/nCols;
			if (m_cConfigItems.size())
			{
				int nGap = tSize.cy*4/24;
				*a_nSizeX = tSize.cx*nCols + nGap*(nCols-1);
				*a_nSizeY = tSize.cy*nRows + nGap*(nRows-1);
			}
			else
			{
				*a_nSizeX = 1;
				*a_nSizeY = 1;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
	{
		try
		{
			CComObject<CConfigGUIDlg>* pWnd = NULL;
			CComObject<CConfigGUIDlg>::CreateInstance(&pWnd);
			CComPtr<IChildWindow> pTmp = pWnd;

			pWnd->Create(a_hParent, a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_bParentBorder, a_pConfig, m_cConfigItems, m_fSplit);

			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

public:
	static void GradientToText(CGradientColorPicker::CGradient const& gradient, CComBSTR& str)
	{
		for (CGradientColorPicker::CGradient::const_iterator i = gradient.begin(); i != gradient.end(); ++i)
		{
			wchar_t psz[128];
			swprintf(psz, L"%i,%g,%g,%g,%g;", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA);
			str.Append(psz);
		}
	}
	static bool TextToGradient(wchar_t const* psz, CGradientColorPicker::CGradient& gradient)
	{
		while (true)
		{
			int wPos;
			CButtonColorPicker::SColor sColor;
			if (5 == swscanf(psz, L"%i,%g,%g,%g,%g;", &wPos, &sColor.fR, &sColor.fG, &sColor.fB, &sColor.fA))
				gradient[wPos] = sColor;
			else
				break;
			while (*psz && *psz != L';') ++psz;
			if (*psz)
				++psz;
			else
				break;
		}
		return gradient.find(0) != gradient.end() && gradient.find(0xffff) != gradient.end();
	}

private:
	enum EConfigItemType
	{
		ECITStaticText = 0,
		ECITEditBox,
		ECITSlider,
		ECITCheckBox,
		ECITComboBox,
		ECITColorPicker,
		ECITFilePath,
		ECITAlphaColorPicker,
		ECITColorGradient,
		ECITAlignment,
	};
	struct SConfigItem
	{
		CComBSTR bstrID;
		EConfigItemType eType;
		CComBSTR bstrName;
		CComBSTR bstrDesc;

		CConfigValue cDefault;
		CComBSTR bstrFilter;
		LONG nMin;
		LONG nMax;
		CComPtr<IDispatch> pOptions;
	};
	typedef std::vector<SConfigItem> CConfigItems;

	class ATL_NO_VTABLE CConfigGUIDlg :
		public CComObjectRootEx<CComMultiThreadModel>,
		public CChildWindowImpl<CConfigGUIDlg, IChildWindow>,
		public Win32LangEx::CLangIndirectDialogImpl<CConfigGUIDlg>//,
		//public CObserverImpl<CConfigGUIDlg, IConfigObserver, IUnknown*>
	{
	public:
		CConfigGUIDlg() : m_bEnableUpdates(false), m_nDirtyEditIDC(0), m_fSplit(-1.0f)
		{
		}
		~CConfigGUIDlg()
		{
			for (CColorButtons::iterator i = m_cColorButtons.begin(); i != m_cColorButtons.end(); ++i)
				delete i->second;
			for (CColorGradients::iterator i = m_cColorGradients.begin(); i != m_cColorGradients.end(); ++i)
			{
				delete i->second.first;
				delete i->second.second;
			}
			for (CAlignmentCtrls::iterator i = m_cAlignmentCtrls.begin(); i != m_cAlignmentCtrls.end(); ++i)
				delete i->second;
		}
		void Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, CConfigItems const& a_cItems, float a_fSplit)
		{
			m_tLocaleID = a_tLocaleID;
			m_pConfig = a_pConfig;
			m_cItems = a_cItems;
			m_fSplit = a_fSplit;
			m_nCols = 1;//a_nCols;
			if (m_nCols > m_cItems.size())
				m_nCols = m_cItems.size();
			if (m_nCols > 4)
				m_nCols = 4;
			if (m_nCols < 1)
			{
				static int const s_aCols[] = {1, 1, 2, 3, 2, 3, 3};
				m_nCols = m_cItems.size() > 6 ? 4 : s_aCols[m_cItems.size()];
			}

			Win32LangEx::CLangIndirectDialogImpl<CConfigGUIDlg>::Create(a_hParent);
			if (!IsWindow()) throw E_FAIL;

			MoveWindow(a_prcPositon);
			SetWindowLong(GWL_ID, a_nCtlID);
			ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
		}

		enum { IDC_BASE = 200, IDCsPerItem = 4 };

		BEGIN_DIALOG_EX(0, 0, 125, 24, 0)
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL)
			DIALOG_EXSTYLE(WS_EX_CONTROLPARENT)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
		END_CONTROLS_MAP()

		BEGIN_COM_MAP(CConfigGUIDlg)
			COM_INTERFACE_ENTRY(IChildWindow)
		END_COM_MAP()

		BEGIN_MSG_MAP(CConfigGUIDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_CODE_HANDLER(EN_CHANGE, OnEditChanged)
			COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnEditKillFocus)
			COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
			COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnSelectionChange)
			NOTIFY_CODE_HANDLER(CButtonColorPicker::BCPN_SELCHANGE, OnColorChange)
			NOTIFY_CODE_HANDLER(UDN_DELTAPOS, OnUpDownChange)
			NOTIFY_CODE_HANDLER(CGradientColorPicker::GCPN_ACTIVESTOPCHANGED, OnGradientStopChanged)
			NOTIFY_CODE_HANDLER(CRectanglePosition::C2DP_POSITION_CHANGED, OnAlignmentChanged)
			MESSAGE_HANDLER(WM_HELP, OnHelp)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_RW_CFGSPLIT, OnRWCfgSplit)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		void OwnerNotify(TCookie, IUnknown*)
		{
			try
			{
				// TODO: implement - low priority, it is not used under normal circumstances anyway
			}
			catch (...)
			{
			}
		}

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			bool bContxtTips = true;
			CComPtr<IGlobalConfigManager> pMgr;
			RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
			if (pMgr)
			{
				CComPtr<IConfig> pCfg;
				// hacks: copied CLSID and CFGVAL
				static CLSID const tID = {0x2e85563c, 0x4ff0, 0x4820, {0xa8, 0xba, 0x1b, 0x47, 0x63, 0xab, 0xcc, 0x1c}}; // CLSID_GlobalConfigMainFrame
				pMgr->Config(tID, &pCfg);
				CConfigValue cVal;
				if (pCfg) pCfg->ItemValueGet(CComBSTR(L"CtxHelpTips"), &cVal);
				if (cVal.TypeGet() == ECVTBool) bContxtTips = cVal;
			}

			RECT rcDims = {4, 4, 124, 24};
			MapDialogRect(&rcDims);
			rcDims.left = rcDims.top = rcDims.bottom*4/24;
			RECT rcMisc = {0, 8, 35, 12};
			MapDialogRect(&rcMisc);
			HFONT hFont = GetFont();
			for (CConfigItems::const_iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
			{
				HWND hHelpCtl = NULL;
				size_t nIndex = i-m_cItems.begin();
				int nRow = nIndex/m_nCols;
				int nCol = nIndex%m_nCols;
				UINT nIDC = IDC_BASE+nIndex*IDCsPerItem;
				RECT rc = {(rcDims.left+rcDims.right)*nCol, (rcDims.top+rcDims.bottom)*nRow};
				switch (i->eType)
				{
				case ECITEditBox:
					{
						CStatic wndLabel;
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);
						CEdit wndEdit;
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom;
						CConfigValue cVal;
						OLECHAR szTmp[32] = L"";
						LPOLESTR psz = szTmp;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						DWORD dwAlign = 0;
						switch (cVal.TypeGet())
						{
						case ECVTString:
							psz = cVal;
							break;
						case ECVTInteger:
							swprintf(szTmp, L"%i", cVal.operator LONG());
							dwAlign = ES_RIGHT;
							break;
						case ECVTFloat:
							swprintf(szTmp, L"%g", cVal.operator float());
							dwAlign = ES_RIGHT;
							break;
						}
						wndEdit.Create(m_hWnd, &rc, CW2CT(psz), WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|dwAlign, WS_EX_CLIENTEDGE, nIDC+1);
						wndEdit.SetFont(hFont);
						hHelpCtl = wndEdit;
						if (cVal.TypeGet() == ECVTInteger/* || cVal.TypeGet() == ECVTFloat*/)
						{
							CUpDownCtrl wndUD;
							wndUD.Create(m_hWnd, &rc, NULL, WS_CHILD|WS_VISIBLE|UDS_ALIGNRIGHT|UDS_ARROWKEYS|UDS_AUTOBUDDY|UDS_NOTHOUSANDS, 0, nIDC+2);
						}
					}
					break;
				case ECITSlider:
					{
						CStatic wndLabel;
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);
						CTrackBarCtrl wndSlider;
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom;
						CConfigValue cVal;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						RECT rcSlider = rc;
						rcSlider.right -= rcDims.left*5;
						--rcSlider.top;
						++rcSlider.bottom;
						wndSlider.Create(m_hWnd, &rcSlider, _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP|TBS_NOTICKS|TBS_TOP, 0, nIDC+1);
						wndSlider.SetRange(i->nMin, i->nMax);
						wndSlider.SetPos(cVal.operator LONG());
						hHelpCtl = wndSlider;
						RECT rcEdit = rc;
						rcEdit.left = rcSlider.right;
						TCHAR szNum[32];
						_swprintf(szNum, _T("%i"), cVal.operator LONG());
						CEdit wndEdit;
						wndEdit.Create(m_hWnd, &rcEdit, szNum, WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_RIGHT, WS_EX_CLIENTEDGE, nIDC+2);
						wndEdit.SetFont(hFont);
					}
					break;
				case ECITCheckBox:
					{
						CButton wndCheck;
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcDims.bottom;
						wndCheck.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX|BS_VCENTER, 0, nIDC);
						wndCheck.SetFont(hFont);
						CConfigValue cVal;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						if (cVal.operator bool())
							wndCheck.SetCheck(BST_CHECKED);
						hHelpCtl = wndCheck;
					}
					break;
				case ECITStaticText:
					{
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcDims.bottom;
						CStatic wndLabel = ::CreateWindowEx(0, /*WC_LINK*/_T("SysLink"), CW2T(i->bstrName), WS_VISIBLE|WS_CHILD|WS_TABSTOP, rc.left, rc.top,
							rc.right-rc.left, rc.bottom-rc.top, m_hWnd, (HMENU)nIDC, _pModule->get_m_hInst(), 0);
						if (wndLabel == NULL)
							wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);
					}
					break;
				case ECITComboBox:
					{
						CStatic wndLabel;
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);
						CComboBox wndCombo;
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom-1;
						CConfigValue cVal;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						wndCombo.Create(m_hWnd, &rc, _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST, 0, nIDC+1);
						wndCombo.SetFont(hFont);
						DISPPARAMS params;
						ZeroMemory(&params, sizeof params);
						CComVariant res;
						DISPID dl = 0;
						LPOLESTR ln = L"length";
						if (SUCCEEDED(i->pOptions->GetIDsOfNames(IID_NULL, &ln, 1, LOCALE_USER_DEFAULT, &dl)) &&
							SUCCEEDED(i->pOptions->Invoke(dl, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
						{
							res.ChangeType(VT_I4);
							LONG len = res.lVal;

							for (int j = 0; j < len; ++j)
							{
								OLECHAR szIndex[16];
								swprintf(szIndex, L"%i", j);
								LPOLESTR psz = szIndex;
								DISPID id = 0;
								res.Clear();
								if (SUCCEEDED(i->pOptions->GetIDsOfNames(IID_NULL, &psz, 1, LOCALE_USER_DEFAULT, &id)) &&
									SUCCEEDED(i->pOptions->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
								{
									if (SUCCEEDED(res.ChangeType(VT_BSTR)))
									{
										int k = wndCombo.AddString(CW2T(res.bstrVal));
										wndCombo.SetItemData(k, j);
										if (j == cVal.operator LONG())
											wndCombo.SetCurSel(k);
									}
								}
							}
						}
						hHelpCtl = wndCombo;
					}
					break;
				case ECITColorPicker:
				case ECITAlphaColorPicker:
					{
						CStatic wndLabel;
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);

						CButton wndButton;
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom;
						rc.right = rc.left+rcMisc.right;
						wndButton.Create(m_hWnd, rc, 0, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW, 0, nIDC+1);
						wndButton.SetFont(hFont);
						CButtonColorPicker* pCB = new CButtonColorPicker(i->eType == ECITAlphaColorPicker, m_tLocaleID);
						m_cColorButtons[nIDC+1] = pCB;
						pCB->SubclassWindow(wndButton);
						CConfigValue cVal;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						pCB->SetColor(CButtonColorPicker::SColor(cVal.operator LONG(), i->eType == ECITAlphaColorPicker));
						pCB->SetDefaultText(NULL);
						hHelpCtl = pCB->m_hWnd;
					}
					break;
				case ECITColorGradient:
					{
						CStatic wndLabel;
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);

						std::pair<CGradientColorPicker*, CButtonColorPicker*>& t = m_cColorGradients[nIDC];
						t.second = NULL;
						t.first = new CGradientColorPicker();
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom;
						rc.right -= rcMisc.right;
						t.first->Create(m_hWnd, &rc, NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0, nIDC+1);
						CButton wndButton;
						rc.left = rc.right;
						rc.right += rcMisc.right;
						wndButton.Create(m_hWnd, rc, 0, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW, 0, nIDC+2);
						wndButton.SetFont(hFont);
						t.second = new CButtonColorPicker(true, m_tLocaleID);
						t.second->SubclassWindow(wndButton);
						CConfigValue cVal;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						CGradientColorPicker::CGradient gradient;
						if (cVal.TypeGet() == ECVTString && cVal.operator BSTR() && TextToGradient(cVal, gradient))
						{
							t.first->SetGradient(gradient);
							t.second->SetColor(t.first->GetStopColor(0));
						}
						t.second->SetDefaultText(NULL);
						hHelpCtl = t.first->m_hWnd;
					}
					break;
				case ECITAlignment:
					{
						CStatic wndLabel;
						rc.right = rc.left+rcDims.right-rcDims.bottom;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);

						CRectanglePosition* pRP = new CRectanglePosition();
						rc.bottom = rc.top+rcDims.bottom;
						rc.right = rc.left+rcDims.right;
						rc.left = rc.right-rcDims.bottom;
						pRP->Create(m_hWnd, rc, NULL,  WS_CHILD|WS_VISIBLE|WS_TABSTOP, WS_EX_CLIENTEDGE, nIDC+1);
						m_cAlignmentCtrls[nIDC+1] = pRP;
						CConfigValue cVal;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						pRP->SetPosition(cVal[0], cVal[1]);
						hHelpCtl = pRP->m_hWnd;
					}
					break;
				case ECITFilePath:
					{
						CStatic wndLabel;
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.Create(m_hWnd, &rc, CW2CT(i->bstrName), WS_CHILD|WS_VISIBLE, 0, nIDC);
						wndLabel.SetFont(hFont);
						CEdit wndEdit;
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom;
						rc.right -= rcMisc.bottom;
						CConfigValue cVal;
						m_pConfig->ItemValueGet(i->bstrID, &cVal);
						wndEdit.Create(m_hWnd, &rc, CW2CT(cVal.operator BSTR()), WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_READONLY, WS_EX_CLIENTEDGE, nIDC+1);
						wndEdit.SetFont(hFont);
						hHelpCtl = wndEdit;
						rc.left = rc.right;
						rc.right += rcMisc.bottom;
						CButton wndBtn;
						wndBtn.Create(m_hWnd, &rc, _T("..."), WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0, nIDC+2);
						wndBtn.SetFont(hFont);
					}
					break;
				}
				if (!bContxtTips || hHelpCtl == NULL || i->bstrDesc == NULL || i->bstrDesc[0] == L'\0')
					continue;

				if (m_wndToolTip.m_hWnd == NULL)
				{
					m_wndToolTip.Create(m_hWnd);
					HDC hDC = ::GetDC(m_hWnd);
					int nWidth = 420 * GetDeviceCaps(hDC, LOGPIXELSX) / 96;
					::ReleaseDC(m_hWnd, hDC);
					m_wndToolTip.SetMaxTipWidth(nWidth);
				}
				COLE2T strDesc(i->bstrDesc);
				TOOLINFO tTI;
				ZeroMemory(&tTI, sizeof tTI);
				tTI.cbSize = TTTOOLINFO_V1_SIZE;
				tTI.hwnd = m_hWnd;
				tTI.uId = reinterpret_cast<UINT_PTR>(hHelpCtl);
				tTI.uFlags = TTF_PARSELINKS|TTF_SUBCLASS|TTF_IDISHWND;
				tTI.lpszText = strDesc;
				m_wndToolTip.AddTool(&tTI);
			}

			m_bEnableUpdates = true;

			return 1;
		}

		LRESULT OnRWCfgSplit(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (a_lParam)
				*reinterpret_cast<float*>(a_lParam) = m_fSplit;
			return 0;
		}

		LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
		{
			bool bIsVisible = (GetWindowLong(GWL_STYLE) & WS_VISIBLE) != 0;
			if (bIsVisible)
			{
				SetRedraw(FALSE);
			}
			RECT rcDims = {4, 4, 124, 24};
			MapDialogRect(&rcDims);
			rcDims.left = rcDims.top = rcDims.bottom*4/24;
			RECT rcMisc = {0, 8, 35, 12};
			MapDialogRect(&rcMisc);
			SIZE sz = {LOWORD(a_lParam), HIWORD(a_lParam)};
			rcDims.right = (sz.cx-(m_nCols-1)*rcDims.left)/m_nCols;
			for (CConfigItems::const_iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
			{
				size_t nIndex = i-m_cItems.begin();
				int nRow = nIndex/m_nCols;
				int nCol = nIndex%m_nCols;
				UINT nIDC = IDC_BASE+nIndex*IDCsPerItem;
				RECT rc = {(rcDims.left+rcDims.right)*nCol, (rcDims.top+rcDims.bottom)*nRow};
				switch (i->eType)
				{
				case ECITEditBox:
				case ECITSlider:
					{
						CWindow wndLabel = GetDlgItem(nIDC);
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.MoveWindow(&rc);
						CWindow wndUD = GetDlgItem(nIDC+2);
						CWindow wndEdit = GetDlgItem(nIDC+1);
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom;
						if (wndUD.m_hWnd)
						{
							RECT rc2 = {0, 0, 0, 0};
							wndUD.GetWindowRect(&rc2);
							rc2.right -= rc2.left;
							LONG nL = rc.left;
							rc.left = rc.right-rc2.right;
							wndUD.MoveWindow(&rc);
							rc.right = rc.left;
							rc.left = nL;
						}
						wndEdit.MoveWindow(&rc);
					}
					break;
				case ECITComboBox:
					{
						CWindow wndLabel = GetDlgItem(nIDC);
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.MoveWindow(&rc);
						CWindow wndSlider = GetDlgItem(nIDC+1);
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom-1;
						wndSlider.MoveWindow(&rc);
					}
					break;
				case ECITCheckBox:
				case ECITStaticText:
					{
						CWindow wnd = GetDlgItem(nIDC);
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcDims.bottom;
						wnd.MoveWindow(&rc);
					}
					break;
				case ECITColorPicker:
				case ECITAlphaColorPicker:
					{
						CWindow wndLabel = GetDlgItem(nIDC);
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.MoveWindow(&rc);
						CColorButtons::const_iterator iCB = m_cColorButtons.find(nIDC+1);
						if (iCB != m_cColorButtons.end())
						{
							rc.bottom = rc.top+rcDims.bottom;
							rc.top += rcMisc.bottom;
							rc.right = rc.left+rcMisc.right;
							iCB->second->MoveWindow(&rc);
						}
					}
					break;
				case ECITColorGradient:
					{
						CWindow wndLabel = GetDlgItem(nIDC);
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.MoveWindow(&rc);
						CColorGradients::const_iterator iCG = m_cColorGradients.find(nIDC);
						if (iCG != m_cColorGradients.end())
						{
							rc.bottom = rc.top+rcDims.bottom;
							rc.top += rcMisc.bottom;
							rc.right -= rcMisc.right;
							iCG->second.first->MoveWindow(&rc);
							rc.left = rc.right;
							rc.right += rcMisc.right;
							iCG->second.second->MoveWindow(&rc);
						}
					}
					break;
				case ECITFilePath:
					{
						CWindow wndLabel = GetDlgItem(nIDC);
						rc.right = rc.left+rcDims.right;
						rc.bottom = rc.top+rcMisc.top;
						wndLabel.MoveWindow(&rc);
						CWindow wndEdit = GetDlgItem(nIDC+1);
						rc.bottom = rc.top+rcDims.bottom;
						rc.top += rcMisc.bottom;
						rc.right -= rcMisc.bottom;
						wndEdit.MoveWindow(&rc);
						CWindow wndBtn = GetDlgItem(nIDC+2);
						rc.left = rc.right;
						rc.right += rcMisc.bottom;
						wndBtn.MoveWindow(&rc);
					}
					break;
				}
			}
			if (bIsVisible)
			{
				SetRedraw(TRUE);
				RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
			return 0;
		}

		LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
		{
			HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
			if (pHelpInfo->iContextType == HELPINFO_WINDOW)
			{
				ULONG iIndex = (pHelpInfo->iCtrlId-IDC_BASE)/IDCsPerItem;
				if (static_cast<UINT>(pHelpInfo->iCtrlId) >= IDC_BASE && iIndex < m_cItems.size() &&
					m_cItems[iIndex].bstrDesc && m_cItems[iIndex].bstrDesc[0])
				{
					COLE2CT str(m_cItems[iIndex].bstrDesc);
					RECT rcItem;
					::GetWindowRect(static_cast<HWND>(pHelpInfo->hItemHandle), &rcItem);
					HH_POPUP hhp;
					hhp.cbStruct = sizeof(hhp);
					hhp.hinst = _pModule->get_m_hInst();
					hhp.idString = 0;
					hhp.pszText = str;
					hhp.pt.x = rcItem.right;
					hhp.pt.y = rcItem.bottom;
					hhp.clrForeground = 0xffffffff;
					hhp.clrBackground = 0xffffffff;
					hhp.rcMargins.left = -1;
					hhp.rcMargins.top = -1;
					hhp.rcMargins.right = -1;
					hhp.rcMargins.bottom = -1;
					hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
					HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
					return 0;
				}
			}
			a_bHandled = FALSE;
			return 0;
		}
		LRESULT OnHScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (a_lParam == 0)
			{
				a_bHandled = FALSE;
				return 0;
			}
			UINT nIDC = ::GetWindowLong(reinterpret_cast<HWND>(a_lParam), GWLP_ID);
			ULONG iIndex = (nIDC-IDC_BASE)/IDCsPerItem;
			if (nIDC >= IDC_BASE && iIndex < m_cItems.size() &&
				m_cItems[iIndex].eType == ECITSlider)
			{
				CTrackBarCtrl wndSlider(reinterpret_cast<HWND>(a_lParam));
				LONG nPos = wndSlider.GetPos();
				TCHAR szNum[32];
				_swprintf(szNum, _T("%i"), nPos);
				m_bEnableUpdates = false;
				SetDlgItemText(nIDC+1, szNum);
				m_bEnableUpdates = true;
				SetConfigValue(iIndex, nPos);
			}
			return 0;
		}
		LRESULT OnColorChange(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& a_bHandled)
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			ULONG iIndex = (pClrBtn->hdr.idFrom-IDC_BASE)/IDCsPerItem;
			if (pClrBtn->hdr.idFrom >= IDC_BASE && iIndex < m_cItems.size())
			{
				if (m_cItems[iIndex].eType == ECITColorPicker || m_cItems[iIndex].eType == ECITAlphaColorPicker)
				{
					SetConfigValue(iIndex, LONG(m_cItems[iIndex].eType == ECITColorPicker ? pClrBtn->clr.ToCOLORREF() : pClrBtn->clr.ToRGBA()));
					return 0;
				}
				if (m_cItems[iIndex].eType == ECITColorGradient)
				{
					CColorGradients::const_iterator i = m_cColorGradients.find(IDC_BASE+iIndex*IDCsPerItem);
					if (i != m_cColorGradients.end())
					{
						i->second.first->SetStop(i->second.first->GetStop(), pClrBtn->clr);
						CComBSTR bstr;
						GradientToText(i->second.first->GetGradient(), bstr);
						TConfigValue val;
						val.eTypeID = ECVTString;
						val.bstrVal = bstr;
						SetConfigValue(iIndex, val);
					}
					return 0;
				}
			}
			a_bHandled = FALSE;
			return 0;
		}
		LRESULT OnGradientStopChanged(int, LPNMHDR a_pHdr, BOOL& a_bHandled)
		{
			CGradientColorPicker::NMGRADIENT* pGrad = reinterpret_cast<CGradientColorPicker::NMGRADIENT*>(a_pHdr);
			ULONG iIndex = (pGrad->hdr.idFrom-IDC_BASE)/IDCsPerItem;
			if (pGrad->hdr.idFrom >= IDC_BASE && iIndex < m_cItems.size() && m_cItems[iIndex].eType == ECITColorGradient)
			{
				CColorGradients::const_iterator i = m_cColorGradients.find(IDC_BASE+iIndex*IDCsPerItem);
				if (i != m_cColorGradients.end())
				{
					i->second.second->SetColor(pGrad->clr);
					CComBSTR bstr;
					GradientToText(i->second.first->GetGradient(), bstr);
					TConfigValue val;
					val.eTypeID = ECVTString;
					val.bstrVal = bstr;
					SetConfigValue(iIndex, val);
					return 0;
				}
			}

			a_bHandled = FALSE;
			return 0;
		}
		LRESULT OnAlignmentChanged(int, LPNMHDR a_pHdr, BOOL& a_bHandled)
		{
			ULONG iIndex = (a_pHdr->idFrom-IDC_BASE)/IDCsPerItem;
			if (a_pHdr->idFrom >= IDC_BASE && iIndex < m_cItems.size() && m_cItems[iIndex].eType == ECITAlignment)
			{
				CAlignmentCtrls::const_iterator i = m_cAlignmentCtrls.find(IDC_BASE+iIndex*IDCsPerItem+1);
				if (i != m_cAlignmentCtrls.end())
				{
					SetConfigValue(iIndex, CConfigValue(i->second->GetPositionX(), i->second->GetPositionY()));
					return 0;
				}
			}

			a_bHandled = FALSE;
			return 0;
		}
		LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& a_bHandled)
		{
			LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);
			ULONG iIndex = (a_pNMHDR->idFrom-IDC_BASE)/IDCsPerItem;
			if (a_pNMHDR->idFrom >= IDC_BASE && iIndex < m_cItems.size() &&
				m_cItems[iIndex].eType == ECITEditBox)
			{
				TCHAR szPrev[64] = _T("");
				GetDlgItemText(a_pNMHDR->idFrom-1, szPrev, 64);
				szPrev[63] = _T('\0');
				float fPrev = 0.0f;
				LPCTSTR p = szPrev;
				double d = CInPlaceCalc::EvalExpression(szPrev, &p);
				if (p != szPrev && *p == L'\0')
					fPrev = float(d);
				fPrev += (pNMUD->iDelta > 0) ? -1.0f : 1.0f;
				_stprintf(szPrev, _T("%g"), fPrev);
				SetDlgItemText(a_pNMHDR->idFrom-1, szPrev);
			}
			else
			{
				a_bHandled = FALSE;
			}

			return 0;
		}
		LRESULT OnButtonClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			ULONG iIndex = (a_wID-IDC_BASE)/IDCsPerItem;
			if (a_wID >= IDC_BASE && iIndex < m_cItems.size() &&
				m_cItems[iIndex].eType == ECITCheckBox)
			{
				CButton wndCheckBox(a_hWndCtl);
				bool bChecked = wndCheckBox.GetCheck() == BST_CHECKED;
				SetConfigValue(iIndex, bChecked);
			}
			else if (a_wID >= IDC_BASE && iIndex < m_cItems.size() &&
				m_cItems[iIndex].eType == ECITFilePath)
			{
				CComPtr<IStorageManager> pStMgr;
				RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
				CConfigValue cVal;
				m_pConfig->ItemValueGet(m_cItems[iIndex].bstrID, &cVal);
				CComPtr<IEnumUnknownsInit> pFilters;
				if (m_cItems[iIndex].bstrFilter.Length())
				{
					LPCTSTR pszPipe = wcschr(m_cItems[iIndex].bstrFilter, L'|');
					if (pszPipe)
					{
						CComPtr<IDocumentTypeWildcards> pType;
						RWCoCreateInstance(pType, __uuidof(DocumentTypeWildcards));
						pType->Init(new CSimpleLocalizedString(SysAllocStringLen(m_cItems[iIndex].bstrFilter.m_str, pszPipe-m_cItems[iIndex].bstrFilter.m_str)), CComBSTR(pszPipe+1));
						RWCoCreateInstance(pFilters, __uuidof(EnumUnknowns));
						pFilters->Insert(pType);
					}
				}
				CComPtr<IStorageFilter> pLoc;
				static GUID const tID = {0x4683ebb1, 0xabf9, 0x4883, {0x8c, 0x94, 0x56, 0x63, 0xd8, 0xde, 0xde, 0xe}};
				pStMgr->FilterCreateInteractivelyUID(cVal, EFTOpenExisting, m_hWnd, pFilters, NULL, tID, NULL, NULL, m_tLocaleID, &pLoc);
				if (pLoc)
				{
					CComBSTR bstr;
					pLoc->ToText(NULL, &bstr);
					SetConfigValue(iIndex, CConfigValue(bstr.m_str));
					SetDlgItemText(IDC_BASE+iIndex*IDCsPerItem+1, COLE2CT(bstr));
				}
			}
			else
			{
				a_bHandled = FALSE;
			}
			return 0;
		}
		LRESULT OnSelectionChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			ULONG iIndex = (a_wID-IDC_BASE)/IDCsPerItem;
			if (a_wID >= IDC_BASE && iIndex < m_cItems.size() &&
				m_cItems[iIndex].eType == ECITComboBox)
			{
				CComboBox wndComboBox(a_hWndCtl);
				int iSel = wndComboBox.GetCurSel();
				if (iSel >= 0)
				{
					LONG index = wndComboBox.GetItemData(iSel);
					SetConfigValue(iIndex, index);
				}
			}
			else
			{
				a_bHandled = FALSE;
			}
			return 0;
		}
		LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			KillTimer(71);
			if (m_wndToolTip.IsWindow())
				m_wndToolTip.DestroyWindow();
			a_bHandled = FALSE;
			return 0;
		}
		LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			UpdateEditedValue();
			return 0;
		}
		LRESULT OnEditChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			ULONG iIndex = (a_wID-IDC_BASE)/IDCsPerItem;
			if (a_wID >= IDC_BASE && iIndex < m_cItems.size() &&
				(m_cItems[iIndex].eType == ECITEditBox || m_cItems[iIndex].eType == ECITSlider))
			{
				if (m_bEnableUpdates)
				{
					if (m_nDirtyEditIDC && m_nDirtyEditIDC != a_wID)
						UpdateEditedValue();
					m_nDirtyEditIDC = a_wID;
					SetTimer(71, 750);
				}
			}
			else
			{
				a_bHandled = FALSE;
			}
			return 0;
		}
		LRESULT OnEditKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			ULONG iIndex = (a_wID-IDC_BASE)/IDCsPerItem;
			if (a_wID >= IDC_BASE && iIndex < m_cItems.size() &&
				m_cItems[iIndex].eType == ECITEditBox)
			{
				if (m_bEnableUpdates)
				{
					if (m_nDirtyEditIDC && m_nDirtyEditIDC != a_wID)
						UpdateEditedValue();
					m_nDirtyEditIDC = a_wID;
					UpdateEditedValue();
				}
			}
			else if (a_wID >= IDC_BASE && iIndex < m_cItems.size() &&
				m_cItems[iIndex].eType == ECITFilePath)
			{
				CComBSTR bstr;
				GetDlgItemText(a_wID, bstr.m_str);
				if (bstr == NULL) bstr = L"";
				SetConfigValue(iIndex, bstr.m_str);
			}
			else
			{
				a_bHandled = FALSE;
			}
			return 0;
		}
		void UpdateEditedValue()
		{
			if (m_bEnableUpdates && m_hWnd)
			{
				m_bEnableUpdates = false;
				KillTimer(71);
				if (m_nDirtyEditIDC)
				{
					int iIndex = (m_nDirtyEditIDC-IDC_BASE)/IDCsPerItem;
					CComBSTR bstr;
					GetDlgItemText(m_nDirtyEditIDC, bstr.m_str);
					if (bstr == NULL)
						bstr = L"";
					CConfigValue cVal;
					m_pConfig->ItemValueGet(m_cItems[iIndex].bstrID, &cVal);
					switch (cVal.TypeGet())
					{
					case ECVTString:
						SetConfigValue(iIndex, bstr.m_str);
						break;
					case ECVTInteger:
						{
							LONG iVal = cVal;
							LPCOLESTR p = bstr;
							double d = CInPlaceCalc::EvalExpression(bstr, &p);
							if (p != bstr && *p == L'\0')
								iVal = LONG(floor(d+0.5));
							if (m_cItems[iIndex].eType == ECITSlider)
							{
								CTrackBarCtrl wndSlider(GetDlgItem(IDC_BASE+iIndex*IDCsPerItem+1));
								wndSlider.SetPos(iVal);
								iVal = wndSlider.GetPos();
								SetConfigValue(iIndex, iVal);
							}
							SetConfigValue(iIndex, iVal);
						}
						break;
					case ECVTFloat:
						{
							float fVal = cVal;
							LPCOLESTR p = bstr;
							double d = CInPlaceCalc::EvalExpression(bstr, &p);
							if (p != bstr && *p == L'\0')
								fVal = float(d);
							SetConfigValue(iIndex, fVal);
						}
						break;
					}
				}
				m_bEnableUpdates = true;
				m_nDirtyEditIDC = 0;
			}
		}

	private:
		void SetConfigValue(int iIndex, CConfigValue const& a_cVal)
		{
			CConfigValue cPrev;
			m_pConfig->ItemValueGet(m_cItems[iIndex].bstrID, &cPrev);
			if (cPrev != a_cVal)
			{
				m_pConfig->ItemValuesSet(1, &(m_cItems[iIndex].bstrID), a_cVal);
			}
		}

	private:
		typedef std::map<int, CButtonColorPicker*> CColorButtons;
		typedef std::map<int, std::pair<CGradientColorPicker*, CButtonColorPicker*> > CColorGradients;
		typedef std::map<int, CRectanglePosition*> CAlignmentCtrls;

	private:
		CComPtr<IConfig> m_pConfig;
		bool m_bEnableUpdates;
		UINT m_nDirtyEditIDC;
		float m_fSplit;

		CConfigItems m_cItems;
		ULONG m_nCols;
		CColorButtons m_cColorButtons;
		CColorGradients m_cColorGradients;
		CAlignmentCtrls m_cAlignmentCtrls;
		CToolTipCtrl m_wndToolTip;
	};

	typedef std::vector<CComBSTR> CDispIDs;

private:
	CComPtr<IScriptingInterfaceManager> m_pScriptingMgr;
	float m_fSplit;
	CComPtr<IConfig> m_pConfig;
	CConfigItems m_cConfigItems;
	GUID m_tConfigID;
	CDispIDs m_cDispIDs;
};


