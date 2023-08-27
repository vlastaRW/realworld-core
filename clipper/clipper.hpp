/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  6.2.1                                                           *
* Date      :  31 October 2014                                                 *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2014                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
* Attributions:                                                                *
* The code in this library is an extension of Bala Vatti's clipping algorithm: *
* "A generic solution to polygon clipping"                                     *
* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
* http://portal.acm.org/citation.cfm?id=129906                                 *
*                                                                              *
* Computer graphics and geometric modeling: implementation and algorithms      *
* By Max K. Agoston                                                            *
* Springer; 1 edition (January 4, 2005)                                        *
* http://books.google.com/books?q=vatti+clipping+agoston                       *
*                                                                              *
* See also:                                                                    *
* "Polygon Offsetting by Computing Winding Numbers"                            *
* Paper no. DETC2005-85513 pp. 565-575                                         *
* ASME 2005 International Design Engineering Technical Conferences             *
* and Computers and Information in Engineering Conference (IDETC/CIE2005)      *
* September 24-28, 2005 , Long Beach, California, USA                          *
* http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf              *
*                                                                              *
*******************************************************************************/

#ifndef clipper_hpp
#define clipper_hpp

#define CLIPPER_VERSION "6.2.0"

//use_int32: When enabled 32bit ints are used instead of 64bit ints. This
//improve performance but coordinate values are limited to the range +/- 46340
//#define use_int32

//use_xyz: adds a Z member to IntPoint. Adds a minor cost to perfomance.
//#define use_xyz

//use_lines: Enables line clipping. Adds a very minor cost to performance.
//#define use_lines
  
//use_deprecated: Enables temporary support for the obsolete functions
//#define use_deprecated  

#include <vector>
#include <set>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <ostream>
#include <functional>
#include <queue>

namespace ClipperLib {

enum ClipType { ctIntersection, ctUnion, ctDifference, ctXor };
enum PolyType { ptSubject, ptClip };
//By far the most widely used winding rules for polygon filling are
//EvenOdd & NonZero (GDI, GDI+, XLib, OpenGL, Cairo, AGG, Quartz, SVG, Gr32)
//Others rules include Positive, Negative and ABS_GTR_EQ_TWO (only in OpenGL)
//see http://glprogramming.com/red/chapter11.html
enum PolyFillType { pftEvenOdd, pftNonZero, pftPositive, pftNegative };

#ifdef use_int32
  typedef int cInt;
  static cInt const loRange = 0x7FFF;
  static cInt const hiRange = 0x7FFF;
#else
  typedef signed long long cInt;
  static cInt const loRange = 0x3FFFFFFF;
  static cInt const hiRange = 0x3FFFFFFFFFFFFFFFLL;
  typedef signed long long long64;     //used by Int128 class
  typedef unsigned long long ulong64;

#endif

struct IntPoint {
  cInt X;
  cInt Y;
#ifdef use_xyz
  cInt Z;
  IntPoint(cInt x = 0, cInt y = 0, cInt z = 0): X(x), Y(y), Z(z) {};
#else
  IntPoint(cInt x = 0, cInt y = 0): X(x), Y(y) {};
#endif

  friend inline bool operator== (const IntPoint& a, const IntPoint& b)
  {
    return a.X == b.X && a.Y == b.Y;
  }
  friend inline bool operator!= (const IntPoint& a, const IntPoint& b)
  {
    return a.X != b.X  || a.Y != b.Y; 
  }
};
//------------------------------------------------------------------------------

typedef std::vector< IntPoint > Path;
typedef std::vector< Path > Paths;

inline Path& operator <<(Path& poly, const IntPoint& p) {poly.push_back(p); return poly;}
inline Paths& operator <<(Paths& polys, const Path& p) {polys.push_back(p); return polys;}

std::ostream& operator <<(std::ostream &s, const IntPoint &p);
std::ostream& operator <<(std::ostream &s, const Path &p);
std::ostream& operator <<(std::ostream &s, const Paths &p);

struct DoublePoint
{
  double X;
  double Y;
  DoublePoint(double x = 0, double y = 0) : X(x), Y(y) {}
  DoublePoint(IntPoint ip) : X((double)ip.X), Y((double)ip.Y) {}
};
//------------------------------------------------------------------------------

#ifdef use_xyz
typedef void (*ZFillCallback)(IntPoint& e1bot, IntPoint& e1top, IntPoint& e2bot, IntPoint& e2top, IntPoint& pt);
#endif

enum InitOptions {ioReverseSolution = 1, ioStrictlySimple = 2, ioPreserveCollinear = 4};
enum JoinType {jtSquare, jtRound, jtMiter};
enum EndType {etClosedPolygon, etClosedLine, etOpenButt, etOpenSquare, etOpenRound};

class PolyNode;
typedef std::vector< PolyNode* > PolyNodes;

class PolyNode 
{ 
public:
    PolyNode();
    virtual ~PolyNode(){};
    Path Contour;
    PolyNodes Childs;
    PolyNode* Parent;
    PolyNode* GetNext() const;
    bool IsHole() const;
    bool IsOpen() const;
    int ChildCount() const;
private:
    unsigned Index; //node index in Parent.Childs
    bool m_IsOpen;
    JoinType m_jointype;
    EndType m_endtype;
    PolyNode* GetNextSiblingUp() const;
    void AddChild(PolyNode& child);
    friend class Clipper; //to access Index
    friend class ClipperOffset; 
};

class PolyTree: public PolyNode
{ 
public:
    ~PolyTree(){Clear();};
    PolyNode* GetFirst() const;
    void Clear();
    int Total() const;
private:
    PolyNodes AllNodes;
    friend class Clipper; //to access AllNodes
};

bool Orientation(const Path &poly);
double Area(const Path &poly);
int PointInPolygon(const IntPoint &pt, const Path &path);

void SimplifyPolygon(const Path &in_poly, Paths &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(const Paths &in_polys, Paths &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(Paths &polys, PolyFillType fillType = pftEvenOdd);

void CleanPolygon(const Path& in_poly, Path& out_poly, double distance = 1.415);
void CleanPolygon(Path& poly, double distance = 1.415);
void CleanPolygons(const Paths& in_polys, Paths& out_polys, double distance = 1.415);
void CleanPolygons(Paths& polys, double distance = 1.415);

void MinkowskiSum(const Path& pattern, const Path& path, Paths& solution, bool pathIsClosed);
void MinkowskiSum(const Path& pattern, const Paths& paths, Paths& solution, bool pathIsClosed);
void MinkowskiDiff(const Path& poly1, const Path& poly2, Paths& solution);

void PolyTreeToPaths(const PolyTree& polytree, Paths& paths);
void ClosedPathsFromPolyTree(const PolyTree& polytree, Paths& paths);
void OpenPathsFromPolyTree(PolyTree& polytree, Paths& paths);

void ReversePath(Path& p);
void ReversePaths(Paths& p);

struct IntRect { cInt left; cInt top; cInt right; cInt bottom; };

//enums that are used internally ...
enum EdgeSide { esLeft = 1, esRight = 2};

//forward declarations (for stuff used internally) ...
struct TEdge {
  IntPoint Bot;
  IntPoint Curr;
  IntPoint Top;
  IntPoint Delta;
  double Dx;
  PolyType PolyTyp;
  EdgeSide Side;
  int WindDelta; //1 or -1 depending on winding direction
  int WindCnt;
  int WindCnt2; //winding count of the opposite polytype
  int OutIdx;
  TEdge *Next;
  TEdge *Prev;
  TEdge *NextInLML;
  TEdge *NextInAEL;
  TEdge *PrevInAEL;
  TEdge *NextInSEL;
  TEdge *PrevInSEL;
};
struct IntersectNode;
struct LocalMinimum {
  cInt          Y;
  TEdge        *LeftBound;
  TEdge        *RightBound;
};
struct Scanbeam;
struct OutPt;
struct OutRec;
struct Join;

typedef std::vector < OutRec* > PolyOutList;
typedef std::vector < TEdge* > EdgeList;
typedef std::vector < Join* > JoinList;
typedef std::vector < IntersectNode* > IntersectList;

static int const Unassigned = -1;  //edge not currently 'owning' a solution
static int const Skip = -2;        //edge that would otherwise close a path

void RangeTest(const IntPoint& Pt, bool& useFullRange);
inline void InitEdge(TEdge* e, TEdge* eNext, TEdge* ePrev, const IntPoint& Pt)
{
  std::memset(e, 0, sizeof(TEdge));
  e->Next = eNext;
  e->Prev = ePrev;
  e->Curr = Pt;
  e->OutIdx = Unassigned;
}
void InitEdge2(TEdge& e, PolyType Pt);
TEdge* RemoveEdge(TEdge* e);
bool SlopesEqual(const IntPoint pt1, const IntPoint pt2, const IntPoint pt3, bool UseFullInt64Range);
bool SlopesEqual(const IntPoint pt1, const IntPoint pt2, const IntPoint pt3, const IntPoint pt4, bool UseFullInt64Range);
bool Pt2IsBetweenPt1AndPt3(const IntPoint pt1, const IntPoint pt2, const IntPoint pt3);
inline void Swap(cInt& val1, cInt& val2)
{
  cInt tmp = val1;
  val1 = val2;
  val2 = tmp;
}
inline void ReverseHorizontal(TEdge &e)
{
  //swap horizontal edges' Top and Bottom x's so they follow the natural
  //progression of the bounds - ie so their xbots will align with the
  //adjoining lower edge. [Helpful in the ProcessHorizontal() method.]
  Swap(e.Top.X, e.Bot.X);
#ifdef use_xyz  
  Swap(e.Top.Z, e.Bot.Z);
#endif
}
TEdge* FindNextLocMin(TEdge* E);

//------------------------------------------------------------------------------

//ClipperBase is the ancestor to the Clipper class. It should not be
//instantiated directly. This class simply abstracts the conversion of sets of
//polygon coordinates into edge objects that are stored in a LocalMinima list.
class ClipperBase
{
public:
  ClipperBase();
  virtual ~ClipperBase();
  //bool AddPath(const Path &pg, PolyType PolyTyp, bool Closed);
  template<typename TPath>
bool AddPath(const TPath &pg, PolyType PolyTyp, bool Closed)
{
#ifdef use_lines
  if (!Closed && PolyTyp == ptClip)
    throw clipperException("AddPath: Open paths must be subject.");
#else
  if (!Closed)
    throw clipperException("AddPath: Open paths have been disabled.");
#endif

  int highI = (int)pg.size() -1;
  if (Closed) while (highI > 0 && (pg[highI] == pg[0])) --highI;
  while (highI > 0 && (pg[highI] == pg[highI -1])) --highI;
  if ((Closed && highI < 2) || (!Closed && highI < 1)) return false;

  //create a new edge array ...
  TEdge *edges = new TEdge [highI +1];

  bool IsFlat = true;
  //1. Basic (first) edge initialization ...
  try
  {
    edges[1].Curr = pg[1];
    RangeTest(pg[0], m_UseFullRange);
    RangeTest(pg[highI], m_UseFullRange);
    InitEdge(&edges[0], &edges[1], &edges[highI], pg[0]);
    InitEdge(&edges[highI], &edges[0], &edges[highI-1], pg[highI]);
    for (int i = highI - 1; i >= 1; --i)
    {
      RangeTest(pg[i], m_UseFullRange);
      InitEdge(&edges[i], &edges[i+1], &edges[i-1], pg[i]);
    }
  }
  catch(...)
  {
    delete [] edges;
    throw; //range test fails
  }
  TEdge *eStart = &edges[0];

  //2. Remove duplicate vertices, and (when closed) collinear edges ...
  TEdge *E = eStart, *eLoopStop = eStart;
  for (;;)
  {
    //nb: allows matching start and end points when not Closed ...
    if (E->Curr == E->Next->Curr && (Closed || E->Next != eStart))
    {
      if (E == E->Next) break;
      if (E == eStart) eStart = E->Next;
      E = RemoveEdge(E);
      eLoopStop = E;
      continue;
    }
    if (E->Prev == E->Next) 
      break; //only two vertices
    else if (Closed &&
      SlopesEqual(E->Prev->Curr, E->Curr, E->Next->Curr, m_UseFullRange) && 
      (!m_PreserveCollinear ||
      !Pt2IsBetweenPt1AndPt3(E->Prev->Curr, E->Curr, E->Next->Curr)))
    {
      //Collinear edges are allowed for open paths but in closed paths
      //the default is to merge adjacent collinear edges into a single edge.
      //However, if the PreserveCollinear property is enabled, only overlapping
      //collinear edges (ie spikes) will be removed from closed paths.
      if (E == eStart) eStart = E->Next;
      E = RemoveEdge(E);
      E = E->Prev;
      eLoopStop = E;
      continue;
    }
    E = E->Next;
    if ((E == eLoopStop) || (!Closed && E->Next == eStart)) break;
  }

  if ((!Closed && (E == E->Next)) || (Closed && (E->Prev == E->Next)))
  {
    delete [] edges;
    return false;
  }

  if (!Closed)
  { 
    m_HasOpenPaths = true;
    eStart->Prev->OutIdx = Skip;
  }

  //3. Do second stage of edge initialization ...
  E = eStart;
  do
  {
    InitEdge2(*E, PolyTyp);
    E = E->Next;
    if (IsFlat && E->Curr.Y != eStart->Curr.Y) IsFlat = false;
  }
  while (E != eStart);

  //4. Finally, add edge bounds to LocalMinima list ...

  //Totally flat paths must be handled differently when adding them
  //to LocalMinima list to avoid endless loops etc ...
  if (IsFlat) 
  {
    if (Closed) 
    {
      delete [] edges;
      return false;
    }
    E->Prev->OutIdx = Skip;
    if (E->Prev->Bot.X < E->Prev->Top.X) ReverseHorizontal(*E->Prev);
    MinimaList::value_type locMin;
    locMin.Y = E->Bot.Y;
    locMin.LeftBound = 0;
    locMin.RightBound = E;
    locMin.RightBound->Side = esRight;
    locMin.RightBound->WindDelta = 0;
    while (E->Next->OutIdx != Skip)
    {
      E->NextInLML = E->Next;
      if (E->Bot.X != E->Prev->Top.X) ReverseHorizontal(*E);
      E = E->Next;
    }
    m_MinimaList.push_back(locMin);
    m_edges.push_back(edges);
	  return true;
  }

  m_edges.push_back(edges);
  bool leftBoundIsForward;
  TEdge* EMin = 0;

  //workaround to avoid an endless loop in the while loop below when
  //open paths have matching start and end points ...
  if (E->Prev->Bot == E->Prev->Top) E = E->Next;

  for (;;)
  {
    E = FindNextLocMin(E);
    if (E == EMin) break;
    else if (!EMin) EMin = E;

    //E and E.Prev now share a local minima (left aligned if horizontal).
    //Compare their slopes to find which starts which bound ...
    MinimaList::value_type locMin;
    locMin.Y = E->Bot.Y;
    if (E->Dx < E->Prev->Dx) 
    {
      locMin.LeftBound = E->Prev;
      locMin.RightBound = E;
      leftBoundIsForward = false; //Q.nextInLML = Q.prev
    } else
    {
      locMin.LeftBound = E;
      locMin.RightBound = E->Prev;
      leftBoundIsForward = true; //Q.nextInLML = Q.next
    }
    locMin.LeftBound->Side = esLeft;
    locMin.RightBound->Side = esRight;

    if (!Closed) locMin.LeftBound->WindDelta = 0;
    else if (locMin.LeftBound->Next == locMin.RightBound)
      locMin.LeftBound->WindDelta = -1;
    else locMin.LeftBound->WindDelta = 1;
    locMin.RightBound->WindDelta = -locMin.LeftBound->WindDelta;

    E = ProcessBound(locMin.LeftBound, leftBoundIsForward);
    if (E->OutIdx == Skip) E = ProcessBound(E, leftBoundIsForward);

    TEdge* E2 = ProcessBound(locMin.RightBound, !leftBoundIsForward);
    if (E2->OutIdx == Skip) E2 = ProcessBound(E2, !leftBoundIsForward);

    if (locMin.LeftBound->OutIdx == Skip)
      locMin.LeftBound = 0;
    else if (locMin.RightBound->OutIdx == Skip)
      locMin.RightBound = 0;
    m_MinimaList.push_back(locMin);
    if (!leftBoundIsForward) E = E2;
  }
  return true;
}
  //bool AddPaths(const Paths &ppg, PolyType PolyTyp, bool Closed);
  template<typename TPaths>
  bool AddPaths(const TPaths &ppg, PolyType PolyTyp, bool Closed)
  {
    bool result = false;
    for (TPaths::size_type i = 0; i < ppg.size(); ++i)
	  if (AddPath(ppg[i], PolyTyp, Closed)) result = true;
    return result;
  }
  virtual void Clear();
  IntRect GetBounds();
  bool PreserveCollinear() {return m_PreserveCollinear;};
  void PreserveCollinear(bool value) {m_PreserveCollinear = value;};
protected:
  void DisposeLocalMinimaList();
  TEdge* AddBoundsToLML(TEdge *e, bool IsClosed);
  void PopLocalMinima();
  virtual void Reset();
  TEdge* ProcessBound(TEdge* E, bool IsClockwise);
  void DoMinimaLML(TEdge* E1, TEdge* E2, bool IsClosed);
  TEdge* DescendToMin(TEdge *&E);
  void AscendToMax(TEdge *&E, bool Appending, bool IsClosed);

  typedef std::vector<LocalMinimum> MinimaList;
  MinimaList::iterator m_CurrentLM;
  MinimaList           m_MinimaList;

  bool              m_UseFullRange;
  EdgeList          m_edges;
  bool             m_PreserveCollinear;
  bool             m_HasOpenPaths;
};
//------------------------------------------------------------------------------

class Clipper : public virtual ClipperBase
{
public:
  Clipper(int initOptions = 0);
  ~Clipper();
  bool Execute(ClipType clipType,
    Paths &solution,
    PolyFillType subjFillType = pftEvenOdd,
    PolyFillType clipFillType = pftEvenOdd);
  bool Execute(ClipType clipType,
    PolyTree &polytree,
    PolyFillType subjFillType = pftEvenOdd,
    PolyFillType clipFillType = pftEvenOdd);
  bool ReverseSolution() {return m_ReverseOutput;};
  void ReverseSolution(bool value) {m_ReverseOutput = value;};
  bool StrictlySimple() {return m_StrictSimple;};
  void StrictlySimple(bool value) {m_StrictSimple = value;};
  //set the callback function for z value filling on intersections (otherwise Z is 0)
#ifdef use_xyz
  void ZFillFunction(ZFillCallback zFillFunc);
#endif
protected:
  void Reset();
  virtual bool ExecuteInternal();
private:
  PolyOutList       m_PolyOuts;
  JoinList          m_Joins;
  JoinList          m_GhostJoins;
  IntersectList     m_IntersectList;
  ClipType          m_ClipType;
  typedef std::priority_queue<cInt> ScanbeamList;
  ScanbeamList      m_Scanbeam;
  TEdge           *m_ActiveEdges;
  TEdge           *m_SortedEdges;
  bool             m_ExecuteLocked;
  PolyFillType     m_ClipFillType;
  PolyFillType     m_SubjFillType;
  bool             m_ReverseOutput;
  bool             m_UsingPolyTree; 
  bool             m_StrictSimple;
#ifdef use_xyz
  ZFillCallback   m_ZFill; //custom callback 
#endif
  void SetWindingCount(TEdge& edge);
  bool IsEvenOddFillType(const TEdge& edge) const;
  bool IsEvenOddAltFillType(const TEdge& edge) const;
  void InsertScanbeam(const cInt Y);
  cInt PopScanbeam();
  void InsertLocalMinimaIntoAEL(const cInt botY);
  void InsertEdgeIntoAEL(TEdge *edge, TEdge* startEdge);
  void AddEdgeToSEL(TEdge *edge);
  void CopyAELToSEL();
  void DeleteFromSEL(TEdge *e);
  void DeleteFromAEL(TEdge *e);
  void UpdateEdgeIntoAEL(TEdge *&e);
  void SwapPositionsInSEL(TEdge *edge1, TEdge *edge2);
  bool IsContributing(const TEdge& edge) const;
  bool IsTopHorz(const cInt XPos);
  void SwapPositionsInAEL(TEdge *edge1, TEdge *edge2);
  void DoMaxima(TEdge *e);
  void ProcessHorizontals(bool IsTopOfScanbeam);
  void ProcessHorizontal(TEdge *horzEdge, bool isTopOfScanbeam);
  void AddLocalMaxPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  OutPt* AddLocalMinPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  OutRec* GetOutRec(int idx);
  void AppendPolygon(TEdge *e1, TEdge *e2);
  void IntersectEdges(TEdge *e1, TEdge *e2, IntPoint &pt);
  OutRec* CreateOutRec();
  OutPt* AddOutPt(TEdge *e, const IntPoint &pt);
  void DisposeAllOutRecs();
  void DisposeOutRec(PolyOutList::size_type index);
  bool ProcessIntersections(const cInt topY);
  void BuildIntersectList(const cInt topY);
  void ProcessIntersectList();
  void ProcessEdgesAtTopOfScanbeam(const cInt topY);
  void BuildResult(Paths& polys);
  void BuildResult2(PolyTree& polytree);
  void SetHoleState(TEdge *e, OutRec *outrec);
  void DisposeIntersectNodes();
  bool FixupIntersectionOrder();
  void FixupOutPolygon(OutRec &outrec);
  bool IsHole(TEdge *e);
  bool FindOwnerFromSplitRecs(OutRec &outRec, OutRec *&currOrfl);
  void FixHoleLinkage(OutRec &outrec);
  void AddJoin(OutPt *op1, OutPt *op2, const IntPoint offPt);
  void ClearJoins();
  void ClearGhostJoins();
  void AddGhostJoin(OutPt *op, const IntPoint offPt);
  bool JoinPoints(Join *j, OutRec* outRec1, OutRec* outRec2);
  void JoinCommonEdges();
  void DoSimplePolygons();
  void FixupFirstLefts1(OutRec* OldOutRec, OutRec* NewOutRec);
  void FixupFirstLefts2(OutRec* OldOutRec, OutRec* NewOutRec);
#ifdef use_xyz
  void SetZ(IntPoint& pt, TEdge& e1, TEdge& e2);
#endif
};
//------------------------------------------------------------------------------

class ClipperOffset 
{
public:
  ClipperOffset(double miterLimit = 2.0, double roundPrecision = 0.25);
  ~ClipperOffset();
  void AddPath(const Path& path, JoinType joinType, EndType endType);
  void AddPaths(const Paths& paths, JoinType joinType, EndType endType);
  void Execute(Paths& solution, double delta);
  void Execute(PolyTree& solution, double delta);
  void Clear();
  double MiterLimit;
  double ArcTolerance;
private:
  Paths m_destPolys;
  Path m_srcPoly;
  Path m_destPoly;
  std::vector<DoublePoint> m_normals;
  double m_delta, m_sinA, m_sin, m_cos;
  double m_miterLim, m_StepsPerRad;
  IntPoint m_lowest;
  PolyNode m_polyNodes;

  void FixOrientations();
  void DoOffset(double delta);
  void OffsetPoint(int j, int& k, JoinType jointype);
  void DoSquare(int j, int k);
  void DoMiter(int j, int k, double r);
  void DoRound(int j, int k);
};
//------------------------------------------------------------------------------

class clipperException : public std::exception
{
  public:
    clipperException(const char* description): m_descr(description) {}
    virtual ~clipperException() throw() {}
    virtual const char* what() const throw() {return m_descr.c_str();}
  private:
    std::string m_descr;
};
//------------------------------------------------------------------------------

} //ClipperLib namespace

#endif //clipper_hpp


