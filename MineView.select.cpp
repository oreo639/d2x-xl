// MineView.select.cpp: element selection UI functions


#include "stdafx.h"
#include "winuser.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"

#include "PaletteManager.h"
#include "textures.h"
#include "global.h"
#include "FileManager.h"

#include <math.h>
#include <time.h>

//-----------------------------------------------------------------------------

static bool PointIsInTriangle2D (CDoubleVector& p, CDoubleVector t [])
{
CDoubleVector v0 = t [2];
v0 -= t [0];
CDoubleVector v1 = t [1];
v1 -= t [0];
CDoubleVector v2 = p;
v2 -= t [0];
double dot00 = Dot (v0, v0);
double dot11 = Dot (v1, v1);
double dot01 = Dot (v0, v1);
double dot02 = Dot (v0, v2);
double dot12 = Dot (v1, v2);
double invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
return (u >= 0.0) && (v >= 0.0) && (u + v <= 1.0);
}

//-----------------------------------------------------------------------------

void CMineView::SelectCurrentObject (long xMouse, long yMouse) 
{
	CGameObject*	objP, tempObj;
	short				nClosestObj;
	short				i;
	double			radius, closestRadius;

// default to object 0 but set radius very large
nClosestObj = 0;
closestRadius = 1.0e30;

// if there is a secret exit, then enable it in search
int enable_secret = false;
if (DLE.IsD2File ())
	for(i = 0; i < (short) triggerManager.WallTriggerCount ();i++)
		if (triggerManager.Trigger (i)->Type () == TT_SECRET_EXIT) {
			enable_secret = true;
			break;
			}

for (i = 0; i < objectManager.Count (); i++) {
	BOOL drawable = false;
	// define temp object type and position for secret object selection
	if (i == objectManager.Count () && DLE.IsD2File () && enable_secret) {
		objP = &tempObj;
		objP->Type () = OBJ_PLAYER;
		// define objP->position
		CVertex center;
		objP->Position () = segmentManager.CalcCenter (center, (short) objectManager.SecretSegment ());
		}
	else
		objP = objectManager.Object (i);
#if 0
	switch(objP->Type ()) {
		case OBJ_WEAPON:
			if (ViewObject (eViewObjectsPowerups | eViewObjectsWeapons)) {
				drawable = true;
				}
		case OBJ_POWERUP:
			if (ViewObject (powerupTypeTable [objP->Id ()])) {
				drawable = true;
				}
			break;
		default:
			if(ViewObject (1<<objP->Type ()))
				drawable = true;
		}
	if (drawable) 
#else
	if (ViewObject (objP))
#endif
		{
		// translate object's position to screen coordinates
		CVertex v = objP->Position ();
		Renderer ().BeginRender ();
		v.Transform (ViewMatrix ());
		v.Project (ViewMatrix ());
		Renderer ().EndRender ();
		// calculate radius^2 (don't bother to take square root)
		double dx = (double) v.m_screen.x - (double) xMouse;
		double dy = (double) v.m_screen.y - (double) yMouse;
		radius = dx * dx + dy * dy;
	// check to see if this object is closer than the closest so far
		if (radius < closestRadius) {
			nClosestObj = i;
			closestRadius = radius;
			}
		}
	}

// unhighlight current object and select next object
i = current->m_nObject;
RefreshObject(i, nClosestObj);
}

//-----------------------------------------------------------------------------
// Find the nearest textured segment side the user had clicked on.
// First try to read the segment and side secondary render target from the renderer
// If that fails, cast a ray through the mouse position from the near to the 
// far plane and find the nearest textured segment side intersected by the ray
// Disabled because the new side and segment selection method is more flexible
// when it comes to selecting open (untextured) sides.

short CMineView::FindSelectedTexturedSide (long xMouse, long yMouse, short& nSide)
{
if (!m_bEnableQuickSelection)
	return -1;
if (((m_viewOption != eViewTextured) && (m_viewOption != eViewTexturedWireFrame)))
	return -1;

short nSegment;
int nResult = Renderer ().GetSideKey (xMouse, yMouse, nSegment, nSide);
if (nResult == 1)
	return nSegment;
if (nResult == 0)
	return -1;

CVertex p1, p2;
p2.m_proj.v.x = xMouse;
p2.m_proj.v.y = yMouse;
p1.m_proj.v.z = -1.0;
p2.m_proj.v.z = 1.0;
CPoint viewCenter (0, 0);
Renderer ().BeginRender ();
Renderer ().ViewMatrix ()->Unproject (p2, p2.m_proj);
Renderer ().EndRender ();
p1.Clear ();
CDoubleVector mouseDir = p2;
mouseDir.Normalize ();

double minDist = 1e30;
short nMinSeg = -1;
short nMinSide = -1;
short nSegments = segmentManager.Count ();
Renderer ().Project ();
segmentManager.ComputeNormals (true, true);

#pragma omp parallel for
for (short nSegment = 0; nSegment < nSegments; nSegment++) {
	CSegment* segP = segmentManager.Segment (nSegment);
	CSide* sideP = segP->Side (0);
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		double d;
		if (sideP->Shape () > SIDE_SHAPE_TRIANGLE)
			continue;
		if (!sideP->IsVisible ())
			continue;
		if ((d = Dot (mouseDir, sideP->m_vNormal [0])) > 0.0)
			continue; // looking at the back of the side
		d = sideP->LineHitsFace (&p1, &p2, segP->m_info.vertexIds, minDist, true);
		if (d < 0.0)
			continue;
#pragma omp critical
			{
			if (minDist > d) {
				minDist = d;
				nMinSeg = nSegment;
				nMinSide = nSide;
				}
			}
		}
	}
nSide = nMinSide;
return nMinSeg;
}

//-----------------------------------------------------------------------------

bool CMineView::SelectCurrentSide (long xMouse, long yMouse, int bAdd) 
{
if (bAdd < 0)
	vertexManager.UnmarkAll ();

	short nSide, nSegment = FindSelectedTexturedSide (xMouse, yMouse, nSide);

if (0 <= nSegment) 
	current->Setup (nSegment, nSide);
else 
if ((nearest->m_nSegment >= 0) && (nearest->m_nSide >= 0)) {
	current->m_nSegment = nearest->m_nSegment;
	current->m_nSide = nearest->m_nSide;
	}
else
	return false;
if (Perspective () && bAdd)
	current->Segment ()->MarkVertices (MARKED_MASK, current->m_nSide);
DLE.ToolView ()->Refresh ();
Refresh ();
return true;
}

//-----------------------------------------------------------------------------

bool CMineView::SelectCurrentSegment (long xMouse, long yMouse, int bAdd) 
{
return SelectCurrentSide (xMouse, yMouse, bAdd);
}

//-----------------------------------------------------------------------------

bool CMineView::SelectCurrentLine (long xMouse, long yMouse, int bAdd) 
{
if (bAdd < 0)
	vertexManager.UnmarkAll ();

if (nearest->m_nEdge < 0)
	return false;

ushort nVertices [2] = {nearest->Segment ()->VertexId (nearest->m_nSide, nearest->m_nEdge), 
								nearest->Segment ()->VertexId (nearest->m_nSide, nearest->m_nEdge + 1)};
if (bAdd && (nVertices [0] <= MAX_VERTEX) && (nVertices [1] <= MAX_VERTEX)) {
	vertexManager [nVertices [0]].Mark ();
	vertexManager [nVertices [1]].Mark ();
	current->m_nSegment = nearest->m_nSegment;
	current->m_nSide = nearest->m_nSide;
	current->m_nEdge = nearest->m_nEdge;
	current->m_nPoint = nearest->m_nEdge;
	}
DLE.ToolView ()->Refresh ();
Refresh ();
return true;
}

//-----------------------------------------------------------------------------

int CMineView::FindNearestVertex (long xMouse, long yMouse)
{
	CDoubleVector screen, clickPos (m_clickPos.x, m_clickPos.y, 0.0);
	CVertex* vertexP = vertexManager.Vertex (0);
	double dMin = 1e30;
	int xMax = ViewWidth (), yMax = ViewHeight ();
	int nVertex = -1;

for (int i = vertexManager.Count (); i; i--, vertexP++) {
#if 0
	if (!vertexP->InRange (xMax, yMax, m_nRenderer))
		continue;
	double d = Distance (clickPos, vertexP->m_proj);
#else
	CVertex v = *vertexP;
	if (!v.InRange (xMax, yMax, m_nRenderer))
		continue;
	v.m_proj.v.z = 0.0;
	double d = Distance (clickPos, v.m_proj);
#endif
	if (dMin > d) {
		dMin = d;
		nVertex = i;
		}
	}
return (nVertex < 0) ? -1 : vertexManager.Count () - nVertex;
}

//-----------------------------------------------------------------------------

bool CMineView::SelectCurrentPoint (long xMouse, long yMouse, int bAdd) 
{
if (bAdd < 0)
	vertexManager.UnmarkAll ();
int nVertex = FindNearestVertex (xMouse, yMouse);
if (bAdd && (nVertex >= 0))
	vertexManager [nVertex].Mark ();
DLE.ToolView ()->Refresh ();
Refresh ();
return true;
}

//-----------------------------------------------------------------------------

bool CMineView::SelectCurrentElement (long xMouse, long yMouse, int bAdd) 
{
switch (theMine->SelectMode ()) {
	case POINT_MODE:
	case BLOCK_MODE:
		return SelectCurrentPoint (xMouse, yMouse, bAdd);

	case LINE_MODE:
		return SelectCurrentLine (xMouse, yMouse, bAdd);

	case SIDE_MODE:
		return SelectCurrentSide (xMouse, yMouse, bAdd);

	case SEGMENT_MODE:
		return SelectCurrentSegment (xMouse, yMouse, bAdd);

	case OBJECT_MODE:
		SelectCurrentObject (xMouse, yMouse);
		return true;
	}
return false;
}

//-----------------------------------------------------------------------------

void CMineView::NextPoint (int dir) 
{
if (current->Side ()->Shape () > SIDE_SHAPE_TRIANGLE)
	NextSide (dir);
else {
	Wrap (current->m_nPoint, dir, 0, current->Side ()->VertexCount () - 1);
	current->m_nEdge = current->m_nPoint;
	}
Refresh ();
}

//-----------------------------------------------------------------------------

void CMineView::PrevPoint (void)
{
NextPoint (-1);
}

//-----------------------------------------------------------------------------

void CMineView::NextSide (int dir) 
{
short nSide = current->m_nSide; 
do {
	Wrap (current->m_nSide, dir, 0, 5);
	} while ((nSide != current->m_nSide) && (current->Side ()->Shape () > SIDE_SHAPE_TRIANGLE));
Refresh (true);
}

//-----------------------------------------------------------------------------

void CMineView::PrevSide () 
{
NextSide (-1);
}

//-----------------------------------------------------------------------------

void CMineView::NextLine (int dir) 
{
if (current->Side ()->Shape () > SIDE_SHAPE_TRIANGLE)
	NextSide (dir);
else {
	Wrap (current->m_nEdge, dir, 0, current->Side ()->VertexCount () - 1);
	current->m_nPoint = current->m_nEdge;
	}
Refresh ();
}

//-----------------------------------------------------------------------------

void CMineView::PrevLine (void) 
{
NextLine (-1);
}

//-----------------------------------------------------------------------------

void CMineView::NextSegment (int dir) 
{
if (segmentManager.Count () <= 0)
	return;

if (0) {//!ViewOption (eViewWireFrameSparse)) {
	DrawHighlight (1);
	Wrap (current->m_nSegment, dir, 0, segmentManager.Count () - 1);
	Refresh (true);
	DrawHighlight (0);
	}
else {
	Wrap (current->m_nSegment, dir, 0, segmentManager.Count () - 1);
	Refresh (true);
	}
}

//-----------------------------------------------------------------------------

void CMineView::PrevSegment (void) 
{
NextSegment (-1);
}

//-----------------------------------------------------------------------------
// MENU - Forward_Cube
//
// ACTION - If child exists, this routine sets current_segment to child segP
//
// Changes - Smart side selection added (v0.8)
//         Smart side selection done before moving (instead of after) (v0.9)
//-----------------------------------------------------------------------------

void CMineView::SegmentForward (int dir) 
{
	CSegment *segP,*childSegP;
	short nChild, nSide;
	bool bFwd = (dir == 1);

DrawHighlight (1);
segP = segmentManager.Segment (current->m_nSegment);
nChild = segP->ChildId (bFwd ? current->m_nSide: oppSideTable [current->m_nSide]);
if (nChild <= -1) {
	// first try to find a non backwards route
	for (nSide = 0; nSide < 6; nSide++) {
		if (segP->ChildId (nSide) != m_lastSegment && segP->ChildId (nSide) > -1) {
			nChild = segP->ChildId (nSide);
			current->m_nSide =  bFwd ? nSide: oppSideTable [nSide];
			break;
			}
		}
	// then settle for any way out
	if (nSide == 6) {
		for (nSide = 0; nSide < 6; nSide++) {
			if (segP->ChildId (nSide) > -1) {
				nChild = segP->ChildId (nSide);
				current->m_nSide = bFwd ? nSide: oppSideTable [nSide];
				break;
				}
			}			
		}
	}
if (nChild > -1) {
	childSegP = segmentManager.Segment (nChild);
// try to select side which is in same direction as current side
	for (nSide = 0; nSide < 6; nSide++) {
		if (childSegP->ChildId (nSide) == current->m_nSegment) {
			current->m_nSide =  bFwd ? oppSideTable [nSide]: nSide;
			break;
			}
		}
	m_lastSegment = current->m_nSegment;
	if (0) {//!ViewOption (eViewWireFrameSparse)) {
		// DrawHighlight (1);
		current->m_nSegment = nChild;
		// DrawHighlight (0);
		} 
	else {
		current->m_nSegment = nChild;
		Refresh (true);
		}
	}
DrawHighlight (0);
}

//-----------------------------------------------------------------------------

void CMineView::SegmentBackwards () 
{
SegmentForward (-1);
}

//-----------------------------------------------------------------------------

void CMineView::SelectOtherSegment () 
{
Swap (current, other);
//current = &selections [!current->Index ()];
//other = &selections [!current->Index ()];
Refresh (true);
DLE.ToolView ()->SegmentTool ()->Refresh ();
}

//-----------------------------------------------------------------------------

bool CMineView::SelectOtherSide () 
{
CSideKey opp;

if (segmentManager.BackSide (opp) == null)
	return false;

*((CSideKey *) current) = opp;
Refresh (true);
DLE.ToolView ()->SegmentTool ()->Refresh ();
return true;
}

//-----------------------------------------------------------------------------

void CMineView::NextObject (int dir) 
{
  short oldObject = current->m_nObject;
  short newObject = current->m_nObject;

if (objectManager.Count () > 1) {
	Wrap (newObject, dir, 0, (short) objectManager.Count () - 1);
	Refresh (true);
	}
RefreshObject (oldObject, newObject);
}

//-----------------------------------------------------------------------------

void CMineView::PrevObject() 
{
NextObject (-1);
}

//-----------------------------------------------------------------------------

void CMineView::NextSegmentElement (int dir)
{
switch (m_selectMode) {
	case eSelectPoint:
		NextPoint (dir);
		break;
	case eSelectLine:
		NextLine (dir);
		break;
	default:
		NextSide (dir);
		break;
	}
}

//------------------------------------------------------------------------------

void CMineView::PrevSegmentElement ()
{
NextSegmentElement (-1);
}

//------------------------------------------------------------------------------
//eof