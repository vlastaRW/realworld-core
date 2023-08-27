#pragma once

#include "SyncedConfigData.h"
#include "ConfigVisualizer.h"
#include "ClipAreaDlg.h"

class CTableManager :
	public IConfigVisualizer,
	public CSyncedDataListener<TScrollInfo>,
	public CSyncedDataListener<TColumnInfo>,
	private CSyncedData<TColumnInfo>
{
public:
	CTableManager(CSyncedData<TScrollInfo>* a_pData);
	~CTableManager();

	void HeaderTrack();
	void SetAreaSize(SIZE a_tNewSize);
	void Initialize(const CWindow& a_wndParent, LCID a_tLocaleID);

	// CSyncedDataListener<TScrollInfo>::Notify methods
public:
	void Notify(const TScrollInfo& a_tScrollInfo);

	// CSyncedDataListener<TColumnInfo>::Notify methods
public:
	void Notify(const TColumnInfo& a_tColumnInfo);

	// IConfigVisualizer methods
public:
	void ConfigSet(IConfig* a_pConfig);
	void ItemsChanged();

private:
	CClipAreaDlg m_wndTable;
	CHeaderCtrl m_wndHeader;
	CSyncedData<TScrollInfo>* m_pData;
};
