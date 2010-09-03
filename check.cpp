// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "afxpriv.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "palette.h"
#include "textures.h"
#include "global.h"
#include "render.h"
#include "cfile.h"

#include <math.h>

typedef struct tBugPos {
	int	nSegment;
	int	nSide;
	int	nLine;
	int	nPoint;
	int	nChild;
	int	nWall;
	int	nTrigger;
	int	nObject;
} tBugPos;

                        /*--------------------------*/

void CDiagTool::ClearBugList ()
{
if (!(m_bInited && theMine))
	return;

	CListBox	*plb = LBBugs ();
	int h = plb->GetCount ();
	char szText [256];

tBugPos *pbp;
int i;
for (i = 0; i < h; i++) {
	plb->GetText (i, szText);
	if (!strchr (szText, '['))
		if (pbp = (tBugPos *) plb->GetItemDataPtr (i)) {
			delete pbp;
			plb->SetItemDataPtr (i, null);
			}
	}
plb->ResetContent ();
m_nErrors [0] =
m_nErrors [1] = 0;
m_statsWidth = 0;
}

                        /*--------------------------*/

bool CDiagTool::MarkSegment (short nSegment) 
{
if ((nSegment < 0) || (nSegment >= segmentManager.Count ()))
	return false;
segmentManager.GetSegment (nSegment)->m_info.wallFlags &= ~MARKED_MASK;
theMine->MarkSegment (nSegment);
return true;
}

                        /*--------------------------*/

void CDiagTool::OnShowBug (void)
{
CHECKMINE;

	bool bCurSeg;
	CWall *wallP;
	int nWall;

int i = LBBugs ()->GetCurSel ();
if ((i < 0) || (i >= LBBugs ()->GetCount ()))
	return;
tBugPos *pbp = (tBugPos *) LBBugs ()->GetItemDataPtr (i);
if (!pbp)
	return;
theMine->UnmarkAll ();
if (bCurSeg = MarkSegment (pbp->nSegment))
	current.m_nSegment = pbp->nSegment;
MarkSegment (pbp->nChild);
if ((pbp->nSide >= 0) && (pbp->nSide < MAX_SIDES_PER_SEGMENT))
	current.m_nSide = pbp->nSide;
if ((pbp->nLine >= 0) && (pbp->nLine < 4))
	current.m_nLine = pbp->nLine;
if ((pbp->nPoint >= 0) && (pbp->nPoint < 8))
	current.m_nPoint = pbp->nPoint;
if ((pbp->nWall >= 0) && (pbp->nWall < theMine->Info ().walls.count))
	nWall = pbp->nWall;
else if ((pbp->nTrigger >= 0) && (pbp->nTrigger < theMine->Info ().triggers.count))
	wallP = wallManager.FindByTrigger (pbp->nTrigger);
else
	wallP = null;
if ((wallP != null) && MarkSegment ((wallP->m_nSegment))
	if (bCurSeg) {
		other.m_nSegment = wallP->m_nSegment;
		other.m_nSide = wallP->m_nSide;
		}
	else {
		current.m_nSegment = wallP->m_nSegment;
		current.m_nSide = wallP->m_nSide;
		}
if ((pbp->nObject >= 0) && (pbp->nObject < theMine->Info ().objects.count))
	current.m_nObject = pbp->nObject;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CalcFlatnessRatio ()
//
// 1) calculates the average length lines 0 and 2
// 2) calculates the midpoints of lines 1 and 3
// 3) calculates the length between these two midpoints
// 4) calculates the ratio of the midpoint length over the line lengths
// 5) repeats this for lines 1 and 3 and midpoints 0 and 2
// 6) returns the minimum of the two ratios 
//------------------------------------------------------------------------

double CDiagTool::CalcFlatnessRatio (short nSegment, short nSide) 
{
  short		i;
  CVertex	midpoint1, midpoint2;
  double		length1, length2, ave_length, mid_length;
  double		ratio;
  CVertex	vert [4];
  // copy vertnums into an array
	CSegment*	segP = segmentManager.GetSegment (nSegment);

for (i = 0; i < 4; i++)
	vert [i] = *theMine->Vertices (segP->m_info.verts[sideVertTable[nSide][i]]);

length1 = CalcDistance (vert + 0, vert + 1, vert + 2);
length2 = CalcDistance (vert + 0, vert + 1, vert + 3);
ave_length = (length1 + length2) / 2;
midpoint1 = Average (vert [0], vert [1]);
midpoint2 = Average (vert [2], vert [3]);
mid_length = Distance (midpoint1, midpoint2);
ratio = mid_length / ave_length;

length1 = CalcDistance (vert + 1, vert + 2, vert + 3);
length2 = CalcDistance (vert + 1, vert + 2, vert + 0);
ave_length = (length1 + length2) / 2;
midpoint1 = Average (vert [1], vert [2]);
midpoint2 = Average (vert [3], vert [0]);
mid_length = Distance (midpoint1, midpoint2);
return min (ratio, mid_length / ave_length);
}

//--------------------------------------------------------------------------
// CalcDistance
//
// Action - calculates the distance from a line to a point
//--------------------------------------------------------------------------

double CDiagTool::CalcDistance (CVertex* v1, CVertex* v2, CVertex* v3)
{
// normalize all points to vector 1
CDoubleVector A = *v2 - *v1;
CDoubleVector B = *v3 - *v1;

// use formula from page 505 of "Calculase and Analytical Geometry" Fifth Addition
// by Tommas/Finney, Addison-Wesley Publishing Company, June 1981
//          B * A
// B2 = B - ----- A
//          A * A

double a2 = A ^ A;
double c = (a2 != 0) ? (B ^ A) / a2 : 0;
CDoubleVector C = B - (A * c);
return C.Mag ();
}

//--------------------------------------------------------------------------
// CalcAngle ()
//--------------------------------------------------------------------------

double CDiagTool::CalcAngle (short vert0,short vert1,short vert2,short vert3) 
{
  CDoubleVector line1,line2,line3,orthog;
  double ratio;
  double dot_product, magnitude1, magnitude2,angle;
  CVertex* v0 = theMine->Vertices (vert0);
  CVertex* v1 = theMine->Vertices (vert1);
  CVertex* v2 = theMine->Vertices (vert2);
  CVertex* v3 = theMine->Vertices (vert3);
      // define lines
line1 = *v1 - *v0;
line2 = *v2 - *v0;
line3 = *v3 - *v0;
// use cross product to calcluate orthogonal vector
orthog = -CrossProduct (line1, line2);
// use dot product to determine angle A dot B = |A|*|B| * cos (angle)
// therfore: angle = acos (A dot B / |A|*|B|)
dot_product = line3 ^ orthog;
magnitude1 = line3.Mag ();
magnitude2 = orthog.Mag ();
if (dot_product == 0 || magnitude1 == 0 || magnitude2 == 0)
	angle = (200.0 * M_PI)/180.0; 
else {
	ratio = dot_product/ (magnitude1*magnitude2);
	ratio = ( (double) ( (int) (ratio*1000.0))) / 1000.0; // bug int 9/21/96
	if (ratio < -1.0 || ratio > (double)1.0) 
		angle = (199.0 * M_PI)/180.0;
	else
		angle = acos (ratio);
	}
return fabs (angle);  // angle should be positive since acos returns 0 to PI but why not be sure
}

//--------------------------------------------------------------------------
//  CheckId (local subroutine)
//
//  ACTION - Checks the range of the ID number of an object of a particular
//           type.
//
//  RETURN - Returns TRUE if ID is out of range.  Otherwise, FALSE.
//--------------------------------------------------------------------------

int CDiagTool::CheckId (CGameObject *objP) 
{
	int type = objP->m_info.type;
	int id = objP->m_info.id;

	switch (type) {
	case OBJ_ROBOT: /* an evil enemy */
		if (id < 0 || id >= (DLE.IsD1File () ? ROBOT_IDS1 : ROBOT_IDS2)) {
			return 1;
		}
		break;

	case OBJ_HOSTAGE: /* a hostage you need to rescue */
#if 0
		if (id< 0 || id >1) {
			return 1;
		}
#endif
		break;

	case OBJ_PLAYER: /* the player on the console */
		if ((id < 0) || (id >= MAX_PLAYERS)) {
			return 1;
		}
		break;

	case OBJ_POWERUP: /* a powerup you can pick up */
		if (id< 0 || id > MAX_POWERUP_IDS) {
			return 1;
		}
		break;

	case OBJ_CNTRLCEN: /* the control center */
		if (DLE.IsD1File ()) {
			if (id >= 0 || id <= 25) {
				return 0;
			}
		} else {
			if (id >= 1 || id <= 6) {
				return 0;
			}
		}
		if (!m_bAutoFixBugs)
			return 1;
		objP->m_info.id = DLE.IsD1File () ? 1 : 2;
		return 2;
		break;

	case OBJ_COOP: /* a cooperative player object */
		if ((id < 0) || (id > 2)) {
			return 1;
		}
		break;

	case OBJ_WEAPON: // (d2 only)
		if (id != SMALLMINE_ID) {
			if (!m_bAutoFixBugs)
				return 1;
			objP->m_info.id = SMALLMINE_ID;
			return 2;
		}
	}
	return 0;
}

//--------------------------------------------------------------------------
// CDiagTool - WMTimer
//
// Action - Fills list box with data about the theMine->  This
//          routine is called once after the dialog is opened.
//
//--------------------------------------------------------------------------

void CDiagTool::OnCheckMine (void)
{
CHECKMINE;

UpdateData (TRUE);
ClearBugList ();
m_bCheckMsgs = true;
if (m_bAutoFixBugs) {
	undoManager.SetModified (true);
	undoManager.Lock ();
	}
  // set mode to BLOCK mode to make errors appear in red
DLE.MineView ()->SetSelectMode (BLOCK_MODE);

// now do actual checking
DLE.MainFrame ()->InitProgress (segmentManager.Count () * 3 + 
										  theMine->VertCount () +
										  theMine->Info ().walls.count * 2 +
										  theMine->Info ().triggers.count * 3 +
										  theMine->Info ().objects.count * 2 +
										  theMine->NumObjTriggers ());
if (!CheckBotGens ())
	if (!CheckEquipGens ())
		if (!CheckSegments ())
			if (!CheckSegTypes ())
				if (!CheckWalls ())
					if (!CheckTriggers ())
						if (!CheckObjects ())
							CheckVertices ();
DLE.MainFrame ()->Progress ().DestroyWindow ();
LBBugs ()->SetCurSel (0);
if (m_bAutoFixBugs)
	undoManager.Unlock ();
}

//--------------------------------------------------------------------------
// CDiagTool - UpdateStats ()
//
// Action  - Updates error and warning counts.
//
// Returns - TRUE if 1000 warnings or errors have occured 
//
// Parameters - error_msg = null terminated string w/ error message
//              error     = set to a 1 if this is an error
//              warning   = set to a 1 if this is a warning 
//--------------------------------------------------------------------------

void CDiagTool::UpdateStatsWidth (char* s)
{
	CClientDC dc(this);

CFont * f = LBBugs ()->GetFont();
dc.SelectObject(f);

CSize sz = dc.GetTextExtent (s, _tcslen (s));
sz.cx += 3 * ::GetSystemMetrics(SM_CXBORDER);
if (sz.cx > m_statsWidth) 
	LBBugs ()->SetHorizontalExtent (m_statsWidth = sz.cx);
}

//--------------------------------------------------------------------------

bool CDiagTool::UpdateStats (char *szError, int nErrorLevel, 
									  int nSegment, int nSide, int linenum, int pointnum, 
									  int childnum, int nWall, int nTrigger, int objnum)
{
if (!(szError && *szError))
	return false;
if (!(m_bShowWarnings || !strstr (szError, "WARNING:")))
	return true;
int h = AddMessage (szError, -1, true);
UpdateStatsWidth (szError);
if (h >= 0) {
	tBugPos *pbp = new tBugPos;
	if (!pbp)
		return false;
	pbp->nSegment = nSegment;
	pbp->nSide = nSide;
	pbp->nLine = linenum;
	pbp->nPoint = pointnum;
	pbp->nWall = nWall;
	pbp->nTrigger = nTrigger;
	pbp->nObject = objnum;
	pbp->nChild = childnum;
	LBBugs ()->SetItemDataPtr (h, (void *) pbp);
	}
*szError = '\0';
m_nErrors [nErrorLevel]++;
if (m_nErrors [0] < 1000 && m_nErrors [1] < 1000)
	return false;
LBBugs ()->AddString ("Maximum number of errors/warnings reached");
UpdateStatsWidth (szError);
return true;
}

//--------------------------------------------------------------------------

bool CDiagTool::CheckSegTypes (void) 
{
if ((theMine == null)) 
	return false;

	short	i, nBotGens = 0, nEquipGens = 0, nFuelCens = 0;
	CSegment	*segP = theMine->Segments (0);

for (i = segmentManager.Count (); i; i--, segP++)
	switch (segP->m_info.function) {
		case SEGMENT_FUNC_ROBOTMAKER:
			nBotGens++;
			break;
		case SEGMENT_FUNC_EQUIPMAKER:
			nEquipGens++;
			break;
		case SEGMENT_FUNC_FUELCEN:
			nFuelCens++;
			break;
		default:
			break;
	}
if (nBotGens != theMine->RobotMakerCount ()) {
	if (m_bAutoFixBugs) {
		sprintf_s (message, sizeof (message), "FIXED: Invalid robot maker count");
		theMine->RobotMakerCount () = nBotGens;
		}
	else
		sprintf_s (message, sizeof (message),"ERROR: Invalid robot maker count");
	if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, -1))
		return true;
	}
return false;
}

//--------------------------------------------------------------------------
// CDiagTool  LBBugs ()()
//
//  Checks for:
//    out of range data
//    valid cubes geometry
//--------------------------------------------------------------------------

bool CDiagTool::CheckSegments (void) 
{
if ((theMine == null)) 
	return false;

  short nSegment, nSide, nChild, nSide2, pointnum;
  short vert0, vert1, vert2, vert3;
  short i, j;
  double angle,flatness;
  short match[4];
  CSegment *segP = theMine->Segments (0);

  // check Segments ()
  //--------------------------------------------------------------
short sub_errors = m_nErrors [0];
short sub_warnings = m_nErrors [1];
LBBugs ()->AddString ("[Cubes]");

for (nSegment = 0; nSegment < segmentManager.Count (); nSegment++, segP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
// check geometry of segment
// 	Given that each point has 3 lines (called L1, L2, and L3),
//	and an orthogonal vector of L1 and L2 (called V1), the angle
//	between L3 and V1 must be less than PI/2.
//
	if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) {
		if ((segP->m_info.nMatCen >= theMine->Info ().botgen.count) || (theMine->BotGens (segP->m_info.nMatCen)->m_info.nSegment != nSegment)) {
	 		sprintf_s (message, sizeof (message), "%s: Segment has invalid type (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
			if (m_bAutoFixBugs)
				theMine->UndefineSegment (nSegment);
			}
		}
	if (segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) {
		if ((segP->m_info.nMatCen >= theMine->Info ().equipGen.count) || (theMine->EquipGens (segP->m_info.nMatCen)->m_info.nSegment != nSegment)) {
	 		sprintf_s (message, sizeof (message), "%s: Segment has invalid type (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
			if (m_bAutoFixBugs)
				theMine->UndefineSegment (nSegment);
			}
		}

	for (pointnum = 0; pointnum < 8; pointnum++) {
// define vert numbers
		vert0 = segP->m_info.verts[pointnum];
		vert1 = segP->m_info.verts[connectPointTable[pointnum][0]];
		vert2 = segP->m_info.verts[connectPointTable[pointnum][1]];
		vert3 = segP->m_info.verts[connectPointTable[pointnum][2]];
		angle = CalcAngle (vert0,vert1,vert2,vert3);
		angle = max (angle,CalcAngle (vert0,vert2,vert3,vert1));
		angle = max (angle,CalcAngle (vert0,vert3,vert1,vert2));
		if (angle > M_PI_2) {
			sprintf_s (message, sizeof (message), "WARNING: Illegal cube geometry (cube=%d,point=%d,angle=%d)",nSegment,pointnum, (int) ( (angle*180.0)/M_PI));
			if (UpdateStats (message, 0, nSegment, -1, -1, pointnum))
				return true;
			break; // from for loop
			}
		}
	if (pointnum == 8) { // angles must be ok from last test
// now test for flat sides
		for (nSide = 0; nSide < 6; nSide++) {
			flatness = CalcFlatnessRatio (nSegment,nSide);
			if (flatness < 0.80) {
				sprintf_s (message, sizeof (message),"ERROR: Illegal cube geometry (cube=%d,side=%d,flatness=%d%%)",nSegment,nSide, (int) (flatness*100));
				if (UpdateStats (message, 1, nSegment, nSide))
					return true;
				break; // from for loop
				}
			} 
		}

// Check length of each line.
#if 0
	for (linenum=0;linenum<12;linenum++) {
		length = theMine->CalcLength (theMine->Vertices (segP->m_info.verts[lineVertTable[linenum][0]]),
											  theMine->Vertices (segP->m_info.verts[lineVertTable[linenum][1]]));
		if (length < (double)F1_0) {
			sprintf_s (message, sizeof (message),"WARNING: Line length too short (cube=%d,line=%d)",nSegment,linenum);
			if (UpdateStats (message, 0, nSegment, -1, linenum))
				return true;
			}
		}
#endif
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		nChild = segmentManager.GetSegment (nSegment)->GetChild (nSide);
// check nChild range 
		if (nChild != -1 && nChild != -2) {
			if (nChild < -2) {
				sprintf_s (message, sizeof (message),"ERROR: Illegal nChild number %d (cube=%d,side=%d)",nChild,nSegment,nSide);
				if (UpdateStats (message, 1, nSegment, nSide))
					return true;
				}
			else if (nChild >= segmentManager.Count ()) {
				sprintf_s (message, sizeof (message),"ERROR: Child out of range %d (cube=%d,side=%d)",nChild,nSegment,nSide);
				if (UpdateStats (message, 1, nSegment, nSide)) 
					return true;
				}
			else {
			// make sure nChild segment has a nChild from this segment
			// and that it shares the same vertices as this segment
				for (nSide2 = 0; nSide2 < MAX_SIDES_PER_SEGMENT; nSide2++) {
					if (theMine->Segments (nChild)->GetChild (nSide2) == nSegment) 
						break;
					}					
				if (nSide2 < MAX_SIDES_PER_SEGMENT) {
					memset (match, 0, sizeof (match));
					for (i = 0; i < 4; i++)
						for (j = 0; j < 4; j++)
							if (segmentManager.GetSegment (nSegment)->m_info.verts [sideVertTable [nSide][i]] ==
								 theMine->Segments (nChild)->m_info.verts [sideVertTable [nSide2][j]])
								match[i]++;
					if (match[0]!=1 || match[1]!=1 || match[2]!=1 || match[3]!=1) {
						sprintf_s (message, sizeof (message),"WARNING:Child cube does not share points of cube (cube=%d,side=%d,nChild=%d)",nSegment,nSide,nChild);
						if (UpdateStats (message, 0, nSegment, -1, -1, -1, nChild))
							return true;
						}
					}
				else {
					sprintf_s (message, sizeof (message), "WARNING:Child cube does not connect to this cube (cube=%d,side=%d,nChild=%d)",nSegment,nSide,nChild);
					if (UpdateStats (message, 1, nSegment, -1, -1, -1, nChild))
						return false;
					}
				}
			}  
		}
	}
if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Cubes] (no errors)");
	}
return false;
}

//--------------------------------------------------------------------------

bool CDiagTool::CheckAndFixPlayer (int nMin, int nMax, int nObject, int* players)
{
if ((theMine == null)) 
	return false;

int id = theMine->Objects (nObject)->m_info.id;
if ((id < nMin) || (id > nMax))
	sprintf_s (message, sizeof (message), "WARNING: Invalid player id (Object %d)", id, nObject);
else if (players [id])
	sprintf_s (message, sizeof (message), "WARNING: Duplicate player #%d (found %d)", id, players [id]);
else {
	players [id]++;
	return false;
	}
if (m_bAutoFixBugs) {
	for (id = nMin; (id < nMax) && players [id]; id++)
		;
	theMine->Objects (nObject)->m_info.id = (id < nMax) ? id : nMax - 1;
	players [theMine->Objects (nObject)->m_info.id]++;
	}
return true;
}

//--------------------------------------------------------------------------
// CDiagTool - check_objects ()
//
// ACTION - Checks object's: segment number, type, id, position, container.
//          Makes sure the correct number of players coop-players, and
//          control centers (robots) are used.
//          Control center belongs to segment with special = SEGMENT_FUNC_CONTROLCEN
//--------------------------------------------------------------------------

bool CDiagTool::CheckObjects () 
{
if ((theMine == null)) 
	return false;

	int				h, nObject, type, id, count, players [16 + MAX_COOP_PLAYERS], nSegment, flags, corner, nPlayers [2], bFix;
	CVertex			center;
	double			radius, max_radius, object_radius;
	CGameObject*	objP = theMine->Objects (0);
	CGameObject*	pPlayer = null;
	int				objCount = theMine->Info ().objects.count;
	CSegment*		segP;

short sub_errors = m_nErrors [0];
short sub_warnings = m_nErrors [1];
LBBugs ()->AddString ("[Objects]");
for (nObject = 0;nObject < objCount ; nObject++, objP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	// check segment range
	nSegment = objP->m_info.nSegment;
	if (nSegment < 0 || nSegment >= segmentManager.Count ()) {
		if (m_bAutoFixBugs) {
			objP->m_info.nSegment = nSegment = 0;
			sprintf_s (message, sizeof (message),"FIXED: Bad segment number (object=%d,nSegment=%d)",nObject,nSegment);
			}
		else
			sprintf_s (message, sizeof (message),"ERROR: Bad segment number (object=%d,nSegment=%d)",nObject,nSegment);
		if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, nObject))
			return true;
		}

	if (nSegment < 0 || nSegment >= segmentManager.Count ()) {
		if (m_bAutoFixBugs) {
			objP->m_info.nSegment = 0;
			sprintf_s (message, sizeof (message),"FIXED: Bad segment number (object=%d,nSegment=%d)",nObject,nSegment);
			}
		else
			sprintf_s (message, sizeof (message),"ERROR: Bad segment number (object=%d,nSegment=%d)",nObject,nSegment);
		if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, nObject))
			return true;
		}

	segP = segmentManager.GetSegment (nSegment);
    // make sure object is within its cube
    // find center of segment then find maximum distance
	// of corner to center.  Calculate Objects () distance
    // from center and make sure it is less than max corner.
    center.Clear ();
    for (corner = 0; corner < 8; corner++) {
      center += *theMine->Vertices (segP->m_info.verts[corner]);
    }
    center /= 8.0;
    max_radius = 0;
    for (corner = 0; corner < 8; corner++) {
		 radius = Distance (*theMine->Vertices (segP->m_info.verts[corner]), center);
		 max_radius = max (max_radius,radius);
		 }
	object_radius = Distance (objP->m_location.pos, center);
   if ((object_radius > max_radius) && (objP->m_info.type != OBJ_EFFECT)) {
      sprintf_s (message, sizeof (message),"ERROR: Object is outside of cube (object=%d,cube=%d)",nObject,nSegment);
      if (UpdateStats (message, 1, nSegment, -1, -1, -1, -1, -1, -1, nObject))
			return true;
    }

    // check for non-zero flags (I don't know what these flags are for)
   flags = objP->m_info.flags;
	if (flags != 0) {
		if (m_bAutoFixBugs) {
			objP->m_info.flags = 0;
			sprintf_s (message, sizeof (message),"FIXED: Flags for object non-zero (object=%d,flags=%d)",nObject,flags);
			}
		else
			sprintf_s (message, sizeof (message),"ERROR: Flags for object non-zero (object=%d,flags=%d)",nObject,flags);
      if (UpdateStats (message, 1, nSegment, -1, -1, -1, -1, -1, -1, nObject)) 
			return true;
    }

    // check type range
	 if ((objP->m_info.id < 0) || (objP->m_info.id > 255)) {
		 if (m_bAutoFixBugs) {
			sprintf_s (message, sizeof (message),"FIXED: Illegal object id (object=%d,id =%d)",nObject, objP->m_info.id);
			objP->m_info.id = 0;
			}
		 else
			sprintf_s (message, sizeof (message),"WARNING: Illegal object id (object=%d,id =%d)",nObject, objP->m_info.id);
		}
	type = objP->m_info.type;
    switch (type) {
	  case OBJ_PLAYER:
		  if (!pPlayer)
			  pPlayer = objP;
			if (objP->m_info.id >= MAX_PLAYERS) {
				if (m_bAutoFixBugs) {
					sprintf_s (message, sizeof (message),"FIXED: Illegal player id (object=%d,id =%d)",nObject, objP->m_info.id);
				objP->m_info.id = MAX_PLAYERS - 1;
				}
			else
				sprintf_s (message, sizeof (message),"WARNING: Illegal player id (object=%d,id =%d)",nObject, objP->m_info.id);
			}
	  case OBJ_COOP:
		  if (objP->m_info.id > 2) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),"FIXED: Illegal coop player id (object=%d,id =%d)",nObject, objP->m_info.id);
				objP->m_info.id = 2;
				}
			else
				sprintf_s (message, sizeof (message),"WARNING: Illegal coop player id (object=%d,id =%d)",nObject, objP->m_info.id);
			}
			break;
	  case OBJ_EFFECT:
		  if (objP->m_info.id > SOUND_ID) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),"FIXED: effect id (object=%d,id =%d)",nObject, objP->m_info.id);
				objP->m_info.id = 2;
				}
			else
				sprintf_s (message, sizeof (message),"WARNING: Illegal effect id (object=%d,id =%d)",nObject, objP->m_info.id);
			}
			break;
	  case OBJ_ROBOT:
	  case OBJ_HOSTAGE:
	  case OBJ_POWERUP:
     case OBJ_CNTRLCEN:
     case OBJ_WEAPON:
		  break;
	  case OBJ_CAMBOT:
	  case OBJ_SMOKE:
	  case OBJ_MONSTERBALL:
	  case OBJ_EXPLOSION:
			if (DLE.IsD2File ()) 
				break;
	  default:
		 if (m_bAutoFixBugs) {
			 theMine->DeleteObject (nObject);
			sprintf_s (message, sizeof (message),"FIXED: Illegal object type (object=%d,type=%d)",nObject,type);
			}
		 else
			sprintf_s (message, sizeof (message),"WARNING: Illegal object type (object=%d,type=%d)",nObject,type);
		if (UpdateStats (message,0, nSegment, -1, -1, -1, -1, -1, -1, nObject))
			return true;
    }
    id = objP->m_info.id;

    // check id range
    if (h = CheckId (objP)) {
		 if (h == 2)
	      sprintf_s (message, sizeof (message),"FIXED: Illegal object id (object=%d,id=%d)",nObject,id);
		else
	      sprintf_s (message, sizeof (message),"WARNING: Illegal object id (object=%d,id=%d)",nObject,id);
      if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, nObject)) 
			return true;
    }

	// check contains count range
    count = objP->m_info.contents.count;
	if (count < -1) {
		if (m_bAutoFixBugs) {
			objP->m_info.contents.count = 0;
		  sprintf_s (message, sizeof (message),"FIXED: Spawn count must be >= -1 (object=%d,count=%d)",nObject,count);
			}
		else
		  sprintf_s (message, sizeof (message),"WARNING: Spawn count must be >= -1 (object=%d,count=%d)",nObject,count);
      if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, nObject)) 
			return true;
    }

    // check container type range
	if (count > 0) {
      type = objP->m_info.contents.type;
	  if (type != OBJ_ROBOT && type != OBJ_POWERUP) {
		if (m_bAutoFixBugs) {
			objP->m_info.contents.type = OBJ_POWERUP;
			sprintf_s (message, sizeof (message),"FIXED: Illegal contained type (object=%d,contains=%d)",nObject,type);
			}
		else
			sprintf_s (message, sizeof (message),"WARNING: Illegal contained type (object=%d,contains=%d)",nObject,type);
	if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, nObject)) return true;
	  }
	  id = objP->m_info.contents.id;
	  // check contains id range
	  if (CheckId (objP)) {
	sprintf_s (message, sizeof (message),"WARNING: Illegal contains id (object=%d,contains id=%d)",nObject,id);
	if (UpdateStats (message,0,1)) return true;
	  }
	}
  }

  // make sure object 0 is player 0; if not int it
if (theMine->Objects (0)->m_info.type != OBJ_PLAYER || theMine->Objects (0)->m_info.id != 0) {
	if (m_bAutoFixBugs) {
		CGameObject h;
		memcpy (&h, pPlayer, sizeof (CGameObject));
		memcpy (pPlayer, theMine->Objects (0), sizeof (CGameObject));
		memcpy (theMine->Objects (0), pPlayer, sizeof (CGameObject));
		strcpy_s (message, sizeof (message), "FIXED: Object 0 was not Player 0.");
		if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, nObject))
			return true;
		}
	else {
		strcpy_s (message, sizeof (message), "WARNING: Object 0 is not Player 0.");
		if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, nObject))
			return true;
		}
	}

	// make sure there is the proper number of players and coops
	// reset count to zero
	memset (players, 0, sizeof (players));
	memset (nPlayers, 0, sizeof (nPlayers));
	bFix = 0;
	// count each
	objP = theMine->Objects (0);
	for (nObject = 0; nObject < objCount; nObject++, objP++) {
		if (objP->m_info.type == OBJ_PLAYER) {
			nPlayers [0]++;
			if (CheckAndFixPlayer (0, MAX_PLAYERS, nObject, players))
				bFix |= 1;
			}
		else if (objP->m_info.type == OBJ_COOP) {
			nPlayers [1]++;
			if (CheckAndFixPlayer (0, 3, nObject, players + MAX_PLAYERS))
				bFix |= 2;
			}
		}
// resequence player and coop ids
if (m_bAutoFixBugs) {
	int i;
	if (bFix & 1) {
		for (i = id = 0; id < MAX_PLAYERS; id++)
			if (players [id] != 0) 
				players [id] = ++i;
		}
	if (bFix & 2) {
		for (i = 0, id = MAX_PLAYERS; id < MAX_PLAYERS + MAX_COOP_PLAYERS; id++)
			if (players [id] != 0) 
				players [id] = ++i;
		}
	objP = theMine->Objects (0);
	for (nObject = 0; nObject < objCount; nObject++, objP++) {
		if (objP->m_info.type == OBJ_PLAYER) {
			if ((bFix & 1) && (objP->m_info.id >= 0) && (objP->m_info.id < MAX_PLAYERS))
				objP->m_info.id = players [objP->m_info.id] - 1;
			}
		else if (objP->m_info.type == OBJ_COOP) {
			if ((bFix & 2) && (objP->m_info.id >= MAX_PLAYERS) && (objP->m_info.id < MAX_PLAYERS + MAX_COOP_PLAYERS))
				objP->m_info.id = players [objP->m_info.id] - 1;
			}
		}
	}

for (id = 0; id < MAX_PLAYERS; id++) {
	if (players [id] == 0) {
		sprintf_s (message, sizeof (message),"INFO: No player %d", id + 1);
		if (UpdateStats (message, 0)) 
			return true;
		}
	}
if (nPlayers [0] > MAX_PLAYERS) {
	sprintf_s (message, sizeof (message),"WARNING: too many players found (found %d, max. allowed %d)", nPlayers [0], MAX_PLAYERS);
	if (UpdateStats (message,0)) 
		return true;
	}
if (nPlayers [1] < 3) {
	sprintf_s (message, sizeof (message),"INFO: %d coop players found (3 allowed)", nPlayers [1]);
	if (UpdateStats (message,0)) 
		return true;
	}
else if (nPlayers [1] > 3) {
	sprintf_s (message, sizeof (message),"WARNING: too many coop players found (found %d, max. allowed 3)", nPlayers [1]);
	if (UpdateStats (message,0)) 
		return true;
	}

  // make sure there is only one control center
count = 0;
objP = theMine->Objects (0);
for (nObject=0;nObject<objCount;nObject++, objP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	type = objP->m_info.type;
	if (type == OBJ_CNTRLCEN) {
		if (theMine->Segments (objP->m_info.nSegment)->m_info.function != SEGMENT_FUNC_CONTROLCEN) {
			if (m_bAutoFixBugs && theMine->AddRobotMaker (objP->m_info.nSegment, false, false))
				sprintf_s (message, sizeof (message),"FIXED: Reactor belongs to a segment of wrong type (objP=%d, segP=%d)",nObject,objP->m_info.nSegment);
			else
				sprintf_s (message, sizeof (message),"WARNING: Reactor belongs to a segment of wrong type (objP=%d, segP=%d)",nObject,objP->m_info.nSegment);
			if (UpdateStats (message,0, nSegment, -1, -1, -1, -1, -1, -1, nObject))
				return true;
			}
		count++;
		if (count > 1) {
			sprintf_s (message, sizeof (message),"WARNING: More than one Reactor found (object=%d)",nObject);
			if (UpdateStats (message,0, nSegment, -1, -1, -1, -1, -1, -1, nObject))
				return true;
			}
		}
	}
if (count < 1) {
	sprintf_s (message, sizeof (message),"WARNING: No Reactors found (note: ok if boss robot used)");
	if (UpdateStats (message,0)) 
		return true;
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Objects] (no errors)");
	}
return false;
}

//--------------------------------------------------------------------------
// CDiagTool - check_triggers ()
//
//  x unlinked or overlinked Triggers ()
//
//  o key exists for all locked doors
//  o exit (normal and special)
//  o center exists
//  o non-keyed doors have trigers
//  o Triggers () point to something
//--------------------------------------------------------------------------

bool CDiagTool::CheckTriggers ()
 {
if ((theMine == null)) 
	return false;

	int count, nTrigger, deltrignum, nWall, i;
	int nSegment, nSide, linknum;
	short nOppSeg, nOppSide;

	short sub_errors = m_nErrors [0];
	short sub_warnings = m_nErrors [1];
	LBBugs ()->AddString ("[Triggers]");
	int segCount = segmentManager.Count ();
	int trigCount = theMine->Info ().triggers.count;
	CTrigger *trigP = theMine->Triggers (0);
	int wallCount = theMine->Info ().walls.count;
	CWall *wallP;
	CReactorTrigger *reactorTrigger = theMine->ReactorTriggers (0);

	// make sure trigP is linked to exactly one wallP
for (i = 0; i < reactorTrigger->m_count; i++)
	if ((reactorTrigger->Segment (i) >= segCount) ||
		(theMine->Segments (reactorTrigger->Segment (i))->m_sides [reactorTrigger->Side (i)].m_info.nWall >= wallCount)) {
		if (m_bAutoFixBugs) {
			reactorTrigger->Delete (i);
			strcpy_s (message, sizeof (message), "FIXED: Reactor has invalid trigP target.");
			if (UpdateStats (message, 0))
				return true;
			}
		else {
			strcpy_s (message, sizeof (message), "WARNING: Reactor has invalid trigP target.");
			if (UpdateStats (message, 0))
				return true;
			}
		}
for (nTrigger = deltrignum = 0; nTrigger < trigCount; nTrigger++, trigP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	count = 0;
	wallP = theMine->Walls (0);
	for (nWall = 0; nWall < wallCount; nWall++, wallP++) {
		if (wallP->m_info.nTrigger == nTrigger) {
			// if exit, make sure it is linked to CReactorTrigger
			int tt = trigP->m_info.type;
			int tf = trigP->m_info.flags;
			if (DLE.IsD1File () ? tf & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT) : tt == TT_EXIT) {
				for (i = 0; i < reactorTrigger->m_count; i++)
					if (*((CSideKey*) (reactorTrigger)) == *((CSideKey*) (wallP)))
						break; // found it
				// if did not find it
				if (i>=theMine->ReactorTriggers (0)->m_count) {
					if (m_bAutoFixBugs) {
						theMine->AutoUpdateReactor ();
						sprintf_s (message, sizeof (message),"FIXED: Exit not linked to reactor (cube=%d, side=%d)", wallP->m_nSegment, wallP->m_nSide);
						}
					else
						sprintf_s (message, sizeof (message),"WARNING: Exit not linked to reactor (cube=%d, side=%d)", wallP->m_nSegment, wallP->m_nSide);
					if (UpdateStats (message,1,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall))
						return true;
					}
				}
			count++;
			if (count >1) {
				sprintf_s (message, sizeof (message),"WARNING: Trigger belongs to more than one wallP (trig=%d, wallP=%d)",nTrigger,nWall);
				if (UpdateStats (message,0, wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
			}
		}
	}
	if (count < 1) {
		if (m_bAutoFixBugs) {
			theMine->DeleteTrigger (nTrigger);
			nTrigger--;
			trigP--;
			trigCount--;
			sprintf_s (message, sizeof (message),"FIXED: Unused trigP (trigP=%d)",nTrigger + deltrignum);
			deltrignum++;
			}
		else
			sprintf_s (message, sizeof (message),"WARNING: Unused trigP (trigP=%d)",nTrigger);
		if (UpdateStats (message,0,1)) return true;
		}
	}

short trigSeg, trigSide;
trigP = theMine->Triggers (0);
for (nTrigger = 0; nTrigger < trigCount; nTrigger++, trigP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	wallP = wallManager.FindByTrigger (nTrigger);
	if (wallP != null) {
		trigSeg = wallP->m_nSegment;
		trigSide = wallP->m_nSide;
		}
	else
		trigSeg = trigSide = -1;
	// check number of links of trigP (only for
	int tt = trigP->m_info.type;
	int tf = trigP->m_info.flags;
	if (trigP->m_count == 0) {
		if (DLE.IsD1File ()
			 ? tf & (TRIGGER_CONTROL_DOORS | TRIGGER_ON | TRIGGER_ONE_SHOT | TRIGGER_MATCEN | TRIGGER_ILLUSION_OFF | TRIGGER_ILLUSION_ON) 
			 : (tt != TT_EXIT) && (tt != TT_SECRET_EXIT) && (tt != TT_MESSAGE) && (tt != TT_SOUND) && 
			   (tt != TT_SPEEDBOOST) && (tt != TT_SHIELD_DAMAGE_D2) && (tt != TT_ENERGY_DRAIN_D2)
			) {
			sprintf_s (message, sizeof (message),"WARNING: Trigger has no targets (trigP=%d)",nTrigger);
			if (UpdateStats (message,0, -1, -1, -1, -1, -1, -1, nTrigger))
				return true;
			}
		}
	else {
		// check range of links
		for (linknum = 0; linknum < trigP->m_count; linknum++) {
			if (linknum >= MAX_TRIGGER_TARGETS) {
				if (m_bAutoFixBugs) {
					trigP->m_count = MAX_TRIGGER_TARGETS;
					sprintf_s (message, sizeof (message),"FIXED: Trigger has too many targets (trigP=%d, number of links=%d)",nTrigger,linknum);
					}
				else
					sprintf_s (message, sizeof (message),"WARNING: Trigger has too many targets (trigP=%d, number of links=%d)",nTrigger,linknum);
				if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) 
					return true;
				break;
				}
			// check segment range
			nSegment = trigP->Segment (linknum);
			nSide = trigP->Side (linknum);
			if ((nSegment < 0) || ((nSide < 0) ? (nSegment >= theMine->ObjectCount ()) : (nSegment >= segmentManager.Count ()))) {
				if (m_bAutoFixBugs) {
					if (theMine->DeleteTargetFromTrigger (trigP, linknum))
						linknum--;
					else { // => trigP deleted
						linknum = MAX_TRIGGER_TARGETS;	// take care of the loops
						trigP--;
						}
					sprintf_s (message, sizeof (message),"FIXED: Trigger points to non-existant %s (trigP=%d, cube=%d)", 
								  (nSide < 0) ? "object" : "segment", nTrigger, nSegment);
					}
				else
					sprintf_s (message, sizeof (message),"ERROR: Trigger points to non-existant %s (trigP=%d, cube=%d)", 
								  (nSide < 0) ? "object" : "segment", nTrigger, nSegment);
				if (UpdateStats (message,1, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) 
					return true;
				}
			else {
				// check side range
				if (nSide < -1 || nSide > 5) {
					if (m_bAutoFixBugs) {
						if (nSide < 0)
							nSide = 0;
						else if (nSide > 5)
							nSide = 5;
						sprintf_s (message, sizeof (message),"FIXED: Trigger points to non-existant side (trigP=%d, side=%d)",nTrigger,nSide);
						}
					else
						sprintf_s (message, sizeof (message),"ERROR: Trigger points to non-existant side (trigP=%d, side=%d)",nTrigger,nSide);
					if (UpdateStats (message, 1, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) 
						return true;
				} else {
					CSegment *segP = segmentManager.GetSegment (nSegment);
					// check door opening trigP
//						if (trigP->m_info.flags == TRIGGER_CONTROL_DOORS) {
					if (DLE.IsD1File ()
						 ? tf & TRIGGER_CONTROL_DOORS 
						 : tt==TT_OPEN_DOOR || tt==TT_CLOSE_DOOR || tt==TT_LOCK_DOOR || tt==TT_UNLOCK_DOOR) {
						// make sure trigP points to a wallP if it controls doors
						if (segP->m_sides[nSide].m_info.nWall >= wallCount) {
							if (m_bAutoFixBugs) {
								if (theMine->DeleteTargetFromTrigger (trigP, linknum))
									linknum--;
								else {
									linknum = MAX_TRIGGER_TARGETS;
									trigP--;
									}
								sprintf_s (message, sizeof (message),"FIXED: Trigger does not target a door (trigP=%d, link= (%d,%d))",nTrigger,nSegment,nSide);
								}
							else
								sprintf_s (message, sizeof (message),"WARNING: Trigger does not target a door (trigP=%d, link= (%d,%d))",nTrigger,nSegment,nSide);
							if (UpdateStats (message, 0, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) return true;
						}

						// make sure oposite segment/side has a wallP too
						if ((theMine == null)->GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide)) {
							sprintf_s (message, sizeof (message),"WARNING: Trigger opens a single sided door (trigP=%d, link= (%d,%d))",nTrigger,nSegment,nSide);
							if (UpdateStats (message, 0, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) return true;
							}
						else {
							if (theMine->Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall >= wallCount) {
								sprintf_s (message, sizeof (message),"WARNING: Trigger opens a single sided door (trigP=%d, link= (%d,%d))",nTrigger,nSegment,nSide);
								if (UpdateStats (message,1, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) return true;
								}
							}
						}
					else if (DLE.IsD1File () 
								? tf & (TRIGGER_ILLUSION_OFF | TRIGGER_ILLUSION_ON) 
								: tt == TT_ILLUSION_OFF || tt == TT_ILLUSION_ON || tt == TT_OPEN_WALL || tt == TT_CLOSE_WALL || tt == TT_ILLUSORY_WALL
							  ) {
						// make sure trigP points to a wallP if it controls doors
						if (segP->m_sides [nSide].m_info.nWall >= wallCount) {
							if (m_bAutoFixBugs) {
								if (theMine->DeleteTargetFromTrigger (trigP, linknum))
									linknum--;
								else {
									linknum = MAX_TRIGGER_TARGETS;
									trigP--;
									}
								sprintf_s (message, sizeof (message),"FIXED: Trigger target does not exist (trigP=%d, link= (%d,%d))",nTrigger,nSegment,nSide);
								}
							else
								sprintf_s (message, sizeof (message),"ERROR: Trigger target does not exist (trigP=%d, link= (%d,%d))",nTrigger,nSegment,nSide);
							if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) return true;
							}
						}
//						if (trigP->m_info.flags == TRIGGER_MATCEN) {
					else if (DLE.IsD1File () ? tf & TRIGGER_MATCEN : tt == TT_MATCEN) {
						if ((segP->m_info.function != SEGMENT_FUNC_ROBOTMAKER) && (segP->m_info.function != SEGMENT_FUNC_EQUIPMAKER)) {
							sprintf_s (message, sizeof (message),"WARNING: Trigger does not target a robot or equipment maker (trigP=%d, link= (%d,%d))",nTrigger,nSegment,nSide);
							if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) return true;
							}
						}
					}
				}
			}
		}
	}

// make sure there is exactly one exit and its linked to the CReactorTrigger
count = 0;
trigP = theMine->Triggers (0);
for (nTrigger = 0; nTrigger < trigCount; nTrigger++, trigP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	wallP = wallManager.FindByTrigger (nTrigger);
	if (nWall != null) {
		trigSeg = wallP->m_nSegment;
		trigSide = wallP->m_nSide;
		}
	else
		trigSeg = trigSide = -1;
	int tt = trigP->m_info.type;
	int tf = trigP->m_info.flags;
	if (DLE.IsD1File () ? tf & TRIGGER_EXIT : tt == TT_EXIT) {
		count++;
		if (count >1) {
			sprintf_s (message, sizeof (message),"WARNING: More than one exit found (trig=%d)",nTrigger);
			if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, nTrigger)) return true;
			}
		}
	}

trigCount = theMine->NumObjTriggers ();
for (nTrigger = 0; nTrigger < trigCount; nTrigger++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	trigP = theMine->ObjTriggers (nTrigger);
	if ((trigP->m_info.type != TT_MESSAGE) && (trigP->m_info.type != TT_SOUND) && (trigP->m_info.type != TT_COUNTDOWN) && !trigP->m_count) {
		sprintf_s (message, sizeof (message), "ERROR: Object trigP has no targets (trigP=%d, object=%d))", nTrigger, trigP->m_info.nObject);
		if (UpdateStats (message,0, nTrigger, trigP->m_info.nObject, -1, -1, -1, -1, nTrigger)) return true;
		}
	}
if (count < 1) {
	sprintf_s (message, sizeof (message),"WARNING: No exit found");
	if (UpdateStats (message,0)) return true;
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount () - 1);
	LBBugs ()->AddString ("[Triggers] (no errors)");
	}
return false;
}

//------------------------------------------------------------------------

char CDiagTool::FindMatCen (CRobotMaker* matCenP, short nSegment, short* refList)
{
	char	h = -1, i, j = char (theMine->Info ().botgen.count);

if (refList) {
	for (i = 0; i < j; i++) {
		if (refList [i] >= 0) {
			if (matCenP [i].m_info.objFlags [0] || matCenP [i].m_info.objFlags [1])
				return i;
			h = i;
			}
		}
	}
else {
	for (i = 0; i < j; i++) {
		if (matCenP [i].m_info.nSegment == nSegment)
			return i;
		}
	}
return h;
}

//------------------------------------------------------------------------

void CDiagTool::CountMatCenRefs (int nSpecialType, short* refList, CRobotMaker* matCenP, short nMatCens)
{
	CSegment*		segP = theMine->Segments (0);
	short				n, h, i, j = segmentManager.Count ();

memset (refList, 0, sizeof (*refList) * MAX_NUM_MATCENS_D2);
for (h = i = 0; i < j; i++, segP++) {
	if (segP->m_info.function == byte (nSpecialType)) {
		n = segP->m_info.nMatCen;
		if ((n >= 0) && (n < nMatCens) && (refList [n] >= 0)) {
			if (matCenP [n].m_info.nSegment == i)
				refList [n] = -1;
			else
				refList [n]++;
			}
		}
	}
}

//------------------------------------------------------------------------

short CDiagTool::FixMatCens (int nSpecialType, short* segList, short* refList, CRobotMaker* matCenP, short nMatCens, char* pszType)
{
	CSegment*	segP = theMine->Segments (0);
	short			h, i, j = segmentManager.Count ();
	char			n;

for (h = i = 0; i < j; i++, segP++) {
	if (segP->m_info.function != byte (nSpecialType))
		continue;
	n = segP->m_info.nMatCen;
	if ((n < 0) || (n >= nMatCens)) {
		sprintf_s (message, sizeof (message), "%s: %s maker list corrupted (segment=%d)", m_bAutoFixBugs ? "FIXED" : "ERROR", pszType, i);
		if (m_bAutoFixBugs)
			n = segP->m_info.nMatCen = char (FindMatCen (matCenP, i));
		}
	else if (matCenP [n].m_info.nSegment != i) {
		sprintf_s (message, sizeof (message), "%s: %s maker list corrupted (segment=%d)", m_bAutoFixBugs ? "FIXED" : "ERROR", pszType, i);
		if (m_bAutoFixBugs) {
			n = char (FindMatCen (matCenP, i));
			if (n >= 0) {
				segP->m_info.nMatCen = n;
				refList [n] = -1;
				}
			else {
				n = segP->m_info.nMatCen;
				if (refList [n] >= 0) {
					matCenP [n].m_info.nSegment = i;
					refList [n] = -1;
					}
				else
					segP->m_info.nMatCen = -1;
				}
			}
		}
	if (h < MAX_NUM_MATCENS_D2)
		segList [h++] = i;
	else {
		sprintf_s (message, sizeof (message), "%s: Too many %s makers", m_bAutoFixBugs ? "FIXED" : "ERROR", pszType, i);
		if (m_bAutoFixBugs) 
			theMine->UndefineSegment (i);
		}
	}
return h;
}

//------------------------------------------------------------------------

short CDiagTool::AssignMatCens (int nSpecialType, short* segList, short* refList, CRobotMaker* matCenP, short nMatCens)
{
if (!m_bAutoFixBugs)
	return nMatCens;

	CSegment*	segP = theMine->Segments (0);
	short			h, i, j = segmentManager.Count ();
	char			n;

for (h = i = 0; i < j; i++, segP++) {
	if (segP->m_info.function != byte (nSpecialType))
		continue;
	n = segP->m_info.nMatCen;
	if (n >= 0)
		continue;
	n = FindMatCen (matCenP, i, refList);
	if (n >= 0) {
		segP->m_info.nMatCen = n;
		matCenP [n].m_info.nSegment = i;
		refList [n] = -1;
		}
	else if (m_bAutoFixBugs)
		theMine->UndefineSegment (i);
	}
return h;
}

//------------------------------------------------------------------------

short CDiagTool::CleanupMatCens (short* refList, CRobotMaker* matCenP, short nMatCens)
{
if (!m_bAutoFixBugs)
	return nMatCens;

	CSegment*	segP = theMine->Segments (0);
	
for (int i = 0; i < nMatCens; i) {
	if (refList [i] < 0) 
		i++;
	else {
		if (i < --nMatCens) {
			matCenP [i] = matCenP [nMatCens];
			matCenP [i].m_info.nFuelCen =
			segP [matCenP [i].m_info.nSegment].m_info.nMatCen = i;
			refList [i] = refList [nMatCens];
			}
		}
	}
return nMatCens;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckBotGens (void)
{
if ((theMine == null)) 
	return false;

	short					h = segmentManager.Count (), i, nSegment = 0;
	bool					bOk = true;
	short					nMatCenSegs, nMatCens = short (theMine->Info ().botgen.count);
	CSegment*			segP = theMine->Segments (0);
	CRobotMaker*		matCenP = theMine->BotGens (0);
	short					segList [MAX_NUM_MATCENS_D2];
	short					refList [MAX_NUM_MATCENS_D2];

for (i = 0; i < nMatCens; i++)
	matCenP [i].m_info.nFuelCen = i;
CountMatCenRefs (SEGMENT_FUNC_ROBOTMAKER, refList, matCenP, nMatCens);
nMatCenSegs = FixMatCens (SEGMENT_FUNC_ROBOTMAKER, segList, refList, matCenP, nMatCens, "Robot");
AssignMatCens (SEGMENT_FUNC_ROBOTMAKER, segList, refList, matCenP, nMatCens);
theMine->Info ().botgen.count = CleanupMatCens (refList, matCenP, nMatCens);
if (!bOk) {
	sprintf_s (message, sizeof (message), "%s: Robot maker list corrupted (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
	if (UpdateStats (message, 0)) return true;
	}
return false;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckEquipGens (void)
{
	short					i, nSegment = 0;
	bool					bOk = true;
	int					nMatCenSegs, nMatCens = int (theMine->Info ().equipGen.count);
	CRobotMaker*		matCenP = theMine->EquipGens (0);
	short					segList [MAX_NUM_MATCENS_D2];
	short					refList [MAX_NUM_MATCENS_D2];

for (i = 0; i < nMatCens; i++)
	matCenP [i].m_info.nFuelCen = i;
CountMatCenRefs (SEGMENT_FUNC_EQUIPMAKER, refList, matCenP, nMatCens);
nMatCenSegs = FixMatCens (SEGMENT_FUNC_EQUIPMAKER, segList, refList, matCenP, nMatCens, "Equipment");
AssignMatCens (SEGMENT_FUNC_EQUIPMAKER, segList, refList, matCenP, nMatCens);
theMine->Info ().equipGen.count = CleanupMatCens (refList, matCenP, nMatCens);
if (!bOk) {
	sprintf_s (message, sizeof (message), "%s: Equipment maker list corrupted (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
	if (UpdateStats (message, 0)) return true;
	}
return false;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CWall *CDiagTool::OppWall (ushort nSegment, ushort nSide)
{
	short	oppSegnum, oppSidenum, nWall;

if ((theMine == null)->GetOppositeSide (oppSegnum, oppSidenum, nSegment, nSide))
	return null;
nWall = theMine->Segments (oppSegnum)->m_sides [oppSidenum].m_info.nWall;
if ((nWall < 0) || (nWall > MAX_WALLS))
	return null;
return theMine->Walls (nWall);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CDiagTool::CheckWalls (void) 
{
if ((theMine == null)) 
	return false;

	short nSegment,nSide;
	ushort nWall, wallCount = theMine->Info ().walls.count, 
			 maxWalls = MAX_WALLS;
	CSegment *segP;
	CSide *sideP;
	CWall *wallP = theMine->Walls (0), *w, *ow;
	int segCount = segmentManager.Count ();
	byte wallFixed [MAX_WALLS_D2];

	short sub_errors = m_nErrors [0];
	short sub_warnings = m_nErrors [1];
	LBBugs ()->AddString ("[Walls]");

memset (wallFixed, 0, sizeof (wallFixed));
*message = '\0';
for (nSegment = 0, segP = theMine->Segments (0); nSegment < segCount; nSegment++, segP++) {
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++) {
		nWall = sideP->m_info.nWall;
		if ((nWall < 0) || (nWall >= wallCount) || (nWall >= maxWalls)) {
			if (nWall != NO_WALL)
				sideP->m_info.nWall = NO_WALL;
			continue;
			}
		w = theMine->Walls (nWall);
		if (w->m_nSegment != nSegment) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),
							"FIXED: Wall sits in wrong cube (cube=%d, wall=%d, parent=%d)",
							nSegment, nWall, w->m_nSegment);
				if (wallFixed [nWall])
					sideP->m_info.nWall = NO_WALL;
				else {
					if (theMine->Segments (w->m_nSegment)->m_sides [w->m_nSide].m_info.nWall == nWall)
						sideP->m_info.nWall = NO_WALL;
					else {
						w->m_nSegment = nSegment;
						w->m_nSide = nSide;
						}
					wallFixed [nWall] = 1;
					}
				}
			else
				sprintf_s (message, sizeof (message),
							"ERROR: Wall sits in wrong cube (cube=%d, wall=%d, parent=%d)",
							nSegment, nWall, w->m_nSegment);
			if (UpdateStats (message,1, nSegment, nSide, -1, -1, -1, sideP->m_info.nWall)) return true;
			} 
		else if (w->m_nSide != nSide) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),
							"FIXED: Wall sits at wrong side (cube=%d, side=%d, wall=%d, parent=%d)",
							nSegment, nSide, nWall, w->m_nSegment);
				if (wallFixed [nWall])
					sideP->m_info.nWall = NO_WALL;
				else {
					ow = OppWall (nSegment, nSide);
					if (ow && (ow->m_info.type == w->m_info.type)) {
						segP->m_sides [w->m_nSide].m_info.nWall = NO_WALL;
						w->m_nSide = nSide;
						}
					else if (segP->m_sides [w->m_nSide].m_info.nWall == nWall)
						sideP->m_info.nWall = NO_WALL;
					else
						w->m_nSide = nSide;
					wallFixed [nWall] = 1;
					}
				}
			else
				sprintf_s (message, sizeof (message),
							"ERROR: Wall sits at wrong side (cube=%d, side=%d, wall=%d, parent=%d)",
							nSegment, nSide, nWall, w->m_nSegment);
			if (UpdateStats (message,1, -1, -1, -1, -1, -1, sideP->m_info.nWall)) return true;
			}
		} 
	}
for (nWall = 0; nWall < wallCount; nWall++, wallP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	// check wall range type
	if (wallP->m_info.type > (DLE.IsD1File () ? WALL_CLOSED : theMine->IsStdLevel () ? WALL_CLOAKED : WALL_TRANSPARENT)) {
		sprintf_s (message, sizeof (message),
					"ERROR: Wall type out of range (wall=%d, type=%d)",
					nWall,wallP->m_info.type);
		if (UpdateStats (message,1,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
		}
		// check range of segment number that the wall points to
	if (wallP->m_nSegment >= segmentManager.Count ()) {
		sprintf_s (message, sizeof (message),
					"ERROR: Wall sits in non-existant cube (wall=%d, cube=%d)",
					nWall,wallP->m_nSegment);
		if (UpdateStats (message,1,-1, -1, -1, -1, -1, nWall)) return true;
		} 
	else if (wallP->m_nSide >= 6) {
		// check range of side number that the wall points to
		sprintf_s (message, sizeof (message),
					"ERROR: Wall sits on side which is out of range (wall=%d, side=%d)",
					nWall,wallP->m_nSide);
		if (UpdateStats (message,1,-1 -1, -1, -1, -1, nWall)) return true;
		}
	else {
		// check to make sure segment points back to wall
		sideP = theMine->Segments (wallP->m_nSegment)->m_sides + wallP->m_nSide;
		if (sideP->m_info.nWall != nWall) {
			w = theMine->Walls (nWall);
			if ((nWall < wallCount) && (w->m_nSegment == wallP->m_nSegment) && (w->m_nSide == wallP->m_nSide)) {
				if (m_bAutoFixBugs) {
					sprintf_s (message, sizeof (message),
								"FIXED: Duplicate wall found (wall=%d, cube=%d)", nWall, wallP->m_nSegment);
					theMine->DeleteWall (nWall);
					nWall--;
					wallP--;
					wallCount--;
					continue;
					}
				else 
					sprintf_s (message, sizeof (message),
								"ERROR: Duplicate wall found (wall=%d, cube=%d)", nWall, wallP->m_nSegment);
				if (UpdateStats (message, 1, wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
				}
			else {
				if (m_bAutoFixBugs) {
					sideP->m_info.nWall = nWall;
					sprintf_s (message, sizeof (message),
								"FIXED: Cube does not reference wall which sits in it (wall=%d, cube=%d)",
								nWall,wallP->m_nSegment);
					}
				else 
					sprintf_s (message, sizeof (message),
								"ERROR: Cube does not reference wall which sits in it (wall=%d, cube=%d)",
								nWall,wallP->m_nSegment);
				if (UpdateStats (message,1,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
				}
			}
			// make sure trigger number of wall is in range
		if ((wallP->m_info.nTrigger != NO_TRIGGER) && (wallP->m_info.nTrigger >= theMine->Info ().triggers.count)) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),
							"FIXED: Wall has invalid trigger (wall=%d, trigger=%d)",
							nWall, wallP->m_info.nTrigger);
				wallP->m_info.nTrigger = NO_TRIGGER;
				}
			else
				sprintf_s (message, sizeof (message),
							"ERROR: Wall has invalid trigger (wall=%d, trigger=%d)",
							nWall, wallP->m_info.nTrigger);
			if (UpdateStats (message,1,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
			}
#if 1 // linked walls not supported in DLE-XP and D2X-XL
		if (wallP->m_info.linkedWall != -1) {
			short invLinkedWall = wallP->m_info.linkedWall;
			if (m_bAutoFixBugs) {
				wallP->m_info.linkedWall = -1;
				sprintf_s (message, sizeof (message),
							  "FIXED: Wall has invalid linked wall (wall=%d, linked wall=%d [%d])",
							  nWall, invLinkedWall, wallP->m_info.linkedWall);
				}
			else
				sprintf_s (message, sizeof (message),
							  "ERROR: Wall has invalid linked wall (wall=%d, linked wall=%d [%d])",
							  nWall, invLinkedWall, wallP->m_info.linkedWall);
			}
#else
		if ((wallP->m_info.linkedWall < -1) || (wallP->m_info.linkedWall >= wallCount)) {
			if (m_bAutoFixBugs) {
				short	oppSeg, oppSide, invLinkedWall = wallP->m_info.linkedWall;
				if (theMine->GetOppositeSide (oppSeg, oppSide, wallP->m_nSegment, wallP->m_nSide)) {
					wallP->m_info.linkedWall = theMine->Segments (oppSeg)->m_sides [oppSide].m_info.nWall;
					if ((wallP->m_info.linkedWall < -1) || (wallP->m_info.linkedWall >= wallCount))
						wallP->m_info.linkedWall = -1;
					sprintf_s (message, sizeof (message),
						"FIXED: Wall has invalid linked wall (wall=%d, linked wall=%d [%d])",
						nWall, invLinkedWall, wallP->m_info.linkedWall);
					}
				}
			else
				sprintf_s (message, sizeof (message),
					"ERROR: Wall has invalid linked wall (wall=%d, linked wall=%d)",
					nWall,wallP->m_info.linkedWall);
			}
		else if (wallP->m_info.linkedWall >= 0) {
			short	oppSeg, oppSide;
			if (theMine->GetOppositeSide (oppSeg, oppSide, wallP->m_nSegment, wallP->m_nSide)) {
				short oppWall = theMine->Segments (oppSeg)->m_sides [oppSide].m_info.nWall;
				if ((oppWall < 0) || (oppWall >= wallCount)) {
					sprintf_s (message, sizeof (message),
						"%s: Wall links to non-existant wall (wall=%d, linked side=%d,%d)",
						m_bAutoFixBugs ? "FIXED" : "ERROR",
						nWall, theMine->Walls (wallP->m_info.linkedWall)->m_info.nSegment, theMine->Walls (wallP->m_info.linkedWall)->nSide);
						if (m_bAutoFixBugs)
							wallP->m_info.linkedWall = -1;
					}
				else if (wallP->m_info.linkedWall != oppWall) {
					sprintf_s (message, sizeof (message),
						"%s: Wall links to wrong opposite wall (wall=%d, linked side=%d,%d)",
						m_bAutoFixBugs ? "FIXED" : "ERROR",
						nWall, theMine->Walls (wallP->m_info.linkedWall)->m_info.nSegment, theMine->Walls (wallP->m_info.linkedWall)->nSide);
						if (m_bAutoFixBugs)
							wallP->m_info.linkedWall = oppWall;
					}
				}
			else {
				sprintf_s (message, sizeof (message),
					"%s: Wall links to non-existant side (wall=%d, linked side=%d,%d)",
					m_bAutoFixBugs ? "FIXED" : "ERROR",
					nWall, theMine->Walls (wallP->m_info.linkedWall)->m_info.nSegment, theMine->Walls (wallP->m_info.linkedWall)->nSide);
				if (m_bAutoFixBugs)
					wallP->m_info.linkedWall = -1;
				}
			}
#endif
		if (UpdateStats (message, 1, wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
			// check wall nClip
		if ((wallP->m_info.type == WALL_CLOAKED) && (wallP->m_info.cloakValue > 31)) {
			if (m_bAutoFixBugs) {
				wallP->m_info.cloakValue = 31;
				sprintf_s (message, sizeof (message), "FIXED: Wall has invalid cloak value (wall=%d)", nWall);
					}
			else
				sprintf_s (message, sizeof (message), "ERROR: Wall has invalid cloak value (wall=%d)", nWall);
			}
		if ((wallP->m_info.type == WALL_BLASTABLE || wallP->m_info.type == WALL_DOOR) &&
			 (   wallP->m_info.nClip < 0
			  || wallP->m_info.nClip == 2
//			     || wallP->m_info.nClip == 7
			  || wallP->m_info.nClip == 8
			  || (DLE.IsD1File () && wallP->m_info.nClip > 25)
			  || (DLE.IsD2File () && wallP->m_info.nClip > 50))) {
			sprintf_s (message, sizeof (message),
						"ERROR: Illegal wall clip number (wall=%d, clip number=%d)",
						nWall,wallP->m_info.nClip);
			if (UpdateStats (message,1,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
			}
			// Make sure there is a child to the segment
		if (wallP->m_info.type != WALL_OVERLAY) {
			if (!(theMine->Segments (wallP->m_nSegment)->m_info.childFlags & (1<< wallP->m_nSide))) {
				sprintf_s (message, sizeof (message),
							"ERROR: No adjacent cube for this door (wall=%d, cube=%d)",
							nWall,wallP->m_nSegment);
				if (UpdateStats (message,1,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
				}
			else {
				nSegment = theMine->Segments (wallP->m_nSegment)->GetChild (wallP->m_nSide);
				CSegment *segP = segmentManager.GetSegment (nSegment);
				if ((nSegment >= 0 && nSegment < segmentManager.Count ()) &&
					 (wallP->m_info.type == WALL_DOOR || wallP->m_info.type == WALL_ILLUSION)) {
					// find segment's child side
					for (nSide=0;nSide<6;nSide++)
						if (segP->GetChild (nSide) == wallP->m_nSegment)
							break;
					if (nSide != 6) {  // if child's side found
						if (segP->m_sides[nSide].m_info.nWall >= theMine->Info ().walls.count) {
							sprintf_s (message, sizeof (message),
										"WARNING: No matching wall for this wall (wall=%d, cube=%d)", 
										nWall,nSegment);
							if (UpdateStats (message,0,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
							} 
						else {
							ushort wallnum2 = segP->m_sides[nSide].m_info.nWall;
							if ((wallnum2 < wallCount) &&
								 ((wallP->m_info.nClip != theMine->Walls (wallnum2)->m_info.nClip ||
									wallP->m_info.type != theMine->Walls (wallnum2)->m_info.type))) {
								sprintf_s (message, sizeof (message),
											"WARNING: Matching wall for this wall is of different type or clip no. (wall=%d, cube=%d)",
											nWall,nSegment);
								if (UpdateStats (message,0,wallP->m_nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
								}
							}
						}
					}
				}
			}
		}
	}

	// make sure segP's wall points back to the segment
segP = theMine->Segments (0);
for (nSegment=0;nSegment<segCount;nSegment++, segP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	sideP = segP->m_sides;
	for (nSide = 0; nSide < 6; nSide++, sideP++) {
		if (sideP->m_info.nWall <	wallCount) {
			nWall = sideP->m_info.nWall;
			if (nWall >= wallCount) {
				if (m_bAutoFixBugs) {
					sideP->m_info.nWall = wallCount;
					sprintf_s (message, sizeof (message),"FIXED: Cube has an invalid wall number (wall=%d, cube=%d)",nWall,nSegment);
					}
				else
					sprintf_s (message, sizeof (message),"ERROR: Cube has an invalid wall number (wall=%d, cube=%d)",nWall,nSegment);
				if (UpdateStats (message,1, nSegment, nSide)) return true;
			} else {
				if (theMine->Walls (nWall)->m_nSegment != nSegment) {
					if (m_bAutoFixBugs) {
						theMine->Walls (nWall)->m_nSegment = nSegment;
						sprintf_s (message, sizeof (message),"FIXED: Cube's wall does not sit in cube (wall=%d, cube=%d)",nWall,nSegment);
						}
					else
						sprintf_s (message, sizeof (message),"ERROR: Cube's wall does not sit in cube (wall=%d, cube=%d)",nWall,nSegment);
					if (UpdateStats (message,1,nSegment, wallP->m_nSide, -1, -1, -1, nWall)) return true;
					}
				}
			}
		}
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Walls] (no errors)");
	}
return false;
}

//--------------------------------------------------------------------------
// unused verticies
// make sure the mine has an exit
//--------------------------------------------------------------------------

bool CDiagTool::CheckVertices (void) 
{
if ((theMine == null)) 
	return false;

//  bool found;
  int nSegment,nVertex,point;
  int nUnused = 0;

  short sub_errors = m_nErrors [0];
  short sub_warnings = m_nErrors [1];
  LBBugs ()->AddString ("[Misc]");

for (nVertex = theMine->VertCount (); nVertex > 0; )
	theMine->vertexManager.Status (--nVertex) &= ~NEW_MASK;

// mark all used verts
CSegment *segP = theMine->Segments (0);
for (nSegment = segmentManager.Count (); nSegment; nSegment--, segP++)
	for (point = 0; point < 8; point++)
		theMine->vertexManager.Status (segP->m_info.verts [point]) |= NEW_MASK;

for (nVertex = theMine->VertCount (); nVertex > 0; ) {
	DLE.MainFrame ()->Progress ().StepIt ();
	if (!(theMine->vertexManager.Status (--nVertex) & NEW_MASK)) {
		nUnused++;
		if (m_bAutoFixBugs) {
			if (nVertex < --theMine->VertCount ())
				memcpy (theMine->Vertices (nVertex), theMine->Vertices (nVertex + 1), (theMine->VertCount () - nVertex) * sizeof (*theMine->Vertices (0)));
			CSegment *segP = theMine->Segments (0);
			for (nSegment = segmentManager.Count (); nSegment; nSegment--, segP++)
				for (point = 0; point < 8; point++)
					if (segP->m_info.verts [point] >= nVertex)
						segP->m_info.verts [point]--;
			}
		}
	}
for (nVertex = theMine->VertCount (); nVertex > 0; )
	theMine->vertexManager.Status (--nVertex) &= ~NEW_MASK;
if (nUnused) {
	if (m_bAutoFixBugs)
		sprintf_s (message, sizeof (message),"FIXED: %d unused vertices found", nUnused);
	else
		sprintf_s (message, sizeof (message),"WARNING: %d unused vertices found", nUnused);
	if (UpdateStats (message,0)) return true;
	}

if (sub_errors == m_nErrors [0] && sub_warnings == m_nErrors [1]) {
	LBBugs ()->DeleteString (LBBugs ()->GetCount ()-1);
	LBBugs ()->AddString ("[Misc] (no errors)");
	}
return false;
}

//eof check.cpp