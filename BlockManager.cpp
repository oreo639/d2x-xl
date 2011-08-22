// Copyright (C) 1997 Bryan Aamot

#include "Mine.h"
#include "dle-xp.h"

CBlockManager blockManager;

//------------------------------------------------------------------------------

#define CURRENT_POINT(a) ((current->m_nPoint + (a))&0x03)

//------------------------------------------------------------------------------

char *BLOCKOP_HINT =
	"The block of cubes will be saved relative to the current segment.\n"
	"Later, when you paste the block, it will be placed relative to\n"
	"the current segment at that time.  You can change the current side\n"
	"and the current point to affect the relative direction and\n"
	"rotation of the block.\n"
	"\n"
	"Would you like to proceed?";

//------------------------------------------------------------------------------

void CBlockManager::SetupTransformation (CDoubleMatrix& m, CDoubleVector& o)
{
ushort* verts = current->Segment ()->m_info.verts;
byte* sideVerts = sideVertTable [current->m_nSide];
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
	int				i, j, scanRes;
	short				origVertCount;
	CDoubleMatrix	m;
	CDoubleVector	xAxis, yAxis, zAxis, origin;
	short				nNewSegs = 0, nNewWalls = 0, nNewTriggers = 0;
	int				byteBuf; // needed for scanning byte values
	CTrigger			* newTriggers = null;
	
m_oldSegments = m_newSegments = null;
// remember number of vertices for later
origVertCount = vertexManager.Count ();

// set origin
SetupTransformation (m, origin);
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
memset (m_xlatSegNum, 0xff, sizeof (m_xlatSegNum));

for (i = 0, j = segmentManager.Count (); i < j; i++) {
	segmentManager.Segment (i)->SetLink (m_oldSegments);
	m_oldSegments = segmentManager.Segment (i);
	}	

undoManager.Begin (udAll);
while (!fp.EoF ()) {
	DLE.MainFrame ()->Progress ().SetPos (fp.Tell ());
// abort if there are not at least 8 vertices free
	if (MAX_VERTICES - vertexManager.Count () < 8) {
		undoManager.End ();
		ErrorMsg ("No more free vertices");
		return nNewSegs;
		}
	short nSegment = segmentManager.Add ();
	if (nSegment < 0) {
		undoManager.End ();
		ErrorMsg ("No more free segments");
		return nNewSegs;
		}
	CSegment* segP = segmentManager.Segment (nSegment);
	segP->SetLink (m_newSegments);
	m_newSegments = segP;
	segP->m_info.owner = -1;
	segP->m_info.group = -1;
	scanRes = fscanf_s (fp.File (), "segment %d\n", &segP->Index ());
	m_xlatSegNum [segP->Index ()] = nSegment;
	// invert segment number so its children can be children can be fixed later
	segP->Index () = -segP->Index () - 1;

	// read in side information 
	CSide* sideP = segP->m_sides;
	for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
		short test;
		scanRes = fscanf_s (fp.File (), "  side %hd\n", &test);
		if (test != nSide) {
			undoManager.End ();
			ErrorMsg ("Invalid side number read");
			return (0);
			}
		sideP->m_info.nWall = NO_WALL;
		scanRes = fscanf_s (fp.File (), "    tmap_num %hd\n", &sideP->m_info.nBaseTex);
		scanRes = fscanf_s (fp.File (), "    tmap_num2 %hd\n", &sideP->m_info.nOvlTex);
		for (i = 0; i < 4; i++)
			scanRes = fscanf_s (fp.File (), "    uvls %hd %hd %hd\n", 
									  &sideP->m_info.uvls [i].u, &sideP->m_info.uvls [i].v, &sideP->m_info.uvls [i].l);
		if (bExtBlkFmt) {
			scanRes = fscanf_s (fp.File (), "    wall %hd\n", &sideP->m_info.nWall);
			if (sideP->m_info.nWall != NO_WALL) {
				CWall w;
				CTrigger t;
				w.Clear ();
				t.Clear ();
				scanRes = fscanf_s (fp.File (), "        segment %hd\n", &w.m_nSegment);
				scanRes = fscanf_s (fp.File (), "        side %hd\n", &w.m_nSide);
				scanRes = fscanf_s (fp.File (), "        hps %d\n", &w.Info ().hps);
				scanRes = fscanf_s (fp.File (), "        type %d\n", &byteBuf);
				w.Info ().type = (byte) byteBuf;
				scanRes = fscanf_s (fp.File (), "        flags %hd\n", &w.Info ().flags);
				scanRes = fscanf_s (fp.File (), "        state %d\n", &byteBuf);
				w.Info ().state = (byte) byteBuf;
				scanRes = fscanf_s (fp.File (), "        clip %d\n", &byteBuf);
				w.Info ().nClip = (byte) byteBuf;
				scanRes = fscanf_s (fp.File (), "        keys %d\n", &byteBuf);
				w.Info ().keys = (byte) byteBuf;
				scanRes = fscanf_s (fp.File (), "        cloak %d\n", &byteBuf);
				w.Info ().cloakValue = (byte) byteBuf;
				scanRes = fscanf_s (fp.File (), "        trigger %d\n", &byteBuf);
				w.Info ().nTrigger = (byte) byteBuf;
				if ((w.Info ().nTrigger >= 0) && (w.Info ().nTrigger < MAX_TRIGGERS)) {
					scanRes = fscanf_s (fp.File (), "			    type %d\n", &byteBuf);
					t.Info ().type = (byte) byteBuf;
					scanRes = fscanf_s (fp.File (), "			    flags %hd\n", &t.Info ().flags);
					scanRes = fscanf_s (fp.File (), "			    value %d\n", &t.Info ().value);
					scanRes = fscanf_s (fp.File (), "			    timer %d\n", &t.Info ().time);
					scanRes = fscanf_s (fp.File (), "			    count %hd\n", &t.Count ());
					for (i = 0; i < t.Count (); i++) {
						scanRes = fscanf_s (fp.File (), "			        segment %hd\n", &t [i].m_nSegment);
						scanRes = fscanf_s (fp.File (), "			        side %hd\n", &t [i].m_nSide);
						}
					}
				if (wallManager.HaveResources ()) {
					if ((w.Info ().nTrigger >= 0) && (w.Info ().nTrigger < MAX_TRIGGERS)) {
						if (!triggerManager.HaveResources ())
							w.Info ().nTrigger = NO_TRIGGER;
						else {
							w.Info ().nTrigger = (byte) triggerManager.Add ();
							CTrigger* trigP = triggerManager.Trigger (w.Info ().nTrigger);
							*trigP = t;
							trigP->Backup (opAdd);
							trigP->SetLink (newTriggers);
							newTriggers = trigP;
							++nNewTriggers;
							}
						}
					sideP->m_info.nWall = wallManager.Add (false);
					w.m_nSegment = nSegment;
					CWall* wallP = wallManager.Wall (sideP->m_info.nWall);
					*wallP = w;
					wallP->Backup (opAdd);
					++nNewWalls;
					}
				}
			}
		}

	short children [6];
	scanRes = fscanf_s (fp.File (), "  children %hd %hd %hd %hd %hd %hd\n", 
				 children + 0, children + 1, children + 2, children + 3, children + 4, children + 5, children + 6);
	for (i = 0; i < 6; i++)
		segP->SetChild (i, children [i]);
	// read in vertices
	byte bShared = 0;
	for (i = 0; i < 8; i++) {
		int x, y, z, test;
		scanRes = fscanf_s (fp.File (), "  vms_vector %d %d %d %d\n", &test, &x, &y, &z);
		if (test != i) {
			undoManager.End ();
			ErrorMsg ("Invalid vertex number read");
			return (0);
			}
		// each vertex relative to the origin has a x', y', and z' component
		// adjust vertices relative to origin
		CDoubleVector v;
		v.Set (X2D (x), X2D (y), X2D (z));
		v.Set (v ^ xAxis, v ^ yAxis, v ^ zAxis);
		v += origin;
		// add a new vertex
		// if this is the same as another vertex, then use that vertex number instead
		CVertex* vertP = vertexManager.Find (v);
		ushort nVertex;
		if (vertP != null) {
			nVertex = segP->m_info.verts [i] = vertexManager.Index (vertP);
			bShared |= 1 << i;
			}
		// else make a new vertex
		else  {
			nVertex;
			vertexManager.Add (&nVertex);
			vertexManager.Status (nVertex) |= NEW_MASK;
			segP->m_info.verts [i] = nVertex;
			*vertexManager.Vertex (nVertex) = v;
			vertexManager.Vertex (nVertex)->Backup ();
			}
		vertexManager.Status (nVertex) |= MARKED_MASK;
		}

#if 0
	// check each side whether it shares all four vertices with another side
	// if so, make the segment owning that side a child
	for (short nSide = 0; nSide < 6; nSide++) {
		if (segP->Child (nSide) >= 0) // has a child in the block
			continue;
		short nVertex;
		ushort sharedVerts [4];
		for (nVertex = 0; nVertex < 4; nVertex++) {
			ushort h = sideVertTable [nSide][nVertex];
			if ((bShared & (1 << h)) == 0)
				break;
			sharedVerts [nVertex] = segP->m_info.verts [h];
			}
		if (nVertex == 4) {
			for (nVertex = 0; nVertex < 4; nVertex++)
				vertexManager.Status (sharedVerts [nVertex]) |= SHARED_MASK;
			for (CSegment* childSegP = m_oldSegments; childSegP != null; childSegP = dynamic_cast<CSegment*>(childSegP->Link ())) {
				short nChildSide;
				for (nChildSide = 0; nChildSide < 6; nChildSide++) {
					if (childSegP->Child (nChildSide) != -1)
						continue;
					short nChildVert;
					for (nChildVert = 0; nChildVert < 4; nChildVert++) {
						if (!childSegP->Vertex (sideVertTable [nChildSide][nChildVert])->IsMarked (SHARED_MASK))
							break;
						}
					if (nChildVert == 4) {
						childSegP->SetChild (nChildSide, nSegment);
						short nChildSeg = segmentManager.Index (childSegP);
						segP->SetChild (nSide, nChildSeg);
						segmentManager.SetTextures (CSideKey (nSegment, nSide), 0, 0); 
						segmentManager.SetTextures (CSideKey (nChildSeg, nChildSide), 0, 0); 
						childSegP = null; // break out of outer loop
						break;
						}	
					}
				if (childSegP == null)
					break;
				}
			for (nVertex = 0; nVertex < 4; nVertex++)
				vertexManager.Status (sharedVerts [nVertex]) &= ~SHARED_MASK;
			}
		}
#endif
	// mark vertices
	scanRes = fscanf_s (fp.File (), "  static_light %d\n", &segP->m_info.staticLight);
	if (bExtBlkFmt) {
		scanRes = fscanf_s (fp.File (), "  special %d\n", &segP->m_info.function);
		scanRes = fscanf_s (fp.File (), "  matcen_num %d\n", &byteBuf);
		segP->m_info.nMatCen = (byte) byteBuf;
		scanRes = fscanf_s (fp.File (), "  value %d\n", &byteBuf);
		segP->m_info.value = (byte) byteBuf;
		scanRes = fscanf_s (fp.File (), "  child_bitmask %d\n", &byteBuf);
		segP->m_info.childFlags = (byte) byteBuf;
		scanRes = fscanf_s (fp.File (), "  wall_bitmask %d\n", &byteBuf);
		segP->m_info.wallFlags = (byte) byteBuf;
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
			case SEGMENT_FUNC_REACTOR:
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
	segP->Mark (MARKED_MASK); // no other bits
	// calculate childFlags
	segP->m_info.childFlags = 0;
	for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
		if (segP->Child (i) >= 0)
			segP->m_info.childFlags |= (1 << i);
		}
	segP->Backup ();
	nNewSegs++;
	}

#if 0
while (m_newSegments != null) {
	short nSegment = segmentManager.Index (m_newSegments);
	for (int nSide = 0; nSide < 6; nSide++) {
		if (m_newSegments->Child (nSide) < 0)
			segmentManager.Join (CSideKey (nSegment, nSide), true);
		}
	m_newSegments = dynamic_cast<CSegment*> (m_newSegments->Link ());
	}
#endif

while (newTriggers != null) {
	CTrigger* trigP = newTriggers;
	newTriggers = dynamic_cast<CTrigger*> (trigP->Link ());
	for (j = 0; j < trigP->Count (); j++) {
		if (trigP->Segment (j) >= 0)
			trigP->Segment (j) = m_xlatSegNum [trigP->Segment (j)];
		else if (trigP->Count () == 1) {
			triggerManager.Delete (triggerManager.Index (trigP));
			i--;
			}
		else {
			trigP->Delete (j);
			}
		}
	}

undoManager.End ();
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
	short				i, j;
	CVertex			origin;
	CDoubleMatrix	m;
	CDoubleVector	v;
	short				nVertex;

// set origin
SetupTransformation (m, origin);

CSegment* segP = segmentManager.Segment (0);
for (CSegmentIterator si; si; si++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	CSegment* segP = &(*si);
	short nSegment = si.Index ();
	if (segP->IsMarked ()) {
		fprintf (fp.File (), "segment %d\n", nSegment);
		CSide* sideP = segP->m_sides;
		for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++, sideP++) {
			fprintf (fp.File (), "  side %d\n",i);
			fprintf (fp.File (), "    tmap_num %d\n",sideP->m_info.nBaseTex);
			fprintf (fp.File (), "    tmap_num2 %d\n",sideP->m_info.nOvlTex);
			for (j = 0; j < 4; j++) {
				fprintf (fp.File (), "    uvls %d %d %d\n",
							sideP->m_info.uvls [j].u, sideP->m_info.uvls [j].v, sideP->m_info.uvls [j].l);
				}
			if (bExtBlkFmt) {
				fprintf (fp.File (), "    wall %d\n", sideP->m_info.nWall);
				if (sideP->m_info.nWall != NO_WALL) {
					CWall* wallP = sideP->Wall ();
					fprintf (fp.File (), "        segment %d\n", wallP->m_nSegment);
					fprintf (fp.File (), "        side %d\n", wallP->m_nSide);
					fprintf (fp.File (), "        hps %d\n", wallP->Info ().hps);
					fprintf (fp.File (), "        type %d\n", wallP->Type ());
					fprintf (fp.File (), "        flags %d\n", wallP->Info ().flags);
					fprintf (fp.File (), "        state %d\n", wallP->Info ().state);
					fprintf (fp.File (), "        clip %d\n", wallP->Info ().nClip);
					fprintf (fp.File (), "        keys %d\n", wallP->Info ().keys);
					fprintf (fp.File (), "        cloak %d\n", wallP->Info ().cloakValue);
					if (wallP->Info ().nTrigger == NO_TRIGGER)
						fprintf (fp.File (), "        trigger %u\n", NO_TRIGGER);
					else {
						CTrigger *trigP = wallP->Trigger ();
						int iTarget, count = 0;
						// count trigger targets in marked area
						for (iTarget = 0; iTarget < trigP->Count (); iTarget++)
							if (segmentManager.Segment (trigP->Segment (iTarget))->IsMarked ())
								count++;
						fprintf (fp.File (), "        trigger %d\n", wallP->Info ().nTrigger);
						fprintf (fp.File (), "			    type %d\n", trigP->Type ());
						fprintf (fp.File (), "			    flags %d\n", trigP->Info ().flags);
						fprintf (fp.File (), "			    value %d\n", trigP->Info ().value);
						fprintf (fp.File (), "			    timer %d\n", trigP->Info ().time);
						fprintf (fp.File (), "			    count %d\n", count);
						for (iTarget = 0; iTarget < trigP->Count (); iTarget++)
							if (segmentManager.Segment (trigP->Segment (iTarget))->IsMarked ()) {
								fprintf (fp.File (), "			        segment %d\n", trigP->Segment (iTarget));
								fprintf (fp.File (), "			        side %d\n", trigP->Side (iTarget));
								}
						}
					}
				}
			}
		fprintf (fp.File (), "  children");
		for (i = 0; i < 6; i++) {
			short nChild = segP->Child (i);
			fprintf (fp.File (), " %d", ((nChild < 0) || !segmentManager.Segment (nChild)->IsMarked ()) ? -1 : nChild);
			}
		fprintf (fp.File (), "\n");
		// save vertices
		for (i = 0; i < 8; i++) {
			// each vertex relative to the origin has a x', y', and z' component
			// which is a constant (k) times the axis
			// k = (B*A)/(A*A) where B is the vertex relative to the origin
			//                       A is the axis unit vertex (always 1)
			nVertex = segP->m_info.verts [i];
			CVertex v = *vertexManager.Vertex (nVertex) - origin;
			fprintf (fp.File (), "  vms_vector %d %d %d %d\n", i, D2X (v ^ m.rVec), D2X (v ^ m.uVec), D2X (v ^ m.fVec));
			}
		fprintf (fp.File (), "  static_light %d\n",segP->m_info.staticLight);
		if (bExtBlkFmt) {
			fprintf (fp.File (), "  special %d\n", segP->m_info.function);
			fprintf (fp.File (), "  matcen_num %d\n", segP->m_info.nMatCen);
			fprintf (fp.File (), "  value %d\n", segP->m_info.value);
			fprintf (fp.File (), "  child_bitmask %d\n", segP->m_info.childFlags);
			fprintf (fp.File (), "  wall_bitmask %d\n", segP->m_info.wallFlags);
			}
		}
	}
}

//------------------------------------------------------------------------------

void CBlockManager::Copy (char* filename, bool bDelete)
{
if (tunnelMaker.Active ()) 
	return;

  // make sure some cubes are marked
short count = segmentManager.MarkedCount ();
if (count == 0) {
	ErrorMsg ("No block marked.\n\n""Use 'M' or shift left mouse button\n""to mark one or more cubes.");
	return;
	}

if (!bExpertMode && Query2Msg(BLOCKOP_HINT,MB_YESNO) != IDYES)
	return;

char szFile [256];
if (filename && *filename)
	strcpy_s (szFile, sizeof (szFile), filename);
else {
	char szFile [256] = "\0";
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
fp.Close ();

if (bDelete)
	Delete ();
else {
	sprintf_s (message, sizeof (message), " Block tool: %d blocks copied to '%s' relative to current side.", count, szFile);
	DEBUGMSG (message);
  // wrap back then forward to make sure segment is valid
	segmentManager.SetLinesToDraw ();
	DLE.MineView ()->Refresh ();
	}
}

//------------------------------------------------------------------------------

void CBlockManager::Cut (void)
{
Copy (NULL, true);
}

//------------------------------------------------------------------------------

void CBlockManager::Paste (void) 
{
if (tunnelMaker.Active ()) 
	return;
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
undoManager.Begin (udAll);
DLE.MineView ()->DelayRefresh (true);
CSegment* segP = segmentManager.Segment (0);
for (short nSegment = 0; nSegment < MAX_SEGMENTS; nSegment++, segP++) {
	segP->Index () = nSegment;
	segP->Unmark ();
	}

// unmark all vertices
for (ushort nVertex = 0; nVertex < MAX_VERTICES; nVertex++) 
	vertexManager.Vertex (nVertex)->Unmark (MARKED_MASK | NEW_MASK);

DLE.MainFrame ()->InitProgress (fp.Length ());
short count = Read (fp);
DLE.MainFrame ()->Progress ().DestroyWindow ();

// int up the new segmentManager.Segment () children
for (CSegment* newSegP = m_newSegments; newSegP != null; newSegP = dynamic_cast<CSegment*>(newSegP->Link ())) {
	// if child has a segment number that was just inserted, set it to the
	//  segment's offset number, otherwise set it to -1
	for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (newSegP->HasChild (nSide)) // has a child in the block
			newSegP->SetChild (nSide, m_xlatSegNum [newSegP->Child (nSide)]);
		else {
			CVertex* v1 = vertexManager.Vertex (newSegP->m_info.verts [sideVertTable [nSide][0]]);
			for (CSegment* oldSegP = m_oldSegments; oldSegP != null; oldSegP = dynamic_cast<CSegment*>(oldSegP->Link ())) {
				for (short nChildSide = 0; nChildSide < 6; nChildSide++) {
					for (short nChildVertex = 0; nChildVertex < 4; nChildVertex++) {
						CVertex* v2 = vertexManager.Vertex (oldSegP->m_info.verts [sideVertTable [nChildSide][nChildVertex]]);
						if ((fabs (v1->v.x - v2->v.x) < 160.0) && (fabs (v1->v.y - v2->v.y) < 160.0) && (fabs (v1->v.z - v2->v.z) < 160.0)) {
							segmentManager.Link (segmentManager.Index (newSegP), nSide, segmentManager.Index (oldSegP), nChildSide, 3.0);
							break;
							}
						}
					} 
				}
			}
		}
	}
// clear all new vertices as such
for (ushort nVertex = 0; nVertex < MAX_VERTICES; nVertex++)
	vertexManager.Vertex (nVertex)->Unmark (NEW_MASK);
// now set all seg_numbers
segP = segmentManager.Segment (0);
for (short nSegment = 0; nSegment < segmentManager.Count (); nSegment++, segP++)
	segP->Index () = nSegment;
fp.Close ();
DLE.MineView ()->Refresh ();
undoManager.End ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
return 0;
}

//------------------------------------------------------------------------------

void CBlockManager::QuickPaste (void)
{
if (!*m_filename) {
	Paste ();
	return;
	}

if (tunnelMaker.Active ()) 
	return;
if (!Read (m_filename))
	DLE.MineView ()->SetSelectMode (BLOCK_MODE);
}

//------------------------------------------------------------------------------

void CBlockManager::Delete (void)
{
short nSegment, count;

if (tunnelMaker.Active ()) 
	return;
// make sure some cubes are marked
count = segmentManager.MarkedCount ();
if (!count) {
	ErrorMsg ("No block marked.\n\n"
				"Use 'M' or shift left mouse button\n"
				"to mark one or more cubes.");
	return;
	}

undoManager.Begin (udAll);
DLE.MineView ()->DelayRefresh (true);

// delete segmentManager.Segment () from last to first because segmentManager.Count ()
// is effected for each deletion.  When all segmentManager.Segment () are marked
// the segmentManager.Count () will be decremented for each nSegment in loop.
if (QueryMsg ("Are you sure you want to delete the marked cubes?") != IDYES)
	return;

DLE.MainFrame ()->InitProgress (segmentManager.Count ());
for (nSegment = segmentManager.Count () - 1; nSegment >= 0; nSegment--) {
	DLE.MainFrame ()->Progress ().StepIt ();
	if (segmentManager.Segment (nSegment)->IsMarked ()) {
		if (segmentManager.Count () <= 1)
			break;
		if (objectManager.Object (0)->m_info.nSegment == nSegment) {
			short nNewSeg = (nSegment < segmentManager.Count() - 1) ? nSegment + 1 : (nSegment > 0) ? nSegment - 1 : -1;
			if (nNewSeg < 0)
				continue;
			objectManager.Move (objectManager.Object (0), nNewSeg);
			}
		segmentManager.Delete (nSegment, false); // delete segP w/o asking "are you sure"
		}
	}
vertexManager.DeleteUnused ();
DLE.MainFrame ()->Progress ().DestroyWindow ();
// wrap back then forward to make sure segment is valid
Wrap (selections [0].m_nSegment, -1, 0, segmentManager.Count () - 1);
Wrap (selections [0].m_nSegment, 1, 0, segmentManager.Count () - 1);
Wrap (selections [1].m_nSegment, -1, 0, segmentManager.Count () - 1);
Wrap (selections [1].m_nSegment, 1, 0, segmentManager.Count () - 1);
undoManager.End ();
segmentManager.SetLinesToDraw ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
}

//eof block.cpp