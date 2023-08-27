
#pragma once

struct RWType
{
	GUID id; // (encoded) name
	size_t size;
};

struct RWStruct : public RWType
{
	struct SField
	{
		GUID name;
		size_t offset;
		RWCPtr<RWType> type;
		RWCPtr<RWNode> defaultValue;
	};
	size_t count;
	SField fields[0];
};

struct RWArray : public RWType
{
	RWCPtr<RWType> itemType;
};

struct SNode
{
	SNode() : refs(1) {}
	virtual ~SNode() {}
	LONG refs;
	virtual void AddRef() { InterlockedIncrement(&refs); }
	virtual void Release() { if (InterlockedDecrement(&refs) == 0) delete this; }
	virtual void QueryInterface() {  }

	RWCPtr<RWType> type;
};

struct SNodeInt32 : public SNode
{
	int val;
};

struct SNodeFloat32 : public SNode
{
	float val;
};

struct SNode2DSubPath : public SNode
{
	struct Vector2f
	{
		float x;
		float y;
	};
	enum PointFlags
	{
		Smooth = 0,
		Sharp = 1,
	};
	struct CurvePoint
	{
		Vector2f pos;
		Vector2f tanNext;
		Vector2f tanPrev;
		HandleFlags flags;
	};
	size_t count;
	SField points[0];
};

struct SNode2DPath : public SNode
{
	size_t count;
	RWCPtr<SNode2DSubPath> subPaths[0];
};

struct RWColor : public SNode
{
	RWCPtr<RWColor> Create(RWCPtr<SNodeFloat32> r, RWCPtr<SNodeFloat32> g, RWCPtr<SNodeFloat32> b, RWCPtr<SNodeFloat32> a)
	{
		return new RWColor(r, g, b, a);
	}

	RWCPtr<SNodeFloat32> r;
	RWCPtr<SNodeFloat32> g;
	RWCPtr<SNodeFloat32> b;
	RWCPtr<SNodeFloat32> a;

private:
	RWColor(RWCPtr<SNodeFloat32> r, RWCPtr<SNodeFloat32> g, RWCPtr<SNodeFloat32> b, RWCPtr<SNodeFloat32> a) : r(r), g(g), b(b), a(a) {}
};

struct RWSolidFill : public SNode
{
	typedef 
	RWCPtr<RWSolidFill> Create(RWCPtr<RWColor> color)
	{
		return new RWSolidFill(color);
	}

	RWCPtr<RWColor> color;

private:
	RWSolidFill(RWCPtr<RWColor> color) : r(r), g(g), b(b), a(a) {}
};

struct RWGraph
{
};

struct RWAdapterManager
{
	template<typename TDst, typename TSrc>
	RWCPtr<TSrc> convert(RWCPtr<TSrc> src)
	{
	}
};

void dummy(RWCPtr<SNode> color)
{
	RWCPtr<RWColor> myColor = color.into<RWColor>(color);
	myColor->r->val;
}
