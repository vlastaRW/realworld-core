// ThumbnailCache.h : Declaration of the CThumbnailCache

#pragma once
#include "resource.h"       // main symbols
#include "RWThumbnails.h"



// CThumbnailCache

class ATL_NO_VTABLE CThumbnailCache :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CThumbnailCache, &CLSID_ThumbnailCache>,
	public IThumbnailCache
{
public:
	CThumbnailCache() : m_hDirectory(INVALID_HANDLE_VALUE)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CThumbnailCache)
	COM_INTERFACE_ENTRY(IThumbnailCache)
	COM_INTERFACE_ENTRY(IThumbnailRenderer)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IThumbnailCache methods
public:
	STDMETHOD(Init)(IThumbnailRenderer* a_pInternalRenderer, ULONG a_nMemoryItems, ULONG a_nFilesystemItems, BSTR a_bstrTempPath);
	STDMETHOD(Remove)(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY);
	STDMETHOD(Clear)();

	// IThumbnailRenderer methods
public:
	STDMETHOD(GetThumbnail)(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo);

private:
	struct SFileSystemItem
	{
		CComBSTR bstrLocation;
		ULONGLONG nTime;
		ULONG nNativeSizeX;
		ULONG nNativeSizeY;
		ULONG nSizeX;
		ULONG nSizeY;
		GUID tThumbnailID;
		RECT rcBounds;
	};

	typedef std::vector<SFileSystemItem> CFileSystemItems;

private:
	void LockDirectory();
	void UnlockDirectory();
	void WriteDirectory();

	bool LoadThumbnail(int a_nSizeX, int a_nSizeY, GUID const& a_tID, DWORD* a_pRGBAData, BSTR* a_pbstrInfo);
	bool SaveThumbnail(int a_nSizeX, int a_nSizeY, GUID const& a_tID, DWORD* a_pRGBAData, LCID a_tLocaleID, BSTR a_bstrInfo);
	bool DeleteThumbnail(GUID const& a_tID);

private:
	CComPtr<IThumbnailRenderer> m_pInternalRenderer;
	//ULONG m_nMemoryItems;
	ULONG m_nFilesystemItems;
	CComBSTR m_bstrTempPath;

	HANDLE m_hDirectory;
	CFileSystemItems m_cFSItems;
};

OBJECT_ENTRY_AUTO(__uuidof(ThumbnailCache), CThumbnailCache)
