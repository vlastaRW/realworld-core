// Copyright (c) 2002
// Sergey Klimov (kidd@ukr.net)
// WTL Docking windows
//
// This code is provided "as is", with absolutely no warranty expressed
// or implied. Any use is at your own risk.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name is included. If
// the source code in  this file is used in any commercial application
// then a simple email would be nice.

#ifndef WTL_DW_DOCKINGWINDOW_H_INCLUDED_
#define WTL_DW_DOCKINGWINDOW_H_INCLUDED_

#include <DockMisc.h>
#include <DDTracker.h>

namespace dockwins{

class CDocker : protected CWindow
{
public:
	explicit CDocker(HWND hWnd=NULL) : CWindow(hWnd)
	{
	}
	bool AdjustDragRect(DFDOCKRECT* pHdr) const
	{
		pHdr->hdr.code=DC_ADJUSTDRAGRECT;
		return (::SendMessage(m_hWnd,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}
	bool AcceptDock(DFDOCKRECT* pHdr) const
	{
		pHdr->hdr.code=DC_ACCEPT;
		return (::SendMessage(m_hWnd,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}
	bool Dock(DFDOCKRECT* pHdr) const
	{
		pHdr->hdr.code=DC_DOCK;
//		return (::SendMessage(m_hWnd,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
		return (::SendMessage(pHdr->hdr.hBar,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}

	bool Undock(DFMHDR* pHdr) const
	{
		pHdr->code=DC_UNDOCK;
		return (::SendMessage(pHdr->hBar,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}

    bool Replace(DFDOCKREPLACE* pHdr) const
    {
        pHdr->hdr.code=DC_REPLACE;
        return (::SendMessage(pHdr->hdr.hBar,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
    }

	bool GetDockingPosition(DFDOCKPOS* pHdr) const
	{
		pHdr->hdr.code=DC_GETDOCKPOSITION;
		return (::SendMessage(m_hWnd,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}

	bool SetDockingPosition(DFDOCKPOS* pHdr) const
	{
		pHdr->hdr.hBar=m_hWnd;
		pHdr->hdr.code=DC_SETDOCKPOSITION;
		return (::SendMessage(m_hWnd,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}
//#ifdef DF_AUTO_HIDE_FEATURES
	bool PinUp(DFPINUP* pHdr) const
	{
		pHdr->hdr.code=DC_PINUP;
		return (::SendMessage(m_hWnd,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}

	bool IsPinned(DFMHDR* pHdr) const
	{
		pHdr->code=DC_ISPINNED;
		return (::SendMessage(m_hWnd,WMDF_DOCK,NULL,reinterpret_cast<LPARAM>(pHdr))!=FALSE);
	}
//#endif
	operator HWND () const
	{
		return m_hWnd;
	}
};

template < DWORD t_dwStyle = 0, DWORD t_dwExStyle = 0>
struct CDockingBarWinTraits : CWinTraits<t_dwStyle,t_dwExStyle>
{
	typedef dockwins::CDocker CDocker;
};

typedef CDockingBarWinTraits<WS_OVERLAPPEDWINDOW| WS_POPUP/* WS_CHILD*/ | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW/* WS_EX_CLIENTEDGE*/>    CSimpleDockingBarWinTraits;

template <class T,
          class TBase = CWindow,
          class TWinTraits = CSimpleDockingBarWinTraits>
class ATL_NO_VTABLE CDockingWindowBaseImpl : public CWindowImpl< T, TBase, TWinTraits >
{
    typedef CWindowImpl< T, TBase, TWinTraits >   baseClass;
    typedef CDockingWindowBaseImpl< T, TBase, TWinTraits >       thisClass;
	typedef typename TWinTraits::CDocker	CDocker;
protected:
	class CGhostMoveTracker : public CDDTrackerBaseT<CGhostMoveTracker>
	{
//probably better use GetSystemMetrics
        enum{GhostRectSideSize=3};
	public:
		CGhostMoveTracker(const CDocker& docker,const POINT& pt,DFDOCKRECT& dockHdr)
			:m_docker(docker),m_dockHdr(dockHdr),m_dc(::GetWindowDC(NULL))
		{
			m_offset.cx=m_dockHdr.rect.left-pt.x;
			m_offset.cy=m_dockHdr.rect.top-pt.y;
			m_size.cx=m_dockHdr.rect.right-m_dockHdr.rect.left;
			m_size.cy=m_dockHdr.rect.bottom-m_dockHdr.rect.top;
		}
		void DrawGhostRect(CDC& dc,RECT* pRect)
		{
			CBrush brush = CDCHandle::GetHalftoneBrush();
			if(brush.m_hBrush != NULL)
			{
				HBRUSH hBrushOld = dc.SelectBrush(brush);

				dc.PatBlt(pRect->left, pRect->top,
						  pRect->right-pRect->left,GhostRectSideSize, PATINVERT);
				dc.PatBlt(pRect->left, pRect->bottom-GhostRectSideSize,
						  pRect->right-pRect->left,GhostRectSideSize, PATINVERT);

				dc.PatBlt(pRect->left, pRect->top+GhostRectSideSize,
						  GhostRectSideSize,pRect->bottom-pRect->top-2*GhostRectSideSize, PATINVERT);
				dc.PatBlt(pRect->right-GhostRectSideSize, pRect->top+GhostRectSideSize,
						  GhostRectSideSize,pRect->bottom-pRect->top-2*GhostRectSideSize, PATINVERT);


				dc.SelectBrush(hBrushOld);
			}

		}
		void CleanGhostRect(CDC& dc,RECT* pRect)
		{
			DrawGhostRect(dc,pRect);
		}
		void BeginDrag(void)
		{
			DrawGhostRect(m_dc,&m_dockHdr.rect);
		}
		void EndDrag(bool /*bCanceled*/)
		{
			CleanGhostRect(m_dc,&m_dockHdr.rect);
		}
		void OnMove(long x, long y)
		{
			CleanGhostRect(m_dc,&m_dockHdr.rect);
			m_dockHdr.rect.left=x;
			m_dockHdr.rect.top=y;
			::ClientToScreen(m_dockHdr.hdr.hWnd,reinterpret_cast<POINT*>(&m_dockHdr.rect));
			m_dockHdr.rect.right=m_dockHdr.rect.left+m_size.cx;
			m_dockHdr.rect.bottom=m_dockHdr.rect.top+m_size.cy;
			m_docker.AdjustDragRect(&m_dockHdr);
			if((GetKeyState(VK_CONTROL) & 0x8000) || !m_docker.AcceptDock(&m_dockHdr))
			{
				m_dockHdr.hdr.hBar=HNONDOCKBAR;
				m_dockHdr.rect.left=x+m_offset.cx;
				m_dockHdr.rect.top=y+m_offset.cy;
				m_dockHdr.rect.right=m_dockHdr.rect.left+m_size.cx;
				m_dockHdr.rect.bottom=m_dockHdr.rect.top+m_size.cy;
			}
			DrawGhostRect(m_dc,&m_dockHdr.rect);
		}
		bool ProcessWindowMessage(MSG* pMsg)
		{
			bool bHandled=false;
			switch(pMsg->message)
			{
				case WM_KEYDOWN:
				case WM_KEYUP:
					if(pMsg->wParam==VK_CONTROL)
					{
						CPoint point(pMsg->pt.x,pMsg->pt.y);
						::ScreenToClient(m_dockHdr.hdr.hWnd,&point);
						OnMove(point.x,point.y);
						bHandled=true;
					}
					break;
			}
		   return bHandled;
		}
	protected:
		const CDocker&	m_docker;
		CDC				m_dc;
		DFDOCKRECT&		m_dockHdr;
		SIZE			m_size;
		SIZE			m_offset;
	};
public:
	CDockingWindowBaseImpl(void)
		:m_hBarOwner(HNONDOCKBAR)
	{
		m_rcUndock.SetRectEmpty();
	}

	HWND Create(HWND hDockingFrameWnd, RECT& rcPos, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		m_docker=CDocker(hDockingFrameWnd);
		return baseClass::Create(hDockingFrameWnd, rcPos, szWindowName ,
									dwStyle , dwExStyle , nID , lpCreateParam);
	}

	HWND OwnerFrameWindow(void) const
	{
		return m_docker;
	}

	bool IsVisible(void) const
	{
		bool bRes;
#ifdef DF_AUTO_HIDE_FEATURES
		bRes=IsPinned();
		if(!bRes)
#endif //DF_AUTO_HIDE_FEATURES
			bRes=((GetStyle()&WS_VISIBLE)!=0);
		return bRes;
	}
#ifdef DF_AUTO_HIDE_FEATURES
	bool IsPinned(void) const
	{
		bool bRes=IsDocking();
		if(bRes)
		{
			DFMHDR dockHdr;
//			dockHdr.code=DC_ISPINNED;
			dockHdr.hWnd=m_hWnd;
			dockHdr.hBar=GetOwnerDockingBar();
			bRes=m_docker.IsPinned(&dockHdr);
		}
		return bRes;
	}
#endif
	HDOCKBAR GetOwnerDockingBar(void) const
	{
		return m_hBarOwner;
	}

	bool GetDockingPosition(DFDOCKPOS* pHdr) const
	{
		assert(::IsWindow(m_hWnd));
		bool bRes=true;
		pHdr->hdr.hBar=GetOwnerDockingBar();
		if(IsDocking())
		{
			pHdr->hdr.hWnd=m_hWnd;
//		    pHdr->hdr.code=DC_GETDOCKPOSITION;
			bRes=m_docker.GetDockingPosition(pHdr);
		}
		return bRes;
	}

	bool GetDockingWindowPlacement(DFDOCKPOSEX* pHdr) const
	{
		bool bRes=true;
		pHdr->bDocking=IsDocking();
		if(pHdr->bDocking)
		{
			::CopyRect(&pHdr->rect,&m_rcUndock);
			bRes=GetDockingPosition(&(pHdr->dockPos));
		}
		else
		{
			if(IsVisible())
				GetWindowRect(&pHdr->rect);
			else
				::CopyRect(&pHdr->rect,&m_rcUndock);
		}
		return bRes;
	}
	bool SetDockingPosition(DFDOCKPOS* pHdr)
	{
		assert(::IsWindow(m_hWnd));
		if(IsDocking())
					Undock();
		pHdr->hdr.hWnd=m_hWnd;
//	    pHdr->hdr.code=DC_SETDOCKPOSITION;
		return m_docker.SetDockingPosition(pHdr);
	}

	bool SetDockingWindowPlacement(DFDOCKPOSEX* pHdr)
	{
		bool bRes=true;
		if(pHdr->bDocking)
		{
			bRes=SetDockingPosition(&(pHdr->dockPos));
			::CopyRect(&m_rcUndock,&pHdr->rect);
		}
		else
		{
			if(IsDocking())
						Undock();
			bRes=(SetWindowPos(HWND_TOP,&(pHdr->rect),SWP_SHOWWINDOW | SWP_NOACTIVATE)!=FALSE);
		}
		return bRes;
	}

	bool IsDocking(void) const
	{
		return GetOwnerDockingBar()!=HNONDOCKBAR;
	}

	bool Float(LPCRECT pRc,UINT flags=SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_FRAMECHANGED,HWND hWndInsertAfter=HWND_TOP)
	{
		bool bRes=IsDocking();
		if(bRes)
		{
			if(Undock())
				bRes=(SetWindowPos(hWndInsertAfter,pRc,flags)!=FALSE);
		}
		return bRes;
	}

	bool Float(void)
	{
		bool bRes=!m_rcUndock.IsRectEmpty();
		if(bRes)
			bRes=Float(&m_rcUndock);
		return bRes;
	}

	virtual bool Undock(void)
	{
		assert(IsDocking());
		DFMHDR dockHdr;
//		dockHdr.code=DC_UNDOCK;
		dockHdr.hWnd=m_hWnd;
		dockHdr.hBar=GetOwnerDockingBar();
		return m_docker.Undock(&dockHdr);
	}

	bool OnClosing(void)
	{
		bool bRes=true;
		if(IsDocking())
			bRes=Undock();
		return bRes;
	}

	virtual bool DockMe(DFDOCKRECT* pHdr)
	{
		return m_docker.Dock(pHdr);
	}

    bool BeginMoving(const POINT& point)
    {
		DFDOCKRECT dockHdr;
//		dockHdr.hdr.code=DC_ACCEPT;
		dockHdr.hdr.hWnd=m_hWnd;
		dockHdr.hdr.hBar=HNONDOCKBAR;//GetOwnerDockingBar();

		if(m_rcUndock.IsRectEmpty())
		{
			GetWindowRect(&dockHdr.rect);
//			dockHdr.hdr.code=DC_ADJUSTDRAGRECT;
			m_docker.AdjustDragRect(&dockHdr);
			m_rcUndock.CopyRect(&dockHdr.rect);
		}
		GetWindowRect(&dockHdr.rect);
		CPoint pt(point);
		ClientToScreen(&pt);

// 		float ratio=float(pt.x-dockHdr.rect.left)/(dockHdr.rect.right-dockHdr.rect.left);
// 		dockHdr.rect.left=pt.x-long(ratio*m_rcUndock.Width());
		dockHdr.rect.left=pt.x-::MulDiv(m_rcUndock.Width(),pt.x-dockHdr.rect.left,dockHdr.rect.right-dockHdr.rect.left);
// 		ratio=float(pt.y-dockHdr.rect.top)/(dockHdr.rect.bottom-dockHdr.rect.top);
// 		dockHdr.rect.top=pt.y-long(ratio*m_rcUndock.Height());
		dockHdr.rect.top=pt.y-::MulDiv(m_rcUndock.Height(),pt.y-dockHdr.rect.top,dockHdr.rect.bottom-dockHdr.rect.top);


		dockHdr.rect.right=dockHdr.rect.left+m_rcUndock.Width();
		dockHdr.rect.bottom=dockHdr.rect.top+m_rcUndock.Height();

		CGhostMoveTracker tracker(m_docker,point,dockHdr);
		if(TrackDragAndDrop(tracker,m_hWnd))
		{
			CPoint ptCur;
			::GetCursorPos(&ptCur);
			if((dockHdr.hdr.hBar!=HNONDOCKBAR)
				|| (ptCur.x!=pt.x) || (ptCur.y!=pt.y))
			{
				if(IsDocking())
							Undock();
				if(dockHdr.hdr.hBar!=HNONDOCKBAR)
//						m_docker.Dock(&dockHdr);
					DockMe(&dockHdr);
				else
					SetWindowPos(HWND_TOP,&(dockHdr.rect),SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			}
		}
		return true;
	}
	void OnDocked(HDOCKBAR hBar,bool /*bHorizontal*/)
	{
		assert(!IsDocking());
		m_hBarOwner=hBar;
	}
	void OnUndocked(HDOCKBAR /*hBar*/)
	{
		assert(IsDocking());
		m_hBarOwner=HNONDOCKBAR;
	}
////////////////messages handlers/////////////////////////////////
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_NCLBUTTONDOWN,OnNcLButtonDown)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MESSAGE_HANDLER(WMDF_NDOCKSTATECHANGED,OnDockStateChanged)
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	LRESULT OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		LPWINDOWPOS pWPos=reinterpret_cast<LPWINDOWPOS>(lParam);

		if(
			(pWPos->flags&SWP_SHOWWINDOW
			|| ((pWPos->flags&(SWP_NOSIZE | SWP_NOMOVE))!=(SWP_NOSIZE | SWP_NOMOVE)))
				&& !IsDocking()
					&& IsVisible()
						/*&& !(pWPos->flags&SWP_HIDEWINDOW)*/)
		{
			m_rcUndock.left=pWPos->x;
			m_rcUndock.top=pWPos->y;
			m_rcUndock.right=m_rcUndock.left+pWPos->cx;
			m_rcUndock.bottom=m_rcUndock.top+pWPos->cy;
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM/* lParam*/, BOOL& bHandled)
	{
		T* pThis=reinterpret_cast<T*>(this);
		bHandled=!pThis->OnClosing();
		return !bHandled;
	}

    LRESULT OnNcLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		bHandled=(wParam==HTCAPTION);
        if(bHandled)
        {
			SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE );
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if(::DragDetect(m_hWnd,pt))
			{
				T* pThis=static_cast<T*>(this);
				pThis->ScreenToClient(&pt);
				pThis->BeginMoving(pt);
			}
        }
        return 0;
    }

	LRESULT OnDockStateChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
         T* pThis=static_cast<T*>(this);
		if(wParam!=FALSE)
			pThis->OnDocked(reinterpret_cast<HDOCKBAR>(lParam),(DOCKED2HORIZONTAL(wParam)==TRUE));
		else
			pThis->OnUndocked(reinterpret_cast<HDOCKBAR>(lParam));
		return TRUE;
	}
protected:
	CDocker		m_docker;
	HDOCKBAR	m_hBarOwner;
	CRect		m_rcUndock;
};

class CVirtualButton 
	: public CRect
{
	enum {Normal=0,Hotted=1,Pressed=2};
public:
	CVirtualButton(void)
		:m_state(Normal)
	{

	}
	virtual void Draw(CDCHandle dc)=0
	{
		if(m_state!=Normal)
		{
			UINT edge=((m_state&Pressed)!=0) ? BDR_SUNKENOUTER: BDR_RAISEDINNER;
			dc.DrawEdge(this,edge/*|BF_ADJUST*/ ,BF_RECT); //look like button raise
		}
	}

	void Hot(HWND hwnd,bool hot)
	{
		if((m_state&Pressed)==0
			&& (((m_state&Hotted)!=0)!=hot))
		{
			if(hot)
				m_state|=Hotted;
			else
				m_state&=~Hotted;

			if((m_state&Hotted)!=0)
				::SetTimer(hwnd,reinterpret_cast<UINT_PTR>(this),300,&TimeTrack);
			else
				::KillTimer(hwnd,reinterpret_cast<UINT_PTR>(this));
			CWindowDC dc(hwnd);
			Draw(dc.m_hDC);
		}
	}

	void Press(HWND hwnd,bool press)
	{
		Hot(hwnd,false);
		if(press)
			m_state|=Pressed;
		else
			m_state&=~Pressed;
		CWindowDC dc(hwnd);
		Draw(dc.m_hDC);
	}
protected:
	static void CALLBACK TimeTrack(HWND hwnd,UINT /*msg*/,UINT_PTR id,	DWORD /*time*/)
	{
		CVirtualButton* btn=reinterpret_cast<CVirtualButton*>(id);
		POINT pt;
		RECT rc;
		if(!::GetCursorPos(&pt)
			||!::GetWindowRect(hwnd,&rc)
				|| (pt.x-=rc.left,pt.y-=rc.top,!::PtInRect(btn,pt))  )
			btn->Hot(hwnd,false);
	}
private:
	int m_state;
};

class CCaptionBase : public COrientedRect
{
	typedef COrientedRect baseClass;
public:
	typedef CVirtualButton CButton;
public:
	CCaptionBase(bool bHorizontal=true)
		:COrientedRect(bHorizontal,::GetSystemMetrics(SM_CYSMCAPTION))
	{
	}
	CCaptionBase(unsigned long thickness,bool bHorizontal=true)
		:COrientedRect(bHorizontal,thickness)
	{
	}

	bool CalculateRect(CRect& rc,bool bTop)
	{
		return baseClass::CalculateRect(rc,bTop);
	}
	LRESULT HitTest(const CPoint& /*pt*/) const
	{
		return HTNOWHERE;
	}
	void Draw(HWND /*hWnd*/,CDC& dc)
	{
		dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
	}
	void UpdateMetrics(void)
	{
		// Override in derived class if it depends on system metrics
	}
};


template <class TCaption,DWORD t_dwStyle = 0, DWORD t_dwExStyle = 0>
struct CDockingWindowTraits : CDockingBarWinTraits<t_dwStyle,t_dwExStyle>
{
	typedef TCaption CCaption;
};

typedef CDockingWindowTraits<CCaptionBase,WS_OVERLAPPEDWINDOW | WS_POPUP/* WS_CHILD*/ | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW/* WS_EX_CLIENTEDGE*/> CEmptyTitleDockingWindowTraits;

template <class T,
          class TBase = CWindow,
          class TDockingWinTraits = CEmptyTitleDockingWindowTraits>
class ATL_NO_VTABLE CTitleDockingWindowBaseImpl :
	public CDockingWindowBaseImpl< T, TBase, TDockingWinTraits >
{
    typedef CDockingWindowBaseImpl< T, TBase, TDockingWinTraits >		baseClass;
    typedef CTitleDockingWindowBaseImpl< T, TBase, TDockingWinTraits >	thisClass;
protected:
	typedef typename TDockingWinTraits::CCaption	CCaption;
public:
	void GetMinMaxInfo(LPMINMAXINFO pMinMaxInfo) const
	{
		long width=m_caption.GetThickness();
		if(pMinMaxInfo->ptMinTrackSize.y<width)
			pMinMaxInfo->ptMinTrackSize.y=width;
		if(pMinMaxInfo->ptMinTrackSize.x<width)
			pMinMaxInfo->ptMinTrackSize.x=width;
	}

	void NcCalcSize(CRect* pRc)
	{
		DWORD style = GetWindowLong(GWL_STYLE);
		if((style&WS_CAPTION)==0)
			m_caption.SetRectEmpty();
		else
			m_caption.CalculateRect(*pRc,true);
	}

	unsigned int NcHitTest(CPoint pt)
	{
		LRESULT lRes=HTNOWHERE;
		RECT rc;
		if(GetWindowRect(&rc))
		{
			pt.x-=rc.left;
			pt.y-=rc.top;
			lRes=m_caption.HitTest(pt);
			if(lRes==HTNOWHERE)
				lRes=HTCLIENT;
		}
		return lRes;
	}

	void NcMouseMove(const CPoint& /*pt*/,unsigned int nHitTest)
	{
		m_caption.HotTrack(m_hWnd,nHitTest);
	}

	BOOL NcLButtonDown(const CPoint& pt,unsigned int nHitTest)
	{
		T* pThis=static_cast<T*>(this);
		bool res=m_caption.Action(m_hWnd,pt,nHitTest);
		if(res)
		{
			CDropPointTracker tracker;
			bool drop=TrackDragAndDrop(tracker,pThis->m_hWnd);
			if(drop)
			{
				CPoint pt(tracker.DropPoint());
				drop=ClientToScreen(&pt)
						&& (NcHitTest(pt)==nHitTest);
			}
			m_caption.ActionDone(m_hWnd,nHitTest,drop);
			if(drop)
			{
				switch(nHitTest)
				{
					case HTCLOSE:
						pThis->CloseBtnPress();
						break;
#ifdef DF_AUTO_HIDE_FEATURES
					case HTPIN:
						pThis->PinBtnPress();
						break;
#endif
				}
			}
		}
		return res;
	}

	void NcDraw(CDC& dc)
	{
		DWORD style = GetWindowLong(GWL_STYLE);
		if((style&WS_CAPTION)!=0)
			m_caption.Draw(m_hWnd,dc);
	}
	void OnDocked(HDOCKBAR hBar,bool bHorizontal)
	{
		m_caption.SetOrientation(!bHorizontal);
		baseClass::OnDocked(hBar,bHorizontal);
	}
	void OnUndocked(HDOCKBAR hBar)
	{
		m_caption.SetOrientation(true);
		baseClass::OnUndocked(hBar);
	}
	bool CloseBtnPress(void)
	{
		PostMessage(WM_CLOSE);
		return false;
	}
#ifdef DF_AUTO_HIDE_FEATURES
	bool PinUp(const CDockingSide& side)
	{
		CRect rc;
		GetWindowRect(&rc);
		T* pThis=static_cast<T*>(this);
		assert(rc.Width()>0);
		assert(rc.Height()>0);
		return pThis->PinUp(side,(side.IsHorizontal() ? rc.Width() : rc.Height()));
	}

	bool PinUp(const CDockingSide& side,unsigned long width,bool bVisualize=false)
	{
		if(IsDocking())
					Undock();
		DFPINUP pinHdr;
		pinHdr.hdr.hWnd=m_hWnd;
		pinHdr.hdr.hBar=GetOwnerDockingBar();
//		pinHdr.hdr.code=DC_PINUP;
		pinHdr.dwDockSide=side;
		pinHdr.nWidth=width;
		pinHdr.dwFlags= bVisualize ? DFPU_VISUALIZE : 0 ;
		pinHdr.n=0;
		return m_docker.PinUp(&pinHdr);
	}

	bool PinBtnPress(void)
	{
		assert(IsDocking());
		DFDOCKPOS dockHdr;
//		dockHdr.hdr.code=DC_GETDOCKPOSITION;
		dockHdr.hdr.hWnd=m_hWnd;
		dockHdr.hdr.hBar=GetOwnerDockingBar();
		bool bRes=GetDockingPosition(&dockHdr);
		if(bRes)
		{
			bRes=Undock();
			if(bRes)
			{
				T* pThis=static_cast<T*>(this);
				bRes=pThis->PinUp(CDockingSide(dockHdr.dwDockSide),dockHdr.nWidth,true);
			}
		}
		return bRes;
	}
#endif
protected:
	BEGIN_MSG_MAP(thisClass)
		if(IsDocking())
		{
			MESSAGE_HANDLER(WM_WINDOWPOSCHANGING,OnWindowPosChanging)
			MESSAGE_HANDLER(WM_NCCALCSIZE, OnNcCalcSize)
			MESSAGE_HANDLER(WM_NCACTIVATE, OnNcActivate)
			MESSAGE_HANDLER(WM_NCHITTEST,OnNcHitTest)
			MESSAGE_HANDLER(WM_NCPAINT,OnNcPaint)
			MESSAGE_HANDLER(WM_NCMOUSEMOVE, OnNcMouseMove)
			MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNcLButtonDown)
			MESSAGE_HANDLER(WM_NCLBUTTONDBLCLK,OnNcLButtonDblClk)
			MESSAGE_HANDLER(WM_SETTEXT,OnCaptionChange)
			MESSAGE_HANDLER(WM_SETICON,OnCaptionChange)
#ifdef DF_FOCUS_FEATURES
			assert(CDockingFocusHandler::This());
			CHAIN_MSG_MAP_ALT_MEMBER((*CDockingFocusHandler::This()),0)
#endif
		}
		MESSAGE_HANDLER(WM_GETMINMAXINFO,OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_SETTINGCHANGE,OnSettingChange)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE,OnSettingChange/*OnSysColorChange*/)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		T* pThis=static_cast<T*>(this);
		LRESULT lRes=pThis->DefWindowProc(uMsg,wParam,lParam);
		pThis->GetMinMaxInfo(reinterpret_cast<LPMINMAXINFO>(lParam));
		return lRes;
	}

	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(!IsDocking())
		{
			// If we're floating, we're a top level window.
			// We might be getting this message before the main frame
			// (which is also a top-level window).
			// The main frame handles this message, and refreshes
			// system settings cached in CDWSettings.  In case we
			// are getting this message before the main frame,
			// update these cached settings (so that when we update
			// our caption's settings that depend on them,
			// its using the latest).
			CDWSettings settings;
			settings.Update();

			// In addition, because we are a top-level window,
			// we should be sure to send this message to all our descendants
			// in case there are common controls and other windows that
			// depend on cached system metrics.
			this->SendMessageToDescendants(uMsg, wParam, lParam, TRUE);
		}

		m_caption.UpdateMetrics();

		T* pThis=static_cast<T*>(this);
		pThis->SetWindowPos(NULL,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		bHandled = FALSE;
		return 1;
	}
/*
	LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}
*/
	LRESULT OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return NULL;
	}

	LRESULT OnNcActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled=IsWindowEnabled();
		return TRUE;
	}

	LRESULT OnNcCalcSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
        T* pThis=static_cast<T*>(this);
		CRect* pRc=reinterpret_cast<CRect*>(lParam);
		CPoint ptTop(pRc->TopLeft());
		(*pRc)-=ptTop;
		pThis->NcCalcSize(pRc);
		(*pRc)+=ptTop;
		return NULL;
	}

	unsigned int OnNcHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return static_cast<T*>(this)->NcHitTest(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
	}

	LRESULT OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CWindowDC dc(m_hWnd);
		T* pThis=static_cast<T*>(this);
		pThis->NcDraw(dc);
		return NULL;
	}

	LRESULT OnNcMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		T* pThis=static_cast<T*>(this);
		pThis->NcMouseMove(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)),wParam);
		return NULL;
	}

	LRESULT OnNcLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pThis=static_cast<T*>(this);
		bHandled=pThis->NcLButtonDown(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)),wParam);
        return !bHandled;
	}

	LRESULT OnNcLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}
//OnSetIcon
//OnSetText
	LRESULT OnCaptionChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
//		LockWindowUpdate();
		DWORD style = ::GetWindowLong(m_hWnd,GWL_STYLE);
		::SetWindowLong(m_hWnd, GWL_STYLE, style&(~WS_CAPTION));
		LRESULT lRes=DefWindowProc(uMsg,wParam,lParam);
		::SetWindowLong(m_hWnd, GWL_STYLE, style);
		T* pThis=static_cast<T*>(this);
		pThis->SetWindowPos(NULL,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
//		CWindowDC dc(m_hWnd);
//		pThis->NcDraw(dc);
//		LockWindowUpdate(FALSE);
		return lRes;
	}

protected:
	CCaption	m_caption;
};

template <class T,
          class TBase = CWindow,
          class TDockingWinTraits = CEmptyTitleDockingWindowTraits>
class ATL_NO_VTABLE CTitleDockingWindowImpl
			: public CTitleDockingWindowBaseImpl< T, TBase, TDockingWinTraits >
{
    typedef CTitleDockingWindowBaseImpl< T, TBase, TDockingWinTraits >	baseClass;
    typedef CTitleDockingWindowImpl< T, TBase, TDockingWinTraits >		thisClass;
public:
	CTitleDockingWindowImpl(void)
	{
		m_pos.hdr.hBar=HNONDOCKBAR;
	}
	virtual bool Undock(void)
	{
		assert(IsDocking());
		GetDockingPosition(&m_pos);
		return baseClass::Undock();
	}
	virtual bool Hide(void)
	{
		bool bRes=true;
		if(IsDocking())
		{
			bRes=GetDockingPosition(&m_pos);
			assert(bRes);
			if(bRes)
			//	bRes=Undock();
				bRes=Float(&m_rcUndock,SWP_HIDEWINDOW);
			assert(bRes);
		}
		else
			m_pos.hdr.hBar=HNONDOCKBAR;
		return (bRes && ShowWindow(SW_HIDE));
	}
	virtual bool Show(void)
	{
		bool bRes=true;
		if(m_pos.hdr.hBar!=HNONDOCKBAR)
			bRes=SetDockingPosition(&m_pos);
		else
			ShowWindow(SW_SHOW);
		assert(bRes);
		return bRes;
	}

	bool Toggle(void)
	{
		bool bRes=(static_cast<T*>(this)->IsVisible()!=FALSE);
		if(bRes)
		{
			Hide();
			::SetFocus(::GetParent (m_hWnd));
		}
		else
			Show();
		return bRes;
	}
	bool GetDockingWindowPlacement(DFDOCKPOSEX* pHdr) const
	{
		bool bRes=baseClass::GetDockingWindowPlacement(pHdr);
		pHdr->bVisible=static_cast<const T*>(this)->IsVisible();
		if( (!pHdr->bDocking
			  || (!pHdr->bVisible) )
			  /*&& (m_pos.hdr.hBar!=HNONDOCKBAR)*/)
			::CopyMemory(&pHdr->dockPos,&m_pos,sizeof(DFDOCKPOS));
		return bRes;
	}
	bool SetDockingWindowPlacement(DFDOCKPOSEX* pHdr)
	{
		bool bRes=true;
		pHdr->dockPos.hdr.hWnd=m_hWnd;
		::CopyMemory(&m_pos,&(pHdr->dockPos),sizeof(DFDOCKPOS));
		if(pHdr->bVisible)
			bRes=baseClass::SetDockingWindowPlacement(pHdr);
		else
		{
			if(IsDocking())
						Undock();
			::CopyRect(&m_rcUndock,&pHdr->rect);
			bRes=(SetWindowPos(NULL,&m_rcUndock,SWP_NOZORDER | SWP_HIDEWINDOW |
													SWP_NOACTIVATE )!=FALSE);
		}
		return bRes;
	}

	bool CanBeClosed(unsigned long /*param*/)
	{
		Hide();
		return false;
	}

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_NCLBUTTONDBLCLK, OnNcLButtonDblClk)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM/* lParam*/, BOOL& bHandled)
	{
		bHandled=!(static_cast<T*>(this)->CanBeClosed(wParam));
		return 0;
	}
	LRESULT OnNcLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(wParam==HTCAPTION)
		{
			if(IsDocking())
				Float();
			else
			{
				if(m_pos.hdr.hBar!=HNONDOCKBAR)
					SetDockingPosition(&m_pos);
			}
		}
		return 0;
	}
protected:
	DFDOCKPOS m_pos;
};

#define COMMAND_TOGGLE_MEMBER_HANDLER(id, member) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		member.Toggle(); \
	}

//please don't use CStateKeeper class anymore!
//this class is obsolete and provided only for compatibility with previous versions.
//the CTitleDockingWindowImpl class provide all functionality of CStateKeeper
template<class T>
struct CStateKeeper : public T
{
};

//please don't use CTitleExDockingWindowImpl class anymore!
//this class is obsolete and provided only for compatibility with previous versions.
//the CTitleDockingWindowImpl class provide all functionality of CTitleExDockingWindowImpl
template <class T,
          class TBase = CWindow,
          class TDockingWinTraits = CEmptyTitleDockingWindowTraits>
struct ATL_NO_VTABLE CTitleExDockingWindowImpl : CTitleDockingWindowImpl< T, TBase, TDockingWinTraits >
{
};

}//namespace dockwins
#endif // WTL_DW_DOCKINGWINDOW_H_INCLUDED_
