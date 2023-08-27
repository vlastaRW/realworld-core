// DesignerViewTree.h : Declaration of the CDesignerViewTree

#pragma once
#include "resource.h"       // main symbols
#include "RWViewStructure.h"
#include <RWConceptStructuredData.h>
#include <ObserverImpl.h>
#include <Win32LangEx.h>
#include <ContextMenuWithIcons.h>
#include <RWThumbnails.h>
#include "ConfigIDsTree.h"

#define TVGN_EX_ALL			0x000F
#ifndef TVIS_FOCUSED
#define TVIS_FOCUSED	1
#else
#if TVIS_FOCUSED != 1
#error TVIS_FOCUSED was assumed to be 1
#endif
#endif
#define WM_RW_REPOSITIONEDIT (WM_APP+2186)


// CDesignerViewTree

class ATL_NO_VTABLE CDesignerViewTree : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangFrameWindowImpl<CDesignerViewTree>,
	public CObserverImpl<CDesignerViewTree, IStructuredObserver, TStructuredChanges>,
	public CObserverImpl<CDesignerViewTree, ISharedStateObserver, TSharedStateChange>,
	public CDesignerViewWndImpl<CDesignerViewTree, IDesignerView>,
	public CContextMenuWithIcons<CDesignerViewTree>,
	public IDesignerViewStructure,
	public IDesignerViewStatusBar,
	public IDragAndDropHandler,
	public IDesignerViewClipboardHandler
{
public:
	CDesignerViewTree() : m_wndTree(this, 1), m_bHasLines(false),
		m_bSelectionSending(false), m_iidStructuredRoot(GUID_NULL),
		m_bShowCommandDesc(false), m_bShowItemDesc(false),
		m_hDropHighlight(NULL), m_nDropHighlightFlag(0), m_eDragMode(EDMNothing),
		m_bEnsureVisibleOnKillFocus(false), m_nClipboardPriority(-1),
		m_hThumbThread(NULL), m_pThumbData(NULL), m_bTrackingMouse(false),
		m_bTheme(false), m_hTheme(NULL), m_hHotItem(0), m_nHotPart(0),
		m_bSelectPending(FALSE), m_hFirstSelectedItem(NULL), m_bSelectionComplete(TRUE),
		m_bTreeTheme(false), m_hTreeTheme(NULL), m_bSelectDropped(true), m_hContextMenuItem(NULL),
		m_eSelectionMode(CFGVAL_TSM_SAMEPARENT)
	{
	}
	~CDesignerViewTree()
	{
		StopThumbThread();
		if (m_hThumbThread) CloseHandle(m_hThumbThread);
		m_cImageList.Destroy();
		m_cImageMenu.Destroy();
		for (CThumbnailMap::iterator i = m_cThumbnailMap.begin(); i != m_cThumbnailMap.end(); ++i)
			i->first->Release();
		m_cThumbnailMap.clear();
		m_cThumbnailList.Destroy();
	}
	void Init(ISharedStateManager* a_pStateMgr, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID, IMenuCommandsManager* a_pCmdMgr);

	static HRESULT WINAPI QIDND(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
	{
		CDesignerViewTree* pThis = reinterpret_cast<CDesignerViewTree*>(a_pThis);
		if (pThis->m_pRichGUI)
		{
			*a_ppv = reinterpret_cast<void*>(static_cast<IDragAndDropHandler*>(pThis));
			pThis->AddRef();
			return S_OK;
		}
		else
		{
			*a_ppv = NULL;
			return E_NOINTERFACE;
		}
	}
	static HRESULT WINAPI QICB(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
	{
		CDesignerViewTree* pThis = reinterpret_cast<CDesignerViewTree*>(a_pThis);
		if (pThis->m_nClipboardPriority >= 0)
		{
			*a_ppv = reinterpret_cast<void*>(static_cast<IDesignerViewClipboardHandler*>(pThis));
			pThis->AddRef();
			return S_OK;
		}
		else
		{
			*a_ppv = NULL;
			return E_NOINTERFACE;
		}
	}

BEGIN_COM_MAP(CDesignerViewTree)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewStructure)
	COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IDragAndDropHandler), 0, QIDND)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IDesignerViewClipboardHandler), 0, QICB)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewTree)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	//NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelectionChanged)
	NOTIFY_CODE_HANDLER(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	NOTIFY_CODE_HANDLER(TVN_ENDLABELEDIT, OnEndLabelEdit)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewTree>)
	CHAIN_MSG_MAP(Win32LangEx::CLangFrameWindowImpl<CDesignerViewTree>)
	NOTIFY_CODE_HANDLER(TVN_KEYDOWN, OnKeyDown)
	NOTIFY_CODE_HANDLER(NM_KILLFOCUS, OnKillFocus)
	NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
	MESSAGE_HANDLER(WM_THUMBNAILGENERATED, OnThumbnailGenerated)
	NOTIFY_CODE_HANDLER(TVN_BEGINDRAG, OnBeginDrag)
	MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChanged)
	MESSAGE_HANDLER(WM_RW_REPOSITIONEDIT, OnRepositionEdit)

	NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnTreeItemExpanding)
	NOTIFY_CODE_HANDLER(NM_SETFOCUS, OnTreeSetfocus)
	NOTIFY_CODE_HANDLER(NM_KILLFOCUS, OnTreeKillfocus)
	NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeSelchanged)
	//MESSAGE_HANDLER(WM_TIMER, OnDragTimer)
ALT_MSG_MAP(1)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnTreeLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnTreeLButtonUp)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnTreeMouseMove)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnTreeMouseLeave)
	MESSAGE_HANDLER(WM_KEYDOWN, OnTreeKeyDown)
	MESSAGE_HANDLER(WM_RBUTTONDOWN, OnTreeRButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnTreeLButtonDblClk)
	MESSAGE_HANDLER(WM_TIMER, OnTreeTimer)
	MESSAGE_HANDLER(WM_THEMECHANGED, OnTreeThemeChanged)
	MESSAGE_HANDLER(WM_DESTROY, OnTreeDestroy)
	COMMAND_CODE_HANDLER(EN_UPDATE, OnEditUpdated);
	//if (uMsg == TVM_GETITEMRECT)
	//{
	//	if (wParam != 0)
	//	{
	//		RECT* p = (RECT*)lParam;
	//		p->left = 40;
	//		p->right = 100;
	//		p->top = 0;
	//		p->bottom = 100;
	//		return 1;
	//	}
	//}
END_MSG_MAP()


DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pStateMgr)
			m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewTree, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		if (m_pRoot)
			m_pRoot->ObserverDel(CObserverImpl<CDesignerViewTree, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
	}

	// IChildWindow metods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IDesignerView metods
public:

	// IDesignerViewStructureRoot methods
public:
	STDMETHOD(RootIDGet)(GUID* a_pID);
	STDMETHOD(RootIDSet)(REFIID a_tID);
	STDMETHOD(SelectionGet)(IEnumUnknowns** a_ppSelection);

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		if (m_hWnd == NULL)
			return S_OK;

		if (m_bShowCommandDesc)
			a_pStatusBar->SimpleModeSet(m_bstrCommandDesc);
		else if (m_bShowItemDesc)
		{
			POINT pt = {-1, -1};
			GetCursorPos(&pt);
			RECT rc = {0, 0, 0, 0};
			GetWindowRect(&rc);
			if (::PtInRect(&rc, pt))
			{
				a_pStatusBar->SimpleModeSet(m_bstrItemDesc);
			}
			else
			{
				m_bShowItemDesc = false;
				m_pItemForDesc  = NULL;
				m_bstrItemDesc.Empty();
			}
		}
		return S_OK;
	}

	// IDragAndDropHandler methods
public:
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt);

	// IDesignerViewClipboardHandler methods
public:
	STDMETHOD(Priority)(BYTE* a_pPrio);
	STDMETHOD(ObjectName)(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName);
	STDMETHOD(ObjectIconID)(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID);
	STDMETHOD(ObjectIcon)(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay);
	STDMETHOD(Check)(EDesignerViewClipboardAction a_eAction) { return ClipboardTest(static_cast<ERichGUIClipboardAction>(a_eAction)); }
	STDMETHOD(Exec)(EDesignerViewClipboardAction a_eAction) { return ClipboardRun(static_cast<ERichGUIClipboardAction>(a_eAction)); }

	HRESULT ClipboardTest(ERichGUIClipboardAction a_eAction);
	HRESULT ClipboardRun(ERichGUIClipboardAction a_eAction);

public:
	void OwnerNotify(TCookie a_tCookie, TStructuredChanges a_tChanges);
	void OwnerNotify(TCookie a_tCookie, TSharedStateChange a_tParameter);

	// handlers
public:
	//LRESULT OnDragTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnThemeChanged(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnRepositionEdit(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	void Notify(POINT a_tPt, UINT a_nFlags);
	bool OnCanEditLabel(HTREEITEM a_hTI);
	LRESULT OnSelectionChanged(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnBeginLabelEdit(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnEndLabelEdit(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnEditUpdated(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnBeginDrag(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnKillFocus(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnCustomDraw(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	//LRESULT OnToolbarDropdown(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnKeyDown(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	//LRESULT OnToolbarOperation(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_wndTree.GetEditControl().m_hWnd)
			return MA_NOACTIVATE;

		LRESULT lRet = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);

		if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
		{
			//m_bSelectionSending = true;
			m_wndTree.SetFocus();
			//m_bSelectionSending = false;
		}

		return lRet;
	}
	LRESULT OnMenuSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnThumbnailGenerated(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	bool GetMenuItemProps(UINT a_nItemID, HIMAGELIST* a_pImageList, int* a_pIconIndex, LPCTSTR* a_ppszText);

private:
	struct lessIComparable
	{
		bool operator()(IComparable* a_1, IComparable* a_2) const
		{
			CLSID clsid1 = GUID_NULL;
			CLSID clsid2 = GUID_NULL;
			a_1->CLSIDGet(&clsid1);
			a_2->CLSIDGet(&clsid2);
			if (IsEqualCLSID(clsid1, clsid2))
			{
                return a_1->Compare(a_2) == S_LESS;
			}
			return memcmp(&clsid1, &clsid2, sizeof(clsid1)) < 0;
		}
	};
	typedef multimap<IComparable*, HTREEITEM, lessIComparable> CBackMap;

	template<class T>
	struct lessBinary
	{
		bool operator()(const T& a_1, const T& a_2) const
		{
			return memcmp(&a_1, &a_2, sizeof(T)) < 0;
		}
	};
	typedef map<GUID, int, lessBinary<GUID> > CImageMap;
	struct SThumbnailInList { ULONG nTimestamp; int iImageIndex; };
	typedef map<IComparable*, SThumbnailInList, lessIComparable> CThumbnailMap;

	typedef vector<pair<IComparable*, HTREEITEM> > CItems;

	typedef vector<CComPtr<IDocumentMenuCommand> > CContextOps;
	enum
	{
		IDC_TREE_TREE = 100,
		ID_STRUCTUREROOT = 199,
		ID_MENU_FIRST = 1000,
		ID_TOOLBAR_FIRST = 3000,
		ID_TOOLBAR_LAST = 3999,
		WM_THUMBNAILGENERATED = WM_APP+923,
	};

	typedef map<UINT, CComPtr<IDocumentMenuCommand> > CCommands;

	typedef std::list<IComparable*> CQueue;
	struct SThumbData
	{
		SThumbData(CDesignerViewTree* a_pThis) : nRefCount(2), pThis(a_pThis), hSemaphore(NULL)
		{
			InitializeCriticalSection(&tLock);
			hSemaphore = CreateSemaphore(NULL, 0, 1024, NULL);
		}
		~SThumbData()
		{
			DeleteCriticalSection(&tLock);
			CloseHandle(hSemaphore);
		}
		LONG nRefCount;
		CDesignerViewTree* pThis;
		CQueue cQueue;
		HANDLE hSemaphore;
		CRITICAL_SECTION tLock;
	};

private:
	void GetItemDisplayProps(IComparable* a_pItem, bool& bRefresh, bool& bExpanded, TVITEMEX& tItem);

	HTREEITEM InsertItem(HTREEITEM a_hRoot, IComparable* a_pItem, bool a_bExpanded, HTREEITEM a_hAfter = TVI_LAST)
	{
		USES_CONVERSION;

		if (m_pThumbData && m_pThumbData->nRefCount == 2)
		{
			TVITEMEX tItem;
			ZeroMemory(&tItem, sizeof tItem);
			tItem.mask = TVIF_PARAM|TVIF_STATE;
			tItem.lParam = reinterpret_cast<LPARAM>(a_pItem);
			bool bRefresh = false;
			GetItemDisplayProps(a_pItem, bRefresh, a_bExpanded, tItem);
			tItem.state = a_bExpanded ? TVIS_EXPANDED : 0;
			tItem.stateMask = TVIS_EXPANDED;
			TVINSERTSTRUCT tvis;
			ZeroMemory(&tvis, sizeof tvis);
			tvis.hParent = a_hRoot;
			tvis.hInsertAfter = a_hAfter;
			tvis.itemex = tItem;
			HTREEITEM hItem = m_wndTree.InsertItem(&tvis);
			a_pItem->AddRef();
			m_cBackMap.insert(make_pair(a_pItem, hItem));
			if (bRefresh)
			{
				EnterCriticalSection(&m_pThumbData->tLock);
				m_pThumbData->cQueue.push_back(a_pItem);
				a_pItem->AddRef();
				ReleaseSemaphore(m_pThumbData->hSemaphore, 1, NULL);
				LeaveCriticalSection(&m_pThumbData->tLock);
			}
			return hItem;
		}
		else
		{
			int nImage = -1;
			std::tstring strText;
			GetItemDisplayProps(a_pItem, strText, nImage, a_bExpanded);
			HTREEITEM hItem = m_wndTree.InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE, strText.c_str(), nImage, nImage, a_bExpanded ? TVIS_EXPANDED : 0, TVIS_EXPANDED, reinterpret_cast<LPARAM>(a_pItem), a_hRoot, a_hAfter);
			a_pItem->AddRef();
			m_cBackMap.insert(make_pair(a_pItem, hItem));
			return hItem;
		}
	}

	void InsertItems(IComparable* a_pRoot, HTREEITEM a_hRoot, bool a_bExpanded)
	{
		if (m_pRoot != NULL)
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pRoot->ItemsEnum(a_pRoot, &pItems);
			if (pItems != NULL)
			{
				ULONG i;
				CComPtr<IComparable> pItem;
				for (i = 0; SUCCEEDED(pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem))); i++, pItem = NULL)
				{
					HTREEITEM hRoot = InsertItem(a_hRoot, pItem, a_bExpanded);
					InsertItems(pItem, hRoot, false);
				}
			}
		}
	}

	void DeleteItem(HTREEITEM a_hItem);
	void DeleteItems(HTREEITEM a_hRoot);
	void InsertMenuItems(IEnumUnknowns* a_pOps, CContextOps& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID);
	static int CALLBACK CompareTreeItems(LPARAM a_lParam1, LPARAM a_lParam2, LPARAM a_lParamSort);
	void CheckTree();
	HRESULT ProcessContextMenu(IEnumUnknowns* a_pOps, int a_xPos, int a_yPos);
	void GetItemDisplayProps(IComparable* a_pItem, std::tstring& a_strName, int& a_nImage, bool& bExpanded);
	bool GetItemDataAsString(IComparable* a_pItem, bool a_bIncludeBool, std::tstring& a_strData) const;
	bool SetItemDataFromString(LPCTSTR a_pszData, IComparable* a_pItem) const;

	template<int t_nDefault, typename IItem>
	int GetItemImage(IItem* a_pItem, bool* a_pRefresh = NULL)
	{
		GUID tIconID = GUID_NULL;
		int nImage = t_nDefault;
		HRESULT hRes = a_pItem->IconIDGet(&tIconID);
		if (SUCCEEDED(hRes))
		{
			CImageMap::const_iterator iImg = m_cImageMap.find(tIconID);
			if (hRes == S_OK && iImg != m_cImageMap.end())
			{
				nImage = iImg->second;
			}
			else
			{
				HICON hIcon = NULL;
				a_pItem->IconGet(m_nIconSize, &hIcon);
				if (hIcon != NULL)
				{
					if (iImg != m_cImageMap.end())
					{
						m_cImageList.ReplaceIcon(nImage = iImg->second, hIcon);
						DestroyIcon(hIcon);
						if (a_pRefresh) *a_pRefresh = true;
					}
					else
					{
						nImage = m_cImageList.AddIcon(hIcon);
						DestroyIcon(hIcon);
						m_cImageMap[tIconID] = nImage;
					}
				}
			}
		}
		return nImage;
	}

	static unsigned __stdcall ThumbProc(void* a_pRawData);
	void StopThumbThread()
	{
		if (m_pThumbData)
		{
			EnterCriticalSection(&m_pThumbData->tLock);
			LONG nRef = InterlockedDecrement(&m_pThumbData->nRefCount);
			for (CQueue::iterator i = m_pThumbData->cQueue.begin(); i != m_pThumbData->cQueue.end(); ++i)
				(*i)->Release();
			m_pThumbData->cQueue.clear();
			LeaveCriticalSection(&m_pThumbData->tLock);
			ReleaseSemaphore(m_pThumbData->hSemaphore, 10, NULL);
			if (nRef == 0)
				delete m_pThumbData;
			m_pThumbData = NULL;
		}
	}

	HTHEME GetTheme();

	// multiselection tree window support
private:
	typedef CSimpleArray<HTREEITEM> CTreeItemList;

	enum { TCEX_EDITLABEL = 1, TCEX_SCROLLDRAG = 2 };

	enum EDragMode
	{
		EDMNothing = 0,
		EDMCheckBox,
		EDMStartEdit,
		EDMPlusMinus,
		EDMSelectLater,
	};

	EDragMode m_eDragMode;
	POINT m_ptDragOrigin;
	HTREEITEM m_hDragItem;

	void InvalidateItem(HTREEITEM hItem)
	{
		RECT rc;
		m_wndTree.GetItemRect(hItem, &rc, FALSE);
		//m_wndTree.
		m_wndTree.InvalidateRect(&rc);
	}

	LRESULT OnTreeLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_idTimer)
		{
			m_wndTree.KillTimer(m_idTimer);
			m_idTimer = 0;
		}

		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		UINT nHitFlags = 0;
		HTREEITEM hClickedItem = HitTest( tPt, &nHitFlags );

		m_eDragMode = EDMNothing;
		m_ptDragOrigin = tPt;
		m_hDragItem = hClickedItem;

		if (hClickedItem == NULL)
		{
			m_wndTree.SetFocus();
			return 0;
		}

		if (nHitFlags & TVHT_ONITEMSTATEICON)
		{
			m_eDragMode = EDMCheckBox;
			InvalidateItem(m_hDragItem);
			return 0;
		}

		if (nHitFlags & TVHT_ONITEMBUTTON)
		{
			m_eDragMode = EDMPlusMinus;
			InvalidateItem(m_hDragItem);
			m_wndTree.Expand(m_hDragItem, TVE_TOGGLE);
			return 0;
		}

		m_wndTree.SetFocus();

		if (nHitFlags & TVHT_ONITEMLABEL)
		{
			if (hClickedItem == m_wndTree.GetSelectedItem() && !(a_wParam&(MK_CONTROL|MK_SHIFT)) && (m_wndTree.GetStyle()&TVS_EDITLABELS) && OnCanEditLabel(hClickedItem))
			{
				m_eDragMode = EDMStartEdit;
				return 0;
			}
		}

		if (nHitFlags & (TVHT_ONITEMICON|TVHT_ONITEMLABEL|TVHT_ONITEMRIGHT))
		{
			if (a_wParam&(MK_CONTROL|MK_SHIFT))
			{
				SelectMultiple(hClickedItem, a_wParam, tPt);
			}
			else
			{
				m_eDragMode = EDMSelectLater;
			}
			return 0;
		}

		//// Must invoke label editing explicitly. The base class OnLButtonDown would normally
		//// do this, but we can't call it here because of the multiple selection...
		//if( !( a_wParam&( MK_CONTROL|MK_SHIFT ) ) && ( m_wndTree.GetStyle() & TVS_EDITLABELS ) && ( nHitFlags & TVHT_ONITEMLABEL ) )
		//	if ( hClickedItem == m_wndTree.GetSelectedItem() )
		//	{
		//		// Clear multiple selection before label editing
		//		if (OnCanEditLabel(hClickedItem))
		//		{
		//			ClearSelection();
		//			m_wndTree.SelectItem( hClickedItem );

		//			// Invoke label editing
		//			m_eDragMode = EDMStartEdit;
		//			m_idTimer = SetTimer(TCEX_EDITLABEL, GetDoubleClickTime(), NULL);
		//		}

		//		return 0;
		//	}

		//if( nHitFlags & (TVHT_ONITEMICON|TVHT_ONITEMLABEL|TVHT_ONITEMRIGHT) )
		//{
		//	SetFocus();

		//	m_hClickedItem = hClickedItem;

		//	// Is the clicked item already selected ?
		//	BOOL bIsClickedItemSelected = m_wndTree.GetItemState( hClickedItem, TVIS_SELECTED ) & TVIS_SELECTED;

		//	if ( bIsClickedItemSelected )
		//	{
		//		// Maybe user wants to drag/drop multiple items!
		//		// So, wait until OnLButtonUp() to do the selection stuff. 
		//		m_bSelectPending=TRUE;
		//	}
		//	else
		//	{
		//		SelectMultiple( hClickedItem, a_wParam, tPt );
		//		m_bSelectPending=FALSE;
		//	}

		//	m_ptClick=tPt;
		//}
		////else
		////	a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnTreeLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		if (m_eDragMode == EDMCheckBox)
		{
			InvalidateItem(m_hDragItem);
			UINT nHitFlags = 0;
			HTREEITEM hItem = HitTest(tPt, &nHitFlags);
			if (hItem == m_hDragItem && (nHitFlags&TVHT_ONITEMSTATEICON))
			{
				CComQIPtr<IItemBool> pItem(reinterpret_cast<IComparable*>(m_wndTree.GetItemData(m_hDragItem)));
				if (pItem)
				{
					boolean b = 0;
					pItem->ValueGet(&b);
					b = !b;
					pItem->ValueSet(b);
				}
			}
			m_eDragMode = EDMNothing;
			return 0;
		}

		if (m_eDragMode == EDMStartEdit)
		{
			m_idTimer = m_wndTree.SetTimer(TCEX_EDITLABEL, GetDoubleClickTime(), NULL);
			m_eDragMode = EDMNothing;
			return 0;
		}

		if (m_eDragMode == EDMSelectLater)
		{
			// A select has been waiting to be performed here
			SelectMultiple(m_hDragItem, a_wParam, tPt);
			m_wndTree.SetItemState(m_hDragItem, TVIS_FOCUSED, TVIS_FOCUSED);
			//m_bSelectPending=FALSE;
		}

		m_eDragMode = EDMNothing;

		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnTreeLButtonDblClk(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_idTimer)
		{
			m_wndTree.KillTimer(m_idTimer);
			m_idTimer = 0;
		}

		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		UINT nHitFlags = 0;
		HTREEITEM hClickedItem = HitTest( tPt, &nHitFlags );

		m_eDragMode = EDMNothing;
		m_ptDragOrigin = tPt;
		m_hDragItem = hClickedItem;

		if (hClickedItem == NULL)
			return 0;

		if (nHitFlags & TVHT_ONITEMSTATEICON)
		{
			m_eDragMode = EDMCheckBox;
			InvalidateItem(m_hDragItem);
			return 0;
		}

		if (nHitFlags & TVHT_ONITEMBUTTON)
		{
			m_eDragMode = EDMPlusMinus;
			InvalidateItem(m_hDragItem);
			m_wndTree.Expand(m_hDragItem, TVE_TOGGLE);
			return 0;
		}

		if (nHitFlags & (TVHT_ONITEMICON|TVHT_ONITEMLABEL|TVHT_ONITEMRIGHT))
		{
			m_wndTree.Expand(m_hDragItem, TVE_TOGGLE);
		}

		return 0;
	}

	LRESULT OnTreeMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bTrackingMouse)
		{
			// request notification when the mouse leaves
			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_wndTree, 0 };
			TrackMouseEvent(&tme);
			m_bTrackingMouse = true;
		}

		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		// If there is a select pending, check if cursor has moved so much away from the 
		// down-click point that we should cancel the pending select and initiate
		// a drag/drop operation instead!

		UINT nFlags = 0;
		HTREEITEM tItem = HitTest(tPt, &nFlags);
		if (m_hHotItem != tItem || nFlags != m_nHotPart)
		{
			if (m_hHotItem)
				InvalidateItem(m_hHotItem);
			if (m_hHotItem != tItem)
				m_hHotItem = tItem;
			if (m_hHotItem)
				InvalidateItem(m_hHotItem);
			m_nHotPart = nFlags;
		}

		Notify(tPt, a_wParam);

		if (m_eDragMode == EDMSelectLater || m_eDragMode == EDMStartEdit)
		{
			SIZE sizeMoved = {m_ptDragOrigin.x-tPt.x, m_ptDragOrigin.y-tPt.y};

			if ( abs(sizeMoved.cx) > GetSystemMetrics( SM_CXDRAG ) || abs(sizeMoved.cy) > GetSystemMetrics( SM_CYDRAG ) )
			{
				m_bSelectPending=FALSE;

				// Notify parent that he may begin drag operation
				// Since we have taken over OnLButtonDown(), the default handler doesn't
				// do the normal work when clicking an item, so we must provide our own
				// TVN_BEGINDRAG notification for the parent!

				if ( m_hWnd != NULL && !( m_wndTree.GetStyle() & TVS_DISABLEDRAGDROP ) )
				{
					NM_TREEVIEW tv;

					tv.hdr.hwndFrom = m_wndTree;
					tv.hdr.idFrom = m_wndTree.GetWindowLong( GWL_ID );
					tv.hdr.code = TVN_BEGINDRAG;

					tv.itemNew.hItem = m_hDragItem;
					tv.itemNew.state = m_wndTree.GetItemState( m_hDragItem, 0xffffffff );
					tv.itemNew.lParam = m_wndTree.GetItemData( m_hDragItem );

					tv.ptDrag.x = tPt.x;
					tv.ptDrag.y = tPt.y;

					BOOL bHandled;
					OnBeginDrag(tv.hdr.idFrom, &tv.hdr, bHandled);
				}

				m_eDragMode = EDMNothing;
			}
		}

		//a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnTreeMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_hHotItem)
		{
			InvalidateItem(m_hHotItem);
			m_hHotItem = NULL;
			m_nHotPart = 0;
		}
		m_bTrackingMouse = false;
		return 0;
	}

	LRESULT OnTreeKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_eSelectionMode == CFGVAL_TSM_SINGLE)
		{
			a_bHandled = FALSE;
			return 0;
		}

		if ( a_wParam==VK_NEXT || a_wParam==VK_PRIOR )
		{
			if ( !( GetKeyState( VK_SHIFT )&0x8000 ) )
			{
				// User pressed Pg key without holding 'Shift':
				// Clear multiple selection (if multiple) and let base class do 
				// normal selection work!
				if ( GetSelectedCount()>1 )
					ClearSelection( TRUE );

				CallWindowProc(m_wndTree.m_pfnSuperWindowProc, m_wndTree, a_uMsg, a_wParam, a_lParam);
				m_hFirstSelectedItem = m_wndTree.GetSelectedItem();
				return 0;
			}

			// Flag signaling that selection process is NOT complete.
			// (Will prohibit TVN_SELCHANGED from being sent to parent)
			m_bSelectionComplete = FALSE;

			// Let base class select the item
			CallWindowProc(m_wndTree.m_pfnSuperWindowProc, m_wndTree, a_uMsg, a_wParam, a_lParam);
			HTREEITEM hSelectedItem = m_wndTree.GetSelectedItem();

			// Then select items in between
			SelectItems( m_hFirstSelectedItem, hSelectedItem );

			// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED 
			// notification provided by Windows' treectrl, we must now produce one ourselves,
			// so that our parent gets to know about the change of selection.
			m_bSelectionComplete = TRUE;

			//if (pWnd)
			{
				NM_TREEVIEW tv;
				memset(&tv.itemOld, 0, sizeof(tv.itemOld));

				tv.hdr.hwndFrom = m_wndTree;
				tv.hdr.idFrom = m_wndTree.GetWindowLong(GWL_ID);
				tv.hdr.code = TVN_SELCHANGED;

				tv.itemNew.hItem = hSelectedItem;
				tv.itemNew.state = m_wndTree.GetItemState(hSelectedItem, 0xffffffff);
				tv.itemNew.lParam = m_wndTree.GetItemData(hSelectedItem);
				tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;

				tv.action = TVC_UNKNOWN;

				BOOL bHandled;
				OnSelectionChanged(tv.hdr.idFrom, &tv.hdr, bHandled);
				//SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
			}
		}
		else if ( a_wParam==VK_UP || a_wParam==VK_DOWN )
		{
			// Find which item is currently selected
			HTREEITEM hSelectedItem = m_wndTree.GetSelectedItem();

			HTREEITEM hNextItem;
			if (m_eSelectionMode == CFGVAL_TSM_SAMEPARENT)
			{
				if (a_wParam==VK_UP)
					hNextItem = m_wndTree.GetPrevSiblingItem(hSelectedItem);
				else
					hNextItem = m_wndTree.GetNextSiblingItem(hSelectedItem);
			}
			else
			{
				if (a_wParam==VK_UP)
					hNextItem = m_wndTree.GetPrevVisibleItem(hSelectedItem);
				else
					hNextItem = m_wndTree.GetNextVisibleItem(hSelectedItem);
			}

			if ( !( GetKeyState( VK_SHIFT )&0x8000 ) )
			{
				// User pressed arrow key without holding 'Shift':
				// Clear multiple selection (if multiple) and let base class do 
				// normal selection work!
				if ( GetSelectedCount()>1 )
					ClearSelection( TRUE );

				//if ( hNextItem )
					CallWindowProc(m_wndTree.m_pfnSuperWindowProc, m_wndTree, a_uMsg, a_wParam, a_lParam);
				m_hFirstSelectedItem = m_wndTree.GetSelectedItem();
				return 0;
			}

			if ( hNextItem )
			{
				// Flag signaling that selection process is NOT complete.
				// (Will prohibit TVN_SELCHANGED from being sent to parent)
				m_wndTree.SetRedraw(FALSE);
				m_bSelectionComplete = FALSE;

				// If the next item is already selected, we assume user is
				// "moving back" in the selection, and thus we should clear 
				// selection on the previous one
				BOOL bSelect = !( m_wndTree.GetItemState( hNextItem, TVIS_SELECTED ) & TVIS_SELECTED );

				// Select the next item (this will also deselect the previous one!)
				m_wndTree.SelectItem( hNextItem );
				m_wndTree.EnsureVisible(hNextItem);

				if (m_hFirstSelectedItem == NULL)
					m_hFirstSelectedItem = hSelectedItem;

				// Now, re-select the previously selected item
				if ( bSelect || ( !( m_wndTree.GetItemState( hSelectedItem, TVIS_SELECTED ) & TVIS_SELECTED ) ) )
					SelectItems( m_hFirstSelectedItem, hNextItem );

				// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED 
				// notification provided by Windows' treectrl, we must now produce one ourselves,
				// so that our parent gets to know about the change of selection.
				m_bSelectionComplete = TRUE;
				m_wndTree.SetRedraw(TRUE);
				InvalidateItem(hSelectedItem);
				InvalidateItem(hNextItem);

				//if (pWnd)
				{
					NM_TREEVIEW tv;
					memset(&tv.itemOld, 0, sizeof(tv.itemOld));

					tv.hdr.hwndFrom = m_wndTree;
					tv.hdr.idFrom = m_wndTree.GetWindowLong(GWL_ID);
					tv.hdr.code = TVN_SELCHANGED;

					tv.itemNew.hItem = hNextItem;
					tv.itemNew.state = m_wndTree.GetItemState(hNextItem, 0xffffffff);
					tv.itemNew.lParam = m_wndTree.GetItemData(hNextItem);
					tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;

					tv.action = TVC_UNKNOWN;

					BOOL bHandled;
					OnSelectionChanged(tv.hdr.idFrom, &tv.hdr, bHandled);
					//SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
				}
			}

			// Since the base class' OnKeyDown() isn't called in this case,
			// we must provide our own TVN_KEYDOWN notification to the parent

			//CWindow pWnd = GetParent();
			//if ( pWnd.m_hWnd )
			{
				NMTVKEYDOWN tvk;

				tvk.hdr.hwndFrom = m_wndTree;
				tvk.hdr.idFrom = m_wndTree.GetWindowLong( GWL_ID );
				tvk.hdr.code = TVN_KEYDOWN;

				tvk.wVKey = a_wParam;
				tvk.flags = 0;

				BOOL bHandled;
				OnKeyDown(tvk.hdr.idFrom, &tvk.hdr, bHandled);
				//pWnd.SendMessage( WM_NOTIFY, tvk.hdr.idFrom, (LPARAM)&tvk );
			}
		}
		else
			// Behave normally
			a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnTreeRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		//POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		//UINT nHitFlags = 0;
		//HTREEITEM hClickedItem = HitTest( tPt, &nHitFlags );

		//if( nHitFlags&TVHT_ONITEM )
		//{
		//	HTREEITEM hSel = m_wndTree.GetSelectedItem();
		//	if (hSel != hClickedItem)
		//	{
		//		if ( GetSelectedCount()<2 )
		//		{
		//			m_wndTree.SelectItem( hClickedItem );
		//		}
		//		else
		//		{
		//			if ((m_wndTree.GetItemState(hSel, TVIS_SELECTED)&TVIS_SELECTED))
		//			{
		//				if ((m_wndTree.GetItemState(hClickedItem, TVIS_SELECTED)&TVIS_SELECTED))
		//				{
		//					m_wndTree.SelectItem( hClickedItem ); // set focus to clicked item
		//					m_wndTree.SetItemState(hSel, TVIS_SELECTED, TVIS_SELECTED);
		//				}
		//				else
		//				{
		//					ClearSelection();
		//					m_wndTree.SelectItem( hClickedItem );
		//				}
		//			}
		//			else
		//			{
		//				if ((m_wndTree.GetItemState(hClickedItem, TVIS_SELECTED)&TVIS_SELECTED))
		//				{
		//					m_wndTree.SelectItem( hClickedItem );
		//				}
		//				else
		//				{
		//					ClearSelection();
		//					m_wndTree.SelectItem( hClickedItem );
		//				}
		//			}
		//		}
		//	}
		//}

		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnTreeItemExpanding(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
	{
		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)a_pnmh;

		if ( pNMTreeView->action == TVE_COLLAPSE )
		{
			HTREEITEM hItem = m_wndTree.GetChildItem( pNMTreeView->itemNew.hItem );

			while ( hItem )
			{
				if ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
					m_wndTree.SetItemState( hItem, 0, TVIS_SELECTED );

				// Get the next node: First see if current node has a child
				HTREEITEM hNextItem = m_wndTree.GetChildItem( hItem );
				if ( !hNextItem )
				{
					// No child: Get next sibling item
					if ( !( hNextItem = m_wndTree.GetNextSiblingItem( hItem ) ) )
					{
						HTREEITEM hParentItem = hItem;
						while ( !hNextItem )
						{
							// No more children: Get parent
							if ( !( hParentItem = m_wndTree.GetParentItem( hParentItem ) ) )
								break;

							// Quit when parent is the collapsed node
							// (Don't do anything to siblings of this)
							if ( hParentItem == pNMTreeView->itemNew.hItem )
								break;

							// Get next sibling to parent
							hNextItem = m_wndTree.GetNextSiblingItem( hParentItem );
						}

						// Quit when parent is the collapsed node
						if ( hParentItem == pNMTreeView->itemNew.hItem )
							break;
					}
				}

				hItem = hNextItem;
			}
		}
		
		a_bHandled = FALSE;	// Allow parent to handle this notification as well
		return 0;
	}

	LRESULT OnTreeSelchanged(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
	{
		if (!m_bSelectionComplete)
			return TRUE;	
		return OnSelectionChanged(a_idCtrl, a_pnmh, a_bHandled);
	}

	LRESULT OnTreeSetfocus(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
	{
		Invalidate();
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnTreeKillfocus(int a_idCtrl, LPNMHDR a_pnmh, BOOL& a_bHandled)
	{
		Invalidate();
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnTreeTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_wParam == TCEX_EDITLABEL)
		{
			// Stop the timer.
			m_wndTree.KillTimer(m_idTimer);
			m_idTimer = 0;

			// Invoke label editing.
			//if (m_hDragItem)// m_eDragMode == EDMStartEdit)
				m_wndTree.EditLabel(m_wndTree.GetSelectedItem());

			m_eDragMode = EDMNothing;
			return 0;
		}

		return 0;
	}

	LRESULT OnTreeThemeChanged(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_hTreeTheme)
		{
			CloseThemeData(m_hTreeTheme);
			m_hTreeTheme = OpenThemeData(m_wndTree, VSCLASS_TREEVIEW);
		}
		a_bHandled = FALSE;
		return 0;
	}
	HTHEME GetTreeTheme()
	{
		if (!m_bTreeTheme)
		{
			m_bTreeTheme = true;
			m_hTreeTheme = CTheme::IsThemingSupported() ? OpenThemeData(m_wndTree, VSCLASS_TREEVIEW) : NULL;
		}
		return m_hTreeTheme;
	}
	LRESULT OnTreeDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_hTreeTheme)
			CloseThemeData(m_hTreeTheme);
		a_bHandled = FALSE;
		return 0;
	}

	UINT GetSelectedCount() const
	{
		// Only visible items should be selected!
		UINT uCount=0;
		for ( HTREEITEM hItem = m_wndTree.GetRootItem(); hItem!=NULL; hItem = m_wndTree.GetNextVisibleItem( hItem ) )
			if ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
				uCount++;

		return uCount;
	}
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const
	{
		if (nCode==TVGN_EX_ALL)
		{
			// This special code lets us iterate through ALL tree items regardless 
			// of their parent/child relationship (very handy)
			HTREEITEM hNextItem;

			// If it has a child node, this will be the next item
			hNextItem = m_wndTree.GetChildItem( hItem );
			if (hNextItem)
				return hNextItem;

			// Otherwise, see if it has a next sibling item
			hNextItem = m_wndTree.GetNextSiblingItem(hItem);
			if (hNextItem)
				return hNextItem;

			// Finally, look for next sibling to the parent item
			HTREEITEM hParentItem=hItem;
			while (!hNextItem && hParentItem)
			{
				// No more children: Get next sibling to parent
				hParentItem = m_wndTree.GetParentItem(hParentItem);
				hNextItem = m_wndTree.GetNextSiblingItem(hParentItem);
			}

			return hNextItem; // will return NULL if no more parents
		}
		else
			return m_wndTree.GetNextItem(hItem, nCode);	// standard processing
	}
	HTREEITEM GetFirstSelectedItem() const
	{
		for ( HTREEITEM hItem = m_wndTree.GetRootItem(); hItem!=NULL; hItem = m_wndTree.GetNextVisibleItem( hItem ) )
			if ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
				return hItem;

		return NULL;
	}
	HTREEITEM GetNextSelectedItem(HTREEITEM hItem) const
	{
		for ( hItem = m_wndTree.GetNextVisibleItem( hItem ); hItem!=NULL; hItem = m_wndTree.GetNextVisibleItem( hItem ) )
			if ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
				return hItem;

		return NULL;
	}
	HTREEITEM GetPrevSelectedItem(HTREEITEM hItem) const
	{
		for ( hItem = m_wndTree.GetPrevVisibleItem( hItem ); hItem!=NULL; hItem = m_wndTree.GetPrevVisibleItem( hItem ) )
			if ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
				return hItem;

		return NULL;
	}
	HTREEITEM ItemFromData(DWORD dwData, HTREEITEM hStartAtItem=NULL) const;

	BOOL SelectItemEx(HTREEITEM hItem, BOOL bSelect=TRUE)
	{
		HTREEITEM hSelItem = m_wndTree.GetSelectedItem();

		if ( hItem==hSelItem )
		{
			if ( !bSelect )
			{
				m_wndTree.SelectItem( NULL );
				return TRUE;
			}

			return FALSE;
		}

		m_wndTree.SelectItem( hItem );
		m_hFirstSelectedItem=hItem;

		// Reselect previous "real" selected item which was unselected byt SelectItem()
		if ( hSelItem )
			m_wndTree.SetItemState( hSelItem, TVIS_SELECTED, TVIS_SELECTED );

		return TRUE;
	}

	BOOL SelectItems(HTREEITEM hFromItem, HTREEITEM hToItem)
	{
		// Determine direction of selection 
		// (see what item comes first in the tree)
		HTREEITEM hItem = m_wndTree.GetRootItem();

		while ( hItem && hItem!=hFromItem && hItem!=hToItem )
			hItem = m_wndTree.GetNextVisibleItem( hItem );

		if ( !hItem )
			return FALSE;	// Items not visible in tree

		BOOL bReverse = hItem==hToItem;

		// "Really" select the 'to' item (which will deselect 
		// the previously selected item)

		m_wndTree.SelectItem( hToItem );

		// Go through all visible items again and select/unselect

		hItem = m_wndTree.GetRootItem();
		BOOL bSelect = FALSE;

		while ( hItem )
		{
			if ( hItem == ( bReverse ? hToItem : hFromItem ) )
				bSelect = TRUE;

			if ( bSelect )
			{
				if ( !( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) )
					m_wndTree.SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
			}
			else
			{
				if ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
					m_wndTree.SetItemState( hItem, 0, TVIS_SELECTED );
			}

			if ( hItem == ( bReverse ? hFromItem : hToItem ) )
				bSelect = FALSE;

			hItem = m_wndTree.GetNextVisibleItem( hItem );
		}

		return TRUE;
	}
	void ClearSelection(BOOL bMultiOnly=FALSE)
	{
	//	if ( !bMultiOnly )
	//		SelectItem( NULL );

		for ( HTREEITEM hItem=m_wndTree.GetRootItem(); hItem!=NULL; hItem=m_wndTree.GetNextVisibleItem( hItem ) )
			if ( m_wndTree.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
				m_wndTree.SetItemState( hItem, 0, TVIS_SELECTED );
	}

	void GetSelectedList(CTreeItemList& a_cList) const
	{
		a_cList.RemoveAll();

		HTREEITEM hItem = GetFirstSelectedItem();
		while (hItem)
		{
			a_cList.Add(hItem);
			hItem = GetNextSelectedItem(hItem);
		}
	}

	void DrawItemLines(HDC hDC, HTREEITEM hItem, int iLevel, int nTop, int nBot, int nStep, int nWidth, bool bHasButtons, bool bHasLines, bool bRootLines, bool bButtonHot);
	void DrawCheck(HTHEME hTheme, HDC hDC, int left, int top, int size, bool checked, bool pressed, bool hot);

	void SelectMultiple( HTREEITEM hClickedItem, UINT nFlags, POINT point )
	{
		// Start preparing an NM_TREEVIEW struct to send a notification after selection is done
		NM_TREEVIEW tv;
		memset(&tv.itemOld, 0, sizeof(tv.itemOld));

		HTREEITEM hOldItem = m_wndTree.GetSelectedItem();

		if ( hOldItem )
		{
			tv.itemOld.hItem = hOldItem;
			tv.itemOld.state = m_wndTree.GetItemState( hOldItem, 0xffffffff );
			tv.itemOld.lParam = m_wndTree.GetItemData( hOldItem );
			tv.itemOld.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;
		}

		// Flag signaling that selection process is NOT complete.
		// (Will prohibit TVN_SELCHANGED from being sent to parent)
		m_bSelectionComplete = FALSE;

		// Action depends on whether the user holds down the Shift or Ctrl key
		if ( nFlags & MK_SHIFT )
		{
			// Select from first selected item to the clicked item
			if ( !m_hFirstSelectedItem )
				m_hFirstSelectedItem = m_wndTree.GetSelectedItem();

			SelectItems( m_hFirstSelectedItem, hClickedItem );
		}
		else if ( nFlags & MK_CONTROL )
		{
			// Find which item is currently selected
			HTREEITEM hSelectedItem = m_wndTree.GetSelectedItem();

			// Is the clicked item already selected ?
			BOOL bIsClickedItemSelected = m_wndTree.GetItemState( hClickedItem, TVIS_SELECTED ) & TVIS_SELECTED;
			BOOL bIsSelectedItemSelected = FALSE;
			if ( hSelectedItem )
				bIsSelectedItemSelected = m_wndTree.GetItemState( hSelectedItem, TVIS_SELECTED ) & TVIS_SELECTED;

			//// Must synthesize a TVN_SELCHANGING notification
			//if ( pWnd )
			//{
			//	tv.hdr.hwndFrom = m_hWnd;
			//	tv.hdr.idFrom = GetWindowLong( GWL_ID );
			//	tv.hdr.code = TVN_SELCHANGING;

			//	tv.itemNew.hItem = hClickedItem;
			//	tv.itemNew.state = m_wndTree.GetItemState( hClickedItem, 0xffffffff );
			//	tv.itemNew.lParam = m_wndTree.GetItemData( hClickedItem );

			//	tv.itemOld.hItem = NULL;
			//	tv.itemOld.mask = 0;

			//	tv.action = TVC_BYMOUSE;

			//	tv.ptDrag.x = point.x;
			//	tv.ptDrag.y = point.y;

			//	pWnd.SendMessage( WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv );
			//}

			// If the previously selected item was selected, re-select it
			if ( bIsSelectedItemSelected )
				m_wndTree.SetItemState( hSelectedItem, TVIS_SELECTED, TVIS_SELECTED );

			// We want the newly selected item to toggle its selected state,
			// so unselect now if it was already selected before
			if ( bIsClickedItemSelected )
				m_wndTree.SetItemState( hClickedItem, 0, TVIS_SELECTED );
			else
			{
				//SelectItem(hClickedItem);
				m_wndTree.SetItemState( hClickedItem, TVIS_FOCUSED, TVIS_FOCUSED );
				m_wndTree.SetItemState( hClickedItem, TVIS_SELECTED, TVIS_SELECTED );
			}

			// If the previously selected item was selected, re-select it
			if ( bIsSelectedItemSelected && hSelectedItem != hClickedItem )
				m_wndTree.SetItemState( hSelectedItem, TVIS_SELECTED, TVIS_SELECTED );

			// Store as first selected item (if not already stored)
			if ( m_hFirstSelectedItem==NULL )
				m_hFirstSelectedItem = hClickedItem;
		}
		else
		{
			// Clear selection of all "multiple selected" items first
			ClearSelection();

			// Then select the clicked item
			m_wndTree.SelectItem( hClickedItem );
			m_wndTree.SetItemState( hClickedItem, TVIS_SELECTED, TVIS_SELECTED );

			// Store as first selected item
			m_hFirstSelectedItem = hClickedItem;
		}

		// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED 
		// notification provided by Windows' treectrl, we must now produce one ourselves,
		// so that our parent gets to know about the change of selection.
		m_bSelectionComplete = TRUE;

		//if ( pWnd )
		{
			tv.hdr.hwndFrom = m_wndTree;
			tv.hdr.idFrom = m_wndTree.GetWindowLong( GWL_ID );
			tv.hdr.code = TVN_SELCHANGED;

			tv.itemNew.hItem = hClickedItem;
			tv.itemNew.state = m_wndTree.GetItemState( hClickedItem, 0xffffffff );
			tv.itemNew.lParam = m_wndTree.GetItemData( hClickedItem );
			tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;

			tv.action = TVC_UNKNOWN;

			BOOL bHandled;
			OnSelectionChanged(tv.hdr.idFrom, &tv.hdr, bHandled);
			//pWnd.SendMessage( WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv );
		}
	}

	struct SItemLayout
	{
		RECT rcWhole;
		RECT rcButton;
		RECT rcThumbnail;
		RECT rcCheck;
		RECT rcIcon;
		RECT rcLabel;
		RECT rcValue;
	};
	void ComputeItemLayout(HTREEITEM hItem, SItemLayout& ret) const
	{
		TVITEMEX item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_INTEGRAL;
		m_wndTree.GetItem(&item);
		RECT rcItem = {0, 0, 0, 0};
		m_wndTree.GetItemRect(hItem, &ret.rcWhole, FALSE);
		LONG iIndent = 0;
		for (HTREEITEM h = m_wndTree.GetParentItem(hItem); h; h = m_wndTree.GetParentItem(h))
			++iIndent;

		LONG const nLabelWidth = (item.iImage>>2)&0x3fff;
		LONG const nValueWidth = item.iSelectedImage&0xffff;

		ret.rcButton.top = ret.rcWhole.top;
		ret.rcButton.bottom = ret.rcWhole.bottom;
		ret.rcButton.left = m_nIndentSize*iIndent;
		ret.rcButton.right = m_bHasLines ? ret.rcButton.left+m_nButtonSize : ret.rcButton.left;

		if (item.iIntegral == 1)
		{
			ret.rcThumbnail.left = ret.rcThumbnail.right = ret.rcButton.right;
			ret.rcThumbnail.top = ret.rcWhole.top;
			ret.rcThumbnail.bottom = ret.rcWhole.bottom;
			ret.rcCheck.top = (ret.rcWhole.bottom+ret.rcWhole.top-m_nCheckSize)>>1;
			ret.rcCheck.bottom = ret.rcCheck.top+m_nCheckSize;
			ret.rcCheck.left = ret.rcThumbnail.right;
			ret.rcCheck.right = (item.iImage&2) ? ret.rcCheck.left+m_nCheckSize : ret.rcCheck.left;
			ret.rcIcon.top = (ret.rcWhole.bottom+ret.rcWhole.top-m_nIconSize)>>1;
			ret.rcIcon.bottom = ret.rcIcon.top+m_nIconSize;
			ret.rcIcon.left = ret.rcCheck.right > ret.rcCheck.left ? m_nGapSize+ret.rcCheck.right : ret.rcCheck.right;
			ret.rcIcon.right = (item.iImage&1) ? ret.rcIcon.left+m_nIconSize : ret.rcIcon.left;
			ret.rcLabel.top = ret.rcValue.top = ret.rcWhole.top;
			ret.rcLabel.bottom = ret.rcValue.bottom = ret.rcWhole.bottom;
			ret.rcLabel.left = ret.rcIcon.right > ret.rcIcon.left ? m_nGapSize+ret.rcIcon.right : ret.rcIcon.right;
			ret.rcLabel.right = ret.rcLabel.left+nLabelWidth;
			ret.rcValue.left = ret.rcLabel.right;
			ret.rcValue.right = ret.rcValue.left+nValueWidth;
		}
		else // item.iIntegral == 3
		{
			ret.rcThumbnail.left = ret.rcButton.right;
			ret.rcThumbnail.right = ret.rcThumbnail.left+m_nThumbSizeX;
			ret.rcThumbnail.top = (ret.rcWhole.bottom+ret.rcWhole.top-m_nThumbSizeY)>>1;
			ret.rcThumbnail.bottom = ret.rcThumbnail.top+m_nThumbSizeY;

			LONG const mid1 = (ret.rcWhole.top*3+ret.rcWhole.bottom)/4;
			LONG const mid2 = (ret.rcWhole.top+ret.rcWhole.bottom*3)/4;
			LONG const single = (ret.rcWhole.bottom-ret.rcWhole.top)/3;
			ret.rcCheck.top = mid2-(m_nCheckSize>>1);
			ret.rcCheck.bottom = ret.rcCheck.top+m_nCheckSize;
			ret.rcCheck.left = m_nGapSize+ret.rcThumbnail.right;
			ret.rcCheck.right = (item.iImage&2) ? ret.rcCheck.left+m_nCheckSize : ret.rcCheck.left;
			ret.rcIcon.top = mid2-(m_nIconSize>>1);
			ret.rcIcon.bottom = ret.rcIcon.top+m_nIconSize;
			ret.rcIcon.left = ret.rcCheck.right > ret.rcCheck.left ? m_nGapSize+ret.rcCheck.right : ret.rcCheck.right;
			ret.rcIcon.right = (item.iImage&1) ? ret.rcIcon.left+m_nIconSize : ret.rcIcon.left;
			ret.rcLabel.top = mid2-(single>>1);
			ret.rcLabel.bottom = ret.rcLabel.top+single;
			ret.rcLabel.left = ret.rcIcon.right > ret.rcIcon.left ? m_nGapSize+ret.rcIcon.right : ret.rcIcon.right;
			ret.rcLabel.right = ret.rcLabel.left+nLabelWidth;
			ret.rcValue.top = mid1-(single>>1);
			ret.rcValue.bottom = ret.rcValue.top+single;
			ret.rcValue.left = m_nGapSize+ret.rcThumbnail.right;
			ret.rcValue.right = ret.rcValue.left+nValueWidth;
		}
	}

	HTREEITEM HitTest(POINT pt, UINT* pFlags) const
	{
		TVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		HTREEITEM hTreeItem = m_wndTree.HitTest(&hti);
		if (m_pThumbData == NULL || hTreeItem == NULL || pFlags == NULL)
		{
			if (pFlags != NULL)
				*pFlags = hti.flags;
			return hTreeItem;
		}
		SItemLayout tIL;
		ComputeItemLayout(hTreeItem, tIL);
		if (PtInRect(&tIL.rcButton, pt))
			*pFlags = TVHT_ONITEMBUTTON;
		else if (PtInRect(&tIL.rcCheck, pt))
			*pFlags = TVHT_ONITEMSTATEICON;
		else if (PtInRect(&tIL.rcIcon, pt))
			*pFlags = TVHT_ONITEMICON;
		else if (PtInRect(&tIL.rcValue, pt))
			*pFlags = TVHT_ONITEMLABEL;
		else if (pt.x < tIL.rcButton.left)
			*pFlags = TVHT_ONITEMINDENT;
		else
			*pFlags = TVHT_ONITEMRIGHT;
		return hTreeItem;
	}

private:
	BOOL		m_bSelectPending;
	POINT		m_ptClick;
	HTREEITEM	m_hFirstSelectedItem;
	BOOL		m_bSelectionComplete;
	UINT		m_idTimer;
	HTREEITEM m_hHotItem;
	UINT m_nHotPart;
	bool m_bTrackingMouse;
	DWORD m_dwLastScrollTime;

	bool m_bTreeTheme;
	HTHEME m_hTreeTheme;

private:
	CComPtr<ISharedStateManager> m_pStateMgr;
	CComPtr<IDocument> m_pOriginal;
	CComPtr<IStructuredRoot> m_pRoot;
	IID m_iidStructuredRoot;
	CComPtr<IStructuredDocument> m_pDocument;
	CComPtr<IEnumGUIDs> m_pRoots;
	CComPtr<IStructuredItemsRichGUI> m_pRichGUI;
	int m_nClipboardPriority;
	CComPtr<IConfig> m_pConfig;
	CComPtr<IMenuCommandsManager> m_pCmdMgr;
	EDesignerViewWndStyle m_nStyle;

	// tree
	CContainedWindowT<CTreeViewCtrl> m_wndTree;
	CBackMap m_cBackMap;
	LONG m_eKeyDownAction;
	bool m_bEnsureVisibleOnKillFocus;

	HTREEITEM m_hDropHighlight;
	ULONG m_nDropHighlightFlag;
	bool m_bSelectDropped;

	// graphics - image lists , off-thread rendering
	unsigned m_uThumbThreadID;
	HANDLE m_hThumbThread;
	SThumbData* m_pThumbData;
	CComPtr<IThumbnailRendererDoc> m_pThumbnails;

	bool m_bHasLines;
	ULONG m_nIconSize;
	ULONG m_nCheckSize;
	ULONG m_nButtonSize;
	ULONG m_nGapSize;
	ULONG m_nIndentSize;
	ULONG m_nThumbSizeX;
	ULONG m_nThumbSizeY;
	CImageList m_cImageList;
	CImageMap m_cImageMap;
	CImageList m_cThumbnailList;
	CThumbnailMap m_cThumbnailMap;
	std::vector<int> m_cFreeThumbnailIndices;
	CImageList m_cImageMenu;
	CImageMap m_cMenuImageMap;
	HTREEITEM m_hContextMenuItem;

	bool m_bTheme;
	HTHEME m_hTheme;

	// synchronization
	CComBSTR m_bstrSyncGroup;
	bool m_bSelectionSending;
	CComPtr<ISharedState> m_pLastSel;
	CComPtr<ISharedState> m_pContextSel;
	LONG m_eSelectionMode;

	// status bar
	CContextOps m_cContextOps;
	bool m_bShowCommandDesc;
	CComBSTR m_bstrCommandDesc;
	bool m_bShowItemDesc;
	CComBSTR m_bstrItemDesc;
	CComPtr<IUIItem> m_pItemForDesc;
	CComPtr<IStatusBarObserver> m_pStatusBar;
};

