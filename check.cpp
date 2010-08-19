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
#include "io.h"

#include <math.h>

typedef struct tBugPos {
	INT32	nSegment;
	INT32	nSide;
	INT32	nLine;
	INT32	nPoint;
	INT32	nChild;
	INT32	nWall;
	INT32	nTrigger;
	INT32	nObject;
} tBugPos;

                        /*--------------------------*/

void CDiagTool::ClearBugList ()
{
if (!m_bInited)
	return;

	CListBox	*plb = LBBugs ();
	INT32 h = plb->GetCount ();
	char szText [256];

tBugPos *pbp;
INT32 i;
for (i = 0; i < h; i++) {
	plb->GetText (i, szText);
	if (!strchr (szText, '['))
		if (pbp = (tBugPos *) plb->GetItemDataPtr (i)) {
			delete pbp;
			plb->SetItemDataPtr (i, NULL);
			}
	}
plb->ResetContent ();
m_nErrors [0] =
m_nErrors [1] = 0;
}

                        /*--------------------------*/

bool CDiagTool::MarkSegment (INT16 nSegment) 
{
if ((nSegment < 0) || (nSegment >= m_mine->SegCount ()))
	return false;
m_mine->Segments (nSegment)->wall_bitmask &= ~MARKED_MASK;
m_mine->MarkSegment (nSegment);
return true;
}

                        /*--------------------------*/

void CDiagTool::OnShowBug (void)
{
	bool bCurSeg;
	CWall *wallP;
	INT32 nWall;

if (!GetMine ())
	return;
INT32 i = LBBugs ()->GetCurSel ();
if ((i < 0) || (i >= LBBugs ()->GetCount ()))
	return;
tBugPos *pbp = (tBugPos *) LBBugs ()->GetItemDataPtr (i);
if (!pbp)
	return;
m_mine->UnmarkAll ();
if (bCurSeg = MarkSegment (pbp->nSegment))
	m_mine->Current ()->nSegment = pbp->nSegment;
MarkSegment (pbp->nChild);
if ((pbp->nSide >= 0) && (pbp->nSide < MAX_SIDES_PER_SEGMENT))
	m_mine->Current ()->nSide = pbp->nSide;
if ((pbp->nLine >= 0) && (pbp->nLine < 4))
	m_mine->Current ()->nLine = pbp->nLine;
if ((pbp->nPoint >= 0) && (pbp->nPoint < 8))
	m_mine->Current ()->nPoint = pbp->nPoint;
if ((pbp->nWall >= 0) && (pbp->nWall < m_mine->GameInfo ().walls.count))
	nWall = pbp->nWall;
else if ((pbp->nTrigger >= 0) && (pbp->nTrigger < m_mine->GameInfo ().triggers.count))
	nWall = m_mine->FindTriggerWall (pbp->nTrigger);
else
	nWall = -1;
if ((nWall >= 0) && MarkSegment ((wallP = m_mine->Walls (nWall))->nSegment))
	if (bCurSeg) {
		m_mine->Other ()->nSegment = wallP->nSegment;
		m_mine->Other ()->nSide = wallP->nSide;
		}
	else {
		m_mine->Current ()->nSegment = wallP->nSegment;
		m_mine->Current ()->nSide = wallP->nSide;
		}
if ((pbp->nObject >= 0) && (pbp->nObject < m_mine->GameInfo ().objects.count))
	m_mine->Current ()->nObject = pbp->nObject;
theApp.MineView ()->Refresh ();
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

double CDiagTool::CalcFlatnessRatio (INT16 nSegment, INT16 nSide) 
{
  INT16 vertnum[4],i;
  tFixVector midpoint1, midpoint2;
  double length1,length2,ave_length, mid_length;
  double ratio1,ratio2;
	tFixVector *vert [4];
  // copy vertnums into an array
	CDSegment *segP = m_mine->Segments (nSegment);
  for (i=0;i<4;i++) {
    vertnum[i] = segP->verts[side_vert[nSide][i]];
	 vert [i] = m_mine->Vertices (vertnum [i]);
  }

  length1 = CalcDistance (vert [0], vert [1], vert [2]);
  length2 = CalcDistance (vert [0], vert [1], vert [3]);
  ave_length = (length1 + length2) / 2;

  midpoint1.x = (vert [0]->x + vert [1]->x) / 2;
  midpoint1.y = (vert [0]->y + vert [1]->y) / 2;
  midpoint1.z = (vert [0]->z + vert [1]->z) / 2;

  midpoint2.x = (vert [2]->x + vert [3]->x) / 2;
  midpoint2.y = (vert [2]->y + vert [3]->y) / 2;
  midpoint2.z = (vert [2]->z + vert [3]->z) / 2;

  mid_length = m_mine->CalcLength (&midpoint1, &midpoint2);

  ratio1 = mid_length/ave_length;

  length1 = CalcDistance (vert [1], vert [2], vert [3]);
  length2 = CalcDistance (vert [1], vert [2], vert [0]);
  ave_length = (length1 + length2) / 2;

  midpoint1.x = (vert [1]->x + vert [2]->x) / 2;
  midpoint1.y = (vert [1]->y + vert [2]->y) / 2;
  midpoint1.z = (vert [1]->z + vert [2]->z) / 2;

  midpoint2.x = (vert [3]->x + vert [0]->x) / 2;
  midpoint2.y = (vert [3]->y + vert [0]->y) / 2;
  midpoint2.z = (vert [3]->z + vert [0]->z) / 2;

  mid_length = m_mine->CalcLength (&midpoint1, &midpoint2);

  ratio2 = mid_length/ave_length;

  return (min (ratio1,ratio2));
}

//--------------------------------------------------------------------------
// CalcDistance
//
// Action - calculates the distance from a line to a point
//--------------------------------------------------------------------------

double CDiagTool::CalcDistance (tFixVector *v1,tFixVector *v2,tFixVector *v3)
{
  dvector A,B,B2;
  double c,a2,distance;

  // normalize all points to vector 1
  A.x = v2->x - v1->x;
  A.y = v2->y - v1->y;
  A.z = v2->z - v1->z;
  B.x = v3->x - v1->x;
  B.y = v3->y - v1->y;
  B.z = v3->z - v1->z;

  // use formula from page 505 of "Calculase and Analytical Geometry" Fifth Addition
  // by Tommas/Finney, Addison-Wesley Publishing Company, June 1981
  //          B * A
  // B2 = B - ----- A
  //          A * A

  a2 = A.x*A.x + A.y*A.y + A.z*A.z;
  if (a2 != 0) {
    c = (B.x*A.x + B.y*A.y + B.z*A.z) / a2;
  } else {
    c = 0;
  }
  B2.x = B.x - c*A.x;
  B2.y = B.y - c*A.y;
  B2.z = B.z - c*A.z;

  distance = sqrt (B2.x*B2.x + B2.y*B2.y + B2.z*B2.z);
  return (distance);
}

//--------------------------------------------------------------------------
// CalcAngle ()
//--------------------------------------------------------------------------

double CDiagTool::CalcAngle (INT16 vert0,INT16 vert1,INT16 vert2,INT16 vert3) 
{
  dvector line1,line2,line3,orthog;
  double ratio;
  double dot_product, magnitude1, magnitude2,angle;
  tFixVector *v0 = m_mine->Vertices (vert0);
  tFixVector *v1 = m_mine->Vertices (vert1);
  tFixVector *v2 = m_mine->Vertices (vert2);
  tFixVector *v3 = m_mine->Vertices (vert3);
      // define lines
      line1.x = ((double) v1->x - (double) v0->x)/F1_0;
      line1.y = ((double) v1->y - (double) v0->y)/F1_0;
      line1.z = ((double) v1->z - (double) v0->z)/F1_0;
      line2.x = ((double) v2->x - (double) v0->x)/F1_0;
      line2.y = ((double) v2->y - (double) v0->y)/F1_0;
      line2.z = ((double) v2->z - (double) v0->z)/F1_0;
      line3.x = ((double) v3->x - (double) v0->x)/F1_0;
      line3.y = ((double) v3->y - (double) v0->y)/F1_0;
      line3.z = ((double) v3->z - (double) v0->z)/F1_0;
      // use cross product to calcluate orthogonal vector
      orthog.x = - (line1.y*line2.z - line1.z*line2.y);
      orthog.y = - (line1.z*line2.x - line1.x*line2.z);
      orthog.z = - (line1.x*line2.y - line1.y*line2.x);
      // use dot product to determine angle A dot B = |A|*|B|*cos (angle)
      // therfore: angle = acos (A dot B / |A|*|B|)
      dot_product = line3.x*orthog.x
		  + line3.y*orthog.y
		  + line3.z*orthog.z;
      magnitude1 = sqrt ( line3.x*line3.x
			+line3.y*line3.y
			+line3.z*line3.z);
      magnitude2 = sqrt ( orthog.x*orthog.x
			+orthog.y*orthog.y
			+orthog.z*orthog.z);
      if (dot_product == 0 || magnitude1 == 0 || magnitude2 == 0) {
        angle = (200.0 * M_PI)/180.0; 
      } else {
	ratio = dot_product/ (magnitude1*magnitude2);
	ratio = ( (double) ( (INT32) (ratio*1000.0))) / 1000.0; // bug fix 9/21/96
	if (ratio < -1.0 || ratio > (double)1.0) {
	  angle = (199.0 * M_PI)/180.0;
	} else {
	  angle = acos (ratio);
        }
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

INT32 CDiagTool::CheckId (CGameObject *objP) 
{
if (!GetMine ())
	return 1;

	INT32 type = objP->type;
	INT32 id = objP->id;

	switch (type) {
	case OBJ_ROBOT: /* an evil enemy */
		if (id < 0 || id >= (theApp.IsD1File () ? ROBOT_IDS1 : ROBOT_IDS2 (m_mine))) {
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
		if ((id < 0) || (id >= MAX_PLAYERS (m_mine))) {
			return 1;
		}
		break;

	case OBJ_POWERUP: /* a powerup you can pick up */
		if (id< 0 || id > MAX_POWERUP_IDS) {
			return 1;
		}
		break;

	case OBJ_CNTRLCEN: /* the control center */
		if (theApp.IsD1File ()) {
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
		objP->id = theApp.IsD1File () ? 1 : 2;
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
			objP->id = SMALLMINE_ID;
			return 2;
		}
	}
	return 0;
}

//--------------------------------------------------------------------------
// CDiagTool - WMTimer
//
// Action - Fills list box with data about the mine->  This
//          routine is called once after the dialog is opened.
//
//--------------------------------------------------------------------------

void CDiagTool::OnCheckMine ()
{
if (!GetMine ())
	return;

UpdateData (TRUE);
ClearBugList ();
m_bCheckMsgs = true;
if (m_bAutoFixBugs) {
	theApp.SetModified (TRUE);
	theApp.LockUndo ();
	}
  // set mode to BLOCK mode to make errors appear in red
theApp.MineView ()->SetSelectMode (BLOCK_MODE);

// now do actual checking
theApp.MainFrame ()->InitProgress (m_mine->SegCount () * 3 + 
											  m_mine->VertCount () +
											  m_mine->GameInfo ().walls.count * 2 +
											  m_mine->GameInfo ().triggers.count * 3 +
											  m_mine->GameInfo ().objects.count * 2 +
											  m_mine->NumObjTriggers ());
if (!CheckBotGens ())
	if (!CheckEquipGens ())
		if (!CheckSegments ())
			if (!CheckSegTypes ())
				if (!CheckWalls ())
					if (!CheckTriggers ())
						if (!CheckObjects ())
							CheckVertices ();
theApp.MainFrame ()->Progress ().DestroyWindow ();
LBBugs ()->SetCurSel (0);
if (m_bAutoFixBugs)
	theApp.UnlockUndo ();
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

bool CDiagTool::UpdateStats (char *szError, INT32 nErrorLevel, 
									  INT32 nSegment, INT32 nSide, INT32 linenum, INT32 pointnum, 
									  INT32 childnum, INT32 nWall, INT32 trignum, INT32 objnum)
{
if (!(szError && *szError))
	return false;
if (!(m_bShowWarnings || !strstr (szError, "WARNING:")))
	return true;
INT32 h = AddMessage (szError, -1, true);
if (h >= 0) {
	tBugPos *pbp = new tBugPos;
	if (!pbp)
		return false;
	pbp->nSegment = nSegment;
	pbp->nSide = nSide;
	pbp->nLine = linenum;
	pbp->nPoint = pointnum;
	pbp->nWall = nWall;
	pbp->nTrigger = trignum;
	pbp->nObject = objnum;
	pbp->nChild = childnum;
	LBBugs ()->SetItemDataPtr (h, (void *) pbp);
	}
*szError = '\0';
m_nErrors [nErrorLevel]++;
if (m_nErrors [0] < 1000 && m_nErrors [1] < 1000)
	return false;
LBBugs ()->AddString ("Maximum number of errors/warnings reached");
return true;
}

//--------------------------------------------------------------------------

bool CDiagTool::CheckSegTypes () 
{
if (!GetMine ())
	return false;

	INT16	i, nBotGens = 0, nEquipGens = 0, nFuelCens = 0;
	CDSegment	*segP = m_mine->Segments ();

for (i = m_mine->SegCount (); i; i--, segP++)
	switch (segP->function) {
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
if (nBotGens != m_mine->RobotMakerCount ()) {
	if (m_bAutoFixBugs) {
		sprintf_s (message, sizeof (message), "FIXED: Invalid robot maker count");
		m_mine->RobotMakerCount () = nBotGens;
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

bool CDiagTool::CheckSegments () 
{
if (!GetMine ())
	return false;

  INT16 nSegment,nSide,child,sidenum2,pointnum;
  INT16 vert0,vert1,vert2,vert3;
  INT16 i,j;
  double angle,flatness;
  INT16 match[4];
  CDSegment *segP = m_mine->Segments ();

  // check Segments ()
  //--------------------------------------------------------------
INT16 sub_errors = m_nErrors [0];
INT16 sub_warnings = m_nErrors [1];
LBBugs ()->AddString ("[Cubes]");

for (nSegment = 0; nSegment < m_mine->SegCount (); nSegment++, segP++) {
	theApp.MainFrame ()->Progress ().StepIt ();
// check geometry of segment
// 	Given that each point has 3 lines (called L1, L2, and L3),
//	and an orthogonal vector of L1 and L2 (called V1), the angle
//	between L3 and V1 must be less than PI/2.
//
	if (segP->function == SEGMENT_FUNC_ROBOTMAKER) {
		if ((segP->nMatCen >= m_mine->GameInfo ().botgen.count) || (m_mine->BotGens (segP->nMatCen)->nSegment != nSegment)) {
	 		sprintf_s (message, sizeof (message), "%s: Segment has invalid type (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
			if (m_bAutoFixBugs)
				m_mine->UndefineSegment (nSegment);
			}
		}
	if (segP->function == SEGMENT_FUNC_EQUIPMAKER) {
		if ((segP->nMatCen >= m_mine->GameInfo ().equipgen.count) || (m_mine->EquipGens (segP->nMatCen)->nSegment != nSegment)) {
	 		sprintf_s (message, sizeof (message), "%s: Segment has invalid type (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
			if (m_bAutoFixBugs)
				m_mine->UndefineSegment (nSegment);
			}
		}

	for (pointnum = 0; pointnum < 8; pointnum++) {
// define vert numbers
		vert0 = segP->verts[pointnum];
		vert1 = segP->verts[connect_points[pointnum][0]];
		vert2 = segP->verts[connect_points[pointnum][1]];
		vert3 = segP->verts[connect_points[pointnum][2]];
		angle = CalcAngle (vert0,vert1,vert2,vert3);
		angle = max (angle,CalcAngle (vert0,vert2,vert3,vert1));
		angle = max (angle,CalcAngle (vert0,vert3,vert1,vert2));
		if (angle > M_PI_2) {
			sprintf_s (message, sizeof (message), "WARNING: Illegal cube geometry (cube=%d,point=%d,angle=%d)",nSegment,pointnum, (INT32) ( (angle*180.0)/M_PI));
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
				sprintf_s (message, sizeof (message),"ERROR: Illegal cube geometry (cube=%d,side=%d,flatness=%d%%)",nSegment,nSide, (INT32) (flatness*100));
				if (UpdateStats (message, 1, nSegment, nSide))
					return true;
				break; // from for loop
				}
			} 
		}

// Check length of each line.
#if 0
	for (linenum=0;linenum<12;linenum++) {
		length = m_mine->CalcLength (m_mine->Vertices (segP->verts[line_vert[linenum][0]]),
											  m_mine->Vertices (segP->verts[line_vert[linenum][1]]));
		if (length < (double)F1_0) {
			sprintf_s (message, sizeof (message),"WARNING: Line length too INT16 (cube=%d,line=%d)",nSegment,linenum);
			if (UpdateStats (message, 0, nSegment, -1, linenum))
				return true;
			}
		}
#endif
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		child = m_mine->Segments (nSegment)->children[nSide];
// check child range 
		if (child != -1 && child != -2) {
			if (child < -2) {
				sprintf_s (message, sizeof (message),"ERROR: Illegal child number %d (cube=%d,side=%d)",child,nSegment,nSide);
				if (UpdateStats (message, 1, nSegment, nSide))
					return true;
				}
			else if (child >= m_mine->SegCount ()) {
				sprintf_s (message, sizeof (message),"ERROR: Child out of range %d (cube=%d,side=%d)",child,nSegment,nSide);
				if (UpdateStats (message, 1, nSegment, nSide)) 
					return true;
				}
			else {
			// make sure child segment has a child from this segment
			// and that it shares the same vertices as this segment
				for (sidenum2 = 0; sidenum2 < MAX_SIDES_PER_SEGMENT; sidenum2++) {
					if (m_mine->Segments (child)->children[sidenum2] == nSegment) 
						break;
					}					
				if (sidenum2 < MAX_SIDES_PER_SEGMENT) {
					memset (match, 0, sizeof (match));
					for (i = 0; i < 4; i++)
						for (j = 0; j < 4; j++)
							if (m_mine->Segments (nSegment)->verts[side_vert[nSide][i]] ==
								 m_mine->Segments (child)->verts[side_vert[sidenum2][j]])
								match[i]++;
					if (match[0]!=1 || match[1]!=1 || match[2]!=1 || match[3]!=1) {
						sprintf_s (message, sizeof (message),"WARNING:Child cube does not share points of cube (cube=%d,side=%d,child=%d)",nSegment,nSide,child);
						if (UpdateStats (message, 0, nSegment, -1, -1, -1, child))
							return true;
						}
					}
				else {
					sprintf_s (message, sizeof (message), "WARNING:Child cube does not connect to this cube (cube=%d,side=%d,child=%d)",nSegment,nSide,child);
					if (UpdateStats (message, 1, nSegment, -1, -1, -1, child))
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

bool CDiagTool::CheckAndFixPlayer (INT32 nMin, INT32 nMax, INT32 nObject, INT32* players)
{
INT32 id = m_mine->Objects (nObject)->id;
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
	m_mine->Objects (nObject)->id = (id < nMax) ? id : nMax - 1;
	players [m_mine->Objects (nObject)->id]++;
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
if (!GetMine ())
	return false;

	INT32 h,objectnum,type,id,count,players [16 + MAX_COOP_PLAYERS],nSegment,flags,corner, nPlayers [2], bFix;
  tFixVector center;
  double x,y,z,radius, max_radius,object_radius;
  CGameObject *objP = m_mine->Objects ();
	CGameObject *pPlayer = NULL;
  INT32	objCount = m_mine->GameInfo ().objects.count;
  CDSegment *segP;

INT16 sub_errors = m_nErrors [0];
INT16 sub_warnings = m_nErrors [1];
LBBugs ()->AddString ("[Objects]");
for (objectnum = 0;objectnum < objCount ; objectnum++, objP++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	// check segment range
	nSegment = objP->nSegment;
	if (nSegment < 0 || nSegment >= m_mine->SegCount ()) {
		if (m_bAutoFixBugs) {
			objP->nSegment = nSegment = 0;
			sprintf_s (message, sizeof (message),"FIXED: Bad segment number (object=%d,nSegment=%d)",objectnum,nSegment);
			}
		else
			sprintf_s (message, sizeof (message),"ERROR: Bad segment number (object=%d,nSegment=%d)",objectnum,nSegment);
		if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}

	if (nSegment < 0 || nSegment >= m_mine->SegCount ()) {
		if (m_bAutoFixBugs) {
			objP->nSegment = 0;
			sprintf_s (message, sizeof (message),"FIXED: Bad segment number (object=%d,nSegment=%d)",objectnum,nSegment);
			}
		else
			sprintf_s (message, sizeof (message),"ERROR: Bad segment number (object=%d,nSegment=%d)",objectnum,nSegment);
		if (UpdateStats (message, 1, -1, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}

	segP = m_mine->Segments (nSegment);
    // make sure object is within its cube
    // find center of segment then find maximum distance
	// of corner to center.  Calculate Objects () distance
    // from center and make sure it is less than max corner.
    center.x = center.y = center.z = 0;
    for (corner=0;corner<8;corner++) {
		 tFixVector *v = m_mine->Vertices (segP->verts[corner]);
      center.x += v->x;
      center.y += v->y;
      center.z += v->z;
    }
    center.x /= 8; center.y /= 8; center.z /= 8;
    max_radius = 0;
    for (corner=0;corner<8;corner++) {
		 tFixVector *v = m_mine->Vertices (segP->verts[corner]);
      x = v->x - center.x;
      y = v->y - center.y;
      z = v->z - center.z;
	  radius = sqrt (x*x + y*y + z*z);
	  max_radius = max (max_radius,radius);
    }
    x = objP->pos.x - center.x;
    y = objP->pos.y - center.y;
    z = objP->pos.z - center.z;
	object_radius = sqrt (x*x + y*y + z*z);
    if ((object_radius > max_radius) && (objP->type != OBJ_EFFECT)) {
      sprintf_s (message, sizeof (message),"ERROR: Object is outside of cube (object=%d,cube=%d)",objectnum,nSegment);
      if (UpdateStats (message, 1, nSegment, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
    }

    // check for non-zero flags (I don't know what these flags are for)
   flags = objP->flags;
	if (flags != 0) {
		if (m_bAutoFixBugs) {
			objP->flags = 0;
			sprintf_s (message, sizeof (message),"FIXED: Flags for object non-zero (object=%d,flags=%d)",objectnum,flags);
			}
		else
			sprintf_s (message, sizeof (message),"ERROR: Flags for object non-zero (object=%d,flags=%d)",objectnum,flags);
      if (UpdateStats (message, 1, nSegment, -1, -1, -1, -1, -1, -1, objectnum)) 
			return true;
    }

    // check type range
	 if ((objP->id < 0) || (objP->id > 255)) {
		 if (m_bAutoFixBugs) {
			sprintf_s (message, sizeof (message),"FIXED: Illegal object id (object=%d,id =%d)",objectnum, objP->id);
			objP->id = 0;
			}
		 else
			sprintf_s (message, sizeof (message),"WARNING: Illegal object id (object=%d,id =%d)",objectnum, objP->id);
		}
	type = objP->type;
    switch (type) {
	  case OBJ_PLAYER:
		  if (!pPlayer)
			  pPlayer = objP;
			if (objP->id >= MAX_PLAYERS (m_mine)) {
				if (m_bAutoFixBugs) {
					sprintf_s (message, sizeof (message),"FIXED: Illegal player id (object=%d,id =%d)",objectnum, objP->id);
				objP->id = MAX_PLAYERS (m_mine) - 1;
				}
			else
				sprintf_s (message, sizeof (message),"WARNING: Illegal player id (object=%d,id =%d)",objectnum, objP->id);
			}
	  case OBJ_COOP:
		  if (objP->id > 2) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),"FIXED: Illegal coop player id (object=%d,id =%d)",objectnum, objP->id);
				objP->id = 2;
				}
			else
				sprintf_s (message, sizeof (message),"WARNING: Illegal coop player id (object=%d,id =%d)",objectnum, objP->id);
			}
			break;
	  case OBJ_EFFECT:
		  if (objP->id > SOUND_ID) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),"FIXED: effect id (object=%d,id =%d)",objectnum, objP->id);
				objP->id = 2;
				}
			else
				sprintf_s (message, sizeof (message),"WARNING: Illegal effect id (object=%d,id =%d)",objectnum, objP->id);
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
			if (theApp.IsD2File ()) 
				break;
	  default:
		 if (m_bAutoFixBugs) {
			 m_mine->DeleteObject (objectnum);
			sprintf_s (message, sizeof (message),"FIXED: Illegal object type (object=%d,type=%d)",objectnum,type);
			}
		 else
			sprintf_s (message, sizeof (message),"WARNING: Illegal object type (object=%d,type=%d)",objectnum,type);
		if (UpdateStats (message,0, nSegment, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
    }
    id = objP->id;

    // check id range
    if (h = CheckId (objP)) {
		 if (h == 2)
	      sprintf_s (message, sizeof (message),"FIXED: Illegal object id (object=%d,id=%d)",objectnum,id);
		else
	      sprintf_s (message, sizeof (message),"WARNING: Illegal object id (object=%d,id=%d)",objectnum,id);
      if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, objectnum)) 
			return true;
    }

	// check contains count range
    count = objP->contains_count;
	if (count < -1) {
		if (m_bAutoFixBugs) {
			objP->contains_count = 0;
		  sprintf_s (message, sizeof (message),"FIXED: Spawn count must be >= -1 (object=%d,count=%d)",objectnum,count);
			}
		else
		  sprintf_s (message, sizeof (message),"WARNING: Spawn count must be >= -1 (object=%d,count=%d)",objectnum,count);
      if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, objectnum)) 
			return true;
    }

    // check container type range
	if (count > 0) {
      type = objP->contains_type;
	  if (type != OBJ_ROBOT && type != OBJ_POWERUP) {
		if (m_bAutoFixBugs) {
			objP->contains_type = OBJ_POWERUP;
			sprintf_s (message, sizeof (message),"FIXED: Illegal contained type (object=%d,contains=%d)",objectnum,type);
			}
		else
			sprintf_s (message, sizeof (message),"WARNING: Illegal contained type (object=%d,contains=%d)",objectnum,type);
	if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, objectnum)) return true;
	  }
	  id = objP->contains_id;
	  // check contains id range
	  if (CheckId (objP)) {
	sprintf_s (message, sizeof (message),"WARNING: Illegal contains id (object=%d,contains id=%d)",objectnum,id);
	if (UpdateStats (message,0,1)) return true;
	  }
	}
  }

  // make sure object 0 is player 0; if not fix it
if (m_mine->Objects (0)->type != OBJ_PLAYER || m_mine->Objects (0)->id != 0) {
	if (m_bAutoFixBugs) {
		CGameObject h;
		memcpy (&h, pPlayer, sizeof (CGameObject));
		memcpy (pPlayer, m_mine->Objects (0), sizeof (CGameObject));
		memcpy (m_mine->Objects (0), pPlayer, sizeof (CGameObject));
		strcpy_s (message, sizeof (message), "FIXED: Object 0 was not Player 0.");
		if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}
	else {
		strcpy_s (message, sizeof (message), "WARNING: Object 0 is not Player 0.");
		if (UpdateStats (message, 0, nSegment, -1, -1, -1, -1, -1, -1, objectnum))
			return true;
		}
	}

	// make sure there is the proper number of players and coops
	// reset count to zero
	memset (players, 0, sizeof (players));
	memset (nPlayers, 0, sizeof (nPlayers));
	bFix = 0;
	// count each
	objP = m_mine->Objects ();
	for (objectnum = 0; objectnum < objCount; objectnum++, objP++) {
		if (objP->type == OBJ_PLAYER) {
			nPlayers [0]++;
			if (CheckAndFixPlayer (0, MAX_PLAYERS (m_mine), objectnum, players))
				bFix |= 1;
			}
		else if (objP->type == OBJ_COOP) {
			nPlayers [1]++;
			if (CheckAndFixPlayer (0, 3, objectnum, players + MAX_PLAYERS (m_mine)))
				bFix |= 2;
			}
		}
// resequence player and coop ids
if (m_bAutoFixBugs) {
	INT32 i;
	if (bFix & 1) {
		for (i = id = 0; id < MAX_PLAYERS (m_mine); id++)
			if (players [id] != 0) 
				players [id] = ++i;
		}
	if (bFix & 2) {
		for (i = 0, id = MAX_PLAYERS (m_mine); id < MAX_PLAYERS (m_mine) + MAX_COOP_PLAYERS; id++)
			if (players [id] != 0) 
				players [id] = ++i;
		}
	objP = m_mine->Objects ();
	for (objectnum = 0; objectnum < objCount; objectnum++, objP++) {
		if (objP->type == OBJ_PLAYER) {
			if ((bFix & 1) && (objP->id >= 0) && (objP->id < MAX_PLAYERS (m_mine)))
				objP->id = players [objP->id] - 1;
			}
		else if (objP->type == OBJ_COOP) {
			if ((bFix & 2) && (objP->id >= MAX_PLAYERS (m_mine)) && (objP->id < MAX_PLAYERS (m_mine) + MAX_COOP_PLAYERS))
				objP->id = players [objP->id] - 1;
			}
		}
	}

for (id = 0; id < MAX_PLAYERS (m_mine); id++) {
	if (players [id] == 0) {
		sprintf_s (message, sizeof (message),"INFO: No player %d", id + 1);
		if (UpdateStats (message, 0)) 
			return true;
		}
	}
if (nPlayers [0] > MAX_PLAYERS (m_mine)) {
	sprintf_s (message, sizeof (message),"WARNING: too many players found (found %d, max. allowed %d)", nPlayers [0], MAX_PLAYERS (m_mine));
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
objP = m_mine->Objects ();
for (objectnum=0;objectnum<objCount;objectnum++, objP++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	type = objP->type;
	if (type == OBJ_CNTRLCEN) {
		if (m_mine->Segments (objP->nSegment)->function != SEGMENT_FUNC_CONTROLCEN) {
			if (m_bAutoFixBugs && m_mine->AddRobotMaker (objP->nSegment, false, false))
				sprintf_s (message, sizeof (message),"FIXED: Reactor belongs to a segment of wrong type (objP=%d, segP=%d)",objectnum,objP->nSegment);
			else
				sprintf_s (message, sizeof (message),"WARNING: Reactor belongs to a segment of wrong type (objP=%d, segP=%d)",objectnum,objP->nSegment);
			if (UpdateStats (message,0, nSegment, -1, -1, -1, -1, -1, -1, objectnum))
				return true;
			}
		count++;
		if (count > 1) {
			sprintf_s (message, sizeof (message),"WARNING: More than one Reactor found (object=%d)",objectnum);
			if (UpdateStats (message,0, nSegment, -1, -1, -1, -1, -1, -1, objectnum))
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
	if (!GetMine ())
		return false;

	INT32 count, trignum, deltrignum, nWall, i;
	INT32 nSegment, nSide, linknum;
	INT16 opp_segnum, opp_sidenum;

	INT16 sub_errors = m_nErrors [0];
	INT16 sub_warnings = m_nErrors [1];
	LBBugs ()->AddString ("[Triggers]");
	INT32 segCount = m_mine->SegCount ();
	INT32 trigCount = m_mine->GameInfo ().triggers.count;
	CTrigger *trigP = m_mine->Triggers ();
	INT32 wallCount = m_mine->GameInfo ().walls.count;
	CWall *wallP;
	CReactorTrigger *reactorTrigger = m_mine->ReactorTriggers ();

	// make sure trigP is linked to exactly one wallP
for (i = 0; i < reactorTrigger->count; i++)
	if ((reactorTrigger->Segment (i) >= segCount) ||
		(m_mine->Segments (reactorTrigger->Segment (i))->sides [reactorTrigger->Side (i)].nWall >= wallCount)) {
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
for (trignum = deltrignum = 0; trignum < trigCount; trignum++, trigP++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	count = 0;
	wallP = m_mine->Walls ();
	for (nWall = 0; nWall < wallCount; nWall++, wallP++) {
		if (wallP->nTrigger == trignum) {
			// if exit, make sure it is linked to CReactorTrigger
			INT32 tt = trigP->type;
			INT32 tf = trigP->flags;
			if (theApp.IsD1File () ? tf & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT) : tt == TT_EXIT) {
				for (i = 0; i < reactorTrigger->count; i++)
					if (*((CSideKey*) (reactorTrigger)) == *((CSideKey*) (wallP)))
						break; // found it
				// if did not find it
				if (i>=m_mine->ReactorTriggers ()->count) {
					if (m_bAutoFixBugs) {
						m_mine->AutoLinkExitToReactor ();
						sprintf_s (message, sizeof (message),"FIXED: Exit not linked to reactor (cube=%d, side=%d)", wallP->nSegment, wallP->nSide);
						}
					else
						sprintf_s (message, sizeof (message),"WARNING: Exit not linked to reactor (cube=%d, side=%d)", wallP->nSegment, wallP->nSide);
					if (UpdateStats (message,1,wallP->nSegment, wallP->nSide, -1, -1, -1, nWall))
						return true;
					}
				}
			count++;
			if (count >1) {
				sprintf_s (message, sizeof (message),"WARNING: Trigger belongs to more than one wallP (trig=%d, wallP=%d)",trignum,nWall);
				if (UpdateStats (message,0, wallP->nSegment, wallP->nSide, -1, -1, -1, nWall)) return true;
			}
		}
	}
	if (count < 1) {
		if (m_bAutoFixBugs) {
			m_mine->DeleteTrigger (trignum);
			trignum--;
			trigP--;
			trigCount--;
			sprintf_s (message, sizeof (message),"FIXED: Unused trigP (trigP=%d)",trignum + deltrignum);
			deltrignum++;
			}
		else
			sprintf_s (message, sizeof (message),"WARNING: Unused trigP (trigP=%d)",trignum);
		if (UpdateStats (message,0,1)) return true;
		}
	}

INT16 trigSeg, trigSide;
trigP = m_mine->Triggers ();
for (trignum = 0; trignum < trigCount; trignum++, trigP++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	nWall = m_mine->FindTriggerWall (trignum);
	if (nWall < wallCount) {
		wallP = m_mine->Walls (nWall);
		trigSeg = wallP->nSegment;
		trigSide = wallP->nSide;
		}
	else
		trigSeg = trigSide = -1;
	// check number of links of trigP (only for
	INT32 tt = trigP->type;
	INT32 tf = trigP->flags;
	if (trigP->count == 0) {
		if (theApp.IsD1File ()
			 ? tf & (TRIGGER_CONTROL_DOORS | TRIGGER_ON | TRIGGER_ONE_SHOT | TRIGGER_MATCEN | TRIGGER_ILLUSION_OFF | TRIGGER_ILLUSION_ON) 
			 : (tt != TT_EXIT) && (tt != TT_SECRET_EXIT) && (tt != TT_MESSAGE) && (tt != TT_SOUND) && 
			   (tt != TT_SPEEDBOOST) && (tt != TT_SHIELD_DAMAGE_D2) && (tt != TT_ENERGY_DRAIN_D2)
			) {
			sprintf_s (message, sizeof (message),"WARNING: Trigger has no targets (trigP=%d)",trignum);
			if (UpdateStats (message,0, -1, -1, -1, -1, -1, -1, trignum))
				return true;
			}
		}
	else {
		// check range of links
		for (linknum = 0; linknum < trigP->count; linknum++) {
			if (linknum >= MAX_TRIGGER_TARGETS) {
				if (m_bAutoFixBugs) {
					trigP->count = MAX_TRIGGER_TARGETS;
					sprintf_s (message, sizeof (message),"FIXED: Trigger has too many targets (trigP=%d, number of links=%d)",trignum,linknum);
					}
				else
					sprintf_s (message, sizeof (message),"WARNING: Trigger has too many targets (trigP=%d, number of links=%d)",trignum,linknum);
				if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) 
					return true;
				break;
				}
			// check segment range
			nSegment = trigP->Segment (linknum);
			nSide = trigP->Side (linknum);
			if ((nSegment < 0) || ((nSide < 0) ? (nSegment >= m_mine->ObjectCount ()) : (nSegment >= m_mine->SegCount ()))) {
				if (m_bAutoFixBugs) {
					if (m_mine->DeleteTargetFromTrigger (trigP, linknum))
						linknum--;
					else { // => trigP deleted
						linknum = MAX_TRIGGER_TARGETS;	// take care of the loops
						trigP--;
						}
					sprintf_s (message, sizeof (message),"FIXED: Trigger points to non-existant %s (trigP=%d, cube=%d)", 
								  (nSide < 0) ? "object" : "segment", trignum, nSegment);
					}
				else
					sprintf_s (message, sizeof (message),"ERROR: Trigger points to non-existant %s (trigP=%d, cube=%d)", 
								  (nSide < 0) ? "object" : "segment", trignum, nSegment);
				if (UpdateStats (message,1, trigSeg, trigSide, -1, -1, -1, -1, trignum)) 
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
						sprintf_s (message, sizeof (message),"FIXED: Trigger points to non-existant side (trigP=%d, side=%d)",trignum,nSide);
						}
					else
						sprintf_s (message, sizeof (message),"ERROR: Trigger points to non-existant side (trigP=%d, side=%d)",trignum,nSide);
					if (UpdateStats (message, 1, trigSeg, trigSide, -1, -1, -1, -1, trignum)) 
						return true;
				} else {
					CDSegment *segP = m_mine->Segments (nSegment);
					// check door opening trigP
//						if (trigP->flags == TRIGGER_CONTROL_DOORS) {
					if (theApp.IsD1File ()
						 ? tf & TRIGGER_CONTROL_DOORS 
						 : tt==TT_OPEN_DOOR || tt==TT_CLOSE_DOOR || tt==TT_LOCK_DOOR || tt==TT_UNLOCK_DOOR) {
						// make sure trigP points to a wallP if it controls doors
						if (segP->sides[nSide].nWall >= wallCount) {
							if (m_bAutoFixBugs) {
								if (m_mine->DeleteTargetFromTrigger (trigP, linknum))
									linknum--;
								else {
									linknum = MAX_TRIGGER_TARGETS;
									trigP--;
									}
								sprintf_s (message, sizeof (message),"FIXED: Trigger does not target a door (trigP=%d, link= (%d,%d))",trignum,nSegment,nSide);
								}
							else
								sprintf_s (message, sizeof (message),"WARNING: Trigger does not target a door (trigP=%d, link= (%d,%d))",trignum,nSegment,nSide);
							if (UpdateStats (message, 0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
						}

						// make sure oposite segment/side has a wallP too
						if (!m_mine->GetOppositeSide (opp_segnum, opp_sidenum, nSegment, nSide)) {
							sprintf_s (message, sizeof (message),"WARNING: Trigger opens a single sided door (trigP=%d, link= (%d,%d))",trignum,nSegment,nSide);
							if (UpdateStats (message, 0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
							}
						else {
							if (m_mine->Segments (opp_segnum)->sides [opp_sidenum].nWall >= wallCount) {
								sprintf_s (message, sizeof (message),"WARNING: Trigger opens a single sided door (trigP=%d, link= (%d,%d))",trignum,nSegment,nSide);
								if (UpdateStats (message,1, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
								}
							}
						}
					else if (theApp.IsD1File () 
								? tf & (TRIGGER_ILLUSION_OFF | TRIGGER_ILLUSION_ON) 
								: tt == TT_ILLUSION_OFF || tt == TT_ILLUSION_ON || tt == TT_OPEN_WALL || tt == TT_CLOSE_WALL || tt == TT_ILLUSORY_WALL
							  ) {
						// make sure trigP points to a wallP if it controls doors
						if (segP->sides [nSide].nWall >= wallCount) {
							if (m_bAutoFixBugs) {
								if (m_mine->DeleteTargetFromTrigger (trigP, linknum))
									linknum--;
								else {
									linknum = MAX_TRIGGER_TARGETS;
									trigP--;
									}
								sprintf_s (message, sizeof (message),"FIXED: Trigger target does not exist (trigP=%d, link= (%d,%d))",trignum,nSegment,nSide);
								}
							else
								sprintf_s (message, sizeof (message),"ERROR: Trigger target does not exist (trigP=%d, link= (%d,%d))",trignum,nSegment,nSide);
							if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
							}
						}
//						if (trigP->flags == TRIGGER_MATCEN) {
					else if (theApp.IsD1File () ? tf & TRIGGER_MATCEN : tt == TT_MATCEN) {
						if ((segP->function != SEGMENT_FUNC_ROBOTMAKER) && (segP->function != SEGMENT_FUNC_EQUIPMAKER)) {
							sprintf_s (message, sizeof (message),"WARNING: Trigger does not target a robot or equipment maker (trigP=%d, link= (%d,%d))",trignum,nSegment,nSide);
							if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
							}
						}
					}
				}
			}
		}
	}

// make sure there is exactly one exit and its linked to the CReactorTrigger
count = 0;
trigP = m_mine->Triggers ();
for (trignum = 0; trignum < trigCount; trignum++, trigP++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	nWall = m_mine->FindTriggerWall (trignum);
	if (nWall < wallCount) {
		wallP = m_mine->Walls (nWall);
		trigSeg = wallP->nSegment;
		trigSide = wallP->nSide;
		}
	else
		trigSeg = trigSide = -1;
	INT32 tt = trigP->type;
	INT32 tf = trigP->flags;
	if (theApp.IsD1File () ? tf & TRIGGER_EXIT : tt == TT_EXIT) {
		count++;
		if (count >1) {
			sprintf_s (message, sizeof (message),"WARNING: More than one exit found (trig=%d)",trignum);
			if (UpdateStats (message,0, trigSeg, trigSide, -1, -1, -1, -1, trignum)) return true;
			}
		}
	}

trigCount = m_mine->NumObjTriggers ();
for (trignum = 0; trignum < trigCount; trignum++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	trigP = m_mine->ObjTriggers (trignum);
	if ((trigP->type != TT_MESSAGE) && (trigP->type != TT_SOUND) && (trigP->type != TT_COUNTDOWN) && !trigP->count) {
		sprintf_s (message, sizeof (message), "ERROR: Object trigP has no targets (trigP=%d, object=%d))", trignum, trigP->nObject);
		if (UpdateStats (message,0, trignum, trigP->nObject, -1, -1, -1, -1, trignum)) return true;
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

INT8 CDiagTool::FindMatCen (CRobotMaker* matCenP, INT16 nSegment, INT16* refList)
{
	INT8	h = -1, i, j = INT8 (m_mine->GameInfo ().botgen.count);

if (refList) {
	for (i = 0; i < j; i++) {
		if (refList [i] >= 0) {
			if (matCenP [i].objFlags [0] || matCenP [i].objFlags [1])
				return i;
			h = i;
			}
		}
	}
else {
	for (i = 0; i < j; i++) {
		if (matCenP [i].nSegment == nSegment)
			return i;
		}
	}
return h;
}

//------------------------------------------------------------------------

void CDiagTool::CountMatCenRefs (INT32 nSpecialType, INT16* refList, CRobotMaker* matCenP, INT16 nMatCens)
{
	CDSegment*		segP = m_mine->Segments ();
	INT16				n, h, i, j = m_mine->SegCount ();

memset (refList, 0, sizeof (*refList) * MAX_NUM_MATCENS2);
for (h = i = 0; i < j; i++, segP++) {
	if (segP->function == UINT8 (nSpecialType)) {
		n = segP->nMatCen;
		if ((n >= 0) && (n < nMatCens) && (refList [n] >= 0)) {
			if (matCenP [n].nSegment == i)
				refList [n] = -1;
			else
				refList [n]++;
			}
		}
	}
}

//------------------------------------------------------------------------

INT16 CDiagTool::FixMatCens (INT32 nSpecialType, INT16* segList, INT16* refList, CRobotMaker* matCenP, INT16 nMatCens, char* pszType)
{
	CDSegment*	segP = m_mine->Segments ();
	INT16			h, i, j = m_mine->SegCount ();
	INT8			n;

for (h = i = 0; i < j; i++, segP++) {
	if (segP->function != UINT8 (nSpecialType))
		continue;
	n = segP->nMatCen;
	if ((n < 0) || (n >= nMatCens)) {
		sprintf_s (message, sizeof (message), "%s: %s maker list corrupted (segment=%d)", m_bAutoFixBugs ? "FIXED" : "ERROR", pszType, i);
		if (m_bAutoFixBugs)
			n = segP->nMatCen = INT8 (FindMatCen (matCenP, i));
		}
	else if (matCenP [n].nSegment != i) {
		sprintf_s (message, sizeof (message), "%s: %s maker list corrupted (segment=%d)", m_bAutoFixBugs ? "FIXED" : "ERROR", pszType, i);
		if (m_bAutoFixBugs) {
			n = INT8 (FindMatCen (matCenP, i));
			if (n >= 0) {
				segP->nMatCen = n;
				refList [n] = -1;
				}
			else {
				n = segP->nMatCen;
				if (refList [n] >= 0) {
					matCenP [n].nSegment = i;
					refList [n] = -1;
					}
				else
					segP->nMatCen = -1;
				}
			}
		}
	if (h < MAX_NUM_MATCENS2)
		segList [h++] = i;
	else {
		sprintf_s (message, sizeof (message), "%s: Too many %s makers", m_bAutoFixBugs ? "FIXED" : "ERROR", pszType, i);
		if (m_bAutoFixBugs) 
			m_mine->UndefineSegment (i);
		}
	}
return h;
}

//------------------------------------------------------------------------

INT16 CDiagTool::AssignMatCens (INT32 nSpecialType, INT16* segList, INT16* refList, CRobotMaker* matCenP, INT16 nMatCens)
{
if (!m_bAutoFixBugs)
	return nMatCens;

	CDSegment*	segP = m_mine->Segments ();
	INT16			h, i, j = m_mine->SegCount ();
	INT8			n;

for (h = i = 0; i < j; i++, segP++) {
	if (segP->function != UINT8 (nSpecialType))
		continue;
	n = segP->nMatCen;
	if (n >= 0)
		continue;
	n = FindMatCen (matCenP, i, refList);
	if (n >= 0) {
		segP->nMatCen = n;
		matCenP [n].nSegment = i;
		refList [n] = -1;
		}
	else if (m_bAutoFixBugs)
		m_mine->UndefineSegment (i);
	}
return h;
}

//------------------------------------------------------------------------

INT16 CDiagTool::CleanupMatCens (INT16* refList, CRobotMaker* matCenP, INT16 nMatCens)
{
if (!m_bAutoFixBugs)
	return nMatCens;

	CDSegment*	segP = m_mine->Segments ();
	
for (INT32 i = 0; i < nMatCens; i) {
	if (refList [i] < 0) 
		i++;
	else {
		if (i < --nMatCens) {
			matCenP [i] = matCenP [nMatCens];
			matCenP [i].nFuelCen =
			segP [matCenP [i].nSegment].nMatCen = i;
			refList [i] = refList [nMatCens];
			}
		}
	}
return nMatCens;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckBotGens (void)
{
	INT16					h = m_mine->SegCount (), i, nSegment = 0;
	bool					bOk = true;
	INT16					nMatCenSegs, nMatCens = INT16 (m_mine->GameInfo ().botgen.count);
	CDSegment*			segP = m_mine->Segments ();
	CRobotMaker*		matCenP = m_mine->BotGens (0);
	INT16					segList [MAX_NUM_MATCENS2];
	INT16					refList [MAX_NUM_MATCENS2];

for (i = 0; i < nMatCens; i++)
	matCenP [i].nFuelCen = i;
CountMatCenRefs (SEGMENT_FUNC_ROBOTMAKER, refList, matCenP, nMatCens);
nMatCenSegs = FixMatCens (SEGMENT_FUNC_ROBOTMAKER, segList, refList, matCenP, nMatCens, "Robot");
AssignMatCens (SEGMENT_FUNC_ROBOTMAKER, segList, refList, matCenP, nMatCens);
m_mine->GameInfo ().botgen.count = CleanupMatCens (refList, matCenP, nMatCens);
if (!bOk) {
	sprintf_s (message, sizeof (message), "%s: Robot maker list corrupted (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
	if (UpdateStats (message, 0)) return true;
	}
return false;
}

//------------------------------------------------------------------------

bool CDiagTool::CheckEquipGens (void)
{
	INT16					i, nSegment = 0;
	bool					bOk = true;
	INT32					nMatCenSegs, nMatCens = INT32 (m_mine->GameInfo ().equipgen.count);
	CRobotMaker*		matCenP = m_mine->EquipGens (0);
	INT16					segList [MAX_NUM_MATCENS2];
	INT16					refList [MAX_NUM_MATCENS2];

for (i = 0; i < nMatCens; i++)
	matCenP [i].nFuelCen = i;
CountMatCenRefs (SEGMENT_FUNC_EQUIPMAKER, refList, matCenP, nMatCens);
nMatCenSegs = FixMatCens (SEGMENT_FUNC_EQUIPMAKER, segList, refList, matCenP, nMatCens, "Equipment");
AssignMatCens (SEGMENT_FUNC_EQUIPMAKER, segList, refList, matCenP, nMatCens);
m_mine->GameInfo ().equipgen.count = CleanupMatCens (refList, matCenP, nMatCens);
if (!bOk) {
	sprintf_s (message, sizeof (message), "%s: Equipment maker list corrupted (segment=%d))", m_bAutoFixBugs ? "FIXED" : "ERROR", nSegment);
	if (UpdateStats (message, 0)) return true;
	}
return false;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CWall *CDiagTool::OppWall (UINT16 nSegment, UINT16 nSide)
{
	INT16	oppSegnum, oppSidenum, nWall;

if (!m_mine->GetOppositeSide (oppSegnum, oppSidenum, nSegment, nSide))
	return NULL;
nWall = m_mine->Segments (oppSegnum)->sides [oppSidenum].nWall;
if ((nWall < 0) || (nWall > MAX_WALLS (m_mine)))
	return NULL;
return m_mine->Walls (nWall);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CDiagTool::CheckWalls () 
{
	if (!GetMine ())
		return false;
	INT16 nSegment,nSide;
	UINT16 nWall, wallCount = m_mine->GameInfo ().walls.count, 
			 maxWalls = MAX_WALLS (m_mine);
	CDSegment *segP;
	CDSide *side;
	CWall *wall = m_mine->Walls (), *w, *ow;
	INT32 segCount = m_mine->SegCount ();
	UINT8 wallFixed [MAX_WALLS2];

	INT16 sub_errors = m_nErrors [0];
	INT16 sub_warnings = m_nErrors [1];
	LBBugs ()->AddString ("[Walls]");

memset (wallFixed, 0, sizeof (wallFixed));
*message = '\0';
for (nSegment = 0, segP = m_mine->Segments (); nSegment < segCount; nSegment++, segP++) {
	for (nSide = 0, side = segP->sides; nSide < 6; nSide++, side++) {
		nWall = side->nWall;
		if ((nWall < 0) || (nWall >= wallCount) || (nWall >= maxWalls)) {
			if (nWall != NO_WALL (m_mine))
				side->nWall = NO_WALL (m_mine);
			continue;
			}
		w = m_mine->Walls (nWall);
		if (w->nSegment != nSegment) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),
							"FIXED: Wall sits in wrong cube (cube=%d, wall=%d, parent=%d)",
							nSegment, nWall, w->nSegment);
				if (wallFixed [nWall])
					side->nWall = NO_WALL (m_mine);
				else {
					if (m_mine->Segments (w->nSegment)->sides [w->nSide].nWall == nWall)
						side->nWall = NO_WALL (m_mine);
					else {
						w->nSegment = nSegment;
						w->nSide = nSide;
						}
					wallFixed [nWall] = 1;
					}
				}
			else
				sprintf_s (message, sizeof (message),
							"ERROR: Wall sits in wrong cube (cube=%d, wall=%d, parent=%d)",
							nSegment, nWall, w->nSegment);
			if (UpdateStats (message,1, nSegment, nSide, -1, -1, -1, side->nWall)) return true;
			} 
		else if (w->nSide != nSide) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),
							"FIXED: Wall sits at wrong side (cube=%d, side=%d, wall=%d, parent=%d)",
							nSegment, nSide, nWall, w->nSegment);
				if (wallFixed [nWall])
					side->nWall = NO_WALL (m_mine);
				else {
					ow = OppWall (nSegment, nSide);
					if (ow && (ow->type == w->type)) {
						segP->sides [w->nSide].nWall = NO_WALL (m_mine);
						w->nSide = nSide;
						}
					else if (segP->sides [w->nSide].nWall == nWall)
						side->nWall = NO_WALL (m_mine);
					else
						w->nSide = nSide;
					wallFixed [nWall] = 1;
					}
				}
			else
				sprintf_s (message, sizeof (message),
							"ERROR: Wall sits at wrong side (cube=%d, side=%d, wall=%d, parent=%d)",
							nSegment, nSide, nWall, w->nSegment);
			if (UpdateStats (message,1, -1, -1, -1, -1, -1, side->nWall)) return true;
			}
		} 
	}
for (nWall = 0; nWall < wallCount; nWall++, wall++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	// check wall range type
	if (wall->type > (theApp.IsD1File () ? WALL_CLOSED : m_mine->IsStdLevel () ? WALL_CLOAKED : WALL_TRANSPARENT)) {
		sprintf_s (message, sizeof (message),
					"ERROR: Wall type out of range (wall=%d, type=%d)",
					nWall,wall->type);
		if (UpdateStats (message,1,wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
		}
		// check range of segment number that the wall points to
	if (wall->nSegment >= m_mine->SegCount ()) {
		sprintf_s (message, sizeof (message),
					"ERROR: Wall sits in non-existant cube (wall=%d, cube=%d)",
					nWall,wall->nSegment);
		if (UpdateStats (message,1,-1, -1, -1, -1, -1, nWall)) return true;
		} 
	else if (wall->nSide >= 6) {
		// check range of side number that the wall points to
		sprintf_s (message, sizeof (message),
					"ERROR: Wall sits on side which is out of range (wall=%d, side=%d)",
					nWall,wall->nSide);
		if (UpdateStats (message,1,-1 -1, -1, -1, -1, nWall)) return true;
		}
	else {
		// check to make sure segment points back to wall
		side = m_mine->Segments (wall->nSegment)->sides + wall->nSide;
		if (side->nWall != nWall) {
			w = m_mine->Walls (nWall);
			if ((nWall < wallCount) && (w->nSegment == wall->nSegment) && (w->nSide == wall->nSide)) {
				if (m_bAutoFixBugs) {
					sprintf_s (message, sizeof (message),
								"FIXED: Duplicate wall found (wall=%d, cube=%d)", nWall, wall->nSegment);
					m_mine->DeleteWall (nWall);
					nWall--;
					wall--;
					wallCount--;
					continue;
					}
				else 
					sprintf_s (message, sizeof (message),
								"ERROR: Duplicate wall found (wall=%d, cube=%d)", nWall, wall->nSegment);
				if (UpdateStats (message, 1, wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
				}
			else {
				if (m_bAutoFixBugs) {
					side->nWall = nWall;
					sprintf_s (message, sizeof (message),
								"FIXED: Cube does not reference wall which sits in it (wall=%d, cube=%d)",
								nWall,wall->nSegment);
					}
				else 
					sprintf_s (message, sizeof (message),
								"ERROR: Cube does not reference wall which sits in it (wall=%d, cube=%d)",
								nWall,wall->nSegment);
				if (UpdateStats (message,1,wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
				}
			}
			// make sure trigger number of wall is in range
		if ((wall->nTrigger != NO_TRIGGER) && (wall->nTrigger >= m_mine->GameInfo ().triggers.count)) {
			if (m_bAutoFixBugs) {
				sprintf_s (message, sizeof (message),
							"FIXED: Wall has invalid trigger (wall=%d, trigger=%d)",
							nWall, wall->nTrigger);
				wall->nTrigger = NO_TRIGGER;
				}
			else
				sprintf_s (message, sizeof (message),
							"ERROR: Wall has invalid trigger (wall=%d, trigger=%d)",
							nWall, wall->nTrigger);
			if (UpdateStats (message,1,wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
			}
#if 1 // linked walls not supported in DLE-XP and D2X-XL
		if (wall->linkedWall != -1) {
			INT16 invLinkedWall = wall->linkedWall;
			if (m_bAutoFixBugs) {
				wall->linkedWall = -1;
				sprintf_s (message, sizeof (message),
							  "FIXED: Wall has invalid linked wall (wall=%d, linked wall=%d [%d])",
							  nWall, invLinkedWall, wall->linkedWall);
				}
			else
				sprintf_s (message, sizeof (message),
							  "ERROR: Wall has invalid linked wall (wall=%d, linked wall=%d [%d])",
							  nWall, invLinkedWall, wall->linkedWall);
			}
#else
		if ((wall->linkedWall < -1) || (wall->linkedWall >= wallCount)) {
			if (m_bAutoFixBugs) {
				INT16	oppSeg, oppSide, invLinkedWall = wall->linkedWall;
				if (m_mine->GetOppositeSide (oppSeg, oppSide, wall->nSegment, wall->nSide)) {
					wall->linkedWall = m_mine->Segments (oppSeg)->sides [oppSide].nWall;
					if ((wall->linkedWall < -1) || (wall->linkedWall >= wallCount))
						wall->linkedWall = -1;
					sprintf_s (message, sizeof (message),
						"FIXED: Wall has invalid linked wall (wall=%d, linked wall=%d [%d])",
						nWall, invLinkedWall, wall->linkedWall);
					}
				}
			else
				sprintf_s (message, sizeof (message),
					"ERROR: Wall has invalid linked wall (wall=%d, linked wall=%d)",
					nWall,wall->linkedWall);
			}
		else if (wall->linkedWall >= 0) {
			INT16	oppSeg, oppSide;
			if (m_mine->GetOppositeSide (oppSeg, oppSide, wall->nSegment, wall->nSide)) {
				INT16 oppWall = m_mine->Segments (oppSeg)->sides [oppSide].nWall;
				if ((oppWall < 0) || (oppWall >= wallCount)) {
					sprintf_s (message, sizeof (message),
						"%s: Wall links to non-existant wall (wall=%d, linked side=%d,%d)",
						m_bAutoFixBugs ? "FIXED" : "ERROR",
						nWall, m_mine->Walls (wall->linkedWall)->nSegment, m_mine->Walls (wall->linkedWall)->nSide);
						if (m_bAutoFixBugs)
							wall->linkedWall = -1;
					}
				else if (wall->linkedWall != oppWall) {
					sprintf_s (message, sizeof (message),
						"%s: Wall links to wrong opposite wall (wall=%d, linked side=%d,%d)",
						m_bAutoFixBugs ? "FIXED" : "ERROR",
						nWall, m_mine->Walls (wall->linkedWall)->nSegment, m_mine->Walls (wall->linkedWall)->nSide);
						if (m_bAutoFixBugs)
							wall->linkedWall = oppWall;
					}
				}
			else {
				sprintf_s (message, sizeof (message),
					"%s: Wall links to non-existant side (wall=%d, linked side=%d,%d)",
					m_bAutoFixBugs ? "FIXED" : "ERROR",
					nWall, m_mine->Walls (wall->linkedWall)->nSegment, m_mine->Walls (wall->linkedWall)->nSide);
				if (m_bAutoFixBugs)
					wall->linkedWall = -1;
				}
			}
#endif
		if (UpdateStats (message, 1, wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
			// check wall nClip
		if ((wall->type == WALL_CLOAKED) && (wall->cloak_value > 31)) {
			if (m_bAutoFixBugs) {
				wall->cloak_value = 31;
				sprintf_s (message, sizeof (message), "FIXED: Wall has invalid cloak value (wall=%d)", nWall);
					}
			else
				sprintf_s (message, sizeof (message), "ERROR: Wall has invalid cloak value (wall=%d)", nWall);
			}
		if ((wall->type == WALL_BLASTABLE || wall->type == WALL_DOOR) &&
			 (   wall->nClip < 0
			  || wall->nClip == 2
//			     || wall->nClip == 7
			  || wall->nClip == 8
			  || (theApp.IsD1File () && wall->nClip > 25)
			  || (theApp.IsD2File () && wall->nClip > 50))) {
			sprintf_s (message, sizeof (message),
						"ERROR: Illegal wall clip number (wall=%d, clip number=%d)",
						nWall,wall->nClip);
			if (UpdateStats (message,1,wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
			}
			// Make sure there is a child to the segment
		if (wall->type != WALL_OVERLAY) {
			if (!(m_mine->Segments (wall->nSegment)->child_bitmask & (1<< wall->nSide))) {
				sprintf_s (message, sizeof (message),
							"ERROR: No adjacent cube for this door (wall=%d, cube=%d)",
							nWall,wall->nSegment);
				if (UpdateStats (message,1,wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
				}
			else {
				nSegment = m_mine->Segments (wall->nSegment)->children[wall->nSide];
				CDSegment *segP = m_mine->Segments (nSegment);
				if ((nSegment >= 0 && nSegment < m_mine->SegCount ()) &&
					 (wall->type == WALL_DOOR || wall->type == WALL_ILLUSION)) {
					// find segment's child side
					for (nSide=0;nSide<6;nSide++)
						if (segP->children[nSide] == wall->nSegment)
							break;
					if (nSide != 6) {  // if child's side found
						if (segP->sides[nSide].nWall >= m_mine->GameInfo ().walls.count) {
							sprintf_s (message, sizeof (message),
										"WARNING: No matching wall for this wall (wall=%d, cube=%d)", 
										nWall,nSegment);
							if (UpdateStats (message,0,wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
							} 
						else {
							UINT16 wallnum2 = segP->sides[nSide].nWall;
							if ((wallnum2 < wallCount) &&
								 ((wall->nClip != m_mine->Walls (wallnum2)->nClip
									|| wall->type != m_mine->Walls (wallnum2)->type))) {
								sprintf_s (message, sizeof (message),
											"WARNING: Matching wall for this wall is of different type or clip no. (wall=%d, cube=%d)",
											nWall,nSegment);
								if (UpdateStats (message,0,wall->nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
								}
							}
						}
					}
				}
			}
		}
	}

	// make sure segP's wall points back to the segment
segP = m_mine->Segments ();
for (nSegment=0;nSegment<segCount;nSegment++, segP++) {
	theApp.MainFrame ()->Progress ().StepIt ();
	side = segP->sides;
	for (nSide = 0; nSide < 6; nSide++, side++) {
		if (side->nWall <	wallCount) {
			nWall = side->nWall;
			if (nWall >= wallCount) {
				if (m_bAutoFixBugs) {
					side->nWall = wallCount;
					sprintf_s (message, sizeof (message),"FIXED: Cube has an invalid wall number (wall=%d, cube=%d)",nWall,nSegment);
					}
				else
					sprintf_s (message, sizeof (message),"ERROR: Cube has an invalid wall number (wall=%d, cube=%d)",nWall,nSegment);
				if (UpdateStats (message,1, nSegment, nSide)) return true;
			} else {
				if (m_mine->Walls (nWall)->nSegment != nSegment) {
					if (m_bAutoFixBugs) {
						m_mine->Walls (nWall)->nSegment = nSegment;
						sprintf_s (message, sizeof (message),"FIXED: Cube's wall does not sit in cube (wall=%d, cube=%d)",nWall,nSegment);
						}
					else
						sprintf_s (message, sizeof (message),"ERROR: Cube's wall does not sit in cube (wall=%d, cube=%d)",nWall,nSegment);
					if (UpdateStats (message,1,nSegment, wall->nSide, -1, -1, -1, nWall)) return true;
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

bool CDiagTool::CheckVertices () 
{
	if (!GetMine ())
		return false;
//  bool found;
  INT32 nSegment,vertnum,point;
  INT32 nUnused = 0;

  INT16 sub_errors = m_nErrors [0];
  INT16 sub_warnings = m_nErrors [1];
  LBBugs ()->AddString ("[Misc]");

  UINT8 *vStat = m_mine->VertStatus ();

for (vertnum= m_mine->VertCount (); vertnum; vertnum--, vStat++)
	*vStat &= ~NEW_MASK;

// mark all used verts
CDSegment *segP = m_mine->Segments ();
for (nSegment = m_mine->SegCount (); nSegment; nSegment--, segP++)
	for (point = 0; point < 8; point++)
		*m_mine->VertStatus (segP->verts [point]) |= NEW_MASK;
vertnum = m_mine->VertCount () - 1;
for (vStat = m_mine->VertStatus (vertnum); vertnum >= 0; vertnum--, vStat--) {
	theApp.MainFrame ()->Progress ().StepIt ();
	if (!(*vStat & NEW_MASK)) {
		nUnused++;
		if (m_bAutoFixBugs) {
			if (vertnum < --m_mine->VertCount ()) {
				memcpy (m_mine->Vertices (vertnum), m_mine->Vertices (vertnum + 1), (m_mine->VertCount () - vertnum) * sizeof (*m_mine->Vertices ()));
				memcpy (m_mine->VertStatus (vertnum), m_mine->VertStatus (vertnum + 1), (m_mine->VertCount () - vertnum) * sizeof (*m_mine->VertStatus ()));
				}
			CDSegment *segP = m_mine->Segments ();
			for (nSegment = m_mine->SegCount (); nSegment; nSegment--, segP++)
				for (point = 0; point < 8; point++)
					if (segP->verts [point] >= vertnum)
						segP->verts [point]--;
			}
		}
	}
vStat = m_mine->VertStatus ();
for (vertnum= m_mine->VertCount (); vertnum; vertnum--, vStat++)
	*vStat &= ~NEW_MASK;
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