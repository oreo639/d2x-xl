// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "toolview.h"
#include "io.h"
#include "hogmanager.h"

#define CURRENT_POINT(a) ((theMine->Current ()->nPoint + (a))&0x03)

char *BLOCKOP_HINT =
	"The block of cubes will be saved relative to the current cube.\n"
	"Later, when you paste the block, it will be placed relative to\n"
	"the current cube at that time.  You can change the current side\n"
	"and the current point to affect the relative direction and\n"
	"rotation of the block.\n"
	"\n"
	"Would you like to proceed?";

//---------------------------------------------------------------------------
// ReadSegmentInfo()
//
// ACTION - Reads a segment's information in text form from a file.  Adds
//          new vertices if non-identical one does not exist.  Aborts if
//	    MAX_VERTICES is hit.
//
// Change - Now reads verts relative to current side
//---------------------------------------------------------------------------

INT16 CMine::ReadSegmentInfo (FILE *fBlk) 
{
	CSegment		*segP;
	CSide			*sideP;
#if 0
	CGameObject			*objP;
	INT16				objnum, segObjCount;
#endif
	INT16				nSegment, nSide, nVertex;
	INT16				i, j, test;
	INT16				origVertCount, k;
	CFixVector		origin, vVertex;
	CDoubleVector	xPrime, yPrime, zPrime, v;
	CDoubleVector	xAxis, yAxis, zAxis;
	INT16				nNewSegs = 0, nNewWalls = 0, nNewTriggers = 0, nNewObjects = 0;
	INT16				xlatSegNum [MAX_SEGMENTS3];
	INT32				byteBuf;

// remember number of vertices for later
origVertCount = VertCount ();

// set origin
segP = CurrSeg ();
nSide = Current ()->nSide;
nVertex = segP->m_info.verts [sideVertTable [nSide][CURRENT_POINT(0)]];
memcpy (&origin, Vertices (nVertex), sizeof (CVertex));
/*
origin.x = Vertices (nVertex)->x;
origin.y = Vertices (nVertex)->y;
origin.z = Vertices (nVertex)->z;
*/
// set x'
nVertex = segP->m_info.verts [sideVertTable [nSide][CURRENT_POINT(1)]];
xPrime = *Vertices (nVertex) - origin;

// calculate y'
nVertex = segP->m_info.verts [sideVertTable [nSide][CURRENT_POINT(3)]];
vVertex = *Vertices (nVertex) - origin;
yPrime = CrossProduct (xPrime, CDoubleVector (vVertex));
zPrime = CrossProduct (xPrime, yPrime);
xPrime.Normalize ();
yPrime.Normalize ();
zPrime.Normalize ();

// now take the determinant
xAxis.Set (yPrime.v.y * zPrime.v.z - zPrime.v.y * yPrime.v.z, 
			  zPrime.v.y * xPrime.v.z - xPrime.v.y * zPrime.v.z, 
			  xPrime.v.y * yPrime.v.z - yPrime.v.y * xPrime.v.z);
yAxis.Set (zPrime.v.x * yPrime.v.z - yPrime.v.x * zPrime.v.z, 
			  xPrime.v.x * zPrime.v.z - zPrime.v.x * xPrime.v.z,
			  yPrime.v.x * xPrime.v.z - xPrime.v.x * yPrime.v.z);
zAxis.Set (yPrime.v.x * zPrime.v.y - zPrime.v.x * yPrime.v.y,
			  zPrime.v.x * xPrime.v.y - xPrime.v.x * zPrime.v.y,
			  xPrime.v.x * yPrime.v.y - yPrime.v.x * xPrime.v.y);

#if 0
  sprintf_s (message, sizeof (message), "x'=(%0.3f,%0.3f,%0.3f)\n"
		  "y'=(%0.3f,%0.3f,%0.3f)\n"
		  "z'=(%0.3f,%0.3f,%0.3f)\n",
		  (float)xPrime.x,(float)xPrime.y,(float)xPrime.z,
		  (float)yPrime.x,(float)yPrime.y,(float)yPrime.z,
		  (float)zPrime.x,(float)zPrime.y,(float)zPrime.z);
  DebugMsg(message);
  sprintf_s (message, sizeof (message), "x''=(%0.3f,%0.3f,%0.3f)\n"
		  "y''=(%0.3f,%0.3f,%0.3f)\n"
		  "z''=(%0.3f,%0.3f,%0.3f)\n",
		  (float)xAxis.x,(float)xAxis.y,(float)xAxis.z,
		  (float)yAxis.x,(float)yAxis.y,(float)yAxis.z,
		  (float)zAxis.x,(float)zAxis.y,(float)zAxis.z);
  DebugMsg(message);
#endif

nNewSegs = 0;
memset (xlatSegNum, 0xff, sizeof (xlatSegNum));
while(!feof(fBlk)) {
	if (SegCount () >= MAX_SEGMENTS) {
		ErrorMsg ("No more free segments");
		return (nNewSegs);
		}
// abort if there are not at least 8 vertices free
	if (MAX_VERTICES - VertCount () < 8) {
		ErrorMsg ("No more free vertices");
		return(nNewSegs);
		}
	nSegment = SegCount ();
	segP = Segments (nSegment);
	segP->m_info.owner = -1;
	segP->m_info.group = -1;
	fscanf_s (fBlk, "segment %hd\n", &segP->m_info.nIndex);
	xlatSegNum [segP->m_info.nIndex] = nSegment;
	// invert segment number so its children can be children can be fixed later
	segP->m_info.nIndex = ~segP->m_info.nIndex;

	// read in side information 
	sideP = segP->m_sides;
	for (INT32 nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
		fscanf_s (fBlk, "  side %hd\n", &test);
		if (test != nSide) {
			ErrorMsg ("Invalid side number read");
			return (0);
			}
		sideP->m_info.nWall = NO_WALL;
		fscanf_s (fBlk, "    tmap_num %hd\n",&sideP->m_info.nBaseTex);
		fscanf_s (fBlk, "    tmap_num2 %hd\n",&sideP->m_info.nOvlTex);
		for (j = 0; j < 4; j++)
			fscanf_s (fBlk, "    uvls %hd %hd %hd\n",
						&sideP->m_info.uvls [j].u,
						&sideP->m_info.uvls [j].v,
						&sideP->m_info.uvls [j].l);
		if (bExtBlkFmt) {
			fscanf_s (fBlk, "    nWall %d\n",&byteBuf);
			sideP->m_info.nWall = (UINT16) byteBuf;
			if (sideP->m_info.nWall != NO_WALL) {
				CWall w;
				CTrigger t;
				memset (&w, 0, sizeof (w));
				memset (&t, 0, sizeof (t));
				fscanf_s (fBlk, "        segment %ld\n", &w.m_nSegment);
				fscanf_s (fBlk, "        side %ld\n", &w.m_nSide);
				fscanf_s (fBlk, "        hps %ld\n", &w.m_info.hps);
				fscanf_s (fBlk, "        type %d\n", &byteBuf);
				w.m_info.type = byteBuf;
				fscanf_s (fBlk, "        flags %d\n", &byteBuf);
				w.m_info.flags = byteBuf;
				fscanf_s (fBlk, "        state %d\n", &byteBuf);
				w.m_info.state = byteBuf;
				fscanf_s (fBlk, "        nClip %d\n", &byteBuf);
				w.m_info.nClip = byteBuf;
				fscanf_s (fBlk, "        keys %d\n", &byteBuf);
				w.m_info.keys = byteBuf;
				fscanf_s (fBlk, "        cloak %d\n", &byteBuf);
				w.m_info.cloakValue = byteBuf;
				fscanf_s (fBlk, "        trigger %d\n", &byteBuf);
				w.m_info.nTrigger = byteBuf;
				if ((w.m_info.nTrigger >= 0) && (w.m_info.nTrigger < MAX_TRIGGERS)) {
					fscanf_s (fBlk, "			    type %d\n", &byteBuf);
					t.m_info.type = byteBuf;
					fscanf_s (fBlk, "			    flags %hd\n", &t.m_info.flags);
					fscanf_s (fBlk, "			    value %ld\n", &t.m_info.value);
					fscanf_s (fBlk, "			    timer %d\n", &t.m_info.time);
					fscanf_s (fBlk, "			    count %hd\n", &t.m_count);
					INT32 iTarget;
					for (iTarget = 0; iTarget < t.m_count; iTarget++) {
						fscanf_s (fBlk, "			        segP %hd\n", &t [iTarget].m_nSegment);
						fscanf_s (fBlk, "			        side %hd\n", &t [iTarget].m_nSide);
						}
					}
				if (GameInfo ().walls.count < MAX_WALLS) {
					if ((w.m_info.nTrigger >= 0) && (w.m_info.nTrigger < MAX_TRIGGERS)) {
						if (GameInfo ().triggers.count >= MAX_TRIGGERS)
							w.m_info.nTrigger = NO_TRIGGER;
						else {
							w.m_info.nTrigger = GameInfo ().triggers.count++;
							++nNewTriggers;
							*Triggers (w.m_info.nTrigger) = t;
							}
						}
					nNewWalls++;
					sideP->m_info.nWall = GameInfo ().walls.count++;
					w.m_nSegment = nSegment;
					*Walls (sideP->m_info.nWall) = w;
					}
				}
#if 0
			fscanf_s (fBlk, "    object_num %hd\n",&segObjCount);
			while (segObjCount) {
				CGameObject o;
				memset (&o, 0, sizeof (o));
				fscanf_s (fBlk, "            signature %hd\n", &o.signature);
				fscanf_s (fBlk, "            type %d\n", &byteBuf);
				o.type = (INT8) byteBuf;
				fscanf_s (fBlk, "            id %d\n", &byteBuf);
				o.id = (INT8) byteBuf;
				fscanf_s (fBlk, "            controlType %d\n", &byteBuf);
				o.controlType = (UINT8) byteBuf;
				fscanf_s (fBlk, "            movementType %d\n", &byteBuf);
				o.movementType = (UINT8) byteBuf;
				fscanf_s (fBlk, "            renderType %d\n", &byteBuf);
				o.renderType = (UINT8) byteBuf;
				fscanf_s (fBlk, "            flags %d\n", &byteBuf);
				o.flags = (UINT8) byteBuf;
				o.nSegment = nSegment;
				fscanf_s (fBlk, "            pos %ld %ld %ld\n", &o.pos.x, &o.pos.y, &o.pos.z);
				memcpy (&o.lastPos, &o.pos, sizeof (o.pos));
				fscanf_s (fBlk, "            orient %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", 
													&o.orient.rVec.x, &o.orient.rVec.y, &o.orient.rVec.z,
													&o.orient.uVec.x, &o.orient.uVec.y, &o.orient.uVec.z,
													&o.orient.fVec.x, &o.orient.fVec.y, &o.orient.fVec.z);
				fscanf_s (fBlk, "            nSegment %hd\n", &o.nSegment);
				fscanf_s (fBlk, "            size %ld\n", &o.size);
				fscanf_s (fBlk, "            shields %ld\n", &o.shields);
				fscanf_s (fBlk, "            contents.type %d\n", &byteBuf);
				o.contents.type = (INT8) byteBuf;
				fscanf_s (fBlk, "            contents.id %d\n", &byteBuf);
				o.contents.id = (INT8) byteBuf;
				fscanf_s (fBlk, "            contents.count %d\n", &byteBuf);
				o.contents.count = (INT8) byteBuf;
				switch (o.type) {
					case OBJ_POWERUP:
					case OBJ_HOSTAGE:
						// has vclip
						break;
					case OBJ_PLAYER:
					case OBJ_COOP:
					case OBJ_ROBOT:
					case OBJ_WEAPON:
					case OBJ_CNTRLCEN:
						// has poly model;
						break;
					}
				switch (o.controlType) {
					case :
					}
				switch (o.movementType) {
					case MT_PHYSICS:
						fscanf_s (fBlk, "            velocity %ld %ld %ld\n", 
								  &o.physInfo.velocity.x, &o.physInfo.velocity.y, &o.physInfo.velocity.z);
						fscanf_s (fBlk, "            thrust %ld %ld %ld\n", 
								  &o.physInfo.thrust.x, &o.physInfo.thrust.y, &o.physInfo.thrust.z);
						fscanf_s (fBlk, "            mass %ld\n", &o.physInfo.mass);
						fscanf_s (fBlk, "            drag %ld\n", &o.physInfo.drag);
						fscanf_s (fBlk, "            brakes %ld\n", &o.physInfo.brakes);
						fscanf_s (fBlk, "            rotvel %ld %ld %ld\n", 
								  &o.physInfo.rotvel.x, &o.physInfo.rotvel.y, &o.physInfo.rotvel.z);
						fscanf_s (fBlk, "            rotthrust %ld %ld %ld\n", 
								  &o.physInfo.rotthrust.x, &o.physInfo.rotthrust.y, &o.physInfo.rotthrust.z);
						fscanf_s (fBlk, "            turnroll %hd\n", &o.physInfo.turnroll);
						fscanf_s (fBlk, "            flags %hd\n", &o.physInfo.flags);
						break;
					case MT_SPIN:
						fscanf_s (fBlk, "            spinrate %ld %ld %ld\n", 
								  &o.spin_rate.x, &o.spin_rate.y, &o.spin_rate.z);
						break;
					}
				switch (o.renderType) {
					case :
						break;
					}
				}
#endif
			}
		}
	fscanf_s (fBlk, "  children %hd %hd %hd %hd %hd %hd\n",
				&segP->m_info.children [0],&segP->m_info.children [1],&segP->m_info.children [2],
				&segP->m_info.children [3],&segP->m_info.children [4],&segP->m_info.children [5]);
	// read in vertices
	for (i = 0; i < 8; i++) {
		fscanf_s (fBlk, "  CFixVector %hd %ld %ld %ld\n", &test, &vVertex.v.x, &vVertex.v.y, &vVertex.v.z);
		if (test != i) {
			ErrorMsg ("Invalid vertex number read");
			return (0);
			}
		// each vertex relative to the origin has a x', y', and z' component
		// adjust vertices relative to origin
		v.Set (vVertex ^ CFixVector (xAxis), vVertex ^ CFixVector (xAxis), vVertex ^ CFixVector (xAxis));
		v += origin;
		// add a new vertex
		// if this is the same as another vertex, then use that vertex number instead
		CVertex* vertP = Vertices (origVertCount);
		for (k = origVertCount; k < VertCount (); k++, vertP++)
			if (*vertP == v) {
				segP->m_info.verts [i] = k;
				break;
				}
		// else make a new vertex
		if (k == VertCount ()) {
			nVertex = VertCount ();
			VertStatus (nVertex) |= NEW_MASK;
			segP->m_info.verts [i] = nVertex;
			*Vertices (nVertex) = CFixVector (v);
			VertCount ()++;
			}
		}
	// mark vertices
	for (i = 0; i < 8; i++)
		VertStatus (segP->m_info.verts [i]) |= MARKED_MASK;
	fscanf_s (fBlk, "  staticLight %ld\n", &segP->m_info.staticLight);
	if (bExtBlkFmt) {
		fscanf_s (fBlk, "  special %d\n", &byteBuf);
		segP->m_info.function = byteBuf;
		fscanf_s (fBlk, "  nMatCen %d\n", &byteBuf);
		segP->m_info.nMatCen = byteBuf;
		fscanf_s (fBlk, "  value %d\n", &byteBuf);
		segP->m_info.value = byteBuf;
		fscanf_s (fBlk, "  childFlags %d\n", &byteBuf);
		segP->m_info.childFlags = byteBuf;
		fscanf_s (fBlk, "  wallFlags %d\n", &byteBuf);
		segP->m_info.wallFlags = byteBuf;
		switch (segP->m_info.function) {
			case SEGMENT_FUNC_FUELCEN:
				if (!AddFuelCenter (nSegment, SEGMENT_FUNC_FUELCEN, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_REPAIRCEN:
				if (!AddFuelCenter (nSegment, SEGMENT_FUNC_REPAIRCEN, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_ROBOTMAKER:
				if (!AddRobotMaker (nSegment, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_EQUIPMAKER:
				if (!AddEquipMaker (nSegment, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_CONTROLCEN:
				if (!AddReactor (nSegment, false, false))
					segP->m_info.function = 0;
				break;
			default:
				break;
			}
		}
	else {
		segP->m_info.function = 0;
		segP->m_info.nMatCen = -1;
		segP->m_info.value = -1;
		}
	//        fscanf_s (fBlk, "  childFlags %d\n",&test);
	//        segP->m_info.childFlags = test & 0x3f;
	//        fscanf_s (fBlk, "  wallFlags %d\n",&test);
	//        segP->m_info.wallFlags  = (test & 0x3f) | MARKED_MASK;
	segP->m_info.wallFlags = MARKED_MASK; // no other bits
	// calculate childFlags
	segP->m_info.childFlags = 0;
	for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
		if (segP->m_info.children [i] >= 0)
		segP->m_info.childFlags |= (1 << i);
	SegCount ()++;
	nNewSegs++;
	}

CTrigger *trigger = Triggers (GameInfo ().triggers.count);
for (i = nNewTriggers; i; i--) {
	trigger--;
	for (j = 0; j < trigger->m_count; j++) {
		if (trigger->Segment (j) >= 0)
			trigger->Segment (j) = xlatSegNum [trigger->Segment (j)];
		else if (trigger->m_count == 1) {
			DeleteTrigger (INT16 (trigger - Triggers (0)));
			i--;
			}
		else {
			trigger->Delete (j);
			}
		}
	}

sprintf_s (message, sizeof (message),
			" Block tool: %d blocks, %d walls, %d triggers pasted.", 
			nNewSegs, nNewWalls, nNewTriggers);
DEBUGMSG (message);
return (nNewSegs);
}

//---------------------------------------------------------------------------
// dump_seg_info()
//
// ACTION - Writes a segment's information in text form to a fBlk.  Uses
//          actual coordinate information instead of vertex number.  Only
//          saves segment information (no Walls (), Objects (), or Triggers ()).
//
// Change - Now saves verts relative to the current side (point 0)
//          Uses x',y',z' axis where:
//          y' is in the neg dirction of line0
//          z' is in the neg direction orthogonal to line0 & line3
//          x' is in the direction orghogonal to x' and y'
//
//---------------------------------------------------------------------------

void CMine::WriteSegmentInfo (FILE *fBlk, INT16 /*nSegment*/) 
{
	INT16				nSegment;
	CSegment			*segP;
	CSide				*sideP;
	CWall				*wallP;
	INT16				i,j;
	CFixVector		origin;
	CDoubleVector	xPrime,yPrime,zPrime,vVertex;
	INT16				nVertex;

#if DEMO
ErrorMsg ("You cannot save a mine in the demo.");
return;
#endif
// set origin
segP = CurrSeg ();
nVertex = segP->m_info.verts[sideVertTable[Current ()->nSide][CURRENT_POINT(0)]];
origin = *Vertices (nVertex);

// set x'
nVertex = segP->m_info.verts[sideVertTable[Current ()->nSide][CURRENT_POINT(1)]];
xPrime = *Vertices (nVertex) - origin;

// calculate y'
nVertex = segP->m_info.verts[sideVertTable[Current ()->nSide][CURRENT_POINT(3)]];
vVertex = *Vertices (nVertex) - origin;
yPrime = CrossProduct (xPrime, vVertex);
zPrime = CrossProduct (xPrime, yPrime);
xPrime.Normalize ();
yPrime.Normalize ();
zPrime.Normalize ();

#if 0
  sprintf_s (message, sizeof (message), "x'=(%0.3f,%0.3f,%0.3f)\n"
		  "y'=(%0.3f,%0.3f,%0.3f)\n"
		  "z'=(%0.3f,%0.3f,%0.3f)\n",
		  (float)xPrime.x,(float)xPrime.y,(float)xPrime.z,
		  (float)yPrime.x,(float)yPrime.y,(float)yPrime.z,
		  (float)zPrime.x,(float)zPrime.y,(float)zPrime.z);
  DebugMsg(message);
#endif

segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
	if (segP->m_info.wallFlags & MARKED_MASK) {
		fprintf (fBlk, "segment %d\n",nSegment);
		sideP = segP->m_sides;
		for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++, sideP++) {
			fprintf (fBlk, "  side %d\n",i);
			fprintf (fBlk, "    tmap_num %d\n",sideP->m_info.nBaseTex);
			fprintf (fBlk, "    tmap_num2 %d\n",sideP->m_info.nOvlTex);
			for (j = 0; j < 4; j++) {
				fprintf (fBlk, "    uvls %d %d %d\n",
				sideP->m_info.uvls [j].u,
				sideP->m_info.uvls [j].v,
				sideP->m_info.uvls [j].l);
				}
			if (bExtBlkFmt) {
				fprintf (fBlk, "    nWall %d\n", 
							(sideP->m_info.nWall < GameInfo ().walls.count) ? sideP->m_info.nWall : NO_WALL);
				if (sideP->m_info.nWall < GameInfo ().walls.count) {
					wallP = Walls (sideP->m_info.nWall);
					fprintf (fBlk, "        segment %d\n", wallP->m_nSegment);
					fprintf (fBlk, "        side %d\n", wallP->m_nSide);
					fprintf (fBlk, "        hps %d\n", wallP->m_info.hps);
					fprintf (fBlk, "        type %d\n", wallP->m_info.type);
					fprintf (fBlk, "        flags %d\n", wallP->m_info.flags);
					fprintf (fBlk, "        state %d\n", wallP->m_info.state);
					fprintf (fBlk, "        nClip %d\n", wallP->m_info.nClip);
					fprintf (fBlk, "        keys %d\n", wallP->m_info.keys);
					fprintf (fBlk, "        cloak %d\n", wallP->m_info.cloakValue);
					if ((wallP->m_info.nTrigger < 0) || (wallP->m_info.nTrigger >= GameInfo ().triggers.count))
						fprintf (fBlk, "        trigger %u\n", NO_TRIGGER);
					else {
						CTrigger *trigger = Triggers (wallP->m_info.nTrigger);
						INT32 iTarget;
						INT32 count = 0;
						// count trigger targets in marked area
						for (iTarget = 0; iTarget < trigger->m_count; iTarget++)
							if (Segments (trigger->Segment (iTarget))->m_info.wallFlags & MARKED_MASK)
								count++;
#if 0
						if (trigger->m_count && !count)	// no targets in marked area
							fprintf (fBlk, "        trigger %d\n", MAX_TRIGGERS);
						else 
#endif
							{
							fprintf (fBlk, "        trigger %d\n", wallP->m_info.nTrigger);
							fprintf (fBlk, "			    type %d\n", trigger->m_info.type);
							fprintf (fBlk, "			    flags %ld\n", trigger->m_info.flags);
							fprintf (fBlk, "			    value %ld\n", trigger->m_info.value);
							fprintf (fBlk, "			    timer %d\n", trigger->m_info.time);
							fprintf (fBlk, "			    count %d\n", count);
							for (iTarget = 0; iTarget < trigger->m_count; iTarget++)
								if (Segments (trigger->Segment (iTarget))->m_info.wallFlags & MARKED_MASK) {
									fprintf (fBlk, "			        segP %d\n", trigger->Segment (iTarget));
									fprintf (fBlk, "			        side %d\n", trigger->Side (iTarget));
									}
							}
						}
					}
				}
			}
		fprintf (fBlk, "  children");
		for (i = 0; i < 6; i++)
			fprintf (fBlk, " %d", segP->m_info.children [i]);
		fprintf (fBlk, "\n");
		// save vertices
		for (i = 0; i < 8; i++) {
			// each vertex relative to the origin has a x', y', and z' component
			// which is a constant (k) times the axis
			// k = (B*A)/(A*A) where B is the vertex relative to the origin
			//                       A is the axis unit vVertexor (always 1)
			nVertex = segP->m_info.verts [i];
			vVertex = *Vertices (nVertex) - origin;
			fprintf (fBlk, "  CFixVector %d %ld %ld %ld\n",i,
									D2X (vVertex ^ xPrime),
									D2X (vVertex ^ yPrime),
									D2X (vVertex ^ zPrime));
			}
		fprintf (fBlk, "  staticLight %ld\n",segP->m_info.staticLight);
		if (bExtBlkFmt) {
			fprintf (fBlk, "  special %d\n",segP->m_info.function);
			fprintf (fBlk, "  nMatCen %d\n",segP->m_info.nMatCen);
			fprintf (fBlk, "  value %d\n",segP->m_info.value);
			fprintf (fBlk, "  childFlags %d\n",segP->m_info.childFlags);
			fprintf (fBlk, "  wallFlags %d\n",segP->m_info.wallFlags);
			}
		}
	}
}

//==========================================================================
// MENU - Cut
//==========================================================================

void CMine::CutBlock()
{
  FILE *fBlk;
  INT16 nSegment;
  INT16 count;
  char szFile [256] = "\0";

if (m_bSplineActive) {
	ErrorMsg (spline_error_message);
	return;
	}

  // make sure some cubes are marked
count = MarkedSegmentCount ();
if (count==0) {
	ErrorMsg ("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

//  if (disable_saves) {
//    ErrorMsg ("Saves disabled, contact Interplay for your security number.");
//    return;
//  }

if (!bExpertMode && Query2Msg(BLOCKOP_HINT,MB_YESNO) != IDYES)
	return;
#if 1
if (!BrowseForFile (FALSE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  theApp.MainFrame ()))
	return;
#else
  // Initialize data for fBlk open dialog
  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof (OPENFILENAME));
  ofn.lStructSize = sizeof (OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "DMB Block File\0*.blk\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.lpstrDefExt = "blk";
  ofn.nMaxFile = sizeof (szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
  if (!GetSaveFileName(&ofn)) {
	return;
  }
  strcpy (szFile, ofn.lpstrFile);
#endif
_strlwr_s (szFile, sizeof (szFile));
bExtBlkFmt = strstr (szFile, ".blx") != NULL;
fopen_s (&fBlk, szFile, "w");
if (!fBlk) {
	ErrorMsg ("Unable to open block file");
	return;
	}
//UpdateUndoBuffer(0);
strcpy_s (m_szBlockFile, sizeof (m_szBlockFile), szFile); // remember file for quick paste
fprintf (fBlk, bExtBlkFmt ? "DMB_EXT_BLOCK_FILE\n" : "DMB_BLOCK_FILE\n");
WriteSegmentInfo (fBlk, 0);
// delete Segments () from last to first because SegCount ()
// is effected for each deletion.  When all Segments () are marked
// the SegCount () will be decremented for each nSegment in loop.
theApp.SetModified (TRUE);
theApp.LockUndo ();
CSegment *segP = Segments (0) + SegCount ();
for (nSegment = SegCount () - 1; nSegment; nSegment--)
    if ((--segP)->m_info.wallFlags & MARKED_MASK) {
		if (SegCount () <= 1)
			break;
		DeleteSegment (nSegment); // delete segP w/o asking "are you sure"
		}
theApp.UnlockUndo ();
fclose(fBlk);
sprintf_s (message, sizeof (message), " Block tool: %d blocks cut to '%s' relative to current side.", count, szFile);
DEBUGMSG (message);
  // wrap back then forward to make sure segment is valid
wrap(&Current1 ().nSegment,-1,0,SegCount () - 1);
wrap(&Current2 ().nSegment,1,0,SegCount () - 1);
wrap(&Current2 ().nSegment,-1,0,SegCount () - 1);
wrap(&Current2 ().nSegment,1,0,SegCount () - 1);
SetLinesToDraw ();
theApp.MineView ()->Refresh ();
}

//==========================================================================
// MENU - Copy
//==========================================================================

void CMine::CopyBlock(char *pszBlockFile)
{
  FILE *fBlk;
  char szFile [256] = "\0";
  INT16 count;

  // make sure some cubes are marked
count = MarkedSegmentCount ();
if (count==0) {
	ErrorMsg ("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

//  if (disable_saves) {
//    ErrorMsg ("Saves disabled, contact Interplay for your security number.");
//    return;
//  }

if (!bExpertMode && Query2Msg(BLOCKOP_HINT,MB_YESNO) != IDYES)
	return;
#if 1
if (pszBlockFile && *pszBlockFile)
	strcpy_s (szFile, sizeof (szFile), pszBlockFile);
else {
	strcpy_s (szFile, sizeof (szFile), m_szBlockFile);
	if (!BrowseForFile (FALSE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  theApp.MainFrame ()))
		return;
	}
#else
  // Initialize data for fBlk open dialog
  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof (OPENFILENAME));
  ofn.lStructSize = sizeof (OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "DLE-XP Block File\0*.blk\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.lpstrDefExt = "blk";
  ofn.nMaxFile = sizeof (szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

  if (!GetSaveFileName(&ofn)) {
    return;
  strcpy (szFile, ofn.lpstrFile);
  }
#endif
_strlwr_s (szFile, sizeof (szFile));
bExtBlkFmt = strstr (szFile, ".blx") != NULL;
fopen_s (&fBlk, szFile, "w");
if (!fBlk) {
	sprintf_s (message, sizeof (message), "Unable to open block file '%s'", szFile);
	ErrorMsg (message);
	return;
	}
//  UpdateUndoBuffer(0);
strcpy_s (m_szBlockFile, sizeof (m_szBlockFile), szFile); // remember fBlk for quick paste
fprintf (fBlk, bExtBlkFmt ? "DMB_EXT_BLOCK_FILE\n" : "DMB_BLOCK_FILE\n");
WriteSegmentInfo (fBlk, 0);
fclose (fBlk);
sprintf_s (message, sizeof (message), " Block tool: %d blocks copied to '%s' relative to current side.", count, szFile);
DEBUGMSG (message);
SetLinesToDraw ();
theApp.MineView ()->Refresh ();
}

//==========================================================================
// MENU - Paste
//==========================================================================

void CMine::PasteBlock() 
{
if (m_bSplineActive) {
	ErrorMsg (spline_error_message);
	return;
	}
// Initialize data for fBlk open dialog
  char szFile [256] = "\0";

#if 1
if (!BrowseForFile (TRUE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
						  theApp.MainFrame ()))
	return;
#else
  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof (OPENFILENAME));
  ofn.lStructSize = sizeof (OPENFILENAME);
  ofn.hwndOwner = HWindow;
  ofn.lpstrFilter = "DLE-XP Block File\0*.blk\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szFile;
  ofn.nMaxFile = sizeof (szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (!GetOpenFileName(&ofn)) {
    return;
  }
#endif
//  UpdateUndoBuffer(0);

if (!ReadBlock (szFile, 0))
	theApp.MineView ()->SetSelectMode (BLOCK_MODE);
}

//==========================================================================
// read_block_file()
//
// returns 0 on success
// if option == 1, then "x blocks pasted" message is suppressed
//==========================================================================

INT32 CMine::ReadBlock (char *pszBlockFile,INT32 option) 
{
	CSegment *segP,*seg2;
	INT16 nSegment,seg_offset;
	INT16 count,child;
	INT16 nVertex;
	FILE *fBlk;

_strlwr_s (pszBlockFile, 256);
fopen_s (&fBlk, pszBlockFile, "r");
if (!fBlk) {
	ErrorMsg ("Unable to open block file");
	return 1;
	}	

fscanf_s (fBlk, "%s\n", &message, sizeof (message));
if (!strncmp (message, "DMB_BLOCK_FILE", 14))
	bExtBlkFmt = false;
else if (!strncmp (message, "DMB_EXT_BLOCK_FILE", 18))
	bExtBlkFmt = true;
else {
	ErrorMsg ("This is not a block file.");
	fclose (fBlk);
	return 2;
	}

strcpy_s (m_szBlockFile, sizeof (m_szBlockFile), pszBlockFile); // remember file for quick paste

// unmark all Segments ()
// set up all seg_numbers (makes sure there are no negative seg_numbers)
theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
segP = Segments (0);
for (nSegment = 0;nSegment < MAX_SEGMENTS; nSegment++, segP++) {
	segP->m_info.nIndex = nSegment;
	segP->m_info.wallFlags &= ~MARKED_MASK;
	}

// unmark all vertices
for (nVertex = 0; nVertex < MAX_VERTICES; nVertex++) {
	VertStatus (nVertex) &= ~MARKED_MASK;
	VertStatus (nVertex) &= ~NEW_MASK;
	}
count = ReadSegmentInfo (fBlk);

// fix up the new Segments () children
segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
	if (segP->m_info.nIndex < 0) {  // if segment was just inserted
		// if child has a segment number that was just inserted, set it to the
		//  segment's offset number, otherwise set it to -1
		for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
			if (segP->m_info.childFlags & (1 << child)) {
				seg2 = Segments (0);
				for (seg_offset = 0; seg_offset < SegCount (); seg_offset++, seg2++) {
					if (segP->m_info.children [child] == ~seg2->m_info.nIndex) {
						segP->m_info.children [child] = seg_offset;
						break;
						}
					}
				if (seg_offset == SegCount ()) { // no child found
					ResetSide (nSegment,child);
					// auto link the new segment with any touching Segments ()
					seg2 = Segments (0);
					INT32 segnum2, sidenum2;
					for (segnum2 = 0; segnum2 < SegCount (); segnum2++, seg2++) {
						if (nSegment != segnum2) {
							// first check to see if Segments () are any where near each other
							// use x, y, and z coordinate of first point of each segment for comparison
							CFixVector* v1 = Vertices (segP ->m_info.verts [0]);
							CFixVector* v2 = Vertices (seg2->m_info.verts [0]);
							if (labs (v1->v.x - v2->v.x) < 0xA00000L &&
								 labs (v1->v.y - v2->v.y) < 0xA00000L &&
								 labs (v1->v.z - v2->v.z) < 0xA00000L) {
								for (sidenum2 = 0;sidenum2 < 6; sidenum2++) {
									LinkSegments (nSegment, child, segnum2, sidenum2, 3 * F1_0);
									}
								}
							}
						}
					}
				} 
			else {
				segP->m_info.children [child] = -1; // force child to agree with bitmask
				}
			}
		}
	}
// clear all new vertices as such
for (nVertex=0;nVertex<MAX_VERTICES;nVertex++)
	VertStatus (nVertex) &= ~NEW_MASK;
// now set all seg_numbers
segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
	segP->m_info.nIndex = nSegment;
/*
if (option != 1) {
	sprintf_s (message, sizeof (message)," Block tool: %d blocks pasted.",count);
	DEBUGMSG (message);
	}
*/
fclose(fBlk);
//theApp.MineView ()->Refresh ();
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
return 0;
}

//==========================================================================
// MENU - Quick Paste
//==========================================================================

void CMine::QuickPasteBlock ()
{
if (!*m_szBlockFile) {
	PasteBlock ();
//	ErrorMsg ("You must first use one of the cut or paste commands\n"
//				"before you use the Quick Paste command");
	return;
	}

if (m_bSplineActive) {
	ErrorMsg (spline_error_message);
	return;
	}

//UpdateUndoBuffer(0);

if (!ReadBlock (m_szBlockFile, 1))
	theApp.MineView ()->SetSelectMode (BLOCK_MODE);
}

//==========================================================================
// MENU - Delete Block
//==========================================================================

void CMine::DeleteBlock()
{

INT16 nSegment,count;

if (m_bSplineActive) {
	ErrorMsg (spline_error_message);
	return;
	}
// make sure some cubes are marked
count = MarkedSegmentCount ();
if (!count) {
	ErrorMsg ("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);

// delete Segments () from last to first because SegCount ()
// is effected for each deletion.  When all Segments () are marked
// the SegCount () will be decremented for each nSegment in loop.
if (QueryMsg("Are you sure you want to delete the marked cubes?")!=IDYES)
	return;

for (nSegment = SegCount () - 1; nSegment >= 0; nSegment--)
	if (Segments (nSegment)->m_info.wallFlags & MARKED_MASK) {
		if (SegCount () <= 1)
			break;
		if (Objects (0)->m_info.nSegment != nSegment)
			DeleteSegment (nSegment); // delete segP w/o asking "are you sure"
		}
// wrap back then forward to make sure segment is valid
wrap(&Current1 ().nSegment,-1,0,SegCount () - 1);
wrap(&Current2 ().nSegment,1,0,SegCount () - 1);
wrap(&Current2 ().nSegment,-1,0,SegCount () - 1);
wrap(&Current2 ().nSegment,1,0,SegCount () - 1);
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
}

//eof block.cpp