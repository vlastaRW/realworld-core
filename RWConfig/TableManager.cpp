#include "StdAfx.h"
#include "RWConfig.h"

#include "TableManager.h"
#include <Win32LangEx.h>

const DWORD HEADER_HEIGHT = 20;

CTableManager::CTableManager(CSyncedData<TScrollInfo>* a_pData) :
	m_pData(a_pData), m_wndTable(a_pData, this)
{
	m_pData->ListenerAdd(this);
}

CTableManager::~CTableManager()
{
	m_pData->ListenerRemove(this);
}

void CTableManager::Notify(const TScrollInfo& a_tScrollInfo)
{
	if (m_wndHeader.m_hWnd)
	{
		RECT rcWnd;
		rcWnd.left = -a_tScrollInfo.nActPosH;
		rcWnd.top = 0;
		rcWnd.right = a_tScrollInfo.nRangeH > 2000 ? a_tScrollInfo.nRangeH : 2000;
		rcWnd.bottom = HEADER_HEIGHT;
		if (m_wndHeader.m_hWnd)
		{
			m_wndHeader.MoveWindow(&rcWnd);
		}
	}
}

void CTableManager::Notify(const TColumnInfo& a_tColumnInfo)
{
	if (m_wndHeader.m_hWnd)
	{
		HDITEM hdi;
		ZeroMemory(&hdi, sizeof(hdi));
		hdi.mask = HDI_WIDTH;
		hdi.cxy = a_tColumnInfo.nWidthName;
		m_wndHeader.SetItem(0, &hdi);
		hdi.cxy = a_tColumnInfo.nWidthValue;
		m_wndHeader.SetItem(1, &hdi);
	}
}

void CTableManager::Initialize(const CWindow& a_wndParent, LCID a_tLocaleID)
{
	RECT rcWnd;
	a_wndParent.GetClientRect(&rcWnd);
	rcWnd.top = HEADER_HEIGHT;

	m_wndTable.SetLocaleID(a_tLocaleID);
	m_wndTable.Create(a_wndParent);
	m_wndTable.MoveWindow(&rcWnd);

	rcWnd.top = 0;
	rcWnd.bottom = HEADER_HEIGHT;
	BOOL bNoFullDrag = FALSE;
	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &bNoFullDrag, 0); // stupid MS change behavior of flags
	m_wndHeader.Create(a_wndParent, rcWnd, _T("Header"), (bNoFullDrag,0) ? WS_VISIBLE | WS_CHILDWINDOW : WS_VISIBLE | WS_CHILDWINDOW | HDS_FULLDRAG);
	m_wndHeader.SetFont(a_wndParent.GetFont());
	HDITEM hdi;
	ZeroMemory(&hdi, sizeof(hdi));
	TCHAR szTmp[64];
	hdi.pszText = szTmp;
	hdi.mask = HDI_WIDTH | HDI_TEXT;
	hdi.cxy = ValueGet().nWidthName;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COLUMN_NAME, szTmp, itemsof(szTmp), LANGIDFROMLCID(a_tLocaleID));
	m_wndHeader.InsertItem(0, &hdi);
	hdi.cxy = ValueGet().nWidthValue;
	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_COLUMN_VALUE, szTmp, itemsof(szTmp), LANGIDFROMLCID(a_tLocaleID));
	m_wndHeader.InsertItem(1, &hdi);
}

void CTableManager::SetAreaSize(SIZE a_tNewSize)
{
	TScrollInfo tScroll = m_pData->ValueGet();
	tScroll.nPageH = a_tNewSize.cx;
	tScroll.nPageV = a_tNewSize.cy - HEADER_HEIGHT;
	if ((tScroll.nRangeV-tScroll.nPageV) > 0 && tScroll.nActPosV > (tScroll.nRangeV-tScroll.nPageV))
		tScroll.nActPosV = tScroll.nRangeV-tScroll.nPageV;
	if ((tScroll.nRangeH-tScroll.nPageH) > 0 && tScroll.nActPosH > (tScroll.nRangeH-tScroll.nPageH))
		tScroll.nActPosH = tScroll.nRangeH-tScroll.nPageH;
	if (m_wndTable.m_hWnd)
	{
		RECT rcWnd;
		rcWnd.left = 0;
		rcWnd.top = HEADER_HEIGHT;
		rcWnd.right = a_tNewSize.cx;
		rcWnd.bottom = a_tNewSize.cy;
		m_wndTable.MoveWindow(&rcWnd);
	}
	m_pData->ValueSet(tScroll);
}

void CTableManager::ConfigSet(IConfig* a_pConfig)
{
	m_wndTable.ConfigSet(a_pConfig);
}

void CTableManager::ItemsChanged()
{
	m_wndTable.ItemsChanged();
}

void CTableManager::HeaderTrack()
{
	TColumnInfo tColumn = ValueGet();
	TScrollInfo tScroll = m_pData->ValueGet();
	HDITEM hdi;
	hdi.mask = HDI_WIDTH;
	if (m_wndHeader.m_hWnd)
	{
		m_wndHeader.GetItem(0, &hdi);
		tColumn.nWidthName = hdi.cxy;
		m_wndHeader.GetItem(1, &hdi);
		tColumn.nWidthValue = hdi.cxy;
		tScroll.nRangeH = tColumn.nWidthName + tColumn.nWidthValue;
	}
	if (tScroll.nRangeH <= tScroll.nPageH)
	{
		tScroll.nActPosH = 0;
	}
	else if (tScroll.nActPosH > (tScroll.nRangeH - tScroll.nPageH))
	{
		tScroll.nActPosH = tScroll.nRangeH - tScroll.nPageH;
	}
	ValueSet(tColumn);
	m_pData->ValueSet(tScroll);
}
