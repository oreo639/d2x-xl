// Copyright (C) 1997 Bryan Aamot

#include "Mine.h"
#include "dle-xp.h"

#define CURRENT_POINT(a) ((current.m_nPoint + (a))&0x03)

char *BLOCKOP_HINT =
	"The block of cubes will be saved relative to the current cube.\n"
	"Later, when you paste the block, it will be placed relative to\n"
	"the current cube at that time.  You can change the current side\n"
	"and the current point to affect the relative direction and\n"
	"rotation of the block.\n"
	"\n"
	"Would you like to proceed?";

//------------------------------------------------------------------------------

void CBlockManager::MakeTransformation (CDoubleMatrix& m, CDoubleVector& o)
{
short* verts = current.Segment ()->m_info.verts;
byte* sideVerts = sideVertTable [current.m_nSide];
o = *vertexManager.Vertex (verts [sideVerts [CURRENT_POINT(0)]]);
// set x'
m.rVec = *vertexManager.Vertex (verts [sideVerts [CURRENT_POINT(1)]]) - o;
// calculate y'
CVertex v = *vertexManager.Vertex (verts [sideVerts [CURRENT_POINT(3)]]) - o;
m.uVec = CrossProduct (m.rVec, v);
m.fVec = CrossProduct (m.rVec, m.uVec);
m.rVec.Normalize ();
m.uVec.Normalize ();
m.fVec.Normalize ();
}

//------------------------------------------------------------------------------
// Read ()
//
// ACTION - Reads a segment's information in text form from a file.  Adds
//          new vertices if non-identical one does not exist.  Aborts if
//	    MAX_VERTICES is hit.
//
// Change - Now reads verts relative to current side
//------------------------------------------------------------------------------

short CBlockManager::Read (CFileManager& fp) 
{
	int				i, j, k;
	short				origVertCount;
	CDoubleMatrix	m;
	CDoubleVector	xAxis, yAxis, zAxis, origin;
	short				nNewSegs = 0, nNewWalls = 0, nNewTriggers = 0, nNewObjects = 0;
	short				xlatSegNum [SEGMENT_LIMIT];
	int				byteBuf;

// remember number of vertices for later
origVertCount = vertexManager.Count ();

// set origin
MakeTransformation (m, origin);
// now take the determinant
xAxis.Set (m.uVec.v.y * m.fVec.v.z - m.fVec.v.y * m.uVec.v.z, 
			  m.fVec.v.y * m.rVec.v.z - m.rVec.v.y * m.fVec.v.z, 
			  m.rVec.v.y * m.uVec.v.z - m.uVec.v.y * m.rVec.v.z);
yAxis.Set (m.fVec.v.x * m.uVec.v.z - m.uVec.v.x * m.fVec.v.z, 
			  m.rVec.v.x * m.fVec.v.z - m.fVec.v.x * m.rVec.v.z,
			  m.uVec.v.x * m.rVec.v.z - m.rVec.v.x * m.uVec.v.z);
zAxis.Set (m.uVec.v.x * m.fVec.v.y - m.fVec.v.x * m.uVec.v.y,
			  m.fVec.v.x * m.rVec.v.y - m.rVec.v.x * m.fVec.v.y,
			  m.rVec.v.x * m.uVec.v.y - m.uVec.v.x * m.rVec.v.y);

nNewSegs = 0;
memset (xlatSegNum, 0xff, sizeof (xlatSegNum));
while (!fp.EoF ()) {
	DLE.MainFrame ()->Progress ().SetPos (fp.Tell ());
// abort if there are not at least 8 vertices free
	if (MAX_VERTICES - vertexManager.Count () < 8) {
		ErrorMsg ("No more free vertices");
		return nNewSegs;
		}
	short nSegment = segmentManager.Add ();
	if (nSegment < 0) {
		ErrorMsg ("No more free segments");
		return nNewSegs;
		}
	CSegment* segP = segmentManager.Segment (nSegment);
	segP->m_info.owner = -1;
	segP->m_info.group = -1;
	fscanf_s (fp.File (), "segment %hd\n", &segP->m_nIndex);
	xlatSegNum [segP->m_nIndex] = nSegment;
	// invert segment number so its children can be children can be fixed later
	segP->m_nIndex = ~segP->m_nIndex;

	// read in side information 
	CSide* sideP = segP->m_sides;
	for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
		short test;
		fscanf_s (fp.File (), "  side %hd\n", &test);
		if (test != nSide) {
			ErrorMsg ("Invalid side number read");
			return (0);
			}
		sideP->m_info.nWall = NO_WALL;
		fscanf_s (fp.File (), "    tmap_num %hd\n", &sideP->m_info.nBaseTex);
		fscanf_s (fp.File (), "    tmap_num2 %hd\n", &sideP->m_info.nOvlTex);
		for (i = 0; i < 4; i++)
			fscanf_s (fp.File (), "    uvls %hd %hd %hd\n",
						&sideP->m_info.uvls [i].u,
						&sideP->m_info.uvls [i].v,
						&sideP->m_info.uvls [i].l);
		if (bExtBlkFmt) {
			fscanf_s (fp.File (), "    nWall %d\n", &sideP->m_info.nWall);
			if (sideP->m_info.nWall != NO_WALL) {
				CWall w;
				CTrigger t;
				w.Clear ();
				t.Clear ();
				fscanf_s (fp.File (), "        segment %ld\n", &w.m_nSegment);
				fscanf_s (fp.File (), "        side %ld\n", &w.m_nSide);
				fscanf_s (fp.File (), "        hps %ld\n", &w.m_info.hps);
				fscanf_s (fp.File (), "        type %d\n", &w.m_info.type);
				fscanf_s (fp.File (), "        flags %d\n", &w.m_info.flags);
				fscanf_s (fp.File (), "        state %d\n", &w.m_info.state);
				fscanf_s (fp.File (), "        nClip %d\n", &w.m_info.nClip);
				fscanf_s (fp.File (), "        keys %d\n", &w.m_info.keys);
				fscanf_s (fp.File (), "        cloak %d\n", &w.m_info.cloakValue);
				fscanf_s (fp.File (), "        trigger %d\n", &byteBuf);
				w.m_info.nTrigger = byteBuf;
				if ((w.m_info.nTrigger >= 0) && (w.m_info.nTrigger < MAX_TRIGGERS)) {
					fscanf_s (fp.File (), "			    type %d\n", &t.m_info.type);
					fscanf_s (fp.File (), "			    flags %hd\n", &t.m_info.flags);
					fscanf_s (fp.File (), "			    value %ld\n", &t.m_info.value);
					fscanf_s (fp.File (), "			    timer %d\n", &t.m_info.time);
					fscanf_s (fp.File (), "			    count %hd\n", &t.m_count);
					for (i = 0; i < t.m_count; i++) {
						fscanf_s (fp.File (), "			        segP %hd\n", &t.m_targets [i].m_nSegment);
						fscanf_s (fp.File (), "			        side %hd\n", &t.m_targets [i].m_nSide);
						}
					}
				if (wallManager.HaveResources ()) {
					if ((w.m_info.nTrigger >= 0) && (w.m_info.nTrigger < MAX_TRIGGERS)) {
						if (!triggerManager.HaveResources ())
							w.m_info.nTrigger = NO_TRIGGER;
						else {
							w.m_info.nTrigger = (byte) triggerManager.Add ();
							*triggerManager.Trigger (w.m_info.nTrigger) = t;
							++nNewTriggers;
							}
						}
					sideP->m_info.nWall = wallManager.Add ();
					w.m_nSegment = nSegment;
					*wallManager.Wall (sideP->m_info.nWall) = w;
					nNewWalls++;
					}
				}
			}
		}
	short children [6];
	fscanf_s (fp.File (), "  children %hd %hd %hd %hd %hd %hd\n", 
				 children + 0, children + 1, children + 2, children + 3, children + 4, children + 5, children + 6);
	for (i = 0; i < 6; i++)
		segP->SetChild (i, children [i]);
	// read in vertices
	for (i = 0; i < 8; i++) {
		int x, y, z, test;
		fscanf_s (fp.File (), "  vms_vector %hd %ld %ld %ld\n", &test, &x, &y, &z);
		if (test != i) {
			ErrorMsg ("Invalid vertex number read");
			return (0);
			}
		// each vertex relative to the origin has a x', y', and z' component
		// adjust vertices relative to origin
		CDoubleVector v;
		v.Set (x, y, z);
		v.Set (v ^ xAxis, v ^ xAxis, v ^ xAxis);
		v += origin;
		// add a new vertex
		// if this is the same as another vertex, then use that vertex number instead
		CVertex* vertP = vertexManager.Vertex (origVertCount);
		for (k = vertexManager.Count () - origVertCount; k > 0; k--, vertP++)
			if (*vertP == v) {
				segP->m_info.verts [i] = k;
				break;
				}
		// else make a new vertex
		if (k == 0) {
			vertexManager.Add ();
			int nVertex = vertexManager.Count () - 1;
			vertexManager.Status (nVertex) |= NEW_MASK;
			segP->m_info.verts [i] = nVertex;
			*vertexManager.Vertex (nVertex) = v;
			}
		}
	// mark vertices
	for (i = 0; i < 8; i++)
		vertexManager.Status (segP->m_info.verts [i]) |= MARKED_MASK;
	fscanf_s (fp.File (), "  staticLight %ld\n", &segP->m_info.staticLight);
	if (bExtBlkFmt) {
		fscanf_s (fp.File (), "  special %d\n", &segP->m_info.function);
		fscanf_s (fp.File (), "  nMatCen %d\n", &segP->m_info.nMatCen);
		fscanf_s (fp.File (), "  value %d\n", &segP->m_info.value);
		fscanf_s (fp.File (), "  childFlags %d\n", &segP->m_info.childFlags);
		fscanf_s (fp.File (), "  wallFlags %d\n", &segP->m_info.wallFlags);
		switch (segP->m_info.function) {
			case SEGMENT_FUNC_FUELCEN:
				if (!segmentManager.CreateFuelCenter (nSegment, SEGMENT_FUNC_FUELCEN, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_REPAIRCEN:
				if (!segmentManager.CreateFuelCenter (nSegment, SEGMENT_FUNC_REPAIRCEN, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_ROBOTMAKER:
				if (!segmentManager.CreateRobotMaker (nSegment, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_EQUIPMAKER:
				if (!segmentManager.CreateEquipMaker (nSegment, false, false))
					segP->m_info.function = 0;
				break;
			case SEGMENT_FUNC_CONTROLCEN:
				if (!segmentManager.CreateReactor (nSegment, false, false))
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
	segP->m_info.wallFlags = MARKED_MASK; // no other bits
	// calculate childFlags
	segP->m_info.childFlags = 0;
	for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
		if (segP->GetChild (i) >= 0)
		segP->m_info.childFlags |= (1 << i);
	nNewSegs++;
	}

CTrigger *trigP = triggerManager.Trigger (triggerManager.Count (0));
for (i = nNewTriggers; i; i--) {
	trigP--;
	for (j = 0; j < trigP->m_count; j++) {
		if (trigP->Segment (j) >= 0)
			trigP->Segment (j) = xlatSegNum [trigP->Segment (j)];
		else if (trigP->m_count == 1) {
			triggerManager.Delete (triggerManager.Index (trigP));
			i--;
			}
		else {
			trigP->Delete (j);
			}
		}
	}

sprintf_s (message, sizeof (message),
			" Block tool: %d blocks, %d walls, %d triggers pasted.", 
			nNewSegs, nNewWalls, nNewTriggers);
DEBUGMSG (message);
return (nNewSegs);
}

//------------------------------------------------------------------------------
// dump_seg_info()
//
// ACTION - Writes a segment's information in text form to a fp.  Uses
//          actual coordinate information instead of vertex number.  Only
//          saves segment information (no Walls (), Objects (), or Triggers ()).
//
// Change - Now saves verts relative to the current side (point 0)
//          Uses x',y',z' axis where:
//          y' is in the neg dirction of line0
//          z' is in the neg direction orthogonal to line0 & line3
//          x' is in the direction orghogonal to x' and y'
//
//------------------------------------------------------------------------------

void CBlockManager::Write (CFileManager& fp) 
{
	short				nSegment;
	short				i,j;
	CVertex			origin;
	CDoubleMatrix	m;
	CDoubleVector	v;
	short				nVertex;

// set origin
MakeTransformation (m, origin);

CSegment* segP = segmentManager.Segment (0);
for (nSegment = 0; nSegment < segmentManager.Count (); nSegment++, segP++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	if (segP->m_info.wallFlags & MARKED_MASK) {
		fprintf (fp.File (), "segment %d\n",nSegment);
		CSide* sideP = segP->m_sides;
		for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++, sideP++) {
			fprintf (fp.File (), "  side %d\n",i);
			fprintf (fp.File (), "    tmap_num %d\n",sideP->m_info.nBaseTex);
			fprintf (fp.File (), "    tmap_num2 %d\n",sideP->m_info.nOvlTex);
			for (j = 0; j < 4; j++) {
				fprintf (fp.File (), "    uvls %d %d %d\n",
				sideP->m_info.uvls [j].u,
				sideP->m_info.uvls [j].v,
				sideP->m_info.uvls [j].l);
				}
			if (bExtBlkFmt) {
				fprintf (fp.File (), "    nWall %d\n", 
							(sideP->m_info.nWall < wallManager.Count ()) ? sideP->m_info.nWall : NO_WALL);
				if (sideP->m_info.nWall < wallManager.Count ()) {
					CWall* wallP = sideP->Wall ();
					fprintf (fp.File (), "        segment %d\n", wallP->m_nSegment);
					fprintf (fp.File (), "        side %d\n", wallP->m_nSide);
					fprintf (fp.File (), "        hps %d\n", wallP->m_info.hps);
					fprintf (fp.File (), "        type %d\n", wallP->m_info.type);
					fprintf (fp.File (), "        flags %d\n", wallP->m_info.flags);
					fprintf (fp.File (), "        state %d\n", wallP->m_info.state);
					fprintf (fp.File (), "        nClip %d\n", wallP->m_info.nClip);
					fprintf (fp.File (), "        keys %d\n", wallP->m_info.keys);
					fprintf (fp.File (), "        cloak %d\n", wallP->m_info.cloakValue);
					if ((wallP->m_info.nTrigger < 0) || (wallP->m_info.nTrigger >= triggerManager.Count (0)))
						fprintf (fp.File (), "        trigger %u\n", NO_TRIGGER);
					else {
						CTrigger *trigP = wallP->Trigger ();
						int iTarget, count = 0;
						// count trigP targets in marked area
						for (iTarget = 0; iTarget < trigP->m_count; iTarget++)
							if (segmentManager.Segment (trigP->Segment (iTarget))->m_info.wallFlags & MARKED_MASK)
								count++;
						fprintf (fp.File (), "        trigP %d\n", wallP->m_info.nTrigger);
						fprintf (fp.File (), "			    type %d\n", trigP->m_info.type);
						fprintf (fp.File (), "			    flags %ld\n", trigP->m_info.flags);
						fprintf (fp.File (), "			    value %ld\n", trigP->m_info.value);
						fprintf (fp.File (), "			    timer %d\n", trigP->m_info.time);
						fprintf (fp.File (), "			    count %d\n", count);
						for (iTarget = 0; iTarget < trigP->m_count; iTarget++)
							if (segmentManager.Segment (trigP->Segment (iTarget))->m_info.wallFlags & MARKED_MASK) {
								fprintf (fp.File (), "			        segP %d\n", trigP->Segment (iTarget));
								fprintf (fp.File (), "			        side %d\n", trigP->Side (iTarget));
								}
						}
					}
				}
			}
		fprintf (fp.File (), "  children");
		for (i = 0; i < 6; i++)
			fprintf (fp.File (), " %d", segP->GetChild (i));
		fprintf (fp.File (), "\n");
		// save vertices
		for (i = 0; i < 8; i++) {
			// each vertex relative to the origin has a x', y', and z' component
			// which is a constant (k) times the axis
			// k = (B*A)/(A*A) where B is the vertex relative to the origin
			//                       A is the axis unit vVertexor (always 1)
			nVertex = segP->m_info.verts [i];
			CVertex v = *vertexManager.Vertex (nVertex) - origin;
			fprintf (fp.File (), "  vms_vector %d %ld %ld %ld\n", i, D2X (v ^ m.rVec), D2X (v ^ m.uVec), D2X (v ^ m.fVec));
			}
		fprintf (fp.File (), "  staticLight %ld\n",segP->m_info.staticLight);
		if (bExtBlkFmt) {
			fprintf (fp.File (), "  special %d\n",segP->m_info.function);
			fprintf (fp.File (), "  nMatCen %d\n",segP->m_info.nMatCen);
			fprintf (fp.File (), "  value %d\n",segP->m_info.value);
			fprintf (fp.File (), "  childFlags %d\n",segP->m_info.childFlags);
			fprintf (fp.File (), "  wallFlags %d\n",segP->m_info.wallFlags);
			}
		}
	}
}

//------------------------------------------------------------------------------

void CBlockManager::Cut (void)
{
if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message);
	return;
	}

  // make sure some cubes are marked
short count = segmentManager.MarkedCount ();
if (count == 0) {
	ErrorMsg ("No block marked.\n\n""Use 'M' or shift left mouse button\n""to mark one or more cubes.");
	return;
	}

if (!bExpertMode && Query2Msg(BLOCKOP_HINT,MB_YESNO) != IDYES)
	return;

 char szFile [256] = "\0";
if (!BrowseForFile (FALSE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  DLE.MainFrame ()))
	return;
_strlwr_s (szFile, sizeof (szFile));
bExtBlkFmt = strstr (szFile, ".blx") != null;

CFileManager fp;
if (fp.Open (szFile, "w")) {
	ErrorMsg ("Unable to open block file");
	return;
	}
//undoManager.UpdateBuffer(0);
strcpy_s (m_filename, sizeof (m_filename), szFile); // remember file for quick paste
fprintf (fp.File (), bExtBlkFmt ? "DMB_EXT_BLOCK_FILE\n" : "DMB_BLOCK_FILE\n");
DLE.MainFrame ()->InitProgress (segmentManager.Count ());
Write (fp);
DLE.MainFrame ()->Progress ().DestroyWindow ();

undoManager.SetModified (true);
undoManager.Lock ();
DLE.MainFrame ()->InitProgress (segmentManager.Count ());
CSegment *segP = segmentManager.Segment (segmentManager.Count ());
for (short nSegment = segmentManager.Count () - 1; nSegment; nSegment--) {
	DLE.MainFrame ()->Progress ().StepIt ();
    if ((--segP)->m_info.wallFlags & MARKED_MASK) {
		if (segmentManager.Count () <= 1)
			break;
		segmentManager.Delete (nSegment); // delete segP w/o asking "are you sure"
		}
	}
DLE.MainFrame ()->Progress ().DestroyWindow ();
undoManager.Unlock ();
fp.Close ();
sprintf_s (message, sizeof (message), " Block tool: %d blocks cut to '%s' relative to current side.", count, szFile);
DEBUGMSG (message);
  // wrap back then forward to make sure segment is valid
Wrap (selections [0].m_nSegment, -1, 0, segmentManager.Count () - 1);
Wrap (selections [1].m_nSegment, 1, 0, segmentManager.Count () - 1);
Wrap (selections [1].m_nSegment, -1, 0, segmentManager.Count () - 1);
Wrap (selections [1].m_nSegment, 1, 0, segmentManager.Count () - 1);
segmentManager.SetLinesToDraw ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CBlockManager::Copy (char *filename)
{
  CFileManager fp;
  char szFile [256] = "\0";
  short count = segmentManager.MarkedCount ();

// make sure some cubes are marked
if (count == 0) {
	ErrorMsg ("No block marked.\n\n""Use 'M' or shift left mouse button\n""to mark one or more cubes.");
	return;
	}

if (!bExpertMode && Query2Msg (BLOCKOP_HINT,MB_YESNO) != IDYES)
	return;
if (filename && *filename)
	strcpy_s (szFile, sizeof (szFile), filename);
else {
	strcpy_s (szFile, sizeof (szFile), m_filename);
	if (!BrowseForFile (FALSE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  DLE.MainFrame ()))
		return;
	}
_strlwr_s (szFile, sizeof (szFile));
bExtBlkFmt = strstr (szFile, ".blx") != null;
if (fp.Open (szFile, "w")) {
	sprintf_s (message, sizeof (message), "Unable to open block file '%s'", szFile);
	ErrorMsg (message);
	return;
	}
//  undoManager.UpdateBuffer(0);
strcpy_s (m_filename, sizeof (m_filename), szFile); // remember fp for quick paste
fprintf (fp.File (), bExtBlkFmt ? "DMB_EXT_BLOCK_FILE\n" : "DMB_BLOCK_FILE\n");
Write (fp);
fp.Close ();
sprintf_s (message, sizeof (message), " Block tool: %d blocks copied to '%s' relative to current side.", count, szFile);
DEBUGMSG (message);
segmentManager.SetLinesToDraw ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CBlockManager::Paste (void) 
{
if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message);
	return;
	}
// Initialize data for fp open dialog
  char szFile [256] = "\0";

if (!BrowseForFile (TRUE, 
	                 bExtBlkFmt ? "blx" : "blk", szFile, 
						  "Block file|*.blk|"
						  "Extended block file|*.blx|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
						  DLE.MainFrame ()))
	return;
if (!Read (szFile))
	DLE.MineView ()->SetSelectMode (BLOCK_MODE);
}

//------------------------------------------------------------------------------

int CBlockManager::Read (char *filename) 
{
	CSegment *segP, *seg2P;
	short nSegment;
	short count;
	CFileManager fp;

_strlwr_s (filename, 256);
if (fp.Open (filename, "r")) {
	ErrorMsg ("Unable to open block file");
	return 1;
	}	

fscanf_s (fp.File (), "%s\n", &message, sizeof (message));
if (!strncmp (message, "DMB_BLOCK_FILE", 14))
	bExtBlkFmt = false;
else if (!strncmp (message, "DMB_EXT_BLOCK_FILE", 18))
	bExtBlkFmt = true;
else {
	ErrorMsg ("This is not a block file.");
	fp.Close ();
	return 2;
	}

strcpy_s (m_filename, sizeof (m_filename), filename); // remember file for quick paste

// unmark all segmentManager.Segment ()
// set up all seg_numbers (makes sure there are no negative seg_numbers)
undoManager.SetModified (true);
undoManager.Lock ();
DLE.MineView ()->DelayRefresh (true);
segP = segmentManager.Segment (0);
for (nSegment = 0; nSegment < MAX_SEGMENTS; nSegment++, segP++) {
	segP->m_nIndex = nSegment;
	segP->m_info.wallFlags &= ~MARKED_MASK;
	}

// unmark all vertices
for (ushort nVertex = 0; nVertex < MAX_VERTICES; nVertex++) {
	vertexManager.Status (nVertex) &= ~MARKED_MASK;
	vertexManager.Status (nVertex) &= ~NEW_MASK;
	}

DLE.MainFrame ()->InitProgress (fp.Length ());
count = Read (fp);
DLE.MainFrame ()->Progress ().DestroyWindow ();

// int up the new segmentManager.Segment () children
segP = segmentManager.Segment (0);
for (nSegment = 0; nSegment < segmentManager.Count (); nSegment++, segP++) {
	if (segP->m_nIndex < 0) {  // if segment was just inserted
		// if child has a segment number that was just inserted, set it to the
		//  segment's offset number, otherwise set it to -1
		for (short nChild = 0; nChild < MAX_SIDES_PER_SEGMENT; nChild++) {
			if (segP->HasChild (nChild)) {
				seg2P = segmentManager.Segment (0);
				short nSegOffset;
				for (nSegOffset = 0; nSegOffset < segmentManager.Count (); nSegOffset++, seg2P++) {
					if (segP->GetChild (nChild) == ~seg2P->m_nIndex) {
						segP->SetChild (nChild, nSegOffset);
						break;
						}
					}
				if (nSegOffset == segmentManager.Count ()) { // no child found
					segmentManager.ResetSide (nSegment, nChild);
					// auto link the new segment with any touching segmentManager.Segment ()
					seg2P = segmentManager.Segment (0);
					for (short nSegment2 = 0; nSegment2 < segmentManager.Count (); nSegment2++, seg2P++) {
						if (nSegment != nSegment2) {
							// first check to see if segmentManager.Segment () are any where near each other
							// use x, y, and z coordinate of first point of each segment for comparison
							CVertex* v1 = vertexManager.Vertex (segP ->m_info.verts [0]);
							CVertex* v2 = vertexManager.Vertex (seg2P->m_info.verts [0]);
							if (fabs (v1->v.x - v2->v.x) < 10.0 &&
								 fabs (v1->v.y - v2->v.y) < 10.0 &&
								 fabs (v1->v.z - v2->v.z) < 10.0) {
								for (short nSide2 = 0; nSide2 < 6; nSide2++) {
									segmentManager.Link (nSegment, nChild, nSegment2, nSide2, I2X (3));
									}
								}
							}
						}
					}
				} 
			else {
				segP->SetChild (nChild, -1); // force child to agree with bitmask
				}
			}
		}
	}
// clear all new vertices as such
for (ushort nVertex = 0; nVertex < MAX_VERTICES; nVertex++)
	vertexManager.Status (nVertex) &= ~NEW_MASK;
// now set all seg_numbers
segP = segmentManager.Segment (0);
for (nSegment = 0; nSegment < segmentManager.Count (); nSegment++, segP++)
	segP->m_nIndex = nSegment;
fp.Close ();
DLE.MineView ()->Refresh ();
undoManager.Unlock ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
return 0;
}

//------------------------------------------------------------------------------

void CBlockManager::QuickPaste (void)
{
if (!*m_filename) {
	Paste ();
//	ErrorMsg ("You must first use one of the cut or paste commands\n"
//				"before you use the Quick Paste command");
	return;
	}

if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message);
	return;
	}

//undoManager.UpdateBuffer(0);

if (!Read (m_filename))
	DLE.MineView ()->SetSelectMode (BLOCK_MODE);
}

//------------------------------------------------------------------------------

void CBlockManager::Delete (void)
{
short nSegment, count;

if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message);
	return;
	}
// make sure some cubes are marked
count = segmentManager.MarkedCount ();
if (!count) {
	ErrorMsg ("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

undoManager.SetModified (true);
undoManager.Lock ();
DLE.MineView ()->DelayRefresh (true);

// delete segmentManager.Segment () from last to first because segmentManager.Count ()
// is effected for each deletion.  When all segmentManager.Segment () are marked
// the segmentManager.Count () will be decremented for each nSegment in loop.
if (QueryMsg ("Are you sure you want to delete the marked cubes?") != IDYES)
	return;

DLE.MainFrame ()->InitProgress (segmentManager.Count ());
for (nSegment = segmentManager.Count () - 1; nSegment >= 0; nSegment--) {
		DLE.MainFrame ()->Progress ().StepIt ();
		if (segmentManager.Segment (nSegment)->m_info.wallFlags & MARKED_MASK) {
		if (segmentManager.Count () <= 1)
			break;
		if (objectManager.Object (0)->m_info.nSegment != nSegment)
			segmentManager.Delete (nSegment); // delete segP w/o asking "are you sure"
		}
	}
DLE.MainFrame ()->Progress ().DestroyWindow ();
// wrap back then forward to make sure segment is valid
Wrap (selections [0].m_nSegment, -1, 0, segmentManager.Count () - 1);
Wrap (selections [0].m_nSegment, 1, 0, segmentManager.Count () - 1);
Wrap (selections [1].m_nSegment, -1, 0, segmentManager.Count () - 1);
Wrap (selections [1].m_nSegment, 1, 0, segmentManager.Count () - 1);
undoManager.Unlock ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
}

//eof block.cpp