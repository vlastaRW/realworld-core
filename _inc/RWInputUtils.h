
#pragma once

#ifdef __cplusplus

} // pause: extern "C"{


template<class T, CLSID const* t_pBuilderID, EUndoMode a_tDefaultUndoMode = EUMDefault, class TBase = IDocumentData>
class ATL_NO_VTABLE CDocumentDataImpl : public TBase
{
public:
	typedef CDocumentDataImpl<T, t_pBuilderID, a_tDefaultUndoMode, TBase> DataImplClass;
	typedef CReadLock<DataImplClass> CDocumentReadLock;
	typedef CWriteLock<DataImplClass> CDocumentWriteLock;

	CDocumentDataImpl()
	{
	}

	//IStorageFilter* M_Location() const // must be locked externally
	//{
	//	return m_pLocation;
	//}

	HRESULT WriteLock()		{ return m_pBase ? m_pBase->WriteLock() : S_OK; }
	HRESULT WriteUnlock()	{ return m_pBase ? m_pBase->WriteUnlock() : S_OK; }
	HRESULT ReadLock()		{ return m_pBase ? m_pBase->ReadLock() : S_OK; }
	HRESULT ReadUnlock()	{ return m_pBase ? m_pBase->ReadUnlock() : S_OK; }
	HRESULT UndoEnabled() const	{ return m_pBase ? m_pBase->UndoEnabled() : S_FALSE; }
	HRESULT UndoStep(IDocumentUndoStep* a_pStep)
	{ return m_pBase ? m_pBase->UndoStep(a_pStep) : E_UNEXPECTED; }

	// IDocumentData methods
public:
	STDMETHOD(Init)(IDocumentBase* a_pCallback, BSTR a_bstrID)
	{
		m_pBase = a_pCallback;
		m_bstrDataID = a_bstrID;
		return S_OK;
	}
	STDMETHOD(BuilderID)(CLSID* a_pguidBuilder)
	{
		try
		{
			*a_pguidBuilder = *t_pBuilderID;
			return S_OK;
		}
		catch (...)
		{
			return a_pguidBuilder ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Aspects)(IEnumEncoderAspects* /*a_pEnumAspects*/)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DataCopy)(BSTR /*a_bstrPrefix*/, IDocumentBase* /*a_pBase*/, CLSID* /*a_tPreviewEffectID*/, IConfig* /*a_pPreviewEffect*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(QuickInfo)(ULONG /*a_nInfoIndex*/, ILocalizedString** /*a_ppInfo*/)
	{
		return E_NOTIMPL;
	}

	EUndoMode DefaultUndoModeHelper() { return a_tDefaultUndoMode; }
	STDMETHOD(DefaultUndoMode)(EUndoMode* a_peDefaultMode)
	{
		if (a_tDefaultUndoMode == EUMDefault)
			return E_NOTIMPL;
		try
		{
			*a_peDefaultMode = static_cast<T*>(this)->DefaultUndoModeHelper();
			return S_OK;
		}
		catch (...)
		{
			return a_peDefaultMode ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(MaximumUndoSize)(ULONGLONG* /*a_pnMaximumSize*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(LocationChanged)(IStorageFilter* /*a_pOldLoc*/)
	{
		return S_FALSE;
	}
	STDMETHOD(RemovingBlock)()
	{
		return S_FALSE;
	}
	STDMETHOD(ComponentFeatureOverride)(BSTR /*a_bstrID*/, REFIID /*a_iid*/, void** /*a_ppFeatureInterface*/)
	{
		return S_FALSE;
	}
	STDMETHOD(ComponentLocationGet)(BSTR /*a_bstrID*/, IStorageFilter* /*a_pThisLoc*/, IStorageFilter** /*a_ppComponentLoc*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ResourcesManage)(EDocumentResourceManager /*a_eActions*/, ULONGLONG* /*a_pValue*/)
	{
		return E_NOTIMPL;
	}

	IDocumentBase* M_Base() const { return m_pBase; }
	BSTR M_DataID() const { return m_bstrDataID; }
	operator IBlockOperations*() const { return m_pBase; }

private:
	CComPtr<IDocumentBase> m_pBase;
	CComBSTR m_bstrDataID;
};

template<class T, size_t t_nFeatures, IID const* t_apFeatures, class TDocumentTypeCreator, class TBase = CSubjectImpl<CBlockOperationsSimpleImpl<T, IDocument>, IDocumentObserver, ULONG> >
class ATL_NO_VTABLE CDocumentImpl : public TBase
{
public:
	typedef CReadLock<TBase> CDocumentReadLock;
	typedef CWriteLock<TBase> CDocumentWriteLock;

	CDocumentImpl() : m_nChangeFlags(0)
	{
	}

	void AddDocumentChanges(ULONG a_nChanges)
	{
		m_nChangeFlags |= a_nChanges;
	}
	void Fire_NotifyDocument()
	{
		if (m_nChangeFlags)
		{
			this->Fire_Notify(m_nChangeFlags);
			m_nChangeFlags = 0;
		}
	}
	IStorageFilter* M_Location() const // must be locked externally
	{
		return m_pLocation;
	}

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* /*a_pguidBuilder*/) { return E_NOTIMPL; }
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
	{
		return static_cast<T*>(this)->QueryInterface(a_iid, a_ppFeatureInterface);
	}

	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)
	{
		try
		{
			*a_ppLocation = NULL;

			CDocumentReadLock cLock(this);
			if (m_pLocation)
			{
				(*a_ppLocation = m_pLocation)->AddRef();
			}

			return (*a_ppLocation) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(LocationSet)(IStorageFilter* a_pLocation)
	{
		try
		{
			CDocumentWriteLock cLock(this);
			if (m_pLocation != a_pLocation)
			{
				m_pLocation = a_pLocation;
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(EncoderGet)(CLSID* /*a_pEncoderID*/, IConfig** /*a_ppConfig*/) { return E_NOTIMPL; }
	STDMETHOD(EncoderSet)(REFCLSID /*a_tEncoderID*/, IConfig* /*a_pConfig*/) { return E_NOTIMPL; }
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* /*a_pEnumAspects*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IsDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ClearDirty)()
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DocumentCopy)(BSTR /*a_bstrPrefix*/, IDocumentBase* /*a_pBase*/, IConfig* /*a_pPreviewEffect*/)
	{
		return E_NOTIMPL;
		//try
		//{
		//	*a_ppCopy = NULL;
		//	(*a_ppCopy = this)->AddRef();
		//	return S_OK;
		//}
		//catch (...)
		//{
		//	return a_ppCopy ? E_UNEXPECTED : E_POINTER;
		//}
	}
	STDMETHOD(QuickInfo)(ULONG /*a_nInfoIndex*/, ILocalizedString** /*a_ppInfo*/)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DocumentTypeGet)(IDocumentType** a_ppDocumentType)
	{
		try
		{
			*a_ppDocumentType = TDocumentTypeCreator::Create();
			return *a_ppDocumentType ? S_OK : E_NOTIMPL;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}

	void InternSetLocation(IStorageFilter* a_pLocation)
	{
		// caller must catch exceptios
		CDocumentWriteLock cLock(this);
		// TODO: correct locks
		if (m_pLocation != a_pLocation)
		{
			m_pLocation = a_pLocation;
			AddDocumentChanges(EDCLocation);
		}
	}

private:
	CComPtr<IStorageFilter> m_pLocation;
	ULONG m_nChangeFlags;
};

// checks if document supports all features
inline bool SupportsAllFeatures(ULONG a_nFeaturesSupported, IID const* a_aFeaturesSupported, ULONG a_nFeaturesRequired, IID const* a_aFeaturesRequired)
{
	for (ULONG iReq = 0; iReq != a_nFeaturesRequired; iReq++)
	{
		ULONG iSup;
		for (iSup = 0; iSup != a_nFeaturesSupported && !IsEqualIID(a_aFeaturesRequired[iReq], a_aFeaturesSupported[iSup]); iSup++) ;

		if (iSup == a_nFeaturesSupported)
			return false;
	}
	return true;
}
inline bool SupportsAllFeatures(IDocument* a_pDocument, ULONG a_nCount, IID const* a_aFeaturesRequired)
{
	try
	{
		for (ULONG i = 0; i < a_nCount; ++i)
		{
			CComPtr<IUnknown> p;
			a_pDocument->QueryFeatureInterface(a_aFeaturesRequired[i], reinterpret_cast<void**>(&p));
			if (p == NULL)
				return false;
		}
		return true;
	}
	catch (...)
	{
		return false;
	}
}

template<class T, size_t t_nFeatures, IID const* t_apFeatures, wchar_t const* const t_pszTypeNameID, UINT t_nIconID, wchar_t const* const t_pszFormatNameID = NULL, wchar_t const* const t_pszIconPath = NULL, ULONG t_nPriority = EDPAverage, class TBase = IDocumentBuilder>
class ATL_NO_VTABLE CDocumentBuilderImpl : public TBase
{
	// IDocumentBuilder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority)
	{
		try
		{
			*a_pnPriority = t_nPriority;
			return S_OK;
		}
		catch (...)
		{
			return a_pnPriority == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(TypeName)(ILocalizedString** a_ppType)
	{
		try
		{
			*a_ppType = NULL;
			*a_ppType = new CMultiLanguageString(t_pszTypeNameID);
			return S_OK;
		}
		catch (...)
		{
			return a_ppType ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			if (t_nIconID == 0) return E_NOTIMPL;
			*a_phIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_nIconID), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return S_OK;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(FormatInfo)(ILocalizedString** a_ppFormat, BSTR* a_pbstrShellIcon)
	{
		try
		{
			if (a_ppFormat)
				*a_ppFormat = NULL;
			if (a_pbstrShellIcon)
				*a_pbstrShellIcon = NULL;
			if (a_ppFormat && t_pszFormatNameID)
				*a_ppFormat = new CMultiLanguageString(t_pszFormatNameID);
			if (a_pbstrShellIcon && t_pszIconPath)
			{
				if (wcsncmp(L"%MODULE%", t_pszIconPath, 8) == 0)
				{
					OLECHAR szModuleName[MAX_PATH+MAX_PATH] = L"";
					GetModuleFileName(_pModule->get_m_hInst(), szModuleName, MAX_PATH);
					wcscat(szModuleName, t_pszIconPath+8);
					*a_pbstrShellIcon = SysAllocString(szModuleName);
				}
				else
				{
					*a_pbstrShellIcon = SysAllocString(t_pszIconPath);
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(HasFeatures)(ULONG a_nCount, IID const* a_aiidRequired)
	{
		try
		{
			return SupportsAllFeatures(t_nFeatures, t_apFeatures, a_nCount, a_aiidRequired) ? S_OK : S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

template<class T, class TDocumentTypeCreator, class TBuilder, ULONG t_nPriority = EDPAverage, class TBase = IDocumentDecoder>
class ATL_NO_VTABLE CDocumentDecoderImpl : public TBase
{
	// IDocumentDecoder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority)
	{
		try
		{
			*a_pnPriority = t_nPriority;
			return S_OK;
		}
		catch (...)
		{
			return a_pnPriority == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocumentType)
	{
		try
		{
			*a_ppDocumentType = NULL;
			*a_ppDocumentType = TDocumentTypeCreator::Create();
			return S_OK;
		}
		catch (...)
		{
			return a_ppDocumentType == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(IsCompatible)(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders)
	{
		try
		{
			for (ULONG i = 0; i < a_nBuilders; ++i)
			{
				CComPtr<TBuilder> p;
				a_apBuilders[i]->QueryInterface(__uuidof(TBuilder), reinterpret_cast<void**>(&p));
				if (p)
					return S_OK;
			}
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Parse)(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl)
	{
		try
		{
			for (ULONG i = 0; i < a_nBuilders; ++i)
			{
				CComPtr<TBuilder> p;
				a_apBuilders[i]->QueryInterface(__uuidof(TBuilder), reinterpret_cast<void**>(&p));
				if (p)
					return static_cast<T*>(this)->Parse(a_nLen, a_pData, a_pLocation, p, a_bstrPrefix, a_pBase, a_pEncoderID, a_ppEncoderCfg, a_pControl);
			}
			return E_RW_INVALIDPARAM;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	HRESULT Parse(IDataSrcDirect* a_pDataSource, TBuilder* a_pBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
};

struct ATL_NO_VTABLE IDocumentCodec : public IDocumentDecoder, public IDocumentEncoder {};

#define FEATURES(feats) sizeof(feats)/sizeof(feats[0]), feats

struct CDocumentTypeCreatorNone
{
	static IDocumentType* Create()
	{
		return NULL;
	}
};

template<UINT t_nFilterNameID, UINT t_nTypeNameID, LPCOLESTR t_pszSupportedExtensions, UINT t_nFilterIconID, LPCOLESTR t_pszShellIconPath>
struct CDocumentTypeCreatorWildchars
{
	static IDocumentType* Create()
	{
		size_t nExtensions = 0;
		BSTR* pExtensions = NULL;
		CComBSTR bstrFilter;
		if (t_pszSupportedExtensions != NULL)
		{
			nExtensions = 1;
			size_t i;
			for (i = 0; t_pszSupportedExtensions[i]; ++i)
			{
				if (t_pszSupportedExtensions[i] == L'|')
				{
					++nExtensions;
				}
			}
			pExtensions = new BSTR[nExtensions];
			for (i = 0; i < nExtensions; ++i)
			{
				pExtensions[i] = NULL;
			}
			size_t ii = 0;
			LPCOLESTR psz = t_pszSupportedExtensions;
			for (i = 0; t_pszSupportedExtensions[i]; ++i)
			{
				if (t_pszSupportedExtensions[i] == L'|')
				{
					pExtensions[ii] = ::SysAllocStringLen(psz, t_pszSupportedExtensions+i-psz);
					psz = t_pszSupportedExtensions+i+1;
					++ii;
				}
			}
			pExtensions[ii] = ::SysAllocStringLen(psz, t_pszSupportedExtensions+i-psz);
			for (i = 0; i < nExtensions; ++i)
			{
				if (bstrFilter == NULL)
				{
					bstrFilter = L"*.";
				}
				else
				{
					bstrFilter += L";*.";
				}
				bstrFilter += pExtensions[i];
			}
		}

		TCHAR szModuleName[MAX_PATH] = _T("");
		GetModuleFileName(_pModule->get_m_hInst(), szModuleName, MAX_PATH);
		CT2W cModuleName(szModuleName);
		OLECHAR const* pszModuleName = cModuleName;
		OLECHAR szShellIcon[MAX_PATH+32];
		int iSrc = 0;
		int iDst = 0;
		if (t_pszShellIconPath) while (t_pszShellIconPath[iSrc] && iDst < (MAX_PATH+32-1))
		{
			if (0 == wcsncmp(t_pszShellIconPath+iSrc, L"%MODULE%", 8))
			{
				int i = 0;
				while (szModuleName[i] && iDst < (MAX_PATH+32-1))
				{
					szShellIcon[iDst++] = pszModuleName[i++];
				}
				iSrc += 8;
			}
			else
			{
				szShellIcon[iDst] = t_pszShellIconPath[iSrc];
				++iDst;
				++iSrc;
			}
		}
		szShellIcon[iDst] = L'\0';

		CComPtr<IDocumentTypeWildcards> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(DocumentTypeWildcards));
		pTmp->InitEx(
			_SharedStringTable.GetStringAuto(t_nFilterNameID),
			_SharedStringTable.GetStringAuto(t_nTypeNameID),
			nExtensions, pExtensions,
			iDst == 0 ? NULL : CComBSTR(szShellIcon),
			_pModule->get_m_hInst(), t_nFilterIconID,
			bstrFilter);

		for (size_t i = 0; i < nExtensions; ++i)
			::SysFreeString(pExtensions[i]);
		delete[] pExtensions;

		return pTmp.Detach();
	}
};

template<wchar_t const* const t_pszFilterNameID, wchar_t const* const t_pszTypeNameID, LPCOLESTR t_pszSupportedExtensions, UINT t_nFilterIconID, LPCOLESTR t_pszShellIconPath>
struct CDocumentTypeCreatorWildchars2
{
	static IDocumentType* Create()
	{
		size_t nExtensions = 0;
		BSTR* pExtensions = NULL;
		CComBSTR bstrFilter;
		if (t_pszSupportedExtensions != NULL)
		{
			nExtensions = 1;
			size_t i;
			for (i = 0; t_pszSupportedExtensions[i]; ++i)
			{
				if (t_pszSupportedExtensions[i] == L'|')
				{
					++nExtensions;
				}
			}
			pExtensions = new BSTR[nExtensions];
			for (i = 0; i < nExtensions; ++i)
			{
				pExtensions[i] = NULL;
			}
			size_t ii = 0;
			LPCOLESTR psz = t_pszSupportedExtensions;
			for (i = 0; t_pszSupportedExtensions[i]; ++i)
			{
				if (t_pszSupportedExtensions[i] == L'|')
				{
					pExtensions[ii] = ::SysAllocStringLen(psz, t_pszSupportedExtensions+i-psz);
					psz = t_pszSupportedExtensions+i+1;
					++ii;
				}
			}
			pExtensions[ii] = ::SysAllocStringLen(psz, t_pszSupportedExtensions+i-psz);
			for (i = 0; i < nExtensions; ++i)
			{
				if (bstrFilter == NULL)
				{
					bstrFilter = L"*.";
				}
				else
				{
					bstrFilter += L";*.";
				}
				bstrFilter += pExtensions[i];
			}
		}

		TCHAR szModuleName[MAX_PATH] = _T("");
		GetModuleFileName(_pModule->get_m_hInst(), szModuleName, MAX_PATH);
		CT2W cModuleName(szModuleName);
		OLECHAR const* pszModuleName = cModuleName;
		OLECHAR szShellIcon[MAX_PATH+32];
		int iSrc = 0;
		int iDst = 0;
		if (t_pszShellIconPath) while (t_pszShellIconPath[iSrc] && iDst < (MAX_PATH+32-1))
		{
			if (0 == wcsncmp(t_pszShellIconPath+iSrc, L"%MODULE%", 8))
			{
				int i = 0;
				while (szModuleName[i] && iDst < (MAX_PATH+32-1))
				{
					szShellIcon[iDst++] = pszModuleName[i++];
				}
				iSrc += 8;
			}
			else
			{
				szShellIcon[iDst] = t_pszShellIconPath[iSrc];
				++iDst;
				++iSrc;
			}
		}
		szShellIcon[iDst] = L'\0';

		CComPtr<IDocumentTypeWildcards> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(DocumentTypeWildcards));
		pTmp->InitEx(
			CMultiLanguageString::GetAuto(t_pszFilterNameID),
			CMultiLanguageString::GetAuto(t_pszTypeNameID),
			nExtensions, pExtensions,
			iDst == 0 ? NULL : CComBSTR(szShellIcon),
			_pModule->get_m_hInst(), t_nFilterIconID,
			bstrFilter);

		for (size_t i = 0; i < nExtensions; ++i)
			::SysFreeString(pExtensions[i]);
		delete[] pExtensions;

		return pTmp.Detach();
	}
};

template<wchar_t const* const t_pszFilterNameID, wchar_t const* const t_pszTypeNameID, LPCOLESTR t_pszSupportedExtensions, HICON (t_fnGetFilterIcon)(ULONG), LPCOLESTR t_pszShellIconPath>
struct CDocumentTypeCreatorWildchars3
{
	static IDocumentType* Create()
	{
		size_t nExtensions = 0;
		BSTR* pExtensions = NULL;
		CComBSTR bstrFilter;
		if (t_pszSupportedExtensions != NULL)
		{
			nExtensions = 1;
			size_t i;
			for (i = 0; t_pszSupportedExtensions[i]; ++i)
			{
				if (t_pszSupportedExtensions[i] == L'|')
				{
					++nExtensions;
				}
			}
			pExtensions = new BSTR[nExtensions];
			for (i = 0; i < nExtensions; ++i)
			{
				pExtensions[i] = NULL;
			}
			size_t ii = 0;
			LPCOLESTR psz = t_pszSupportedExtensions;
			for (i = 0; t_pszSupportedExtensions[i]; ++i)
			{
				if (t_pszSupportedExtensions[i] == L'|')
				{
					pExtensions[ii] = ::SysAllocStringLen(psz, t_pszSupportedExtensions+i-psz);
					psz = t_pszSupportedExtensions+i+1;
					++ii;
				}
			}
			pExtensions[ii] = ::SysAllocStringLen(psz, t_pszSupportedExtensions+i-psz);
			for (i = 0; i < nExtensions; ++i)
			{
				if (bstrFilter == NULL)
				{
					bstrFilter = L"*.";
				}
				else
				{
					bstrFilter += L";*.";
				}
				bstrFilter += pExtensions[i];
			}
		}

		TCHAR szModuleName[MAX_PATH] = _T("");
		GetModuleFileName(_pModule->get_m_hInst(), szModuleName, MAX_PATH);
		CT2W cModuleName(szModuleName);
		OLECHAR const* pszModuleName = cModuleName;
		OLECHAR szShellIcon[MAX_PATH+32];
		int iSrc = 0;
		int iDst = 0;
		if (t_pszShellIconPath) while (t_pszShellIconPath[iSrc] && iDst < (MAX_PATH+32-1))
		{
			if (0 == wcsncmp(t_pszShellIconPath+iSrc, L"%MODULE%", 8))
			{
				int i = 0;
				while (szModuleName[i] && iDst < (MAX_PATH+32-1))
				{
					szShellIcon[iDst++] = pszModuleName[i++];
				}
				iSrc += 8;
			}
			else
			{
				szShellIcon[iDst] = t_pszShellIconPath[iSrc];
				++iDst;
				++iSrc;
			}
		}
		szShellIcon[iDst] = L'\0';

		CComPtr<IDocumentTypeWildcards> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(DocumentTypeWildcards));
		pTmp->InitEx2(
			CMultiLanguageString::GetAuto(t_pszFilterNameID),
			CMultiLanguageString::GetAuto(t_pszTypeNameID),
			nExtensions, pExtensions,
			iDst == 0 ? NULL : CComBSTR(szShellIcon),
			_pModule->get_m_hInst(), t_fnGetFilterIcon,
			bstrFilter);

		for (size_t i = 0; i < nExtensions; ++i)
			::SysFreeString(pExtensions[i]);
		delete[] pExtensions;

		return pTmp.Detach();
	}
};

class CUndoBlock
{
public:
	CUndoBlock(IDocumentUndo* a_pUndo, ILocalizedString* a_pName = NULL) : m_pUndo(NULL)
	{
		if (a_pUndo)
		{
			if (SUCCEEDED(a_pUndo->StepStart(a_pName)))
				(m_pUndo = a_pUndo)->AddRef();
		}
	}
	CUndoBlock(IDocument* a_pDoc, ILocalizedString* a_pName = NULL) : m_pUndo(NULL)
	{
		CComQIPtr<IDocumentUndo> pUndo(a_pDoc);
		if (pUndo)
		{
			if (SUCCEEDED(pUndo->StepStart(a_pName)))
				m_pUndo = pUndo.Detach();
		}
	}
	operator IDocumentUndo*()
	{
		return m_pUndo;
	}
	operator bool()
	{
		return m_pUndo != NULL;
	}
	~CUndoBlock()
	{
		if (m_pUndo)
		{
			m_pUndo->StepEnd();
			m_pUndo->Release();
		}
	}

private:
	IDocumentUndo* m_pUndo;
};

template<class Q> HRESULT inline QueryFeatureInterface(IDocument* pDoc, Q** pp) { return pDoc->QueryFeatureInterface(__uuidof(Q), (void **)pp); }

extern "C"{ // continue: extern "C"{

#endif//__cplusplus

