// Copyright (c) 2000
// Sergey Klimov (kidd@ukr.net)

#ifndef WTL_DW_DDTRACKER_H_INCLUDED_
#define WTL_DW_DDTRACKER_H_INCLUDED_

#include<cassert>

class IDDTracker
{
public:
    virtual void BeginDrag(){}
    virtual void EndDrag(bool /*bCanceled*/){}
    virtual void OnMove(long /*x*/, long /*y*/){}

    virtual void OnCancelDrag(long /*x*/, long /*y*/){}
    virtual bool OnDrop(long /*x*/, long /*y*/)
	{
		return true;
	}
    virtual bool OnDropRightButton(long x, long y)
    {
       return OnDrop(x,y);
    }
    virtual bool OnDropLeftButton(long x, long y)
    {
       return OnDrop(x,y);
    }
    virtual bool ProcessWindowMessage(MSG* /*pMsg*/)
    {
       return false;
    }


};
template<class T>
class CDDTrackerBaseT
{
public:
    void BeginDrag(){}
    void EndDrag(bool /*bCanceled*/){}
    void OnMove(long /*x*/, long /*y*/){}

    void OnCancelDrag(long /*x*/, long /*y*/){}
    bool OnDrop(long /*x*/, long /*y*/)
	{
		return true;
	}
    bool OnDropRightButton(long x, long y)
    {
       return static_cast<T*>(this)->OnDrop(x,y);
    }
    bool OnDropLeftButton(long x, long y)
    {
       return static_cast<T*>(this)->OnDrop(x,y);
    }
    bool ProcessWindowMessage(MSG* /*pMsg*/)
    {
       return false;
    }
};

template<class T>
bool TrackDragAndDrop(T& tracker,HWND hWnd)
{
    bool bResult=true;
    tracker.BeginDrag();
    ::SetCapture(hWnd);
    MSG msg;
    while((::GetCapture()==hWnd)&&
            (GetMessage(&msg, NULL, 0, 0)))
    {
		if(!tracker.ProcessWindowMessage(&msg))
		{
		  switch(msg.message)
		  {
			   case WM_MOUSEMOVE:
					tracker.OnMove(GET_X_LPARAM( msg.lParam), GET_Y_LPARAM(msg.lParam));
					break;
			   case WM_RBUTTONUP:
					::ReleaseCapture();
					bResult=tracker.OnDropRightButton(GET_X_LPARAM( msg.lParam), GET_Y_LPARAM(msg.lParam));
					break;
			   case WM_LBUTTONUP:
					::ReleaseCapture();
					bResult=tracker.OnDropLeftButton(GET_X_LPARAM( msg.lParam), GET_Y_LPARAM(msg.lParam));
					break;
			   case WM_KEYDOWN:
					if(msg.wParam!=VK_ESCAPE)
						break;
			   case WM_RBUTTONDOWN:
			   case WM_LBUTTONDOWN:
					::ReleaseCapture();
					tracker.OnCancelDrag(GET_X_LPARAM( msg.lParam), GET_Y_LPARAM(msg.lParam));
					bResult=false;
					break;
			   case WM_SYSKEYDOWN:
					::ReleaseCapture();
					tracker.OnCancelDrag(GET_X_LPARAM( msg.lParam), GET_Y_LPARAM(msg.lParam));
					bResult=false;
			   default:
					DispatchMessage(&msg);
		  }
		}
    }
    tracker.EndDrag(!bResult);
    assert(::GetCapture()!=hWnd);
    return bResult;
}

class CDropPointTracker 
	: public CDDTrackerBaseT<CDropPointTracker>
{
public:
	bool OnDrop(long x, long y)
	{
		pt_.x=x;
		pt_.y=y;
		return true;
	}
	const POINT& DropPoint(void) const
	{
		return pt_;
	}
private:
	POINT pt_;
};

#endif // WTL_DW_DDTRACKER_H_INCLUDED_
