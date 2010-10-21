// Segment.cpp

#include "types.h"
#include "mine.h"
#include "dle-xp-res.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------
// define points for a given side 
byte sideVertTable[6][4] = {
	{7,6,2,3},
	{0,4,7,3},
	{0,1,5,4},
	{2,6,5,1},
	{4,5,6,7},
	{3,2,1,0} 
};

// define oppisite side of a given side 
byte oppSideTable[6] = {2,3,0,1,5,4};

// define points for the oppisite side of a given side 
byte oppSideVertTable[6][4] = {
	{4,5,1,0},
	{1,5,6,2},
	{3,2,6,7},
	{3,7,4,0},
	{0,1,2,3},
	{7,6,5,4} 
};

// define 2 points for a given line 
byte lineVertTable[12][2] = {
	{0,1},
	{1,2},
	{2,3},
	{3,0},
	{0,4},
	{4,5},
	{5,6},
	{6,7},
	{7,4},
	{1,5},
	{2,6},
	{3,7}
};

// define lines for a given side 
byte sideLineTable[6][4] = {
	{7,10,2,11},
	{4,8,11,3},
	{0,9,5,4},
	{10,6,9,1},
	{5,6,7,8},
	{2,1,0,3} 
};

// define 3 points which connect to a certain point 
byte connectPointTable[8][3] = {
	{1,3,4},
	{2,0,5},
	{3,1,6},
	{0,2,7},
	{7,5,0},
	{4,6,1},
	{5,7,2},
	{6,4,3}
};

// side numbers for each point (3 sides touch each point)
byte pointSideTable[8][3] = {
    {1,2,5},
    {2,3,5},
    {0,3,5},
    {0,1,5},
    {1,2,4},
    {2,3,4},
    {0,3,4},
    {0,1,4}
};

// CUVL corner for pointSideTable above
byte pointCornerTable[8][3] = {
    {0,0,3},
    {1,3,2},
    {2,0,1},
    {3,3,0},
    {1,3,0},
    {2,2,1},
    {1,1,2},
    {0,2,3} 
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static byte segFuncFromType [] = {
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_FUELCEN,
	SEGMENT_FUNC_REPAIRCEN,
	SEGMENT_FUNC_REACTOR,
	SEGMENT_FUNC_ROBOTMAKER,
	SEGMENT_FUNC_GOAL_BLUE,
	SEGMENT_FUNC_GOAL_RED,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_TEAM_BLUE,
	SEGMENT_FUNC_TEAM_RED,
	SEGMENT_FUNC_SPEEDBOOST,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_SKYBOX,
	SEGMENT_FUNC_EQUIPMAKER,
	SEGMENT_FUNC_NONE
	};

static byte segPropsFromType [] = {
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_WATER,
	SEGMENT_PROP_LAVA,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_BLOCKED,
	SEGMENT_PROP_NODAMAGE,
	SEGMENT_PROP_BLOCKED,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_OUTDOORS
	};

void CSegment::Upgrade (void)
{
m_info.props = segPropsFromType [m_info.function];
m_info.function = segFuncFromType [m_info.function];
m_info.damage [0] =
m_info.damage [1] = 0;
}

// -----------------------------------------------------------------------------

byte CSegment::ReadWalls (CFileManager* fp, int nLevelVersion)
{
	byte wallFlags = byte (fp->ReadSByte ());
	int	i;

for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	if (m_info.wallFlags & (1 << i)) 
		m_sides [i].m_info.nWall = (nLevelVersion >= 13) ? fp->ReadUInt16 () : (ushort) fp->ReadByte ();
return wallFlags;
}

// -----------------------------------------------------------------------------

void CSegment::ReadExtras (CFileManager* fp, int nLevelType, int nLevelVersion, bool bExtras)
{
if (bExtras) {
	m_info.function = fp->ReadSByte ();
	m_info.nMatCen = fp->ReadSByte ();
	m_info.value = fp->ReadSByte ();
	fp->ReadSByte ();
	}
else {
	m_info.function = 0;
	m_info.nMatCen = -1;
	m_info.value = 0;
	}
if (nLevelType) {
	if (nLevelVersion < 20)
		Upgrade ();
	else {
		m_info.props = fp->ReadSByte ();
		m_info.damage [0] = fp->ReadInt16 ();
		m_info.damage [1] = fp->ReadInt16 ();
		}
	}
m_info.staticLight = (nLevelType == 0) ? (int) fp->ReadInt16 () : fp->ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CSegment::Read (CFileManager* fp, int nLevelType, int nLevelVersion)
{
	int	i;

if (nLevelVersion >= 9) {
	m_info.owner = fp->ReadSByte ();
	m_info.group = fp->ReadSByte ();
	}
else {
	m_info.owner = -1;
	m_info.group = -1;
	}
// read in child mask (1 byte)
m_info.childFlags = byte (fp->ReadSByte ());

// read 0 to 6 children (0 to 12 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	m_sides [i].m_info.nChild = (m_info.childFlags & (1 << i)) ? fp->ReadInt16 () : -1;

// read vertex numbers (16 bytes)
for (int i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	m_info.verts [i] = fp->ReadInt16 ();

if (nLevelType == 0)
	ReadExtras (fp, nLevelType, nLevelVersion, (m_info.childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

// read the wall bit mask
m_info.wallFlags = byte (fp->ReadSByte ());

// read in wall numbers (0 to 6 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	m_sides [i].m_info.nWall = (m_info.wallFlags & (1 << i)) 
										? ushort ((nLevelVersion < 13) ? fp->ReadSByte () : fp->ReadInt16 ())
										: NO_WALL;

// read in textures and uvls (0 to 60 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	m_sides [i].Read (fp, (Child (i) == -1) || ((m_info.wallFlags & (1 << i)) != 0));
}

// -----------------------------------------------------------------------------

byte CSegment::WriteWalls (CFileManager* fp, int nLevelVersion)
{
	int	i;

m_info.wallFlags = 0;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (m_sides [i].m_info.nWall != NO_WALL) 
		m_info.wallFlags |= (1 << i);
	}
fp->Write (m_info.wallFlags);

// write wall numbers
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (m_info.wallFlags & (1 << i)) {
		CWall _const_ * wallP = m_sides [i].Wall ();
		if (nLevelVersion >= 13)
			fp->WriteUInt16 ((ushort) const_cast<CWall*>(wallP)->Index ());
		else
			fp->WriteSByte ((sbyte) const_cast<CWall*>(wallP)->Index ());
		}
	}
return m_info.wallFlags;
}

// -----------------------------------------------------------------------------

void CSegment::WriteExtras (CFileManager* fp, int nLevelType, bool bExtras)
{
if (bExtras) {
	fp->Write (m_info.function);
	fp->Write (m_info.nMatCen);
	fp->Write (m_info.value);
	fp->WriteByte (0); // s2Flags
	}
if (nLevelType == 2) {
	fp->Write (m_info.props);
	fp->Write (m_info.damage [0]);
	fp->Write (m_info.damage [1]);
	}
if (nLevelType == 0)
	fp->WriteInt16 ((short) m_info.staticLight);
else
	fp->Write (m_info.staticLight);
}

// -----------------------------------------------------------------------------

void CSegment::Write (CFileManager* fp, int nLevelType, int nLevelVersion)
{
	int	i;

if (nLevelType == 2) {
	fp->Write (m_info.owner);
	fp->Write (m_info.group);
	}

#if 1
m_info.childFlags = 0;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (Child (i) != -1) {
		m_info.childFlags |= (1 << i);
		}
	}
if (nLevelType == 0) {
	if (m_info.function != 0) { // if this is a special segment
		m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
		}
	}
#endif
fp->Write (m_info.childFlags);

// write children numbers (0 to 6 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	if (m_info.childFlags & (1 << i)) 
		fp->WriteInt16 (Child (i));

// write vertex numbers (16 bytes)
for (i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	fp->WriteUInt16 ((ushort) vertexManager.Vertex (m_info.verts [i])->Index ());

// write special info (0 to 4 bytes)
if ((m_info.function == SEGMENT_FUNC_ROBOTMAKER) && (m_info.nMatCen == -1)) {
	m_info.function = SEGMENT_FUNC_NONE;
	m_info.value = 0;
	m_info.childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
	}
if (nLevelType == 0)
	WriteExtras (fp, nLevelType, (m_info.childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

// calculate wall bit mask
WriteWalls (fp, nLevelVersion);
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	if ((Child (i) == -1) || (m_info.wallFlags & (1 << i))) 
		m_sides [i].Write (fp);
}

// -----------------------------------------------------------------------------

void CSegment::Setup (void)
{
	int i;

m_info.owner = -1;
m_info.group = -1;
m_info.function = 0; 
m_info.nMatCen = -1; 
m_info.value = -1; 
m_info.childFlags = 0;
m_info.wallFlags = 0; 
m_info.flags = 0;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	m_sides [i].Setup (); 
	SetUV (i, 0, 0); 
	SetChild (i, -1);
	}
m_info.staticLight = 0; 
}

// -----------------------------------------------------------------------------

void CSegment::SetUV (short nSide, short x, short y)
{
	CDoubleVector	A [4], B [4], C [4], D [4], E [4]; 
	int				i; 
	double			angle, sinAngle, cosAngle; 

// for testing, x is used to tell how far to convert vector
// 0, 1, 2, 3 represent B, C, D, E coordinate transformations

// copy side's four points into A

for (i = 0; i < 4; i++)
	A [i] = CDoubleVector (*vertexManager.Vertex (m_info.verts [sideVertTable [nSide][i]])); 

// subtract point 0 from all points in A to form B points
for (i = 0; i < 4; i++) 
	B [i] = A [i] - A [0]; 

// calculate angle to put point 1 in x - y plane by spinning on x - axis
// then rotate B points on x - axis to form C points.
// check to see if on x - axis already
angle = atan3 (B [1].v.z, B [1].v.y); 
sinAngle = sin (angle);
cosAngle = cos (angle);
for (i = 0; i < 4; i++) 
	C [i].Set (B [i].v.x, B [i].v.y * cosAngle + B [i].v.z * sinAngle, -B [i].v.y * sinAngle + B [i].v.z * cosAngle); 

#if UV_DEBUG
if (abs((int)C [1].z) != 0) {
	sprintf_s (message, sizeof (message),  "SetUV: point 1 not in x/y plane\n(%f); angle = %f", (float)C [1].z, (float)angle); 
	DEBUGMSG (message); 
	}
#endif

// calculate angle to put point 1 on x axis by spinning on z - axis
// then rotate C points on z - axis to form D points
// check to see if on z - axis already
angle = atan3 (C [1].v.y, C [1].v.x); 
sinAngle = sin (angle);
cosAngle = cos (angle);
for (i = 0; i < 4; i++) {
	D [i].Set (C [i].v.x * cosAngle + C [i].v.y * sinAngle, -C [i].v.x * sinAngle + C [i].v.y * cosAngle, C [i].v.z); 
	}
#if UV_DEBUG
if (abs((int)D [1].y) != 0) {
	DEBUGMSG (" SetUV: Point 1 not in x axis"); 
	}
#endif

// calculate angle to put point 2 in x - y plane by spinning on x - axis
// the rotate D points on x - axis to form E points
// check to see if on x - axis already
angle = atan3 (D [2].v.z, D [2].v.y); 
for (i = 0; i < 4; i++) 
	E [i].Set (D [i].v.x, D [i].v.y * cos (angle) + D [i].v.z * sin (angle), -D [i].v.y * sin (angle) + D [i].v.z * cos (angle)); 

// now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
// set v to x axis and u to negative u axis to match default (u, v)
// (remember to scale by dividing by 640)
CUVL *uvls = m_sides [nSide].m_info.uvls;
#if UV_DEBUG
switch (x) {
	case 0:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (B [i].x/640); 
			uvls [i].u = - (B [i].y/640); 
			}
		break; 
	case 1:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (C [i].x/640); 
			uvls [i].u = 0x400/10 - (C [i].y/640); 
			}
		break; 
	case 2:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (D [i].x/640); 
			uvls [i].u = 2*0x400/10 - (D [i].y/640); 
			}
		break; 
	case 3:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (E [i].x/640); 
			uvls [i].u = 3*0x400/10 - (E [i].y/640); 
			}
	break; 
	}
#else
undoManager.Begin (udSegments); 
m_sides [nSide].LoadTextures ();
double scale = 1.0; //textureManager.Textures () [m_fileType][sideP->m_info.nBaseTex].Scale (sideP->m_info.nBaseTex);
for (i = 0; i < 4; i++, uvls++) {
	uvls->v = (short) ((y + D2X (E [i].v.x / 640)) / scale); 
	uvls->u = (short) ((x - D2X (E [i].v.y / 640)) / scale); 
	}
undoManager.End ();
#endif
}

// -----------------------------------------------------------------------------

short CSegment::SetChild (short nSide, short nSegment) 
{
m_sides [nSide].m_info.nChild = nSegment;
if (nSegment == -1)
	m_info.childFlags &= ~(1 << nSide);
else
	m_info.childFlags |= (1 << nSide);
return nSegment;
}

// -----------------------------------------------------------------------------

CVertex _const_ * CSegment::Vertex (short nVertex) _const_
{
return vertexManager.Vertex (m_info.verts [nVertex]);
}

// -----------------------------------------------------------------------------

void CSegment::UpdateChildren (short nOldChild, short nNewChild)
{
for (short nSide = 0; nSide < 6; nSide++)
	if (m_sides [nSide].UpdateChild (nOldChild, nNewChild))	// no two sides can have the same child
		return;
}

// -----------------------------------------------------------------------------

void CSegment::Reset (short nSide)
{
m_info.group = -1;
m_info.owner = -1;
m_info.function = 0;
m_info.value = -1;
m_info.nMatCen = -1;
m_info.wallFlags = 0;
m_info.flags = 0;
SetChild (nSide, -1); 
m_info.childFlags &= ~(1 << nSide); 
if (nSide < 0) {
	for (nSide = 0; nSide < 6; nSide++)
		m_sides [nSide].Reset ();
	}	
else
	m_sides [nSide].Reset ();
}

// -----------------------------------------------------------------------------

CGameItem* CSegment::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CSegment*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

CGameItem* CSegment::Clone (void)
{
return Copy (new CSegment);	// only make a copy if modified
}

// -----------------------------------------------------------------------------

void CSegment::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------

void CSegment::Undo (void)
{
switch (EditType ()) {
	case opAdd:
		segmentManager.Remove (Index ());
		break;
	case opDelete:
		segmentManager.Add ();
		// fall through
	case opModify:
		*Parent () = *this;
		break;
	}
}

// -----------------------------------------------------------------------------

void CSegment::Redo (void)
{
switch (EditType ()) {
	case opDelete:
		segmentManager.Remove (Index ());
		break;
	case opAdd:
		segmentManager.Add ();
		// fall through
	case opModify:
		*Parent () = *this;
		break;
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CMatCenter::Read (CFileManager* fp, int version, bool bFlag)
{
m_info.objFlags [0] = fp->ReadInt32 ();
if (DLE.IsD2File ())
	m_info.objFlags [1] = fp->ReadInt32 ();
m_info.hitPoints = fp->ReadInt32 ();
m_info.interval = fp->ReadInt32 ();
m_info.nSegment = fp->ReadInt16 ();
m_info.nFuelCen = fp->ReadInt16 ();
}

// -----------------------------------------------------------------------------

void CMatCenter::Write (CFileManager* fp, int version, bool bFlag)
{
fp->Write (m_info.objFlags [0]);
if (DLE.IsD2File ())
	fp->Write (m_info.objFlags [1]);
fp->Write (m_info.hitPoints);
fp->Write (m_info.interval);
fp->Write (m_info.nSegment);
fp->Write (m_info.nFuelCen);
}

// -----------------------------------------------------------------------------
// make a copy of this segment for the undo manager
// if segment was modified, make a copy of the current segment
// if segment was added or deleted, just make a new CGameItem instance and 
// mark the operation there

CGameItem* CMatCenter::Clone (void)
{
CMatCenter* cloneP = new CMatCenter;	// only make a copy if modified
if (cloneP != null)
	*cloneP = *this;
return cloneP;
}

// -----------------------------------------------------------------------------

CGameItem* CMatCenter::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CMatCenter*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

void CMatCenter::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//eof segment.cpp